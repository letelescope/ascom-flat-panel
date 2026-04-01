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

    // Initialize helper properties used by the driver client
    IUFillText(&FirmwareT[0], "FIRMWARE", "Firmware version", "unknown");
    IUFillTextVector(&FirmwareTP, FirmwareT, 1, getDeviceName(), "FFFP_FIRMWARE", "Firmware version", MAIN_CONTROL_TAB, IP_RO, 0, IPS_OK);
    IDDefText(&FirmwareTP, nullptr);

    IUFillText(&MaxBrightnessT[0], "MAX_BRIGHTNESS", "Max brightness", "0");
    IUFillTextVector(&MaxBrightnessTP, MaxBrightnessT, 1, getDeviceName(), "FFFP_MAX_BRIGHTNESS", "Max brightness", MAIN_CONTROL_TAB, IP_RO, 0, IPS_OK);
    IDDefText(&MaxBrightnessTP, nullptr);

    IUFillSwitch(&LightSwitchS[0], "ON", "Light On", ISS_OFF);
    IUFillSwitch(&LightSwitchS[1], "OFF", "Light Off", ISS_ON);
    IUFillSwitchVector(&LightSwitchSP, LightSwitchS, 2, getDeviceName(), "FFFP_LIGHT", "Light control", MAIN_CONTROL_TAB, IP_RW, ISR_1OFMANY, 0, IPS_OK);
    IDDefSwitch(&LightSwitchSP, nullptr);

    IUFillNumber(&BrightnessN[0], "BRIGHTNESS", "Brightness", "%0.0f", 0, 100, 1, 0);
    IUFillNumberVector(&BrightnessNP, BrightnessN, 1, getDeviceName(), "FFFP_BRIGHTNESS_SET", "Set brightness", MAIN_CONTROL_TAB, IP_RW, 0, IPS_OK);
    IDDefNumber(&BrightnessNP, nullptr);

    IUFillSwitch(&CoverSwitchS[0], "OPEN", "Open Cover", ISS_OFF);
    IUFillSwitch(&CoverSwitchS[1], "CLOSE", "Close Cover", ISS_ON);
    IUFillSwitchVector(&CoverSwitchSP, CoverSwitchS, 2, getDeviceName(), "FFFP_COVER", "Cover control", MAIN_CONTROL_TAB, IP_RW, ISR_1OFMANY, 0, IPS_OK);
    IDDefSwitch(&CoverSwitchSP, nullptr);

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
        if (hardwareAdapter)
        {
            char version[MAXRBUF] = {0};
            if (hardwareAdapter->getFirmwareVersion(version))
            {
                IUSaveText(&FirmwareT[0], version);
                IDSetText(&FirmwareTP, nullptr);
            }

            int maxVal = 0;
            if (hardwareAdapter->getMaxBrightness(&maxVal))
            {
                char maxBrightnessStr[64];
                snprintf(maxBrightnessStr, sizeof(maxBrightnessStr), "%d", maxVal);
                IUSaveText(&MaxBrightnessT[0], maxBrightnessStr);
                IDSetText(&MaxBrightnessTP, nullptr);
                maxSupportedBrightness = maxVal;

                BrightnessN[0].max = maxVal;
                IDSetNumber(&BrightnessNP, nullptr);
            }

            // Keep toggles in sync
            if (hardwareAdapter->getBrightness(&maxVal))
            {
                // `maxVal` returns current brightness.
                if (maxVal > 0)
                {
                    LightSwitchS[0].s = ISS_ON;
                    LightSwitchS[1].s = ISS_OFF;
                }
                else
                {
                    LightSwitchS[0].s = ISS_OFF;
                    LightSwitchS[1].s = ISS_ON;
                }
                IDSetSwitch(&LightSwitchSP, nullptr);
            }

            PanelCoverStatus coverStatus{};
            if (hardwareAdapter->getCoverStatus(&coverStatus))
            {
                if (coverStatus == COVER_STATUS_OPEN || coverStatus == COVER_STATUS_OPENING)
                {
                    CoverSwitchS[0].s = ISS_ON;
                    CoverSwitchS[1].s = ISS_OFF;
                }
                else
                {
                    CoverSwitchS[0].s = ISS_OFF;
                    CoverSwitchS[1].s = ISS_ON;
                }
                IDSetSwitch(&CoverSwitchSP, nullptr);
            }
        }
    }
    else
    {
        IDDelete(getDeviceName(), "FFFP_FIRMWARE", nullptr);
        IDDelete(getDeviceName(), "FFFP_MAX_BRIGHTNESS", nullptr);
        IDDelete(getDeviceName(), "FFFP_LIGHT", nullptr);
        IDDelete(getDeviceName(), "FFFP_BRIGHTNESS_SET", nullptr);
        IDDelete(getDeviceName(), "FFFP_COVER", nullptr);
    }

    return true;
}

