/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */


#include "main.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include <esp_http_server.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "lwip/inet.h"

// Task handlers
TaskHandle_t task_bme280_handlr;
UBaseType_t uxHighWaterMark;

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

			// Unused stec size
			uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
			ESP_LOGI("Stack usage task_bme280 Task", "Remaining stack: %u", uxHighWaterMark);

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

// Wi-Fi configuration
#define WIFI_SSID CONFIG_ESP_WIFI_SSID
#define WIFI_PASS CONFIG_ESP_WIFI_PASSWORD

#define WIFI_CONNECTED_BIT BIT0
static EventGroupHandle_t s_wifi_event_group;

// Обробник подій Wi-Fi та IP
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) 
{
	static const char *TAG = "EVENT HANDLER";
	
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect();  // Почати підключення до Wi-Fi
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        ESP_LOGI(TAG, "Disconnected from Wi-Fi, reconnecting...");
        esp_wifi_connect();  // Спробувати перепідключитися
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: %s", ip4addr_ntoa((const ip4_addr_t*)&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);  // Підключено
    }
}
// --------------------------------------------------------------------------------------------------------------------
esp_err_t hello_get_handler(httpd_req_t *req) 
{
    const char *response = "Hello, World!";
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
// --------------------------------------------------------------------------------------------------------------------
void start_webserver(void) 
{
	static const char *TAG = "START WEBSERVER";
	
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) 
    {
        httpd_uri_t hello_uri = {
            .uri = "/hello",
            .method = HTTP_GET,
            .handler = hello_get_handler
        };
        httpd_register_uri_handler(server, &hello_uri);
    } 
    else 
    {
        ESP_LOGI(TAG, "Error starting server!");
    }
}
// --------------------------------------------------------------------------------------------------------------------
// Wi-Fi initialization and connection function
void wifi_init_sta(void)
{
	static const char *TAG = "WIFI INIT STA";
 	 // Ініціалізація NVS
    ESP_ERROR_CHECK(nvs_flash_init());
    
    s_wifi_event_group = xEventGroupCreate();

    // Ініціалізація Wi-Fi
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Реєстрація обробників подій
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,  					// SSID з sdkconfig
            .password = CONFIG_ESP_WIFI_PASSWORD 	 		// Пароль з sdkconfig
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));  						// Налаштування ESP32 як станції
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));  		// Конфігурація Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_start());  										// Старт Wi-Fi

    ESP_LOGI(TAG, "Wi-Fi initialization completed. Waiting for connection...");
    
    // Очікувати на підключення
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to Wi-Fi");
}
// --------------------------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------
void app_main(void)
{
	init_output_gpio();
	RGB_LEDs_blink(5, 25);
	xTaskCreate(task_bme280, "task_bme280", 2048, NULL, configMAX_PRIORITIES - 1, &task_bme280_handlr);
	
	
	//WiFi
	nvs_flash_init();
	wifi_init_sta();
	start_webserver();
}
// ------------------------------------------------------------------------------------------
