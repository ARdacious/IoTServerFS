#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <gfxfont.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT_Macros.h>

// OLED display TWI address
#define OLED_ADDR   0x3C
// reset pin not used on 4-pin OLED module
Adafruit_SSD1306 display(-1);  // -1 = no reset pin  

#include "I2CScanner.h"


void oled() {
  Serial.println("OLED\n");
  //scanAllI2C();
  Wire.pins(4,5);
  Wire.begin();
  scanI2C();
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();
  display.invertDisplay(1);
  delay(500);
  display.invertDisplay(0);
  Serial.println("..oled done\n");
  pinMode(12, INPUT);
}
void sayHi() {
  Serial.println("Say hi...\n");
  // display a line of text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(27,30);
  display.print("IoTServerFS!");
  // update display with all of the above graphics
  display.display();
  delay(500);
  display.clearDisplay();
  display.display();
}

