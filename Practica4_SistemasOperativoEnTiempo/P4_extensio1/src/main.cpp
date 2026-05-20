#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"


// Definición de pines según tu petición
#define LED_SEGUNDOS 2  // LED integrado o externo en el 2
#define LED_MODO 42     // Segundo LED en el pin 42
#define BTN_MODO 35     // Botón para cambiar modo
#define BTN_INCREMENTO 38 // Botón para incrementar valor


// Variables del reloj protegidas
volatile int horas = 0;
volatile int minutos = 0;
volatile int segundos = 0;
volatile int modo = 0; // 0: normal, 1: horas, 2: minutos


QueueHandle_t botonQueue;
SemaphoreHandle_t relojMutex;


typedef struct {
 uint8_t boton;
 uint32_t tiempo;
} EventoBoton;


// ISR para botones
void IRAM_ATTR ISR_Boton(void *arg) {
 uint8_t numeroPulsador = (uint32_t)arg;
 EventoBoton evento;
 evento.boton = numeroPulsador;
 evento.tiempo = xTaskGetTickCountFromISR();
 xQueueSendFromISR(botonQueue, &evento, NULL);
}


// Prototipos
void TareaReloj(void *pvParameters);
void TareaLecturaBotones(void *pvParameters);
void TareaActualizacionDisplay(void *pvParameters);
void TareaControlLEDs(void *pvParameters);


void setup() {
 Serial.begin(115200);
  pinMode(LED_SEGUNDOS, OUTPUT);
 pinMode(LED_MODO, OUTPUT);
 pinMode(BTN_MODO, INPUT_PULLUP);
 pinMode(BTN_INCREMENTO, INPUT_PULLUP);


 botonQueue = xQueueCreate(10, sizeof(EventoBoton));
 relojMutex = xSemaphoreCreateMutex();


 attachInterruptArg(BTN_MODO, ISR_Boton, (void*)BTN_MODO, FALLING);
 attachInterruptArg(BTN_INCREMENTO, ISR_Boton, (void*)BTN_INCREMENTO, FALLING);


 xTaskCreate(TareaReloj, "Reloj", 2048, NULL, 1, NULL);
 xTaskCreate(TareaLecturaBotones, "Botones", 2048, NULL, 2, NULL);
 xTaskCreate(TareaActualizacionDisplay, "Display", 2048, NULL, 1, NULL);
 xTaskCreate(TareaControlLEDs, "LEDs", 1024, NULL, 1, NULL);
}


void loop() {
 vTaskDelay(portMAX_DELAY);
}


void TareaReloj(void *pvParameters) {
 TickType_t xLastWakeTime = xTaskGetTickCount();
 const TickType_t xPeriod = pdMS_TO_TICKS(1000);
 for (;;) {
   vTaskDelayUntil(&xLastWakeTime, xPeriod);
   if (xSemaphoreTake(relojMutex, portMAX_DELAY) == pdTRUE) {
     if (modo == 0) {
       segundos++;
       if (segundos >= 60) { segundos = 0; minutos++; }
       if (minutos >= 60) { minutos = 0; horas++; }
       if (horas >= 24) { horas = 0; }
     }
     xSemaphoreGive(relojMutex);
   }
 }
}


void TareaLecturaBotones(void *pvParameters) {
 EventoBoton evento;
 uint32_t ultimoTiempo = 0;
 for (;;) {
   if (xQueueReceive(botonQueue, &evento, portMAX_DELAY) == pdPASS) {
     if ((evento.tiempo - ultimoTiempo) >= pdMS_TO_TICKS(300)) {
       if (xSemaphoreTake(relojMutex, portMAX_DELAY) == pdTRUE) {
         if (evento.boton == BTN_MODO) {
           modo = (modo + 1) % 3;
           Serial.printf("MODO ACTUAL: %d\n", modo);
         } else if (evento.boton == BTN_INCREMENTO) {
           if (modo == 1) horas = (horas + 1) % 24;
           else if (modo == 2) { minutos = (minutos + 1) % 60; segundos = 0; }
         }
         xSemaphoreGive(relojMutex);
       }
       ultimoTiempo = evento.tiempo;
     }
   }
 }
}


void TareaActualizacionDisplay(void *pvParameters) {
 for (;;) {
   if (xSemaphoreTake(relojMutex, portMAX_DELAY) == pdTRUE) {
     Serial.printf("HORA: %02d:%02d:%02d | MODO: %d\n", horas, minutos, segundos, modo);
     xSemaphoreGive(relojMutex);
   }
   vTaskDelay(pdMS_TO_TICKS(1000));
 }
}


void TareaControlLEDs(void *pvParameters) {
 for (;;) {
   if (xSemaphoreTake(relojMutex, portMAX_DELAY) == pdTRUE) {
     // El LED 2 parpadea con los segundos
     digitalWrite(LED_SEGUNDOS, segundos % 2);
     // El LED 42 se enciende si estamos en modo ajuste
     digitalWrite(LED_MODO, modo > 0);
     xSemaphoreGive(relojMutex);
   }
   vTaskDelay(pdMS_TO_TICKS(100));
 }
}
