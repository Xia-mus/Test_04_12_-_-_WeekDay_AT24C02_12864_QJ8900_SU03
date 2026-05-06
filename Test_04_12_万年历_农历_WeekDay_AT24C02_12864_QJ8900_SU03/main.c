#include <REGX52.H>
#include "MatrixKey.h"
#include "lcd12864.h"
#include "DS18B20.h"
#include "AT24C02.h"
#include "DS1302.h"
#include "Timer0.h"
#include "Buzzer.h"
#include "QJ8900.H"
#include "Delay.H"
#include "Delay.h"

sbit Buzzer = P1^5;
sbit Voice = P3^2;

// 2020-2030年农历数据表 (格式: 0xAA BB CC DD)
// AA=春节月份, BB=春节日期, CC=闰月月份(0无闰月), DD=月份大小信息(低16位)
code unsigned long lunar_table[] = {
    // 2020: 春节1月25日, 无闰月, 月份数据: 101010101010 (0x0AAA)
    0x01190AAA, 
    
    // 2021: 春节2月12日, 无闰月, 月份数据: 101011010101 (0x0AD5)
    0x020C0AD5,
    
    // 2022: 春节2月1日, 无闰月, 月份数据: 101101011010 (0x0B5A)
    0x02010B5A,
    
    // 2023: 春节1月22日, 闰2月, 月份数据: 1001010101010 (0x12AA)
    // 月份顺序: 正月(1),二月(0),闰2月(0),三月(1),四月(0),五月(1),六月(0),七月(1),八月(0),九月(1),十月(0),冬月(1),腊月(0)
    0x011602AA,
    
    // 2024: 春节2月10日, 无闰月, 月份数据: 101010101010 (0x0AAA)
    0x020A0AAA,
    
    // 2025: 春节1月29日, 闰6月, 月份数据: 1010100101010 (0x152A)
    // 月份顺序: 正月(1),二月(0),三月(1),四月(0),五月(1),六月(0),闰6月(0),七月(1),八月(0),九月(1),十月(0),冬月(1),腊月(0)
    0x011D062A,
    
//    // 2026: 春节2月17日, 无闰月, 月份数据: 101010110110 (0x0AB6)
//    0x02170AB6,
		0x02110AB6,
    
    // 2027: 春节2月7日, 无闰月, 月份数据: 101010101011 (0x0AAB)
    0x02070AAB,
    
//    // 2028: 春节1月26日, 无闰月, 月份数据: 101010101011 (0x0AAB)
//    0x01260AAB,
			0x011A0AAB,
    
    // 2029: 春节2月4日, 无闰月, 月份数据: 101010101011 (0x0AAB)
    0x02040AAB,
    
//    // 2030: 春节1月23日, 无闰月, 月份数据: 101010101011 (0x0AAB)
//    0x01230AAB
		0x01170AAB
};

unsigned char Day_Temp  = 0;
unsigned char Hour_Temp = 0;
unsigned char Min_Temp  = 0;
unsigned char sec_Temp  = 0;
unsigned char Temp_Str  = 0;
unsigned char Br   = 1;
unsigned char TR   = 0;
unsigned char Week = 0;
int fahrenhei;

unsigned char QJ8900_SU_03 = 0;
unsigned char QJ8900_Hour  = 0;
unsigned char QJ8900_Min 	 = 0;
unsigned char QJ8900_Sec   = 0;
unsigned char QJ8900_OK    = 0;
static unsigned int T0Count2 = 1;

	
unsigned char Clock_Day  = 0;
unsigned char Clock_Hour = 0;
unsigned char Clock_Min  = 0;
unsigned char Clock_sec  = 0;
unsigned char Clock_Time = 0;

unsigned char BuzzerR = 0;
bit BuzzerStop = 0;

char year[] = {0xC4, 0xEA, 0x00}; // "年"的GB2312编码
char min[] = {0xD4, 0xC2,0x00};  // "月"的GB2312编码
char day[] = {0xC8, 0xD5,0x00};   // "日"的GB2312编码
char run[] = {0xC8, 0xF3, 0x00}; // "润"的GB2312编码
char wen[] = {0xCE, 0xC2, 0x00}; // "温"的GB2312编码
char du[] = {0xB6, 0xC8, 0x00};  // "度"的GB2312编码

unsigned char T = 0;

unsigned char KeyNum,MODE,TimeSetSelect,TimeSetFlashFlag;

