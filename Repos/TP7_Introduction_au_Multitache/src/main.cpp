#include <Arduino.h>
// semaphores.ino
// Practical ESP32 Multitasking
// Binary Semaphores
#define LED1_GPIO 2
#define LED2_GPIO 18
#define Button 12
static SemaphoreHandle_t hsem;

void led_task(void *argp) {
  int led = (int)argp;
  BaseType_t rc;
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  for (;;) {
    // First gain control of hsem
    rc = xSemaphoreTake(hsem, portMAX_DELAY);
    assert(rc == pdPASS);
    for ( int x = 0; x < 6; ++x ) {
      digitalWrite(led, digitalRead(led) ^ 1);
      vTaskDelay(500);
    }
    rc = xSemaphoreGive(hsem);
    assert(rc == pdPASS);
  }
}

void setup() {

  pinMode(Button, INPUT_PULLUP);
  int app_cpu = xPortGetCoreID();
  BaseType_t rc; 
  Serial.print("Task is running on: ");
  Serial.println(xPortGetCoreID());  
  // Return code
  hsem = xSemaphoreCreateBinary();
  assert(hsem);
  rc = xTaskCreatePinnedToCore(
         led_task, // Function
         "led1task", // Task name
         3000, // Stack size 
         (void*)LED1_GPIO, // arg
         1, // Priority
         nullptr, // No handle returned
         app_cpu); // CPU
  assert(rc == pdPASS);

  // Allow led1task to start first
  rc = xSemaphoreGive(hsem);
  assert(rc == pdPASS);
  rc = xTaskCreatePinnedToCore(
         led_task, // Function
         "led2task", // Task name
         3000, // Stack size 
         (void*)LED2_GPIO, // argument
         1, // Priority
         nullptr, // No handle returned
         app_cpu); // CPU
  assert(rc == pdPASS);

}

// Not used
void loop() {

 /* int app_cpu = xPortGetCoreID();
  BaseType_t rc; 

  if (Button == LOW){

    hsem = xSemaphoreCreateBinary();
  assert(hsem);
  rc = xTaskCreatePinnedToCore(
         led_task, // Function
         "led1task", // Task name
         3000, // Stack size 
         (void*)LED1_GPIO, // arg
         1, // Priority
         nullptr, // No handle returned
         app_cpu); // CPU
  assert(rc == pdPASS);
  }

  else{

   // Allow led1task to start first
  rc = xSemaphoreGive(hsem);
  assert(rc == pdPASS);
  rc = xTaskCreatePinnedToCore(
         led_task, // Function
         "led2task", // Task name
         3000, // Stack size 
         (void*)LED2_GPIO, // argument
         1, // Priority
         nullptr, // No handle returned
         app_cpu); // CPU
  assert(rc == pdPASS);
  }
  vTaskDelete(nullptr);*/
}