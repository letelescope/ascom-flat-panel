#include <Arduino.h>

/************************************************
 *     Types, Objects, and data structures      *
 ***********************************************/

/*
 * Messages/Protocol related types
 */

// Represents a command message payload right after parsing, before decoding.

struct msg_cmd_payload
{
  String name;
  String args;
};

// Defines the behavior of a command. Each command
//  - may perform an action that will modify the flat panel firware state
//  - may be given args as a String. It is the responsability of the command to check the correctness of the given input
typedef void (*cmd_handler_ptr)(String);

// Main command data structure. A command has a name, and holds a pointer to a function that will perform the action.
struct command_t
{
  const char *name;
  cmd_handler_ptr handle;
};

void receive_commands();
void check_for_calibration();
void update_panel_cover();
/************************************************
 *                   Commands                   *
 ***********************************************/
void cmd_ping(const String args);
void cmd_info(const String args);
void cmd_brigthness_get(const String args);
void cmd_brightness_set(const String args);
void cmd_brightness_reset(const String args);
void cmd_cover_get_state(const String args);
void cmd_cover_open(const String args);
void _open_cover(bool verbose);
void cmd_cover_close(const String args);
void _close_cover(bool verbose);
void cmd_cover_calibration_run(const String args);
void cmd_cover_calibration_get(const String args);
void cmd_disconnect(const String args);
void cmd_unknown(const String args);

/************************************************
 *                   helpers                    *
 ***********************************************/

msg_cmd_payload get_cmd_payload(const String message, bool *error);
command_t get_command_from_payload(const msg_cmd_payload input);
void serialize_result(String command, String message);
void cond_serialize_result(String command, String message, bool verbose);
void serialize_error(String error);
void cond_serialize_error(String error, bool verbose);
bool has_only_zeros(String num);
void set_brightness();
int powerUpServo();
void powerDownServo();
int get_current_servo_pos();
bool is_panel_calibrated();
void linear_regression(double x[], double y[], int n, double *slope, double *intercept);