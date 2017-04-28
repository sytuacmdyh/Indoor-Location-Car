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
	usart2_init(38400);//����ATģʽ������38400��ͨ��ģʽ9600
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
	
//	while(font_init()) 				//����ֿ�
//	{
//		LCD_ShowString(30,50,200,16,16,"Font Error!");
//		delay_ms(200);
//		LCD_Fill(30,50,240,66,WHITE);//�����ʾ
//	}
//	init_HC05();
	//init_wifi_sta_client_trans();
	car_init();
	VS_Set_Volum(4);
	mp3_init();
	
	while(MPU_Init())					//��ʼ��MPU6050
	{
//		LCD_ShowString(30,130,200,16,16,"MPU6050 init Error");
//		delay_ms(200);
//		LCD_Fill(30,130,239,130+16,WHITE);
// 		delay_ms(200);
		u3_printf("MPU6050 init Error");
	}
	while(mpu_dmp_init())
 	{
//		LCD_ShowString(30,130,200,16,16,"MPU6050 dmp Error");
//		delay_ms(200);
//		LCD_Fill(30,130,239,130+16,WHITE);
// 		delay_ms(200);
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
//	char str[20];
//	u32 t=0;
	
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
		
		
		//����bluetooth��Ϣ��ɣ����͸�������
//		send_bluetooth_info();
		
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
				//u3_printf();
			}
			UPDATE_OLA_FLAG=0;
		}
		
//		if(t>=100000)
//		{
//			LED1=!LED1;
//			sprintf(str,"yaw: %f\n roll: %f\n pitch: %f\n left: %hd\n right: %hd",yaw,roll,pitch,TIM3->CCR1,TIM3->CCR2);
//			LCD_Show_Help_str(30,50,16,str);//����y=-1������ʾ��ǰһ������һ��
//			sprintf(str,"ACC\r\n  x:%f\r\n  y:%f\r\n  z:%f\r\n",9.8*aacx*1.0/16384.0,9.8*aacy*1.0/16384.0,9.8*aacz*1.0/16384.0);
//			LCD_Show_Help_str(30,-1,16,str);//����y=-1������ʾ��ǰһ������һ��
//			sprintf(str,"GYR\nx:%d\ny:%d\nz:%d",gyrox,gyroy,gyroz);
//			LCD_Show_Help_str(30,-1,16,str);//����y =-1������ʾ��ǰһ������һ��
//			t=0;
//		}
//		t++;
	}
}
