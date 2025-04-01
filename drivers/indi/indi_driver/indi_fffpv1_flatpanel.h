//
// Indi AuxDevice, LightBox and DustCap driver for LeTelescopeFFFPV1
//
// Copyright(C) 2025 - Present, Le Télescope - Ivry sur Seine - All Rights Reserved
// Licensed under the MIT License. See the accompanying LICENSE file for terms.
//
// Description: Definitiion of the FFFPV1FlatPanel class.
//
// The FFFPV1FlatPanel class represents a driver instance controlling flat panel Hardware/Firmware. 
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
#pragma once

#include "libindi/defaultdevice.h"
#include "libindi/indilightboxinterface.h"
#include "libindi/indidustcapinterface.h"

namespace Connection
{
    class Serial;
}

class FFFPV1FlatPanel : public INDI::DefaultDevice, public INDI::LightBoxInterface, public INDI::DustCapInterface
{
public:
    FFFPV1FlatPanel();
    virtual ~FFFPV1FlatPanel() = default;

    virtual const char *getDefaultName() override;

    virtual bool initProperties() override;
    virtual bool updateProperties() override;

    virtual void ISGetProperties(const char *dev) override;
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;
    virtual bool ISSnoopDevice(XMLEle *root) override;

    virtual void TimerHit() override;

protected:
    virtual bool saveConfigItems(FILE *fp) override;

    // LightBoxInterface methodes
    virtual bool SetLightBoxBrightness(uint16_t value) override;
    virtual bool EnableLightBox(bool enable) override;

    // DustCap Interface methodes
    virtual IPState ParkCap() override;
    virtual IPState UnParkCap() override;


private: // serial connection
    bool Handshake();
    bool sendCommand(const char *cmd);
    int PortFD{-1};

    Connection::Serial *serialConnection{nullptr};
};
