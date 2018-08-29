
// MEGA2560 ALDLTAPr3 
// This sketch reads the ALDL for 8192 baud data on serial2 and retransmits it as hex at 115200 baud over Serial 0
// Designed to moditor data packets from the 1227165 ECM - 1986 to 1989 Camaro and Corvette L98
// With some minor modification to the command codes, this code should be adaptable for other 8192 baud GM OBD1 ALDL data streams
// Written by R.C. Stanley 08/16/2018
// This software is licensed under the GNU General License. The licensee is free to copy and modify this software.
// Use this software at your own risk no warranties are expressed or implied.
// R2 - loop to find preamble from ECM
// R3 - added silence detect and Mode 1 Command
// R4 - added checksum calculation -- Checksum Byte = 0x200 - Sum of all bytes transmitted including the Mode1 Command
// R5 - multiple output page options and comment updates to include description of circuit and $32, $32B and $6E data stream items
//
// Connections to the GM ALDL - 1)Arduino Rx1 and DIO 4 pins are connected together to the ALDL data pin "E" on the ALDL connector
//                              2)The Tx1 Arduino pin connects to the cathode of a 1n4001(or similar) diode with the anode connected to ALDL pin "E"
//                                This allows Tx to pull down pin "E" during transmission without interfering with the ECM serial output.
//                              3)Arduino GND is connected to ECM Gnd at ALDL pin "A"
//                              4)A 10K resisor between ALDL pins "A" and "E" puts the ECM into ALDL mode. 
// Note: In ALDL 10K mode, the ECM adds 10 degrees of ignition timing at idle - revise the ALDL adder in your PROM if you intend to drive in ALDL mode
//
/* 
 * The ECM starts up actively transmitting serial data a 160 baud in 4K mode.  After each transmit package, it waits 50msec to listen at 8192 baud for an incoming
 * command from the data terminal before starting another packet send.   If it sees the Mode 1 command sequence at 8192 baud it will switch into 8192 mode
 * and send a packet of data.  Then it will remain in 8192 mode and wait for the next command.
 * The following Data Stream definition was extracted from disassembly of the $6E code and should work for $32, $32B and $6E masks running on the 1227165 ECM.
 
         ;*****************************************************
          ;MODE 1 FIXED DATA DATA STREAM 
          ;ALDL DEV MUST REQUEST MODE 1 BY 
          ;XMITING THE FOLLOWING MSG TO 
          ;THE ECM: 
          ;
          ;MSG ID EQU 0x80 
          ;MSG LEN 1+0x55 EQU 0x56 
          ;MODE EQU 0x01 
          ;CKSUM EQU 0x29 
          ;
          ;THE ECM WILL RESPOND WITH: 
          ;
          ;MSG ID EQU 0x80 
          ;MSG LEN 64+85 EQU 0x95 
          ;MODE EQU 0x01 
          ;DATA BYTE 1 EQU 0xXX 
          ;. 
          ;. 
          ;DATA BYTE 63 EQU 0xXX 
          ;CKSUM EQU 0xcc  -- Last byte transmitted
          ;
          ;*****************************************************
    Byte 1, PROM ID MSB                         CONVERSION:  hex value
    Byte 2, PROM ID LSB                         CONVERSION:  hex value
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 3, Err Flg #1 
          ;
          ; Bit 0 EQU ERR 23 MAT SENSOR LOW 
          ; 1 EQU ERR 22 TPS LOW 
          ; 2 EQU ERR 21 TPS HIGH 
          ; 3 EQU ERR 16 NOT USED 
          ; 
          ; 4 EQU ERR 15 COOL SENSOR LOW TEMP. 
          ; 5 EQU ERR 14 COOL SENSOR HIGH TEMP. 
          ; 6 EQU ERR 13 O2 SENSOR 
          ; 7 EQU ERR 12 NO REF PULSES 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 4, Err Flg #2 
          ; 
          ; Bit 0 EQU ERR 35 NOT USED 
          ; 1 EQU ERR 34 MAF SENSOR LOW 
          ; 2 EQU ERR 33 MAF SENSOR HIGH 
          ; 3 EQU ERR 32 EGR DIAG. 
          ; 
          ; 4 EQU ERR 31 NOT USED 
          ; 5 EQU ERR 26 NOT USED 
          ; 6 EQU ERR 25 MAT SENSOR HIGH 
          ; 7 EQU ERR 24 VSS 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 5, Err Flg #3 
          ; 
          ; Bit 0 EQU ERR 51 PROM ERROR 
          ; 1 EQU ERR 46 VATS FAILED 
          ; 2 EQU ERR 45 O2 SENSOR RICH 
          ; 3 EQU ERR 44 O2 SENSOR LEAN 
          ; 
          ; 4 EQU ERR 43 ESC FAILURE 
          ; 5 EQU ERR 42 EST ERROR 
          ; 6 EQU ERR 41 CYL SELECT ERROR 
          ; 7 EQU ERR 36 BURNOFF DIAG. 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 6, Err Flg #4 
          ; 
          ; Bit 0 EQU ERR 63 NOT USED 
          ; 1 EQU ERR 62 NOT USED 
          ; 2 EQU ERR 61 NOT USED 
          ; 3 EQU ERR 56 NOT USED 
          ; 
          ; 4 EQU ERR 55 NOT USED 
          ; 5 EQU ERR 54 FUEL PUMP VOLTAGE 
          ; 6 EQU ERR 53 OVER VOLTAGE 
          ; 7 EQU ERR 52 CAL PAC MISSING 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 7, Err Flg #5 
          ; 
          ; Bit 0 EQU NOT USED 
          ; 1 EQU NOT USED 
          ; 2 EQU NOT USED 
          ; 3 EQU NOT USED 
          ; 
          ; 4 EQU NOT USED 
          ; 5 EQU ERR 66 NOT USED 
          ; 6 EQU ERR 65 NOT USED 
          ; 7 EQU ERR 64 NOT USED 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 8, Coolant, A/D                      CONVERSION: TEMP = value * 1.35 -40 in degrees C 
    Byte 9, Strt up Coolant, Deg C            CONVERSION: TEMP = value * 1.35 -40 in degrees C
    Byte 10, TPS, A/D                         CONVERSION: TPS% =  value*0.019608 + 0 percent open
    Byte 11, RPM, Var                         CONVERSION: RPM =  value*25.0 + 0 in revolutions per minute
    Byte 12, Ref Period                       CONVERSION: Time between Ref pulses = 16 bit unsigned value * 15.26 in microseconds
    Byte 13, LSB
    Byte 14, MPH Var for Disp.                CONVERSION: MPH = value in MPH (Range is 0 - 255)
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 15, Mode Word 3
          ; b0
          ; b1 VATS OK
          ; b2
          ; b3 error 51, chksum
          ; b4
          ; b5 VATS AREADY PASSED
          ; b6 error 36, MAF Burn off (B/O) PASSED
          ; b7 KNOCK ENBLED BY DIFF COOL
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 16, N/V Ratio                        CONVERSION: NV =  value
    Byte 17, o2 Filtered                      CONVERSION: O2 sensor = value * 4.44 in millivolts
    Byte 18, ALDL Rich/Lean Cross Counts      CONVERSION: value = 0 - 255 cross counts - rolls over to 0
    Byte 19, Base PW Correction               CONVERSION: UNKNOWN
    Byte 20, BLM base fuel pw correction      CONVERSION: BLM = 8 bit value  >128 is adding fuel, <128 is taking it out (Long Term Adjustment)
    Byte 21, BLM Cell Number                  CONVERSION: value = current BLM cell number being used (There are 16 cells)
    Byte 22, INT Clsd/Lp Int.                 CONVERSION: Integrater = 8 bit value  ( Below 128 subtracts fuel over 128 adds fuel) (current fast adjustment)
    Byte 23, IAC Present Posit.               CONVERSION: value = Idle Air Control motor steps (Max value is limited to 160 steps for L98)
    Byte 24, IAC Step Dir Cmd                 CONVERSION: value = Idle Air Control motor direction (0 to open or 255 to close) 
    Byte 25, Requested Idle Spd.              CONVERSION: Desired Idle RPM = value * 12.50 in Revs per Minute
    Byte 26, Filtered Ld Val                  CONVERSION: LV* = value  Load variable 8 range 0 - 255
    Byte 27, A/D test channel
    Byte 28, Raw Ld Val                       CONVERSION: LV* = value  Load variable 8 range 0 - 255
    Byte 29, MAT Val                     ***  CONVERSION: TEMP = Interpolated from a lookup table based on value (see example below)
    Byte 30, MAT Val, A/D                ***  CONVERSION: TEMP = Interpolated from a lookup table based on value (see example below)
    Byte 31, EGR D.C.
    Byte 32, Can Prg D.C.                     CONVERSION: Charcoal Cannister PWM Duty cycle pct = value *0.398406
    Byte 33, Fan D.C.                         CONVERSION: Cooling Fan Duty cycle pct = value *0.398406
    Byte 34, Batt Volts, A/D                  CONVERSION: Volts = value * 0.10 +0 in Volts 
    Byte 35, Pump VDC                         CONVERSION: Volts = value * 0.10 +0 in Volts 
    Byte 36, Mass Air Flow Gms/Sec, MSB       CONVERSION: MAF = 16 bit unsigned value * 0.003906 +0 in Grams/Sec 
    Byte 37, LSB
    Byte 38, RAW AIRFLOW, (MAF Diag)          CONVERSION: MAF = 8 bit value * 0.003906 +0 in Grams/Sec
    Byte 39, Tot Sprk Adv rel to TDC          CONVERSION: Spark Advance from TDC = 16 bit unsigned value * 0.351563 in degrees
    Byte 40, LSB
    Byte 41, Sprk Adv rel to ref pulse MSB    CONVERSION: Spark Advance from REF = 16 bit unsigned value * 0.351563 in degrees
    Byte 42, LSB
    Byte 43, ESC Knock Cnt'r                  CONVERSION: value = 0 - 255 counts - rolls over to 0
    Byte 44, Knock Retard                     CONVERSION: Knock retard = value * 0.10  (amount of spark retard in degrees during knock event)
    Byte 45, Base PW, Last Inject MSB         CONVERSION: PW= 16 bit unsigned value * .015259 (last injection pulse in milliseconds)
    Byte 46, LSB
    Byte 47, Total Air/Fuel Val MSB           CONVERSION: TARGET A/F = 6553.6/(16 bit unsigned value) displayed as ratio :1
    Byte 48, LSB
    Byte 49, Run tot Fuel Consumed MSB        CONVERSION: Unknown
    Byte 50, LSB
    Byte 51, Run tot Dist Run, (.0005mi/bit)  CONVERSION: Total Distance Run in Miles = value * 0.0005
    Byte 52, Eng Run Time, Sec                CONVERSION: RUNTIME = 16 bit unsigned value of Engine Run time in Seconds
    Byte 53, LSB
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 54, Minor Lp mode word #2 
          ; 
          ; Bit 0 EQU 1EQU OVERDRIVE ON 
          ; 0EQU OVERDRIVE OFF 
          ; 1 EQU ERR 14 or 15 THIS STARTUP 
          ; 2 EQU REF PULSES OCCURRED (6.25 MSEC CHECK) 
          ; 3 EQU 1EQU ALDL MODE, 8192 LOCKED IN, & MODE 4 
          ; 
          ; 4 EQU DIAGNOSTIC SWITCH IN DIAGNOSTIC POSITION 
          ; 5 EQU DIAGNOSTIC SWITCH IN ALDL POSITION 
          ; 6 EQU HIGH BAT. VOLT. , DISABLE SOLENOID DISCRETS 
          ; 7 EQU SHIFT LIGHT (1EQU ON, 0EQU OFF) 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 55, MPU Status wd, (Fan, P.S. etc) 
          ; 
          ; Bit 0 EQU PARK/NEUTRAL MODE 
          ; 1 EQU NOT IN THIRD GEAR 
          ; 2 EQU OVERDRIVE REQUEST 
          ; 3 EQU EXCESSIVE POWER STEER PRES. 
          ; , CLUTCH ANTICIPTE 
          ; 
          ; 4 EQU EGR DIAGNOSTIC SWITCH CLOSED 
          ; 5 EQU TCC LOCKED 
          ; 6 EQU FAN REQUEST BIT 
          ; 7 EQU 0 EQU A/C REQUEST 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 56, Mode wd 
          ; 
          ; Bit 0 EQU PARK/NEUTRAL (0 EQU DRIVE) 
          ; 1 EQU 3rd GEAR 
          ; 2 EQU 4th GEAR 
          ; 3 EQU POWER STEERING ( 1 EQU CRAMP) 
          ; 
          ; 4 EQU READ BUT NOT USED 
          ; 5 EQU EGR DIAGNOSTIC 
          ; 6 EQU FAN REQUEST <EQU > ACHP (A/C HI PRESSURE) 
          ; 7 EQU AIR CONDITIONER ( 0 EQU A/CREQUESTED) 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 57, Minor Lp wd #1 
          ; 
          ; Bit 0 EQU ADVANCE FLAG ( 0EQU ADV. , 1EQU RTD) 
          ; 1 EQU CHK ENGINE LIGHT DELAY FLAG 
          ; 2 EQU INTERRUPT SERVICE EXECUTION EXCEED 6.25 MSEC 
          ; 3 EQU FAN ON DISABLED BY PID 
          ; 
          ; 4 EQU TCC ROAD SPEED 1st PULSE FLAG 
          ; 5 EQU A/C CLUTCH FLAG ( 0EQU A/C CLUTCH ON) 
          ; 6 EQU BYPASS CHECK ENABLE 
          ; 7 EQU ENGINE RUNNING FLAG ( 1EQU RUNNING) 
          ; 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 58, Non Vol Mem Mode wd 
          ; 
          ; Bit 0 EQU O2 SENSOR READY 
          ; 1 EQU CLOSED LOOP TIMER TIMED OUT 
          ; 2 EQU NOT USED 
          ; 3 EQU IMPROPER SHUTDOWN 
          ; 
          ; 4 EQU NOT USED 
          ; 5 EQU IAC KICKDOWN ENABLED 
          ; 6 EQU KWARM KICKDOWN ENABLED 
          ; 7 EQU ERR 42 FAILED 
          ; 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 59, MANUAL Xmission Mode wd 
          ; 
          ; Bit 0 EQU OVERDRIVE ON 
          ; 1 EQU 1st GEAR DIRECT 
          ; 2 EQU 1st GEAR LOOK­AHEAD OK 
          ; 3 EQU 4th GEAR 
          ; 
          ; 4 EQU 1st GEAR OVERDRIVE 
          ; 5 EQU DOWNSHIFT/OFF REQUEST 
          ; 6 EQU UPSHIFT/ON REQUEST 
          ; 7 EQU OVERDRIVE ACTIVE 
          ; 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 60, ALCL Mode Wd. 
          ; 
          ; Bit 0 EQU ALDL RESET 
          ; 1 EQU NOT USED 
          ; 2 EQU FIELD SERVICE MODE 
          ; 3 EQU FIRST C/L PASS THRU FIELD SERVICE MODE DONE 
          ; (1EQU DONE) 
          ; 
          ; 4 EQU FIELD SERVICE. O2 TRANSITION 
          ; 5 EQU ONE SECOND FLAG (SYMMETRICAL) 
          ; 6 EQU 200 MSEC TOGGLE BIT FOR 2.5 HZ FSM FLASH RATE 
          ; 7 EQU PULLUP RESISTOR FOR COOL 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 61, Mjr Lp mode wd 
          ; 
          ; Bit 0 EQU 100 MSEC, OLD CCP PURGE ON FLAG (0EQU OFF) 
          ; 1 EQU AIR CONTROLLED, 0EQU AIR DIVERTED 
          ; 2 EQU AIR SWITCHED TO PORT 
          ; 3 EQU NOT USED 
          ; 
          ; 4 EQU SKIP BURNOFF DUE TO > 17 VOLTS THIS STARTUP 
          ; 5 EQU D.E. QSEC 
          ; 6 EQU BURN OFF AIR METER 
          ; 7 EQU DECEL ENLEANMENT 
          ; 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 62, Serial Data Mode Word 
          ; 
          ; Bit 0 EQU EXPECTING FIRST 160 BAUD INTERRUPT 
          ; 1 EQU EXPECTING SECOND 160 BAUD INTERRUPT 
          ; 2 EQU IN 8192 MODE 
          ; 3 EQU LOCKED IN 8192 MODE 
          ; 
          ; 4 EQU NOT USED 
          ; 5 EQU NOT USED 
          ; 6 EQU NOT USED 
          ; 7 EQU NOT USED 
          ; 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
    Byte 63, Flg Word Fuel/Air Mode word 
          ; 
          ; Bit 0 EQU NOT USED 
          ; 1 EQU LEARN CONTROL FLAG 
          ; (1EQU ENABLE STORE, 0EQU DISABLE) 
          ; 2 EQU NOT USED 
          ; 3 EQU NOT USED 
          ; 
          ; 4 EQU VSS FAILURE 
          ; 5 EQU EECC SLOW O2 RICH/LEAN FLAG 
          ; 6 EQU RICH/LEAN FLAG (1EQU RICH , 0EQU LEAN) 
          ; 7 EQU CLOSED LOOP FLAG (1EQU C/L , 0EQU O/L) 
          ;­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­­ 
          ;
          ;*****************************************************
    Byte 64 - Twos complement checksum of all bytes transmitted, including the preamble
 
 */

