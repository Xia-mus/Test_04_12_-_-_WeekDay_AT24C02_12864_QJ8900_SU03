#include <REGX52.H>
#include "QJ8900.H"

void Delay1us(int T)
{
    T = T - 70;
    do{;}
    while(T--);
}

void QJ8900_SetVolume(unsigned char volume)
{
	if(volume > 3) volume = 0X03;
	SendData(0x0a);
	SendData(volume); 
	SendData(0x00); 
	SendData(0x0c); 
}

void QJ8900_PlayOneTrack(unsigned char track)
{
   SendData(0x0a); 
	 SendData(track); 
	 SendData(0x0b);
}

void QJ8900_PlayTrack(unsigned char track)
{
   SendData(0x0a);
	 SendData(track / 10);
	 SendData(track % 10);
	 SendData(0x0b);
}

void SendData(unsigned char Addr)
{
    unsigned char i;
    EA = 0;
    SDA = 1;
    Delay1us(1000);
    SDA = 0;
    Delay1us(2200);
    for ( i = 0; i < 8; i++ )
    {
			SDA = 1;
			if ( Addr & 0x01 )
			{
					Delay1us(500);
					SDA = 0;
					Delay1us(210);
			}
			else
			{
					Delay1us(210);
					SDA = 0;
					Delay1us(500);
			}
			Addr >>= 1;
    }
    SDA = 1;
    EA = 1;
}