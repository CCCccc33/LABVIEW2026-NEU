/**************************************
工作频率: 24.000MHz
**************************************/
#include "REG51.H"
#include "INTRINS.H"
#include "DS18B20.H"

typedef enum   
{  
    SEARCH_ROM          =   0xf0,   //搜索ROM指令  
    READ_ROM            =   0x33,   //读取ROM指令
    MATH_ROM            =   0x55,   //匹配ROM指令
    SKIP_ROM            =   0xcc,   //跳过ROM指令
    ALARM_SEARCH        =   0xec,   //报警搜索指令 
    CONVERT_T           =   0x44,   //温度转换指令
    WRITE_SCRATCHPAD    =   0x4e,   //写暂存器指令
    READ_SCRATCHPAD     =   0xbe,   //读取转存器指令
    COPY_SCRATCHPAD     =   0x48,   //拷贝暂存器指令  
    RECALL_E2           =   0xb8,   //召回EEPROM指令
    READ_POWER_SUPPLY   =   0xb4,   //读取电源模式指令  
} DS18B20_CMD; 

unsigned char xdata seril[8];
extern unsigned char xdata Temp_data[32];
//*********************************************************************
//void read_seril()
//{	
//		unsigned char i;
//		DS18B20_Reset();
//		DS18B20_WriteByte(0x33);				
//		for(i=0;i<8;i++)
//	{
//		seril[i] = DS18B20_ReadByte();
//		Temp_data[i] = seril[i];
//	}
//}

//*************************延时X微秒(*2)********************************
void DelayXus(unsigned char n)
{
    while (n--)
    {
        _nop_();
        _nop_();
    }
}

//*************************复位DS18B20，返回0则设备存在*******************
bit DS18B20_Reset()
{
		bit result;
		DQ = 0;                     	//送出低电平复位信号
		DelayXus(240);              	//延时至少480us
		DelayXus(240);
		DelayXus(240);
		DelayXus(240);
		DQ = 1;                     	//释放数据线
		DelayXus(120);              	//等待60us
		result = DQ;                 	//检测存在脉冲
		DelayXus(240);              	//等待设备释放数据线
		DelayXus(240);
		DelayXus(180);
		DelayXus(180);
		return result;
}
//*************************************************************************
bit DS18B20_Readbit()
{
		bit res;
		DQ = 0;                     	//开始时间片
		DelayXus(3);                	//延时等待
		DQ = 1;                     	//准备接收
		DelayXus(3);                	//接收延时
		res=DQ;				        				//读取数据
		DelayXus(120);               	//等待时间片结束
		return res;
}
//*************************从DS18B20读1字节数据*****************************
unsigned char DS18B20_ReadByte()
{
    unsigned char i;
    unsigned char dat = 0;

    for (i=0; i<8; i++)
    {
        dat >>= 1;
        if (DS18B20_Readbit()) dat |= 0x80; //读取数据
    }

    return dat;
}

//*************************向DS18B20写1字节数据*****************************
void DS18B20_WriteByte(unsigned char dat)
{
    char i;

    for (i=0; i<8; i++)             	//8位计数器
    {
        DQ = 0;                     	//开始时间片
        DelayXus(3);                	//延时等待
        dat >>= 1;                 	 	//送出数据
        DQ = CY;
        DelayXus(120);               	//等待时间片结束
        DQ = 1;                     	//恢复数据线
        DelayXus(3);                	//恢复延时
    }
}
//*************************读取指定ID的DS18B20的温度值************************
unsigned int DS18B20_ReadDesignateTemper(unsigned char rom[8])  
{  
    unsigned char th, tl;  
    unsigned int mdata;  
//		read_seril();												//读取总线上每个DS18B20的序列号，测试时使用
		DS18B20_Reset(); 
    DS18B20_WriteByte(SKIP_ROM);        //跳过ROM指令
    DS18B20_WriteByte(CONVERT_T);       //启动温度转换 指令
    DS18B20_Reset();  
    DS18B20_WriteByte(MATH_ROM);        //匹配ROM指令
    for(mdata = 0;mdata < 8;mdata ++)   //发送8个字节的序列号   
    {  
       DS18B20_WriteByte(rom[mdata]);  
    }  
    DelayXus(20);;  
    DS18B20_WriteByte(READ_SCRATCHPAD); //读取温度指令 
    tl = DS18B20_ReadByte();    				//低8位数据
    th = DS18B20_ReadByte();    				//高8位数据
    mdata = (th<<8) | tl;   
		
    return mdata;												//返回值;  
}
