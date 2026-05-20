#include <Arduino.h>


const int ledPin = 2;


SemaphoreHandle_t xSemaphore;


void taskEncender(void *pvParameters) {
 for (;;) {
   if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
     digitalWrite(ledPin, HIGH);
     Serial.println("LED ENCENDIDO - Ejecutado por Tarea 1");
    
     vTaskDelay(1000 / portTICK_PERIOD_MS);
    
     xSemaphoreGive(xSemaphore);
    
     vTaskDelay(10 / portTICK_PERIOD_MS);
   }
 }
}


void taskApagar(void *pvParameters) {
 for (;;) {
   if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
     digitalWrite(ledPin, LOW);
     Serial.println("LED APAGADO - Ejecutado por Tarea 2");
    
     vTaskDelay(1000 / portTICK_PERIOD_MS);
    
     xSemaphoreGive(xSemaphore);
    
     vTaskDelay(10 / portTICK_PERIOD_MS);
   }
 }
}


void setup() {
 Serial.begin(115200);
 pinMode(ledPin, OUTPUT);


 xSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(xSemaphore);


 xTaskCreate(
   taskEncender,  
   "Encender",   
   2048,        
   NULL,        
   1,           
   NULL           
 );


 xTaskCreate(
   taskApagar,
   "Apagar",
   2048,
   NULL,
   1,
   NULL
 );
}


void loop() {
}
