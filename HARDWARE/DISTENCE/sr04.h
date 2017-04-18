#ifndef __SR04_H
#define __SR04_H
#include "sys.h"

#define SR04_TRIG PDout(11)

extern int distence;
extern u8 GET_DIS_FLAG;

void SR04_Init(void);//初始化
int SR04_get_distence(void);//获取前方距离，单位毫米
void send_distence_info(void);//发送获取的距离distence给服务器

#endif