// ***** Variable declariations
const byte ALDLTestPin =4;                                                // Input used to listen for 160 BAUD ALDL activity befor commanding 8192 BAUD
const byte DecodeDataOutputPin =5;                                        // Setting this input pin LOW will put decoded data out on the serial port
const byte HexDataOutputPin =6;                                           // Setting this input LOW will output hex data on the serial port
int ALDLbyte = 0;                                                         // One byte of aldl data
int bytecounter =0;                                                       // count bytes for line spacing in output
const int linecount = 64;                                                 // 32 bytes per line
byte M1Cmd[4] = {0x80,0x56,0x01,0x29};                                    // Mode 1 command to start 8192 Mode 1 DataStream (80 56 01 29 HEX)
byte Preamble[3] = {0x80,0x95,0x01};                                      // Preamble from ECM indicates start of 8192 Mode 1 DataStream (80 95 01 HEX)
bool PreambleFound = false;                                               // Reset preamble found flag
bool SilenceFound = false;                                                // Flag to indicate that ECM silence in 160 baud mode has been found
bool CommInit8192 = false;                                                // Flag indicates that 8192 baud communications has been initialized
const double SilenceWait = 15000;                                         // Minimum silence period before transmitting a Mode 1 command
unsigned long PreambleTimer;                                              // Timeout timer for preamble
const int ByteCount = 64;                                                 // Number of data bytes following preamble in data stream including checksum
byte DataBytes[ByteCount];                                                // Array to hold the serial data stream
int DataStreamIndex = 1;                                                  // Data stream byte Index, start at 1 and end at 63 to match Assembly listing
int i = 0;                                                                // Preamble index counter
unsigned long StartTime;                                                  // Microsecond counter for measuring the time of incoming data
unsigned int  CheckTotal;                                                 // Total of bytes recieved for calculating the checksum 
byte CheckSum;                                                            // Actual calculated Checksum

