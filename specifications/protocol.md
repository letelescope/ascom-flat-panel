# Com protocol

In this scenario a flat panel is made of two things
- A light panel with variable brightness
- A motorized cover

The firmware updates the state of the flat panel upon instructions of the (ASCOM or INDI) driver. The communication protocol is serial based. The serial connection is established via USB.  

## Messages 

The protocol is thus message based.
- Both the driver and the flat panel (the firmware) exchange single-line messages terminated by '\n'.
- A message is structured as `TYPE:PAYLOAD`, where `TYPE` is one of `COMMAND`, `RESULT`, or `ERROR`, and `PAYLOAD` may include alphanumeric characters, spaces, and `@` separators.

The **driver emits** and the **firmware handles** incoming `COMMAND` messages. A command message is structured as:

- `COMMAND:NAME[@ARGS]` 

where `NAME` is `[A-Z_]+` and `ARGS` may be optional or mandatory depending on the command. For example, in `COMMAND:BRIGHTNESS_SET@ARGS` the `ARGS` are required and represent a single integer value.

The firmware then responds with:
- `RESULT:CMD_NAME@VALUE` when the operation succeeds
- `ERROR:ERR_MESSAGE@DETAILS` when the operation fails

In a nutshell

     -----------------                           ------------
    |                 |                         |            |
    |      Driver     | COMMAND:CMD_NAME[@ARGS] | Flat Panel |
    | (ASCOM or INDI) | --------------------->  | (firmware) |
    |                 | <---------------------  |            |
     -----------------  RESULT:CMD_NAME@VALUE    ------------
                                or
                        ERROR:ERR_MESSAGE@DETAILS

### Command summary
Allowed commands are:
- `COMMAND:PING` — no args, responds with `RESULT:PING@PONG`
- `COMMAND:INFO` — no args, responds with firmware info string
- `COMMAND:VERSION` — no args, responds with firmware version string
- `COMMAND:MAX_BRIGHTNESS` — no args, responds with maximum brightness value
- `COMMAND:BRIGHTNESS_GET` — no args, responds with current brightness value
- `COMMAND:BRIGHTNESS_SET@{value}` — sets brightness and responds with the new value
- `COMMAND:BRIGHTNESS_RESET` — resets brightness to 0
- `COMMAND:COVER_GET_STATE` — returns current cover state
- `COMMAND:COVER_OPEN` — opens the cover
- `COMMAND:COVER_CLOSE` — closes the cover
- `COMMAND:COVER_CALIBRATION_RUN` — runs servo calibration
- `COMMAND:COVER_CALIBRATION_GET` — returns calibration parameters
- `COMMAND:DISCONNECT` — acknowledges disconnection

### Examples
- `COMMAND:PING` → `RESULT:PING@PONG`
- `COMMAND:BRIGHTNESS_SET@100` → `RESULT:BRIGHTNESS_SET@100`
- `COMMAND:COVER_GET_STATE` → `RESULT:COVER_GET_STATE@OPEN`

## Command Details 

### Ping 

- Incoming message :  `COMMAND:PING`
- Args             :  Ignored
- Serial response  :  `RESULT:PING@PONG`
- Serial error     :  Never 
  
### Info 

- Incoming message : `COMMAND:INFO`
- Args             : Ignored
- Serial response  : `RESULT:INFO@{RESULT_INFO}`, where {RESULT_INFO} is the value of the firmware information, e.g. `Le Télescope - Ivry sur Seine - Flat Panel Firmware v1.0`.
- Serial error     : Never 

### Version

- Incoming message : `COMMAND:VERSION`
- Args             :  Ignored
- Serial response  : `RESULT:VERSION@{full_version}`, where {full_version} is the firmware semantic version including panel type and git revision, e.g. `1.0.0.el_panel.<Git Revision>` or `1.0.0.led_panel.<Git Revision>`.
- Serial error     : Never 

### Get Max Brightness

- Incoming message : `COMMAND:MAX_BRIGHTNESS`
- Args             :  Ignored
- Serial response  : `RESULT:MAX_BRIGHTNESS@{MAX_BRIGHTNESS}`, where {MAX_BRIGHTNESS} is the maximum brightness supported by the panel type.
- Serial error     : Never 

### Get Brightness

- Incoming message : `COMMAND:BRIGHTNESS_GET`
- Args             : Ignored
- Serial response  : `RESULT:BRIGHTNESS_GET@{brightness}`, where {brightness} is the current value of the brightness of the panel, as an (int), ex: 50.
- Serial error     : Never

