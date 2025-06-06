﻿//
// ASCOM CoverCalibrator hardware class for LeTelescopeFFFPV1
//
// Copyright(C) 2025 - Present, Le Télescope - Ivry sur Seine - All Rights Reserved
// Licensed under the MIT License. See the accompanying LICENSE file for terms.
//
// Description:	 The base layout of this class has been generated using the Visual Studio Code ASCOM 6 driver template.
//
// The CoverCalibrator hardware class is responsible for the actual interaction with the flat panel Hardware/Firmware
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
// | ASCOM driver | --------------------->  | (firmware) |
// |              | < --------------------  |            |
//  --------------   RESULT:CMD_NAME @MSG    ------------
//                          or
//                 ERROR:ERR_MESSAGE @DETAILS
//
// The protocol, the implementation of both the driver and the firmware, the electronics and the 3D models
// are heavily inspired by the the work  of Dark Sky Geek (https://github.com/jlecomte/) especially 
// - https://github.com/jlecomte/ascom-flat-panel
// - https://github.com/jlecomte/ascom-wireless-flat-panel
// - https://github.com/jlecomte/ascom-telescope-cover-v2
//
// We delegate synchronisation to the SharedResources class that handles the shared serial client.
//
// This should prevent any race condition and dead locks. More involved patterns may be implemented but we
// prefer to keep it simple stupid ragarding the fact that the load put by the softwares using this driver
// should be quite low. For the same reason we do not implement any cache mechanism. 
//
// Implements:	ASCOM CoverCalibrator interface version: 1
//
// 
// Authors:
//   - Florian Thibaud
//   - Florian Gautier		
//

using System;
using System.Collections;
using System.Threading;
using System.Windows.Forms;
using ASCOM.DeviceInterface;
using ASCOM.LocalServer;
using ASCOM.Utilities;

namespace ASCOM.LeTelescopeFFFPV1.CoverCalibrator
{
    /// <summary>
    /// ASCOM CoverCalibrator hardware class for LeTelescopeFFFPV1.
    /// </summary>
    [HardwareClass()] // Class attribute flag this as a device hardware class that needs to be disposed by the local server when it exits.
    internal static class CoverCalibratorHardware
    {
        // Constants used for Profile persistence
        internal const string comPortProfileName = "COM Port";
        internal const string comPortDefault = "COM1";
        internal const string traceStateProfileName = "Trace Level";
        internal const string traceStateDefault = "true";

        // Constants related to message structure as per our protocol
        // A Message is TYPE:MESSAGE
        // This driver will
        // - emit Command messages : COMMAND:CMD_NAME[@ARGS] where ARGS are optional and command dependent
        // - receive either
        //   + result messages : RESULT:CMD_NAME@RSLT
        //   + error message   : ERROR:ERR_NAME@DETAILS
        private const string TYPE_COMMAND_SEPARATOR = ":";
        private const string COMMAND_ARGS_SEPARATOR = "@";
        private const string COMMAND_TYPE = "COMMAND";
        private const string RESULT_TYPE = "RESULT";
        private const string EMPTY_ARGS = "";
        // Command Names
        private const string CMD_PING = "PING";
        private const string CMD_COVER_GET = "COVER_GET_STATE";
        private const string CMD_COVER_OPEN = "COVER_OPEN";
        private const string CMD_COVER_CLOSE = "COVER_CLOSE";
        private const string CMD_BRIGHTNESS_GET = "BRIGHTNESS_GET";
        private const string CMD_BRIGHTNESS_SET = "BRIGHTNESS_SET";
        private const string CMD_BRIGHTNESS_RESET = "BRIGHTNESS_RESET";
        // Expected restults
        private const string GENERIC_RSLT_OK = "OK";
        private const string PING_RSLT_PONG = "PONG";
        private const int MAX_BRIGHTNESS = 1023; // Maybe could be given by the hardware for better flexibility
        private const int MIN_BRIGNTESS = 0;


        private static string DriverProgId = ""; // ASCOM DeviceID (COM ProgID) for this driver, the value is set by the driver's class initialiser.
        private static string DriverDescription = ""; // The value is set by the driver's class initialiser.
        internal static string comPort; // COM port name (if required)
        private static bool runOnce = false; // Flag to enable "one-off" activities only to run once.
        internal static TraceLogger tl; // Local server's trace logger object for diagnostic log with information that you specify

