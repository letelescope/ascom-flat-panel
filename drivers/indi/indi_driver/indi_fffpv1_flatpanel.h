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

    virtual bool SetLightBoxBrightness(uint16_t value) override;
    virtual bool EnableLightBox(bool enable) override;

    virtual IPState ParkCap() override;
    virtual IPState UnParkCap() override;


private: // serial connection
    bool Handshake();
    bool sendCommand(const char *cmd);
    int PortFD{-1};

    Connection::Serial *serialConnection{nullptr};
};
