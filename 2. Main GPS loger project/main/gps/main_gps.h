/*
 * main_gps.h
 *
 *  Created on: Dec 25, 2023
 *      Author: odemki
 */

#ifndef MAIN_GPS_MAIN_GPS_H_
#define MAIN_GPS_MAIN_GPS_H_

#include "../main.h"

void turn_on_gps(void);
void turn_off_gps(void);
void gps_log_task(void *arg);

void init_on_off_gps_gpio(void);
void turn_on_gps_module(void);
void turn_off_gps_module(void);


#endif /* MAIN_GPS_MAIN_GPS_H_ */
