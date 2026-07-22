#ifndef __MYCAR_H
#define __MYCAR_H

void mycar_init(void);
unsigned char car_get_default_speed(void);
void car_set_speed(unsigned char speed_percent);
void car_set_speed_lr(unsigned char left_speed_percent, unsigned char right_speed_percent);
unsigned char car_get_speed(void);
void go(void);
void stop(void);
void car_forward(void);
void car_backward(void);
void car_left(void);
void car_right(void);

#endif
