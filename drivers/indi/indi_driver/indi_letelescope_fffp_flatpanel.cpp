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

    // initialize the parent's properties first
    //initLightBoxProperties(getDeviceName(), MAIN_CONTROL_TAB);

    // TODO: Add any custom properties you need here.

    // Add debug/simulation/etc controls to the driver.
    addAuxControls();

    setDriverInterface(DUSTCAP_INTERFACE| LIGHTBOX_INTERFACE | AUX_INTERFACE);

    serialConnection = new Connection::Serial(this);
    serialConnection->registerHandshake([&]() { return Handshake(); });
    serialConnection->setDefaultBaudRate(Connection::Serial::B_57600);
    serialConnection->setDefaultPort("/dev/ttyACM0");
    registerConnection(serialConnection);

    return true;
}

void FFFPFlatPanel::ISGetProperties(const char *dev)
{
    INDI::DefaultDevice::ISGetProperties(dev);

    //isGetLightBoxProperties(dev);
}

bool FFFPFlatPanel::updateProperties()
{
    INDI::DefaultDevice::updateProperties();

    //if (!updateLightBoxProperties())
    //{
    //    return false;
    //}

    if (isConnected())
    {
        // TODO: Call define* for any custom properties only visible when connected.
    }
    else
    {
        // TODO: Call deleteProperty for any custom properties only visible when connected.
    }

    return true;
}

bool FFFPFlatPanel::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Number properties.
    }

    //if (processLightBoxNumber(dev, name, values, names, n))
    //{
    //    return true;
    //}

    // Nobody has claimed this, so let the parent handle it
    return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}

bool FFFPFlatPanel::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Switch properties.
    }

    //if (processLightBoxSwitch(dev, name, states, names, n))
    //{
    //    return true;
    //}

    // Nobody has claimed this, so let the parent handle it
    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

bool FFFPFlatPanel::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Text properties.
    }

    //if (processLightBoxText(dev, name, texts, names, n))
    //{
    //    return true;
    //}

    // Nobody has claimed this, so let the parent handle it
    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

bool FFFPFlatPanel::ISSnoopDevice(XMLEle *root)
{
    // TODO: Check to see if this is for any of my custom Snoops. Fo shizzle.

    //snoopLightBox(root);

    return INDI::DefaultDevice::ISSnoopDevice(root);
}

bool FFFPFlatPanel::saveConfigItems(FILE *fp)
{
    //saveLightBoxConfigItems(fp);

    // TODO: Call IUSaveConfig* for any custom properties I want to save.

    return INDI::DefaultDevice::saveConfigItems(fp);
}

bool FFFPFlatPanel::Handshake()
{
    if (isSimulation())
    {
        LOGF_INFO("Connected successfuly to simulated %s.", getDeviceName());
        return true;
    }

    PortFD = serialConnection->getPortFD();

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
    // TODO: Implement your own code to set the brightness of the lightbox.
    // Be sure to return true if successful, or false otherwise.

    INDI_UNUSED(value);

    return false;
}

bool FFFPFlatPanel::EnableLightBox(bool enable)
{
    // TODO: Implement your own code to turn on/off the lightbox.
    // Be sure to return true if successful, or false otherwise.

    INDI_UNUSED(enable);

    return false;
}

IPState FFFPFlatPanel::ParkCap()
{
    // TODO: Implement your own code to close the dust cap.

    return IPS_OK;
}

IPState FFFPFlatPanel::UnParkCap()
{
    // TODO: Implement your own code to open the dust cap.

    return IPS_OK;
}