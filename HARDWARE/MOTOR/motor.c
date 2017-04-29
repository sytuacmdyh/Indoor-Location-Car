#include "motor.h"
#include "inv_mpu.h"
#include "usart3.h"
#include "delay.h"
#include "exti.h"

//speed ��������
u8 SPEED_DIF=2;
u8 START_AUST=1;
u8 ADJUST_DEALY=30;//���ε���

//ת���ٶ�
u8 TURN_SPEED=120;

u8 cur_task=_STOP;//��ǰ״̬
short raw_yaw=0;//������ʼǰ�ĺ����
short tar_yaw=0;//Ŀ�ĺ����
short last_yaw_dis;

//��TIM3 ������ֵΪ255
int speed_l=215;//ǰ������ �����ٶ�
int speed_r=130;//ǰ������ �����ٶ�

//��������
int temp_yaw;
u8 reverse_flag;
u8 error_count;
u8 adjust_count;
u8 total_adjust_count;

//180>yaw1,yaw2>-180
short yaw_dis(short yaw1,short yaw2){
	short ans;
	if(yaw1<yaw2){
		short t=yaw1;
		yaw1=yaw2;
		yaw2=t;
	}
	ans=yaw1-yaw2;
	if(ans>180){
		ans=360-ans;
	}
	return ans;
}

void reset_dir(){
	tar_yaw=raw_yaw=yaw;
	global_x=global_y=0;
}

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
	
	//�⺯����ֹJTAG����
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//ʹ�ܶ�ʱ��3ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC  | RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIO�����AFIO���ù���ģ��ʱ��
	
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); //Timer3��ȫ��ӳ��  TIM3_CH2->Pc7  TIM3_CH1->Pc6
 
   //���ø�����Ϊ�����������,���TIM3 CH1 CH2��PWM���岨��	TIM3_CH2->Pc7  TIM3_CH1->Pc6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIO

   //��ʼ��TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	//��ʼ��TIM3 Channel 1,2 PWMģʽ
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ը�
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM3 OC2
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ը�
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM3 OC2

	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //ʹ��TIM3��CCR2�ϵ�Ԥװ�ؼĴ���
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIM3
}
//basic
void set_run_speed(){
	TIM_SetCompare1(TIM3,speed_l);
	TIM_SetCompare2(TIM3,speed_r);
}
void set_turn_speed(){
	TIM_SetCompare1(TIM3,TURN_SPEED);
	TIM_SetCompare2(TIM3,TURN_SPEED);
}
void turn_left()
{
	MOTOR_L_F=0;
	MOTOR_L_B=0;
	MOTOR_R_F=1;
	MOTOR_R_B=0;
	set_turn_speed();
}
void turn_left_center_short()//�������붯
{
	MOTOR_L_F=0;
	MOTOR_L_B=1;
	MOTOR_R_F=1;
	MOTOR_R_B=0;
	set_turn_speed();
	delay_ms(ADJUST_DEALY);
	MOTOR_L_F=0;
	MOTOR_L_B=0;
	MOTOR_R_F=0;
	MOTOR_R_B=0;
}
void turn_left_reverse()
{
	MOTOR_L_F=0;
	MOTOR_L_B=0;
	MOTOR_R_F=0;
	MOTOR_R_B=1;
	set_turn_speed();
}
void turn_right()
{
	MOTOR_L_F=1;
	MOTOR_L_B=0;
	MOTOR_R_F=0;
	MOTOR_R_B=0;
	set_turn_speed();
}
void turn_right_center_short()
{
	MOTOR_L_F=1;
	MOTOR_L_B=0;
	MOTOR_R_F=0;
	MOTOR_R_B=1;
	set_turn_speed();
	delay_ms(ADJUST_DEALY);
	MOTOR_L_F=0;
	MOTOR_L_B=0;
	MOTOR_R_F=0;
	MOTOR_R_B=0;
}
void turn_right_reverse()
{
	MOTOR_L_F=0;
	MOTOR_L_B=1;
	MOTOR_R_F=0;
	MOTOR_R_B=0;
	set_turn_speed();
}
//extend
void car_forward(u32 count){
	exti_task=count;
	cur_task=_FORWARD;
	
	TIM_Cmd(TIM5,ENABLE);//������ʱ��5
	MOTOR_L_F=1;
	MOTOR_L_B=0;
	MOTOR_R_F=1;
	MOTOR_R_B=0;
	set_run_speed();
}
void car_back(){
	
	cur_task=_BACK;
	
	TIM_Cmd(TIM5,ENABLE);//������ʱ��5
	MOTOR_L_F=0;
	MOTOR_L_B=1;
	MOTOR_R_F=0;
	MOTOR_R_B=1;
	set_run_speed();
}
void start_adjust(){
	cur_task=_ADUEST;
	total_adjust_count=0;
	adjust_count=0;
	TIM_Cmd(TIM5,ENABLE);//������ʱ��5
}
void car_left(u8 degree){
	if(degree>180)
		return;
	cur_task=_LEFT;
	raw_yaw=yaw;
	tar_yaw=raw_yaw+degree;
	last_yaw_dis=0x3fff;
	reverse_flag=0;
	error_count=0;
	if(tar_yaw>180)
	{
		tar_yaw-=360;
	}
	
	TIM_Cmd(TIM5,ENABLE);//������ʱ��5
}
void car_right(u8 degree){
	if(degree>180)
		return;
	cur_task=_RIGHT;
	raw_yaw=yaw;
	tar_yaw=raw_yaw-degree;
	last_yaw_dis=0x3fff;
	reverse_flag=0;
	error_count=0;
	if(tar_yaw<-180)
	{
		tar_yaw+=360;
	}
	
	TIM_Cmd(TIM5,ENABLE);//������ʱ��5
}