        /// <summary>
        /// Initializes a new instance of the device Hardware class.
        /// </summary>
        static CoverCalibratorHardware()
        {
            try
            {
                // Create the hardware trace logger in the static initialiser.
                // All other initialisation should go in the InitialiseHardware method.
                tl = new TraceLogger("", "LeTelescopeFFFPV1.Hardware");

                // DriverProgId has to be set here because it used by ReadProfile to get the TraceState flag.
                DriverProgId = CoverCalibrator.DriverProgId; // Get this device's ProgID so that it can be used to read the Profile configuration values

                // ReadProfile has to go here before anything is written to the log because it loads the TraceLogger enable / disable state.
                ReadProfile(); // Read device configuration from the ASCOM Profile store, including the trace state

                LogMessage("CoverCalibratorHardware", $"Static initialiser completed.");
            }
            catch (Exception ex)
            {
                try { LogMessage("CoverCalibratorHardware", $"Initialisation exception: {ex}"); } catch { }
                MessageBox.Show($"{ex.Message}", "Exception creating ASCOM.LeTelescopeFFFPV1.CoverCalibrator", MessageBoxButtons.OK, MessageBoxIcon.Error);
                throw;
            }
        }

        /// <summary>
        /// Place device initialisation code here
        /// </summary>
        /// <remarks>Called every time a new instance of the driver is created.</remarks>
        internal static void InitialiseHardware()
        {
            // This method will be called every time a new ASCOM client loads your driver
            LogMessage("InitialiseHardware", $"Start.");

            // Make sure that "one off" activities are only undertaken once
            if (runOnce == false)
            {
                LogMessage("InitialiseHardware", $"Starting one-off initialisation.");

                DriverDescription = CoverCalibrator.DriverDescription; // Get this device's Chooser description

                LogMessage("InitialiseHardware", $"ProgID: {DriverProgId}, Description: {DriverDescription}");

                LogMessage("InitialiseHardware", "Completed basic initialisation");

                // Add your own "one off" device initialisation here e.g. validating existence of hardware and setting up communications

                LogMessage("InitialiseHardware", $"One-off initialisation complete.");
                runOnce = true; // Set the flag to ensure that this code is not run again
            }
        }

        // PUBLIC COM INTERFACE ICoverCalibratorV1 IMPLEMENTATION

        #region Common properties and methods.

        /// <summary>
        /// Displays the Setup Dialogue form.
        /// If the user clicks the OK button to dismiss the form, then
        /// the new settings are saved, otherwise the old values are reloaded.
        /// THIS IS THE ONLY PLACE WHERE SHOWING USER INTERFACE IS ALLOWED!
        /// </summary>
        public static void SetupDialog()
        {
            // Don't permit the setup dialogue if already connected
            if (IsConnected)
                MessageBox.Show("Already connected, just press OK");

            using (SetupDialogForm F = new SetupDialogForm(tl))
            {
                var result = F.ShowDialog();
                if (result == DialogResult.OK)
                {
                    WriteProfile(); // Persist device configuration values to the ASCOM Profile store
                }
            }
        }

        /// <summary>Returns the list of custom action names supported by this driver.</summary>
        /// <value>An ArrayList of strings (SafeArray collection) containing the names of supported actions.</value>
        public static ArrayList SupportedActions
        {
            get
            {
                LogMessage("SupportedActions Get", "Returning empty ArrayList");
                return new ArrayList();
            }
        }

        /// <summary>Invokes the specified device-specific custom action.</summary>
        /// <param name="ActionName">A well known name agreed by interested parties that represents the action to be carried out.</param>
        /// <param name="ActionParameters">List of required parameters or an <see cref="String.Empty">Empty String</see> if none are required.</param>
        /// <returns>A string response. The meaning of returned strings is set by the driver author.
        /// <para>Suppose filter wheels start to appear with automatic wheel changers; new actions could be <c>QueryWheels</c> and <c>SelectWheel</c>. The former returning a formatted list
        /// of wheel names and the second taking a wheel name and making the change, returning appropriate values to indicate success or failure.</para>
        /// </returns>
        public static string Action(string actionName, string actionParameters)
        {
            LogMessage("Action", $"Action {actionName}, parameters {actionParameters} is not implemented");
            throw new ActionNotImplementedException("Action " + actionName + " is not implemented by this driver");
        }

