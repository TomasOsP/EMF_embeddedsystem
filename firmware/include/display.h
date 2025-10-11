#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "sensors.h"

// Cola global para enviar resultados al display / WiFi
void initDisplay();
void TaskDisplay(void *pvParameters);