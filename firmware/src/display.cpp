#include "display.h"
#include <U8g2lib.h>
#include <Arduino.h>
#include "sensors.h"

extern SemaphoreHandle_t mutexCampo;
extern Vector3 campoMag;

// Usa I2C (ajusta pines si es necesario)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void initDisplay() {
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x13_tr);
  u8g2.drawStr(0, 20, "Display listo...");
  u8g2.sendBuffer();
}

void taskDisplay(void *pvParameters) {
  Vector3 tempCampo;

  for (;;) {
    if (xSemaphoreTake(mutexCampo, portMAX_DELAY)) {
      tempCampo = campoMag;
      xSemaphoreGive(mutexCampo);
    }

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x13_tr);
    u8g2.drawStr(0, 10, "Campo Magnetico:");
    char line[32];
    snprintf(line, sizeof(line), "X: %.2f", tempCampo.x);
    u8g2.drawStr(0, 26, line);
    snprintf(line, sizeof(line), "Y: %.2f", tempCampo.y);
    u8g2.drawStr(0, 38, line);
    snprintf(line, sizeof(line), "Z: %.2f", tempCampo.z);
    u8g2.drawStr(0, 50, line);
    u8g2.sendBuffer();

    vTaskDelay(pdMS_TO_TICKS(100));  // 10 Hz
  }
}