// 农历日期结构体
typedef struct {
    unsigned char year;   // 年份偏移(0=2020)
    unsigned char month;  // 农历月
    unsigned char day;    // 农历日
    unsigned char isLeap; // 是否闰月
} LunarDate;

void TimeShow(void)//时间显示功能
{ 
	DS1302_ReadTime();//读取时间
	LCD12864_ShowNumber(0,0,DS1302_Time[0]);//显示年
	LCD12864_ShowNumber(0,2,DS1302_Time[1]);//显示月
	LCD12864_ShowNumber(1,6,Week);//显示Week
	LCD12864_ShowNumber(0,4,DS1302_Time[2]);//显示日
	LCD12864_Show2DigitNumber(1,0,DS1302_Time[3]);//显示时
	LCD12864_Show2DigitNumber(1,2,DS1302_Time[4]);//显示分
	LCD12864_Show2DigitNumber(1,4,DS1302_Time[5]);//显示秒
}

void TimeSet(void)//时间设置功能
{
	if(KeyNum==2)//按键2按下
	{
		Buzzer_Time(100);
		TimeSetSelect++;//设置选择位加1
		TimeSetSelect%=6;//越界清零
	}
	if(KeyNum==3)//按键3按下
	{
		Buzzer_Time(100);
		DS1302_Time[TimeSetSelect]++;//时间设置位数值加1
		if(DS1302_Time[0]>99){DS1302_Time[0]=0;}//年越界判断
		if(DS1302_Time[1]>12){DS1302_Time[1]=1;}//月越界判断
		if( DS1302_Time[1]==1 || DS1302_Time[1]==3 || DS1302_Time[1]==5 || DS1302_Time[1]==7 || 
			DS1302_Time[1]==8 || DS1302_Time[1]==10 || DS1302_Time[1]==12)//日越界判断
		{
			if(DS1302_Time[2]>31){DS1302_Time[2]=1;}//大月
		}
		else if(DS1302_Time[1]==4 || DS1302_Time[1]==6 || DS1302_Time[1]==9 || DS1302_Time[1]==11)
		{
			if(DS1302_Time[2]>30){DS1302_Time[2]=1;}//小月
		}
		else if(DS1302_Time[1]==2)
		{
			if(DS1302_Time[0]%4==0)
			{
				if(DS1302_Time[2]>29){DS1302_Time[2]=1;}//闰年2月
			}
			else
			{
				if(DS1302_Time[2]>28){DS1302_Time[2]=1;}//平年2月
			}
		}
		if(DS1302_Time[3]>23){DS1302_Time[3]=0;}//时越界判断
		if(DS1302_Time[4]>59){DS1302_Time[4]=0;}//分越界判断
		if(DS1302_Time[5]>59){DS1302_Time[5]=0;}//秒越界判断
	}
	if(KeyNum==4)//按键4按下
	{
		Buzzer_Time(100);
		DS1302_Time[TimeSetSelect]--;//时间设置位数值减1
		if(DS1302_Time[0]<0){DS1302_Time[0]=99;}//年越界判断
		if(DS1302_Time[1]<1){DS1302_Time[1]=12;}//月越界判断
		if( DS1302_Time[1]==1 || DS1302_Time[1]==3 || DS1302_Time[1]==5 || DS1302_Time[1]==7 || 
			DS1302_Time[1]==8 || DS1302_Time[1]==10 || DS1302_Time[1]==12)//日越界判断
		{
			if(DS1302_Time[2]<1){DS1302_Time[2]=31;}//大月
			if(DS1302_Time[2]>31){DS1302_Time[2]=1;}
		}
		else if(DS1302_Time[1]==4 || DS1302_Time[1]==6 || DS1302_Time[1]==9 || DS1302_Time[1]==11)
		{
			if(DS1302_Time[2]<1){DS1302_Time[2]=30;}//小月
			if(DS1302_Time[2]>30){DS1302_Time[2]=1;}
		}
		else if(DS1302_Time[1]==2)
		{
			if(DS1302_Time[0]%4==0)
			{
				if(DS1302_Time[2]<1){DS1302_Time[2]=29;}//闰年2月
				if(DS1302_Time[2]>29){DS1302_Time[2]=1;}
			}
			else
			{
				if(DS1302_Time[2]<1){DS1302_Time[2]=28;}//平年2月
				if(DS1302_Time[2]>28){DS1302_Time[2]=1;}
			}
		}
		if(DS1302_Time[3]<0){DS1302_Time[3]=23;}//时越界判断
		if(DS1302_Time[4]<0){DS1302_Time[4]=59;}//分越界判断
		if(DS1302_Time[5]<0){DS1302_Time[5]=59;}//秒越界判断
	}
	//更新显示，根据TimeSetSelect和TimeSetFlashFlag判断可完成闪烁功能
	if(TimeSetSelect==0 && TimeSetFlashFlag==1){LCD12864_SetWindow(0,0,"  ");}
	else {LCD12864_ShowNumber(0,0,DS1302_Time[0]);}
	if(TimeSetSelect==1 && TimeSetFlashFlag==1){LCD12864_SetWindow(0,2,"  ");}
	else {LCD12864_ShowNumber(0,2,DS1302_Time[1]);}
	if(TimeSetSelect==2 && TimeSetFlashFlag==1){LCD12864_SetWindow(0,4,"  ");}
	else {LCD12864_ShowNumber(0,4,DS1302_Time[2]);}
	if(TimeSetSelect==3 && TimeSetFlashFlag==1){LCD12864_SetWindow(1,0,"  ");}
	else {LCD12864_ShowNumber(1,0,DS1302_Time[3]);}
	if(TimeSetSelect==4 && TimeSetFlashFlag==1){LCD12864_SetWindow(1,2,"  ");}
	else {LCD12864_ShowNumber(1,2,DS1302_Time[4]);}
	if(TimeSetSelect==5 && TimeSetFlashFlag==1){LCD12864_SetWindow(1,4,"  ");}
	else {LCD12864_ShowNumber(1,4,DS1302_Time[5]);}
}

