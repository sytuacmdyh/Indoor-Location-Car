#include "motor.h"
#include "inv_mpu.h"

//speed 调整幅度
u8 SPEED_DIF=5;
u8 MAX_DEGREE=90;//微调最大限度
u8 START_AUST=2;

#define STOP	0
#define FORWARD 1
#define BACK	2
#define LEFT	3
#define RIGHT	4

//转弯速度
u8 TURN_SPEED=120;

u8 cur_task=STOP;//当前状态
float raw_yaw=0;//动作开始前的航向角
float tar_yaw=0;//目的航向角

//现TIM3 计数初值为255
int speed=150;//约80<=speed<=255转

u8 get_speed(int speed,int l,int r)
{
	if(speed<l)
		return l;
	if(speed>r)
		return r;
	return speed;
}

void TIM3_PWM_Init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	//库函数禁止JTAG方法
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//使能定时器3时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); //Timer3完全重映射  TIM3_CH2->Pc7  TIM3_CH1->Pc6
 
   //设置该引脚为复用输出功能,输出TIM3 CH1 CH2的PWM脉冲波形	TIM3_CH2->Pc7  TIM3_CH1->Pc6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIO

   //初始化TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//初始化TIM3 Channel 1,2 PWM模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性高
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC2
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性高
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC2

	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_Cmd(TIM3, ENABLE);  //使能TIM3
}

void car_forward(){
	
	cur_task=FORWARD;
	raw_yaw=tar_yaw=yaw;
	
	TIM_Cmd(TIM5,ENABLE);//开启定时器5
	MOTOR_L_F=1;
	MOTOR_L_B=0;
	MOTOR_R_F=1;
	MOTOR_R_B=0;
	TIM_SetCompare1(TIM3,speed);
	TIM_SetCompare2(TIM3,speed);
}
void car_back(){
	
	cur_task=BACK;
	raw_yaw=tar_yaw=yaw;
	
	TIM_Cmd(TIM5,ENABLE);//开启定时器5
	MOTOR_L_F=0;
	MOTOR_L_B=1;
	MOTOR_R_F=0;
	MOTOR_R_B=1;
	TIM_SetCompare1(TIM3,speed);
	TIM_SetCompare2(TIM3,speed);
}
void turn_left()
{
	MOTOR_L_F=0;
	MOTOR_L_B=0;
	MOTOR_R_F=1;
	MOTOR_R_B=0;
	TIM_SetCompare1(TIM3,TURN_SPEED);
	TIM_SetCompare2(TIM3,TURN_SPEED);
}
void turn_right()
{
	MOTOR_L_F=1;
	MOTOR_L_B=0;
	MOTOR_R_F=0;
	MOTOR_R_B=0;
	TIM_SetCompare1(TIM3,TURN_SPEED);
	TIM_SetCompare2(TIM3,TURN_SPEED);
}
void car_left(u8 degree){
	if(degree>180)
		return;
	cur_task=LEFT;
	raw_yaw=yaw;
	tar_yaw=raw_yaw+degree;
	if(tar_yaw>180)
	{
		tar_yaw-=360;
	}
	
	TIM_Cmd(TIM5,ENABLE);//开启定时器5
}
void car_right(u8 degree){
	if(degree>180)
		return;
	cur_task=RIGHT;
	raw_yaw=yaw;
	tar_yaw=raw_yaw-degree;
	if(tar_yaw<-180)
	{
		tar_yaw+=360;
	}
	
	TIM_Cmd(TIM5,ENABLE);//开启定时器5
}
void car_set_speed(int s)
{
	s=(255-80)*1.0*s/10.0+80;
	speed=get_speed(s,80,255);
	TIM_SetCompare1(TIM3,speed);
	TIM_SetCompare2(TIM3,speed);
}
void car_slow(){
	speed-=SPEED_STEP;
	speed=get_speed(speed,80,255);
	TIM_SetCompare1(TIM3,speed);
	TIM_SetCompare2(TIM3,speed);
}
void car_fast(){
	speed+=SPEED_STEP;
	speed=get_speed(speed,80,255);
	TIM_SetCompare1(TIM3,speed);
	TIM_SetCompare2(TIM3,speed);
}

void car_stop(void){
	
	cur_task=STOP;
	raw_yaw=tar_yaw=yaw;
	
	TIM_Cmd(TIM5,DISABLE);//关闭定时器5
	MOTOR_L_F=0;
	MOTOR_L_B=0;
	MOTOR_R_F=0;
	MOTOR_R_B=0;
	TIM_SetCompare1(TIM3,0);
	TIM_SetCompare2(TIM3,0);
}

void left_more()//往左微调
{
	if(TIM3->CCR2-TIM3->CCR1<MAX_DEGREE)
	{
		TIM3->CCR1=get_speed(TIM3->CCR1-SPEED_DIF,80,255);//左轮减小一点速度
		TIM3->CCR2=get_speed(TIM3->CCR2+SPEED_DIF,80,255);//右轮增加一点速度
	}
}

void right_more()//往右微调
{
	if(TIM3->CCR1-TIM3->CCR2<MAX_DEGREE)
	{
		TIM3->CCR1=get_speed(TIM3->CCR1+SPEED_DIF,80,255);//左轮增加一点速度
		TIM3->CCR2=get_speed(TIM3->CCR2-SPEED_DIF,80,255);//右轮减小一点速度
	}
}

//定时器5中断服务程序		    
void TIM5_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)//是更新中断
	{
		//todo
		switch(cur_task)
		{
			case STOP:
				TIM_Cmd(TIM5,DISABLE);//关闭定时器5
				break;
			case FORWARD:
				if(yaw>tar_yaw+START_AUST)
					right_more();
				else if(yaw<tar_yaw-START_AUST)
					left_more();
				break;
			case LEFT:
				if(yaw<tar_yaw-START_AUST||yaw>tar_yaw+START_AUST)
					turn_left();
				else
					car_stop();
				break;
			case RIGHT:
				if(yaw<tar_yaw-START_AUST||yaw>tar_yaw+START_AUST)
					turn_right();
				else
					car_stop();
				break;
			case BACK:
				if(yaw>tar_yaw+START_AUST)
					left_more();
				else if(yaw<tar_yaw-START_AUST)
					right_more();
				break;
			default:
				break;
		}
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update  );  //清除TIM5更新中断标志    
	}
}

void TIM5_Int_Init(u16 arr,u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);//TIM5时钟使能    
	
	//定时器TIM5初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE ); //使能指定的TIM5中断,允许更新中断
	
	TIM_Cmd(TIM5,ENABLE);//开启定时器5
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
}

//初始化PF1 2 3 4
//小车IO初始化
void car_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	TIM3_PWM_Init(255,573);	 //PWM频率=72000000/((255+1)*(573+1))约为490HZ，与arduino一致

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);	 //使能PF端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;						//LED0-->PB.5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口速度为50MHz
	GPIO_Init(GPIOF, &GPIO_InitStructure);					 		//根据设定参数初始化GPIOB.5

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	TIM5_Int_Init(500-1,7200-1);//50ms
	car_stop();
}

