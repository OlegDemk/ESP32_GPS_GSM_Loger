/*
 * gpio.h
 *
 *  Created on: Dec 28, 2023
 *      Author: odemki
 */

#ifndef MAIN_LEDS_GPIO_GPIO_H_
#define MAIN_LEDS_GPIO_GPIO_H_

#include "../main.h"

void init_output_gpio(void);
void RGB_LEDs_blink(int times, int delay);

#endif /* MAIN_LEDS_GPIO_GPIO_H_ */
