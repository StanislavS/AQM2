//********************************//
//*** THE MAIN PROGRAMM MODULE ***//
//------> "net.h"
//------> "sensors.h"
//********************************//
#include <Wire.h>

#define BLUE_LED 2                  //*** Blue Led ESP-12
#define WIFIACC  10                 //*** numbers of tries WiFi access
#define TLSTEP 100                  //*** time step for Time Line script working with sensors millisec
#define TLLIMIT 600                //*** Time Line limit for one sensors session in numbers of time steps, TLSTEP x TLLIMIT = Time Line in millisec

ADC_MODE(ADC_VCC);        //*** INTERNAL VOLTMETER ON
//float Vdd;                //*** CURRENT NODEMCU ESP8266 POWER VOLTAGE 

//volatile uint32_t tm = millis();       //*** TIMER for BUTTON interrupts functions
uint32_t tdc = millis();      //*** TIMER for START AP MODE
uint32_t tp = millis();       //*** TIMER for BUTTON bounce trouble ui.n/handleButton()

byte click = 0;                   //*** USING IN ui.n/handleButton()
byte button_state = 0;            //*** USING IN ui.n/handleButton()

uint16_t tlCur = 0;           //*** TIMELINE CURRENT POSITION
uint32_t tlTime = millis();   //*** TimeLine timer on TLSTEP ms
uint32_t tlCilce = 0;         //*** Number of measures, TimeLine cicles

#include "net.h"

#include "PMS.h"              //*** PMS7003 DUST SENSOR LIBRARY
PMS pms(Serial);              //*** PMS7003 DUST SENSOR ON HARDWARE SERIAL UART (pin1, pin3)
PMS::DATA PMSdata;            //*** PMS7003 DUST SENSOR data structure

#include <SoftwareSerial.h>   //*** Hardware Serial - PMS7003
SoftwareSerial SW3(14, 12);   //*** Software Serial MH-Z19
SoftwareSerial SW2(13, 15);   //*** Software Serial NEO-6M

#include <Adafruit_ADS1015.h> //*** ADS1015 ANALOG MULTIPLEXER 4 CHANNELS LIBRARY
Adafruit_ADS1015 ads1(0x48);  //*** ADS1015 ANALOG MULTIPLEXER 4 CHANNELS 1 MODULE ADDRESS
Adafruit_ADS1015 ads2(0x49);  //*** ADS1015 ANALOG MULTIPLEXER 4 CHANNELS 2 MODULE ADDRESS

#include <Adafruit_BME280.h>  //*** BME280 TEMPERATURE, HUMIDITY, PRESSURE SENSOR LIBRARY
Adafruit_BME280 bme;          //*** BME280 TEMPERATURE, HUMIDITY, PRESSURE SENSOR

#include "sensors.h"

//*********************//
//***   S T A R T   ***//
//*********************//
void setup() {
  
  pinMode(BLUE_LED, OUTPUT);
  digitalWrite(BLUE_LED, HIGH);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  ssd1306();           //*** OLED initialisation

  Serial.begin(9600);  //*** HARDWARE SERIAL UART for PMS7003 DUST MODULE 
  SW2.begin(9600);     //*** SOFTWARE SERIAL UART for NEO-6M GPS MODULE
  SW3.begin(9600);     //*** SOFtWARE SERIAL UART for MH-Z19 CO2 MODULE
   
  ads1.begin();        //*** ADS1015 ANALOG MULTIPLEXER 4 CHANNELS
  ads2.begin();        //*** ADS1015 ANALOG MULTIPLEXER 4 CHANNELS
  
//  bme.begin();
  if (!bme.begin()) {  
  oledPrint(0, 16 , "BME sensor NOT FOUND!"); 
  }
  
  pms.passiveMode();            //*** PMS7003 Switch to passive mode
  
  EEPROM.begin(EEPROMSIZE);     //*** EEPROM 4096 bytes
  
  conf = eepReadConfig();             //*** READING CONFIGURATION STRUCTURE FROM EEPROM
  co = eepReadConstante();            //*** READING CONSTANTES STRUCTURE FROM EEPROM
  recordsnumber = eepReadDataRec();   //*** READING CURRENT RECORD NUMBER OF DATA STRUCTURE FROM EEPROM

  oledPrint(30,  0, "H E L L O ! ");  //*** HELLO OLED TEST
  delay(2000);
//***************************************************** POWER TEST
  PowerTest();
  delay(2000);
//*****************************************************************************************
    
//****************************************************** NUMBER OF DATA RECORDS IN EEPROM  
  oledPrint(0, 8, "===== Data rec: " + String(recordsnumber)); 
  delay(2000);

//****************************************************** WIFI INIT AND CONNECTION
  WiFi.mode( WIFI_OFF );
  delay(1000);  
  WiFi.mode(WIFI_AP_STA);

  if (wifi()) {    //*** TRY WIFI CONNECTION USING SSID AND PASSWORD FROM EEPROM
      APsite();   //*** IF NO WIFI - GO TO AP WEBSITE);
  }
  else {
      ota();       //*** IF WE HAVE WIFI - GO TO OTA
  }   

  if (wifiACC && recordsnumber)                       //*** IF WE HAVE DATA RECORDS IN EEPROM
     while (recordsnumber > 0) {                      //*** DATA TRANSFER ALL RECORDS  
            uint8_t ret = dataTransfer();             //*** DATA TRANSFER TO DATA SERVER
            if (ret == 1) {                           //*** IF DATA SENT SUCCSESS
                recordsnumber--;                      //*** -> DECREASE RECORDS NUMBER AND WRITE NEW FIGURE IN EEPROM
                eepWriteDataRec(recordsnumber);
                delay(200);
            }    
            else 
                break;        
     }
  oledPrint(0, 8, "                   ");
  oledPrint(0, 24, "CLICK for AP MODE");
  tdc = millis();  //*** TIMER FOR CLICK FOR AP MODE START  
}

