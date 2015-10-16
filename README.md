## Code for a Telemetry WiFi module based on Espressif ESP8266 ##

This repository contains the code for a telemetry bridge on a WiFi module. The device used is the Espressif ESP8266. The used SDK version of the WiFi module is [1.3.0_15_08_08] (http://bbs.espressif.com/viewtopic.php?f=46&t=919). The code was develop on the Eclipse IDE using the [unofficial SDK] (http://www.esp8266.com/viewtopic.php?t=820). 


The main code is located on the user folder of the "APTest_v2" project.

### Description ###

The applications consist on one user task that manages the reception and transmision of MAVLink frames, and reception of commands. 

For the MAVLink frames, the application acts as a bridge (UART - WiFi). 

For commands, received through WiFi or Serial UART, the application decodes them and execute the corresponding actions. 

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

AT+CWSAP="3DRx","1234567890"\r\n

*   Change the UART baudrate

AT+UART=57600\r\n

*   Change the transmission power

AT+RF_PWR=81\r\n

The value has to be in the range of [0 82]

*   Change the IP of the Access Point and listening UDP port

AT+CIPPORT="192.168.4.1",8080\r\n

*   Test command

AT+TEST\r\n

This is a test command, when received the module sends the message “HELLO\r\n" through serial. This command is intended to detect the current baud rate of the module.


#### Saving the user configuration ####

The configuration of the above parameters is saved on the flash memory of the WiFi module.


#### Configuring the WiFi module ####

You can configure the WiFi module using the ResetAPSettings.jar or the NetBeans project "ResetAPSettings_nex". 

On this application you can:
*   Change the Network name and password (through WiFi or Serial)
*   Change the IP address and listening port (through Serial)
*   Change the Baud rate (through WiFi)
*   Upload a new Firmware (through Serial)

For the options that communicate with the WiFi module through Serial you need [python] (https://www.python.org/) and [pyserial] (https://pypi.python.org/pypi/pyserial).


NOTE: When changing the IP address of the WiFi module, you have to disconnect and then reconnect your computer to its network to allow the Operating System to ask for a new IP.



