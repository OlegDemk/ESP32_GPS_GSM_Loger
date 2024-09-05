/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */


#include "main.h"

// Task handlers
TaskHandle_t task_tx_gsm_handler;
TaskHandle_t task_rx_gsm_handler;

// Queues
QueueHandle_t gsm_data_queue = NULL;


// ------------------------------------------------------------------------------------------
//void task_tx_gsm(void *ignore)
//{
//	 static const char *GSM_TX_TASK_TAG = "GSM_TX_TASK";
////	 esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
//	 vTaskDelay(1000 / portTICK_PERIOD_MS);
//
//	 turn_ON_power_of_gsm_module();
//
//
//	 vTaskDelay(1000 / portTICK_PERIOD_MS);
//	 init_gsm();
//	 vTaskDelay(10000 / portTICK_PERIOD_MS);
//	 sendData(GSM_TX_TASK_TAG, "ATD+380931482354;\n\r");
//
//
//
//
//
//	 while (1)
//	 {
//		 //sendData(TX_TASK_TAG, "Hello world\n\r");
//		 //sendData(TX_TASK_TAG, "ATD+380931482354;\n\r");
//
////		 init_gsm();
//
//
//
//
//	     vTaskDelay(15000 / portTICK_PERIOD_MS);
//	 }
//}

// -------------------------------------------------------------------------------------
void task_tx_gsm(void *ignore)
{
	 static const char *TX_TASK_TAG = "TX_TASK";
	 esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
	 vTaskDelay(2000 / portTICK_PERIOD_MS);

	 init_gsm();

	 while (1)
	 {
		 //sendData(TX_TASK_TAG, "Hello world\n\r");
		 vTaskDelay(15000 / portTICK_PERIOD_MS);
		 sendData(TX_TASK_TAG, "ATD+380931482354;\n\r");

//		 init_gsm();




	     //vTaskDelay(5000 / portTICK_PERIOD_MS);
	 }
}
// ----------------------------------------------------------------------------------------------
void task_rx_gsm(void *ignore)
{
	BaseType_t qStatus = false;
	gsm_ansver_buf gsm_ansver_t;

	static const char *RX_TASK_TAG = "RX_TASK";
	esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
	uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);

	while (1)
	{
		const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 10 / portTICK_PERIOD_MS);
		if (rxBytes > 0)
		{
			data[rxBytes] = 0;
			ESP_LOGI(RX_TASK_TAG, "------>Read %d bytes: '%s'", rxBytes, data);
//			ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);

			// Якщо прийшли дані в чергу, я

			qStatus = xQueueReceive(gsm_data_queue, &gsm_ansver_t, 1000/portTICK_PERIOD_MS);
			if(qStatus == pdPASS)
			{
				ESP_LOGI(RX_TASK_TAG, "RESEIVED ANSVER: %s", gsm_ansver_t.gsm_buf);

//				char buf_1[30] = {0,};
//				char buf_2[30] = {0,};
//				strcpy(buf_1, (char*)data);
//				strcpy(buf_2, gsm_ansver_t.gsm_buf);
//				ESP_LOGI(RX_TASK_TAG, "buf_1: %s", buf_1);
//				ESP_LOGI(RX_TASK_TAG, "buf_2: %s", buf_2);



				char *result = strstr((char*)data, (char*)gsm_ansver_t.gsm_buf);
//				char *result = strstr(buf_1, buf_2);
				if(result != NULL)
				{
					ESP_LOGI(RX_TASK_TAG, "OKOKOKOKOPKOKOKO");
				}
				else
				{
					ESP_LOGE(RX_TASK_TAG, "WRONG ANSVER!!!!!!!!!");
				}


			}

			// Input call
			char buf_ring[] = "+CLIP: \"+380931482354\"";
			char *result = strstr((char*)data, (char*)buf_ring);
			//				char *result = strstr(buf_1, buf_2);
			if(result != NULL)
			{
				sendData(RX_TASK_TAG, "ATA\n\r");
			}



	    }
	}
	free(data);
}
// ----------------------------------------------------------------------------------------------
void gsm(uint8_t status)
{
	static const char *GSM_LOG = "GSM";

	static uint8_t prev_status = 0;

	if(prev_status != status)
	{
		prev_status = status;

		if(status == 0)
		{
			ESP_LOGI(GSM_LOG, "GSM module OFF");

			vQueueDelete(gsm_data_queue);
			vTaskDelete(task_tx_gsm_handler);
			vTaskDelete(task_rx_gsm_handler);
			deinit_uart_for_gsm();
			turn_OFF_power_of_gsm_module();
		}
		else
		{
			ESP_LOGI(GSM_LOG, "GSM module ON");

			turn_ON_power_of_gsm_module();

			gsm_data_queue = xQueueCreate(5, sizeof(gsm_ansver_buf));
			init_uart_for_gsm();
			xTaskCreate(task_tx_gsm, "task_tx_gsm", 2048, NULL, configMAX_PRIORITIES - 1, &task_tx_gsm_handler);
			xTaskCreate(task_rx_gsm, "task_rx_gsm", 2048, NULL, configMAX_PRIORITIES - 1, &task_rx_gsm_handler);
		}
	}

}
// ----------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------
void app_main(void)
{
	init_output_gpio();

	RGB_LEDs_blink(5, 25);

	gsm(1);			// turn on GSM module by defoult


	Не піднімає вхідний звінок, не виводить в компорт що є вхідний звінок

	//xTaskCreate(task_tx_gsm, "task_tx_gsm", 4096, NULL, configMAX_PRIORITIES - 1, &task_tx_gsm_handler);


}
// ------------------------------------------------------------------------------------------
