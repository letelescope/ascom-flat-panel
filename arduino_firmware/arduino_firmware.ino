/*
 * arduino_firmware.ino
 * Copyright (C) 2025 - Present, Le Télescope - Ivry sur Seine - All Rights Reserved
 * Licensed under the MIT License. See the accompanying LICENSE file for terms.
 *
 * Description: Arduino firmware for an ASCOM driven automated and motorised flat panel
 * 
 * Authors:
 * - Florian Thibaud
 * - Florian Gautier 
 *
 * Principle: In this scenario a flat panel is made of two things
 * - A light panel with variable brightness
 * - A motorized cover
 *
 * The firmware updates the state of the flat panel upon instructions of the ASCOM driver. The communication protocol 
 * is serial based. The serial connection is established via USB. Here is a a summary of the protocol 
 *  - Both the driver and the flat panel (and hence this firmware) exchange single line messages.
 *  - A message is structured as TYPE:MESSAGE, where TYPE is in "COMMAND, RESULT, ERROR" and MESSAGE is alaphanumerical with spaces and "@"
 *
 * This firware only handles incoming "COMMAND" messages. This kind of message is stuctured as 
 *  
 * COMMAND:NAME[@ARGS]
 *
 * where NAME is [A-Z_]+ and ARGS are optional and their nature may depend on the command. For instance for a COMMAND:BRIGHTNESS_SET@ARGS message the ARGS
 * are mandatory and should consists of a single "int"
 *
 * This firware reponse with 
 * - either a RESULT:CMD_NAME@VALUE  if operation succeeded
 * - or an ERROR:ERR_MESSAGE@DETAILS if anything went wrong
 *
 * In a nutshell
 *  --------------                           ------------
 * |              |                         |            |
 * |              | COMMAND:CMD_NAME[@ARGS] | Flat Panel |
 * | ASCOM driver | --------------------->  | (firmware) |
 * |              | <---------------------  |            |
 * ---------------  RESULT:CMD_NAME@VALUE    ------------
 *                          or
 *                 ERROR:ERR_MESSAGE@DETAILS
 *
 * Allowed commands are:
 * - COMMAND:PING
 * - COMMAND:INFO
 * - COMMAND:BRIGHTNESS_GET
 * - COMMAND:BRIGHTNESS_SET@{(int) DESIRED_VALUE}
 * - COMMAND:BRIGHTNESS_RESET
 * - COMMAND:COVER_GET
 * - COMMAND:COVER_OPEN
 * - COMMAND:COVER_CLOSE
 * - COMMAND:CALIBRATION_RUN
 * - COMMAND:CALIBRATION_GET
 *
 * The protocol, the implementation of both the ASCOM driver and this firmware, the electronics and the 3D models
 * are heavily inspired by the the work  of Dark Sky Geek (https://github.com/jlecomte/) especially 
 * - https://github.com/jlecomte/ascom-flat-panel
 * - https://github.com/jlecomte/ascom-wireless-flat-panel
 * - https://github.com/jlecomte/ascom-telescope-cover-v2
 * 
 */

#include <Servo.h>
#include <FlashStorage.h>

/************************************************
 *     Types, Objects, and data structures      *
 ***********************************************/

/*
 * Messages/Protocol related types 
 */

//Represents a command message payload right after parsing, before decoding.
typedef struct msg_cmd_payload {
  String name;
  String args;
};

//Defines the behavior of a command. Each command 
// - may perform an action that will modify the flat panel firware state
// - may be given args as a String. It is the responsability of the command to check the correctness of the given input
typedef void (*cmd_handler_ptr)(String);

//Main command data structure. A command has a name, and holds a pointer to a function that will perform the action. 
typedef struct command_t {
  const char *name;
  cmd_handler_ptr handle;
};

/*
 * Flat panel state related types/data structures 
 */

// Represents the state of the flat panel cover. The cover is either open, opening, closing or closed. 
typedef enum {
  OPENING,
  OPEN,
  CLOSING,
  CLOSED,
} cover_state_t;

// Represents the state of the flat panel servo configuration. 
//
// This configuration will be stored/retreived from flash memory. Hence de magic number, to check if the retrieved
// configuration is garbage or actual configuraton that was correctly stored previously.
typedef struct {
  // Magic
  unsigned int magic_number;
  double slope;
  double intercept;
} servo_cal_state_t;

// Represents the panel(led and cover) state.
// Could ben restructured but this would bring layers of inderiction and less readability
typedef struct {
  uint32_t brightness;
  int servo_position;
  unsigned long last_step_time;
  cover_state_t cover;
  servo_cal_state_t calibration;
} panel_state_t;


