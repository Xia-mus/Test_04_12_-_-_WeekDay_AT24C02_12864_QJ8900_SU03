#include <REGX52.H>

sbit OneWire_DQ=P3^7;

//单总线初始化（复位脉冲 + 存在检测）
unsigned char OneWire_Init(void)
{
	unsigned char i;
	unsigned char AckBit;
	EA=0;										// 禁用总中断，确保时序精确
	OneWire_DQ=1;						// 确保总线为高电平
	OneWire_DQ=0;						// 主机拉低总线，开始复位脉冲
	i = 247;while (--i);		//Delay 500us
	OneWire_DQ=1;						// 主机释放总线，由上拉电阻拉高
	i = 32;while (--i);			//Delay 70us
	AckBit=OneWire_DQ;			// 采样存在脉冲：0-设备存在，1-无设备
	i = 247;while (--i);		//Delay 500us
	EA=1;										// 重新启用总中断
	return AckBit;
}

/**
  * @brief  单总线发送一位
  * @param  Bit 要发送的位
  * @retval 无
  */
void OneWire_SendBit(unsigned char Bit)
{
	unsigned char i;
	EA=0;
	OneWire_DQ=0;
	i = 4;while (--i);			//Delay 10us
	OneWire_DQ=Bit;
	i = 24;while (--i);			//Delay 50us
	OneWire_DQ=1;
	EA=1;
}

/**
  * @brief  单总线接收一位
  * @param  无
  * @retval 读取的位
  */
unsigned char OneWire_ReceiveBit(void)
{
	unsigned char i;
	unsigned char Bit;
	EA=0;
	OneWire_DQ=0;
	i = 2;while (--i);			//Delay 5us
	OneWire_DQ=1;
	i = 2;while (--i);			//Delay 5us
	Bit=OneWire_DQ;
	i = 24;while (--i);			//Delay 50us
	EA=1;
	return Bit;
}

/**
  * @brief  单总线发送一个字节
  * @param  Byte 要发送的字节
  * @retval 无
  */
void OneWire_SendByte(unsigned char Byte)
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		OneWire_SendBit(Byte&(0x01<<i));
	}
}

/**
  * @brief  单总线接收一个字节
  * @param  无
  * @retval 接收的一个字节
  */
unsigned char OneWire_ReceiveByte(void)
{
	unsigned char i;
	unsigned char Byte=0x00;
	for(i=0;i<8;i++)
	{
		if(OneWire_ReceiveBit()){Byte|=(0x01<<i);}
	}
	return Byte;
}
