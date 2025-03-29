# ascom-flat-panel
Open source ASCOM driven automated flat panel for astrophotography. 

## Introduction

This project should contain all you need to build your own ASCOM driven Flat Panel.

- arduino_firmware: Firware of the programble chip used to control the panel and its cover
- ASCOM_drivers: Implementant of an ASCOM driver server contro

This project, the protocol, the implementation of both the driver and the firmware, the electronics and the 3D models are heavily inspired by the the work  of Dark Sky Geek (https://github.com/jlecomte/) especially 
- https://github.com/jlecomte/ascom-flat-panel
- https://github.com/jlecomte/ascom-wireless-flat-panel
- https://github.com/jlecomte/ascom-telescope-cover-v2

Kudos to him. 

## Principle

In this scenario a flat panel is made of two things
- A light panel with variable brightness
- A motorized cover

The firmware updates the state of the flat panel upon instructions of the ASCOM driver. The communication protocol is serial based. The serial connection is established via USB. Here is a a summary of the protocol 
- Both the driver and the flat panel (and hence this firmware) exchange single line messages; ie terminated by '\n'.
- A message is structured as TYPE:MESSAGE, where TYPE is in "COMMAND, RESULT, ERROR" and MESSAGE is alaphanumerical with spaces and "@"

The driver emmits and the firmware handles the incoming "COMMAND" messages. This kind of message is stuctured as 

COMMAND:NAME[@ARGS]

where NAME is [A-Z_]+ and ARGS may be optional or mandatory and their nature may depend on the command. For instance for a COMMAND:BRIGHTNESS_SET@ARGS message the ARGS are mandatory and should consists of a single "int"

The firware reponse with 
- either a RESULT:CMD_NAME}@MSG   if operation succeeded
- or an ERROR:ERR_MESSAGE@DETAILS if anything went wrong

In a nutshell

     --------------                           ------------
    |              |                         |            |
    |              | COMMAND:CMD_NAME[@ARGS] | Flat Panel |
    | ASCOM driver | --------------------->  | (firmware) |
    |              | <---------------------  |            |
     --------------  RESULT:CMD_NAME@VALUE    ------------
                          or
                    ERROR:ERR_MESSAGE@DETAILS

More details on this protocol are provided in [protocol.md](./serial_protocol/protocol.md)




 