        /// <summary>
        /// Transmits an arbitrary string to the device and does not wait for a response.
        /// Optionally, protocol framing characters may be added to the string before transmission.
        /// </summary>
        /// <param name="Command">The literal command string to be transmitted.</param>
        /// <param name="Raw">
        /// if set to <c>true</c> the string is transmitted 'as-is'.
        /// If set to <c>false</c> then protocol framing characters may be added prior to transmission.
        /// </param>
        public static void CommandBlind(string command, bool raw)
        {
            CheckConnected("CommandBlind");
            // TODO The optional CommandBlind method should either be implemented OR throw a MethodNotImplementedException
            // If implemented, CommandBlind must send the supplied command to the mount and return immediately without waiting for a response

            throw new MethodNotImplementedException($"CommandBlind - Command:{command}, Raw: {raw}.");
        }

        /// <summary>
        /// Transmits an arbitrary string to the device and waits for a boolean response.
        /// Optionally, protocol framing characters may be added to the string before transmission.
        /// </summary>
        /// <param name="Command">The literal command string to be transmitted.</param>
        /// <param name="Raw">
        /// if set to <c>true</c> the string is transmitted 'as-is'.
        /// If set to <c>false</c> then protocol framing characters may be added prior to transmission.
        /// </param>
        /// <returns>
        /// Returns the interpreted boolean response received from the device.
        /// </returns>
        public static bool CommandBool(string command, bool raw)
        {
            CheckConnected("CommandBool");
            // TODO The optional CommandBool method should either be implemented OR throw a MethodNotImplementedException
            // If implemented, CommandBool must send the supplied command to the mount, wait for a response and parse this to return a True or False value

            throw new MethodNotImplementedException($"CommandBool - Command:{command}, Raw: {raw}.");
        }

        /// <summary>
        /// Transmits an arbitrary string to the device and waits for a string response.
        /// Optionally, protocol framing characters may be added to the string before transmission.
        /// </summary>
        /// <param name="Command">The literal command string to be transmitted.</param>
        /// <param name="Raw">
        /// if set to <c>true</c> the string is transmitted 'as-is'.
        /// If set to <c>false</c> then protocol framing characters may be added prior to transmission.
        /// </param>
        /// <returns>
        /// Returns the string response received from the device.
        /// </returns>
        public static string CommandString(string command, bool raw)
        {
            CheckConnected("CommandString");
            // TODO The optional CommandString method should either be implemented OR throw a MethodNotImplementedException
            // If implemented, CommandString must send the supplied command to the mount and wait for a response before returning this to the client

            throw new MethodNotImplementedException($"CommandString - Command:{command}, Raw: {raw}.");
        }

        /// <summary>
        /// Deterministically release both managed and unmanaged resources that are used by this class.
        /// </summary>
        /// <remarks>
        /// TODO: Release any managed or unmanaged resources that are used in this class.
        /// 
        /// Do not call this method from the Dispose method in your driver class.
        ///
        /// This is because this hardware class is decorated with the <see cref="HardwareClassAttribute"/> attribute and this Dispose() method will be called 
        /// automatically by the  local server executable when it is irretrievably shutting down. This gives you the opportunity to release managed and unmanaged 
        /// resources in a timely fashion and avoid any time delay between local server close down and garbage collection by the .NET runtime.
        ///
        /// For the same reason, do not call the SharedResources.Dispose() method from this method. Any resources used in the static shared resources class
        /// itself should be released in the SharedResources.Dispose() method as usual. The SharedResources.Dispose() method will be called automatically 
        /// by the local server just before it shuts down.
        /// 
        /// </remarks>
        public static void Dispose()
        {
            try { LogMessage("Dispose", $"Disposing of assets and closing down."); } catch { }

            try
            {
                // Clean up the trace logger and utility objects
                tl.Enabled = false;
                tl.Dispose();
                tl = null;
            }
            catch { }
        }

        /// <summary>
        /// Set True to connect to the device hardware. Set False to disconnect from the device hardware.
        /// You can also read the property to check whether it is connected. This reports the current hardware state.
        /// </summary>
        /// <value><c>true</c> if connected to the hardware; otherwise, <c>false</c>.</value>
        public static bool Connected
        {
            get
            {
                LogMessage("Connected", $"Get {IsConnected}");
                return IsConnected;
            }
            set
            {
                LogMessage("Connected", $"Set {value}");
                if (value == IsConnected)
                {
                    return;
                }

                if (value)
                {
                    LogMessage("Connected Set", $"Connecting to device on port {comPort}");

                    try
                    {
                        SharedResources.SerialPortName = comPort;
                        SharedResources.SerialConnected = true;

                        var ping_command_msg = CommandBuilder(CMD_PING);
                        var ping_expected_result_msg = ExpectedResultPrefixBuilder(CMD_PING) + PING_RSLT_PONG;
                        SharedResources.ValidateDevice(ping_command_msg,ping_expected_result_msg);

                    }
                    catch (Exception e)
                    {
                        LogMessage("Connected Set", $"Connection to port {comPort} failed: {e.Message}");  
                        throw new DriverException($"Connection to port {comPort} failed", e);
                    }

                    LogMessage("Connected Set", $"Connected to device on port {comPort}");

                }
                else
                {
                    LogMessage("Connected Set", $"Disconnecting from port {comPort}");
                    SharedResources.SerialConnected = false;
                }
            }
        }

