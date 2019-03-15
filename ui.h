
#define BUTTON_PIN 0
#define BUTTON_DELAY  300
#define BUTTON_DELAY1 800
#define BUTTON_DELAY2 1500
#define BUTTON_DELAY3 2000
#define CLICK       1
#define DUBLECLICK  2
#define PUSH        3


#include "ssd1306.h"    //oled lib  i2c  SCL - D1/5  SDA - D2/4

////////////////////////////////////////////// OLED initialisation
void ssd1306() {
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_128x64_i2c_init();
  ssd1306_fillScreen( 0x00 );
  ssd1306_clearScreen();
}

/////////////////////////////////////////////// Show String on the OLED
void oledPrint(int x, int y, String data) {
//  ssd1306_printFixed(0,  0, "START", STYLE_NORMAL);
 char line[30]; 
 data.toCharArray(line, 30);
 ssd1306_printFixed(x,  y, line, STYLE_NORMAL);
 
}

//////////////////////////////////////////////// Handle Button pressing
byte handleButton() {
   byte but = 0;
  if (digitalRead(BUTTON_PIN) == 0 && (millis() - tp) > BUTTON_DELAY) {  
    tp = millis();
    switch (button_state) {
      case 0:
              button_state = CLICK;
              break;
      case 1:
              button_state = DUBLECLICK;
              break;
      case DUBLECLICK:
              button_state = PUSH;
              break;
    }
  }

  if (digitalRead(BUTTON_PIN) == 1) {    
    if (button_state == CLICK && (millis() - tp) > BUTTON_DELAY1 ) {
//      Serial.println("CLICK ");
      tp = millis();
      button_state = 0;
      return(CLICK);
    }   
    if (button_state == DUBLECLICK && (millis() - tp) > BUTTON_DELAY2) {
//      Serial.println("DUBLECLICK ");
      tp = millis();
      button_state = 0;
      return(DUBLECLICK);
    }  
    if (button_state == PUSH && (millis() - tp) > BUTTON_DELAY3) {
//      Serial.println("PUSH ");
      tp = millis();
      button_state = 0;
      return(PUSH);
    }
  } 

   return(0);
}

