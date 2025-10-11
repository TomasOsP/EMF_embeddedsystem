#include <Arduino.h>
#include "display.h"
#include <U8g2lib.h>
#include <Wire.h>

// Declarar el objeto u8g2 para el SH1106, 128x64 píxeles, con hardware I2C
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void displayInit() {
  u8g2.begin();
  u8g2.enableUTF8Print();  // Habilitar soporte para UTF-8
  displayClear();
}

void displayClear() {
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}

void displayPrint(const char* message) {
  u8g2.setFont(u8g2_font_logisoso16_tr); // Fuente grande y legible
  u8g2.drawStr(5, 35, message);
  u8g2.sendBuffer();
}