void Alarm_clock(void)
{
	if(KeyNum == 5) 
	{
		Day_Temp = DS1302_Time[2];
		Hour_Temp = DS1302_Time[3];
		Min_Temp = DS1302_Time[4];
		sec_Temp = DS1302_Time[5];
		TR = 0;
	}
	
	if(TR == 0)
		{
			//Br++;
			if(Br == 10)
			{
				//Buzzer_Time(100);
				Br = 0;
			}
			if(KeyNum == 10)
			{
				
				Clock_Day = DS1302_Time[2];
				Clock_Hour = DS1302_Time[3];
				Clock_Min = DS1302_Time[4];
				Clock_sec = DS1302_Time[5];
				
				DS1302_Time[2] = Day_Temp;
				DS1302_Time[3] = Hour_Temp;
				DS1302_Time[4] = Min_Temp;
				DS1302_Time[5] = sec_Temp;
				DS1302_SetTime();
				MODE=0;
			}
			if(KeyNum == 6)
			{
				Buzzer_Time(100);
				TimeSetSelect=2;
				DS1302_Time[2]++;
				if(DS1302_Time[2] >= 24) DS1302_Time[2] = 1;
			}
			if(KeyNum == 7)
			{
				Buzzer_Time(100);
				TimeSetSelect=3;
				DS1302_Time[3]++;
				if(DS1302_Time[3] >= 24) DS1302_Time[3] = 1;
			}
			if(KeyNum == 8)
			{
				Buzzer_Time(100);
				TimeSetSelect=4;
				DS1302_Time[4]++;
				if(DS1302_Time[4] >= 60) DS1302_Time[4] = 1;
			}
			if(KeyNum == 9)
			{
				Buzzer_Time(100);
				TimeSetSelect=5;
				DS1302_Time[5]++;
				if(DS1302_Time[5] >= 60) DS1302_Time[5] = 1;
			}
		}
		//更新显示，根据TimeSetSelect和TimeSetFlashFlag判断可完成闪烁功能
	if(TimeSetSelect==0 && TimeSetFlashFlag==1){LCD12864_SetWindow(0,0,"  ");}
	else {LCD12864_ShowNumber(0,0,DS1302_Time[0]);}
	if(TimeSetSelect==1 && TimeSetFlashFlag==1){LCD12864_SetWindow(0,2,"  ");}
	else {LCD12864_ShowNumber(0,2,DS1302_Time[1]);}
	if(TimeSetSelect==2 && TimeSetFlashFlag==1){LCD12864_SetWindow(0,4,"  ");}
	else {LCD12864_ShowNumber(0,4,DS1302_Time[2]);}
	if(TimeSetSelect==3 && TimeSetFlashFlag==1){LCD12864_SetWindow(1,0,"  ");}
	else {LCD12864_ShowNumber(1,0,DS1302_Time[3]);}
	if(TimeSetSelect==4 && TimeSetFlashFlag==1){LCD12864_SetWindow(1,2,"  ");}
	else {LCD12864_ShowNumber(1,2,DS1302_Time[4]);}
	if(TimeSetSelect==5 && TimeSetFlashFlag==1){LCD12864_SetWindow(1,4,"  ");}
	else {LCD12864_ShowNumber(1,4,DS1302_Time[5]);}
}

