/*
 * gsm_sim800l.c
 *
 *  Created on: Jan 17, 2024
 *      Author: odemki
 */

#include "gsm_sim800l.h"


static const char *GSM_TAG = "GSM";

//static const int RX_BUF_SIZE = 1024;

#define BUF_SIZE 1024

#define TXD_PIN (CONFIG_GSM_UART_TXD)
#define RXD_PIN (CONFIG_GSM_UART_RXD)

#define GPIO_POWER_GSM (1ULL<<CONFIG_POWER_GSM_GPIO)

// --------------------------------------------------------------------------------------------------
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GSM commands handlers
void command_1_turn_on_gps_log_handler(void)
{
	make_blink(3, 200, 10);
	
	gps_log_on();
}
// ------------------------------------------------------------------------------------------------------------
void command_2_turn_off_gps_log_handler(void)
{
	make_blink(3, 50, 10);
	
	gps_log_off();
}
// ------------------------------------------------------------------------------------------------------------
void command_3_send_one_point_gps_data_handler(void)
{
	make_blink(3, 20, 10);
	send_one_point_gps_data();
}
// ------------------------------------------------------------------------------------------------------------
void command_4_restart_handler(void)
{
	make_blink(3, 20, 10);
	restart_all_esp32();
}
// ------------------------------------------------------------------------------------------------------------
// Structure for command and handler
typedef struct{
	const char* command_name;
	void (*command_handler)(void);
} command_t;


command_t command_list[] = {
	{"logON", command_1_turn_on_gps_log_handler},
	{"logOFF", command_2_turn_off_gps_log_handler},
	{"point", command_3_send_one_point_gps_data_handler},
	{"restart", command_4_restart_handler},
	// get status
	// restart all dev
};

#define COMMAND_COUNT (sizeof(command_list)/sizeof(command_list[0]))		// How many commands defined

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------------------------
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
			deinit_uart_for_gsm();
			turn_OFF_power_of_gsm_module();
		}
		else
		{
			ESP_LOGI(GSM_LOG, "GSM module ON");
			turn_ON_power_of_gsm_module();
		}
	}
}
// ----------------------------------------------------------------------------------------------
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
	uart_param_config(UART_NUM_1, &uart_config);
	uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}
