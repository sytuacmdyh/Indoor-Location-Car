#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
//#include "lcd.h"
#include "usart.h"
#include "usart3.h"
#include "usart2.h"
#include "w25qxx.h"
#include "vs10xx.h"
#include "malloc.h"
#include "exfuns.h"
#include "fontupd.h"
#include "motor.h"
#include "beep.h"
#include "sr04.h"
#include "mp3player.h"
#include "servo.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "exti.h"
#include "stdlib.h"

void init()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	delay_init();	    	 //��ʱ������ʼ��
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
	usart2_init(115200);
	usart3_init(115200);
	LED_Init();			     //LED�˿ڳ�ʼ��
	BEEP_Init();
	EXTIX_Init();
//	LCD_Init();
	W25QXX_Init();				//��ʼ��W25Q128
	VS_Init();	  				//��ʼ��VS1053 
//	SR04_Init();
//	Servo_Init();
 	
 	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
	f_mount(fs[0],"0:",1); 		//����SD�� 
 	f_mount(fs[1],"1:",1); 		//����FLASH.
	
	car_init();
	VS_Set_Volum(4);
	mp3_init();
	
	while(MPU_Init())//��ʼ��MPU6050
	{
		u3_printf("MPU6050 init Error");
	}
	while(mpu_dmp_init())
 	{
		u3_printf("MPU6050 dmp Error");
	}
}

void init_car(){
	init_weel();//��ʼ��С�����ӵ�������������Ӧ��
	while(!init_ok()){
		delay_ms(100);
	}
}

int main()
{
	//��ʼ��ģ��
	init();
	
	//��ʼ����ɣ���һ��
	BEEP_DI();
	
	//��ʼС��״̬
	init_car();

	//��ʼ����ɣ���2��
	BEEP_DI2();
	
	while(1)
	{
		//���ɼ�����
		if(RUNNING_TASK_FLAG && !(TIM5->CR1 & TIM_CR1_CEN) && global_seconds >=delay_task_second_counter){//�����������У�����С������ֹͣ״̬������ͨ����ʱ��5�жϣ� ,������ʱ���������
			switch(tasks[cur_index][0]){
				case 'F':
					car_forward(atoi(tasks[cur_index]+1));
					break;
				case 'L':
					car_left(atoi(tasks[cur_index]+1));
					break;
				case 'R':
					car_right(atoi(tasks[cur_index]+1));
					break;
				case 'D':
					delay_task_second_counter=global_seconds+atoi(tasks[cur_index]+1);
					break;
			}
			cur_index++;
			if(cur_index>=task_count){
				cur_index=0;
			}
		}
		
		if(RUNNING_TASK_FLAG){
			
			if(USART_RX_STA&0x8000){//wifi�źż�����
				u3_printf("%s",USART_TX_BUF);
				start_search_wifi();
			}
			if(USART2_RX_STA&0x8000){
				u3_printf("%s",USART2_TX_BUF);
				start_search_ibeacon();
			}
		}
		
		//�������ֲ���
		mp3_play();
		
		//�����������벢����
//		if(GET_DIS_FLAG)
//		{
//			SR04_get_distence();
//			send_distence_info();
//			GET_DIS_FLAG=0;
//		}
		
		if(UPDATE_OLA_FLAG)
		{
			LED0=!LED0;
			if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)//100000����ʱ45s   ����Ƶ�ʱ����һ�㡣����
			{
				MPU_Get_Accelerometer(&aacx,&aacy,&aacz);	//�õ����ٶȴ���������
				MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);	//�õ�����������
			}
			UPDATE_OLA_FLAG=0;
		}
	}
}