// 输入：year(2000-2099), month(1-12), day(1-31)
// 输出：0-6 (0=星期日, 1=星期一, ... 6=星期六)
unsigned char CalculateWeekday(unsigned int year, unsigned char month, unsigned char day) 
{
    unsigned char m = month;
    unsigned int y = year;
		unsigned char base;
		unsigned int weekday;
    
    // 1月和2月当作上一年的13月和14月
    if (month < 3) 
		{
			m = month + 12;
			y = year - 1;
    }
    
    base = (y + 2000 < 2000) ? 1 : 0;  // 世纪调整值
    
    // 基姆拉尔森计算公式
    weekday = day + 2 * m + 3 * (m + 1) / 5 + y + y / 4 + base;
    weekday %= 7;
    
    // 转换为7=周日,1=周一...6=周六
    weekday = (weekday + 1) % 7;
		if(weekday == 0)weekday = 7;
    
    return (unsigned char)weekday;
}

int CelsiusToFahrenheit(int celsius) 
{
// 等价于 (celsius * 9/5) + 32
int fahrenheit = (celsius * 9/5)  + 32;
return fahrenheit;
}

// 获取公历月份天数
unsigned char GetSolarMonthDays(unsigned int year, unsigned char month) {
    if (month == 2) {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) 
            return 29;
        return 28;
    }
    
    // 其他月份天数
    if (month == 4 || month == 6 || month == 9 || month == 11)
        return 30;
    return 31;
}

// 公历转农历函数
void SolarToLunar(unsigned int syear, unsigned char smonth, unsigned char sday, LunarDate *result) 
{
    // 获取农历数据
    unsigned long lunarData;
    unsigned char spring_month;
    unsigned char spring_day;
    unsigned char leap_month;
    unsigned int month_info;
    unsigned int days = 0;
    unsigned int tempDays = 0;
    unsigned char month_count;
    unsigned char month_days;
    int i;
    
    // 初始化结果
    result->year = 0;
    result->month = 0;
    result->day = 0;
    result->isLeap = 0;
    
    // 只支持2020-2030年
    if (syear < 2020 || syear > 2030) 
        return;
    
    // 获取农历数据
    lunarData = lunar_table[syear - 2020];
    spring_month = (lunarData >> 24) & 0xFF;
    spring_day   = (lunarData >> 16) & 0xFF;
    leap_month   = (lunarData >> 8)  & 0xFF;
    month_info   = lunarData & 0xFFFF;
    
    // 计算到春节的天数差
    days = 0;
    
    // 处理春节前的日期(属于上一年)
    if ((smonth < spring_month) || 
        (smonth == spring_month && sday < spring_day)) 
    {
        if (syear == 2020) return;
        
        // 获取上一年数据
        lunarData = lunar_table[syear - 1 - 2020]; // 修正索引计算
        spring_month = (lunarData >> 24) & 0xFF;
        spring_day   = (lunarData >> 16) & 0xFF;
        leap_month   = (lunarData >> 8)  & 0xFF;
        month_info   = lunarData & 0xFFFF;
        result->year = syear - 1 - 2020;
    } 
    else 
    {
        result->year = syear - 2020;
    }
    
    // 计算从春节到目标日期的天数
    // 从春节所在月份的下一个月开始计算
    if (smonth > spring_month) 
    {
        // 先计算春节当月剩余天数
        days = GetSolarMonthDays(syear, spring_month) - spring_day;
        
        // 计算中间月份天数
        for (i = spring_month + 1; i < smonth; i++) 
        {
            days += GetSolarMonthDays(syear, i);
        }
        
        // 加上目标月天数
        days += sday;
    } 
    else if (smonth == spring_month) 
    {
        // 同月计算
        days = sday - spring_day;
    }
    else 
    {
        // 处理跨年情况（春节在1月，目标月在上年12月）
        // 这里需要特殊处理，但2020-2030年春节都在1-2月，暂不处理
        return;
    }
    
    // 确保天数不为负
    if (days < 0) days = 0;
    
    // 计算农历月份和日期
    tempDays = 0;
    month_count = leap_month ? 13 : 12;
    
    // 从低位到高位解析月份大小信息
    for (i = 0; i < month_count; i++) 
    {
        // 注意：月份大小信息是从低位到高位对应月份
        month_days = (month_info & (1 << i)) ? 30 : 29;
        
        if (days < tempDays + month_days) 
        {
            // 处理闰月
            if (leap_month) 
            {
                if (i < leap_month - 1) 
                {
                    result->month = i + 1;
                    result->isLeap = 0;
                } 
                else if (i == leap_month - 1) 
                {
                    result->month = leap_month;
                    result->isLeap = 1;
                } 
                else 
                {
                    result->month = i; // 闰月后的月份减1
                    result->isLeap = 0;
                }
            } 
            else 
            {
                result->month = i + 1;
                result->isLeap = 0;
            }
            
            result->day = days - tempDays + 1;
            break;
        }
        tempDays += month_days;
    }
}

