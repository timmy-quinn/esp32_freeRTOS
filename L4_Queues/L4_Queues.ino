/*
 * ************ DigiKey Introduction to RTOS Part 5******************
 * Topic: Queues
 * Problem: 
 * Task A should print any new messages it receives from Queue 2. 
 * Additionally, it should read any Serial input from the user and echo back this input to the serial input. 
 * If the user enters “delay” followed by a space and a number, it should send that number to Queue 1.

 * Task B should read any messages from Queue 1. 
 * If it contains a number, it should update its delay rate to that number (milliseconds). 
 * It should also blink an LED at a rate specified by that delay. 
 * Additionally, every time the LED blinks 100 times, it should send the string “Blinked” to Queue 2. 
 * You can also optionally send the number of times the LED blinked (e.g. 100) as part of struct that encapsulates the string and this number.

 * https://www.youtube.com/watch?v=Qske3yZRW5I&list=PLEBQazB0HUyQ4hAPU1cJED6t3DU0h34bz&index=4
 * File path: C:\..\Arduino15\packages\esp32\hardware\esp32\2.0.5\tools\sdk\esp32\include\freertos
*/ 

#include <stdio.h>
#include <string.h> 

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0; 
#else
  static const BaseType_t app_cpu = 1; 
#endif

#define END_CHAR                  0x0D
#define END_CHAR_BYTE             1
#define MAX_DELAY_CMD_LEN         12
#define MAX_MESSAGE_LEN           15
#define TWOBYTE_DIGITS_DEC        5
#define BLINK_MSG_THRESHOLD       100

typedef struct _messageItem {
  char str[MAX_MESSAGE_LEN];
  uint8_t len;
  uint8_t numBlinks;  
} messageItem_t; 

//Simple linked list implementation
typedef struct _listMember listMember;
struct _listMember {
  char input; 
  listMember* nextMember; 
};

static const uint8_t delayStrLen = 6; 
static const char delayStr[delayStrLen] = {'d', 'e', 'l', 'a', 'y', ' '}; 

static TaskHandle_t hnl_A = NULL; 
static TaskHandle_t hnl_B = NULL; 

static QueueHandle_t queue_1;
static QueueHandle_t queue_2; 

static messageItem_t messageBuff; 
static char blinkMessage[] = "Blinked";

static const int led_pin = LED_BUILTIN; 

listMember *firstMember = NULL;
listMember *currMember = NULL;  

uint16_t exp(uint8_t base, uint8_t pow) 
{
  uint16_t result = base;
  if(pow == 0)
  {
    return 1; 
  } 
  for(uint8_t i = 1; i < pow; i++) 
  {
    result*= base; 
  }
  return result; 
}

