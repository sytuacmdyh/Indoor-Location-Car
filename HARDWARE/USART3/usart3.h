#ifndef __USART3_H
#define __USART3_H	 
#include "sys.h"   

#define WIFI_TCP_MODEL 'S'
//#define WIFI_TCP_MODEL "C"

#ifndef FALSE

#define FALSE	0
#define TRUE	1

#endif

#define AT_CWLAP "AT+CWLAP\r\n"

#define USART3_MAX_RECV_LEN		2000					//�����ջ����ֽ���
#define USART3_MAX_SEND_LEN		2000					//����ͻ����ֽ���
#define USART3_RX_EN 			1					//0,������;1,����.

#define MOVE_KEEP_TIME 600

//ͨ��Э��  ����̫��
#define HELLO					"HELLO"
//#define FORWARD 				"F"
//#define FORWARD_SHORT 			"FS"
//#define BACK 					"B"
//#define BACK_SHORT 				"BS"
//#define LEFT					"L"
//#define LEFT_SHORT				"LS"
//#define RIGHT					"R"
//#define RIGHT_SHORT				"RS"
//#define SUB_SPEED				"-"
//#define ADD_SPEED				"+"
//#define STOP					"S"
//#define QUIT_TRANS 				"QUIT"
//#define GET_WIFI_INFO			"WIFI"
//#define GET_BLUETOOTH_INFO		"BLUE"
//#define GET_DISTENCE			"DIS"
//#define UNDO_GET_BLUETOOTH_INFO	"_BLUE"
//#define SERVO_LEFT				"SL"
//#define SERVO_FORWARD			"SF"
//#define SERVO_RIGHT				"SR"
//����Э��
//1:	"SP"+speed_level�� 
//		˵��:��10�� speed_level��Ϊ0~9 ��һ���֣�
//		����"SP5"

extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//���ջ���,���USART3_MAX_RECV_LEN�ֽ�
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
extern vu16 USART3_RX_STA;   						//��������״̬

//�ɼ�����
extern u8 RUNNING_TASK_FLAG;//ִ�����ݲɼ������־
extern u8 task_count;//��������
extern u8 cur_index;//��ǰִ�е���������
extern u32 delay_task_second_counter;//ȫ����ʱ����ʱʱ��
extern char tasks[20][10];//�����б�

void usart3_init(u32 bound);				//����3��ʼ�� 
void u3_printf(char* fmt,...);			//�򴮿ڴ�ӡ����
void analyse(char * str);						//�������ڷ��ص�ͨ������
void atk_8266_quit_trans(void);			//atk-esp8266 wifiģ���˳�͸��ģʽָ��
void send_bluetooth_info(void);		//������Χ�����ź�

#endif













