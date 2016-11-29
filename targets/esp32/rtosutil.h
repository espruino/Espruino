#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"


#define queueMax 10 //for queus we use an array of name/handle info. Core RTOS is not very helpful for this
struct RTOSqueue{ char *name;QueueHandle_t handle;};
struct RTOSqueue RTOSqueues[queueMax];

#define taskMax 10 //for tasks we use an array of name/handle info similiar to queue array
struct RTOStask{ char *name;TaskHandle_t handle;int rx;};
struct RTOStask RTOStasks[taskMax];

struct espruinoIO{int rx;}; //definition of struct for espruino IO, prepared to add more, like tx , .....
struct espruinoIO EspruinoIOs[];  //once we know how to get multiple tasks running with Espruino, .....
#define EspruinoMaster 0    //lets start with one task, .....

#define uartRxCharMax 256  //length of buffer for console queue, reading
#define uartTxCharMax 256  //not used (yet)

int consoleRxIdx;  //pointer to be prepared for a switch between Espruino sessions

void queues_init(); //initializes array of queues
int queue_indexByName(char *queueName); //returns index of queue in queue array by name
QueueHandle_t queue_handleByName(char *queueName); //returns handle of queue by name
int queue_init(char *queueName,int length,int sizeOfEntry); //initializes a queue with name,length and size of each entry
char *queue_read(int idx); //reads one character from queue
void queue_writeChar(int idx,char c); //writes one char to queue

void tasks_init(); //initializes array of tasks
int task_indexByName(char *taskName); //returns index of task in task array by name
TaskHandle_t task_handleByName(char *taskName); //returns handle of task by name
int *task_getCurrentIndex(); //returns index of actual task
char *task_getCurrentName(); //returns name of actual task
int task_init(TaskFunction_t taskCode, char *taskName,unsigned short stackDepth,UBaseType_t priority,BaseType_t coreId);
//initializes a task, using nonstandard rtos api call xTaskCreatePinnedToCore
void task_list(); //lists all entrys of task array usinf printf
void task_Suspend(int idx); //suspends task given by index in task array
void task_Resume(int idx); //resumes task given by index in task array

void console_readToQueue(); //reads char from uart and writes to RTOS queue, not using interrupts (yet)