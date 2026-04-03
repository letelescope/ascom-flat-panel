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
// 
// The driver architecture is is heavily inspired by the Gemini Adapter for the Gemini Flat Panel (https://github.com/indilib/indi/blob/master/drivers/auxiliary/gemini_flatpanel_adapters.h)
// as well as the Flip Flat driver (https://github.com/indilib/indi/blob/master/drivers/auxiliary/flip_flat.h)
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

    LightIntensityNP[0].setMin(0);
    LightIntensityNP[0].setMax(255);
    LightIntensityNP[0].setStep(1);
    
    DI::initProperties(MAIN_CONTROL_TAB);
    LI::initProperties(MAIN_CONTROL_TAB, CAN_DIM);

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

        LI::updateProperties();        
        DI::updateProperties();
    
        FirmwareT[0].text = const_cast<char*>(firmareVersion.c_str());    
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
        LOGF_WARN("%s: Failed to query max brightness from hardware.", getDeviceName());
    }

    char version[MAXRBUF];
    if (hardwareAdapter->getFirmwareVersion(version))
    {
        firmareVersion = version;
        LOGF_INFO("Connected successfuly to %s (%s).", getDeviceName(), version);
    }
    else
    {
        LOGF_INFO("%s: Failed to query firmware version from hardware.", getDeviceName());
    }

    return true;
}

void FFFPFlatPanel::TimerHit()
{
    if (!isConnected())
        return;

    IPState currentCapState = ParkCapSP.getState();
    if (currentCapState == IPS_BUSY || currentCapState == IPS_ALERT)
    {
        PanelCoverStatus currentStatus = COVER_STATUS_UNKNOWN;
        if (!hardwareAdapter->getCoverStatus(&currentStatus))
        {
            ParkCapSP.setState(IPS_ALERT);
            ParkCapSP.apply();
            LOGF_ERROR("%s: Failed to get cap status during polling.", getDeviceName());
            SetTimer(10 * POLLMS);
            return;
        }

        if (currentStatus == COVER_STATUS_CLOSED)
        {
            ParkCapSP[0].s = ISS_ON;
            ParkCapSP[1].s = ISS_OFF;
            ParkCapSP.setState(IPS_OK);
            ParkCapSP.apply();
            LOGF_INFO("%s: Cap parked successfully.", getDeviceName());
        }
        else if (currentStatus == COVER_STATUS_OPEN)
        {
            ParkCapSP[0].s = ISS_OFF;
            ParkCapSP[1].s = ISS_ON;
            ParkCapSP.setState(IPS_OK);
            ParkCapSP.apply();
            LOGF_INFO("%s: Cap unparked, cap is open.", getDeviceName());
        }
        else if(currentStatus == COVER_STATUS_UNKNOWN)
        {
            ParkCapSP.setState(IPS_ALERT);
            ParkCapSP.apply();
            LOGF_ERROR("%s: Failed to get cap status during parking operation.", getDeviceName());
        }
    }

    if(ParkCapSP.getState() == IPS_BUSY)
    {
        // If you don't call SetTimer, we'll never get called again, until we disconnect
        // and reconnect.
        SetTimer(POLLMS);
    } else if (ParkCapSP.getState() == IPS_ALERT)
    {
        SetTimer(10 * POLLMS); // The harware may be in a dangling state. Wait a bit longer before polling again to avoid spamming the logs if the device is in a bad state.
    }

}

int FFFPFlatPanel::userToHardwareBrightness(uint16_t userValue) const
{
    if (maxSupportedBrightness <= 0)
        return 0;

    int hw = (static_cast<int>(userValue) * maxSupportedBrightness + USER_BRIGHTNESS_MAX / 2) / USER_BRIGHTNESS_MAX;
    if (hw < 0)
        hw = 0;
    if (hw > maxSupportedBrightness)
        hw = maxSupportedBrightness;
    return hw;
}

uint16_t FFFPFlatPanel::hardwareToUserBrightness(int hardwareValue) const
{
    if (maxSupportedBrightness <= 0)
        return 0;

    int user = (hardwareValue * USER_BRIGHTNESS_MAX + maxSupportedBrightness / 2) / maxSupportedBrightness;
    if (user < 0)
        user = 0;
    if (user > USER_BRIGHTNESS_MAX)
        user = USER_BRIGHTNESS_MAX;
    return static_cast<uint16_t>(user);
}

bool FFFPFlatPanel::SetLightBoxBrightness(uint16_t value)
{
    if (!hardwareAdapter || maxSupportedBrightness <= 0)
        return false;

    int hwValue = userToHardwareBrightness(value);
    bool result = hardwareAdapter->setBrightness(hwValue);

    if (result)
    {
        // keep the INDI property in sync with the user scale
        LightIntensityNP[0].value = value;
        
        if (value > 0)
        {
            LightSP[0].s = ISS_ON;
            LightSP[1].s = ISS_OFF;
            LightIntensityNP.setState(IPS_OK);
            LightSP.setState(IPS_OK);  
        }
        else
        {
            LightSP[0].s = ISS_OFF;
            LightSP[1].s = ISS_ON;
            LightIntensityNP.setState(IPS_IDLE);
            LightSP.setState(IPS_IDLE);
        }
        LightIntensityNP.apply();
        LightSP.apply();
        LOGF_INFO("%s: Brightness set to %d (hardware value: %d).", getDeviceName(), value, hwValue);
    } else
    {
        LOGF_ERROR("%s: Failed to set brightness to %d (hardware value: %d).", getDeviceName(), value, hwValue);
    }


    return result;
}

bool FFFPFlatPanel::EnableLightBox(bool enable)
{
    if (!hardwareAdapter)
        return false;

    if (enable)
    {
        int currentBrightness = LightIntensityNP[0].value;
        return SetLightBoxBrightness(currentBrightness);
    }
    else
    {
        if (!hardwareAdapter->lightOff())
        {
            LOGF_ERROR("%s: Failed to turn off the light box.", getDeviceName());
            return false;
        }
        LightIntensityNP.setState(IPS_IDLE);
        LightSP.setState(IPS_IDLE);
        LightIntensityNP.apply();
        LightSP.apply();
        LOGF_INFO("%s: Light box turned off.", getDeviceName());
        return true;
    }
}

IPState FFFPFlatPanel::ParkCap()
{
    if (!hardwareAdapter)
        return IPS_ALERT;

    if (!hardwareAdapter->closeCover())
    {
        LOGF_ERROR("%s: Failed to close the cap.", getDeviceName());
        SetTimer(10 * POLLMS); // Start polling to monitor the cover status after failed attempt.   
        return IPS_ALERT;
    }

    LOGF_INFO("%s: Attempting to close the cap.", getDeviceName());
    SetTimer(POLLMS); // Start polling to monitor the cover status after sending the close command.
    return IPS_BUSY;
}

IPState FFFPFlatPanel::UnParkCap()
{
    if (!hardwareAdapter)
        return IPS_ALERT;

    if (!hardwareAdapter->openCover())
    {
        LOGF_ERROR("%s: Failed to open the cap.", getDeviceName());
        SetTimer(10 * POLLMS); // Start polling to monitor the cover status after failed attempt.
        return IPS_ALERT;
    }

    LOGF_INFO("%s: Attempting to open the cap.", getDeviceName());
    SetTimer(POLLMS); // Start polling to monitor the cover status after sending the open command.
    return IPS_BUSY;
}