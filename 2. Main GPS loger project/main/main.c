/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */


#include "main.h"


#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() 				 		// Налаштування таймера
#define configGENERATE_RUN_TIME_STATS 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configUSE_TRACE_FACILITY 1



// Task handlers
TaskHandle_t task_resurse_monitor_handlr;
TaskHandle_t task_bme280_handlr;
TaskHandle_t task_log_data_into_file_handlr;

// Queue
QueueHandle_t gps_data_log_queue = NULL;

UBaseType_t uxHighWaterMark;

// ------------------------------------------------------------------------------------------
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

		vTaskDelay(5000/portTICK_PERIOD_MS);
	}
}
// ------------------------------------------------------------------------------------------
void task_bme280(void *ignore)
{
	static const char *TAG_BME280 = "BME280";

	i2c_master_init_BME280();

	struct bme280_t bme280 = {
		.bus_write = BME280_I2C_bus_write,
		.bus_read = BME280_I2C_bus_read,
		.dev_addr = BME280_I2C_ADDRESS2,
		.delay_msec = BME280_delay_msek
	};

	s32 com_rslt;
	s32 v_uncomp_pressure_s32;
	s32 v_uncomp_temperature_s32;
	s32 v_uncomp_humidity_s32;

	com_rslt = bme280_init(&bme280);

	com_rslt += bme280_set_oversamp_pressure(BME280_OVERSAMP_16X);
	com_rslt += bme280_set_oversamp_temperature(BME280_OVERSAMP_2X);
	com_rslt += bme280_set_oversamp_humidity(BME280_OVERSAMP_1X);
	com_rslt += bme280_set_standby_durn(BME280_STANDBY_TIME_1_MS);
	com_rslt += bme280_set_filter(BME280_FILTER_COEFF_16);

	com_rslt += bme280_set_power_mode(BME280_NORMAL_MODE);

	if (com_rslt == SUCCESS)
	{
		while(true)
		{
			com_rslt = bme280_read_uncomp_pressure_temperature_humidity(
				&v_uncomp_pressure_s32, &v_uncomp_temperature_s32, &v_uncomp_humidity_s32);

			if (com_rslt == SUCCESS)
			{
				double t = bme280_compensate_temperature_double(v_uncomp_temperature_s32);
				double p = bme280_compensate_pressure_double(v_uncomp_pressure_s32)/100;
				double h = bme280_compensate_humidity_double(v_uncomp_humidity_s32);

				ESP_LOGI(TAG_BME280, "%.2f degC / %.3f hPa / %.3f %%", t, p, h);
			}
			else
			{
				ESP_LOGE(TAG_BME280, "measure error. code: %d", com_rslt);
			}

			vTaskDelay(5000/portTICK_PERIOD_MS);
		}
	}
	else
	{
		ESP_LOGE(TAG_BME280, "init or setting error. code: %d", com_rslt);
	}

	vTaskDelete(NULL);
}
// --------------------------------------------------------------------------------------------------------------------
void gps_signal_led_indication(gps_data_t *gps_data);

void gps_signal_led_indication(gps_data_t *gps_data)
{
	static const char *TAG_LOG = "GPS: ";
	static bool status = 0;
	if(gps_data->latitude == 0 )
	{
		gpio_set_level(CONFIG_GREEN_GPIO, 0);
		gpio_set_level(CONFIG_RED_GPIO, status);
		ESP_LOGI(TAG_LOG, "NO GPS DATA !!!");
	}
	else
	{
		gpio_set_level(CONFIG_RED_GPIO, 0);
		gpio_set_level(CONFIG_GREEN_GPIO, status);
		ESP_LOGI(TAG_LOG, "GPS DATA OK");
	}
	status = !status;
}
// --------------------------------------------------------------------------------------------------------------------
void show_gps_data(gps_data_t *gps_data)
{
	static const char *TAG_LOG = "GPS: ";

	ESP_LOGI(TAG_LOG, "---------- GPS data ----------");
	ESP_LOGI(TAG_LOG, "latitude: %.05f°N", gps_data->latitude);
	ESP_LOGI(TAG_LOG, "longitude: %.05f°E", gps_data->longitude);
	ESP_LOGI(TAG_LOG, "altitude: %.02fm", gps_data->altitude);
	ESP_LOGI(TAG_LOG, "speed: %.02fm", gps_data->speed);

	ESP_LOGI(TAG_LOG, "second: %d", gps_data->time.second);
	ESP_LOGI(TAG_LOG, "minute: %d", gps_data->time.minute);
	ESP_LOGI(TAG_LOG, "hour: %d", gps_data->time.hour);

	ESP_LOGI(TAG_LOG, "day: %d", gps_data->date.day);
	ESP_LOGI(TAG_LOG, "month: %d", gps_data->date.month);
	ESP_LOGI(TAG_LOG, "year: %d", gps_data->date.year);
	ESP_LOGI(TAG_LOG, "sats -in view: %d", gps_data->sats_in_view);
	ESP_LOGI(TAG_LOG, "---------------------------------");
}
// --------------------------------------------------------------------------------------------------------------------
void task_log_data_into_file(void *ignore)
{
	uint8_t log_data_save_period = 5;				// Period of lging data into Micro CD
	int gps_data_incoming_counter = 1;
	gps_data_t gps_data;
	BaseType_t qStatus = false;
	const char* base_path = "/data";
	int gps_point_counter = 0;

	char name[20] = {0,};
	get_file_name(&name);

	while(1)
	{
		qStatus = xQueueReceive(gps_data_log_queue, &gps_data, 1000/portTICK_PERIOD_MS);
		if(qStatus == pdPASS)
		{
			show_gps_data(&gps_data);
			gps_signal_led_indication(&gps_data);

			if(gps_data_incoming_counter == log_data_save_period)
			{
				static bool init = true;

				if(init == true)			//	If save data first time
				{
					ESP_ERROR_CHECK(example_mount_storage(base_path));
					log_data1(base_path, name, gps_data.latitude, gps_data.longitude);
					init = false;
				}
				else
				{
					log_data2(base_path, name, gps_data.latitude , gps_data.longitude, gps_point_counter++);
				}
			}
			gps_data_incoming_counter ++;
			if(gps_data_incoming_counter > log_data_save_period)
			{
				gps_data_incoming_counter = 0;
			}

		}
		vTaskDelay(500/portTICK_PERIOD_MS);
	}
}
// ------------------------------------------------------------------------------------------
void app_main(void)
{
	init_output_gpio();
	RGB_LEDs_blink(5, 25);

	xTaskCreate(task_resurse_monitor, "task_resurse_monitor", 4096, NULL, configMAX_PRIORITIES - 1, &task_resurse_monitor_handlr);
	xTaskCreate(task_bme280, "task_bme280", 2048, NULL, configMAX_PRIORITIES - 1, &task_bme280_handlr);
	xTaskCreate(task_log_data_into_file, "task_log_data_into_file", 4096, NULL, configMAX_PRIORITIES - 1, &task_log_data_into_file_handlr);

	gps_data_log_queue = xQueueCreate(5, sizeof(gps_data_t));
	turn_on_gps();
}
// ------------------------------------------------------------------------------------------

