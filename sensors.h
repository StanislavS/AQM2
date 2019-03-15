
#define ADCLEVEL      850   //*** ADCLEVEL FOR ADS1015 12bit ADC >= 2.55v DIGITAL LEVEL FOR SBM-20) GEIGER DETECTOR

#define PMSWAKEUP     170
#define PMSREAD       480
bool pmsready = true;

#define GEOPAUSE 5000
uint32_t geoTimer = millis();
bool gpsready = false;

#define BME280        500
bool bme280ready = true;

#define RAD_PIN 1      //ads2 chA1
#define RADIATIONSTART  10
//#define RADIATIONFINISH 470   // 46 sec for calculation rcount
//#define SBM20CONST 1540
uint16_t rcount = 0;  // SBM20 clicks
bool sbm20ready = true;
uint32_t tmp = millis();

#define SOUND_PIN 2      //ads2 chA2
#define SOUNDDETECTOR   475
uint8_t sound = 0;

#define CO_PIN  3      //ads1 chA3
#define NH3_PIN 2      //ads1 chA2
#define NO2_PIN 1      //ads1 chA1
#define MICS6814        510
bool mics6814ready = true;

#define OSONE_PIN 0      //ads1 chA0
#define MQ131           520
#define MQ131valueRL 10000
#define MQ131valueR0 110479.60
bool mq131ready = true;

//float O3Curve[2]      =  {42.84561841, -1.043297135};  //MQ131
//float Ro2 = 2.501;    //MQ131   2.24 this has to be tuned 10K Ohm
//float RL2 = 0.679;    //MQ131   Sainsmart
//MyMessage msg_mq131(CHILD_ID_MQ131, 42);    //Aq03
//MyMessage pcMsg_mq131(CHILD_ID_MQ131,V_VAR1);


#define SO2_PIN 3      //ads2 chA3
#define SO2SH12         530
//float           SO2_Curve[2]    =  {40.44109566, -1.085728557};  //MQ136 http://china-total.com/product/meter/gas-sensor/MQ136.pdf
//float Ro5 = 2.511;    //2SH12   
//float RL5 = 4000;     //2SH12   MQ-XL-V2 auto-ctrl.com 
//#define CHILD_ID_2SH12 6
//MyMessage msg_2sh12(CHILD_ID_2SH12, 46);    //AqSO2
//MyMessage pcMsg_2sh12(CHILD_ID_2SH12,V_VAR1);

bool so2sh12ready = true;

#define MHZ19           550
bool mhZ19ready = true;
byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
unsigned char resp[9];

#define POWER_PIN 0    //ads2 chA0

//const float adc_correction = 0.55;

//  int PM01Value=0;
//  int PM2_5Value=0;   
//  int PM10Value=0;     
//  #define LENG 23   
//  unsigned char buf[LENG];

//#include <SoftwareSerial.h>
//SoftwareSerial SW3(14, 12);   //MH-Z19
//SoftwareSerial SW2(13, 15); //neo-6m


//#include <Adafruit_ADS1015.h>

//Adafruit_ADS1115 ads1(0x48); 
//Adafruit_ADS1115 ads2(0x49); 
//Adafruit_ADS1015 ads(0x48); 
//Adafruit_ADS1015 ads1(0x49); 

//#include <Adafruit_BMP280.h>
//#include <Adafruit_BME280.h>
//Adafruit_BMP280 bme; // I2C
//Adafruit_BME280 bme;

#include <TinyGPS++.h>
// The TinyGPS++ object
TinyGPSPlus gps;
uint16_t gpsTr = 0;