// 显示农历日期示例
void DisplayLunar(unsigned int year, unsigned char month, unsigned char day) 
{
	LunarDate lunar;
  SolarToLunar(year, month, day, &lunar);
	// 显示农历年份: 2020 + lunar.year
	
	LCD12864_ShowNumber(2,0,2020 + lunar.year);//显示年
	
	if (lunar.isLeap) 
	{
		LCD12864_SetWindow(2,7,run);
	}
	else
	{
		LCD12864_SetWindow(2,7,"  ");
	}
	LCD12864_ShowNumber(2,3,lunar.month);//显示月
	LCD12864_ShowNumber(2,5,lunar.day);//显示日
}

void QJ8900_DAY(void)
{
	if(KeyNum == 12)
	{
		KeyNum = 0;
		SendData(0x0a);
		QJ8900_SetVolume(3);
		QJ8900_PlayTrack(11); //系统时间
		Delay(1000);								
		QJ8900_Hour = DS1302_Time[3];
		QJ8900_Min  = DS1302_Time[4];
		QJ8900_Sec  = DS1302_Time[5];
		SendData(0x0a);
		if(QJ8900_Hour < 10)
		{
			QJ8900_PlayOneTrack(QJ8900_Hour);
			Delay(500);	
			QJ8900_PlayTrack(12);
			Delay(500);				
		}
		if(QJ8900_Hour >= 10)
		{
			QJ8900_PlayOneTrack(QJ8900_Hour / 10);
			Delay(500);
			QJ8900_PlayTrack(10);
			Delay(500);
			QJ8900_PlayOneTrack(QJ8900_Hour % 10);	
			Delay(500);
			QJ8900_PlayTrack(12);
			Delay(500);			
		}
		SendData(0x0a);
		if(QJ8900_Min < 10)
		{
			QJ8900_PlayOneTrack(QJ8900_Min);
			Delay(500);	
			QJ8900_PlayTrack(13);
			Delay(500);				
		}
		if(QJ8900_Min >= 10)
		{
			QJ8900_PlayOneTrack(QJ8900_Min / 10);
			Delay(500);
			QJ8900_PlayTrack(10);
			Delay(500);
			QJ8900_PlayOneTrack(QJ8900_Min % 10);	
			Delay(500);
			QJ8900_PlayTrack(13);
			Delay(500);			
		}
		SendData(0x0a);
		if(QJ8900_Sec < 10)
		{
			QJ8900_PlayOneTrack(QJ8900_Sec);
			Delay(500);	
			QJ8900_PlayTrack(14);
			Delay(500);				
		}
		SendData(0x0a);
		if(QJ8900_Sec >= 10)
		{
			QJ8900_PlayOneTrack(QJ8900_Sec / 10);
			Delay(500);
			QJ8900_PlayTrack(10);
			Delay(500);
			QJ8900_PlayOneTrack(QJ8900_Sec % 10);	
			Delay(500);
			QJ8900_PlayTrack(14);
			Delay(500);			
		}
	}
}

