constexpr auto MESSAGE_END_DELIMITER = "\n";
constexpr auto TYPE_COMMAND_SEPARATOR = ":";
constexpr auto COMMAND_ARGS_SEPARATOR = "@";

constexpr auto COMMAND_MSG_TYPE = "COMMAND";
constexpr auto RESULT_MSG_TYPE = "RESULT";
constexpr auto ERROR_MSG_TYPE = "ERROR";

constexpr auto COMMAND_PING = "PING";
constexpr auto RESULT_PING = "PONG";

constexpr auto COMMAND_INFO = "INFO";
constexpr auto RESULT_INFO = "Le Telescope Flat Panel Firmware v1.0";

constexpr auto COMMAND_BRIGHTNESS_GET = "BRIGHTNESS_GET";
constexpr auto COMMAND_BRIGHTNESS_SET = "BRIGHTNESS_SET";
constexpr auto COMMAND_BRIGHTNESS_RESET = "BRIGHTNESS_RESET";

constexpr auto COMMAND_COVER_GET_STATE = "COVER_GET_STATE";
constexpr auto COMMAND_COVER_OPEN = "COVER_OPEN";
constexpr auto COMMAND_COVER_CLOSE = "COVER_CLOSE";
constexpr auto COMMAND_COVER_CALIBRATION_RUN = "COVER_CALIBRATION_RUN";
constexpr auto COMMAND_COVER_CALIBRATION_GET = "COVER_CALIBRATION_GET";

constexpr auto ERROR_NOT_IMPLEMENTED = "UNSOPPORTEF_COMMAND@Not implemented";
constexpr auto ERROR_INVALID_INCOMING_MESSAGE = "INVALID_INCOMING_MESSAGE@Allowed messages are TYPE:COMMAND[@PARAMETER]";
constexpr auto ERROR_INVALID_INCOMING_MESSAGE_TYPE = "INVALID_INCOMING_MESSAGE_TYPE@Allowed types COMMAND";
constexpr auto ERROR_INVALID_COMMAND = "INVALID_COMMAND@Allowed commands PING, INFO, BRIGHTNESS_GET, BRIGHTNESS_SET, BRIGHTNESS_RESET";
constexpr auto ERROR_WANTED_BRIGHTNESS_MSG_START = "INVALID_BRIGHTNESS@Wanted brightness {";
constexpr auto ERROR_WANTED_BRIGHTNESS_NAN_MSG_END = "} is not a number.";
constexpr auto ERROR_WANTED_BRIGHTNESS_NEGATIVE_MSG_END = "} is negative.";
constexpr auto ERROR_WANTED_BRIGHTNESS_TOO_BIG_MSG_END = "} is bigger than max allowed value 1023";


typedef struct command {
  String name;
  String args;
};

//Keeps the record of knwom commands index
enum COMMAND_HANDLER {
  PING,
  INFO,
  BRIGHTNESS_GET,
  BRIGTHNESS_SET,
  BRIGHTNESS_RESET,
  COVER_GET_STATE,
  COVER_OPEN, 
  COVER_CLOSE,
  COVER_CALIBRATION_RUN,
  COVER_CALIBRATION_GET, 
  UNKONW,
};

typedef void (*command_t)(String);
constexpr command_t cmd_handlers[] = {&cmd_ping, 
                                      &cmd_info,
                                      &cmd_brigthness_get, 
                                      &cmd_brightness_set, 
                                      &cmd_brightness_reset,
                                      &cmd_cover_get_state,
                                      &cmd_cover_open,
                                      &cmd_cover_close,
                                      &cmd_cover_calibration_run,
                                      &cmd_cover_calibration_get,
                                      &cmd_unknown};

constexpr uint32_t MIN_BRIGHTNESS = 0;
constexpr uint32_t MAX_BRIGHTNESS = 1023;
constexpr uint32_t PWM_FREQ = 20000;

constexpr auto LEDSTRIP_PIN = 8;

uint32_t brightness = 0;

void setup() {
  // start serial port at 9600 bps:
  Serial.begin(9600);
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
}

void loop() {
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil('\n');

    bool error = false;
    
    auto cmd = get_command(message, &error);
    if (error) { return; }
    
    auto cmd_idx = handler_index(cmd.name);
    cmd_handlers[cmd_idx](cmd.args);
  }
}

//
// Commands
//

// Ping
// Answers PONG
void cmd_ping(const String args) {
  serialize_result(COMMAND_PING, RESULT_PING);
}

// Info
// Answers Firware version info
void cmd_info(const String args) {
  serialize_result(COMMAND_INFO, RESULT_INFO);
}

// Get Brightness
// Answers Birghtness value
void cmd_brigthness_get(const String args){
  serialize_result(COMMAND_BRIGHTNESS_GET, String (brightness));
}

// Set Brightness value
// Answers with set brightness value
void cmd_brightness_set(const String args){
  auto wanted_brightness = args.toInt();

  if (wanted_brightness == 0 && ! has_only_zeros(args)) {
    auto message = ERROR_WANTED_BRIGHTNESS_MSG_START + args + ERROR_WANTED_BRIGHTNESS_NAN_MSG_END;
    serialize_error(message); 
    return;  
  }
  
  if (wanted_brightness < 0 ) {
    auto message = ERROR_WANTED_BRIGHTNESS_MSG_START + args + ERROR_WANTED_BRIGHTNESS_NEGATIVE_MSG_END;
    serialize_error(message); 
    return;  
  }
  
  if (wanted_brightness > MAX_BRIGHTNESS) {
    auto message = ERROR_WANTED_BRIGHTNESS_MSG_START + args + ERROR_WANTED_BRIGHTNESS_TOO_BIG_MSG_END;
    serialize_error(message);
    return;
  }

  brightness = (uint32_t) wanted_brightness;
  set_brightness();

  serialize_result(COMMAND_BRIGHTNESS_SET, String (brightness));
}

