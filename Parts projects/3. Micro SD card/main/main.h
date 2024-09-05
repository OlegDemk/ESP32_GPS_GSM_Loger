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

// ------------------------------------------------------
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "leds_gpio/gpio.h"
// ------------------------------------------------------
#include "microsd/mount.h"

#define ON 1
#define OFF 0


#endif /* MAIN_MAIN_H_ */
