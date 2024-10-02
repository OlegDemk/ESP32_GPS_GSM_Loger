/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "main.h"

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() 				 		// Timer settings
#define configGENERATE_RUN_TIME_STATS 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configUSE_TRACE_FACILITY 1

#define SMS_FITBACK ON

static const char *AUTHORIZED_NUMBER = "+380931482354";

// Task handlers
TaskHandle_t task_resurse_monitor_handlr;
TaskHandle_t task_bme280_handlr;
TaskHandle_t task_log_data_into_file_handlr;
TaskHandle_t task_led_blink_handler;
TaskHandle_t task_gsm_handler;
TaskHandle_t task_get_gps_data_one_time_handler;

// Queue
QueueHandle_t gps_data_log_queue = NULL;

UBaseType_t uxHighWaterMark;

// ------------------------------------------------------------------------------------------
void task_blink(void *ignore)	// For debug
{

	while(1)
	{
		gpio_set_level(CONFIG_BLUE_GPIO, 1);
		vTaskDelay(100 / portTICK_PERIOD_MS);
		gpio_set_level(CONFIG_BLUE_GPIO, 0);
		vTaskDelay(900 / portTICK_PERIOD_MS);
	}
}
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

		vTaskDelay(15000/portTICK_PERIOD_MS);
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

			vTaskDelay(10000/portTICK_PERIOD_MS);
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
int check_gps_data_valid(gps_data_t *gps_data);

