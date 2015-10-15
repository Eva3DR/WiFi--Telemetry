## Code for a Telemetry WiFi module based on Espressif ESP8266 ##

This repository contains the code for a telemetry bridge on a WiFi module. The device used is the Espressif ESP8266. The used SDK version of the WiFi module is [1.3.0_15_08_08] (http://bbs.espressif.com/viewtopic.php?f=46&t=919). The code was develop on the Eclipse IDE.

The main code is located on the user folder.

### Description ###

The applications consist on one user task that manages the reception and transmision of MAVlink frames, and reception of commands. 

For the MAVlink frames, the application acts as a bridge (UART - WiFi). 

For commands, received through WiFi or UART, the application decodes them and execute the corresponding actions. 

The WiFi reception/transmission is on UDP packets. 


### Commands ###

The available commands are to:
*   Change the Access Point SSID and password
*   Change the UART baudrate
*   Change the transmission power
*   Change the IP of the Access Point and listening UDP port
*   Test command
 
The commands are in the form of AT, and have to end with: \r\n. 

#### Usage ####

*   Change the Access Point SSID and password:

AT+CWSAP=”3DRx”,”1234567890”\r\n

*   Change the UART baudrate

AT+UART=57600\r\n

*   Change the transmission power

AT+RF_PWR=81\r\n

The value has to be in the range of [0 82]

*   Change the IP of the Access Point and listening UDP port

AT+CIPPORT=”192.168.4.1”,8080\r\n

*   Test command

AT+TEST\r\n

This is a test command, when received the module sends the message “HELLO\r\n” through serial. This command is intended to detect the current baud rate of the module.


#### Saving the user configuration ####

The configuration of the above parameters is saved on the flash memory of the WiFi module.



