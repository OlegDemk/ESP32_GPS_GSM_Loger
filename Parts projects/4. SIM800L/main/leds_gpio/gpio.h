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
void init_on_off_gsm_gpio(void);
void make_blink(int color, int delay_ms, int times);

#endif /* MAIN_LEDS_GPIO_GPIO_H_ */