//*********************//
//***    L O O P    ***//
//*********************//
void loop() {

  if (APmode) {                       //*** Handle of AP WEBsite
        server.handleClient();
        return;
  }

//******************************************** TEST IF BUTTON PRESSING
   
  if (APstart) {                        //*** WAIT CLICK DURING FIRST 5 SECUNDS FOR GO TO AP MODE
     while (millis() - tdc < 5000) {
          click = handleButton();       //*** IF CLICK -> JUMP FROM WHILE CICLE
          if (click) break;
          delay(0);
     }
     APstart = false;                            //*** CLOSE WAITING
     oledPrint(0, 24, "                  ");     //*** CLEARING STRING

     if (click) {                       //*** IF BUTTON CLICK DURING FIRST 5 SEC - START AP 
        APsite();                       //*** GO TO AP WEBSITE);
     }
  }
  else {                                //*** IF BUTTON CLICK DURING SESSION (EXCLUDING FIRST 5 SEC
        click = handleButton();   
        if (click == PUSH) {            //*** IF BUTTON PUSHED (CLICK MORE 1.5 SEC)
            gpsready = true;            //*** GO TO GPS MODULE INFORMATION
            geoTimer = millis();
         }
  }
  

//************************************************************ Calculation of radioactive particles crossing over SBM-20
  if (sbm20ready && rcount > 0 ) {            
      int16_t adc0 = ads2.readADC_SingleEnded(RAD_PIN);  //*** READ ADC FROM RAD_PIN ON THE ADS1015 MODULE
      if (adc0 > ADCLEVEL) {                             //*** IF ADC MORE ADCLEVEL (2.55v) => IONPARTICLE FIXED           
          rcount++;
          oledPrint(0, 56, "                      ");
          oledPrint(0, 56, "RAD:" + String(rcount) );
      }

    delay(co.RADdelay);                                   //*** DELAY MILLISEC FOR FIXED SBM-20 MEASURING, SENSOR CONSTANTE
  }


  if (tlCur >= SOUNDDETECTOR && !sbm20ready) {            //*** CALCULATE SOUND LEVEL
     handleSound();   
  }

     if ( (tlCur == co.RADtime * 10 + RADIATIONSTART + 1) && sbm20ready) handleSBM20();      //*** CLOSING RADIATION MEASURING, IF TIME MEASURING (RADtime SENSOR CONSTANTE) ENDED
     
//*************************************************** TimeLine      
     switch(tlCur) {
      
         case BME280:
                        if (bme280ready) handleBME();
                        break;  

         case PMSWAKEUP:
                        pms.wakeUp();
                        break;

         case PMSREAD:
                        pms.requestRead();
                        break;

         case PMSREAD + 10:
//                        for (int i = 0; i < 10; i++ )
//                             if (handlePMS()) i = 10;
                        if (pmsready) handlePMS();
                             
                        break;

         case RADIATIONSTART:
                        rcount = 1;
                        break;
      
         case MICS6814:
                        if (mics6814ready) handleMICS6814();
                        break;

         case MQ131:
                        if (mq131ready) handleMQ131();
                        break;

         case SO2SH12:
                        if (so2sh12ready) handle2SH12();
                        break;     

         case MHZ19:
                        if (mhZ19ready) handleMHZ19();
                        break;     

    } // SWITCH

    if (millis() - tlTime > TLSTEP && !gpsready) {                //*** CHECK TIME LINE CURRENT POSITION
      tlTime = millis();
      tlCur++;
      if (tlCur >= TLLIMIT ) {                                    //*** IF TIME LINE CURRENT POSITION = TLLIMIT -> TEST GPS MODULE
          tlCur = 0;
          gpsready = true;
          geoTimer = millis();
          gpsTr = 0;
          tlCilce++;
          
        }
      }


  if (gpsready) {
    uint8_t ret = gpsInfo();
    if (ret == 0) {                                                 //*** IF WE GOT GPS DATA -> WRITE DATA FROM ALL SENSORS IN TO THE EEPROM
      eepWriteData(dat);
      oledPrint(0,  0, "                   "); 
      oledPrint(0,  0, "DATA RECORDED-" + String(recordsnumber)); 
      gpsready = false;
      sensorsReset();                                                //*** RESET SENSORS FOR NEW MEASURING
      }
    
    if (millis() - geoTimer > GEOPAUSE) {                            //*** DELAY FOR GPS MODULE
       geoTimer = millis();
       gpsready = false;
       sensorsReset();
    }
   }

      
}