// **** Variables specifically for the decoded data stream 

float RPM;                                                                // Engine RPM
float TPS;                                                                // Percent TPS
float MAF;                                                                // Mass Air Flow gm/sec
float InjPW;                                                              // Injector Pulse Width
float O2mv;                                                               // O2 sensor MV
int BLCELL;                                                               // Currently used block learn cell #. There are 16 cells organized bye RPM and air flow 0, is idle, 15 is max RPM & Airflow
int BLM;                                                                  // Block Learn Value for cell being currently used
int INTEGRATOR;                                                           // Current fueling control integrator value over 128 indicates fuel is added, under 128 fuel is taken out, 160 max
float MAT;                                                                // Intake Manifold Air Temperature
unsigned int Runtime;                                                     // Engine Run time

// ***** Hardware configuration
//HardwareSerial Serial1(2);                                              // Define uart2 as Serial1 - **** USED ONLY ON ESP32 *****

void setup() 
{
// **** I/O configuration and setup first
pinMode(ALDLTestPin,INPUT);                                               // define D4 as an input pin to listen for the 160 baud input data
pinMode(DecodeDataOutputPin,INPUT_PULLUP);                                // User convenience pin.  Grounding this pin will send Decoded Data to the Serial Port
pinMode(HexDataOutputPin,INPUT_PULLUP);                                   // User convenience pin.  Grounding this pin will send HEX Data to the Serial Port
// **** Now, start the serial functions
Serial.begin(115200);                                                     // Open serial monitoring port
Serial1.begin(8192);                                                      // Test the capability of esp 8222 to run at 8192 baud directly
delay(1500);                                                              // delay for diagnostic print
Serial.println("Ready for data capture");

// **** Initialize the variables, flags, etc

i=0;                                                                      // Reset the preamble index flag
}

