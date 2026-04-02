//
// Hardware adapter definition for the LeTelescopeFFFP Flat Panel. 
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
#pragma once

#include <cstdint>

enum PanelCoverStatus
{
    COVER_STATUS_UNKNOWN = 0,
    COVER_STATUS_OPEN = 1,
    COVER_STATUS_OPENING = 2,
    COVER_STATUS_CLOSED = 3,
    COVER_STATUS_CLOSING = 4,
};

#define SERIAL_TIMEOUT_SEC 10
#define PANEL_MIN_BRIGHTNESS 0

/**
 * @brief Abstract hardware adapter interface for Le Telescope FFFP Flatpanel firmware
 *
 * This interface abstracts the differences between simulation and real hardware.
 * It defines the common set of operations that the driver will use to interact with the flat panel, regardless of the underlying implementation.
 */
class HardwareAdapter
{
    public:
        virtual ~HardwareAdapter() = default;

        // Device detection and identification

        /**
         * @brief Ping the device to check if it responds to this adapter's protocol
         * @return true if device responds correctly, false otherwise
         */
        virtual bool ping() = 0;


        /**
         * @brief Get the firmware version from the device
         * @param version pointer to store the firmware version
         * @return true if successful, false otherwise
         */
        virtual bool getFirmwareVersion(char *version) = 0;

        /**
         * @brief Get the maximum allowed brightness level
         * @param brightness pointer to store the maximum brightness value
         * @return true if successful, false otherwise
         */
        virtual bool getMaxBrightness(int *brightness) = 0;
        
        /**
         * @brief Set up communication parameters and initialize the hardware adapter. Should be called after establishing the serial connection and before any other operations.
         * @param portFD file descriptor for the serial port
         */
        virtual void init(int portFD) = 0;

        /**
         * @brief Get the current brightness level
         * @param brightness pointer to store the brightness value (0-MAX_BRIGHTNESS)
         * @return true if successful, false otherwise
         */
        virtual bool getBrightness(int *brightness) = 0;


        /**
         * @brief Set the brightness level
         * @param value brightness value (0-MAX_BRIGHTNESS)
         * @return true if successful, false otherwise
         */
        virtual bool setBrightness(int value) = 0;

        /**
         * @brief Turn the light on
         * @return true if successful, false otherwise
         */
        virtual bool lightOn() = 0;

        /**
         * @brief Turn the light off
         * @return true if successful, false otherwise
         */
        virtual bool lightOff() = 0;

        /**
         * @brief Open the dust cover
         * @return true if successful, false otherwise
         */
        virtual bool openCover() = 0;

        /**
         * @brief Close the dust cover
         * @return true if successful, false otherwise
         */
        virtual bool closeCover() = 0;

        /**
         * @brief Get the current device status
         * @param coverStatus pointer to store cover status
         */
        virtual bool getCoverStatus(PanelCoverStatus *coverStatus) = 0;

};


class LeTelescopeFFFPHardwareAdapter : public HardwareAdapter
{
    public:
        LeTelescopeFFFPHardwareAdapter();
        ~LeTelescopeFFFPHardwareAdapter() override;

        bool ping() override;
        bool getFirmwareVersion(char *version) override;
        bool getBrightness(int *brightness) override;
        bool getMaxBrightness(int *brightness) override;
        bool setBrightness(int value) override;
        bool lightOn() override;
        bool lightOff() override;
        bool openCover() override;
        bool closeCover() override;
        bool getCoverStatus(PanelCoverStatus *coverStatus) override;
        void init(int portFD) override;

    private:
        int serialPortFD; // File descriptor for the serial port connection
        int maxBrightness; // Maximum brightness level supported by the device

        /**
         * @brief Send a command to the device and receive response
         * @param command command string to send
         * @param response buffer to store response (can be nullptr if no response expected)
         * @param timeout timeout in seconds
         * @param log whether to log errors
         * @return true if successful, false otherwise
         */
        bool sendCommand(const char *command, char *response, int timeout = SERIAL_TIMEOUT_SEC, bool log = true);

};

class SimulationHardwareAdapter : public HardwareAdapter
{
    public:
        SimulationHardwareAdapter();
        ~SimulationHardwareAdapter() override;

        bool ping() override;
        bool getFirmwareVersion(char *version) override;
        bool getBrightness(int *brightness) override;
        bool getMaxBrightness(int *brightness) override;
        bool setBrightness(int value) override;
        bool lightOn() override;
        bool lightOff() override;
        bool openCover() override;
        bool closeCover() override;
        bool getCoverStatus(PanelCoverStatus *coverStatus) override;
        void init(int portFD) override;

    private:
        
        int currentBrightness; // Simulated current brightness level
        PanelCoverStatus coverStatus; // Simulated cover status
};