/************************************************
 *                  Constants                   *
 ***********************************************/

/*
 * Messages parser related constants
 */
constexpr auto MESSAGE_END_DELIMITER = "\n";
constexpr auto TYPE_COMMAND_SEPARATOR = ":";
constexpr auto COMMAND_ARGS_SEPARATOR = "@";

/*
 * Available commands (names), custom results, and errors 
 */
constexpr auto COMMAND_MSG_TYPE = "COMMAND";
constexpr auto RESULT_MSG_TYPE = "RESULT";
constexpr auto ERROR_MSG_TYPE = "ERROR";

constexpr auto COMMAND_PING = "PING";
constexpr auto RESULT_PING = "PONG";

constexpr auto COMMAND_INFO = "INFO";
constexpr auto RESULT_INFO = "Le Télescope - Ivry sur Seine - Flat Panel Firmware v1.0";

constexpr auto COMMAND_BRIGHTNESS_GET = "BRIGHTNESS_GET";
constexpr auto COMMAND_BRIGHTNESS_SET = "BRIGHTNESS_SET";
constexpr auto COMMAND_BRIGHTNESS_RESET = "BRIGHTNESS_RESET";

constexpr auto COMMAND_COVER_GET_STATE = "COVER_GET_STATE";
constexpr auto RESULT_COVER_GET_STATE_OPEN = "OPEN";
constexpr auto RESULT_COVER_GET_STATE_OPENING = "OPENING";
constexpr auto RESULT_COVER_GET_STATE_CLOSED = "CLOSED";
constexpr auto RESULT_COVER_GET_STATE_CLOSING = "CLOSING";

constexpr auto COMMAND_COVER_OPEN = "COVER_OPEN";
constexpr auto RESULT_COVER_OPEN = "OK";
constexpr auto ERROR_COVER_OPEN = "COVER_OPEN_IMPOSSIBLE@Cover is not closed nor closing.";
constexpr auto COMMAND_COVER_CLOSE = "COVER_CLOSE";
constexpr auto RESULT_COVER_CLOSE = "OK";
constexpr auto ERROR_COVER_CLOSE = "COVER_CLOSE_IMPOSSIBLE@Cover is not open nor opening.";
constexpr auto COMMAND_COVER_CALIBRATION_RUN = "COVER_CALIBRATION_RUN";
constexpr auto RESULT_COVER_CALIBRATION_RUN = "0K";
constexpr auto COMMAND_COVER_CALIBRATION_GET = "COVER_CALIBRATION_GET";

constexpr auto COMMAND_UNKNOWN = "UNKNOWN";

constexpr auto ERROR_NOT_IMPLEMENTED = "UNSUPPORTED_COMMAND@Not implemented";
constexpr auto ERROR_INVALID_INCOMING_MESSAGE = "INVALID_INCOMING_MESSAGE@Allowed messages are TYPE:MESSAGE:[@PARAMETER]";
constexpr auto ERROR_INVALID_INCOMING_MESSAGE_TYPE = "INVALID_INCOMING_MESSAGE_TYPE@Allowed types COMMAND";
constexpr auto ERROR_INVALID_COMMAND = "INVALID_COMMAND@Allowed commands PING, INFO, BRIGHTNESS_GET, BRIGHTNESS_SET, BRIGHTNESS_RESET";
constexpr auto ERROR_WANTED_BRIGHTNESS_MSG_START = "INVALID_BRIGHTNESS@Wanted brightness {";
constexpr auto ERROR_WANTED_BRIGHTNESS_NAN_MSG_END = "} is not a number.";
constexpr auto ERROR_WANTED_BRIGHTNESS_NEGATIVE_MSG_END = "} is negative.";
constexpr auto ERROR_WANTED_BRIGHTNESS_TOO_BIG_MSG_END = "} is bigger than max allowed value 1023";
constexpr auto ERROR_SERVO_NOT_CALIBRATED = "SERVO_NO_CALIBRATED@Run command COVER_CALIBRATION_RUN first";

#define NB_COMMANDS 10
//Keeps the record of allowed/known commands
constexpr command_t allowed_cmds[NB_COMMANDS] = {{ COMMAND_PING, &cmd_ping },
                                      { COMMAND_INFO, &cmd_info },
                                      { COMMAND_BRIGHTNESS_GET, &cmd_brigthness_get },
                                      { COMMAND_BRIGHTNESS_SET, &cmd_brightness_set },
                                      { COMMAND_BRIGHTNESS_RESET, &cmd_brightness_reset },
                                      { COMMAND_COVER_GET_STATE, &cmd_cover_get_state },
                                      { COMMAND_COVER_OPEN, &cmd_cover_open },
                                      { COMMAND_COVER_CLOSE, &cmd_cover_close },
                                      { COMMAND_COVER_CALIBRATION_RUN, &cmd_cover_calibration_run },
                                      { COMMAND_COVER_CALIBRATION_GET, &cmd_cover_calibration_get }};

