#define EEPROMSIZE 4096
#define BEGINADDRESS 0
#define NAMESIZE 65
#define PASSSIZE 25
#include <EEPROM.h>

#include "ui.h"

//*********************************** DEVICE CONFIGURATION STRUCTURE 
struct AQM2_EEPROM_Config {
  uint16_t PIN;                 //*** PIN - number of board NodeMCU ESP8266 
  uint8_t devtype;              //*** device type, 1 - for AirQualityMonitor, 2 - for Water Quality Tester e.t.c. 
  uint16_t devnumber;           //*** device number
  char firmware[7];             //*** firmware version, format: XXX.YY 
  char hostname[NAMESIZE];      //*** hast url for firmware bin file 
  char binpath[NAMESIZE];       //*** folder with firmware bin file
  char ssid [NAMESIZE];         //*** last accessable Wifi SSID
  char password[PASSSIZE];      //*** last accessable Wifi SSID PASSWORD
  char dataserver[NAMESIZE];    //*** data server url
  char datapath[NAMESIZE];      //*** folder with data store
  char datapass[PASSSIZE];      //*** data server password
};

//*********************************** INDIVIDUAL DEVICE CONSTANTS STRUCTURE 
struct AQM2_EEPROM_Const {
  float vcc;                    //*** voltage offset (positive or negative float) for ESP.getVcc() in ADC_MODE(ADC_VCC)  
  uint8_t timeoffset;           //*** Timezone offset
  float longtitude;             //*** Neo-6M GPS longtitude offset
  float lattitude;              //*** Neo-6M GPS lattitude offset 
  uint8_t altitude;             //*** Neo-6M GPS altitude offset
  float temp;                   //*** BME280 temperature offset 
  uint8_t humi;                 //*** BME280 humidity offset  
  uint8_t press;                //*** BME280 pressure offset
  uint8_t pm1;                  //*** PMS7003, pm1.0 offset 
  uint8_t pm25;                 //*** PMS7003, pm2.5 offset
  uint8_t pm10;                 //*** PMS7003, pm10.0 offset
  float OSONE;                  //*** MQ-131 offset
  float SO2;                    //*** SO2 SH12, offset
  float NO2;                    //*** MISC6814, NO2 offset
  float CO;                     //*** MISC6814, CO  offset
  float NH3;                    //*** MISC6814, NH3 offset
  float CO2;                    //*** MH-Z19,   CO2 offset
  uint8_t RADtime;              //*** SBM-20, radiation time measuring in sec
  uint8_t RADdelay;             //*** SBM-20, radiation impuls measuring delay in millisec
  float SOUND;                  //*** Microphone, sound offset  
};

//*********************************** CURRENT RECORD NUMBER 
  uint16_t recordsnumber;       //*** current quantity of data records in EEPROM

//*********************************** DATA RECORD STRUCTURE
struct AQM2_EEPROM_Data {
  uint16_t record;              //*** record number
  uint8_t hour;                 //*** record time
  uint8_t minute;               //*** record time
  uint8_t second;               //*** record time
  uint8_t day;                  //*** record date
  uint8_t month;                //*** record date
  uint8_t year;                 //*** record date
  float longtitude;             //*** GPS longtitude
  float lattitude;              //*** GPS lattitude
  uint8_t altitude;             //*** GPS altitude
  uint8_t temp;                 //*** Temperature C*           
  uint8_t humi;                 //*** Humidity %
  uint8_t press;                //*** Pressure mm/H
  uint16_t pm1;                 //*** Dust pm1.0 uG/m3
  uint16_t pm25;                //*** Dust pm2.5 uG/m3
  uint16_t pm10;                //*** Dust pm10.0 uG/m3
  float OSONE;                  //*** Ozon (O3), ppm 
  float SO2;                    //*** Sulfur Dioxide (SO2), ppm 
  float NO2;                    //*** Nitrogen Dioxide (NO2), ppm
  float CO;                     //*** Carbon Monoxide (CO), ppm
  float NH3;                    //*** Ammonia  (NH3), ppm
  uint16_t CO2;                 //*** Carbon Dioxide (CO2), ppm
  uint16_t RAD;                 //*** Radioactive Background, uR/h  
  float SOUND;                  //*** Sound Level, dB
};

//*********************************** WRITING CONFIGURATION STRUCTURE IN EEPROM
void eepWriteConfig(AQM2_EEPROM_Config configurate) {
   EEPROM.put(BEGINADDRESS, configurate);
   EEPROM.commit();
}

//*********************************** READING CONFIGURATION STRUCTURE FROM EEPROM
AQM2_EEPROM_Config eepReadConfig() {
   AQM2_EEPROM_Config configurate;
   EEPROM.get(BEGINADDRESS, configurate);
   return(configurate);   
}

//*********************************** WRITING CONSTANTES STRUCTURE IN EEPROM
void eepWriteConstante(AQM2_EEPROM_Const constantes) {
   EEPROM.put(BEGINADDRESS + sizeof(AQM2_EEPROM_Config), constantes);
   EEPROM.commit();
}

//*********************************** READING CONSTANTES STRUCTURE FROM EEPROM
AQM2_EEPROM_Const eepReadConstante() {
   AQM2_EEPROM_Const constantes;
   EEPROM.get(BEGINADDRESS + sizeof(AQM2_EEPROM_Config), constantes);
   return(constantes);   
}

//*********************************** WRITING CURRENT NUMBER OF RECORDS OF DATA STRUCTURE IN EEPROM
void eepWriteDataRec(uint16_t numRecord) {
   EEPROM.put(BEGINADDRESS + sizeof(AQM2_EEPROM_Config) + sizeof(AQM2_EEPROM_Const), numRecord);
   EEPROM.commit();
}

//*********************************** READING CURRENT RECORD NUMBER OF DATA STRUCTURE FROM EEPROM
uint16_t eepReadDataRec() {
   uint16_t numRecords;
   EEPROM.get(BEGINADDRESS + sizeof(AQM2_EEPROM_Config) + sizeof(AQM2_EEPROM_Const), numRecords);  
   return(numRecords);
}

//*********************************** WRITING DATA STRUCTURE IN EEPROM USING CURRENT RECORD NUMBER
void eepWriteData(AQM2_EEPROM_Data dta) {
   recordsnumber = eepReadDataRec();         // get number of current record 
   EEPROM.put(BEGINADDRESS + sizeof(AQM2_EEPROM_Config) + sizeof(AQM2_EEPROM_Const) + sizeof(recordsnumber) + sizeof(AQM2_EEPROM_Data) * recordsnumber, dta);
   EEPROM.commit();
   recordsnumber++;
   eepWriteDataRec(recordsnumber);
}

//*********************************** READING DATA STRUCTURE FROM EEPROM USING RECORD NUMBER
AQM2_EEPROM_Data eepReadData(uint16_t numRecord) {
   AQM2_EEPROM_Data dta;
   EEPROM.get(BEGINADDRESS + sizeof(AQM2_EEPROM_Config) + sizeof(AQM2_EEPROM_Const) + sizeof(recordsnumber) + sizeof(AQM2_EEPROM_Data) * numRecord, dta);
   return(dta);
}

  AQM2_EEPROM_Config conf;          //GLOBAL VARIABLE OF CONFIGURATION STRUCTURE
  AQM2_EEPROM_Const co;             //GLOBAL VARIABLE OF CONSTANTES STRUCTURE
  AQM2_EEPROM_Data dat;             //GLOBAL VARIABLE OF DATA STRUCTURE

