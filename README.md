# 8192-Baud-ALDL-for-Arduino
This is firmware for the Arduino Mega2560 or ESP32 Development Kit board to read the OBD1 GM 8192 Baud ALDL data stream from the 1227165 
ECM and retransmit over the usb serial port at 115,200 baud.
Designed to moditor data packets from the 1227165 ECM - 1986 to 1989 Camaro and Corvette L98
With some minor modification to the command codes, this code could be adaptable for other 8192 baud GM OBD1 ALDL data streams

Written by R.C. Stanley 08/16/2018
This software is licensed under the GNU3.0 General License. 
Use this software at your own risk as no warranties are expressed or implied.

Connections to the GM ALDL - 
1) Arduino Rx1 and DIO 4 pins are connected together to the ALDL data pin "E" on the ALDL connector
2) The Tx1 Arduino pin connects to the cathode of a 1n4001(or similar) diode with the anode connected to ALDL                                 pin "E".  This allows Tx to pull down pin "E" during transmission without interfering with the ECM serial                                 output.
3) Arduino GND is connected to ECM Gnd at ALDL pin "A"
4) A 10K resisor between ALDL pins "A" and "E" puts the ECM into ALDL mode. 

NOTE: In ALDL 10K mode, the ECM adds 10 degrees of ignition timing at idle - revise the ALDL adder in your PROM if you intend to drive in ALDL mode

On startup, the code expects the ECM to be transmitting in ALDL mode at 160 Baud.  Input pin 4 is used to listen for a silent period between the data packets.  The 8192 Baud Mode1 command is then sent from the Arduino to the ECM, switching to 8192 baud command mode.  Sometimes this takes a few seconds.  Once communications are in sync, the Arduino requests a MODE1 data packet from the ECM and the ECM responds with a preamble, 63 bytes of data, and a checksum.  The Arduino then re-transmits the data at 115200 baud over the built in usb serial port.  
Three selectable output options are provided:
1) Output data stream as HEX with the checksum and a timestamp.
2) Output specific data stream items, fully decoded and labeled.
3) Verbatim re-transmission of the data stream packet with preamble and checksum.