        /// <summary>
        /// Returns a description of the device, such as manufacturer and model number. Any ASCII characters may be used.
        /// </summary>
        /// <value>The description.</value>
        public static string Description
        {
            get
            {
                LogMessage("Description Get", DriverDescription);
                return DriverDescription;
            }
        }

        /// <summary>
        /// Descriptive and version information about this ASCOM driver.
        /// </summary>
        public static string DriverInfo
        {
            get
            {
                Version version = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
                string driverInfo = $"Le Telescope FFFPV1 ASCOM CoverCalibrator driver . Version: {version.Major}.{version.Minor}";
                LogMessage("DriverInfo Get", driverInfo);
                return driverInfo;
            }
        }

        /// <summary>
        /// A string containing only the major and minor version of the driver formatted as 'm.n'.
        /// </summary>
        public static string DriverVersion
        {
            get
            {
                Version version = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
                string driverVersion = $"{version.Major}.{version.Minor}";
                LogMessage("DriverVersion Get", driverVersion);
                return driverVersion;
            }
        }

        /// <summary>
        /// The interface version number that this device supports.
        /// </summary>
        public static short InterfaceVersion
        {
            // set by the driver wizard
            get
            {
                LogMessage("InterfaceVersion Get", "1");
                return Convert.ToInt16("1");
            }
        }

        /// <summary>
        /// The short name of the driver, for display purposes
        /// </summary>
        public static string Name
        {
            get
            {
                string name = "Le Telescope FFFPV1 ASCOM driver";
                LogMessage("Name Get", name);
                return name;
            }
        }

        #endregion

        #region ICoverCalibrator Implementation


        /// <summary>
        /// Returns the state of the device cover, if present, otherwise returns "NotPresent"
        /// </summary>
        internal static CoverStatus CoverState
        {
            get
            {
                var identifier = "CoverState Get";
                string response = SendCommand(identifier: identifier, command: CMD_COVER_GET);
                LogMessage(identifier, $"Cover is {response}");
                switch (response)
                {
                    case "OPEN":
                        return CoverStatus.Open;
                    case "OPENING":
                        return CoverStatus.Moving;
                    case "CLOSING":
                        return CoverStatus.Moving;
                    case "CLOSED":
                        return CoverStatus.Closed;
                    default:
                        LogMessage(identifier, "{response}: Unknown cover status");
                        return CoverStatus.Unknown;
                }
            }
        }

        /// <summary>
        /// Initiates cover opening if a cover is present
        /// </summary>
        internal static void OpenCover()
        {
            var identifier = "OpenCover";

            string response = SendCommand(identifier: identifier, command: CMD_COVER_OPEN);


            LogMessage(identifier, $"{response}");
            if (response != GENERIC_RSLT_OK)
            {
                LogMessage(identifier, $"Invalid response {response} from device. Hardware may be in a weird state");
                throw new InvalidOperationException($"Invalid response {response} from device. Hardware may be in a weird state");

            }
        }

        /// <summary>
        /// Initiates cover closing if a cover is present
        /// </summary>
        internal static void CloseCover()
        {
            var identifier = "CloseCover";

            string response = SendCommand(identifier: identifier, command: CMD_COVER_CLOSE);

            LogMessage(identifier, $"{response}");

            if (response != GENERIC_RSLT_OK)
            {
                LogMessage(identifier, $"Invalid response {response} from device. Hardware may be in a weird state");
                throw new InvalidOperationException($"Invalid response {response} from device. Hardware may be in a weird state");
            }
        }

        /// <summary>
        /// Stops any cover movement that may be in progress if a cover is present and cover movement can be interrupted.
        /// </summary>
        internal static void HaltCover()
        {
            var identifier = "HaltCover";
            CheckConnected($"{identifier}: Flat panel not connected");
            LogMessage(identifier, "Not implemented");
            throw new MethodNotImplementedException(identifier);
        }