//Add an item to a linked list
void addListMember(char newInput)
{
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

// Print the linked list to the screen
void printList(void) {
  if(firstMember == NULL) return; 

  listMember* printMember;
  printMember = firstMember; 
  while(printMember != NULL) {
    Serial.print(printMember->input); 
    printMember = printMember->nextMember; //Move the first pointer to point to the next member in the list
  }
  Serial.println(); 
}

// Free the linked list
void freeList(void) {

  listMember* freeMember; 
  freeMember = firstMember; 
  while(freeMember != NULL) {
    firstMember = firstMember->nextMember;
    vPortFree(freeMember);
    freeMember = firstMember; 
  } 
  currMember = NULL; 
}


// This function is ugly but no real point in optimizing at this point. 
// Maybe use a doubly linked list? Or add index parameter to LL item, or count variable?
uint16_t isDelayCommand() {
  if(firstMember == NULL) { return 0; } 

  listMember* checkMember = firstMember; 
  uint16_t delay = 0; 
  uint8_t numDigits = 0; 
  char numStr[TWOBYTE_DIGITS_DEC];
  uint16_t result = 0;  
  Serial.println("isDelayCommand");
  // Check if the beginning of the string == 'Delay '
  for(uint8_t i = 0; i < delayStrLen; i++)
  {  
    if((delayStr[i] != checkMember->input) || (checkMember == NULL)) {
      return 0; 
    } 
    if(checkMember->nextMember != NULL) {
      checkMember = checkMember->nextMember; 
    }
  }

  Serial.println("Delay string found"); 

  for(numDigits = 0; (checkMember != NULL ) && (checkMember->input != END_CHAR); numDigits++)
  {
    Serial.print("Numdigits"); 
    Serial.println(numDigits); 
    Serial.print("checkMember->input"); 
    Serial.println(checkMember->input); 
    // if((checkMember->nextMember == NULL ) || (checkMember->input == END_CHAR)){
    //   Serial.println("breaking"); 
    //   break; 
    // }
    if((checkMember->input < '0') || (checkMember->input > '9')) {
      return 0; 
    }

    numStr[numDigits] = checkMember->input -'0'; 

    // if(checkMember->nextMember != NULL) {
    checkMember = checkMember->nextMember;
    // }
  }

  if(numDigits > TWOBYTE_DIGITS_DEC) { return 0xffff; } 

  Serial.println("delay found"); 
  Serial.println((int)numStr[0]); 
  Serial.println(numDigits); 

  for(uint8_t i = 0; i < numDigits; i++) {
    Serial.println("numDigits loop");
    result += numStr[i] * exp(10, numDigits - i - 1);
  }
  Serial.print("delay detected: "); 
  Serial.print(result); 
  return result; 
}

uint16_t receiveSerialInput()
{ 
  uint16_t delay; 
  char newChar; 
  while(Serial.available() > 0) {
    newChar = Serial.read(); 
    addListMember(newChar);  
    if(newChar == END_CHAR) {
      continue; 
    }
  }
  delay = isDelayCommand(); 
  printList(); 
  freeList(); 
  return delay;
}

void tsk_A(void * parameter) {
  messageItem_t msg; 
  char newChar; 
  uint16_t delay; 
  while(1)
  {
    //Serial.println("Task A"); 
    if(xQueueReceive(queue_1, (void *)&msg, 0) ==pdTRUE)
    {
      Serial.print("Message length "); 
      Serial.println(msg.len); 
      for(uint8_t i = 0; i < msg.len; i++) {
        Serial.print(msg.str[i]); 
      }
    }

    delay = receiveSerialInput(); 
    if(delay) {
      if (xQueueSend(queue_2, (void *)&delay, 10) != pdTRUE) {
        Serial.println("Queue full"); 
      }
    }
    
    vTaskDelay(10);
  }
}

void tsk_B(void * parameter) {
  uint16_t delay; 
  uint16_t tempDelay; 
  uint8_t blinks; 
  while(1) {
    if(xQueueReceive(queue_2, (void *)&tempDelay, 0) ==pdTRUE)
    {
      delay = tempDelay; 
    }

    digitalWrite(led_pin, HIGH); 
    vTaskDelay(delay / portTICK_PERIOD_MS); 
    digitalWrite(led_pin, LOW); 
    vTaskDelay(delay / portTICK_PERIOD_MS);
    blinks++; 
    if(blinks == BLINK_MSG_THRESHOLD) {
      //Serial.println("Task B"); 
      messageItem_t msg; 
      strlcpy(&msg.str[0], "Blinked", sizeof("Blinked")); 
      msg.len = sizeof("Blinked"); 
      msg.numBlinks = blinks; 
      if(xQueueSend(queue_1, (void *)&msg, 10) != pdTRUE) {
        Serial.println("Queue 1 full");
      }
      Serial.println("Msg sent"); 
      blinks = 0; 
    }
    //vTaskDelay(delay / portTICK_PERIOD_MS); 
  }
}

void setup() {
  Serial.begin(19200);
  Serial.println("Printing"); 
  pinMode(led_pin, OUTPUT); 

  queue_1 = xQueueCreate(5, sizeof(messageItem_t)); // Create queue before using it in tasks
  queue_2 = xQueueCreate(10, sizeof(uint16_t)); 

  BaseType_t result = xTaskCreatePinnedToCore(tsk_A, 
                                              "task A", 
                                              10000,
                                              NULL, 
                                              1, 
                                              &hnl_A,
                                              app_cpu);

  if(result != pdPASS)
  {
    Serial.println("Task creation failed");  
  }

  result = xTaskCreatePinnedToCore(tsk_B, 
                          "task B", 
                          10000,
                          NULL, 
                          1, 
                          &hnl_B,
                          app_cpu); 

  if(result != pdPASS)
  {
    Serial.println("Task creation failed"); 
  }
}


// This function is required for arduino to build this code.
// Can't be deleted so just stick a delay in there to prevent it from hogging every tick
void loop() { 
  vTaskDelay(0xffff); 
  Serial.println("Stupid loop"); // This is stupid, therefor print stupid
}