////////////////////////////////////////// POWER TEST
void PowerTest() {
  float vcc = ESP.getVcc()/1000.0 + co.vcc;                                   //*** Getting Power Voltage include correction constant
  int pw = 100 - (3.3 - vcc) * 266.67;                                  //*** Calculation of POWER %, voltage range from 3.0v till 3.3v, if 3.3v - 100%, 3.0v - 20%, 2.92v - 0%
  int16_t adc0 = ads2.readADC_SingleEnded(POWER_PIN);  //Voltage measuring in ADC on the 0 pin of 2 channel
  float Vdd = adc0 * 0.003;        //Convertation in volts  (adc0 * 0.1875)/1000;

  oledPrint(0,  0, "BOARD " + String(conf.PIN) + " Vdd=" + String(Vdd)); 

/*  
  if (pw > 0) 
    oledPrint(0,  0, "BOARD " + String(conf.PIN) + " Power " + String(pw) + "%"); 
  else
    oledPrint(0,  0, "BOARD " + String(conf.PIN) + " POWER TOO LOW"); 
*/
  
}

////////////////////////////////////////// Handle Temperature, Humidity, Pressure by the BME sensor
void handleBME() {
   int8_t tt = bme.readTemperature();
   dat.temp = tt;
   tt = bme.readHumidity();
   dat.humi = tt;
   tt = bme.readPressure() / 133.33;
   dat.press = tt;
   bme280ready = false; 
}

//////////////////////////////////////////// Handle Dust by the PMS7003 sensor
uint8_t handlePMS() {
  if (pms.readUntil(PMSdata))
    {
        dat.pm1 = (int)PMSdata.PM_AE_UG_1_0;
        dat.pm25 = (int)PMSdata.PM_AE_UG_2_5;
        dat.pm10 = (int)PMSdata.PM_AE_UG_10_0;
//        return(1);
    }
//   else
//      return(0);

    pmsready = false;
    pms.sleep();     

}

/////////////////////////////////////////////// Handle SO2  by the 2SH12 sensor
void handle2SH12() {
  int16_t adc0 = ads2.readADC_SingleEnded(SO2_PIN);  //Voltage measuring in ADC on the 3 pin of 2 channel
  float Voltage = adc0 * 0.003;        //Convertation in volts  (adc0 * 0.1875)/1000;
  dat.SO2 = Voltage;
  so2sh12ready = false;
}

/////////////////////////////////////////////// Handle OSONE by the MQ-131 sensor
void handleMQ131() {
  int16_t adc0 = ads1.readADC_SingleEnded(OSONE_PIN);  //Voltage measuring in ADC on the 0 pin of 1 channel 
  float Voltage = adc0 * 0.003;        //Convertation in volts
  adc0 = ads2.readADC_SingleEnded(POWER_PIN);  //Voltage measuring in ADC on the 0 pin of 2 channel
  float Vdd = adc0 * 0.003;        //Convertation in volts  (adc0 * 0.1875)/1000;
   // Compute the resistance of the sensor (for 5V Arduino)
  float rS = (Vdd / Voltage - 1.0) * MQ131valueRL;
//  String gasOSONE = String(rS);    
//  oledPrint(65, 32, gasOSONE);

  int temperatureCelsuis = bme.readTemperature();
  int humidityPercent = bme.readHumidity();

  float correctRatio = 0.0;
   // Select the right equation based on humidity
  // If default value, ignore correction ratio
  if(humidityPercent == 60 && temperatureCelsuis == 20) {
    correctRatio = 1.0;
  }
  // For humidity > 75%, use the 85% curve
  if(humidityPercent > 75) {
    // R^2 = 0.9986
    correctRatio =  -0.0141 * temperatureCelsuis + 1.5623;
  }
  // For humidity > 50%, use the 60% curve
  if(humidityPercent > 50) {
    // R^2 = 0.9976
    correctRatio = -0.0119 * temperatureCelsuis + 1.3261;
  }

  // Humidity < 50%, use the 30% curve
  // R^2 = 0.996
  correctRatio = -0.0103 * temperatureCelsuis + 1.1507;

      // Use the equation to compute the O3 concentration in ppm
      // R^2 = 0.9987
      // Compute the ratio Rs/R0 and apply the environmental correction
      float ratio = rS / MQ131valueR0 * correctRatio;
      dat.OSONE = (9.4783 * pow(ratio, 2.3348)) * 1000;
//      concentration = 1.0;
//  dat.OSONE = concentration;
  mq131ready = false;
}

