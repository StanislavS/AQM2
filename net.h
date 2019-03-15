#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>

#include "eep.h"

const char *ss = "AQM2_DEV";                      //*** SSID PREFIX AP
const char *pass = "1234567890";                  //*** PASSWORD AP

bool submit = false;
const int httpPort = 80;
unsigned long utime = millis();
WiFiClient client;
ESP8266WebServer server(80);
bool wifiACC = false;
bool APstart = true;
bool APmode = false;

//*************************************************************** HTTP UPDATE FIRMWARE
int HTTPUpdate() {
     
 String binfile = conf.binpath + String(conf.PIN) + ".bin";                         //*** SAMPLE BINFILE PATH -> /ino/bin/916.bin
 t_httpUpdate_return ret = ESPhttpUpdate.update(conf.hostname, httpPort, binfile);
 switch(ret) {
              case HTTP_UPDATE_FAILED:
                   break;
              case HTTP_UPDATE_NO_UPDATES:
                   break;
              case HTTP_UPDATE_OK:
                   break;
             }
return 1;
}

//**************************************************************** HTTP GET REQUEST CREATION
uint8_t reqGET(uint8_t dataVar) {                            //*** IF dataVar = 0 - UPDATE FIRMWARE, = 1 - DATA TRANSFER

 String infofile;
 String datahost;

 uint8_t j = 0;
 uint8_t num= 0;

 if (dataVar == 0) {                //*** FORMATION STRINGS FOR OTA REQUEST
    infofile = String(conf.binpath) + "verification.php?board=" + String(conf.PIN);
    datahost = String(conf.hostname);
 }

 if (dataVar == 1) {                //*** FORMATION STRINGS FOR DATA TRANSFER REQUEST
    
    dat = eepReadData(recordsnumber);
    infofile = String(conf.datapath) + "aqm2datatransfer.php?device=" + String(conf.devnumber) + "&password=" + String(conf.datapass);
    infofile += "&hour=" + String( dat.hour);                 //*** record time
    infofile += "&minute=" + String( dat.minute);               //*** record time
    infofile += "&second=" + String( dat.second);               //*** record time
    infofile += "&day=" + String( dat.day);                  //*** record date
    infofile += "&month=" + String( dat.month);                //*** record date
    infofile += "&year=" + String( dat.year);                 //*** record date
    infofile += "&longtitude=" + String( dat.longtitude);             //*** GPS longtitude
    infofile += "&lattitude=" + String( dat.lattitude);              //*** GPS lattitude
    infofile += "&altitude=" + String( dat.altitude);             //*** GPS altitude
    infofile += "&temp=" + String( dat.temp);                 //*** Temperature C*           
    infofile += "&humi=" + String( dat.humi);                 //*** Humidity %
    infofile += "&press=" + String( dat.press);                //*** Pressure mm/H
    infofile += "&pm1=" + String( dat.pm1);                 //*** Dust pm1.0 uG/m3
    infofile += "&pm25=" + String( dat.pm25);                //*** Dust pm2.5 uG/m3
    infofile += "&pm10=" + String( dat.pm10);                //*** Dust pm10.0 uG/m3
    infofile += "&OSONE=" + String(dat.OSONE);                  //*** Ozon (O3), ppm 
    infofile += "&SO2=" + String( dat.SO2);                    //*** Sulfur Dioxide (SO2), ppm 
    infofile += "&NO2=" + String( dat.NO2);                    //*** Nitrogen Dioxide (NO2), ppm
    infofile += "&CO=" + String( dat.CO);                     //*** Carbon Monoxide (CO), ppm
    infofile += "&NH3=" + String( dat.NH3);                    //*** Ammonia  (NH3), ppm
    infofile += "&CO2=" + String( dat.CO2);                 //*** Carbon Dioxide (CO2), ppm
    infofile += "&RAD=" + String( dat.RAD);                 //*** Radioactive Background, uR/h  
    infofile += "&SOUND=" + String( dat.SOUND);                  //*** Sound Level, dB
    
    datahost = String(conf.dataserver);
 }
     
 if (!client.connect(datahost.c_str(), httpPort)) {
     return (2);
 }

 String req1 = "GET " + infofile + " HTTP/1.1\r\nHost: " + datahost + "\r\nConnection: close\r\n\r\n";

 client.print(req1);  
 unsigned long timeout = millis();
 while (client.available() == 0) {
    if (millis() - timeout > 5000) {
//      Serial.println(">>> Client Timeout !");
      client.stop();
      return(3);
    }
  }
  
                                  //*** Read all the lines of the reply from server and print them to Serial
 while(client.available()){
   j++;
   String line = client.readStringUntil('\r');
   
   if (j == 10) num = line.toInt();  //*** IF num != 0 REQUEST EXECUTED
   }

  client.stop();
  return (num);
}
//******************************************************************* New Firmware TESTED
void ota() {

  utime = millis();
  if (wifiACC) {
      if ( reqGET(0) == 1) {
           ssd1306_clearScreen();
           oledPrint(0, 0, "NEW FIRMWARE UPDATE" );
           oledPrint(0, 16, "  Please, wait! " );
           int k = HTTPUpdate();
      }
      delay(100);
  }     
}

