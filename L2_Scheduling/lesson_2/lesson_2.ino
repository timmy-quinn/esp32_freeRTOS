#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0; 
#else
  static const BaseType_t app_cpu = 1; 
#endif

#define TIMEOUT     10
#define NUM_DIGITS  10

const char msg[] = "Hey how's it going?"; 
unsigned char delayTimeDigits[NUM_DIGITS]; 

static TaskHandle_t hnl_blinkLED = NULL; 
static TaskHandle_t hnl_readSerial = NULL; 


uint32_t delayTime;

static const int led_pin = LED_BUILTIN; 

void clearDelayTimeChar() 
{
  for(uint8_t i = 0; i < NUM_DIGITS; i++)
  {
    delayTimeDigits[i] = 0; 
  }
}

uint32_t exp(uint8_t base, uint8_t pow) 
{
  Serial.println("Power function"); 
  Serial.println(base); 
  Serial.println(pow); 
  uint32_t result = base;
  if(pow == 0)
  {
    Serial.println(1); 
    return 1; 
  } 
  for(uint8_t i = 1; i < pow; i++) 
  {
    result*= base; 
  }
  Serial.println(result); 
  return result; 
}

void setDelayTime(uint8_t numDelayDigits)
{
  uint32_t test; 
  Serial.println("setDelayTimefunction"); 
  Serial.println(numDelayDigits);

  if(delayTimeDigits[0] < 48 || delayTimeDigits[0] > 57) {
    Serial.print("Delaytime: "); 
    Serial.println(delayTime); 
    return; 
  }
  else {
    delayTime = 0; 
  }
  for(int8_t i = 0; i < numDelayDigits; i++) 
  {
    if(delayTimeDigits[i] < 48 || delayTimeDigits[i] > 57) {
      Serial.print("Delaytime: "); 
      Serial.println(delayTime); 
      return; 
    }
    test = ((uint32_t)delayTimeDigits[i] - 48) * exp(10, numDelayDigits - 1 - i); 
    Serial.print("test: ");
    Serial.println(test); 
    delayTime += test; //((uint32_t)delayTimeDigits[i] - 48) * exp(10, numDelayDigits - 1 - i); 
  }
  Serial.print("Delaytime: ");
  Serial.println(delayTime);  
  Serial.println("returning"); 

}

void tsk_readSerial(void * parameter) {
  uint8_t billionMS[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //Converting from uint32_t to ASCII
  uint8_t charNum = 0; 
  
  while(1) {
    if (Serial.available() > 0) {
      // read the incoming byte:
      Serial.println("Delay time changing "); 
      delayTimeDigits[charNum] = Serial.read();
      Serial.println(delayTimeDigits[charNum]);  
      if(delayTimeDigits[charNum] == 10 || charNum == NUM_DIGITS - 1) // If new line character or at end of available space
      {
        setDelayTime(charNum); 
        charNum = 0; 
      }
      else
      {
        charNum++; 
      }
    }
  }
}

void tsk_blinkLED(void * parameter) {
  while(1) {
    digitalWrite(led_pin, HIGH); 
    Serial.println(delayTime); 
    vTaskDelay(delayTime); 
    digitalWrite(led_pin, LOW); 
    vTaskDelay(delayTime);

  }

}

void setup() {
  Serial.begin(115200); 
  pinMode(led_pin, OUTPUT); 
  delayTime = 1000; 

  vTaskDelay(1000 / portTICK_PERIOD_MS); 
  Serial.println(); 
  Serial.println("-----FreeRTOS Task Demo------"); 

  //PRint self priority
  Serial.print("Setup and loop task running on core "); 
  Serial.print(xPortGetCoreID()); 
  Serial.print(" with priority "); 
  Serial.println(uxTaskPriorityGet(NULL)); 

  // Task to run forever
  xTaskCreatePinnedToCore(tsk_blinkLED, 
                          "Blink LED", 
                          1024, 
                          NULL, 
                          1, 
                          &hnl_blinkLED, 
                          app_cpu);

  Serial.print("first task finished"); 

  xTaskCreatePinnedToCore(tsk_readSerial, 
                          "Serial read", 
                          1024, 
                          NULL, 
                          1, 
                          &hnl_readSerial, 
                          app_cpu);
  Serial.print("2nd task finished"); 
  
}

void loop() 
{
  
}
