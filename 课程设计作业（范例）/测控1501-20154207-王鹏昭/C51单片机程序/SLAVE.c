//***************************************************************
#include <REG51.H>				//通讯波特率115200bps（24M）
#include "CRC.H"
#include "SLAVE.H"
#include "DS18B20.H"
#include "DHT11_1.h"
#include "DHT11_2.h"
#include "BH1750.h"
//***************************************************************
#define FOSC            24000000UL
#define BRT             (65536 - FOSC / 115200 / 4)
//***************************************************************
sfr     AUXR        =   0x8e;
sfr     T2H         =   0xd6;
sfr     T2L         =   0xd7;
//***************************************************************
unsigned char xdata receBuf[16]={0};	//接收缓冲区
unsigned char xdata sendBuf[32]={0};	//发送缓冲区
unsigned char xdata Save_data[32]={0};//温度存储区
unsigned char localAddr = 0x01; 			//此从机的地址
unsigned char sendCount;  						//发送字节个数
unsigned char receCount;     					//接收到的字节个数
unsigned char sendPosi;     					//发送位置指针
unsigned int watchdog = 0;						//软件看门狗初始化，防通讯死机
unsigned char xdata rom1[8]={0x28, 0xFF, 0xEC, 0x16, 0x31, 0x18, 0x01, 0x33};
unsigned char xdata rom2[8]={0x28, 0xFF, 0x42, 0xF4, 0x30, 0x18, 0x01, 0x47};
unsigned char xdata rom3[8]={0x28, 0xFF, 0x0C, 0xE3, 0x30, 0x18, 0x01, 0xB2};
extern unsigned char RH,RL,TH,TL; 	 	//TH保存温度，RH保存湿度
extern unsigned char RH2,RL2,TH2,TL2; 	 	//TH保存温度，RH保存湿度
void Get_humidity();
bit busy,DHT_flag;															//发送完成标志
//*****************定时器0初始化(24M)*******************************
void TIM0INT(void)
{									//波特率10/115200*3.5 = 304us
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0x80;			//设置定时初值
	TH0 = 0xE3;			//设置定时初值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0 = 1;				//使能定时器0中断
}
//*****************串口初始化函数（24M，115200bps）*****************
void UartInit(void)
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设定定时器1为16位自动重装方式
	TL1 = BRT;			//设定定时初值
	TH1 = BRT>>8;		//设定定时初值
	ET1 = 0;				//禁止定时器1中断
	TR1 = 1;				//启动定时器1
	ES = 1;
}
//*****************发送字节数据函数*********************************
void UartSend(char dat)
{
    while (busy);
    busy = 1;
    SBUF = dat;
}
//*****************发送函数*****************************************
void beginSend(void)
{
		sendPosi=0;				//设定初始发送位置
		SBUF=sendBuf[0];	//进入串口中断继续发送接下来的字节
}
//*****************获取寄存器值*************************************
unsigned char get_Reg(unsigned int m) 
{
		unsigned char result;
		result=Save_data[m];
		return result;
}
//*****************定义共用体变量*************************************
union DATA
{
	float   tp;
	char   	a[4]; 	
}temp1,temp2,temp3,humidity1,humidity2,brightness;
//******************读输入寄存器04号命令****************************
/*主机发送格式（0x04）
［设备地址］［0x04］［寄存器起始高地址］［寄存器起始低地址］［读寄存器数量高］［读寄存器数量低］ ［CRC高8］［CRC低8］
从机回复格式（0x04）
［设备地址］［0x04］［返回字节个数］［数据1高8］［数据1低8］［数据2高8］［数据2低8］～～［数据n］［CRC高8］［CRC低8］*/
//******************************************************************
void read_Registers(void)
{
		unsigned char addr,tempAddr,readCount,byteCount;
		unsigned int crcData,i;
		addr=receBuf[2]*256+receBuf[3];
		tempAddr=addr*2;										//BTTE地址
		readCount=receBuf[4]*256+receBuf[5];
		byteCount=readCount*2;
		for(i=0;i<byteCount;i++)
		{
			sendBuf[3+i]=get_Reg(tempAddr++);
		}
		sendBuf[0]=localAddr;
		sendBuf[1]=0x04;
		sendBuf[2]=byteCount;
		byteCount=byteCount+3;
		crcData=crc16(sendBuf,byteCount);
		sendBuf[byteCount]=(unsigned char)(crcData>>8);
		byteCount++;
		sendBuf[byteCount]=(unsigned char)(crcData&0xff);
		sendCount=byteCount+1;
		beginSend();
}

