/*
 * gsm_sim800l.h
 *
 *  Created on: Jan 17, 2024
 *      Author: odemki
 */

#ifndef MAIN_GSM_GSM_SIM800L_H_
#define MAIN_GSM_GSM_SIM800L_H_

#include "../main.h"

static const int RX_BUF_SIZE = 1024;


void init_uart_for_gsm(void);
void deinit_uart_for_gsm(void);
//void turn_on_gsm_module(void);
//void turn_off_gsm_module(void);
void turn_ON_power_of_gsm_module(void);
void turn_OFF_power_of_gsm_module(void);

void init_sim800l(void);
void check_for_call(void);
void wait_for_sms(void);
void answer_call(void);
void send_at_command(const char* command);
void process_sms(const char* sms);
void show_gsm_response(void);
int read_gsm_response(uint8_t* buffer, int buffer_size);
void gsm(uint8_t status);
int send_at_command_read_ansver(char* at_command, char* expected_response);

#endif /* MAIN_GSM_GSM_SIM800L_H_ */
