//
// Hardware adapter implementation for the LeTelescopeFFFP Flat Panel. 
// 
// This class is responsible for managing the connection to the flat panel, sending commands and receiving responses.
//
// This is heavily inspired by the Gemini Adapter for the Gemini Flat Panel (https://github.com/indilib/indi/blob/master/drivers/auxiliary/gemini_flatpanel_adapters.h)
//
// Copyright(C) 2025 - Present, Le Télescope - Ivry sur Seine - All Rights Reserved
// Licensed under the MIT License. See the accompanying LICENSE file for terms.
// 
// Authors:	    Florian Thibaud	
//              Florian Gautier
#include "indi_letelescope_fffp_harware_adapter.h"
#include "libindi/indibase.h"
#include "libindi/indicom.h"
#include <cstring>
#include <cstdio>
#include <termios.h>

static constexpr int SIM_MAX_BRIGHTNESS = 100;

LeTelescopeFFFPHardwareAdapter::LeTelescopeFFFPHardwareAdapter()
    : serialPortFD(-1), maxBrightness(SIM_MAX_BRIGHTNESS)
{
}

LeTelescopeFFFPHardwareAdapter::~LeTelescopeFFFPHardwareAdapter() = default;

bool LeTelescopeFFFPHardwareAdapter::ping()
{
    if (serialPortFD < 0)
        return false;

    // Platform-specific serial ping might be added here.
    return true;
}

bool LeTelescopeFFFPHardwareAdapter::getFirmwareVersion(char *version)
{
    if (!version)
        return false;

    if (firmwareVersion.empty())
    {
        if (!fetchFirmwareVersionFromDevice())
            return false;
    }

    std::snprintf(version, MAXRBUF, "%s", firmwareVersion.c_str());

    return true;
}

bool LeTelescopeFFFPHardwareAdapter::fetchFirmwareVersionFromDevice()
{
    if (serialPortFD < 0)
        return false;

    char request[MAXRBUF] = {0};
    char response[MAXRBUF] = {0};
    std::snprintf(request, MAXRBUF, "COMMAND:VERSION\n");

    if (!sendCommand(request, response, SERIAL_TIMEOUT_SEC, true))
        return false;

    // By protocol, response after parsing must contain version text.
    if (response[0] == '\0')
        return false;

    firmwareVersion = response;
    return true;
}

bool LeTelescopeFFFPHardwareAdapter::fetchFirmwareMaxBrightnessFromDevice()
{
    if (serialPortFD < 0)
        return false;

    char request[MAXRBUF] = {0};
    char response[MAXRBUF] = {0};
    std::snprintf(request, MAXRBUF, "COMMAND:MAX_BRIGHTNESS\n");

    if (!sendCommand(request, response, SERIAL_TIMEOUT_SEC, true))
        return false;

    // Numeric payload expected
    char *endptr = nullptr;
    long value = strtol(response, &endptr, 10);
    if (endptr == response || *endptr != '\0' || value < 0)
        return false;

    maxBrightness = static_cast<int>(value);
    return true;
}

bool LeTelescopeFFFPHardwareAdapter::getBrightness(int *brightness)
{
    if (!brightness)
        return false;

    if (serialPortFD < 0)
        return false;

    char request[MAXRBUF] = {0};
    char response[MAXRBUF] = {0};
    std::snprintf(request, MAXRBUF, "COMMAND:BRIGHTNESS_GET\n");

    if (!sendCommand(request, response, SERIAL_TIMEOUT_SEC, true))
        return false;

    // Expect numeric payload
    char *endptr = nullptr;
    long value = strtol(response, &endptr, 10);
    if (endptr == response || *endptr != '\0' || value < 0)
        return false;

    if (value > maxBrightness)
        return false;

    *brightness = static_cast<int>(value);
    return true;
}

bool LeTelescopeFFFPHardwareAdapter::getMaxBrightness(int *brightness)
{
    if (!brightness)
        return false;

    if(maxBrightness < 0)
    {
        return false;
    }

    *brightness = maxBrightness;
    return true;
}

