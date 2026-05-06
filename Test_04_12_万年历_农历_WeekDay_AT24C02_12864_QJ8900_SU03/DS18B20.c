#include <REGX52.H>
#include "OneWire.h"

// DS18B20 指令宏定义
#define DS18B20_SKIP_ROM			0xCC			// 跳过ROM寻址指令（用于单设备或广播）
#define DS18B20_CONVERT_T			0x44			// 开始温度转换指令
#define DS18B20_READ_SCRATCHPAD 	0xBE	// 读取暂存器指令

//启动DS18B20进行温度转换
void DS18B20_ConvertT(void)
{
	OneWire_Init();												// 初始化单总线（复位+存在检测）
	OneWire_SendByte(DS18B20_SKIP_ROM);		// 发送跳过ROM指令（针对单个传感器）
	OneWire_SendByte(DS18B20_CONVERT_T);	// 发送开始温度转换指令
}

//从DS18B20读取温度值
float DS18B20_ReadT(void)
{
	unsigned char TLSB,TMSB;							// 存放温度值的低字节和高字节
	int Temp;															// 组合后的16位温度数据
	float T;															// 最终的温度值（摄氏度）
	OneWire_Init();												// 初始化单总线
	OneWire_SendByte(DS18B20_SKIP_ROM);		// 发送跳过ROM指令
	OneWire_SendByte(DS18B20_READ_SCRATCHPAD);// 发送读取暂存器指令
	TLSB=OneWire_ReceiveByte();						// 读取温度低字节（LSB）
	TMSB=OneWire_ReceiveByte();						// 读取温度高字节（MSB）
	Temp=(TMSB<<8)|TLSB;									// 将两个字节组合成16位温度数据
	T=Temp/16.0;													// 将原始数据转换为实际温度值（DS18B20精度为0.0625°C/位）
	return T;
}