void loop() {
                                                            
// Wait for silence period on the ALDL
Serial.print("wait for silence ");
SilenceFound = false;                                                     // Reset silence flag
StartTime= micros();                                                      // First look for an active signal or a timeout - intialize timer
                                                                          // Should exit this loop on the start of a bit
while ((micros() - StartTime)< 15000)                                     // Wait for a 15 ms silent period 
  {
    if (digitalRead(4)== 0)                                               // Any line activity resets the start time
     {
       StartTime= micros();                                               // Timing starts over
     }
  }                                                             
SilenceFound = true;                                                      // Set the silence flag on exit

while (SilenceFound == true)                                              // While silence found flag is set, continuously request and transmit Mode 1 data 
  {                                                                       // Now send a Mode 1 dump command
   PreambleFound = false;                                                 // Reset preamble found flag                                                             
 
   while (PreambleFound == false)                                         // First look at data until the preamble has been found will read data forever until a preamble is read
   {                                                                      // Loop through this while preamble has not been found
    Serial.print(" M1 cmd ");
    i=0;                                                                  // use bytecounter to send the outgoing Mode1CMD sequence
    while (i<4)
    {
      Serial1.write(M1Cmd[i]);                                            // sends 1 byte of the command sequence
      i++;
    }  
 
    Serial.println(" Finding Preamble  ");  
    i=0;                                                                  // Then reset byte counter and scan incoming data for the preamble
    PreambleTimer = millis();                                             // Initialize timer to detect timeout
    while ((((millis() - PreambleTimer) < 100)) && (PreambleFound == false))                    // First look at data for 100 ms or until the preamble has been found will read data forever until a preamble is read
      {
        if (Serial1.available() > 0)                                      // Check for available data on the serial port
        {
          ALDLbyte = Serial1.read();                                      // Read it and look for the preamble
          if ( ALDLbyte == Preamble[i])                                   // Look for matching byte of data preamble in serial stream 
            {
              i++;                                                        // Increment the preamble index and look for the next character
              if (i>2)PreambleFound = true;                               // Once three characters are found, the preamble has been found, time to read in the data
            }        
          else                                                            // If there isn't tch, start over looking from the beginning
            {
              PreambleFound = false;                                      // Reset preamble found flag
              i=0;                                                        // Reset the preamble index flag          
            }                                                             // End of Preamble check
          }                                                               // End of Serial Available & read
       }                                                                  // End of the preamble finding routine either preamble found or timeout          
    }
                                                                          // While loop only exits after a valid preamble is found - *** need to add a timeout option here later
                                                                          // READ A COMPLETE DATA STREAM PACKET
    DataStreamIndex = 1;                                                  // Once a valid preamble has been found set the data stream index to the first byte
    while (DataStreamIndex < 65)                                          // and read data up to byte #63 + 1 byte Checksum = 64 Bytes total
      { 
          if (Serial1.available() > 0)                                    // Check for available data on the serial port
            {
              DataBytes[DataStreamIndex] = Serial1.read();                // And read bytes into the array as they come
              DataStreamIndex++;                                          // update the index
            }                                                             // end of read if
      }                                                                   // End of datastream read while - reads 63 bytes of data and skips the checksum byte for new...
    
    // Checksum Calculation  - Checksum Byte = 0x200 - Sum of all bytes transmitted including the Preamble
    i=1;                                                                  // use bytecounter as an index
    CheckTotal = 0x80+0x95+0x01;                                          // First start by summing the preamble bytes
    while (i< (ByteCount ))                                               // Add recived bytes to the Checksum
      {                                                                   // except the Checksum byte ittself
        CheckTotal = CheckTotal + DataBytes[i];                           // add a byte
        i++;
      }   
    CheckSum = 0x200 - CheckTotal;                                        // Two's complement the checksum and cast it to a byte
    // once the data stream has been read, resend it to the the outgoing serial port
    if (digitalRead(DecodeDataOutputPin) == LOW)                          // Check decoded output bit - if it's low send some select decoded data to the serial port
      {                                                                   // this output mode is slowed by some inserted waiting time to allow user to see the data
                                                                          // and is included here primarily to show how to use the conversions 
        Serial.print("New Data Stream received at ");
        Serial.print(millis());
        Serial.print(" Calc CHECKSUM: ");
        Serial.print(CheckSum, HEX);
        Serial.print(" Transmitted CHECKSUM: ");
        Serial.print(DataBytes[ByteCount], HEX);                          //Last byte (64'th) transmitted is ECM's checksum
        if (CheckSum == DataBytes[ByteCount])                             // Verify Checksums match and print good data or error message
          {
            Serial.println(" Checksum GOOD - Decoded Data as follows:   (Page 1) "); 
          }
        else 
          {
            Serial.println(" Checksum *** ERROR *** -  Decoded Data as follows:   (Page 1) ");                  
          }

             RPM =  DataBytes[11]* 25;                                    // Engine RPM
             TPS = DataBytes[10]* 0.019608;                               // TPS volts  ***Verified
             MAF= ((DataBytes[36]*256) + (DataBytes[37]))*0.003906;         // Mass Air Flow gm/sec
             BLCELL = DataBytes[21];                                      // Block Learn Cell # being used
             BLM = DataBytes[20];                                         // Long Term Fuel correction (value of current Block Learn Cell)
             INTEGRATOR = DataBytes[22];                                  // Currently calculated fuel trim number
             InjPW = ( (DataBytes[45]*256) +  ( DataBytes[46]))* .015259 ;     // Injector Pulse Width in Msec
             O2mv = DataBytes[17] *4.44;                                  // O2 sensor MV ***Verified
             MAT = T_interpolate(DataBytes[30]);                          // Intake Manifold Air Temperature
             Runtime = (DataBytes[52]*256 + DataBytes[53]);               // Engine Run time (seconds)
                                                                          // Now burst print the page of calculated data
             Serial.print ("Engine Speed     : ");
             Serial.print(RPM);
             Serial.println (" RPM");
             Serial.print ("Throttle Position: ");
             Serial.print(TPS);
             Serial.println (" Volts");                                   //Note percent TPS could be calculated and displayted instead using max and min voltage          
             Serial.print ("Mass Air Flow    : ");
             Serial.print(MAF);
             Serial.println (" Grams/Sec");       
             Serial.print("Current BLM Cell: ");
             Serial.print(BLCELL);
             Serial.print(" BLM Value: ");
             Serial.print(BLM);
             Serial.print("  Current Fuel Integrator: ");
             Serial.println(INTEGRATOR);  
             Serial.print ("Injector Pulse   : ");
             Serial.print(InjPW);
             Serial.println (" Milliseconds"); 
             Serial.print ("O2 Sensor Voltage: ");
             Serial.print(O2mv);
             Serial.println (" Millivolts");       
             Serial.print ("Intake Air Temp  : ");
             Serial.print(MAT);
             Serial.println (" Deg C");     
             Serial.print ("Engine Run Time  : ");
             Serial.print(Runtime);
             Serial.println (" Seconds"); 
      // The following code implements a delay with an incoming serial buffer purge to allow it to run alongside other data captures such as TunerPro - Artduino Tx1 is disconnected
      // Not keeping the incoming serial buffer empty before the next command and read loop causes a parity error
            unsigned long StartTime = millis();                           // Get the starting time value
            while (millis() < StartTime +3000)                            // Wait for 3000 milliseconds - allows user to read the displayed data
              {
              if (Serial1.available() > 0) ALDLbyte = Serial1.read();     // Check for available data on the serial port and throw it away - i.e. flush the buffer
              }  
      }
    else if (digitalRead(HexDataOutputPin) == LOW)                        // Check hex output flag 
      {                                                                   // and send Hex output of the data stream to the Serial Port - This mode can run at 10hz
      
        Serial.print("New Data Stream received at ");
        Serial.print(millis());
        Serial.print(" Calc CHECKSUM: ");
        Serial.print(CheckSum, HEX);
        Serial.print(" Transmitted CHECKSUM: ");
        Serial.print(DataBytes[ByteCount], HEX);                          //Last byte (64'th) transmitted is ECM's checksum
        if (CheckSum == DataBytes[ByteCount])                             // Verify Checksums match and print good data or error messag
          {
            Serial.println(" Checksum GOOD - Data as follows: ");      
          }
        else Serial.println("Checksum *** ERROR *** -  Data as follows: ");
    
        int j=1;// Local Byte Count for message output  
        bytecounter = 0;                                                  // Used to create 32 byte lines of data
        while (j<ByteCount +1)                                            // Printing loop from byte 1 to bytecount
          {
            Serial.print(DataBytes[j], HEX);                              // print the aldl data byte as hex
            j++;                                                          // Move on to the next byte
            bytecounter++;                                                // Increment byte counter 
            if (bytecounter >= linecount)                                 // check if time for new line
              {
                bytecounter = 0;                                          // Reset byte count for next line
                Serial.println("");                                       // add a new line
              }
            else
              {
                Serial.print(" ");                                        // add a space
              }
          }
        Serial.println("");                                               // New Line At end of transmit, loop back for the next data stream capture
      }                                                                   // End of Hex Output Loop
    else                                                                  // Default is to send the raw binary data stream to the Serial Port
      {                                                                   // Send Raw Bytewise Data Stream to Serial Port and get another group of data 


       Serial.write(0x80);                                                // write the preamble bytes (80 95 01 HEX)
       Serial.write(0x95);                                                     
       Serial.write(0x01);                                                    
       for (int j=1;(j<ByteCount +1); j++)                                // Printing loop from data byte 1 to bytecount, including checksum
          {
            Serial.write(DataBytes[j]);                                   // write the aldl data byte as a binary byte
          }
       }                                                                  // End of Raw Data Stream transmission
    }                                                                     // Loop back for next Mode 1 Dump
}

