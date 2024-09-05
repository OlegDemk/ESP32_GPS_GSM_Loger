/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */



#include "main.h"

// Task handlers


// ------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------
void app_main(void)
{
	init_output_gpio();
	RGB_LEDs_blink(5, 25);

	vTaskDelay(1000/portTICK_PERIOD_MS);
	turn_on_gps();
	vTaskDelay(2000/portTICK_PERIOD_MS);
	turn_off_gps();
	vTaskDelay(2000/portTICK_PERIOD_MS);
	turn_on_gps();
}
// ------------------------------------------------------------------------------------------
