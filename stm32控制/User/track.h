#ifndef __TRACK_H
#define __TRACK_H

#include "stm32f10x.h"

void track_init(void);
uint8_t track_run_step(void);
uint8_t track_read_left(void);
uint8_t track_read_mid(void);
uint8_t track_read_right(void);
uint8_t track_should_hold(void);
void track_clear_hold(void);

#endif