//******************定时器0中断函数*******************************
void timer0lntProc(void) interrupt 1
{
	unsigned int crcData,recCRC=0;
	unsigned char len;
	if((receBuf[0]==localAddr)&&(receBuf[1]==0x04)&&(receCount==8)) 
	{									//为本机地址并且为04号命令并且接收到8字节数据
		len=receCount-2;
		crcData=crc16(receBuf,len);
		recCRC=recCRC+receBuf[len];
		recCRC=(recCRC<<8)+receBuf[len+1];
		if(crcData==recCRC)
		{
			watchdog = 0;								//软件看门狗清零
			read_Registers();						//读储存的温度值并发送
		}
	}
	TL0 = 0x80;			//设置定时初值
	TH0 = 0xE3;			//设置定时初值
	receCount=0;															
}

//******************串口1中断函数*********************************
void UART1_Isr(void) interrupt 4
{
	ET0=0;
	DHT_flag = 0;
	if(TI)										//判断是否为发送
	{
		TI = 0;
		busy = 0;
		if(sendPosi<sendCount-1)//如果发送位置小于发送字节
		{
			sendPosi++;
			UartSend(sendBuf[sendPosi]);
		}
		else
		{
			sendPosi = 0;
			receCount=0;				//把接收到的字节个数清0
		} 
	}
	else if(RI)							//判断是否有接收数据。
	{
		RI = 0;
		TR0=0;		
		TL0 = 0x80;			//设置定时初值
		TH0 = 0xE3;			//设置定时初值	
		receBuf[receCount++]=SBUF;
		TR0=1;
	} 
	ET0 = 1;
}
void Get_Tempdata()
{
		unsigned char i;
		temp1.tp = (float)DS18B20_ReadDesignateTemper(rom1)*0.0625;
		for(i=0;i<4;i++)
		{
			Save_data[i] = temp1.a[i];
		}
		temp2.tp = (float)DS18B20_ReadDesignateTemper(rom2)*0.0625;
		for(i=0;i<4;i++)
		{
			Save_data[i+4] = temp2.a[i];
		}
		temp3.tp = (float)DS18B20_ReadDesignateTemper(rom3)*0.0625;
		for(i=0;i<4;i++)
		{
			Save_data[i+8] = temp3.a[i];
		}
}
void Get_humidity()
{
	DHT_flag = 1;
	receive();
	Save_data[12] = RH;
	Save_data[13] =	RL;
	Save_data[14] =	TH;
	Save_data[15] =	TL;
	
	DHT_flag = 1;
	receive2();
	Save_data[16] = RH2;
	Save_data[17] =	RL2;
	Save_data[18] =	TH2;
	Save_data[19] =	TL2;
}
void Get_light()
{
	unsigned char i;
	brightness.tp = Read_Light();
	for(i=0;i<4;i++)
	{
		Save_data[20+i] = brightness.a[i];
	}
}
//******************主函数*****************************************
void main(void)
{
	TIM0INT();
	UartInit();
	EA = 1;
	
	while(1)
	{
		watchdog++;
		Get_Tempdata();
		Get_humidity();
		Get_light();

		if(watchdog > 10000)
		{
			watchdog=0;
			TIM0INT();
			UartInit();
			EA=1;
		}
	}
}