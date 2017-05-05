#ifndef __USART2_H
#define __USART2_H	 
#include "sys.h"   

#ifndef FALSE

#define FALSE	0
#define TRUE	1

#endif

#define USART2_MAX_RECV_LEN		200					//�����ջ����ֽ���
#define USART2_MAX_SEND_LEN		200					//����ͻ����ֽ���
#define USART2_RX_EN 			1					//0,������;1,����.

extern u8  USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		//���ջ���,���USART3_MAX_RECV_LEN�ֽ�
extern u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 		//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
extern u16 USART2_RX_STA;   						//��������״̬

void usart2_init(u32 bound);				//����2��ʼ�� 
void u2_printf(char* fmt,...);			//�򴮿ڴ�ӡ����
void start_search_ibeacon(void);//��ʼ���ibeacon��Ϣ

#endif