///////////////////////////////////////////////// Handle CO, NO2, NH3 by the Mics-6814 sensor
void handleMICS6814() {
  int16_t adc0 = ads1.readADC_SingleEnded(CO_PIN);  //Voltage measuring in ADC on the 3 pin of 1 channel 
  float Voltage = adc0 * 0.003;  
  dat.CO = Voltage;

  adc0 = ads1.readADC_SingleEnded(NH3_PIN);          //Voltage measuring in ADC on the 2 pin of 1 channel 
  Voltage = adc0 * 0.003;  
  dat.NH3 = Voltage;

  adc0 = ads2.readADC_SingleEnded(NO2_PIN);          //Voltage measuring in ADC on the 3 pin of 2 channel 
  Voltage = adc0 * 0.003;       
  dat.NO2 = Voltage;
  
  mics6814ready = false;
}

/////////////////////////////////////////////////// Handle Radiation by the SBM-20 sensor
void handleSBM20() {
//  rcount = 3 * rcount; // 3 * 20 sec = 1 min
  dat.RAD = rcount;
  oledPrint(0, 56, "RAD:" + String(dat.RAD) + "uR/h");
  rcount = 0;
  sbm20ready = false;    
}

//////////////////////////////////////////////////// Handle SOUND NOISE by the Sound detector
void handleSound() {
  delay(10);  
  int16_t adc0 = ads2.readADC_SingleEnded(SOUND_PIN);     //Voltage measuring in ADC on the 2 pin of 2 channel 
//  float Voltage = adc0 * 0.003;
  if (adc0 < 20) sound = 30;
  else if (adc0 < 200) sound = 40;
  else if (adc0 < 1000) sound = 50;
  else if (adc0 < 1500) sound = 60;
  else if (adc0 < 1600) sound = 70;
  else sound = 80;
    
  dat.SOUND = sound;
//  oledPrint(65, 56, "         ");
  oledPrint(65, 56, "SND:" + String((int)dat.SOUND) + "db");

}

///////////////////////////////////////////////////// Handle CO2 by the MH-Z19 sensor
void handleMHZ19() {
//  byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
//  unsigned char resp[9];

  SW3.write(cmd, 9);
  memset(resp, 0, 9);
  SW3.readBytes(resp, 9);
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc+=resp[i];
  crc = 255 - crc;
  crc++;

  if ( !(resp[0] == 0xFF && resp[1] == 0x86 && resp[8] == crc) ) {
//    oledPrint(0, 8, "CRC ER " + String(crc) + "/" +String(resp[8]));
    dat.CO2 = 5555;
  } else {
    unsigned int responseHigh = (unsigned int) resp[2];
    unsigned int responseLow = (unsigned int) resp[3];
    unsigned int ppm = (256*responseHigh) + responseLow;
    dat.CO2 = ppm;
//    Serial.println(ppm);
  }
  mhZ19ready = false;
}

////////////////////////////////////////////////////// GPS NEO-6M SECTION ///////////////////////////////////
/////////////////////////////////////////////
static void smartDelay(unsigned long ms)                // This custom version of delay() ensures that the gps object is being "fed".
{
  unsigned long start = millis();
  do 
  {
    while (SW2.available())
      gps.encode(SW2.read());
  } while (millis() - start < ms);
}


