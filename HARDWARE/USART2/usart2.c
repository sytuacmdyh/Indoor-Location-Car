#include "usart2.h"
#include "usart3.h"
#include "exti.h"
#include "stdio.h"

//串口接收缓存区
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//接收缓冲,最大USART2_MAX_RECV_LEN个字节.
u8 USART2_TX_BUF[USART2_MAX_SEND_LEN]; 		//发送缓冲,最大USART3_MAX_SEND_LEN字节

//[14:0]:接收到的数据长度
//15 : 接收完成标志
u16 USART2_RX_STA=0;

#define             POTO_HEAD            0xAA // 0XFD       // 协议头
#define             POTO_TAIL            0X55 // 0XFF       // 协议尾
#define             POTO_TRANS           0XA5 // 0XFE       // 转义字符
u8 findHead=0;
u8 translate_flag=0;
u8 check_sum=0;

void start_search_ibeacon(){
	findHead=0;
	translate_flag=0;
	check_sum=0;
	USART2_RX_STA=0;
}

void USART2_IRQHandler()
{
	u8 res;
	int i;
	char uuid[80];
	int rssi=-1000;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//接收到数据
	{
		res =USART_ReceiveData(USART2);//这句话必须第一句执行...
		if(USART2_RX_STA & 0x8000)
			return;
//		//测试使用
//		USART2_RX_BUF[USART2_RX_STA++]=res;
//		if(res==POTO_TAIL){
//			for(i=0;i<USART2_RX_STA;i++){
//				u3_printf("%02X ",USART2_RX_BUF[i]);
//			}
//			//u3_printf("%s",(char *)USART2_RX_BUF);
//			USART2_RX_STA=0;
//		}
		if(translate_flag){
			USART2_RX_BUF[USART2_RX_STA++]=res;
			check_sum+=res;
			translate_flag=0;
		}
		else if(POTO_TRANS==res) {
			translate_flag=1;
		}
		else if(POTO_HEAD==res){
			check_sum = 0;
			USART2_RX_STA = 0;
			findHead = 1;
            translate_flag=0;
		}
		else if ( POTO_TAIL == res )
        {
			if(findHead && !check_sum && USART2_RX_STA==29)
			{
				//构造UUID
				for(i=0;i<16;i++){
					sprintf(uuid+i*3,"%02X ",USART2_RX_BUF[i+7]);
				}
				uuid[16*3-1]=0;
				
				//构造RSSI
				//very keng daddy
				rssi=USART2_RX_BUF[6];
				rssi^=0x000000FF;
				rssi+=1;
				rssi*=-1;
				
				sprintf((char *)USART2_TX_BUF,"{\"type\": \"ibeacon\",\"x\": %f,\"y\": %f,\"uuid\": \"%s\",\"rssi\": %d}\r\n",global_x,global_y,uuid,rssi);
				USART2_RX_STA|=0x8000;
			}
			else{
				start_search_ibeacon();
			}
		}
		else  if(findHead)
		{
			check_sum += res;
			USART2_RX_BUF[USART2_RX_STA++]=res;
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
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//USART2_RX	  PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = bound;//波特率一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  
	USART_Init(USART2, &USART_InitStructure);
	USART_Cmd(USART2, ENABLE);
	//使能接收中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART2_RX_STA=0;
}

