/*
 * gpio.c
 *
 *  Created on: Dec 28, 2023
 *      Author: odemki
 */

#include "leds_gpio.h"
//#include "../main.h"

// -----------------------------------------------------------------
#define GPIO_OUTPUT_PIN_SEL_RED (1ULL<<CONFIG_RED_GPIO)
#define GPIO_OUTPUT_PIN_SEL_GREEN (1ULL<<CONFIG_GREEN_GPIO)
#define GPIO_OUTPUT_PIN_SEL_BLUE (1ULL<<CONFIG_BLUE_GPIO)

// -----------------------------------------------------------------
void init_output_gpio(void)
{
	gpio_config_t io_conf = {};		// structure for configure GPIOs
	// Configure output PIN
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL_RED | GPIO_OUTPUT_PIN_SEL_GREEN | GPIO_OUTPUT_PIN_SEL_BLUE;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
}
// -----------------------------------------------------------------
void RGB_LEDs_blink(int times, int delay)
{
	for(int i = 0; i < 10; i++)
	{
		gpio_set_level(CONFIG_RED_GPIO, 1);
		gpio_set_level(CONFIG_GREEN_GPIO, 1);
		gpio_set_level(CONFIG_BLUE_GPIO, 1);
		vTaskDelay(100 / portTICK_PERIOD_MS);

		gpio_set_level(CONFIG_RED_GPIO, 0);
		gpio_set_level(CONFIG_GREEN_GPIO, 0);
		gpio_set_level(CONFIG_BLUE_GPIO, 0);
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}
// -----------------------------------------------------------------
//void gps_signal_led_indication(gps_data_t *gps_data)
//{
//	static const char *TAG_LOG = "GPS: ";
//	static bool status = 0;
//	if(gps_data->latitude == 0 )
//	{
//		gpio_set_level(CONFIG_GREEN_GPIO, 0);
//		gpio_set_level(CONFIG_RED_GPIO, status);
//		ESP_LOGI(TAG_LOG, "NO GPS DATA !!!");
//	}
//	else
//	{
//		gpio_set_level(CONFIG_RED_GPIO, 0);
//		gpio_set_level(CONFIG_GREEN_GPIO, status);
//		ESP_LOGI(TAG_LOG, "GPS DATA OK");
//	}
//	status = !status;
//}
// -----------------------------------------------------------------
