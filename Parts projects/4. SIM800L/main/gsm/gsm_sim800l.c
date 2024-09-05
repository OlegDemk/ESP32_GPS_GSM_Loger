/*
 * gsm_sim800l.c
 *
 *  Created on: Jan 17, 2024
 *      Author: odemki
 */

#include "gsm_sim800l.h"

extern QueueHandle_t gsm_data_queue;

//static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (CONFIG_GSM_UART_TXD)
#define RXD_PIN (CONFIG_GSM_UART_RXD)


void init_uart_for_gsm(void)
{
	const uart_config_t uart_config = {
	    .baud_rate = 115200,
	    .data_bits = UART_DATA_8_BITS,
	    .parity = UART_PARITY_DISABLE,
	    .stop_bits = UART_STOP_BITS_1,
	    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	    .source_clk = UART_SCLK_DEFAULT,
	};
	// We won't use a buffer for sending data.
	uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
	uart_param_config(UART_NUM_1, &uart_config);
	uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}
// ----------------------------------------------------------------------------------------------
void deinit_uart_for_gsm(void)
{
	uart_driver_delete(UART_NUM_1);
}
// ----------------------------------------------------------------------------------------------
int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}
// ----------------------------------------------------------------------------------------------
int send_command_to_gsm(char* command, char* ansver)
{
	static const char* GSM_LOG_TAG = "---> GSM ";

	gsm_ansver_buf gsm_ansver_t;

	sendData(GSM_LOG_TAG, command);				// відіслати команду до GSM модуля

	// передати в чергу до задачі, відповідь яка повинна прийти від модуля, для підтвердження виконання AT команди модулем
	strcpy(gsm_ansver_t.gsm_buf, ansver);
	ESP_LOGI(GSM_LOG_TAG, "must be OK----------------->: %s", gsm_ansver_t.gsm_buf);
	xQueueSendToBack(gsm_data_queue, &gsm_ansver_t, 0);




	char buf_rx[30] = {0,};





	return 1;
}
// ----------------------------------------------------------------------------------------------
int init_gsm(void)
{
	static const char* GSM_LOG_TAG = "---> GSM ";

	send_command_to_gsm("AT\n\r", "OK");
	vTaskDelay(50 / portTICK_PERIOD_MS);

	send_command_to_gsm("ATE 0\n\r", "OK");
	vTaskDelay(50 / portTICK_PERIOD_MS);

	send_command_to_gsm("AT+CSMINS?\n\r", "+CSMINS: 0,1");
	vTaskDelay(50 / portTICK_PERIOD_MS);

	send_command_to_gsm("AT+CLIP=1\n\r", "OK");
	vTaskDelay(50 / portTICK_PERIOD_MS);


	return 1;
}

// --------------------------------------------------------------------------------------------------
void turn_on_gsm_module(void)
{
	gpio_set_level(CONFIG_POWER_GSM_GPIO, 0);
}
// --------------------------------------------------------------------------------------------------
void turn_off_gsm_module(void)
{
	gpio_set_level(CONFIG_POWER_GSM_GPIO, 1);
}
// --------------------------------------------------------------------------------------------------
void turn_ON_power_of_gsm_module(void)
{
	init_on_off_gsm_gpio();
	turn_on_gsm_module();
}
// --------------------------------------------------------------------------------------------------
void turn_OFF_power_of_gsm_module(void)
{
	init_on_off_gsm_gpio();
	turn_off_gsm_module();
}
// --------------------------------------------------------------------------------------------------

#define GPIO_POWER_GSM (1ULL<<CONFIG_POWER_GSM_GPIO)

void init_on_off_gsm_gpio(void)
{
	gpio_config_t io_conf = {};		// structure for configure GPIOs
	// Configure output PIN
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = GPIO_POWER_GSM;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
}
// --------------------------------------------------------------------------------------------------





















//// ----------------------------------------------------------------------------------------------
//#define GPIO_POWER_GSM (1ULL<<CONFIG_POWER_GSM_GPIO)
//
//void init_on_off_gsm_gpio(void)
//{
//	gpio_config_t io_conf = {};		// structure for configure GPIOs
//	// Configure output PIN
//	io_conf.intr_type = GPIO_INTR_DISABLE;
//	io_conf.mode = GPIO_MODE_OUTPUT;
//	io_conf.pin_bit_mask = GPIO_POWER_GSM;
//	io_conf.pull_down_en = 0;
//	io_conf.pull_up_en = 0;
//	gpio_config(&io_conf);
//
////	turn_off_gsm_module();
//}
//// --------------------------------------------------------------------------------------------------
//void turn_on_gsm_module(void)
//{
////	init_on_off_gsm_gpio();
//	gpio_set_level(GPIO_POWER_GSM, 0);
//}
//// --------------------------------------------------------------------------------------------------
//void turn_off_gsm_module(void)
//{
////	init_on_off_gsm_gpio();
//	gpio_set_level(GPIO_POWER_GSM, 1);
//}
//// --------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------

