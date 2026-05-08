//*********************************************************
#include  <REG51.H>	
#include  <math.h>    //Keil library  
#include  <stdio.h>   //Keil library	
#include  <INTRINS.H>
#include "BH1750.h"
//*********************************************************
#define   uchar unsigned char
#define   uint unsigned int	
#define	  SlaveAddress   0x46 //定义器件在IIC总线中的从地址,根据ALT  ADDRESS地址引脚不同修改,ALT  ADDRESS引脚接地时地址为0xA6，接电源时地址为0x3A
//*********************************************************
typedef   unsigned char BYTE;
typedef   unsigned short WORD;
sbit	  	SCL=P1^3;      //IIC时钟引脚定义
sbit  		SDA=P1^4;      //IIC数据引脚定义
BYTE    BUF[8];                         //接收数据缓存区      	
unsigned int     dis_data;              //变量
float illumination;
//*********************************************************
void delay_nms(unsigned int k)	
{
	while(k--)
	{
		unsigned char i, j;
		_nop_();
		i = 32;
		j = 40;
		do
		{
			while (--j);
		} while (--i);
	}
}
//*********************************************************
void Delay5us()		//@24.000MHz
{
	unsigned char i;

	i = 38;
	while (--i);
}
//*********************************************************
void Delay5ms()		//@24.000MHz
{
	unsigned char i, j;

	_nop_();
	_nop_();
	i = 156;
	j = 213;
	do
	{
		while (--j);
	} while (--i);
}
/**************************************
起始信号
**************************************/
void BH1750_Start()
{
    SDA = 1;                    //拉高数据线
    SCL = 1;                    //拉高时钟线
    Delay5us();                 //延时
    SDA = 0;                    //产生下降沿
    Delay5us();                 //延时
    SCL = 0;                    //拉低时钟线
}

/**************************************
停止信号
**************************************/
void BH1750_Stop()
{
    SDA = 0;                    //拉低数据线
    SCL = 1;                    //拉高时钟线
    Delay5us();                 //延时
    SDA = 1;                    //产生上升沿
    Delay5us();                 //延时
}

/**************************************
发送应答信号
入口参数:ack (0:ACK 1:NAK)
**************************************/
void BH1750_SendACK(bit ack)
{
    SDA = ack;                  //写应答信号
    SCL = 1;                    //拉高时钟线
    Delay5us();                 //延时
    SCL = 0;                    //拉低时钟线
    Delay5us();                 //延时
}

/**************************************
接收应答信号
**************************************/
bit BH1750_RecvACK()
{
    SCL = 1;                    //拉高时钟线
    Delay5us();                 //延时
    CY = SDA;                   //读应答信号
    SCL = 0;                    //拉低时钟线
    Delay5us();                 //延时

    return CY;
}

/**************************************
向IIC总线发送一个字节数据
**************************************/
void BH1750_SendByte(BYTE dat)
{
    BYTE i;

    for (i=0; i<8; i++)         //8位计数器
    {
        dat <<= 1;              //移出数据的最高位
        SDA = CY;               //送数据口
        SCL = 1;                //拉高时钟线
        Delay5us();             //延时
        SCL = 0;                //拉低时钟线
        Delay5us();             //延时
    }
    BH1750_RecvACK();
}

/**************************************
从IIC总线接收一个字节数据
**************************************/
BYTE BH1750_RecvByte()
{
    BYTE i;
    BYTE dat = 0;

    SDA = 1;                    //使能内部上拉,准备读取数据,
    for (i=0; i<8; i++)         //8位计数器
    {
        dat <<= 1;
        SCL = 1;                //拉高时钟线
        Delay5us();             //延时
        dat |= SDA;             //读数据               
        SCL = 0;                //拉低时钟线
        Delay5us();             //延时
    }
    return dat;
}
//*********************************************************
void Single_Write_BH1750(uchar REG_Address)
{
    BH1750_Start();                  //起始信号
    BH1750_SendByte(SlaveAddress);   //发送设备地址+写信号
    BH1750_SendByte(REG_Address);    //内部寄存器地址，请参考中文pdf22页 
  //  BH1750_SendByte(REG_data);       //内部寄存器数据，请参考中文pdf22页 
    BH1750_Stop();                   //发送停止信号
}
//*********************************************************
//连续读出BH1750内部数据
//*********************************************************
void Multiple_read_BH1750(void)
{   uchar i;	
    BH1750_Start();                     //起始信号
    BH1750_SendByte(SlaveAddress+1);    //发送设备地址+读信号
	
	 for (i=0; i<3; i++)                  //连续读取6个地址数据，存储中BUF
    {
        BUF[i] = BH1750_RecvByte();     //BUF[0]存储0x32地址中的数据
        if (i == 3)
        {

           BH1750_SendACK(1);           //最后一个数据需要回NOACK
        }
        else
        {		
          BH1750_SendACK(0);             //回应ACK
       }
   }

    BH1750_Stop();                       //停止信号
    Delay5ms();
}
//*********************************************************
void Init_BH1750()
{
   Single_Write_BH1750(0x01);  

}
//*********************************************************
unsigned int Read_Light()
{
		Init_BH1750();       					//初始化BH1750
	
    Single_Write_BH1750(0x01);   	// power on
    Single_Write_BH1750(0x10);   	// H- resolution mode

    delay_nms(130);              //延时130ms

    Multiple_read_BH1750();       //连续读出数据，存储在BUF中

    dis_data=BUF[0];
    dis_data=(dis_data<<8)+BUF[1];//合成数据 
    
    illumination=(float)dis_data/1.2;
		return illumination;
}