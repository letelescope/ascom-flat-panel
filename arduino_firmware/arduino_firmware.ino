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

constexpr auto COMMAND_UNKNOWN = "UNKNOWN";

constexpr auto ERROR_NOT_IMPLEMENTED = "UNSOPPORTEF_COMMAND@Not implemented";
constexpr auto ERROR_INVALID_INCOMING_MESSAGE = "INVALID_INCOMING_MESSAGE@Allowed messages are TYPE:COMMAND[@PARAMETER]";
constexpr auto ERROR_INVALID_INCOMING_MESSAGE_TYPE = "INVALID_INCOMING_MESSAGE_TYPE@Allowed types COMMAND";
constexpr auto ERROR_INVALID_COMMAND = "INVALID_COMMAND@Allowed commands PING, INFO, BRIGHTNESS_GET, BRIGHTNESS_SET, BRIGHTNESS_RESET";
constexpr auto ERROR_WANTED_BRIGHTNESS_MSG_START = "INVALID_BRIGHTNESS@Wanted brightness {";
constexpr auto ERROR_WANTED_BRIGHTNESS_NAN_MSG_END = "} is not a number.";
constexpr auto ERROR_WANTED_BRIGHTNESS_NEGATIVE_MSG_END = "} is negative.";
constexpr auto ERROR_WANTED_BRIGHTNESS_TOO_BIG_MSG_END = "} is bigger than max allowed value 1023";

typedef struct msg_cmd_payload {
  String name;
  String args;
};

//Keeps the record of knwom commands index
typedef void (*cmd_handler_t)(String);

typedef struct command {
  const char * name;
  cmd_handler_t handle;
};

constexpr command allowed_cmds[] = {{COMMAND_PING,&cmd_ping}, 
                                      {COMMAND_INFO,&cmd_info},
                                      {COMMAND_BRIGHTNESS_GET,&cmd_brigthness_get}, 
                                      {COMMAND_BRIGHTNESS_SET,&cmd_brightness_set}, 
                                      {COMMAND_BRIGHTNESS_RESET,&cmd_brightness_reset},
                                      {COMMAND_COVER_GET_STATE,&cmd_cover_get_state},
                                      {COMMAND_COVER_OPEN,&cmd_cover_open},
                                      {COMMAND_COVER_CLOSE,&cmd_cover_close},
                                      {COMMAND_COVER_CALIBRATION_RUN,&cmd_cover_calibration_run},
                                      {COMMAND_COVER_CALIBRATION_GET,&cmd_cover_calibration_get},
};

constexpr uint32_t MIN_BRIGHTNESS = 0;
constexpr uint32_t MAX_BRIGHTNESS = 1023;
constexpr uint32_t PWM_FREQ = 20000;

constexpr auto LEDSTRIP_PIN = 8;

typedef struct panel_state_t {
  uint32_t brightness;
};

panel_state_t panel_state {0};

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
    
    auto cmd_payload = get_cmd_payload(message, &error);
    if (error) { return; }
    
    auto command =  get_command_from_payload(cmd_payload);
    command.handle(cmd_payload.args);
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
  serialize_result(COMMAND_BRIGHTNESS_GET, String (panel_state.brightness));
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

  panel_state.brightness = (uint32_t) wanted_brightness;
  set_brightness();

  serialize_result(COMMAND_BRIGHTNESS_SET, String (panel_state.brightness));
}

// Reset Brightness value
// Answers with reset brightness value
void cmd_brightness_reset(const String args){
  panel_state.brightness = 0;
  set_brightness();

  serialize_result(COMMAND_BRIGHTNESS_RESET, String (panel_state.brightness));
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
command get_command_from_payload(const msg_cmd_payload input){
  
  for(auto cmd : allowed_cmds) {
    if(input.name.equalsConstantTime(cmd.name)) {
      return command{cmd.name, cmd.handle};
    }
  }
	return command {COMMAND_UNKNOWN,&cmd_unknown};
}

msg_cmd_payload get_cmd_payload(const String message, bool *error) {
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
      return msg_cmd_payload {}; 
    }
    
    String type = message.substring(0,type_cmd_sep_idx);

    if (type != COMMAND_MSG_TYPE) { 
      serialize_error(ERROR_INVALID_INCOMING_MESSAGE_TYPE); 
      *error = true;
      return msg_cmd_payload {};
    } 

    String cmd_name = (has_arg_sep) ? message.substring(type_cmd_sep_idx+1, cmd_args_sep_idx) : message.substring(type_cmd_sep_idx+1, message_end_idx);
    String cmd_args = (has_arg_sep) ? message.substring(cmd_args_sep_idx + 1, message_end_idx) : "";

    return msg_cmd_payload {cmd_name, cmd_args};
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
    pwm(LEDSTRIP_PIN, PWM_FREQ, panel_state.brightness);
}
