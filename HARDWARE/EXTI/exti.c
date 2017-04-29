#include "exti.h"
#include "motor.h"
#include "beep.h"
#include "usart3.h"
#include "delay.h"
#include <math.h>
#include "inv_mpu.h"

#define WHEEL_PERIMETER 207
#define PI 3.1415927
//轮子周长 66*3.14 mm

u32 STEP=0;
float global_x=0,global_y=0;
u32 exti_task=0;

u8 init_weel_flag=0;

void init_weel(){
	init_weel_flag=1;
	car_forward(100);
}

u8 init_ok(){
	return init_weel_flag==0;
}

//外部中断初始化
void EXTIX_Init(void)
{
 
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟

	//GPIOF.4	  中断线以及中断初始化配置  下降沿触发	//KEY0
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOF,GPIO_PinSource4);
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_InitStructure.EXTI_Line=EXTI_Line4;
  	EXTI_Init(&EXTI_InitStructure);	  	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

  	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			//使能外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//子优先级3
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure); 
}

void EXTI4_IRQHandler(void)
{
	float temp_yaw=yaw*(PI/180.0);
	if(init_weel_flag){//初始化模式下
		car_stop();
		STEP=0;
		exti_task=0;
		init_weel_flag=0;
		u3_printf("ok");
	}
	else {
//		u3_printf("%d",exti_task);
		STEP++;
		if(cur_task==_FORWARD){
			global_x+=WHEEL_PERIMETER*cos(temp_yaw);
			global_y+=WHEEL_PERIMETER*sin(temp_yaw);
		}
		//u3_printf("x:%f y:%f yaw:%f\n",global_x,global_y, yaw);
		if(exti_task>0){
			exti_task--;
			if(exti_task<=0){
				car_stop();
			}
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line4);  //清除LINE4上的中断标志位  
}
 
