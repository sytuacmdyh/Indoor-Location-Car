README
===========================
Resources
--------------------------
	Timer
	2	完全重映射	ch2:pwm波形输出		控制舵机
	3	完全重映射	ch1,ch2:pwm波形输出	控制左右轮转速
	4	映射		ch1:输入捕获		用于读取超声波echo返回的高电平持续时间
	5	无映射		无通道			矫正小车运动状态
	6	无映射		无通道			定时刷新MPU数据 并执行一些定时任务
	7	无映射		无通道			监视usart3数据是否接受完成
	Usart
	1	无映射		与上位机通讯
	2	无映射		与蓝牙通讯
	3	无映射		与WIFI模块通讯
IO
---------------------------
	PA2	USART2_TX	蓝牙
	PA3	USART2_RX	蓝牙
	PA9	USART1_RX	上位机
	PA10	USART1_TX	上位机
	PB3	TIM2_CH2	舵机
	PB6	I2C1_SCL	MPU6050加速度陀螺仪
	PB7	I2C1_SDA	MPU6050加速度陀螺仪
	PB10	USART3_TX	WIFI
	PB11	USART3_RX	WIFI
	PC6	TIM3_CH1	小车运动left
	PC7 	TIM3_CH2	小车运动right
	PD11	IO		超声波TIRG
	PD12	TIM4_CH1	超声波ECHO
	PF0	IO		小车运动
	PF1	IO		小车运动
	PF2	IO		小车运动
	PF3	IO		小车运动
	PF4	IO		霍尔传感器 外部中断
