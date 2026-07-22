#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#include "stm32f10x.h"

void ultrasonic_init(void);
uint32_t ultrasonic_get_distance_mm(void);

#endif
