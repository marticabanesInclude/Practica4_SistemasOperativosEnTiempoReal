#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Prototip de la funció de la tasca
void anotherTask(void * parameter);

void setup() {
  // Inicialització del port sèrie a la velocitat indicada a la pràctica
  Serial.begin(115200);

  // Creació de la tasca seguint els paràmetres de l'exercici
  xTaskCreate(
    anotherTask,      
    "another Task",  
    10000,            
    NULL,            
    1,               
    NULL            
  );
}

void loop() {
  // Tasca principal d'Arduino (loopTask)
  Serial.println("this is ESP32 Task");
  delay(1000);
}

// Implementació de la tasca addicional
void anotherTask(void * parameter) {
  for(;;) { // Bucle infinit obligatori per a tasques RTOS
    Serial.println("this is another Task");
    delay(1000);
  }

  // Cridada de seguretat per si la tasca sortís del bucle
  vTaskDelete(NULL);
}

