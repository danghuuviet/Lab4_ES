#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_random.h"
#include <string.h>

#define TASK_STACK_SIZE 4096

QueueHandle_t queueHandle;

struct data
{
    int taskID;
    char taskData[500];
};

typedef struct data DATA;

void functionalTask(void *pvParameter)
{
    int myTaskID = (int)pvParameter;
    DATA receivedData;

    while (1)
    {
        if (xQueueReceive(queueHandle, &receivedData, portMAX_DELAY))
        {
            if (receivedData.taskID == myTaskID)
            {
                // Process the data
                printf("Task %d: %s\n", myTaskID, receivedData.taskData);
            }
            else
            {
                // Not for this task, put it back on the queue
                xQueueSend(queueHandle, &receivedData, portMAX_DELAY);
            }
        }
    }
}

void receptionTask(void *pvParameter)
{
    DATA dataToSend = {};

    while (1)
    {
        dataToSend.taskID = esp_random() % 3 + 1;
        sprintf(dataToSend.taskData, "Task is running: %d", dataToSend.taskID);

        if (!xQueueSend(queueHandle, &dataToSend, 100))
        {
            printf("Queue Send Failed\n");
        }

        UBaseType_t queueLength = uxQueueMessagesWaiting(queueHandle);
        if (queueLength > 20)
        {
            printf("Error: Unhandled Request\n");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main()
{
    queueHandle = xQueueCreate(20, sizeof(DATA));

    xTaskCreatePinnedToCore(functionalTask, "Task1", TASK_STACK_SIZE, (void *)1, 1, NULL, 0);
    xTaskCreatePinnedToCore(functionalTask, "Task2", TASK_STACK_SIZE, (void *)2, 1, NULL, 0);
    xTaskCreatePinnedToCore(functionalTask, "Task3", TASK_STACK_SIZE, (void *)3, 1, NULL, 0);

    xTaskCreatePinnedToCore(receptionTask, "ReceptionTask", TASK_STACK_SIZE, NULL, 2, NULL, 0);
}