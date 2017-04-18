#ifndef __USART2_H
#define __USART2_H	 
#include "sys.h"   

#ifndef FALSE

#define FALSE	0
#define TRUE	1

#endif

#define USART2_MAX_RECV_LEN		200					//最大接收缓存字节数
#define USART2_MAX_SEND_LEN		100					//最大发送缓存字节数
#define USART2_RX_EN 			1					//0,不接收;1,接收.

//#define BLUETOOTH_AT_MODEL PAout(4)// PA4
//#define BLUETOOTH_CS PAout(5)// PA5 ENABLE

extern u8  USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		//接收缓冲,最大USART3_MAX_RECV_LEN字节
extern u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 		//发送缓冲,最大USART3_MAX_SEND_LEN字节
extern u8 BLUETOOTH_MESSAGE_BUF[USART2_MAX_RECV_LEN];
extern vu16 USART2_RX_STA;   						//接收数据状态
extern u8	GETTING_INFO_FLAG;
extern vu16 BLUETOOTH_MESSAGE_LEN;

void usart2_init(u32 bound);				//串口2初始化 
void u2_printf(char* fmt,...);			//向串口打印数据
void init_HC05(void);							//初始化HC-05
u8 get_bluetooth_info(void);						//请求查询周围蓝牙信息
u8 undo_get_bluetooth_info(void);				//取消请求

#endif













