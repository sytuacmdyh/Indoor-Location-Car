#ifndef __MOTOR_H
#define __MOTOR_H	 
#include "sys.h"


#define MOTOR_L_B PFout(0)// PF0
#define MOTOR_L_F PFout(1)// PF1
#define MOTOR_R_F PFout(2)// PF2
#define MOTOR_R_B PFout(3)// PF3

#define SPEED_STEP 20

extern u8 SPEED_DIF;
extern u8 MAX_DEGREE;
extern u8 START_AUST;

void car_init(void);//≥ı ºªØ
void car_forward(void);
void car_back(void);
void car_left(u8 degree);
void car_right(u8 degree);
void car_slow(void);
void car_fast(void);
void car_stop(void);
void car_set_speed(int s);

#endif
