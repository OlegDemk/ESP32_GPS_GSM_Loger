/*
 * main_gps.c
 *
 *  Created on: Dec 25, 2023
 *      Author: odemki
 */



#include "main_gps.h"

/////////////////////////////////////////////////////////////////////////////////////////

#define TIME_ZONE (+2)   //Beijing Time
#define YEAR_BASE (2000) //date in GPS starts from 2000

extern QueueHandle_t gps_data_log_queue;
//extern QueueHandle_t gps_data_html_queue;
//extern QueueHandle_t spiffs_size_queue;



//nmea_parser_config_t config;
//nmea_parser_handle_t nmea_hdl;

/////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief GPS Event Handler
 *
 * @param event_handler_arg handler specific arguments
 * @param event_base event base, here is fixed to ESP_NMEA_EVENT
 * @param event_id event id
 * @param event_data event specific arguments
 */
static void gps_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	static const char *TAG = "GPS: ";

    gps_t *gps = NULL;

    switch (event_id) {
    case GPS_UPDATE:
        gps = (gps_t *)event_data;
        /* print information parsed from GPS statements */
        ESP_LOGI(TAG, "%d/%d/%d %d:%d:%d => \r\n"
                 "\t\t\t\t\t\tlatitude   = %.05f°N\r\n"
                 "\t\t\t\t\t\tlongitude = %.05f°E\r\n"
                 "\t\t\t\t\t\taltitude   = %.02fm\r\n"
                 "\t\t\t\t\t\tspeed      = %fm/s",
                 gps->date.year + YEAR_BASE, gps->date.month, gps->date.day,
                 gps->tim.hour + TIME_ZONE, gps->tim.minute, gps->tim.second,
				 gps->latitude, gps->longitude, gps->altitude, gps->speed);

        break;
    case GPS_UNKNOWN:
        /* print unknown statements */
        ESP_LOGW(TAG, "Unknown statement:%s", (char *)event_data);
        break;
    default:
        break;
    }
}
// --------------------------------------------------------------------------------------------------

nmea_parser_handle_t nmea_hdl = NULL;
//nmea_parser_config_t config = NULL;

void turn_on_gps(void)
{
	init_on_off_gps_gpio();
	turn_on_gps_module();
	nmea_parser_config_t config = NMEA_PARSER_CONFIG_DEFAULT();	    /* NMEA parser configuration */
	nmea_hdl = nmea_parser_init(&config);		/* init NMEA parser library */
	nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL);		/* register event handler for NMEA parser library */
}
// --------------------------------------------------------------------------------------------------
void turn_off_gps(void)
{
	init_on_off_gps_gpio();
	turn_off_gps_module();
	nmea_parser_remove_handler(nmea_hdl, gps_event_handler);		/* unregister event handler */
	nmea_parser_deinit(nmea_hdl);									//    /* deinit NMEA parser library */
}
// --------------------------------------------------------------------------------------------------

#define GPIO_POWER_GPS (1ULL<<CONFIG_POWER_GPS_GPIO)

void init_on_off_gps_gpio(void)
{
	gpio_config_t io_conf = {};		// structure for configure GPIOs
	// Configure output PIN
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = GPIO_POWER_GPS;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
}
// --------------------------------------------------------------------------------------------------
void turn_on_gps_module(void)
{
	gpio_set_level(CONFIG_POWER_GPS_GPIO, 0);
}
// --------------------------------------------------------------------------------------------------
void turn_off_gps_module(void)
{
	gpio_set_level(CONFIG_POWER_GPS_GPIO, 1);
}
// --------------------------------------------------------------------------------------------------

