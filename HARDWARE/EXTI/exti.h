#ifndef __EXTI_H
#define __EXIT_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//外部中断 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   

extern u32 exti_task;//小车还有多少步要走
extern float global_x,global_y;//全局坐标

void init_weel(void);//初始化轮子位置在磁铁处
u8 init_ok(void);//返回小车是否已初始化成功，调用需要放在init_weel之后
void EXTIX_Init(void);//外部中断初始化
#endif

