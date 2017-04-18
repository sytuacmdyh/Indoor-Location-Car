#ifndef __MYSERVO_H
#define __MYSERVO_H	 
#include "sys.h"

#define MIN_PULSE_WIDTH       690     // the shortest pulse sent to a servo  
#define MAX_PULSE_WIDTH      2390     // the longest pulse sent to a servo   Ô½´óÔ½Íù×ó
#define DEFAULT_PULSE_WIDTH  (MAX_PULSE_WIDTH-MIN_PULSE_WIDTH)/2     // default pulse width when servo is attached

void Servo_Init(void);
void Servo_Set_Degree(int degree);

#endif
