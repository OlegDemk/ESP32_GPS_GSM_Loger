/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */


#include "main.h"

// Task handlers
TaskHandle_t task_test_micro_sd_card_handler;

UBaseType_t uxHighWaterMark;

// ------------------------------------------------------------------------------------------
void task_test_micro_sd_card(void* ignore)
{
	char test_write_data[100] = "111222333";

	const char* base_path = "/data";
	ESP_ERROR_CHECK(example_mount_storage(base_path));

	log_data("/data", "test1234.txt", test_write_data,  "writing BME280 data");

	while(1)
	{
		 vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

// ------------------------------------------------------------------------------------------
void app_main(void)
{
	init_output_gpio();
	RGB_LEDs_blink(5, 25);

	xTaskCreate(task_test_micro_sd_card, "task_test_micro_sd_card", 4096, NULL, configMAX_PRIORITIES - 1, &task_test_micro_sd_card_handler);

}
// ------------------------------------------------------------------------------------------