bool LeTelescopeFFFPHardwareAdapter::setBrightness(int value)
{
    if (value < 0 || value > maxBrightness || serialPortFD < 0)
        return false;

    char request[MAXRBUF] = {0};
    char response[MAXRBUF] = {0};
    std::snprintf(request, MAXRBUF, "COMMAND:BRIGHTNESS_SET@%d\n", value);

    if (!sendCommand(request, response, SERIAL_TIMEOUT_SEC, true))
        return false;

    // After parsing, response should contain the brightness value set on the panel
    char *endptr = nullptr;
    long setValue = strtol(response, &endptr, 10);
    if (endptr == response || *endptr != '\0' || setValue < 0 || setValue > maxBrightness || setValue != value)
        return false;

    return true;
}

bool LeTelescopeFFFPHardwareAdapter::lightOn()
{
    return setBrightness(maxBrightness);
}

bool LeTelescopeFFFPHardwareAdapter::lightOff()
{
    return setBrightness(0);
}

bool LeTelescopeFFFPHardwareAdapter::openCover()
{
    if (serialPortFD < 0)
        return false;

    char request[MAXRBUF] = {0};
    char response[MAXRBUF] = {0};
    std::snprintf(request, MAXRBUF, "COMMAND:COVER_OPEN\n");

    if (!sendCommand(request, response, SERIAL_TIMEOUT_SEC, true))
        return false;

    return (strcmp(response, "OK") == 0);
}

bool LeTelescopeFFFPHardwareAdapter::closeCover()
{
    if (serialPortFD < 0)
        return false;

    char request[MAXRBUF] = {0};
    char response[MAXRBUF] = {0};
    std::snprintf(request, MAXRBUF, "COMMAND:COVER_CLOSE\n");

    if (!sendCommand(request, response, SERIAL_TIMEOUT_SEC, true))
        return false;

    return (strcmp(response, "OK") == 0);
}

bool LeTelescopeFFFPHardwareAdapter::getCoverStatus(PanelCoverStatus *coverStatus)
{
    if (!coverStatus || serialPortFD < 0)
        return false;

    char request[MAXRBUF] = {0};
    char response[MAXRBUF] = {0};
    std::snprintf(request, MAXRBUF, "COMMAND:COVER_GET_STATE\n");

    if (!sendCommand(request, response, SERIAL_TIMEOUT_SEC, true))
        return false;

    // Parse the response and map to enum
    if (std::strcmp(response, "OPEN") == 0)
        *coverStatus = COVER_STATUS_OPEN;
    else if (std::strcmp(response, "OPENING") == 0)
        *coverStatus = COVER_STATUS_OPENING;
    else if (std::strcmp(response, "CLOSED") == 0)
        *coverStatus = COVER_STATUS_CLOSED;
    else if (std::strcmp(response, "CLOSING") == 0)
        *coverStatus = COVER_STATUS_CLOSING;
    else
        *coverStatus = COVER_STATUS_UNKNOWN;

    return true;
}

bool LeTelescopeFFFPHardwareAdapter::init(int portFD)
{
    serialPortFD = portFD;

    if (!ping())
        return false;

    if (!fetchFirmwareMaxBrightnessFromDevice())
        return false;

    if (!fetchFirmwareVersionFromDevice())
        return false;

    return true;
}

