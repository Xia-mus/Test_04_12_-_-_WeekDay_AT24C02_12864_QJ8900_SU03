#include"lcd12864.h"

void LCD12864_Delay1ms(uint c) 							//内置软件延时
{
  char a,b;
	for(; c>0; c--)
	{
	  for(b=199; b>0; b--)
		{
	        for(a=1; a>0; a--);
		}
	}
}

uchar LCD12864_Busy(void) 									//检测LCD是否忙（1表示不忙，0表示忙）
{
	uchar i = 0;

	LCD12864_RS = 0;   												//选择命令
	LCD12864_RW = 1;													//选择读取

	LCD12864_EN = 1;
	LCD12864_Delay1ms(1);

	while((LCD12864_DATAPORT & 0x80) == 0x80)	//检测读取到的值
	{
		i++;
		if(i > 100)
		{
			LCD12864_EN = 0;
			return 0;	   													//超过等待时间返回0表示失败
		}
	}

	LCD12864_EN = 0;

	return 1;
}

void LCD12864_WriteCmd(uchar cmd) 		//写命令
{
	uchar i;
	i = 0;
	while( LCD12864_Busy() == 0)
	{
		LCD12864_Delay1ms(1);
		i++;
		if( i>100)
		{
			return;	   											//超过等待退出
		}	
	}
	
	LCD12864_RS = 0;     								//选择命令
	LCD12864_RW = 0;     								//选择写入
	LCD12864_EN = 0;     								//初始化使能端

	LCD12864_DATAPORT = cmd;   					//放置数据

	LCD12864_EN = 1;		   							//写时序
	LCD12864_Delay1ms(5);
	LCD12864_EN = 0;    					
}

void LCD12864_WriteData(uchar dat)		//写数据
{
	uchar i;
	i = 0;
	while( LCD12864_Busy() == 0)
	{
		LCD12864_Delay1ms(1);
		i++;
		if( i>100)
		{
			return;	   											//超过等待退出
		}	
	}

	LCD12864_RS = 1;     								//选择数据
	LCD12864_RW = 0;     								//选择写入
	LCD12864_EN = 0;     								//初始化使能端

	LCD12864_DATAPORT = dat;   					//放置数据

	LCD12864_EN = 1;		   							//写时序
	LCD12864_Delay1ms(5);
	LCD12864_EN = 0;    								
}

void IntToStr(int num, char *str, int digits) //将数字转化为字符，以支持数字的动态显示
{
	int i = 0;
	int isNegative = 0;
	int start, end;
	int len = 0;
	int temp = num;

	// 处理负数
	if (num < 0) 
	{
		isNegative = 1;
		num = -num;
		temp = num;
	}

	// 计算数字长度
	do 
	{
		len++;
		temp /= 10;
	} while (temp != 0);

	// 如果需要固定位数且实际位数不足，前面补零
	if (digits > 0 && len < digits) 
	{
		for (i = 0; i < digits - len; i++) 
		{
			str[i] = '0';
		}
	}

	// 处理0的情况
	if (num == 0) 
	{
		if (digits <= 0) 
		{
			str[i++] = '0';
		} 
		else 											// 如果要求固定位数，0已经通过前面的补零处理了
		{
			
			i = digits - 1;
			str[i] = '0';
		}
	} 
	else 												// 将数字转换为字符（反向）
	{													
		temp = num;
		do 
		{
			str[i++] = (temp % 10) + '0';
			temp /= 10;
		} while (temp != 0);
	}

	// 处理负数
	if (isNegative) 
	{
		str[i++] = '-';
	}

	// 反转字符串
	start = (digits > 0 && digits > len) ? digits - len : 0;
	end = i - 1;
	while (start < end) 
	{
		char temp_char = str[start];
		str[start] = str[end];
		str[end] = temp_char;
		start++;
		end--;
	}

	str[i] = '\0'; 												// 添加字符串结束符
}

void IntToStr2Digit(int num, char *str) //将数字转化为两位数字符串，不足两位前面补零
{
	// 确保数字在0-99范围内
	if (num < 0) num = 0;
	if (num > 99) num = 99;
	
	// 转换为两位数字符串
	str[0] = (num / 10) + '0';
	str[1] = (num % 10) + '0';
	str[2] = '\0'; // 字符串结束符
}

void LCD12864_Init()										//初始化LCD12864
{
	LCD12864_PSB = 1;	  									//选择并行输入
	LCD12864_RST = 1;	  									//复位

	LCD12864_WriteCmd(0x30);  						//选择基本指令操作
	LCD12864_WriteCmd(0x0c);  						//显示开，关光标
	LCD12864_WriteCmd(0x01);  						//清除LCD12864的显示内容
}

void LCD12864_SetWindow(uchar x, uchar y, uchar *word)//设置在基本指令模式下设置显示坐标，并显示汉字。注意：x是设置行，y是设置列
{
	uchar pos;
	uchar i = 0;  												// 从字符串的第一个字符开始显示

	// 设置行地址
	if(x == 0) pos = 0x80 + y;        		// 第一行的地址是80H
	else if(x == 1) pos = 0x90 + y;   		// 第二行的地址是90H
	else if(x == 2) pos = 0x88 + y;   		// 第三行的地址是88H
	else if(x == 3) pos = 0x98 + y;   		// 第四行的地址是98H
	else return;													// 行错误，直接返回
			        
	LCD12864_WriteCmd(pos);  							// 设置光标位置

	// 显示字符串，直到字符串结束或者超出本行（一行最多16个字符，从y列开始，所以最多显示16-y个字符）
	while (word[i] != '\0' && y < 16) 
	{
		LCD12864_WriteData(word[i]);
		i++;
		y++;  															// 列位置增加，同时也可以用来判断是否超出本行
	}
}

void LCD12864_ShowNumber(uchar x, uchar y, int number)  //显示数字的函数
{
	char buffer[16]; 																			// 缓冲区，用于存储转换后的字符串
	IntToStr(number, buffer,2); 													// 转换数字为字符串
	LCD12864_SetWindow(x, y, (uchar *)buffer); 						// 调用现有的显示函数
}

void LCD12864_Show2DigitNumber(uchar x, uchar y, int number) //显示两位数字的函数（适用于时钟显示）
{
    char buffer[3]; 																		// 缓冲区，用于存储两位数字符串
    IntToStr2Digit(number, buffer); 										// 转换数字为两位数字符串
    LCD12864_SetWindow(x, y, (uchar *)buffer);
}
