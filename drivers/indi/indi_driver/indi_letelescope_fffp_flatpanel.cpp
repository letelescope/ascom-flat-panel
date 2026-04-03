//
// Indi AuxDevice, LightBox and DustCap driver for LeTelescopeFFFP
//
// Copyright(C) 2025 - Present, Le Télescope - Ivry sur Seine - All Rights Reserved
// Licensed under the MIT License. See the accompanying LICENSE file for terms.
//
// Description: Implemenation of the FFFPFlatPanel class.
//
// The FFFPFlatPanel class represents a driver instance controlling flat panel Hardware/Firmware. 
//
// In this scenario a flat panel is made of two things
//   - A light panel with variable brightness, called calibrator in this driver
//   - A motorized cover
//
// The firmware updates the state of the flat panel upon instructions of the ASCOM driver. The communication protocol 
// is serial based. The serial connection is established via USB. Here is a a summary of the protocol 
//  - Both the driver and the flat panel (and hence this firmware) exchange single line messages.
//  - A message is structured as TYPE:MESSAGE, where TYPE is in "COMMAND, RESULT, ERROR" and MESSAGE is alaphanumerical with spaces and "@"
//
// This Driver only emmits incoming "COMMAND" messages. This kind of message is stuctured as 
//  
// COMMAND:NAME[@ARGS]
//
// where NAME is [A - Z_] + and ARGS are optional and their nature may depend on the command. For instance for a COMMAND:BRIGHTNESS_SET @ARGS message the ARGS
// are mandatory and should consists of a single "int"
//
// This firware reponse with 
// - either a RESULT:CMD_NAME@MSG   if operation succeeded
// - or an ERROR:ERR_MESSAGE @DETAILS if anything went wrong
//
// 
// In a nutshell
//  --------------                           ------------
// |              |                         |            |
// |              | COMMAND:CMD_NAME[@ARGS] | Flat Panel |
// |  Indi driver | --------------------->  | (firmware) |
// |              | < --------------------  |            |
//  --------------   RESULT:CMD_NAME @MSG    ------------
//                          or
//                 ERROR:ERR_MESSAGE @DETAILS
//
// The protocol, the implementation of the firmware, the electronics and the 3D models
// are heavily inspired by the the work  of Dark Sky Geek (https://github.com/jlecomte/) especially 
// - https://github.com/jlecomte/ascom-flat-panel
// - https://github.com/jlecomte/ascom-wireless-flat-panel
// - https://github.com/jlecomte/ascom-telescope-cover-v2
//
// Implements:	INDI::LightBoxInterface, INDI::DustCapInterface
// Inherits:    INDI::DefaultDevice
// 
// Authors:	    Florian Thibaud	
//              Florian Gautier
#include <cstring>

#include "libindi/indicom.h"
#include "libindi/connectionplugins/connectionserial.h"

#include "config.h"
#include "indi_letelescope_fffp_flatpanel.h"

// We declare an auto pointer to FFFPFlatPanel.
static std::unique_ptr<FFFPFlatPanel> mydriver(new FFFPFlatPanel());

FFFPFlatPanel::FFFPFlatPanel() : 
    INDI::LightBoxInterface(this),//(this, true),
    INDI::DustCapInterface(this)
{
    setVersion(CDRIVER_VERSION_MAJOR, CDRIVER_VERSION_MINOR);
}

const char *FFFPFlatPanel::getDefaultName()
{
    return "Le Telescope FFFP Flat Panel";
}

bool FFFPFlatPanel::initProperties()
{
    // initialize the parent's properties first
    INDI::DefaultDevice::initProperties();

    // Firmware version
    IUFillText(&FirmwareT[0], "Version", "Version", nullptr);
    IUFillTextVector(&FirmwareTP, FirmwareT, 1, getDeviceName(), "Firmware", "Firmware", CONNECTION_TAB, IP_RO, 60, IPS_IDLE);

    DI::initProperties(MAIN_CONTROL_TAB);
    LI::initProperties(MAIN_CONTROL_TAB, CAN_DIM);
    
    LightIntensityNP[0].setMin(0);
    LightIntensityNP[0].setStep(10);

    // Add debug/simulation/etc controls to the driver.
    addAuxControls();

    setDriverInterface(DUSTCAP_INTERFACE| LIGHTBOX_INTERFACE | AUX_INTERFACE);

    serialConnection = new Connection::Serial(this);
    serialConnection->registerHandshake([&]() { return Handshake(); });
    serialConnection->setDefaultBaudRate(Connection::Serial::B_57600);
    registerConnection(serialConnection);

    return true;
}

