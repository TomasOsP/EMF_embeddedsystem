#include "display.h"
#include <U8g2lib.h>
#include <Arduino.h>
#include "sensors.h"
#include <stdio.h>

static const unsigned char image_Layer_8_bits[] = {0x1c,0x3e,0x7f,0x7f,0x7f,0x3e,0x1c};

// Inicializa el objeto U8G2 para pantalla 0.96" 128x64 OLED (I2C)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Inicialización del display
void initDisplay() {
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    // Layer 1
    // u8g2.setFont(u8g2_font_haxrcorp4089_tr);
    u8g2.setFont(u8g2_font_ncenB08_tr); // Fuente legible
    u8g2.drawStr(0, 20, "Display listo!");
    u8g2.sendBuffer();
    delay(500);
}

// Tarea que recibe datos por la cola y los muestra en el display
void TaskDisplay(void *pvParameters) {
    Vector3 campo;
    char buffer[32];
    int rec = 0;
    
    u8g2.clearBuffer();
    for (;;) {
        u8g2.clearBuffer();
        u8g2.drawLine(0, 13, 128, 13);
        if (rec) {
          rec = 0;
          u8g2.drawEllipse(121, 6, 4, 4);
        }
        else {
          rec = 1;
          u8g2.drawEllipse(121, 6, 4, 4);
          u8g2.drawXBM(118, 3, 7, 7, image_Layer_8_bits);
        }

        // Espera datos de la cola display_queue
        if (xQueueReceive(display_queue, &campo, portMAX_DELAY) == pdPASS) {
            Serial.println("Dato recibido");
            // Formatea los datos
            snprintf(buffer, sizeof(buffer), "X: %.2f", campo.x);
            u8g2.drawStr(11, 56, buffer);

            snprintf(buffer, sizeof(buffer), "Y: %.2f", campo.y);
            u8g2.drawStr(11, 42, buffer);

            snprintf(buffer, sizeof(buffer), "Z: %.2f", campo.z);
            u8g2.drawStr(11, 28, buffer);

            // Magnitud del campo
            float magnitude = sqrt(campo.x * campo.x + campo.y * campo.y + campo.z * campo.z);
            snprintf(buffer, sizeof(buffer), "Mag: %.2f uT", magnitude);
            u8g2.drawStr(70, 41, buffer);
        }
        else {
            u8g2.drawStr(0, 20, "Error leyendo cola");
        }
        u8g2.sendBuffer();
        vTaskDelay(pdMS_TO_TICKS(500)); // Actualiza cada 50 ms
    }
}