int check_gps_data_valid(gps_data_t *gps_data)
{
	static const char *TAG_LOG = "GPS: ";
	static bool status = 0;
	
	if(gps_data->latitude == 0 )
	{	
		return 0;
	}
	else
	{
		return 1;
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
bool init_gps_status_flag = false;
bool gps_log_working_flag = false;

void task_log_data_into_file(void *ignore)
{
	static const char *LOG_TAG = "LOG GPS DATA";
	
	uint8_t log_data_save_period = 5;				// Period of lging data into Micro CD.
	int gps_data_incoming_counter = 1;			
	int gps_point_counter = 0;						// Point counter, for save into file.
	bool init = true;								// For create new file(first time) or add new GPS data to old file.
	
	gps_data_t gps_data;
	BaseType_t qStatus = false;
	
	const char* base_path = "/data";
	
	// Get name for new file (using flash memory)
	char name[20] = {0,};
	get_file_name(&name);
	
	if(init_gps_status_flag == false)				// If GPS module was OFF
	{
		turn_on_gps();
		init_gps_status_flag = true;
	}
	
	#if SMS_FITBACK == ON
		send_sms(AUTHORIZED_NUMBER, "GPS log Start...");
	#endif
	
	// Mount microsd card only one time peer work time.
	static bool mount_smicrosd = false;
	if(mount_smicrosd == false)
	{	  
		ESP_ERROR_CHECK(example_mount_storage(base_path));
		ESP_LOGI(LOG_TAG, "MOUNTING MICROSD CARD");
		mount_smicrosd = true;
	}

	while(1)
	{
		qStatus = xQueueReceive(gps_data_log_queue, &gps_data, 1000/portTICK_PERIOD_MS);
		if(qStatus == pdPASS)
		{
			show_gps_data(&gps_data);
			gps_signal_led_indication(&gps_data);

			if(gps_data_incoming_counter == log_data_save_period)		// SaveGPS data periodically
			{
				if(init == true)										//	If save data first time
				{
					log_data1(base_path, name, gps_data.latitude, gps_data.longitude);
					init = false;
				}
				else
				{
					log_data2(base_path, name, gps_data.latitude , gps_data.longitude, gps_point_counter++);
				}
			}
			gps_data_incoming_counter++;
			if(gps_data_incoming_counter > log_data_save_period)
			{
				gps_data_incoming_counter = 0;
			}
		}
		vTaskDelay(500/portTICK_PERIOD_MS);
	}
}
// ----------------------------------------------------------------------------------------------
void task_get_gps_data_one_time(void* ignode)
{
	static const char *GSM_TAG = "GPS";
	
	gps_data_t gps_data;
	BaseType_t qStatus = false;
	int status = 0;
	
	#if SMS_FITBACK == ON
		send_sms(AUTHORIZED_NUMBER, "One point GPS START...");
	#endif
	
	ESP_LOGI(GSM_TAG, "GET ONE SHOT GPS DATA...");
	
	vTaskDelay(10000 / portTICK_PERIOD_MS);					// ПОтрібна затримка між відсиланнями SMS біля 10 секунд
	
	if(init_gps_status_flag == false)						// If GPS module in OFF state
	{
		ESP_LOGI(GSM_TAG, "TURN ON GPS MODULE !!!! --------------------------------------------");    // FOR DEBUG 
		turn_on_gps(); 
		init_gps_status_flag = true;
	}
	
	while(1)
	{
		qStatus = xQueueReceive(gps_data_log_queue, &gps_data, 1000/portTICK_PERIOD_MS);
		if(qStatus == pdPASS)
		{
			gps_signal_led_indication(&gps_data);
			show_gps_data(&gps_data);
			
			int status = check_gps_data_valid(&gps_data); 
			if(status == 1)									// if GPS data valid 
			{
				ESP_LOGI(GSM_TAG, "GSM data valid ");
				char buff[50] = {0,};
				
				sprintf(buff, "%05f, %05f", gps_data.latitude, gps_data.longitude);
				//send sms with GPS data
				#if SMS_FITBACK == ON
					ESP_LOGI(GSM_TAG, "SENGING SMS WITH DATA: %s <<<<<<<<<<<<<<<<", buff);			// FOR DEBUG 
					send_sms(AUTHORIZED_NUMBER, buff); 		// BUG: Коли працює логування GPS даних, не відбувається надсилання SMS з координатами
				#endif
				ESP_LOGI(GSM_TAG, "Data send: %s <<<<<<<<<<<<<<<<", buff);
		
				vTaskDelay(1000 / portTICK_PERIOD_MS);

				// якщо дані не логуються постійно тоді вимкнути GPS модуль
				if(gps_log_working_flag == false)
				{
					ESP_LOGI(GSM_TAG, "TURN OFF GPS MODULE !!!! --------------------------------------------");    // FOR DEBUG 
					turn_off_gps();
					init_gps_status_flag = false;
				}
				gpio_set_level(CONFIG_GREEN_GPIO, 0);
				
				ESP_LOGI(GSM_TAG, "Delete one shot GPS log task");
				vTaskDelete(NULL);
				
			}
		}
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
void gps_log_on(void)
{
	ESP_LOGI("SMS command", "LOG ON, command from SMS");
	
	if(gps_log_working_flag == false)
	{
		gps_log_working_flag = true;
	
		xTaskCreate(task_log_data_into_file, "task_log_data_into_file", 4096, NULL, configMAX_PRIORITIES - 1, &task_log_data_into_file_handlr);
		gps_data_log_queue = xQueueCreate(5, sizeof(gps_data_t));
	}
	else
	{
		#if SMS_FITBACK == ON
			send_sms(AUTHORIZED_NUMBER, "GPS log was started!");
		#endif
	}
}
// ------------------------------------------------------------------------------------------
void gps_log_off(void)
{
	ESP_LOGI("SMS command", "LOG OFF, command from SMS");
	
	if(gps_log_working_flag == true)
	{
		#if SMS_FITBACK == ON
			send_sms(AUTHORIZED_NUMBER, "GPS log STOP");
		#endif
	
		// Delete log task
		if(task_log_data_into_file_handlr != NULL)
		{
			vTaskDelete(task_log_data_into_file_handlr);
			task_log_data_into_file_handlr = NULL;
		}	
		// Delete queue
		if(gps_data_log_queue != NULL)
		{
			vQueueDelete(gps_data_log_queue);
			gps_data_log_queue = NULL;
		}
	
		turn_off_gps();
		gpio_set_level(CONFIG_GREEN_GPIO, 0);
		init_gps_status_flag = false;
		gps_log_working_flag = false;
	}
	else
	{
		#if SMS_FITBACK == ON
			send_sms(AUTHORIZED_NUMBER, "GPS log was stop!");
		#endif
	}
	
}
// ------------------------------------------------------------------------------------------
void send_one_point_gps_data(void) 
{
	ESP_LOGI("SMS command", "Send one point GPS data, command from SMS");
	
	xTaskCreate(task_get_gps_data_one_time, "task_get_gps_data_one_time", 4096, NULL, configMAX_PRIORITIES - 1, &task_get_gps_data_one_time_handler);
	gps_data_log_queue = xQueueCreate(5, sizeof(gps_data_t));
}
// ------------------------------------------------------------------------------------------
void restart_all_esp32(void)
{
	turn_off_gps_module();
	#if SMS_FITBACK == ON
		send_sms(AUTHORIZED_NUMBER, "Restarting...");
	#endif

	esp_restart();
}
// ------------------------------------------------------------------------------------------
void app_main(void)
{
	init_output_gpio();
	RGB_LEDs_blink(5, 25);

	xTaskCreate(task_blink, "task_blink", 1024, NULL, configMAX_PRIORITIES - 1, &task_led_blink_handler);
	xTaskCreate(task_resurse_monitor, "task_resurse_monitor", 4096, NULL, configMAX_PRIORITIES - 1, &task_resurse_monitor_handlr);
	xTaskCreate(task_bme280, "task_bme280", 2048, NULL, configMAX_PRIORITIES - 1, &task_bme280_handlr);
	xTaskCreate(task_gsm, "task_gsm", 4096, NULL, configMAX_PRIORITIES - 1, &task_gsm_handler);
	
	
	/*
	BUG: 
	
	
	
	TODO: code refactoring
	TODO: wifi web server storage
	TODO: Sleep Modes
	TODO: MQTT 

	
	TODO: Добавити до мітки number Point? мітку часу взяту з GPS, і швидкості
	BUG: бага, файли рандомно обрізаються, і дані пишуться в нові файли ??? 
	
	Life тариф "Гаджет безпека"
	
	*/
}
// ------------------------------------------------------------------------------------------

