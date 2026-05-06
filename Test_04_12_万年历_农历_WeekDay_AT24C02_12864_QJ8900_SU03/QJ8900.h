#ifndef __QJ8900_H__
#define __QJ8900_H__

void Delay1us(int T);
void QJ8900_SetVolume(unsigned char volume);
void QJ8900_PlayOneTrack(unsigned char track);
void QJ8900_PlayTrack(unsigned char track);
void SendData(unsigned char Addr);

sbit  SDA=P1^0;

#endif