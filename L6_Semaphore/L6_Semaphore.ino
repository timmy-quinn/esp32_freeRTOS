/**
 * FreeRTOS Counting Semaphore Challenge
 * 
 * Challenge: use a mutex and counting semaphores to protect the shared buffer 
 * so that each number (0 throguh 4) is printed exactly 3 times to the Serial 
 * monitor (in any order). Do not use queues to do this!
 * 
 * Hint: you will need 2 counting semaphores in addition to the mutex, one for 
 * remembering number of filled slots in the buffer and another for 
 * remembering the number of empty slots in the buffer.
 * 
 * Date: January 24, 2021
 * Author: Shawn Hymel
 * License: 0BSD
 */

// You'll likely need this on vanilla FreeRTOS
//#include <semphr.h>

#define CONFIG_FREERTOS_UNICORE 1

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
enum {BUF_SIZE = 5};                  // Size of buffer array
static const int num_prod_tasks = 5;  // Number of producer tasks
static const int num_cons_tasks = 2;  // Number of consumer tasks
static const int num_writes = 3;      // Num times each producer writes to buf

// Globals
static int buf[BUF_SIZE];             // Shared buffer
static int head = 0;                  // Writing index to buffer
static int tail = 0;                  // Reading index to buffer
static SemaphoreHandle_t bin_sem;     // Waits for parameter to be read
static SemaphoreHandle_t emptySlots; 
static SemaphoreHandle_t filledSlots; 
static SemaphoreHandle_t mutex;

//*****************************************************************************
// Tasks

// Producer: write a given number of times to shared buffer
void producer(void *parameters) {

  // Copy the parameters into a local variable
  int num = *(int *)parameters;

  // Release the binary semaphore
  xSemaphoreGive(bin_sem);

  // Fill shared buffer with task number
  for (int i = 0; i < num_writes; ) {
    
    // Critical section (accessing shared buffer)
    if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) 
    { 
      if(xSemaphoreTake(emptySlots, 200) == pdTRUE) // IF this delay is too long, it will block the following 
      {
        xSemaphoreGive(filledSlots);
        buf[head] = num;
        head = (head + 1) % BUF_SIZE; 
        i++; 
      }
      xSemaphoreGive(mutex); 
    }
    vTaskDelay(20/ portTICK_PERIOD_MS);
  }

  // Delete self task
  Serial.print("Deleting task: ");  
  Serial.println(val);
  vTaskDelete(NULL);
}

// Consumer: continuously read from shared buffer
void consumer(void *parameters) {

  int val; 

  // Read from buffer
  while (1) {
    //Serial.println("while"); 
    // Critical section (accessing shared buffer and Serial)
    //Serial.println("Consumer"); 
    if(xSemaphoreTake(mutex, 1) == pdTRUE)
    {
      //Serial.println("Mutex taken consumer"); 
      if(xSemaphoreTake(filledSlots, 1) == pdTRUE)
      {
        xSemaphoreGive(emptySlots); 

        val = buf[tail];

        tail = (tail + 1) % BUF_SIZE;
        Serial.println(val);
      }
      xSemaphoreGive(mutex); 
      vTaskDelay(1/portTICK_PERIOD_MS); 
    }
  }
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)
void setup() {
  vTaskDelay(5000); 
  char task_name[12];
  
  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Semaphore Alternate Solution---");

  // Create mutexes and semaphores before starting tasks
  bin_sem = xSemaphoreCreateBinary();
  emptySlots = xSemaphoreCreateCounting(BUF_SIZE, BUF_SIZE); // Keep track of empty slots
  filledSlots = xSemaphoreCreateCounting(BUF_SIZE, 0);       // Keep track of filled slots
  mutex = xSemaphoreCreateMutex(); 


  // Start producer tasks (wait for each to read argument)
  for (int i = 0; i < num_prod_tasks; i++) {
    sprintf(task_name, "Producer %i", i);
    xTaskCreatePinnedToCore(producer,
                            task_name,
                            1024,
                            (void *)&i,
                            1,
                            NULL,
                            app_cpu);
    xSemaphoreTake(bin_sem, portMAX_DELAY);
  }

  Serial.println("Producer tasks created"); 

  // Start consumer tasks
  for (int i = 0; i < num_cons_tasks; i++) {
    sprintf(task_name, "Consumer %i", i); 
    xTaskCreatePinnedToCore(consumer,
                            task_name,
                            1024,
                            NULL,
                            1,
                            NULL,
                            app_cpu);
  }

  // Notify that all tasks have been created
  Serial.println("All tasks created");
}

void loop() {
  // Do nothing but allow yielding to lower-priority tasks
  vTaskDelay(10000 / portTICK_PERIOD_MS);
}