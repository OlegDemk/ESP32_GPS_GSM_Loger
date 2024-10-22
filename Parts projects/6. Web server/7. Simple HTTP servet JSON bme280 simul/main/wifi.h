/*
 * wifi.h
 *
 *  Created on: May 18, 2023
 *      Author: odemki
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_

#include <stdio.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "http.h"


esp_err_t wifi_init_sta(void);

#endif /* MAIN_WIFI_H_ */