### Set Brightness

- Incoming message : `COMMAND:BRIGHTNESS_SET@{desired_value}`
- Args             : **(uint)** desired_value
- Serial response  : `RESULT:BRIGHTNESS_SET@{panel.brightness}`, where {panel.brightness} is the value of the  brightness member after being set, as an (int).
- Serial error     : Errors in three cases
  - {desired_value} not parsable as int => `INVALID_BRIGHTNESS@Wanted brightness {desired_value} is not a number`
  - {desired_value} <0                  => `INVALID_BRIGHTNESS@Wanted brightness {desired_value} is negative`
  - {desired_value} >  MAX_BRIGHTNESS   => `INVALID_BRIGHTNESS@Wanted brightness {desired_value} is bigger than max allowed value 1023`

### Reset brightness

- Incoming message : `COMMAND:BRIGHTNESS_RESET`
- Args             : Ignored
- Serial response  : `RESULT:BRIGHTNESS_RESET@{0}`
- Serial error     : Never

### Get cover state 

- Incoming message : `COMMAND:COVER_GET_STATE`
- Args             : Ignored
- Serial response  : `RESULT:COVER_GET_STATE@{panel.cover}`, where panel.cover is string human readable translation of the current cover state. Possible values are in [**OPEN**, **OPENING**, **CLOSING**, **CLOSED**]
- Serial error     : Never

### Open cover

- Incoming message : `COMMAND:COVER_OPEN`
- Args             : Ignored
- Serial response  : `RESULT:COVER_OPEN@OK`
- Serial error     : If panel is not calibrated => `ERROR:SERVO_NOT_CALIBRATED@Run command COVER_CALIBRATION_RUN first`


### Close cover

- Incoming message : `COMMAND:COVER_CLOSE`
- Args             : Ignored
- Serial response  : `RESULT:COVER_CLOSE@OK`
- Serial error     : If panel is not calibrated => `ERROR:SERVO_NOT_CALIBRATED@Run command COVER_CALIBRATION_RUN first`



### Run calibration

- Incoming message : `COMMAND:COVER_CALIBRATION_RUN`
- Args             :  Ignored
- Serial response  : `RESULT:COVER_CALIBRATION_RUN@OK`
- Serial error     : Never 

WARNING: Disconnect the right arm from the servo before running the calibration

### Get calibration

- Incoming message : `COMMAND:COVER_CALIBRATION_GET`
- Args             :  Ignored
- Serial response  : `RESULT:COVER_CALIBRATION_GET@slope={panel.calibration.slope} - intercept={panel.calibration.intercept}`
- Serial error     : if panel is not calibrated => `ERROR:SERVO_NOT_CALIBRATED@Run command COVER_CALIBRATION_RUN first`

### Disconnect

- Incoming message : `COMMAND:DISCONNECT`
- Args             :  Ignored
- Serial response  : `RESULT:DISCONNECT@OK`
- Serial error     : Never

### Unknown commands

This one is a special command that is not meant to be called. It is in fact the result of an unknown command sent to the firmware.

- Incoming message : Any message not in the previously mentioned ones.
- Args             : Ignored
- Serial response  : Never
- Serial error     : `ERROR:INVALID_COMMAND@Allowed commands PING, INFO, VERSION, MAX_BRIGHTNESS, BRIGHTNESS_GET, BRIGHTNESS_SET, BRIGHTNESS_RESET, COVER_GET_STATE, COVER_OPEN, COVER_CLOSE, COVER_CALIBRATION_RUN, COVER_CALIBRATION_GET, DISCONNECT` (always)

## Specific error cases

The firmware may emit error messages not related to a specific command.

- If the driver/client serializes a string that is not parsable as a message, the firmware should respond with `ERROR:INVALID_INCOMING_MESSAGE@Allowed messages are TYPE:MESSAGE`

- If the driver/client emits a valid message that is not a command, the firmware should respond with `ERROR:INVALID_INCOMING_MESSAGE_TYPE@Allowed types COMMAND`

- If the driver/client emits an unknown command, i.e. one not in this list, the firmware should respond with `ERROR:INVALID_COMMAND@Allowed commands PING, INFO, VERSION, MAX_BRIGHTNESS, BRIGHTNESS_GET, BRIGHTNESS_SET, BRIGHTNESS_RESET, COVER_GET_STATE, COVER_OPEN, COVER_CLOSE, COVER_CALIBRATION_RUN, COVER_CALIBRATION_GET, DISCONNECT`, c.f. [unknown "commands"](#unknown-commands) above
