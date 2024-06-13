#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0; 
#else
  static const BaseType_t app_cpu = 1; 
#endif

#define TIMEOUT     10
#define NUM_DIGITS  10

// FreeRTOS file path
// C:\Users\tquin\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.5\tools\sdk\esp32\include\freertos

const char msg[] = "Hey how's it going?"; 
unsigned char delayTimeDigits[NUM_DIGITS]; 

static TaskHandle_t hnl_printSerial = NULL; 
static TaskHandle_t hnl_readSerial = NULL; 
static const int led_pin = LED_BUILTIN; 


//Simple linked list implementation
typedef struct _listMember listMember;
struct _listMember
{
  char input; 
  listMember* nextMember; 
};

listMember *firstMember = NULL;
listMember *currMember = NULL;  


//Add an item to a linked list
void addListMember(char newInput)
{
  Serial.println("New character: "); 
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

void tsk_readSerial(void * parameter) {
  uint8_t newChar; 
  while(1) {
    if (Serial.available() > 0) {
      newChar = Serial.read();
      addListMember(newChar); //Add member to linked list
      if(newChar == 0x0D)
      {
        Serial.println("Endline character"); 
        vTaskResume(hnl_printSerial);
        vTaskSuspend(hnl_readSerial); 
      } 
    }
  }
}

void tsk_printSerial(void * parameter) {
  listMember* printMember; 
  while(1){
    //Serial.println("Print serial task"); 
    printMember = firstMember; 
    if(printMember == NULL) {
      Serial.println(); 
      Serial.println("firstMember = null"); 
      currMember = NULL; 
      vTaskResume(hnl_readSerial);
      vTaskSuspend(NULL);
      Serial.println("Returning");  
    }
    else {
      Serial.print(printMember->input); 
      firstMember = printMember->nextMember; //Move the first pointer to point to the next member in the list
      vPortFree(printMember);
    }
  } 
}

void setup() {
  Serial.begin(115200); 

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

void loop(){}
