# Com protocol

In this scenario a flat panel is made of two things
- A light panel with variable brightness
- A motorized cover

The firmware updates the state of the flat panel upon instructions of the (ASCOM or INDI) driver. The communication protocol is serial based. The serial connection is established via USB.  

## Messages 

The protocol is thus "message" based. 
- Both the driver and the flat panel (the firmware) exchange single line messages; ie terminated by '\n'.
- A message is structured as TYPE:MESSAGE, where TYPE is in "COMMAND, RESULT, ERROR" and MESSAGE is alaphanumerical with spaces and "@"

The **driver emmits** and the **firmware handles** the incoming "COMMAND" messages. This kind of message is stuctured as 

- `COMMAND:NAME[@ARGS]` 

where NAME is [A-Z_]+ and ARGS may be optional or mandatory and their nature may depend on the command. For instance for a `COMMAND:BRIGHTNESS_SET@ARGS` message the ARGS are mandatory and should consists of a single "int"

The firmware then responds with 
- either a `RESULT:CMD_NAME@VALUE`   if operation succeeded
- or an `ERROR:ERR_MESSAGE@DETAILS` if anything went wrong

In a nutshell

     -----------------                           ------------
    |                 |                         |            |
    |      Driver     | COMMAND:CMD_NAME[@ARGS] | Flat Panel |
    | (ASCOM or INDI) | --------------------->  | (firmware) |
    |                 | <---------------------  |            |
     -----------------  RESULT:CMD_NAME@VALUE    ------------
                                or
                        ERROR:ERR_MESSAGE@DETAILS

Allowed commands are:
- `COMMAND:PING`
- `COMMAND:INFO`
- `COMMAND:BRIGHTNESS_GET`
- `COMMAND:BRIGHTNESS_SET@{(int) DESIRED_VALUE}`
- `COMMAND:BRIGHTNESS_RESET`
- `COMMAND:COVER_GET_STATE`
- `COMMAND:COVER_OPEN`
- `COMMAND:COVER_CLOSE`
- `COMMAND:COVER_CALIBRATION_RUN`
- `COMMAND:COVER_CALIBRATION_GET`

## Command Details 

### Ping 

- Incoming message :  `COMMAND:PING`
- Args             :  Ignored
- Serial response  :  `RESULT:PING@PONG`
- Serial error     :  Never 
  
### Info 

- Incoming message : `COMMAND:INFO`
- Args             : Ignored
- Serial response  : `RESULT:INFO@{RESULT_INFO}`, where {RESULT_INFO} is the value of the firmware informations ex Le Télescope - Ivry sur Seine - Flat Panel Firmware v1.0.
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

- Incoming message : `COMMAND:COVER_GET`
- Args             : Ignored
- Serial response  : `RESULT:COVER_GET@{panel.cover}`, where panel.cover is string human readable translation of the current cover state. Possible values are in [**OPEN**, **OPENING**, **CLOSING**, **CLOSED**]
- Serial error     : Never

### Open cover

- Incoming message : `COMMAND:COVER_OPEN`
- Args             : Ignored
- Serial response  : `RESULT:COVER_OPEN@OK`
- Serial error     : If panel is not calibrated => `ERROR:SERVO_NO_CALIBRATED@Run command COVER_CALIBRATION_RUN first`


### Close cover

- Incoming message : `COMMAND:COVER_CLOSE`
- Args             : Ignored
- Serial response  : `RESULT:COVER_CLOSE@OK`
- Serial error     : If panel is not calibrated => `SERVO_NO_CALIBRATED@Run command COVER_CALIBRATION_RUN first`



### Run calibration

- Incoming message : `COMMAND:CALIBRATION_RUN`
- Args             : Ignored
- Serial response  : `RESULT:CALIBRATION_RUN@OK`
- Serial error     : Never

WARNING: Disconect the right arm from the servo before runing the calibration

### Get calibration

- Incoming message : `COMMAND:CALIBRATION_GET`
- Args             : Ignored
- Serial response  : `RESULT:CALIBRATION_GET@slope={panel.calibration.slope} - intercept={panel.calibration.intercept}`
- Serial error     : if panel is not calibrated => `SERVO_NO_CALIBRATED@Run command COVER_CALIBRATION_RUN first`

### Unknown commands

This one is a special command that is not meant to be called. It is in fact the result of an unknow command sent to the firmware.

- Incoming message : Any message not in the previously mentionned ones.
- Args             : Ignored
- Serial response  : Never
- Serial error     : `ERROR:INVALID_COMMAND@Allowed commands PING, INFO, BRIGHTNESS_GET, BRIGHTNESS_SET, BRIGHTNESS_RESET, COVER_GET_STATE, COVER_OPEN, COVER_CLOSE, COVER_CALIBRATION_RUN, COVER_CALIBRATION_GET` (always)

## Specific error cases

The firmware may emit error message not related to a specific command. 

- If the driver/client serialise a string  that is not parsable as a *message*, as understood in this context, the firmware should respon with `ERROR:INVALID_INCOMING_MESSAGE@Allowed messages are TYPE:MESSAGE`

- If the driver/client emmits a valid message that is not a command, the firmware should respon with `ERROR:INVALID_INCOMING_MESSAGE_TYPE@Allowed types COMMAND`

- If the driver/client emmits an unknown command, ie not one in this list, the firmware shoudl respond with `ERROR:INVALID_COMMAND@Allowed commands PING, INFO, BRIGHTNESS_GET, BRIGHTNESS_SET, BRIGHTNESS_RESET, COVER_GET_STATE, COVER_OPEN, COVER_CLOSE, COVER_CALIBRATION_RUN, COVER_CALIBRATION_GET`, c.f. [unknow "commands"](#unknown-commands) above