float T_interpolate(byte DS_Temp)                                         // Subroutine to interpolate MAT temperature from data stream
{                                                                         // Input data is one byte of AD from the temperature thermistor (Very Non Linear)
//Temperature scale for interpolating air temperature taken from the ADS file There are 38 values in the table ranging from -40C for a value of 1 to 200 C for a value of 256
  const float TempScale[38] = {1,5,6,9,11,15,19,25,31,38,47,57,67,79,91,104,117,130,142,154,164,175,184,192,200,206,212,217,222,226,230,233,235,238,240,242,244,256};  // Data Values (38)
  const float TempValue[38] = {-40,-30,-25,-20,-15,-10,-5,0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,105,110,115,120,125,130,135,140,145,200}; // Temp Values (38)
  float T_Range;                                                          // Interpolation range
  float T_Diff;                                                           // Difference between input value and the bottom of the interpolation range
  float TempRange;                                                        // Difference between points on the output value scale
  float Temperature;                                                      // Interpolated Temperature value
  int i = 0;
  while (i<38)                                                            // Loop through increasing values to find start of the interpolation
    {
      if  (TempScale[i]> DS_Temp) break;                                  // Until a higher value is found - exit with i pointing to the higher value for interpolation
      i++;
    }
  if (i>0)                                                                // Figure out the linear interpolation range and difference along the scale
    {
      T_Range = TempScale[i] - TempScale[i-1];                            // Range between these points along the input scale (all calculated as a floating point)
      T_Diff =  DS_Temp - TempScale[i-1];                                 // Difference between data and the lower point
      TempRange = TempValue[i] - TempValue[i-1];                          // Difference between points along the output value scale  
      Temperature =  TempValue[i-1] + (T_Diff/T_Range)*TempRange;         // Interpolated Temperature
    }
  else Temperature = TempValue[0];                                        // unless the input date is <= to the bottom of the scale and the minimum value is used

  return Temperature;                                                     // Return the interpolated temperature
                                               


}
 