        /// <summary>
        /// Returns the state of the calibration device, if present, otherwise returns "NotPresent"
        /// </summary>
        internal static CalibratorStatus CalibratorState
        {
            get
            {
                var identifier = "CalibratorState Get";

                CheckConnected($"{identifier}: Flat panel not connected");
                LogMessage(identifier, "Calibrator is ready");
                return CalibratorStatus.Ready;
            }
        }

        /// <summary>
        /// Returns the current calibrator brightness in the range 0 (completely off) to <see cref="MaxBrightness"/> (fully on)
        /// </summary>
        internal static int Brightness
        {
            get
            {
                var identifier = "Brightness Get";

                string response = SendCommand(identifier: identifier, command: CMD_BRIGHTNESS_GET);

                LogMessage(identifier, $"{response}");

                int brightness = -1;
                try
                {
                    brightness = int.Parse(response);
                }
                catch
                {
                    LogMessage(identifier, $"Invalid response {response} from device. Hardware may be in a weird state");
                    throw new InvalidOperationException($"Invalid response {response} from device. Hardware may be in a weird state");
                }

                if (brightness < MIN_BRIGNTESS || brightness > MAX_BRIGHTNESS)
                {
                    LogMessage(identifier, $"Invalid response {response} from device. Hardware may be in a weird state");
                    throw new InvalidOperationException($"Invalid response {response} from device. Hardware may be in a weird state");
                }

                return brightness;
            }
        }

        /// <summary>
        /// The Brightness value that makes the calibrator deliver its maximum illumination.
        /// </summary>
        internal static int MaxBrightness
        {
            get
            {
                LogMessage("MaxBrightness Get", $"{MAX_BRIGHTNESS}");
                return MAX_BRIGHTNESS;
            }
        }

        /// <summary>
        /// Turns the calibrator on at the specified brightness if the device has calibration capability
        /// </summary>
        /// <param name="Brightness"></param>
        internal static void CalibratorOn(int Brightness)
        {
            var identifier = "CalibratorOn";

            if (Brightness < MIN_BRIGNTESS && Brightness > MAX_BRIGHTNESS)
            {
                LogMessage(identifier, $"Invalid brightness {Brightness}. Should be an int ranging from {MIN_BRIGNTESS} to {MAX_BRIGHTNESS}");
                throw new InvalidOperationException($"Invalid brightness {Brightness}. Should be an int ranging from {MIN_BRIGNTESS} to {MAX_BRIGHTNESS}");

            }

            string response = SendCommand(identifier: identifier, command: CMD_BRIGHTNESS_SET, args: Brightness.ToString());

            if (response != Brightness.ToString())
            {
                LogMessage(identifier, $"Invalid response {response} from device. Hardware may be in a weird state");
                throw new InvalidOperationException($"Invalid response {response} from device. Hardware may be in a weird state");
            }

            LogMessage(identifier, $"On:{response}");
        }

        /// <summary>
        /// Turns the calibrator off if the device has calibration capability
        /// </summary>
        internal static void CalibratorOff()
        {
            var identifier = "CalibratorOff";

            string response = SendCommand(identifier: identifier, command: CMD_BRIGHTNESS_RESET);

            if (response.Trim() != "0")
            {
                LogMessage(identifier, $"Invalid response {response} from device. Hardware may be in a weird state");
                throw new InvalidOperationException($"Invalid response {response} from device. Hardware may be in a weird state");
            }

            LogMessage(identifier, $"Off");
        }

        #endregion

        #region Private properties and methods

        /// <summary>
        /// Send a command via Serial and wait for the firmware response
        /// </summary>
        /// <param name="identifier">Where this command originates from</param>
        /// <param name="command">the command</param>
        /// <param name="args">the argument of the command</param>
        /// <returns>the parsed result of the firware response </returns>
        /// <exception>May throw expection if something went wrong</exception>
        private static string SendCommand(string identifier, string command, string args = EMPTY_ARGS)
        {
            CheckConnected($"{identifier}: Flat panel not connected");

            string message = CommandBuilder(command, args);

            // Rethrow as is the error if it happens no added value to add comments here
            string raw_cmd_result = SendMessage(identifier, message);
            string expected_prefix = ExpectedResultPrefixBuilder(command);
            if (!raw_cmd_result.StartsWith(expected_prefix))
            {
                LogMessage(identifier, $"Command '{command}' with args '{args}' failed to : {raw_cmd_result}");
                throw new InvalidOperationException($"Command '{command}' with args '{args}' failed: {raw_cmd_result}");
            }

            string cmd_result = raw_cmd_result.Replace(expected_prefix, string.Empty);
            LogMessage(identifier, $"Command '{command}' with args '{args}' returned : {cmd_result}");

            return cmd_result.Trim();
        }

