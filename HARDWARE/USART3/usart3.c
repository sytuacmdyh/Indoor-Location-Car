#include "delay.h"
#include "usart3.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "lcd.h"
#include "motor.h"
#include "usart2.h"
#include "mp3player.h"
#include "vs10xx.h"
#include "beep.h"
#include "sr04.h"
#include "servo.h"

#define is_number(a) ((a)>='0'&&(a)<='9')

//串口接收缓存区
u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				//接收缓冲,最大USART3_MAX_RECV_LEN个字节.
u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 			//发送缓冲,最大USART3_MAX_SEND_LEN字节

//数据采集任务
u8 RUNNING_TASK_FLAG=0;
u8 task_count=0;
u8 cur_index=0;
u32 delay_task_second_counter=0;
char tasks[20][10];

//通过判断接收连续2个字符之间的时间差不大于10ms来决定是不是一次连续的数据.
//如果2个字符接收间隔超过10ms,则认为不是1次连续数据.也就是超过10ms没有接收到
//任何数据,则表示此次接收完毕.
//接收到的数据状态
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
vu16 USART3_RX_STA=0;

void analyse(char * str)
{
	int t;
	int len=strlen(str);
	char * p;
	//在采集过程中，不接受除了停止采集命令之外的任何命令
	if(RUNNING_TASK_FLAG){
		if(str[0]=='S' && str[1]=='T' && str[2]=='O' && str[3]=='P'){
			RUNNING_TASK_FLAG=0;
			car_stop();
		}
		return;
	}
	//执行数据采集任务命令 (样例：   TASK:F2,L180,R10,    )
	if(str[0]=='T' && str[1]=='A' && str[2]=='S' && str[3]=='K' && str[4]==':'){
		RUNNING_TASK_FLAG=1;
		memset(tasks,0,sizeof(tasks));
		task_count=0;
		p=str+5;
		cur_index=0;
		t=0;
		while(*p){
			if(*p==','){
				task_count++;
				tasks[task_count][t++]=0;
				t=0;
			}
			else{
				if(t<5 && task_count<20)//防止越界
					tasks[task_count][t++]=*p;
			}
			p++;
		}
//		//test 接收
//		u3_printf("%d\n",task_count);
//		for(t=0;t<=task_count;t++){
//			u3_printf("%s\n",tasks[t]);
//		}
	}
	else if(str[0]=='L' && str[1]=='D' && is_number(str[2]))//向左旋转指定角度
	{
		t=atoi(str+2);
		u3_printf("%d\n",t);
		if(t>=0&&t<=180)
			car_left(t);
	}
	else if(str[0]=='R' && str[1]=='D' && is_number(str[2]))//向右旋转指定角度
	{
		t=atoi(str+2);
		u3_printf("%d\n",t);
		if(t>=0&&t<=180)
			car_right(t);
	}
	else if(str[0]=='F' && str[1]=='D' && is_number(str[2]))//向前前进单位长度
	{
		t=atoi(str+2);
		u3_printf("%d\n",t);
		if(t>=0&&t<=180)
			car_forward(t);
	}
	//test
	else if(str[0]=='A'&&str[1]=='D'){
		ADJUST_DEALY=atoi(str+2);
		u3_printf("\nset ADJUST_DEALY ms:%d",TURN_SPEED);
	}
	else if(str[0]=='T'&&str[1]=='S'){
		TURN_SPEED=atoi(str+2);
		u3_printf("\nset turn speed:%d",TURN_SPEED);
	}
	else if(str[0]=='S'&&str[1]=='D'){
		SPEED_DIF=atoi(str+2);
		u3_printf("\nset SPEED_DIF:%d",SPEED_DIF);
	}
	//不带参协议
	if(strcmp(HELLO,str)==0)
	{
		strcpy(song_path,"0:/MUSIC/HELLO.mp3");
	}
	else if(strcmp("reset_dir",str)==0){
		reset_dir();
	}
}

//定时器7中断服务程序
void TIM7_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)//是更新中断
	{
		USART3_RX_STA|=1<<15;				//强制标记接收完成
		USART3_RX_BUF[USART3_RX_STA&0x7FFF]=0;
		printf("%s",USART3_RX_BUF);//转发到usart1
		analyse((char*)USART3_RX_BUF);
		USART3_RX_STA=0;
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update  );  //清除TIM7更新中断标志
		TIM_Cmd(TIM7, DISABLE);  //关闭TIM7
	}
}

void TIM7_Int_Init(u16 arr,u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);//TIM7时钟使能

	//定时器TIM7初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位

	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE ); //使能指定的TIM7中断,允许更新中断

	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	TIM_Cmd(TIM7,ENABLE);//开启定时器7
}


void USART3_IRQHandler(void)
{
	u8 res;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//接收到数据
	{
		res =USART_ReceiveData(USART3);
		if((USART3_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{
			if(USART3_RX_STA<USART3_MAX_RECV_LEN)	//还可以接收数据
			{
				TIM_SetCounter(TIM7,0);//计数器清空          				//计数器清空
				if(USART3_RX_STA==0) 				//使能定时器7的中断
				{
					TIM_Cmd(TIM7,ENABLE);//使能定时器7
				}
				USART3_RX_BUF[USART3_RX_STA++]=res;	//记录接收到的值
			}
			else
			{
				USART3_RX_STA|=1<<15;				//强制标记接收完成
				USART3_RX_BUF[USART3_RX_STA&0x7FFF]=0;
				printf("%s",USART3_RX_BUF);//转发到usart1
				analyse((char*)USART3_RX_BUF);
			}
		}
	}
}


//初始化IO 串口3
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率
void usart3_init(u32 bound)
{

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); //串口3时钟使能

	USART_DeInit(USART3);  //复位串口3
	//USART3_TX   PB10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PB10

	//USART3_RX	  PB11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);  //初始化PB11

	USART_InitStructure.USART_BaudRate = bound;//波特率一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART3, &USART_InitStructure); //初始化串口	3


	USART_Cmd(USART3, ENABLE);                    //使能串口

	//使能接收中断
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断

	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器


	TIM7_Int_Init(1000-1,7200-1);		//100ms中断
	USART3_RX_STA=0;		//清零
	TIM_Cmd(TIM7,DISABLE);			//关闭定时器7
}

//串口3,printf 函数
//确保一次发送数据不超过USART3_MAX_SEND_LEN字节
void u3_printf(char* fmt,...)
{
	u16 i,j;
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)USART3_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART3_TX_BUF);		//此次发送数据的长度
	for(j=0; j<i; j++)							//循环发送数据
	{
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕
		USART_SendData(USART3,USART3_TX_BUF[j]);
	}
}

void atk_8266_quit_trans(void)
{
	while((USART3->SR&0X40)==0);	//等待发送空
	USART3->DR='+';
	delay_ms(15);					//大于串口组帧时间(10ms)
	while((USART3->SR&0X40)==0);	//等待发送空
	USART3->DR='+';
	delay_ms(15);					//大于串口组帧时间(10ms)
	while((USART3->SR&0X40)==0);	//等待发送空
	USART3->DR='+';
	delay_ms(500);					//等待500ms
}

void send_bluetooth_info()
{
	if(GETTING_INFO_FLAG && (BLUETOOTH_MESSAGE_LEN&(1<<15)))
	{
		u3_printf((char *)BLUETOOTH_MESSAGE_BUF);
		GETTING_INFO_FLAG=FALSE;
		BLUETOOTH_MESSAGE_LEN=0;
		USART2_RX_STA=0;
	}
}
