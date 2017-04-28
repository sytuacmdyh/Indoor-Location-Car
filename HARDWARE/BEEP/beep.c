#include "beep.h"
#include "delay.h"

//��ʼ��PB8Ϊ�����.��ʹ������ڵ�ʱ��		    
//��������ʼ��
void BEEP_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��GPIOB�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //BEEP-->PB.8 �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //�ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);	 //���ݲ�����ʼ��GPIOB.8
	
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);//���0���رշ��������
}

void BEEP_DI()//��һ��
{
	BEEP=1;
	delay_ms(20);
	BEEP=0;
}

void BEEP_DI2(void)
{
	BEEP=1;
	delay_ms(20);
	BEEP=0;
	delay_ms(100);
	BEEP=1;
	delay_ms(20);
	BEEP=0;
}
