/*
 * ************ DigiKey Introduction to RTOS Part 6******************
 * Topic: Mutex
 * Problem: Starting with the code given below, modify it to protect the task parameter (delay_arg) with a mutex. 
 * With the mutex in place, the task should be able to read the parameter (parameters) into the local variable (num) 
 * before the calling function’s stack memory goes out of scope (the value given by delay_arg).  
 * 
 * Video link: https://www.youtube.com/watch?v=I55auRpbiTs
 * File path: C:\..\Arduino15\packages\esp32\hardware\esp32\2.0.5\tools\sdk\esp32\include\freertos
*/ 

/**
 * FreeRTOS Mutex Challenge
 * 
 * Pass a parameter to a task using a mutex.
 * 
 * Date: January 20, 2021
 * Author: Shawn Hymel
 * License: 0BSD
 */

// You'll likely need this on vanilla FreeRTOS
//#include semphr.h

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Pins (change this if your Arduino board does not have LED_BUILTIN defined)
static const int led_pin = LED_BUILTIN;

// Mutex handler: 
static SemaphoreHandle_t mutex1; 


//*****************************************************************************
// Tasks

// Blink LED based on rate passed by parameter
void blinkLED(void *parameters) {

  xSemaphoreTake(mutex1, 1); 

  //vTaskDelay(10); 
  // Copy the parameter into a local variable
  int num = *(int *)parameters;
  xSemaphoreGive(mutex1);

  // Print the parameter
  Serial.print("Received: ");
  Serial.println(num);


  Serial.println("Semaphore given back"); 

  // Configure the LED pin
  pinMode(led_pin, OUTPUT);

  // Blink forever and ever
  while (1) {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(num / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(num / portTICK_PERIOD_MS);
  }
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  long int delay_arg;

  mutex1 = xSemaphoreCreateMutex(); 
  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Mutex Challenge---");
  Serial.println("Enter a number for delay (milliseconds)");

    // Wait for input from Serial
    while (Serial.available() <= 0);

    // Read integer value
    delay_arg = Serial.parseInt();
    Serial.print("Sending: ");
    Serial.println(delay_arg);

  xSemaphoreTake(mutex1, portMAX_DELAY);
  // Start task 1
  xTaskCreatePinnedToCore(blinkLED,
                          "Blink LED",
                          1024,
                          (void *)&delay_arg,
                          1,
                          NULL,
                          app_cpu);

  // Show that we accomplished our task of passing the stack-based argumen
  Serial.println("Done!");

  while(1) {
     if(xSemaphoreTake(mutex1, 0) == pdTRUE) {
      xSemaphoreGive(mutex1); 
     }
     else{
       Serial.println("breaking"); 
       break; 
     }
  }

}

void loop() { 
  // Do nothing but allow yielding to lower-priority tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}