///////////////////////////////////
uint8_t gpsInfo()
{

//  return (0);
  
  gpsTr++;
  smartDelay(1000);                                      // Run Procedure smartDelay

  if (gps.location.isValid())
  {
//    Serial.print(gps.location.lat(), 6);
//    data1 = "GPS:" + String(gps.location.lat());
//    Serial.print(F(","));
//    Serial.print(gps.location.lng(), 6);
//    data1 += " " + String(gps.location.lng());
     dat.lattitude = gps.location.lat();
     dat.longtitude = gps.location.lng();
     dat.altitude = gps.altitude.meters();
     oledPrint(0, 0, "GPS: " + String(dat.lattitude, 4) + "  " + String(dat.longtitude, 4));
  }
  else
  {
//    Serial.print(F("INVALID"));
//    data1 = "GPS NO DATA " + String(gpsTr);
     oledPrint(0, 0, "                      "); 
     oledPrint(0, 0, "GPS NO DATA " + String(gpsTr)); 
     return (1);
  }

//   data1.toCharArray(line, 30);
//   ssd1306_printFixed(0,  32, line, STYLE_NORMAL);

//  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
//    Serial.print(gps.date.month());
//    Serial.print(F("/"));
//    Serial.print(gps.date.day());
//    Serial.print(F("/"));
//    Serial.print(gps.date.year());
    String d = "";
    String m = "";

    if (gps.date.day() < 10) d = "0";
     d += String(gps.date.day());
    if (gps.date.month() < 10) m = "0";
     m += String(gps.date.month());    
     dat.day = (int)gps.date.day();
     dat.month = (int)gps.date.month();
     dat.year = (int)gps.date.year();
     oledPrint(0, 8, String(dat.day) + "/" + String(dat.month) + "/" + String(dat.year)); 
  }
  else
  {
    return(2);
//    Serial.print(F("INVALID"));
  }

//  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    String h = "";
    String m = "";
    String s = "";
    int hh = gps.time.hour() + co.timeoffset;
    if (hh == 24) hh = 0;
    if (hh == 25) hh = 1;
    if (hh == 0) hh = 2;
     
    if (hh < 10) h = "0"; //Serial.print(F("0"));
//    Serial.print(gps.time.hour());
//    Serial.print(F(":"));
      h += String(hh);
    if (gps.time.minute() < 10) m = "0"; //Serial.print(F("0"));
//    Serial.print(gps.time.minute());
//    Serial.print(F(":"));
      m += String(gps.time.minute());
    if (gps.time.second() < 10) s = "0"; //Serial.print(F("0"));
//    Serial.print(gps.time.second());
//    Serial.print(F("."));
      s += String(gps.time.second());

      dat.hour = (int)gps.time.hour();
      dat.minute = (int)gps.time.minute();
      dat.second = (int)gps.time.second();
      
//    if (gps.time.centisecond() < 10) Serial.print(F("0"));
//    Serial.print(gps.time.centisecond());
     oledPrint(65, 8, String(dat.hour) + ":" + String(dat.minute) + ":" + String(dat.second)); 
  }
  else
  {
    return(3);
//    Serial.print(F("INVALID"));
  }

 return(0);
  
}
//////////////////////////////////
void sensorsDisplay() {
  oledPrint(0, 16, "T:" + String(dat.temp) + "*C ");        //*** BME DATA SHOW
  oledPrint(40, 16, "H:" + String(dat.humi) + "% ");
  oledPrint(76, 16, "Pr:" + String(dat.press) + "mm");
  
  String data1 = "DUST PMS:" + String(dat.pm1) + "/" + String(dat.pm25) + "/" + String(dat.pm10);  
  oledPrint(0, 24, "                     ");
  oledPrint(0, 24, data1);

  oledPrint(0, 32, "SO2:" + String(dat.SO2));

  oledPrint(65, 32, "OZONE:" + String(dat.OSONE));

  oledPrint(0, 40, "CO:" + String(dat.CO));                 //*** MICS-6814 DATA SHOW
  oledPrint(65, 40, "NH3:" + String(dat.NH3));
  oledPrint(0, 48, "NO2:" + String(dat.NO2));

  oledPrint(65, 48, "CO2:" + String(dat.CO2));

}
//////////////////////////////////
void sensorsReset() {
     sensorsDisplay();
          pmsready = true;
          bme280ready = true;
          sbm20ready = true;
          mics6814ready = true;
          mq131ready = true;
          so2sh12ready = true;
          mhZ19ready = true;  
          sound = 0;
}