void car_stop(void){
	
	cur_task=_STOP;
	
	//u3_printf("\nl:%d r:%d\n",speed_l,speed_r);
	//u3_printf("\ntar:%d yaw:%f\n",tar_yaw,yaw);
	
	TIM_Cmd(TIM5,DISABLE);//�رն�ʱ��5
	MOTOR_L_F=0;
	MOTOR_L_B=0;
	MOTOR_R_F=0;
	MOTOR_R_B=0;
	TIM_SetCompare1(TIM3,0);
	TIM_SetCompare2(TIM3,0);
}

void left_more()//����΢��
{
	speed_l=get_speed(TIM3->CCR1-SPEED_DIF,90,255);
	speed_r=get_speed(TIM3->CCR2+SPEED_DIF,90,255);
	TIM3->CCR1=speed_l;//���ּ�Сһ���ٶ�
	TIM3->CCR2=speed_r;//��������һ���ٶ�
}

void right_more()//����΢��
{
	speed_l=get_speed(TIM3->CCR1+SPEED_DIF,90,255);
	speed_r=get_speed(TIM3->CCR2-SPEED_DIF,90,255);
	TIM3->CCR1=speed_l;//��������һ���ٶ�
	TIM3->CCR2=speed_r;//���ּ���һ���ٶ�
}

//��ʱ��5�жϷ������		    
void TIM5_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)//�Ǹ����ж�
	{
		//todo
		short cur_yaw_dis;
		temp_yaw=yaw;
		switch(cur_task)
		{
			case _ADUEST://��ǰ����У׼
				total_adjust_count++;
				if(adjust_count>10){//��ȷ��׼ȷ
					car_stop();
					break;
				}
				else if(total_adjust_count>100){//�����������࣬����ʧ��
					car_stop();
					break;
				}
				cur_yaw_dis=yaw_dis(temp_yaw,tar_yaw);
				if(cur_yaw_dis<=START_AUST){
					adjust_count++;
					MOTOR_L_F=0;
					MOTOR_L_B=0;
					MOTOR_R_F=0;
					MOTOR_R_B=0;
				}
				else{
					adjust_count=0;
					if(temp_yaw<tar_yaw){
						if(tar_yaw-temp_yaw<180){
							turn_left_center_short();
						}
						else{
							turn_right_center_short();
						}
					}
					else{
						if(temp_yaw-tar_yaw<180){
							turn_right_center_short();
						}
						else{
							turn_left_center_short();
						}
					}
				}
				break;
			case _STOP:
				TIM_Cmd(TIM5,DISABLE);//�رն�ʱ��5
				break;
			case _FORWARD:
				cur_yaw_dis=yaw_dis(temp_yaw,tar_yaw);
				if(cur_yaw_dis<START_AUST)
					break;
				if(temp_yaw<tar_yaw){
					if(tar_yaw-temp_yaw<180){
						left_more();
//						u3_printf("1");
					}
					else{
						right_more();
//						u3_printf("2");
					}
				}
				else{
					if(temp_yaw-tar_yaw<180){
						right_more();
//						u3_printf("3");
					}
					else{
						left_more();
//						u3_printf("4");
					}
				}
				break;
			case _LEFT:
				cur_yaw_dis=yaw_dis(temp_yaw,tar_yaw);
				if(cur_yaw_dis<=START_AUST){
					start_adjust();
				}
				else if(reverse_flag){
					turn_left_reverse();
				}
				else{
					turn_left();
				}
				
				if(cur_yaw_dis>last_yaw_dis){//��ʾ��ǰ�����Ƿ���
					error_count++;
					if(error_count>5){//��ֹ����
						reverse_flag=!reverse_flag;
						error_count=0;
					}
				} else {
					error_count=0;
				}
				
				last_yaw_dis=cur_yaw_dis;
				break;
			case _RIGHT:
				cur_yaw_dis=yaw_dis(temp_yaw,tar_yaw);
//				u3_printf("%d %d %d %d %d\n",tar_yaw, temp_yaw, cur_yaw_dis, last_yaw_dis, reverse_flag);
				if(cur_yaw_dis<=START_AUST){
					start_adjust();
				}
				else if(reverse_flag){
					turn_right_reverse();
				}
				else{
					turn_right();
				}
				
				if(cur_yaw_dis>last_yaw_dis){//��ʾ��ǰ�����Ƿ���
					error_count++;
					if(error_count>5){//��ֹ����
						reverse_flag=!reverse_flag;
						error_count=0;
					}
				} else {
					error_count=0;
				}
				
				last_yaw_dis=cur_yaw_dis;
				break;
			case _BACK:
				if(temp_yaw>tar_yaw+START_AUST)
					left_more();
				else if(temp_yaw<tar_yaw-START_AUST)
					right_more();
				break;
			default:
				break;
		}
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update  );  //���TIM5�����жϱ�־    
	}
}

void TIM5_Int_Init(u16 arr,u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);//TIM5ʱ��ʹ��    
	
	//��ʱ��TIM5��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM5�ж�,��������ж�
	
	TIM_Cmd(TIM5,ENABLE);//������ʱ��5
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
}

//��ʼ��PF1 2 3 4
//С��IO��ʼ��
void car_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	TIM3_PWM_Init(255,573);	 //PWMƵ��=72000000/((255+1)*(573+1))ԼΪ490HZ����arduinoһ��

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);	 //ʹ��PF�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;						//LED0-->PB.5 �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOF, &GPIO_InitStructure);					 		//�����趨������ʼ��GPIOB.5

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	TIM5_Int_Init(1000-1,7200-1);//100ms
	car_stop();
}

