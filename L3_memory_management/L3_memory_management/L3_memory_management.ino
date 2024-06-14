/*
 * ************ DigiKey Introduction to RTOS Part 4******************
 * Topic: Memory management
 * Problem: Write a task that reads serial inputs, and another task that prints them, upon end line character
 * https://www.youtube.com/watch?v=Qske3yZRW5I&list=PLEBQazB0HUyQ4hAPU1cJED6t3DU0h34bz&index=4
 * File path: C:\..\Arduino15\packages\esp32\hardware\esp32\2.0.5\tools\sdk\esp32\include\freertos
*/ 
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0; 
#else
  static const BaseType_t app_cpu = 1; 
#endif

#define END_CHAR 0x0D

const char msg[] = "Hey how's it going?"; 
unsigned char delayTimeDigits[NUM_DIGITS]; 

static TaskHandle_t hnl_printSerial = NULL; 
static TaskHandle_t hnl_readSerial = NULL; 
static const int led_pin = LED_BUILTIN; 

bool printChars = false; 


//Simple linked list implementation
typedef struct _listMember listMember;
struct _listMember {
  char input; 
  listMember* nextMember; 
};

listMember *firstMember = NULL;
listMember *currMember = NULL;  

//Add an item to a linked list
void addListMember(char newInput)
{
  Serial.println(newInput);

  listMember *prevMember;
  prevMember = currMember; 
  currMember = (listMember*)pvPortMalloc(sizeof(listMember)); //Allocate memory for a new list item. Add it to the list. 

  currMember->nextMember = NULL; 
  currMember->input = newInput; 

  if(prevMember == NULL){ //Check to see if this is the first member added to the linked list
    firstMember = currMember; 
  }
  else{ //If there are previously existing linked list members, update the previous to point to the new addition
    prevMember->nextMember = currMember; 
  }
}


// Read serial messages and add chars to linked list until carriage return is reached
void tsk_readSerial(void * parameter) {
  uint8_t newChar; 

  while(1) {
    if (Serial.available() > 0) {
      newChar = Serial.read();
      addListMember(newChar); //Add member to linked list
      if(newChar == END_CHAR) {
        printChars = true;
      } 
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// Print the linked list to the screen
void tsk_printSerial(void * parameter) {
  listMember* printMember; 
  while(1){
    if(printChars) {
      printMember = firstMember; 
      if(printMember == NULL) { //Reached the end of the linked list
        printChars = false;
        currMember = NULL; 
      }
      else {
        Serial.print(printMember->input); 
        firstMember = printMember->nextMember; //Move the first pointer to point to the next member in the list
        vPortFree(printMember);
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  } 
}

//Setup 
void setup() {
  Serial.begin(19200); 

  vTaskDelay(1000 / portTICK_PERIOD_MS); 
  Serial.println(); 
  Serial.println("-----FreeRTOS Task Demo------"); 

  //PRint self priority
  Serial.print("Setup task running on core "); 
  Serial.print(xPortGetCoreID()); 
  Serial.print(" with priority "); 
  Serial.println(uxTaskPriorityGet(NULL)); 

  //Task to run forever
  xTaskCreatePinnedToCore(tsk_printSerial, 
                          "Serial Print", 
                          5000, 
                          NULL, 
                          1, 
                          &hnl_printSerial, 
                          app_cpu);

  Serial.println("first task created"); 

  xTaskCreatePinnedToCore(tsk_readSerial, 
                          "Serial read", 
                          1024, 
                          NULL, 
                          1, 
                          &hnl_readSerial, 
                          app_cpu);
  Serial.println("2nd task created"); 
  //vTaskResume(hnl_printSerial);
  
}

void loop() {}