// Reset Brightness value
// Answers with reset brightness value
void cmd_brightness_reset(const String args){
  brightness = 0;
  set_brightness();
  serialize_result(COMMAND_BRIGHTNESS_RESET, String (brightness));
}

void cmd_cover_get_state(const String args) { serialize_error(ERROR_INVALID_COMMAND); }
void cmd_cover_open(const String args) { serialize_error(ERROR_INVALID_COMMAND); }
void cmd_cover_close(const String args) { serialize_error(ERROR_INVALID_COMMAND); }
void cmd_cover_calibration_run(const String args) { serialize_error(ERROR_INVALID_COMMAND); }
void cmd_cover_calibration_get(const String args) { serialize_error(ERROR_INVALID_COMMAND); }

// Handles unknown commands
// Send error message stating command is unknow
void cmd_unknown(const String args) {
    serialize_error(ERROR_INVALID_COMMAND);
}
// ---------------------
// Helpers
// ---------------------- 

// Returns the Enum value corresponding to the input string.
// Could be implemented as a Map if O(1) lookup is needed. 
byte handler_index(const String input){
  if ( input.equalsConstantTime(COMMAND_PING)) return COMMAND_HANDLER::PING;
	if ( input.equalsConstantTime(COMMAND_INFO)) return COMMAND_HANDLER::INFO;
  if ( input.equalsConstantTime(COMMAND_BRIGHTNESS_GET)) return COMMAND_HANDLER::BRIGHTNESS_GET;
  if ( input.equalsConstantTime(COMMAND_BRIGHTNESS_SET)) return COMMAND_HANDLER::BRIGTHNESS_SET;
  if ( input.equalsConstantTime(COMMAND_BRIGHTNESS_RESET)) return COMMAND_HANDLER::BRIGHTNESS_RESET;
  if ( input.equalsConstantTime(COMMAND_COVER_GET_STATE)) return COMMAND_HANDLER::COVER_GET_STATE;
  if ( input.equalsConstantTime(COMMAND_COVER_OPEN)) return COMMAND_HANDLER::COVER_OPEN;
  if ( input.equalsConstantTime(COMMAND_COVER_CLOSE)) return COMMAND_HANDLER::COVER_CLOSE;
  if ( input.equalsConstantTime(COMMAND_COVER_CALIBRATION_RUN)) return COMMAND_HANDLER::COVER_CALIBRATION_RUN;
  if ( input.equalsConstantTime(COMMAND_COVER_CALIBRATION_GET)) return COMMAND_HANDLER::COVER_CALIBRATION_GET;
	return COMMAND_HANDLER::UNKONW;
}

command get_command(const String message, bool *error) {
    auto type_cmd_sep_idx = message.indexOf(TYPE_COMMAND_SEPARATOR);
    auto cmd_args_sep_idx = message.indexOf(COMMAND_ARGS_SEPARATOR);
    auto message_end_idx = message.length();

    bool has_type_cmd_sep = type_cmd_sep_idx > 0;
    bool is_type_sep_at_beginnig = type_cmd_sep_idx == 0;
    bool is_type_sep_before_end = type_cmd_sep_idx < (message_end_idx -1); 
    bool has_arg_sep = cmd_args_sep_idx > 0;
    bool is_type_sep_before_arg_sep = type_cmd_sep_idx < cmd_args_sep_idx;
    bool is_arg_sep_before_end = cmd_args_sep_idx < (message_end_idx -1);

    bool valid_separators = has_type_cmd_sep 
                            && !is_type_sep_at_beginnig 
                            && is_type_sep_before_end
                            && (has_arg_sep && is_type_sep_before_arg_sep)
                            && (has_arg_sep && is_arg_sep_before_end);

    if ( !valid_separators) { 
      serialize_error(ERROR_INVALID_INCOMING_MESSAGE_TYPE); 
      *error = true; 
      return command {"",""}; 
    }
    
    String type = message.substring(0,type_cmd_sep_idx);

    if (type != COMMAND_MSG_TYPE) { 
      serialize_error(ERROR_INVALID_INCOMING_MESSAGE_TYPE); 
      *error = true;
      return command {"",""};
    } 

    String cmd_name = (has_arg_sep) ? message.substring(type_cmd_sep_idx+1, cmd_args_sep_idx) : message.substring(type_cmd_sep_idx+1, message_end_idx);
    String cmd_args = (has_arg_sep) ? message.substring(cmd_args_sep_idx + 1, message_end_idx) : "";

    return command {cmd_name, cmd_args};
}

void serialize_result(String command, String message) {
  Serial.print(RESULT_MSG_TYPE);
  Serial.print(TYPE_COMMAND_SEPARATOR);
  Serial.print(command);
  Serial.print(COMMAND_ARGS_SEPARATOR);
  Serial.println(message);
}

void serialize_error(String error) {
  Serial.print(ERROR_MSG_TYPE);
  Serial.print(TYPE_COMMAND_SEPARATOR);
  Serial.println(error);
}

bool has_only_zeros(String num) {
  bool only_zeros = num.length() > 0;
  for (auto c : num) {
    only_zeros = only_zeros && c =='0';
    if (!only_zeros) { return only_zeros; }
  }

  return only_zeros;
}

void set_brightness() {
    // This is ripped as is from https://github.com/jlecomte/ascom-flat-panel all credits to him. 
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
    pwm(LEDSTRIP_PIN, PWM_FREQ, brightness);
}