bool FFFPFlatPanel::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0 && name != nullptr && n > 0)
    {
        if (strcmp(name, "FFFP_BRIGHTNESS_SET") == 0)
        {
            int brightness = static_cast<int>(values[0]);
            if (hardwareAdapter && hardwareAdapter->setBrightness(brightness))
            {
                BrightnessN[0].value = brightness;
                IDSetNumber(&BrightnessNP, "Brightness set to %d", brightness);

                LightSwitchS[0].s = (brightness > 0) ? ISS_ON : ISS_OFF;
                LightSwitchS[1].s = (brightness > 0) ? ISS_OFF : ISS_ON;
                IDSetSwitch(&LightSwitchSP, nullptr);
                return true;
            }
            IDSetNumber(&BrightnessNP, "Failed to set brightness");
            return false;
        }
    }

    return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}

bool FFFPFlatPanel::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    if (dev == nullptr || strncmp(dev, getDeviceName(), MAXINDINAME) != 0 || name == nullptr || states == nullptr || n <= 0)
        return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);

    if (strcmp(name, "FFFP_LIGHT") == 0 && n == 2)
    {
        if (states[0] == ISS_ON)
        {
            if (hardwareAdapter)
                hardwareAdapter->lightOn();
            LightSwitchS[0].s = ISS_ON;
            LightSwitchS[1].s = ISS_OFF;
        }
        else if (states[1] == ISS_ON)
        {
            if (hardwareAdapter)
                hardwareAdapter->lightOff();
            LightSwitchS[0].s = ISS_OFF;
            LightSwitchS[1].s = ISS_ON;
        }
        IDSetSwitch(&LightSwitchSP, nullptr);
        return true;
    }

    if (strcmp(name, "FFFP_COVER") == 0 && n == 2)
    {
        if (states[0] == ISS_ON)
        {
            if (hardwareAdapter)
                hardwareAdapter->openCover();
            CoverSwitchS[0].s = ISS_ON;
            CoverSwitchS[1].s = ISS_OFF;
        }
        else if (states[1] == ISS_ON)
        {
            if (hardwareAdapter)
                hardwareAdapter->closeCover();
            CoverSwitchS[0].s = ISS_OFF;
            CoverSwitchS[1].s = ISS_ON;
        }
        IDSetSwitch(&CoverSwitchSP, nullptr);
        return true;
    }

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

    PortFD = serialConnection->getPortFD();
    hardwareAdapter->setupCommunication(PortFD);

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
    if (!hardwareAdapter)
        return false;

    if (!hardwareAdapter->setBrightness(static_cast<int>(value)))
        return false;

    // NOTE: Update INDI state may be required here with actual implementation.
    return true;
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

    return IPS_OK;
}

IPState FFFPFlatPanel::UnParkCap()
{
    if (!hardwareAdapter)
        return IPS_ALERT;

    if (!hardwareAdapter->openCover())
        return IPS_ALERT;

    return IPS_OK;
}