/*
 * Light panel related constants
 */
constexpr uint32_t MIN_BRIGHTNESS = 0;
constexpr uint32_t MAX_BRIGHTNESS = 1023;
constexpr uint32_t PWM_FREQ = 20000;

/*
 * Pins assignment. Must be set according to the exact actual wiring
 */
// There is conflict yet between LEDSTRIP pin and SERVO_FEEDBACK
// High frequency PWM seems to work on PIN 8 according to Julien Lecomte (at least not on pin 7)
// Maybe we should test if high freq works on pin 6 in order to have a spatial sepation of pins between leds and servo
// Or we should set the servo pins to 4,5,6 and let led pin to 8 still in order to have a spatial sepation of pins between leds and servo
constexpr unsigned int LEDSTRIP_PIN = 8;

constexpr unsigned int SERVO_SWITCH_PIN = 7;
constexpr unsigned int SERVO_FEEDBACK_PIN = 8;
constexpr unsigned int SERVO_CONTROL_PIN = 9;

/*
 * Misc.
 */
// Value used to determine whether the NVM (Non-Volatile Memory) was written, or we are just reading garbage...
constexpr unsigned int NVM_MAGIC_NUMBER = 0x12345678;

// How long do we wait between each step (loop) in order to achieve the desired speed?
constexpr unsigned long STEP_DELAY_MICROSEC = 30L * 1000; // 30 msec

/************************************************
 *             Global/State variables           *
 ***********************************************/

// Holds the panel(led and cover) state.
panel_state_t panel;

// Client used to interact with the servo motor.
Servo servo;

// Defines "nvm_store", the actual Flash sotrage where the calibration data will be stored/retrieved
FlashStorage(nvm_store, servo_cal_state_t);

/************************************************
 *              Arduino entrypoints             *
 ***********************************************/

/*
 * Called once at boot, used to setup the state. In our case :
 *
 * - Initialize the serial port I/O (over USB)
 * - Setup pins layout
 * - Retrieve stored calibration data and initialize panel state
 *   + Brightness to 0 (ie led turned off)
 *   + Panel last step time to 0
 *   + If panel IS NOT calibrated, the cover is ASSUMED CLOSED and servo position set to 0. Hence close it first !
 *   + If panel IS calibrated, cover wil be instructed to close and servo position will be retrieved from the feedback pin (corrected by calibration data).  
 */
