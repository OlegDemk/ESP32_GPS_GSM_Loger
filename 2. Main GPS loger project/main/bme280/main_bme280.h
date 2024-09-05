/*
 * main_bme280.h
 *
 *  Created on: Dec 25, 2023
 *      Author: odemki
 */

#ifndef MAIN_BME280_MAIN_BME280_H_
#define MAIN_BME280_MAIN_BME280_H_

#include "../main.h"

void i2c_master_init_BME280(void);
s8 BME280_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
s8 BME280_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
void BME280_delay_msek(u32 msek);


#endif /* MAIN_BME280_MAIN_BME280_H_ */
