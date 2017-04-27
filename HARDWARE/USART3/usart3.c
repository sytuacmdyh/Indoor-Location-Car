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

//���ڽ��ջ�����
u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				//���ջ���,���USART3_MAX_RECV_LEN���ֽ�.
u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 			//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�

//ͨ���жϽ�������2���ַ�֮���ʱ������10ms�������ǲ���һ������������.
//���2���ַ����ռ������10ms,����Ϊ����1����������.Ҳ���ǳ���10msû�н��յ�
//�κ�����,���ʾ�˴ν������.
//���յ�������״̬
//[15]:0,û�н��յ�����;1,���յ���һ������.
//[14:0]:���յ������ݳ���
vu16 USART3_RX_STA=0;

void analyse(char * str)
{
	int t;
	int len=strlen(str);
	if(len==4 && str[0]=='S' && is_number(str[1]))
	{
		t=0;
		t+=str[1]-'0';
		t*=10;
		t+=str[2]-'0';
		t*=10;
		t+=str[3]-'0';
		if(t>=0&&t<=180)
			Servo_Set_Degree(t);
		
	}
	else if(len==3 && str[0]=='S' && str[1]=='P')//setspeed
	{
		if(str[2]>='0'&&str[2]<='9')
			car_set_speed(str[2]-'0');
	}
	else if(len==2 && str[0]=='V')//set VOL
	{
		if(str[1]>='0'&&str[1]<='9')
		{
			VS_Set_Volum(str[1]-'0');
			//printf("---------%d\n",vsset.mvol);
		}
	}
	else if(str[0]=='L' && str[1]=='D' && is_number(str[2]))//������תָ���Ƕ�
	{
		if(strlen(str)==5)
		{
			t=0;
			t+=str[2]-'0';
			t*=10;
			t+=str[3]-'0';
			t*=10;
			t+=str[4]-'0';
			u3_printf("%d\n",t);
			if(t>=0&&t<=180)
				car_left(t);
		}
	}
	else if(str[0]=='R' && str[1]=='D' && is_number(str[2]))//������תָ���Ƕ�
	{
		if(strlen(str)==5)
		{
			t=0;
			t+=str[2]-'0';
			t*=10;
			t+=str[3]-'0';
			t*=10;
			t+=str[4]-'0';
			u3_printf("%d\n",t);
			if(t>=0&&t<=180)
				car_right(t);
		}
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
	else if(strlen(str)==4 && str[0]=='T')//����С���Զ���������
	{
		SPEED_DIF=str[1]-'0';
		MAX_DEGREE=(str[2]-'0')*10;
		START_AUST=str[3]-'0';
	}
	//������Э��
	if(strcmp(HELLO,str)==0)
	{
		strcpy(song_path,"0:/MUSIC/HELLO.mp3");
	}
	else if(strcmp(FORWARD,str)==0)
	{
		car_forward();
	}
	else if(strcmp(FORWARD_SHORT,str)==0)
	{
		car_forward();
		delay_ms(MOVE_KEEP_TIME);
		car_stop();
	}
	else if(strcmp(BACK,str)==0)
	{
		car_back();
		strcpy(song_path,"0:/MUSIC/BACK.mp3");
	}
	else if(strcmp(BACK_SHORT,str)==0)
	{
		car_back();
		strcpy(song_path,"0:/MUSIC/BACK.mp3");
		delay_ms(MOVE_KEEP_TIME);
		car_stop();
	}
	else if(strcmp(LEFT,str)==0)
	{
		car_left(90);
	}
	else if(strcmp(LEFT_SHORT,str)==0)
	{
		car_left(90);
		delay_ms(MOVE_KEEP_TIME);
		car_stop();
	}
	else if(strcmp(RIGHT,str)==0)
	{
		car_right(90);
	}
	else if(strcmp(RIGHT_SHORT,str)==0)
	{
		car_right(90);
		delay_ms(MOVE_KEEP_TIME);
		car_stop();
	}
	else if(strcmp(SUB_SPEED,str)==0)
	{
		car_slow();
	}
	else if(strcmp(ADD_SPEED,str)==0)
	{
		car_fast();
	}
	else if(strcmp(STOP,str)==0)
	{
		car_stop();
	}
	else if(strcmp(QUIT_TRANS,str)==0)
	{
		atk_8266_quit_trans();
	}
	else if(strcmp(GET_BLUETOOTH_INFO,str)==0)
	{
		get_bluetooth_info();
	}
	else if(strcmp(UNDO_GET_BLUETOOTH_INFO,str)==0)
	{
		undo_get_bluetooth_info();
	}
	else if(strcmp(GET_DISTENCE,str)==0)
	{
		GET_DIS_FLAG=1;
	}
	else if(strcmp(SERVO_LEFT,str)==0)
	{
		Servo_Set_Degree(180);
	}
	else if(strcmp(SERVO_FORWARD,str)==0)
	{
		Servo_Set_Degree(90);
	}
	else if(strcmp(SERVO_RIGHT,str)==0)
	{
		Servo_Set_Degree(0);
	}
}

//��ʱ��7�жϷ������
void TIM7_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)//�Ǹ����ж�
	{
		USART3_RX_STA|=1<<15;				//ǿ�Ʊ�ǽ������
		USART3_RX_BUF[USART3_RX_STA&0x7FFF]=0;
		printf("%s",USART3_RX_BUF);//ת����usart1
		analyse((char*)USART3_RX_BUF);
		USART3_RX_STA=0;
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update  );  //���TIM7�����жϱ�־
		TIM_Cmd(TIM7, DISABLE);  //�ر�TIM7
	}
}

