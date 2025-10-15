#include "display.h"
#include <U8g2lib.h>
#include <Arduino.h>
#include "sensors.h"
#include "data_an.h" // Para acceder a analysisResults
#include <stdio.h>

// Declarar el semáforo como externo
extern SemaphoreHandle_t buttonSemaphore;

// Icono de grabación (píxeles)
static const unsigned char image_Layer_8_bits[] = {0x1c,0x3e,0x7f,0x7f,0x7f,0x3e,0x1c};

// Inicializa el objeto U8G2 para pantalla 0.96" 128x64 OLED (I2C)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// --- Definición de Estados de Pantalla (como ya lo tienes) ---
typedef enum {
    SCREEN_LIVE_DATA = 0, // Datos X, Y, Z actuales
    SCREEN_RMS_MAX,       // Valores RMS Máximos
    SCREEN_AVG_6MIN,      // Promedio de 6 Minutos
    SCREEN_STATUS,        // Estado del sistema o la hora
    NUM_SCREENS           // Total de estados definidos
} DisplayScreen_t;

// Variable para rastrear el estado actual
DisplayScreen_t currentScreen = SCREEN_LIVE_DATA;

// --- 🎯 LISTA DE PANTALLAS EN LA ROTACIÓN ---
// Define las pantallas que el botón debe rotar.
const DisplayScreen_t activeScreens[] = {
    SCREEN_LIVE_DATA,
    SCREEN_RMS_MAX,
    SCREEN_AVG_6MIN
    // SCREEN_STATUS se omite si no quieres que rote
};

// Obtiene el número total de pantallas en la rotación
const size_t NUM_ACTIVE_SCREENS = sizeof(activeScreens) / sizeof(activeScreens[0]);

// Variable para rastrear el índice dentro de la lista (0, 1, 2...)
size_t currentScreenIndex = 0;

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

void draw_live_data_screen(const Vector3& campo) {
    char buffer[32];
    
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

    u8g2.drawStr(70, 56, "Unit: uT");
}

void draw_rms_max_screen() {
    char buffer[32];
    
    snprintf(buffer, sizeof(buffer), "X: %.2f", analysisResults.vrms_max.x);
    u8g2.drawStr(11, 56, buffer);

    snprintf(buffer, sizeof(buffer), "Y: %.2f", analysisResults.vrms_max.y);
    u8g2.drawStr(11, 42, buffer);

    snprintf(buffer, sizeof(buffer), "Z: %.2f", analysisResults.vrms_max.z);
    u8g2.drawStr(11, 28, buffer);
    u8g2.drawStr(70, 56, "Unit: uT");
}

void draw_avg_6min_screen() {
    char buffer[32];
    
    snprintf(buffer, sizeof(buffer), "X: %.2f", analysisResults.avg_6min.x);
    u8g2.drawStr(11, 56, buffer);

    snprintf(buffer, sizeof(buffer), "Y: %.2f", analysisResults.avg_6min.y);
    u8g2.drawStr(11, 42, buffer);

    snprintf(buffer, sizeof(buffer), "Z: %.2f", analysisResults.avg_6min.z);
    u8g2.drawStr(11, 28, buffer);

    snprintf(buffer, sizeof(buffer), "Mag: %.2f", 
             sqrt(analysisResults.avg_6min.x * analysisResults.avg_6min.x +
                  analysisResults.avg_6min.y * analysisResults.avg_6min.y +
                  analysisResults.avg_6min.z * analysisResults.avg_6min.z));
    u8g2.drawStr(70, 41, buffer);
    u8g2.drawStr(70, 56, "Unit: uT");
}
// Tarea que recibe datos por la cola y los muestra en el display
void TaskDisplay(void *pvParameters) {
    Vector3 campo;
    char buffer[32];
    int rec = 0;
    
    u8g2.clearBuffer();

    for (;;) {
        // Espera datos de la cola display_queue
        xQueueReceive(display_queue, &campo, portMAX_DELAY);

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
        switch (currentScreen) {
            case SCREEN_LIVE_DATA:
                // Title
                u8g2.drawStr(10, 10, "Inst. Value");    
                draw_live_data_screen(campo);
                break;

            case SCREEN_RMS_MAX:
                u8g2.drawStr(10, 10, "RMS Max Values");
                draw_rms_max_screen();
                break;

            case SCREEN_AVG_6MIN:
                u8g2.drawStr(10, 10, "Avg. Values");
                draw_avg_6min_screen();
                break;

            case SCREEN_STATUS:
                // Aquí puedes mostrar estado del sistema o la hora
                snprintf(buffer, sizeof(buffer), "Status OK");
                u8g2.drawStr(0, 30, buffer);
                break;

            default:
                break;
        }
        
        u8g2.sendBuffer();

        if (xSemaphoreTake(buttonSemaphore, 0) == pdTRUE) {
            // Cambia al siguiente estado en la lista activa
            currentScreenIndex = (currentScreenIndex + 1) % NUM_ACTIVE_SCREENS;
            currentScreen = activeScreens[currentScreenIndex];
            Serial.printf("Cambiando a pantalla %d\n", currentScreen);
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // Actualiza cada 50 ms
    }
}



