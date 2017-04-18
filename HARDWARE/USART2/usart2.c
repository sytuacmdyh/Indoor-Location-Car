#include "delay.h"
#include "usart2.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"
#include "lcd.h"
#include "lcd.h"
#include "motor.h"

//串口接收缓存区
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//接收缓冲,最大USART2_MAX_RECV_LEN个字节.
u8 USART2_TX_BUF[USART2_MAX_SEND_LEN]; 			//发送缓冲,最大USART2_MAX_SEND_LEN字节
u8 BLUETOOTH_MESSAGE_BUF[USART2_MAX_RECV_LEN];
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
vu16 BLUETOOTH_MESSAGE_LEN;

//通过判断接收连续2个字符之间的时间差不大于10ms来决定是不是一次连续的数据.
//如果2个字符接收间隔超过10ms,则认为不是1次连续数据.也就是超过10ms没有接收到
//任何数据,则表示此次接收完毕.
//接收到的数据状态
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
vu16 USART2_RX_STA=0;
u8	BLUETOOTH_INIT_FLAG;
vu8	CTR_OK;
u8	GETTING_INFO_FLAG;

void _analyse(char * str)
{
	if(BLUETOOTH_INIT_FLAG)
	{
		LCD_Clear(WHITE);
		LCD_Show_Help_str(30,50,16,str);
		if(strstr(str,"OK")||strstr(str,"ERROR:(17)"))
			CTR_OK=TRUE;
	}
	else//处理蓝牙连接成功之后发来的消息，暂未用
	{
		
	}
	USART2_RX_STA=0;//标记处理完成
}

void USART2_IRQHandler()
{
	u8 res;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//接收到数据
	{	 
		res =USART_ReceiveData(USART2);
		if((USART2_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{
			if(USART2_RX_STA<USART2_MAX_RECV_LEN)	//还可以接收数据
			{
				USART2_RX_BUF[USART2_RX_STA++]=res;	//记录接收到的值
				if(res=='\n')
				{
					USART2_RX_BUF[USART2_RX_STA]=0;
					USART2_RX_STA|=1<<15;
					printf("%s",USART2_RX_BUF);//转发到usart1
					if(!GETTING_INFO_FLAG)
						_analyse((char*)USART2_RX_BUF);
					else if(!(BLUETOOTH_MESSAGE_LEN&(1<<15)))
					{
						strcpy((char *)BLUETOOTH_MESSAGE_BUF+BLUETOOTH_MESSAGE_LEN,(char *)USART2_RX_BUF);
						BLUETOOTH_MESSAGE_LEN+=USART2_RX_STA&0x7FFF;
						if(strstr((char *)USART2_RX_BUF,"OK\r\n"))
						{
							BLUETOOTH_MESSAGE_LEN|=1<<15;//标志接收完成
						}
					}
					USART2_RX_STA=0;
				}
			}else 
			{
				USART2_RX_STA|=1<<15;				//强制标记接收完成
				USART2_RX_BUF[USART2_RX_STA&0x7FFF]=0;
				printf("%s",USART2_RX_BUF);//转发到usart1
				if(!GETTING_INFO_FLAG)
					_analyse((char*)USART2_RX_BUF);
				else if(!(BLUETOOTH_MESSAGE_LEN&(1<<15)))
				{
					USART2_RX_BUF[USART2_RX_STA]=0;//添加结束符
					strcpy((char *)BLUETOOTH_MESSAGE_BUF+BLUETOOTH_MESSAGE_LEN,(char *)USART2_RX_BUF);
					BLUETOOTH_MESSAGE_LEN+=USART2_RX_STA;
					if(strstr((char *)USART2_RX_BUF,"OK\r\n"))
					{
						BLUETOOTH_MESSAGE_LEN|=1<<15;//标志接收完成
					}
				}
				USART2_RX_STA=0;
			} 
		}
	}  				 											 
}   


//初始化IO 串口2
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率
void usart2_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //串口2时钟使能

 	USART_DeInit(USART2);  //复位串口2
	//USART2_TX   PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化
	
	//USART2_RX	  PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PB11
	
	USART_InitStructure.USART_BaudRate = bound;//波特率一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  
	USART_Init(USART2, &USART_InitStructure); //初始化串口	3
	USART_Cmd(USART2, ENABLE);                    //使能串口 
	//使能接收中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	USART2_RX_STA=0;		//清零
}

//串口3,printf 函数
//确保一次发送数据不超过USART2_MAX_SEND_LEN字节
void u2_printf(char* fmt,...)  
{
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)USART2_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART2_TX_BUF);		//此次发送数据的长度
	for(j=0;j<i;j++)							//循环发送数据
	{
	  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
		USART_SendData(USART2,USART2_TX_BUF[j]); 
	}
}

void init_HC05()
{
	BLUETOOTH_INIT_FLAG=TRUE;
	
	CTR_OK=FALSE;
	while(!CTR_OK)
	{
		u2_printf("AT\r\n");
		delay_ms(1500);
	}
	
	CTR_OK=FALSE;
	while(!CTR_OK)
	{
		u2_printf("AT+INIT\r\n");
		delay_ms(1500);
	}
	
	CTR_OK=FALSE;
	while(!CTR_OK)
	{
		u2_printf("AT+IAC=9E8B33\r\n");
		delay_ms(1500);
	}
	
	CTR_OK=FALSE;
	while(!CTR_OK)
	{
		u2_printf("AT+CLASS=0\r\n");
		delay_ms(1500);
	}
	
	CTR_OK=FALSE;
	while(!CTR_OK)
	{
		u2_printf("AT+INQM=1,5,3\r\n");
		delay_ms(1500);
	}
	
	BLUETOOTH_INIT_FLAG=FALSE;
	GETTING_INFO_FLAG=FALSE;
	BLUETOOTH_MESSAGE_LEN=0;
}

u8 get_bluetooth_info()
{
	if(!GETTING_INFO_FLAG && !BLUETOOTH_INIT_FLAG)
	{
		GETTING_INFO_FLAG=TRUE;
		u2_printf("AT+INQ\r\n");
		return TRUE;
	}
	return FALSE;
}

u8 undo_get_bluetooth_info()
{
	int count=0;
	BLUETOOTH_INIT_FLAG=TRUE;
	CTR_OK=FALSE;
	while(!CTR_OK)
	{
		count++;
		if(count>=10)
			break;
		u2_printf("AT+INQC\r\n");
		delay_ms(1500);
	}
	BLUETOOTH_INIT_FLAG=FALSE;
	return CTR_OK?TRUE:FALSE;
}

