#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "sensors.h"


void initDisplay();
void TaskDisplay(void *pvParameters);