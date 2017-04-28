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

#define USART3_MAX_RECV_LEN		2000					//最大接收缓存字节数
#define USART3_MAX_SEND_LEN		2000					//最大发送缓存字节数
#define USART3_RX_EN 			1					//0,不接收;1,接收.

#define MOVE_KEEP_TIME 600

//通信协议  不可太长
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
//带参协议
//1:	"SP"+speed_level； 
//		说明:共10档 speed_level可为0~9 任一数字；
//		例："SP5"

extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//接收缓冲,最大USART3_MAX_RECV_LEN字节
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//发送缓冲,最大USART3_MAX_SEND_LEN字节
extern vu16 USART3_RX_STA;   						//接收数据状态

//采集任务
extern u8 RUNNING_TASK_FLAG;//执行数据采集任务标志
extern u8 task_count;//总任务数
extern u8 cur_index;//当前执行到哪条命令
extern u32 delay_task_second_counter;//全局延时任务到时时间
extern char tasks[20][10];//任务列表

void usart3_init(u32 bound);				//串口3初始化 
void u3_printf(char* fmt,...);			//向串口打印数据
void analyse(char * str);						//分析串口返回的通信内容
void atk_8266_quit_trans(void);			//atk-esp8266 wifi模块退出透传模式指令
void send_bluetooth_info(void);		//发送周围蓝牙信号

#endif