void TIM7_Int_Init(u16 arr,u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);//TIM7ʱ��ʹ��

	//��ʱ��TIM7��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM7�ж�,��������ж�

	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	TIM_Cmd(TIM7,ENABLE);//������ʱ��7
}


void USART3_IRQHandler(void)
{
	u8 res;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//���յ�����
	{
		res =USART_ReceiveData(USART3);
		if((USART3_RX_STA&(1<<15))==0)//�������һ������,��û�б�����,���ٽ�����������
		{
			if(USART3_RX_STA<USART3_MAX_RECV_LEN)	//�����Խ�������
			{
				TIM_SetCounter(TIM7,0);//���������          				//���������
				if(USART3_RX_STA==0) 				//ʹ�ܶ�ʱ��7���ж�
				{
					TIM_Cmd(TIM7,ENABLE);//ʹ�ܶ�ʱ��7
				}
				USART3_RX_BUF[USART3_RX_STA++]=res;	//��¼���յ���ֵ
			}
			else
			{
				USART3_RX_STA|=1<<15;				//ǿ�Ʊ�ǽ������
				USART3_RX_BUF[USART3_RX_STA&0x7FFF]=0;
				printf("%s",USART3_RX_BUF);//ת����usart1
				analyse((char*)USART3_RX_BUF);
			}
		}
	}
}


//��ʼ��IO ����3
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������
void usart3_init(u32 bound)
{

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// GPIOBʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); //����3ʱ��ʹ��

	USART_DeInit(USART3);  //��λ����3
	//USART3_TX   PB10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOB, &GPIO_InitStructure); //��ʼ��PB10

	//USART3_RX	  PB11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);  //��ʼ��PB11

	USART_InitStructure.USART_BaudRate = bound;//������һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(USART3, &USART_InitStructure); //��ʼ������	3


	USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���

	//ʹ�ܽ����ж�
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�����ж�

	//�����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���


	TIM7_Int_Init(1000-1,7200-1);		//100ms�ж�
	USART3_RX_STA=0;		//����
	TIM_Cmd(TIM7,DISABLE);			//�رն�ʱ��7
}

//����3,printf ����
//ȷ��һ�η������ݲ�����USART3_MAX_SEND_LEN�ֽ�
void u3_printf(char* fmt,...)
{
	u16 i,j;
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)USART3_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART3_TX_BUF);		//�˴η������ݵĳ���
	for(j=0; j<i; j++)							//ѭ����������
	{
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); //ѭ������,ֱ���������
		USART_SendData(USART3,USART3_TX_BUF[j]);
	}
}

void atk_8266_quit_trans(void)
{
	while((USART3->SR&0X40)==0);	//�ȴ����Ϳ�
	USART3->DR='+';
	delay_ms(15);					//���ڴ�����֡ʱ��(10ms)
	while((USART3->SR&0X40)==0);	//�ȴ����Ϳ�
	USART3->DR='+';
	delay_ms(15);					//���ڴ�����֡ʱ��(10ms)
	while((USART3->SR&0X40)==0);	//�ȴ����Ϳ�
	USART3->DR='+';
	delay_ms(500);					//�ȴ�500ms
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