        private static string ExpectedResultPrefixBuilder(string command)
        {
            return $"{RESULT_TYPE}{TYPE_COMMAND_SEPARATOR}{command}{COMMAND_ARGS_SEPARATOR}";
        }

        private static string CommandBuilder(string command, string args = EMPTY_ARGS)
        {
            string message = $"{COMMAND_TYPE}{TYPE_COMMAND_SEPARATOR}{command}";
            message = string.IsNullOrWhiteSpace(args) ? message : $"{message}{COMMAND_ARGS_SEPARATOR}{args}";
            return message;
        }

        /// <summary>
        /// Sends a message and waits for the response
        /// </summary>
        /// <param name="serial_client">the client used</param>
        /// <param name="identifier">Where this message originates from</param>
        /// <param name="message">the message</param>
        /// <returns>the raw response message</returns>
        /// <exception cref="NotConnectedException"></exception>
        private static string SendMessage(string identifier, string message)
        {
            try
            {
                LogMessage(identifier, $"Transmitting: {message}");
                var raw_response = SharedResources.SendMessage(message);
                LogMessage(identifier, $"Received: {raw_response}");
                return raw_response;
            }
            catch (Exception e)
            {
                LogMessage(identifier, $"Connection issue with hardware: {e}");
                throw new NotConnectedException($"Connection issue with hardware: {e.Message}", e);
            }
        }


        // Useful methods that can be used as required to help with driver development

        /// <summary>
        /// Returns true if there is a valid connection to the driver hardware
        /// </summary>
        private static bool IsConnected
        {
            // DO NOT Synchronize this getter as it is used in syncrhonized function and we may end in 
            // a deadlock situation.
            get
            {
                return SharedResources.SerialConnected;
            }
        }

        /// <summary>
        /// Use this function to throw an exception if we aren't connected to the hardware
        /// </summary>
        /// <param name="message"></param>
        private static void CheckConnected(string message)
        {
            if (!IsConnected)
            {
                throw new NotConnectedException(message);
            }
        }

        /// <summary>
        /// Read the device configuration from the ASCOM Profile store
        /// </summary>
        internal static void ReadProfile()
        {
            using (Profile driverProfile = new Profile())
            {
                driverProfile.DeviceType = "CoverCalibrator";
                tl.Enabled = Convert.ToBoolean(driverProfile.GetValue(DriverProgId, traceStateProfileName, string.Empty, traceStateDefault));
                comPort = driverProfile.GetValue(DriverProgId, comPortProfileName, string.Empty, comPortDefault);
            }
        }

        private static void validateDevice(String identifier) {
            LogMessage(identifier, $"Validate device using ping command");
            string ping_rslt = SendCommand(identifier: identifier, command: CMD_PING);
            if (ping_rslt != PING_RSLT_PONG) 
            {
                LogMessage(identifier, $"Incorrect devive: Ping answer: actual {ping_rslt} - expected {PING_RSLT_PONG}");
                throw new DriverException($"Incorrect devive: Ping answer: actual {ping_rslt} - expected {PING_RSLT_PONG}");
            }
            LogMessage(identifier, $"Device OK");
        }

        /// <summary>
        /// Write the device configuration to the  ASCOM  Profile store
        /// </summary>
        internal static void WriteProfile()
        {
            using (Profile driverProfile = new Profile())
            {
                driverProfile.DeviceType = "CoverCalibrator";
                driverProfile.WriteValue(DriverProgId, traceStateProfileName, tl.Enabled.ToString());
                driverProfile.WriteValue(DriverProgId, comPortProfileName, comPort.ToString());
            }
        }

        /// <summary>
        /// Log helper function that takes identifier and message strings
        /// </summary>
        /// <param name="identifier"></param>
        /// <param name="message"></param>
        internal static void LogMessage(string identifier, string message)
        {
            tl.LogMessageCrLf(identifier, message);
        }

        /// <summary>
        /// Log helper function that takes formatted strings and arguments
        /// </summary>
        /// <param name="identifier"></param>
        /// <param name="message"></param>
        /// <param name="args"></param>
        internal static void LogMessage(string identifier, string message, params object[] args)
        {
            var msg = string.Format(message, args);
            LogMessage(identifier, msg);
        }
        #endregion
    }
}

