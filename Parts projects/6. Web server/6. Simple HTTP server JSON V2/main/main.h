/*
 * main.h
 *
 *  Created on: May 18, 2023
 *      Author: odemki
 */

#ifndef MAIN_MAIN_H_
#define MAIN_MAIN_H_

#include <stdio.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs_flash.h"
// #include "led_strip.h"
#include "sdkconfig.h"
#include "esp_timer.h"		// software timers
#include "esp_log.h"
#include "esp_wifi_types.h"
#include "http.h"

#include "wifi.h"

#include "esp_spiffs.h"
#include "spiffs_config.h"


// ------------------------------------------------------------------------------------------
typedef struct {
	uint8_t hour;      /*!< Hour */
	uint8_t minute;    /*!< Minute */
	uint8_t second;    /*!< Second */
	uint16_t thousand; /*!< Thousand */
} time_gps;

typedef struct {
    uint8_t day;   /*!< Day (start from 1) */
    uint8_t month; /*!< Month (start from 1) */
    uint16_t year; /*!< Year (start from 2000) */
} date_gps;

typedef struct
{
	float latitude;
	float longitude;
	float altitude;
	float speed;
	time_gps time;
	date_gps date;
	uint8_t sats_in_view;
} gps_data_gps;
// ------------------------------------------------------------------------------------------
typedef struct {
	float temperature;
	float humidity;
	float preasuere;
} bme280_thp;
// ------------------------------------------------------------------------------------------

#endif /* MAIN_MAIN_H_ */
