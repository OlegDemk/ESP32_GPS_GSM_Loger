/*
 * nain.h
 *
 *  Created on: Aug 17, 2024
 *      Author: odemki
 */

#ifndef MAIN_MAIN_H_
#define MAIN_MAIN_H_

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "string.h"
#include "stdint.h"
#include "esp_system.h"
// ------------------------------------------------------
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "bme280/bme280.h"
#include "bme280/main_bme280.h"
#include "leds_gpio/leds_gpio.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "microsd/mount.h"
#include "gps/main_gps.h"
#include "gps/nmea_parser.h"
// ------------------------------------------------------
#include "driver/uart.h"
#include "gsm/gsm_sim800l.h"

#define ON 1
#define OFF 0
// ------------------------------------------------------

typedef struct {
	uint8_t hour;      /*!< Hour */
	uint8_t minute;    /*!< Minute */
	uint8_t second;    /*!< Second */
	uint16_t thousand; /*!< Thousand */
} time_t;

typedef struct {
    uint8_t day;   /*!< Day (start from 1) */
    uint8_t month; /*!< Month (start from 1) */
    uint16_t year; /*!< Year (start from 2000) */
} date_t;

typedef struct
{
	float latitude;
	float longitude;
	float altitude;
	float speed;
	time_t time;
	date_t date;
	uint8_t sats_in_view;
} gps_data_t;


#endif /* MAIN_MAIN_H_ */