// ----------------------------------------------------------------------------------------------
void deinit_uart_for_gsm(void)
{
	uart_driver_delete(UART_NUM_1);
}
// ----------------------------------------------------------------------------------------------
void send_at_command(const char* command)
{
	ESP_LOGI("GSM", "Command to GSM: %s", command);
	uart_write_bytes(UART_NUM_1, command, strlen(command));
}
// ----------------------------------------------------------------------------------------------
void show_gsm_response(void)
{
	uint8_t data[BUF_SIZE];
	int lenght = uart_read_bytes(UART_NUM_1, data, BUF_SIZE, 1000/portTICK_PERIOD_MS);
	if(lenght > 0)
	{
		data[lenght] = '\0';
		ESP_LOGI("GSM", "Response from GSM: %s", data);
	}
}
// ---------------------------------------------------------------------------------------------
int read_gsm_response(uint8_t* buffer, int buffer_size)
{
	static const char *GSM_TAG = "GSM";
    int length = uart_read_bytes(UART_NUM_1, buffer, buffer_size, 1000 / portTICK_PERIOD_MS);
    if (length > 0)
    {
        buffer[length] = '\0';
        ESP_LOGI(GSM_TAG, "Response from GSM: %s", buffer);
    }
    return length;
}
// ----------------------------------------------------------------------------------------------
void init_sim800l(void)
{
	send_at_command("ATE0\r\n");						// Перевірка звязку з модулем
	vTaskDelay(500/portTICK_PERIOD_MS);
	show_gsm_response();
	
	if(send_at_command_read_ansver("AT\r\n", "OK") != 0)
	{
		ESP_LOGE(GSM_TAG, "GSM initialization failed at AT command");
        return;
	}
	if(send_at_command_read_ansver("AT+CMGF=1\r\n", "OK") != 0)
	{
		ESP_LOGE(GSM_TAG, "GSM initialization failed at AT+CMGF=1 command");
        return;
	}
	if(send_at_command_read_ansver("AT+CNMI=1,2,0,0,0\r\n", "OK") != 0)
	{
		ESP_LOGE(GSM_TAG, "GSM initialization failed at AT+CNMI=1,2,0,0,0 command");
        return;
	}
	if(send_at_command_read_ansver("AT+CPAS\r\n", "OK") != 0)
	{
		ESP_LOGE(GSM_TAG, "GSM initialization failed at AT+CPAS command");
        return;
	}
	if(send_at_command_read_ansver("AT+CSQ\r\n", "OK") != 0)
	{
		ESP_LOGE(GSM_TAG, "GSM initialization failed at AT+CSQ command");
        return;
	}
	if(send_at_command_read_ansver("AT+CLIP=1\r\n", "OK") != 0)
	{
		ESP_LOGE(GSM_TAG, "GSM initialization failed at AT+CLIP=1 command");
        return;
	}
	/*
		Тут перевіряти статус реєстрації в мережі 
	Перевірку рівня сигналу і перевірку готовності до прийому смс
	
	1. AT+CREG?    чекати поки модуль не зареєструється в мережі
	2. AT+CSQ
	3. AT+CPMS?		якщо память на СМС заповнен, то видалити всі
	*/
	
	if(send_at_command_read_ansver("AT+CREG?\r\n", "OK") != 0)
	{
		ESP_LOGE(GSM_TAG, "GSM initialization failed at AT+CREG? command");
        return;
	}
	// Check memory 
	if(send_at_command_read_ansver("AT+CPMS?\r\n", "OK") != 0)
	{
		ESP_LOGE(GSM_TAG, "GSM initialization failed at AT+CREG? command");
        return;
	}
	 
	 // ЯКЩО ІНІЦІАЦІЯ ПРОЙШЛА ОК, ТОДІ ВІДПРАВИТИ СМС З ПОВІДОМЛЕННЯМ ЩО ДІВАС ГОТОВИЙ  <<<<<<<<<<<<<
	 
	 
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	
	
	
		
	ESP_LOGI(GSM_TAG, "GSM module initialized successfully");
}
// ----------------------------------------------------------------------------------------------
void process_sms(const char* sms)
{
	for (int i = 0; i < COMMAND_COUNT; i++)
	{
		if(strstr(sms, command_list[i].command_name) != NULL)
		{
			ESP_LOGI(GSM_TAG, "Executing %s", command_list[i].command_name);
			command_list[i].command_handler();
			return;
		}
	}
	ESP_LOGI(GSM_TAG, "Wrong SMS command");
}
// ----------------------------------------------------------------------------------------------
void wait_for_sms(void)
{
	while(1)
	{
		uint8_t data[BUF_SIZE];
		int length = uart_read_bytes(UART_NUM_1, data, BUF_SIZE, 1000/portTICK_PERIOD_MS);
		if(length > 0)
		{
			data[length] = '\0';
			ESP_LOGI(GSM_TAG, "New SMS: %s", data);
			process_sms((char*)data);
		}
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}
// ----------------------------------------------------------------------------------------------
void answer_call(void)
{
	send_at_command("ATA\n\r");
}
// ----------------------------------------------------------------------------------------------
void hungup_call(void)
{
	send_at_command("ATH\r\n");					// End of call
}
// ----------------------------------------------------------------------------------------------
void check_for_call(void)
{
	
	while(1)
	{
		uint8_t data[BUF_SIZE];
		int length = uart_read_bytes(UART_NUM_1, data, BUF_SIZE, 1000/portTICK_PERIOD_MS);
		if(length > 0)
		{
			data[length] = '\0';
			ESP_LOGI(GSM_TAG, "New message:  %s", data);
			if(strstr((char*)data, "RING") != NULL)
			{
				ESP_LOGI(GSM_TAG, "Incoming call");
				answer_call();
			}
		}
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}
// ----------------------------------------------------------------------------------------------
void send_sms(const char* phone_number, const char* message)
{
	char command[50];
	sprintf(command, "AT+CMGS=\"%s\"\r\n", phone_number);
	send_at_command(command);
	vTaskDelay(1000/portTICK_PERIOD_MS);
	send_at_command(message);
	send_at_command("\x1A");		// End of SMS
}
// --------------------------------------------------------------------------------------------------
int send_at_command_read_ansver(char* at_command, char* expected_response)
{
	send_at_command(at_command);
	
	int buff_size = 1024;
	uint8_t response_buff[buff_size];
	int response_lenght = 0;
	int count = 1; 
	
	while((response_lenght == 0) && (count <= 10))
	{
		response_lenght = read_gsm_response(response_buff, buff_size);
		if(response_lenght > 0)
		{
			if(strstr((char*)response_buff, expected_response) != NULL)
			{
				ESP_LOGI(GSM_TAG, "Expected response found: %s", expected_response);
				return 0;
			}
			else
			{
				ESP_LOGE(GSM_TAG, "Expected response NOT found !!!");
				return 1;
			}
		}
		vTaskDelay(10/portTICK_PERIOD_MS);
		count++;
	}
	ESP_LOGE(GSM_TAG, "No response from GSM !!!");
	return 1;
}
// --------------------------------------------------------------------------------------------------



