#ifndef __MOTOR_H
#define __MOTOR_H	 
#include "sys.h"


#define MOTOR_L_B PFout(0)// PF0
#define MOTOR_L_F PFout(1)// PF1
#define MOTOR_R_F PFout(2)// PF2
#define MOTOR_R_B PFout(3)// PF3

#define SPEED_STEP 20

//状态
#define _STOP	0
#define _FORWARD 1
#define _BACK	2
#define _LEFT	3
#define _RIGHT	4
#define _ADUEST	5

extern u8 SPEED_DIF;
extern u8 MAX_DEGREE;
extern u8 START_AUST;
extern u8 TURN_SPEED;
extern u8 ADJUST_DEALY;
extern u8 cur_task;//当前状态

void car_init(void);//初始化
void car_forward(u32 count);
void car_back(void);
void car_left(u8 degree);
void car_right(u8 degree);
void car_stop(void);
void reset_dir(void);//矫正前20秒的yaw偏移

#endif
