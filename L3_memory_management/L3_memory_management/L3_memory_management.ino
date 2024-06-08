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

listMember * firstMember = NULL;
listMember * currMember = NULL;  


//Add an item to a linked list
void addListMember(char newInput)
{
  currMember->nextMember = (listMember*)pvPortMalloc(sizeof(listMember)); //Allocate memory for a new list item. Add it to the list. 
  if(currMember->nextMember == NULL)
  {
    return;
  }
  currMember = currMember->nextMember; // Update it so that the current member points to the newly created member
  currMember->input = newInput; 
  currMember->nextMember = currMember; 
  if(firstMember == NULL) {
    firstMember = currMember; 
  }
}

void tsk_readSerial(void * parameter) {
  uint8_t newChar; 
  while(1) {
    if (Serial.available() > 0) {
      newChar = Serial.read();
      if(newChar == '0x0D')
      {
        vTaskResume(hnl_printSerial);
        vTaskSuspend(hnl_readSerial); 
      } 
      else{
        addListMember(newChar); //Add member to linked list
      }
    }
  }
}

void tsk_printSerial(void * parameter) {
  listMember* toBeErased; 
  if(firstMember->input == NULL)
  {
    vTaskResume(hnl_readSerial); 
    vTaskSuspend(hnl_printSerial);
  }
  Serial.print(firstMember->input); 
  toBeErased = firstMember; 
  firstMember = firstMember->nextMember; 
  vPortFree(toBeErased); 
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

  // Task to run forever
  xTaskCreatePinnedToCore(tsk_printSerial, 
                          "Serial Print", 
                          1024, 
                          NULL, 
                          1, 
                          &hnl_printSerial, 
                          app_cpu);

  Serial.print("first task created"); 

  xTaskCreatePinnedToCore(tsk_readSerial, 
                          "Serial read", 
                          1024, 
                          NULL, 
                          1, 
                          &hnl_readSerial, 
                          app_cpu);
  Serial.print("2nd task created"); 
  vTaskResume(hnl_printSerial);
  
}

void loop(){}