bool LeTelescopeFFFPHardwareAdapter::sendCommand(const char *command, char *response, int timeout, bool log)
{
    if (serialPortFD < 0 || command == nullptr || response == nullptr)
        return false;

    tcflush(serialPortFD, TCIOFLUSH);

    int nbytes_written = 0;
    int rc = tty_write_string(serialPortFD, command, &nbytes_written);
    if (rc != TTY_OK)
    {
        if (log)
        {
            char errstr[MAXRBUF] = {0};
            tty_error_msg(rc, errstr, MAXRBUF);
            // `errstr` contains human-readable TTY failure message.
            // We intentionally do not call printf/LOGF here as per request.
        }
        return false;
    }

    int nbytes_read = 0;
    rc = tty_nread_section(serialPortFD, response, MAXRBUF, '\n', timeout, &nbytes_read);
    if (rc != TTY_OK)
    {
        if (log)
        {
            char errstr[MAXRBUF] = {0};
            tty_error_msg(rc, errstr, MAXRBUF);
            // `errstr` contains human-readable TTY failure message.
            // No printf/LOGF in this scope.
        }
        return false;
    }

    if (nbytes_read > 0)
    {
        while (nbytes_read > 0 && (response[nbytes_read - 1] == '\n' || response[nbytes_read - 1] == '\r'))
        {
            response[nbytes_read - 1] = '\0';
            --nbytes_read;
        }
    }
    else
    {
        response[0] = '\0';
    }

    tcflush(serialPortFD, TCIOFLUSH);

    if (response[0] == '\0')
        return false;

    // Protocol parsing: ERROR:MSG or RESULT:COMMAND@VALUE
    const char *errorPrefix = "ERROR:";
    const char *resultPrefix = "RESULT:";

    if (strncmp(response, errorPrefix, strlen(errorPrefix)) == 0)
    {
        // response left intact for caller logging
        return false;
    }

    if (strncmp(response, resultPrefix, strlen(resultPrefix)) == 0)
    {
        char *token = response + strlen(resultPrefix);
        char *at = strchr(token, '@');
        if (!at || at[1] == '\0')
        {
            // malformed response, caller can inspect raw data
            return false;
        }

        // Replace response contents with payload only for caller
        char *value = at + 1;
        memmove(response, value, strlen(value) + 1);
        return true;
    }

    // Unrecognized response format: should be ERROR:... or RESULT:...@...
    return false;
}

// --- Simulation adapter implementation ---
SimulationHardwareAdapter::SimulationHardwareAdapter()
    : currentBrightness(0), coverStatus(COVER_STATUS_CLOSED)
{
}

SimulationHardwareAdapter::~SimulationHardwareAdapter() = default;

bool SimulationHardwareAdapter::ping()
{
    // Always alive in simulation mode
    return true;
}

bool SimulationHardwareAdapter::getFirmwareVersion(char *version)
{
    if (!version)
        return false;

    std::snprintf(version, MAXRBUF, "LeTelescopeFFFP Simulator v1.0");
    return true;
}

bool SimulationHardwareAdapter::getBrightness(int *brightness)
{
    if (!brightness)
        return false;

    *brightness = currentBrightness;
    return true;
}

bool SimulationHardwareAdapter::getMaxBrightness(int *brightness)
{
    if (!brightness)
        return false;

    *brightness = SIM_MAX_BRIGHTNESS;
    return true;
}

bool SimulationHardwareAdapter::setBrightness(int value)
{
    if (value < 0 || value > SIM_MAX_BRIGHTNESS)
        return false;

    currentBrightness = value;
    return true;
}

bool SimulationHardwareAdapter::lightOn()
{
    currentBrightness = SIM_MAX_BRIGHTNESS;
    return true;
}

bool SimulationHardwareAdapter::lightOff()
{
    currentBrightness = 0;
    return true;
}

bool SimulationHardwareAdapter::openCover()
{
    coverStatus = COVER_STATUS_OPENING;
    coverStatus = COVER_STATUS_OPEN;
    return true;
}

bool SimulationHardwareAdapter::closeCover()
{
    coverStatus = COVER_STATUS_CLOSING;
    coverStatus = COVER_STATUS_CLOSED;
    return true;
}

bool SimulationHardwareAdapter::getCoverStatus(PanelCoverStatus *status)
{
    if (!status)
        return false;

    *status = coverStatus;
    return true;
}

bool SimulationHardwareAdapter::init(int /*portFD*/)
{
    // No real communication needed in simulation mode.
    return true;
}
