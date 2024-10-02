/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "main.h"



static const char *TAG = "main";
// -----------------------------------------------------------------
#define GPIO_OUTPUT_PIN_SEL (1ULL<<CONFIG_BLINK_GPIO)
#define GPIO_OUTPUT_PIN_SEL_RED (1ULL<<CONFIG_RED_GPIO)
#define GPIO_OUTPUT_PIN_SEL_GREEN (1ULL<<CONFIG_GREEN_GPIO)
#define GPIO_OUTPUT_PIN_SEL_BLUE (1ULL<<CONFIG_BLUE_GPIO)

//RED_GPIO
//GREEN_GPIO
//BLUE_GPIO
// -----------------------------------------------------------------
void init_output_gpio(void)
{
	gpio_config_t io_conf = {};		// structure for configure GPIOs
	// Configure output PIN
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL | GPIO_OUTPUT_PIN_SEL_RED | GPIO_OUTPUT_PIN_SEL_GREEN | GPIO_OUTPUT_PIN_SEL_BLUE;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);

	gpio_set_level(CONFIG_RED_GPIO, 1);
	gpio_set_level(CONFIG_GREEN_GPIO, 1);
	gpio_set_level(CONFIG_BLUE_GPIO, 1);

	vTaskDelay(300 / portTICK_PERIOD_MS);

	gpio_set_level(CONFIG_RED_GPIO, 0);
	gpio_set_level(CONFIG_GREEN_GPIO, 0);
	gpio_set_level(CONFIG_BLUE_GPIO, 0);
}

// ----------------------------------------------------------------
void init_ipsffs_memory(void)
{
	ESP_LOGI(TAG, "Initializing SPIFFS");

	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 5,
		.format_if_mount_failed = true
	};

	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		}
		else if (ret == ESP_ERR_NOT_FOUND)
		{
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		}
		else
		{
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
		return;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(conf.partition_label, &total, &used);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
	}
	else
	{
		ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
	}

//	// Open HTML file and print
//	FILE* f;
//	ESP_LOGI(TAG, "Opening file");
//	f = fopen("/spiffs/index.html", "rb");
//	if (f == NULL)
//	{
//		ESP_LOGE(TAG, "Failed to open file for writing");
//		return;
//	}
//
//	char str1[500];
//	size_t n;
//	n = fread(str1, 1, sizeof(str1), f);
//	fclose(f);
//	str1[n] = 0;
//
//	ESP_LOGI(TAG, "Read from file: \r\n%s", str1);
//	ESP_LOGI(TAG, "----------------------------------------------------\r\n");


}
// ----------------------------------------------------------------
void app_main(void)
{
	init_output_gpio();

	init_ipsffs_memory();

	esp_err_t ret  = 0;

	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	    ret = nvs_flash_erase();
	    ESP_LOGI(TAG, "nvs_flash_erase: 0x%04x", ret);
	    ret = nvs_flash_init();
	    ESP_LOGI(TAG, "nvs_flash_init: 0x%04x", ret);
	}
	ESP_LOGI(TAG, "nvs_flash_init: 0x%04x", ret);

	ret = esp_netif_init();
	ESP_LOGI(TAG, "esp_netif_init: %d", ret);
	ret = esp_event_loop_create_default();
	ESP_LOGI(TAG, "esp_event_loop_create_default: %d", ret);

	ret = wifi_init_sta();
	ESP_LOGI(TAG, "wifi_init_sta: %d", ret);



	printf("OK\n\r");
}
// ----------------------------------------------------------------















//