void setup() {
  // start serial port at 57600 bps:
  Serial.begin(57600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Serial.flush();

  // Make sure the RX, TX, and built-in LEDs don't turn on, they are very bright!
  // Even though the board is inside an enclosure, the light can be seen shining
  // through the small opening for the USB connector! Unfortunately, it is not
  // possible to turn off the power LED (green) in code...
  pinMode(PIN_LED_TXL, INPUT);
  pinMode(PIN_LED_RXL, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  // Setup LED pin as output
  pinMode(LEDSTRIP_PIN, OUTPUT);

  // Setup Servo related pins
  pinMode(SERVO_SWITCH_PIN, OUTPUT);
  pinMode(SERVO_FEEDBACK_PIN, INPUT);
  pinMode(SERVO_CONTROL_PIN, OUTPUT);

  // initializing panel
  panel.brightness = 0;
  panel.servo_position = 0;
  panel.last_step_time = 0L;

  // Read servo calibration data oin Flash storage:
  panel.calibration = nvm_store.read();

  // When there is no calibration data yet, we have to assume that the cover is closed...
  if (!is_panel_calibrated()) {
      panel.cover = CLOSED;
  } else {
    // Close the cover, in case it is not completely closed.
    // To make sure that `_closeçcover` does not have an undefined behavior,
    // we initialize the `panel.state` variable to `OPEN`, just in case.
    // That variable will be updated in the `_close_cover` function,
    // and then again once the cover has completely closed.
    // 
    // Similarly the panel.servo_position variable will be updated with the corrected servo
    // feedback_pin value in the "_close_cover" funcion.
    panel.cover = OPEN;
    bool verbose = false;
    _close_cover(verbose);
  }
}

/*
 * Main arduino entrypoint. It is called in an endless loop until the end of times or till the Seeduino is shut down. 
 *
 * In our case, each loop will sequentially
 *
 * - Handles incoming serial messages and fires the according commands 
 * - check if panel is calibrated and notify (by blinking buil-in led) the users to do so
 * - Update the servo position, if need be, according to the OPENING or CLOSING state of the cover
 */
void loop() {
  // First we seek for commands
  receive_commands();

  // Then we check if calibration if needed
  check_for_calibration();

  // Then we update panel cover position if needed
  update_panel_cover();
}

/*
 * Collect incoming serial messages and fires the according commands.
 * 
 * If data has arrived through the serial port
 *
 * - Reads a full line, i.e a string that ends with \n
 * - Parse the message and get the command payload
 *    + "Serials" the error message ERROR_INVALID_INCOMING_MESSAGE
 *      if message does not comply with the message structure defined in the protocol TYPE:BODY
 *    + "Serials" the error message ERROR_INVALID_INCOMING_MESSAGE_TYPE) if message is not a command, 
 *      i.e, TYPE is not COMMAND
 * - Fires the correct command.
 *   + Each command will serial their own results/errors
 *   + If no matching command is found, the "UNKNOWN" command is fired that "serials" the error message ERROR_INVALID_COMMAND
 */
void receive_commands() {
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil('\n');

    bool error = false;

    auto cmd_payload = get_cmd_payload(message, &error);
    if (error) { return; }

    auto command = get_command_from_payload(cmd_payload);
    command.handle(cmd_payload.args);
  }
}

/*
 * Checks if panel is calibrated and notify (by blinking buil-in led) the users to do so
 */
void check_for_calibration() {

  if (!is_panel_calibrated()) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}

/*
 * Updates the servo/cover physical position, if need be, according to the OPENING or CLOSING state of the cover
 *
 * - If the Panel cover is "OPENING", increments the servo position
 *   +  If it reaches 180, marks the cover as OPEN
 * - If the Panel cover is "CLOSING", decrements the servo position 
 *   +  If it reaches 0, marks the cover as OPEN
 * - Updates the "physical" servo postion with the calculated one
 * - Power down the servo il Panel is OPEN or CLOSED
 *
 * Will do nothing if
 * - Panel is not calibrated
 * - Panel is OPEN
 * - Panel is CLOSED
 */
void update_panel_cover() {
  // We do not move we are not calibrated
  if (!is_panel_calibrated()) return;
  // We do not move we we are yet either OPEN or CLOSED
  if (panel.cover == OPEN || panel.cover == CLOSED) return;

  unsigned long now = micros();
  // We do not move it is too early
  if (now - panel.last_step_time < STEP_DELAY_MICROSEC) return;

  panel.last_step_time = now;

  if (panel.cover == OPENING) {
    panel.servo_position++;
    if (panel.servo_position >= 180) {
      panel.servo_position = 180;
      panel.cover = OPEN;
    }
  } else if (panel.cover == CLOSING) {
    panel.servo_position--;
    if (panel.servo_position <= 0) {
      panel.servo_position = 0;
      panel.cover = CLOSED;
    }
  }

  servo.write(panel.servo_position);

  if (panel.cover == OPEN || panel.cover == CLOSED) {
        powerDownServo();
  }
}

/************************************************
 *                   Commands                   *
 ***********************************************/

/*
 * Ping command handler. 
 * 
 * Dummy command that answers PONG to a COMMAND:PING message.
 *
 * Incoming message :  COMMAND:PING 
 * Args             :             Ignored
 * Serial response  :   RESULT:PING@PONG
 * Serial error     : Never 
 */
void cmd_ping(const String args) {
  serialize_result(COMMAND_PING, RESULT_PING);
}

/*
 * Info command handler. 
 * 
 * This command answers with the version info of the firmware to a COMMAND:INFO message.
 *
 * Incoming message : COMMAND:INFO 
 * Args             : Ignored
 * Serial response  : RESULT:INFO@{RESULT_INFO}, where {RESULT_INFO} is the value of the same constant.
 * Serial error     : Never 
 */
void cmd_info(const String args) {
  serialize_result(COMMAND_INFO, RESULT_INFO);
}

/*
 * "Get Brightness" command handler. 
 * 
 * This command answers with the current brightness value to a COMMAND:BRIGHTNESS_GET message.
 *
 * Incoming message : COMMAND:BRIGHTNESS_GET 
 * Args             : Ignored
 * Serial response  : RESULT:BRIGHTNESS_GET@{panel.brightness}, where {panel.brightness} is the 
 *                    current value of the  brightness member of the panle "panle_state_t" struct.
 * Serial error     : Never
 */
void cmd_brigthness_get(const String args) {
  serialize_result(COMMAND_BRIGHTNESS_GET, String(panel.brightness));
}

/*
 * "Set Brightness" command handler. 
 * 
 * This command sets the current brigthess value according to COMMAND:BRIGHTNESS_SET message parameter value.
 *
 * Incoming message : COMMAND:BRIGHTNESS_SET@{desired_value}
 * Args             : (uint) desired_value
 * Serial response  : RESULT:BRIGHTNESS_SET@{panel.brightness}, where {panel.brightness} is the 
 *                    current value of the  brightness member after being set.
 * Serial error     : Errors in three cases
 *                    - {desired_value} not parsable as int => "INVALID_BRIGHTNESS@Wanted brightness {desired_value} is not a number"
 *                    - {desired_value} <0                  => "INVALID_BRIGHTNESS@Wanted brightness {desired_value} is negative"
 *                    - {desired_value} >  MAX_BRIGHTNESS   => "INVALID_BRIGHTNESS@Wanted brightness {desired_value} is bigger than max allowed value 1023"
 */
void cmd_brightness_set(const String args) {
  auto wanted_brightness = args.toInt();

  if (wanted_brightness == 0 && !has_only_zeros(args)) {
    auto message = ERROR_WANTED_BRIGHTNESS_MSG_START + args + ERROR_WANTED_BRIGHTNESS_NAN_MSG_END;
    serialize_error(message);
    return;
  }

  if (wanted_brightness < 0) {
    auto message = ERROR_WANTED_BRIGHTNESS_MSG_START + args + ERROR_WANTED_BRIGHTNESS_NEGATIVE_MSG_END;
    serialize_error(message);
    return;
  }

  if (wanted_brightness > MAX_BRIGHTNESS) {
    auto message = ERROR_WANTED_BRIGHTNESS_MSG_START + args + ERROR_WANTED_BRIGHTNESS_TOO_BIG_MSG_END;
    serialize_error(message);
    return;
  }

  panel.brightness = (uint32_t)wanted_brightness;
  set_brightness();

  serialize_result(COMMAND_BRIGHTNESS_SET, String(panel.brightness));
}

/*
 * "Reset Brightness" command handler. 
 * 
 * This command resets the current brigthess to 0 when a COMMAND:BRIGHTNESS_RESET message is streamed.
 *
 * Incoming message : COMMAND:BRIGHTNESS_RESET
 * Args             : Ignored
 * Serial response  : RESULT:BRIGHTNESS_RESET@{0}
 * Serial error     : Never
 */
void cmd_brightness_reset(const String args) {
  panel.brightness = 0;
  set_brightness();

  serialize_result(COMMAND_BRIGHTNESS_RESET, String(panel.brightness));
}

/*
 * "Get cover state" command handler. 
 * 
 * This commands gives the current cover state (OPEN, OPENING, CLOSING, CLOSED) in response to a COMMAND:COVER_GET message.
 *
 * Incoming message : COMMAND:COVER_GET
 * Args             : Ignored
 * Serial response  : RESULT:COVER_GET@{panel.cover}, where panel.cover is string human readable translation of the current cover state
 * Serial error     : Never
 */
void cmd_cover_get_state(const String args) {
  switch (panel.cover) {
    case OPENING:
      serialize_result(COMMAND_COVER_GET_STATE, RESULT_COVER_GET_STATE_OPENING);
      break;
    case OPEN:
      serialize_result(COMMAND_COVER_GET_STATE, RESULT_COVER_GET_STATE_OPEN);
      break;
    case CLOSING:
      serialize_result(COMMAND_COVER_GET_STATE, RESULT_COVER_GET_STATE_CLOSING);
      break;
    case CLOSED:
      serialize_result(COMMAND_COVER_GET_STATE, RESULT_COVER_GET_STATE_CLOSED);
      break;
  }
}

/*
 * "Open cover" command handler. 
 * 
 * This commands tells the cover to open in response to a COMMAND:COVER_OPEN message.
 * 
 * - Powers up the servo 
 * - Sets the current "panel.servo_position" according to the (corrected) feedback_pin value
 * - Sets the panel.cover state to OPENING
 *
 * Incoming message : COMMAND:COVER_OPEN
 * Args             : Ignored
 * Serial response  : RESULT:COVER_OPEN@OK
 * Serial error     : Errors in two cases
 *                    - panel is not calibrated              => "SERVO_NO_CALIBRATED@Run command COVER_CALIBRATION_RUN first"
 *                    - panel is neither CLOSING nor CLOSING => "COVER_OPEN_IMPOSSIBLE@Cover is not closed nor closing.""
 */
void cmd_cover_open(const String args) {
  bool verbose = true;
  _open_cover(verbose);
}

// Inner logic funtion of the cmd_cover_open command handler
// Serialization or not of errors and results is parametrised so that the logic can be used in other places (setup for instance)
void _open_cover(bool verbose) {
  if (!is_panel_calibrated()) {
    cond_serialize_error(ERROR_SERVO_NOT_CALIBRATED, verbose);
    return;
  }
  if (!(panel.cover == CLOSED || panel.cover == CLOSING)) {
    cond_serialize_error(ERROR_COVER_OPEN, verbose);
    return;
  }

  panel.cover = OPENING;
  panel.servo_position = powerUpServo();

  cond_serialize_result(COMMAND_COVER_OPEN, RESULT_COVER_OPEN, verbose);
}

/*
 * "Close cover" command handler. 
 * 
 * This commands tells the cover to close in response to a COMMAND:COVER_CLOSE message.
 * 
 * - Powers up the servo 
 * - Sets the current "panel.servo_position" according to the (corrected) feedback_pin value
 * - Sets the panel.cover state to CLOSINGs
 *
 * Incoming message : COMMAND:COVER_CLOSE
 * Args             : Ignored
 * Serial response  : RESULT:COVER_CLOSE@OK
 * Serial error     : Errors in two cases
 *                    - panel is not calibrated              => "SERVO_NO_CALIBRATED@Run command COVER_CALIBRATION_RUN first"
 *                    - panel is neither OPEN nor OPENING    => "COVER_CLOSE_IMPOSSIBLE@Cover is not open nor opening."
 */
void cmd_cover_close(const String args) {
  bool verbose = true;
  _close_cover(verbose);
}

// Inner logic funtion of the cmd_cover_close command handler
// Serialization or not of errors and results is parametrised so that the logic can be used in other places (calibration for instance)
void _close_cover(bool verbose) {
  if (!is_panel_calibrated()) {
    cond_serialize_error(ERROR_SERVO_NOT_CALIBRATED, verbose);
    return;
  }

  if (!(panel.cover == OPEN || panel.cover == OPENING)) {
    cond_serialize_error(ERROR_COVER_CLOSE, verbose);
    return;
  }

  panel.cover = CLOSING;
  panel.servo_position = powerUpServo();

  cond_serialize_result(COMMAND_COVER_CLOSE, RESULT_COVER_CLOSE, verbose);
}

/*
 * "Run calibration" command handler. 
 * 
 * This commands calibrates the servo in response to a COMMAND:CALIBRATION_RUN message.
 * 
 * WARNING: Disconect the right arm from the servo before runing the calibration
 * 
 * - Powers up the servo 
 * - Use linear regression to calibrate the servo feedback pin response
 * - Sets the NVM flag to a known constant value
 * - Stores the calibration data (and NVM flag) in the flash memory
 *
 * This has to be done only once after the firmware has been uploaded.
 *
 * Incoming message : COMMAND:CALIBRATION_RUN
 * Args             : Ignored
 * Serial response  : RESULT:CALIBRATION_RUN@OK
 * Serial error     : Never
 */
void cmd_cover_calibration_run(const String args) {
  
  powerUpServo();

  int step = 10;
  int n_data_points = 1 + 180 / step;

  double x[n_data_points] = { 0 };
  double y[n_data_points] = { 0 };

  for (int i = 0, pos = 0; pos <= 180; i++, pos = i * step) {
    servo.write(pos);
    delay(1000);
    int feedbackValue = analogRead(SERVO_FEEDBACK_PIN);
    x[i] = pos;
    y[i] = feedbackValue;
  }

  linear_regression(x, y, n_data_points, &panel.calibration.slope, &panel.calibration.intercept);
  panel.calibration.magic_number = NVM_MAGIC_NUMBER;
  nvm_store.write(panel.calibration);

  panel.cover = OPEN;

  bool silent = true;
  _close_cover(silent);

  serialize_result(COMMAND_COVER_CALIBRATION_RUN, RESULT_COVER_CALIBRATION_RUN);
}

/*
 * "Get calibration" command handler. 
 * 
 * This commands gives the calibration data in response to a COMMAND:CALIBRATION_RUN message.
 *
 * Incoming message : COMMAND:CALIBRATION_GET
 * Args             : Ignored
 * Serial response  : RESULT:CALIBRATION_GET@slope={panel.calibration.slope} - intercept={panel.calibration.intercept}
 * Serial error     : if panel is not calibrated => "SERVO_NO_CALIBRATED@Run command COVER_CALIBRATION_RUN first"
 */
void cmd_cover_calibration_get(const String args) {
  if (!is_panel_calibrated()) {
      serialize_error(ERROR_SERVO_NOT_CALIBRATED);
      return;
  }

  String message = String("slope=") + panel.calibration.slope + " - "+ "intercept=" + panel.calibration.intercept;
  serialize_result(COMMAND_COVER_CALIBRATION_GET, message);
}

/*
 * Special "unknown" command handler. 
 * 
 * This commands always answers with an error message.
 * Used when no matching command has been found according to the given "command" message. 
 *
 * Incoming message : Any
 * Args             : Ignored
 * Serial response  : Never
 * Serial error     : ERROR_INVALID_COMMAND (always)
 */
void cmd_unknown(const String args) {
  serialize_error(ERROR_INVALID_COMMAND);
}

/************************************************
 *                   Helpers                    *
 ***********************************************/

/*
 * Very simple command message parser. It simply check for position of the various delimiters 
 * and cut the incoming message accordingly. 
 *
 * As this firware only handles "COMMANDS", this methods expect messages formated as TYPE:COMMAND[@ARGS] with:
 * - TYPE == "COMMAND"
 * - COMMAND is [A-Z_]+
 * - ARGS is optional and [:alnum:_\s]+
 *
 * Basically it splits the the message into three part
 * - Considers what is before ":" as TYPE
 * - Considers what is between ":" and "@" (if present) as COMMAND
 * - Considers what is after "@" (if present) as ARGS
 *
 *
 * returns                            : Parsed message as  msg _cmd_payload
 * Serials errors and stop treatment  :
 * - ERROR_INVALID_INCOMING_MESSAGE If separators are ill placed
 * - ERROR_INVALID_INCOMING_MESSAGE_TYPE If the message is not a "COMMAND"
 */
msg_cmd_payload get_cmd_payload(const String message, bool *error) {
  auto type_cmd_sep_idx = message.indexOf(TYPE_COMMAND_SEPARATOR);
  auto cmd_args_sep_idx = message.indexOf(COMMAND_ARGS_SEPARATOR);
  auto message_end_idx = message.length();

  bool has_type_cmd_sep = type_cmd_sep_idx >= 0;
  bool is_type_sep_at_beginnig = type_cmd_sep_idx == 0;
  bool is_type_sep_before_end = type_cmd_sep_idx < (message_end_idx - 1);
  bool has_arg_sep = cmd_args_sep_idx >= 0;
  bool is_type_sep_before_arg_sep = type_cmd_sep_idx < cmd_args_sep_idx;
  bool is_arg_sep_before_end = cmd_args_sep_idx < (message_end_idx - 1);

  bool valid_separators = has_type_cmd_sep
                          && !is_type_sep_at_beginnig
                          && is_type_sep_before_end
                          && (!has_arg_sep || is_type_sep_before_arg_sep)
                          && (!has_arg_sep || is_arg_sep_before_end);

  if (!valid_separators) {
    serialize_error(ERROR_INVALID_INCOMING_MESSAGE);
    *error = true;
    return msg_cmd_payload{};
  }

  String type = message.substring(0, type_cmd_sep_idx);

  if (type != COMMAND_MSG_TYPE) {
    serialize_error(ERROR_INVALID_INCOMING_MESSAGE_TYPE);
    *error = true;
    return msg_cmd_payload{};
  }

  String cmd_name = (has_arg_sep) ? message.substring(type_cmd_sep_idx + 1, cmd_args_sep_idx) : message.substring(type_cmd_sep_idx + 1, message_end_idx);
  String cmd_args = (has_arg_sep) ? message.substring(cmd_args_sep_idx + 1, message_end_idx) : "";

  return msg_cmd_payload{ cmd_name, cmd_args };
}

// Returns the Enum value corresponding to the input string.
// Could be implemented as a Map if O(1) lookup is needed.
command_t get_command_from_payload(const msg_cmd_payload input) {

  for (int i = 0; i < NB_COMMANDS; i++) {
    auto cmd = allowed_cmds[i];
    if (input.name.equalsConstantTime(cmd.name)) {
      return command_t{ cmd.name, cmd.handle };
    }
  }
  return command_t{ COMMAND_UNKNOWN, &cmd_unknown };
}

// The serial message will be
// RESULT:{command}@{message}
void serialize_result(String command, String message) {
  bool verbose = true;
  cond_serialize_result(command, message, verbose);
}

// Serialize a result if verbose
void cond_serialize_result(String command, String message, bool verbose) {
  if (verbose) {
    Serial.print(RESULT_MSG_TYPE);
    Serial.print(TYPE_COMMAND_SEPARATOR);
    Serial.print(command);
    Serial.print(COMMAND_ARGS_SEPARATOR);
    Serial.println(message);
  }
}

void serialize_error(String error) {
  bool verbose = true;
  cond_serialize_error(error, true);
}

void cond_serialize_error(String error, bool verbose) {
  if (verbose) {
    Serial.print(ERROR_MSG_TYPE);
    Serial.print(TYPE_COMMAND_SEPARATOR);
    Serial.println(error);
  }
}

bool has_only_zeros(String num) {
  bool only_zeros = num.length() > 0;
  for (auto c : num) {
    only_zeros = only_zeros && c == '0';
    if (!only_zeros) { return only_zeros; }
  }

  return only_zeros;
}

void set_brightness() {
  // This is ripped almost as is from https://github.com/jlecomte/ascom-flat-panel all credits to him.
  // This only works on Seeeduino Xiao.
  // The `pwm` function is defined in the following file:
  // %localappdata%\Arduino15\packages\Seeeduino\hardware\samd\1.8.2\cores\arduino\wiring_pwm.cpp
  // For other Arduino-compatible boards, consider using:
  //   analogWrite(ledPin, brightness);
  // The nice thing about the `pwm` function is that we can set the frequency
  // to a much higher value (I use 20kHz) This does not work on all pins!
  // For example, it does not work on pin 7 of the Xiao, but it works on pin 8.
  //
  // No need to map anymore our bightness, it laredy a number between 0 and 1023 see BRIGHTNESS_SET command
  // int value = map(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 0, 1023);
  //pwm(ledPin, PWM_FREQ, value);
  pwm(LEDSTRIP_PIN, PWM_FREQ, panel.brightness);
}


// Energize and attach servo.
// This is ripped almost as is from https://github.com/jlecomte/ascom-telescope-cover-v2 all credits to him.
int powerUpServo() {
  digitalWrite(SERVO_SWITCH_PIN, HIGH);

  // Default position (closed), which will be used only once,
  // before we have successfully calibrated the servo.
  int current_pos = panel.servo_position;

  if (is_panel_calibrated()) {
    // Short delay, so that the servo has been fully initialized.
    // Not 100% sure this is necessary, but it won't hurt.
    delay(100);

    int feedbackValue = analogRead(SERVO_FEEDBACK_PIN);
    current_pos = (int)((feedbackValue - panel.calibration.intercept) / panel.calibration.slope);

    // Deal with slight errors in the calibration process...
    if (current_pos < 0) {
      current_pos = 0;
    } else if (current_pos > 180) {
      current_pos = 180;
    }
  }

  // This step is critical! Without it, the servo does not know its position when it is attached below,
  // and the first write command will make it jerk to that position, which is what we want to avoid...
  servo.write(current_pos);
  panel.servo_position = current_pos;

  // The optional min and max pulse width parameters are actually quite important
  // and depend on the exact servo you are using. Without specifying them, you may
  // not be able to use the full range of motion (270 degrees for this project)
  servo.attach(SERVO_CONTROL_PIN, 500, 2500);

  return current_pos;
}

// Detach and de-energize servo to eliminate any possible sources of vibrations.
// Magnets will keep the cover in position, whether it is open or closed.
void powerDownServo() {
  servo.detach();
  digitalWrite(SERVO_SWITCH_PIN, LOW);
}

bool is_panel_calibrated() {
  return panel.calibration.magic_number == NVM_MAGIC_NUMBER;
}

// Function to calculate the mean of an array.
// This is ripped almost as is from https://github.com/jlecomte/ascom-telescope-cover-v2 all credits to him.
double mean(double arr[], int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum / n;
}

// Function to calculate the slope and intercept of a linear regression line.
// This is ripped almost as is from https://github.com/jlecomte/ascom-telescope-cover-v2 all credits to him.
void linear_regression(double x[], double y[], int n, double *slope, double *intercept) {
    double x_mean = mean(x, n);
    double y_mean = mean(y, n);
    double numerator = 0.0;
    double denominator = 0.0;
    for (int i = 0; i < n; i++) {
        numerator += (x[i] - x_mean) * (y[i] - y_mean);
        denominator += (x[i] - x_mean) * (x[i] - x_mean);
    }
    *slope = numerator / denominator;
    *intercept = y_mean - (*slope * x_mean);
}
