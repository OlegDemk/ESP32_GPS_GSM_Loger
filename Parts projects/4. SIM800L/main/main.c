/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */


#include "main.h"

// Task handlers
TaskHandle_t task_gsm_handler;
TaskHandle_t task_led_blink_handler;
TaskHandle_t task_resurse_monitor_handlr;

static const char *AUTHORIZED_NUMBER = "+380931482354";


#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() 				 		// Налаштування таймера
#define configGENERATE_RUN_TIME_STATS 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configUSE_TRACE_FACILITY 1

// ----------------------------------------------------------------------------------------------
void task_resurse_monitor(void *ignore)
{

	while(1)
	{
		char stats[512] = {0,};

		vTaskDelay(5000/portTICK_PERIOD_MS);

		// Shows: spend time cheduller on each tasks
		vTaskGetRunTimeStats(stats);
		ESP_LOGI("\033[1;36mRunTimeStats", "\n%s\033[1;36m", stats);
		memset(stats, 0, sizeof(stats));

		// Shows: name of tasks, status, priority, free stack size and task number on each tasks
		vTaskList(stats);
		ESP_LOGI("\033[1;36mTaskList", "\n%s\033[1;36m", stats);

		vTaskDelay(15000/portTICK_PERIOD_MS);
	}
}

// ----------------------------------------------------------------------------------------------
void task_blink(void *ignore)	// For debug
{

	while(1)
	{
		gpio_set_level(CONFIG_GREEN_GPIO, 1);
		vTaskDelay(300 / portTICK_PERIOD_MS);
		gpio_set_level(CONFIG_GREEN_GPIO, 0);
		vTaskDelay(300 / portTICK_PERIOD_MS);
	}
}
// ----------------------------------------------------------------------------------------------
void task_gsm(void *ignore)
{
	static const char *GSM_TAG = "GSM";
	// #define BUF_SIZE 1024
	int buf_size = 1024;
	
	vTaskDelay(1000 / portTICK_PERIOD_MS);

	gsm(1);									// turn on GSM module by defoult
	init_uart_for_gsm();
	vTaskDelay(12000 / portTICK_PERIOD_MS);
	
	init_sim800l();

	while(1)
	{
		uint8_t data[buf_size];
		
		// Waiting data from GSM module
		int length = uart_read_bytes(UART_NUM_1, data, buf_size, 1000/portTICK_PERIOD_MS);
		if(length > 0)
		{
			data[length] = '\0';
			ESP_LOGI(GSM_TAG, "New message from GSM: %s", data);
			
			if(strstr((char*)data, "RING") != NULL)				// If incoming call detected
			{
				ESP_LOGI(GSM_TAG, "Incoming call");
				
				// Define incoming phone namber. If it my number answer call
				send_at_command("AT+CLCC\r\n");					
				vTaskDelay(1000 / portTICK_PERIOD_MS);
				
				uint8_t response[buf_size];
                read_gsm_response(response, buf_size);
				
				
				if (strstr((char*)response, AUTHORIZED_NUMBER) != NULL)
                {
                    ESP_LOGI(GSM_TAG, "Authorized number calling: %s", AUTHORIZED_NUMBER);
                    answer_call(); 					// Pick up the phone
                }
                else
                {
                    ESP_LOGI(GSM_TAG, "Unauthorized number calling. Ignoring call.");
                    send_at_command("ATH\r\n");  	// Hung up the phone
                }	
			}
			else if(strstr((char*)data, "+CMT") != NULL)		// If incoming SMS detected
			{
				ESP_LOGI(GSM_TAG, "SMS received, checking the content...");
				
				// Check if it's from the authorized number
    			if (strstr((char*)data, AUTHORIZED_NUMBER) != NULL)
    			{
					process_sms((char*)data);
				}
				else
				{
					ESP_LOGI(GSM_TAG, "SMS from unauthorized number. Ignoring.");
				}
			}
		}
	}
}
// ------------------------------------------------------------------------------------------
void app_main(void)
{
	init_output_gpio();
	RGB_LEDs_blink(5, 25);

	xTaskCreate(task_gsm, "task_gsm", 4096, NULL, configMAX_PRIORITIES - 1, &task_gsm_handler);
	xTaskCreate(task_blink, "task_blink", 1024, NULL, configMAX_PRIORITIES - 1, &task_led_blink_handler);
	xTaskCreate(task_resurse_monitor, "task_resurse_monitor", 4096, NULL, configMAX_PRIORITIES - 1, &task_resurse_monitor_handlr);

	while(1)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);

	}
}
// ------------------------------------------------------------------------------------------