void FFFPFlatPanel::ISGetProperties(const char *dev)
{
    INDI::DefaultDevice::ISGetProperties(dev);

    // Get Light box properties
    LI::ISGetProperties(dev);
}

bool FFFPFlatPanel::updateProperties()
{
    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {

        DI::updateProperties();
        LI::updateProperties();
        
        FirmwareT[0].text = const_cast<char*>(firmareVersion.c_str());
        LightIntensityNP[0].setMax(maxSupportedBrightness);
        
        defineProperty(&FirmwareTP);
    }
    else
    {
        deleteProperty(FirmwareTP.name);
        DI::updateProperties();
        LI::updateProperties();
    }

    return true;
}

bool FFFPFlatPanel::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    if (LI::processNumber(dev, name, values, names, n))
        return true;

    return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}

bool FFFPFlatPanel::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        if (DI::processSwitch(dev, name, states, names, n))
            return true;

        if (LI::processSwitch(dev, name, states, names, n))
            return true;
    }

    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);

}

bool FFFPFlatPanel::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        if (LI::processText(dev, name, texts, names, n))
            return true;
    }

    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

bool FFFPFlatPanel::ISSnoopDevice(XMLEle *root)
{
    // TODO: Check to see if this is for any of my custom Snoops. Fo shizzle.

    LI::snoop(root);

    return INDI::DefaultDevice::ISSnoopDevice(root);
}

bool FFFPFlatPanel::saveConfigItems(FILE *fp)
{
    INDI::DefaultDevice::saveConfigItems(fp);

    return LI::saveConfigItems(fp);
}

bool FFFPFlatPanel::Handshake()
{
    if (isSimulation())
    {
        hardwareAdapter = std::make_unique<SimulationHardwareAdapter>();
    }
    else
    {
        hardwareAdapter = std::make_unique<LeTelescopeFFFPHardwareAdapter>();
    }

    if (!hardwareAdapter)
    {
        LOGF_ERROR("%s: Failed to allocate hardware adapter.", getDeviceName());
        return false;
    }

    int PortFD = serialConnection->getPortFD();
    hardwareAdapter->init(PortFD);

    if (!hardwareAdapter->ping())
    {
        LOGF_ERROR("%s: Hardware adapter ping failed.", getDeviceName());
        return false;
    }

    // Retrieve and store the maximum brightness after successful ping
    int brightness = 0;
    if (hardwareAdapter->getMaxBrightness(&brightness))
    {
        maxSupportedBrightness = brightness;
        LOGF_INFO("%s: Max brightness set to %d.", getDeviceName(), maxSupportedBrightness);
    }
    else
    {
        LOGF_WARN("%s: Failed to query max brightness from adapter.", getDeviceName());
    }

    char version[MAXRBUF];
    if (hardwareAdapter->getFirmwareVersion(version))
    {
        firmareVersion = version;
        LOGF_INFO("Connected successfuly to %s (%s).", getDeviceName(), version);
    }
    else
    {
        LOGF_INFO("Connected successfuly to %s (unknown firmware).", getDeviceName());
    }

    return true;
}

void FFFPFlatPanel::TimerHit()
{
    if (!isConnected())
        return;

    // TODO: Poll your device if necessary. Otherwise delete this method and it's
    // declaration in the header file.

    LOG_INFO("timer hit");

    // If you don't call SetTimer, we'll never get called again, until we disconnect
    // and reconnect.
    SetTimer(POLLMS);
}

bool FFFPFlatPanel::SetLightBoxBrightness(uint16_t value)
{
    return hardwareAdapter? hardwareAdapter->setBrightness(static_cast<int>(value)) : false;
}

bool FFFPFlatPanel::EnableLightBox(bool enable)
{
    if (!hardwareAdapter)
        return false;

    if (enable)
        return hardwareAdapter->lightOn();
    else
        return hardwareAdapter->lightOff();
}

IPState FFFPFlatPanel::ParkCap()
{
    if (!hardwareAdapter)
        return IPS_ALERT;

    if (!hardwareAdapter->closeCover())
        return IPS_ALERT;

    return IPS_BUSY;
}

IPState FFFPFlatPanel::UnParkCap()
{
    if (!hardwareAdapter)
        return IPS_ALERT;

    if (!hardwareAdapter->openCover())
        return IPS_ALERT;

    return IPS_BUSY;
}