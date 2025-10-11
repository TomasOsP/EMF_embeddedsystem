#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// Declarar el objeto u8g2 para el SH1106, 128x64 píxeles, con hardware I2C
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

void displayInit();
void displayClear();
void displayPrint(const char* message);