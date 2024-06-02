#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0; 
#else 
static const BaseType_t app_cpu = 1; 
#endif

// FreeRTOS file path
// C:\Users\tquin\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.5\tools\sdk\esp32\include\freertos

//Pins
static const int led_pin = LED_BUILTIN; 

void toggleLED(void * parameter) {
  while(1) {
    digitalWrite(led_pin, HIGH); 
    vTaskDelay(500 / portTICK_PERIOD_MS); 
    digitalWrite(led_pin, LOW); 
    vTaskDelay(5000 / portTICK_PERIOD_MS); 
  }

}

void setup() {
  pinMode(led_pin, OUTPUT); 

  xTaskCreatePinnedToCore(    //use xTaskCreate() is used in vanilla FreeRTOS
              toggleLED,      //fuction to be called     
              "Toggle LED",   // Task name
              1024,           // stack size (bytes in ESP32)
              NULL,           // parameter to pass to function
              1,              // Priority (0 - 24) 0 is the lowest priority
              NULL,           // Task handle
              app_cpu
  );

  // If this was vanilla free RTOS you'd need to call vTaskStartScheduler() in main after setting up your , 
  // But that is already done for us prior to setup

}


void loop() 
{
  //Setup and loop functions both run as their own tasks with priority 1, in core 1 of the ESP32
}