//********************************************************************* WIFI CONNECTION
uint8_t wifi() {
   
     byte acc = 0;
     WiFi.begin(conf.ssid, conf.password);
     delay(500);
     while (WiFi.status() != WL_CONNECTED ) 
          {       
          delay(250);
          digitalWrite(BLUE_LED, LOW);
          delay(250);
          digitalWrite(BLUE_LED, HIGH);
          acc++;
          if (acc > WIFIACC) { 
            wifiACC = false;
            break;
            }
          }
            
    if (WiFi.status() == WL_CONNECTED ) wifiACC = true;

    ssd1306_clearScreen();
    if (wifiACC)    { 
      oledPrint(0,  0, "WiFi " + String(conf.ssid) + "  "); 
      digitalWrite(BLUE_LED, HIGH); 
      return(0); 
      }
    else            { 
      oledPrint(0,  0, "No WiFi connection   "); 
      digitalWrite(BLUE_LED, LOW); 
      return(1); 
      }
   
   
}
//************************************************************************* AP MODE WEB SERVER PAGE1
void webpage() {
          server.send(200, "text/html", "<html><body><form  name='frm'  method='post'><input type='number' size='5' name='x'><input type='submit' value='Enter PIN or 0 for close'>   </form></body></html>");
}
//************************************************************************* AP MODE WEB SERVER PAGE2
void response(){
  if(server.hasArg("x") && (server.arg("x").length()>0)){ // TODO check that it's not longer than 31 characters
//    Serial.print("User entered:\t");
//    Serial.println(server.arg("x"));
    String l = String(server.arg("x"));
    l = l.substring(0, 4);
    if (l.toInt() == conf.PIN) {
          server.send(200, "text/html", 
          "<html><body><form  name='frm1'  method='post'>PIN: <input type='number' size='5' name='pin' value='" + String(conf.PIN) + "'> Device Type: <input type='text' size='8' name='devtype' value='" + String(conf.devtype) + 
          "'> Device Serial Number:<input type='number'  size='8' name='devnumber' value='" + String(conf.devnumber) +
          "'><br>Device Host Name:<input type='text'  size='55' name='hostname' value='" + String(conf.hostname) + 
          "'><br>FIRMWARE Binary File Path:<input type='text' name='binpath' value='" + String(conf.binpath) + 
          "'><br>SSID:<input type='text' name='ssid' value='" + String(conf.ssid) + "'> PASSWORD:<input type='text' name='pass' value='" + String(conf.password) +
          "'><br>DATA SERVER:<input type='text' name='dataserver' value='" + String(conf.dataserver) + "'> DATA PATH:<input type='text' name='datapath' value='" + String(conf.datapath) + 
          "'><br>DATASERVER PASSWORD:<input type='text' name='datapass' value='" + String(conf.datapass) +
          "'><br>CONSTANTS<br>Vcc:<input type='text'  size='5' name='vcc' value='" + String(co.vcc) + "'> Time Offset:<input type='text'  size='5' name='timeoffset' value='" + String(co.timeoffset) +
          "'>GPS Longtitude:<input type='text'  size='8' name='long' value='" + String(co.longtitude) + "'> GPS Lattitude:<input type='text'  size='8' name='lat' value='" + String(co.lattitude) + "'> GPS Altitude:<input type='text'  size='8' name='alt' value='" + String(co.altitude) +
          "'><br>Temper.:<input type='text'  size='6' name='temp' value='" + String(co.temp) + "'> Humi.:<input type='text'  size='6' name='humi' value='" + String(co.humi) + "'> Press.:<input type='text'  size='6' name='press' value='" + String(co.press) +
          "'> Dust pm1.0:<input type='text'  size='6' name='pm1' value='" + String(co.pm1) + "'> pm2.5:<input type='text'  size='6' name='pm25' value='" + String(co.pm25) + "'> pm10:<input type='text'  size='6' name='pm10' value='" + String(co.pm10) +
          "'><br> Osone:<input type='text'  size='8' name='osone' value='" + String(co.OSONE) + "'> SO2:<input type='text'  size='8' name='so2' value='" + String(co.SO2) + "'> NO2:<input type='text'  size='8' name='no2' value='" + String(co.NO2) +
          "'> CO:<input type='text'  size='6' name='co' value='" + String(co.CO) + "'> NH3:<input type='text'  size='6' name='nh3' value='" + String(co.NH3) + "'> CO2:<input type='text'  size='6' name='co2' value='" + String(co.CO2) +
          "'> RADtime:<input type='text'  size='6' name='radtime' value='" + String(co.RADtime) + "'> RADdel:<input type='text'  size='6' name='raddel' value='" + String(co.RADdelay) + "'> SOUND:<input type='text'  size='6' name='sound' value='" + String(co.SOUND) + 
          "'><br><br> <input type='submit' value='Submit'> <input type='reset' value='Reset'> </form></body></html>");
    }
    if  (l.toInt() == 0) {                      //*** IF USER TYPE '0' -> CLOSE AP MODE
      APmode = false;
      WiFi.softAPdisconnect (true);
      delay(1000);
//      wifi();
//      ota();
    }
   else {
//    inString = server.arg("x");
    server.send(200, "text/html", "<html><body><h1>Please, try again. </h1><a href='/'> Home </a></body></html>");
    }
  } 
//***************************************************************************** CHECK ALL DATA FROM INPUT FIELDS WEB PAGE 2  
  if(server.hasArg("pin") && (server.arg("pin").length()>0)){ 
    String l = String(server.arg("pin"));
    conf.PIN = l.toInt();
    submit = true;
  }

  if(server.hasArg("devtype") && (server.arg("devtype").length()>0)){ 
    String l = String(server.arg("devtype"));
    conf.devtype = l.toInt();
    submit = true;
  }

   if(server.hasArg("devnumber") && (server.arg("devnumber").length()>0)){ 
    String l = String(server.arg("devnumber"));
    conf.devnumber = l.toInt();
  }

   if(server.hasArg("hostname") && (server.arg("hostname").length()>0)){ 
    server.arg("hostname").toCharArray(conf.hostname, NAMESIZE);
  }

    if(server.hasArg("binpath") && (server.arg("binpath").length()>0)){ 
    server.arg("binpath").toCharArray(conf.binpath, NAMESIZE);
  }

    if(server.hasArg("ssid") && (server.arg("ssid").length()>0)){ 
    server.arg("ssid").toCharArray(conf.ssid,NAMESIZE);
  }

    if(server.hasArg("pass") && (server.arg("pass").length()>0)){ 
    server.arg("pass").toCharArray(conf.password,PASSSIZE);
  }

    if(server.hasArg("dataserver") && (server.arg("dataserver").length()>0)){ 
    server.arg("dataserver").toCharArray(conf.dataserver, NAMESIZE);
  }

    if(server.hasArg("datapath") && (server.arg("datapath").length()>0)){ 
    server.arg("datapath").toCharArray(conf.datapath, NAMESIZE);
  }

    if(server.hasArg("datapass") && (server.arg("datapass").length()>0)){ 
    server.arg("datapass").toCharArray(conf.datapass, PASSSIZE);
  }

    if(server.hasArg("vcc") && (server.arg("vcc").length()>0)){ 
  }

    if(server.hasArg("vcc") && (server.arg("vcc").length()>0)){ 
    co.vcc = server.arg("vcc").toFloat();
  }

    if(server.hasArg("timeoffset") && (server.arg("timeoffset").length()>0)){ 
    co.timeoffset = server.arg("timeoffset").toInt();
  }

    if(server.hasArg("long") && (server.arg("long").length()>0)){ 
    co.longtitude = server.arg("long").toFloat();
  }

    if(server.hasArg("lat") && (server.arg("lat").length()>0)){ 
    co.lattitude = server.arg("lat").toFloat();
  }

    if(server.hasArg("alt") && (server.arg("alt").length()>0)){ 
    co.altitude = server.arg("alt").toInt();
  }

    if(server.hasArg("temp") && (server.arg("temp").length()>0)){ 
    co.temp = server.arg("temp").toFloat();
  }

    if(server.hasArg("humi") && (server.arg("humi").length()>0)){ 
    co.humi = server.arg("humi").toInt();
  }

    if(server.hasArg("press") && (server.arg("press").length()>0)){ 
    co.press = server.arg("press").toInt();
  }

    if(server.hasArg("pm1") && (server.arg("pm1").length()>0)){ 
    co.pm1 = server.arg("pm1").toInt();
  }

    if(server.hasArg("pm25") && (server.arg("pm25").length()>0)){ 
    co.pm25 = server.arg("pm25").toInt();
  }

    if(server.hasArg("pm10") && (server.arg("pm10").length()>0)){ 
    co.pm10 = server.arg("pm10").toInt();
  }

    if(server.hasArg("osone") && (server.arg("osone").length()>0)){ 
    co.OSONE = server.arg("osone").toFloat();
  }

    if(server.hasArg("so2") && (server.arg("so2").length()>0)){ 
    co.OSONE = server.arg("osone").toFloat();
  }

    if(server.hasArg("no2") && (server.arg("no2").length()>0)){ 
    co.NO2 = server.arg("no2").toFloat();
  }

    if(server.hasArg("co") && (server.arg("co").length()>0)){ 
    co.CO = server.arg("co").toFloat();
  }

    if(server.hasArg("nh3") && (server.arg("nh3").length()>0)){ 
    co.NH3 = server.arg("nh3").toFloat();
  }

    if(server.hasArg("co2") && (server.arg("co2").length()>0)){ 
    co.CO2 = server.arg("co2").toFloat();
  }

    if(server.hasArg("radtime") && (server.arg("radtime").length()>0)){ 
    co.RADtime = server.arg("radtime").toInt();
  }

    if(server.hasArg("raddel") && (server.arg("raddel").length()>0)){ 
    co.RADdelay = server.arg("raddel").toInt();
  }

    if(server.hasArg("sound") && (server.arg("sound").length()>0)){ 
    co.SOUND = server.arg("sound").toFloat();
    submit = true;
  }
//****************************************************************************** WRITE DATA IN EEPROM AFTER PRESS SUBMIT BUTTON IN WEB PAGE 2
  if (submit) {
      submit = false;
      eepWriteConfig(conf);
      eepWriteConstante(co);
      server.send(200, "text/html", "<html><body><h1>Successful</h1><a href='/'>Home</a></body></html>");
  }
  else {
    server.send(400, "text/html", "<html><body><h1>HTTP Error 400</h1><p>Bad request. Please enter a value.</p></body></html>");
  }

  
}
//**************************************************************************************** AP WEB SITE STRART PROCEDURE
void APsite() {
  
           String APss = ss + String(conf.devnumber);
           WiFi.softAP(APss.c_str(), pass);
           IPAddress myIP = WiFi.softAPIP();
           server.on("/",HTTP_GET, webpage);
           server.on("/",HTTP_POST,response);
           server.begin();
           ssd1306_clearScreen();
           oledPrint(0, 0, "AP " + String(myIP[0]) + "." + String(myIP[1]) + "." + String(myIP[2]) + "." + String(myIP[3]) );
           APmode = true;
  
}
//****************************************************************************************** DATA TRANSFER 1 RECORD PROCEDURE
uint8_t dataTransfer() {
  if (wifiACC) {
      uint8_t req = reqGET(1);
      
      oledPrint(0, 8, "               " );
      if ( req == 1) {
           oledPrint(0, 8, "DATA TRANSFER..." + String(recordsnumber ));
           return(1);
      }
      if ( req == 2) {
           oledPrint(0, 8, "HOST ERROR!" );
      }
      if ( req == 3) {
           oledPrint(0, 8, "CLIENT TIMEOUT" );
      }
      if ( req == 0) {
           oledPrint(0, 8, "DATA ERROR!" );
      }

      return (0);
  }     
  
}

