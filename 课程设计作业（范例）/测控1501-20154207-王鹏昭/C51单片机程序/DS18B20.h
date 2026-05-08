#ifndef __DS18B20_H_
#define __DS18B20_H_

#include<reg52.h>
//---重定义关键词---//
#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint 
#define uint unsigned int
#endif

//--定义使用的IO口--//
sbit  DQ = P1^7;

void DelayXus(unsigned char n);
bit DS18B20_Reset();
bit DS18B20_Readbit();
void DS18B20_WriteByte(unsigned char dat);
unsigned char DS18B20_ReadByte();
unsigned int DS18B20_ReadDesignateTemper(unsigned char rom[8]) ;
#endif