void main()
{
	DS18B20_ConvertT();		//上电先转换一次温度，防止第一次读数据错误
	Delay(1000);			//等待转换完成
	DS1302_Init();
	LCD12864_Init();
	Timer0Init();
	DS1302_ReadTime();
//	if(AT24C02_ReadByte(0) == 3)
//	{
//		DS1302_Time[0] = AT24C02_ReadByte(1);
//		DS1302_Time[1] = AT24C02_ReadByte(2);
//		DS1302_Time[2] = AT24C02_ReadByte(3);
//		DS1302_Time[3] = AT24C02_ReadByte(4);
//		DS1302_Time[4] = AT24C02_ReadByte(5);
//		DS1302_Time[5] = AT24C02_ReadByte(6);
//	}
	
	DS1302_SetTime();//设置时间
	Buzzer_Time(100);
	
	LCD12864_SetWindow(0,1,year);		//由于文件历史编码为UTF8
	LCD12864_SetWindow(0,3,min);		//现在只能直接输入GB2312的编码
	LCD12864_SetWindow(0,5,day);
	LCD12864_SetWindow(1,0,"  --  --   --");
	
	LCD12864_SetWindow(2,2,year);
	LCD12864_SetWindow(2,4,min);
	LCD12864_SetWindow(2,6,day);
	
	LCD12864_SetWindow(3,0,wen);
	LCD12864_SetWindow(3,1,du);
	LCD12864_SetWindow(3,2,":");
	Voice = 0;
	while(1)
	{
		LCD12864_SetWindow(2,6,day);
		DS18B20_ConvertT();	//转换温度 
		T=DS18B20_ReadT();	//读取温度
		LCD12864_ShowNumber(3,3,T);//不考虑低于0度
		
		KeyNum = MatrixKey();//读取键码
		Week = CalculateWeekday(DS1302_Time[0], DS1302_Time[1], DS1302_Time[2]);
		
    DisplayLunar(2000 + DS1302_Time[0], DS1302_Time[1], DS1302_Time[2]);
		if(Voice == 1 && QJ8900_SU_03 == 0) 
		{
			QJ8900_SU_03 = 1;
			Voice = 0;
			if(QJ8900_SU_03 == 1) 
			{
				KeyNum = 12;
				LCD12864_SetWindow(3,6,":");
			}
		}
		if(KeyNum == 13) QJ8900_SU_03 = 0;
		QJ8900_DAY();
		
		
		
		if(KeyNum==1)//按键1按下
		{
			Buzzer_Time(100);
			BuzzerStop = 0;
			if(MODE==0){MODE=1;TimeSetSelect=0;}//功能切换
			else if(MODE==1){MODE=0;DS1302_SetTime();}
		}
		if(KeyNum==5)//按键5按下
		{
			Buzzer_Time(100);
			BuzzerStop = 0;
			if(MODE==0){MODE=2;TimeSetSelect=0;}//功能切换
			else if(MODE==2){;}
		}
		if(KeyNum == 11)
		{
			BuzzerR = 0;
		}
		if(Clock_Day == DS1302_Time[2] && Clock_Hour == DS1302_Time[3] && Clock_Min == DS1302_Time[4] && Clock_sec == DS1302_Time[5])
		{
			for(Clock_Time = 0;Clock_Time < 10;Clock_Time++)BuzzerR = 1;
		}
		switch(MODE)//根据不同的功能执行不同的函数
		{
			case 0:TimeShow();break;
			case 1:TimeSet();break;
			case 2:Alarm_clock();break;
		}
//		AT24C02_WriteByte(0,3);
//		AT24C02_WriteByte(1,DS1302_Time[0]);
//		AT24C02_WriteByte(2,DS1302_Time[1]);
//		AT24C02_WriteByte(3,DS1302_Time[2]);
//		AT24C02_WriteByte(4,DS1302_Time[3]);
//		AT24C02_WriteByte(5,DS1302_Time[4]);
//		AT24C02_WriteByte(6,DS1302_Time[5]);
	}
}

void Timer0_Routine() interrupt 1
{
	static unsigned int T0Count;
	static unsigned int T0Count0;
	static unsigned int T0Count1;
	TL0 = 0x18;		//设置定时初值
	TH0 = 0xFC;		//设置定时初值
	T0Count++;
	if(T0Count>=500)//每500ms进入一次
	{
		T0Count=0;
		TimeSetFlashFlag=!TimeSetFlashFlag;//闪烁标志位取反
	}
	T0Count0++;
	if(T0Count0>=1 && BuzzerR == 1 )
	{
		T0Count0=0;
		Buzzer = !Buzzer;
	}
	
	if(BuzzerR == 1)T0Count1++;
	if(T0Count1 >= 3000)
	{
		T0Count1 = 0;
		BuzzerR = 0;
	}
	if(QJ8900_OK == 1)T0Count2++;
	if(T0Count2 >= 1000)
	{
		T0Count2 = 0;
		QJ8900_OK = 0;
	}
}