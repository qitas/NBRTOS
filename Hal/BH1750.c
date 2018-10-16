/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	i2c.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		软件IIC总线IO初始化，读写控制
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/
//单片机头文件
#include "stm32f10x.h"

//硬件驱动
#include "BH1750.h"
#include "delay.h"
#include <stdio.h>

float result_lx=0;
u8 BUF[2]={0};
u16 result=0;


IIC_1750_INFO iic1750Info;



/*
************************************************************
*	函数名称：	IIC_1750_SpeedCtl
*
*	函数功能：	软件IIC速度控制
*
*	入口参数：	speed：延时参数
*
*	返回参数：	无
*
*	说明：		单位：微秒
************************************************************
*/
void IIC_1750_SpeedCtl(unsigned short speed)
{

	iic1750Info.speed = speed;

}

/*
************************************************************
*	函数名称：	IIC_1750_Init
*
*	函数功能：	软件IIC总线IO初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		使用开漏方式，这样可以不用切换IO口的输入输出方向
************************************************************
*/
void IIC_1750_Init(void)
{

	GPIO_InitTypeDef gpioInitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//打开GPIOB的时钟
	
	gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_OD;			//开漏，这样不用去切换输出输入方向
	gpioInitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpioInitStruct);
	
	IIC_1750_SpeedCtl(2);										//IIC速度控制，单位：微秒
	
	SDA1750_H;													//拉高SDA线，处于空闲状态
	SCL1750_H;													//拉高SCL线，处于空闲状态

}

/*
************************************************************
*	函数名称：	IIC_1750_Start
*
*	函数功能：	软件IIC开始信号
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void IIC_1750_Start(void)
{
	
	SDA1750_H;						//拉高SDA线
	SCL1750_H;						//拉高SCL线
	uDelay(iic1750Info.speed);		//延时，速度控制
	
	SDA1750_L;						//当SCL线为高时，SDA线一个下降沿代表开始信号
	uDelay(iic1750Info.speed);		//延时，速度控制
	SCL1750_L;						//钳住SCL线，以便发送数据

}

/*
************************************************************
*	函数名称：	IIC_1750_Stop
*
*	函数功能：	软件IIC停止信号
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void IIC_1750_Stop(void)
{

	SDA1750_L;						//拉低SDA线
	SCL1750_L;						//拉低SCL先
	uDelay(iic1750Info.speed);		//延时，速度控制
	
	SCL1750_H;						//拉高SCL线
	SDA1750_H;						//拉高SDA线，当SCL线为高时，SDA线一个上升沿代表停止信号
	uDelay(iic1750Info.speed);

}

/*
************************************************************
*	函数名称：	IIC_1750_WaitAck
*
*	函数功能：	软件IIC等待应答
*
*	入口参数：	timeOut：超时时间
*
*	返回参数：	无
*
*	说明：		单位：微秒
************************************************************
*/
_Bool IIC_1750_WaitAck(unsigned int timeOut)
{
	
	
	SDA1750_H;uDelay(iic1750Info.speed);			//拉高SDA线
	SCL1750_H;uDelay(iic1750Info.speed);			//拉高SCL线
	
	while(SDA1750_R)							//如果读到SDA线为1，则等待。应答信号应是0
	{
		if(--timeOut)
		{
			printf("WaitAck TimeOut\r\n");

			IIC_1750_Stop();						//超时未收到应答，则停止总线
			
			return IIC_1750_Err;					//返回失败
		}
		
		uDelay(iic1750Info.speed);
	}
	
	SCL1750_L;									//拉低SCL线，以便继续收发数据
	
	return IIC_1750_OK;							//返回成功
	
}

/*
************************************************************
*	函数名称：	IIC_1750_Ack
*
*	函数功能：	软件IIC产生一个应答
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		当SDA线为低时，SCL线一个正脉冲代表发送一个应答信号
************************************************************
*/
void IIC_1750_Ack(void)
{
	
	SCL1750_L;						//拉低SCL线
	SDA1750_L;						//拉低SDA线
	uDelay(iic1750Info.speed);
	SCL1750_H;						//拉高SCL线
	uDelay(iic1750Info.speed);
	SCL1750_L;						//拉低SCL线
	
}

/*
************************************************************
*	函数名称：	IIC_1750_NAck
*
*	函数功能：	软件IIC产生一非个应答
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		当SDA线为高时，SCL线一个正脉冲代表发送一个非应答信号
************************************************************
*/
void IIC_1750_NAck(void)
{
	
	SCL1750_L;						//拉低SCL线
	SDA1750_H;						//拉高SDA线
	uDelay(iic1750Info.speed);
	SCL1750_H;						//拉高SCL线
	uDelay(iic1750Info.speed);
	SCL1750_L;						//拉低SCL线
	
}

/*
************************************************************
*	函数名称：	IIC_1750_SendByte
*
*	函数功能：	软件IIC发送一个字节
*
*	入口参数：	byte：需要发送的字节
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void IIC_1750_SendByte(unsigned char byte)
{

	unsigned char count = 0;
	
    SCL1750_L;							//拉低时钟开始数据传输
	
    for(; count < 8; count++)		//循环8次，每次发送一个bit
    {
		if(byte & 0x80)				//发送最高位
			SDA1750_H;
		else
			SDA1750_L;
		
		byte <<= 1;					//byte左移1位
		
		uDelay(iic1750Info.speed);
		SCL1750_H;
		uDelay(iic1750Info.speed);
		SCL1750_L;
    }

}

/*
************************************************************
*	函数名称：	IIC_1750_RecvByte
*
*	函数功能：	软件IIC接收一个字节
*
*	入口参数：	无
*
*	返回参数：	接收到的字节数据
*
*	说明：		
************************************************************
*/
unsigned char IIC_1750_RecvByte(void)
{
	
	unsigned char count = 0, receive = 0;
	
	SDA1750_H;							//拉高SDA线，开漏状态下，需线拉高以便读取数据
	
    for(; count < 8; count++ )		//循环8次，每次发送一个bit
	{
		SCL1750_L;
		uDelay(iic1750Info.speed);
		SCL1750_H;
		
        receive <<= 1;				//左移一位
		
        if(SDA1750_R)					//如果SDA线为1，则receive变量自增，每次自增都是对bit0的+1，然后下一次循环会先左移一次
			receive++;
		
		uDelay(iic1750Info.speed);
    }
	
    return receive;
	
}

/*
************************************************************
*	函数名称：	I2C_1750_WriteByte
*
*	函数功能：	软件IIC写一个数据
*
*	入口参数：	slaveAddr：从机地址
*				regAddr：寄存器地址
*				byte：需要写入的数据
*
*	返回参数：	0-写入成功	1-写入失败
*
*	说明：		*byte是缓存写入数据的变量的地址，因为有些寄存器只需要控制下寄存器，并不需要写入值
************************************************************
*/
_Bool I2C_1750_WriteByte(unsigned char slaveAddr, unsigned char regAddr, unsigned char byte)
{

	unsigned char addr = 0;

	addr = slaveAddr << 1;		//IIC地址是7bit，这里需要左移1位，bit0：1-读	0-写
	
	IIC_1750_Start();				//起始信号
	
	IIC_1750_SendByte(addr);			//发送设备地址(写)
	if(IIC_1750_WaitAck(5000))		//等待应答
		return IIC_1750_Err;
	
	IIC_1750_SendByte(regAddr);		//发送寄存器地址
	if(IIC_1750_WaitAck(5000))		//等待应答
		return IIC_1750_Err;
	
	if(byte)
	{
		IIC_1750_SendByte(byte);	//发送数据
		if(IIC_1750_WaitAck(5000))	//等待应答
			return IIC_1750_Err;
	}
	
	IIC_1750_Stop();					//停止信号
	
	return IIC_1750_OK;

}

/*
************************************************************
*	函数名称：	I2C_1750_ReadByte
*
*	函数功能：	软件IIC读取一个字节
*
*	入口参数：	slaveAddr：从机地址
*				regAddr：寄存器地址
*				val：需要读取的数据缓存
*
*	返回参数：	0-成功		1-失败
*
*	说明：		val是一个缓存变量的地址
************************************************************
*/
_Bool I2C_1750_ReadByte(unsigned char slaveAddr, unsigned char regAddr, unsigned char *val)
{

	unsigned char addr = 0;

    addr = slaveAddr << 1;		//IIC地址是7bit，这里需要左移1位，bit0：1-读	0-写
	
	IIC_1750_Start();				//起始信号
	
	IIC_1750_SendByte(addr);			//发送设备地址(写)
	if(IIC_1750_WaitAck(5000))		//等待应答
		return IIC_1750_Err;
	
	IIC_1750_SendByte(regAddr);		//发送寄存器地址
	if(IIC_1750_WaitAck(5000))		//等待应答
		return IIC_1750_Err;
	
	IIC_1750_Start();				//重启信号
	
	IIC_1750_SendByte(addr + 1);		//发送设备地址(读)
	if(IIC_1750_WaitAck(5000))		//等待应答
		return IIC_1750_Err;
	
	*val = IIC_1750_RecvByte();		//接收
	IIC_1750_NAck();					//产生一个非应答信号，代表读取接收
	
	IIC_1750_Stop();					//停止信号
	
	return IIC_1750_OK;

}

/*
************************************************************
*	函数名称：	I2C_1750_WriteBytes
*
*	函数功能：	软件IIC写多个数据
*
*	入口参数：	slaveAddr：从机地址
*				regAddr：寄存器地址
*				buf：需要写入的数据缓存
*				num：数据长度
*
*	返回参数：	0-写入成功	1-写入失败
*
*	说明：		*buf是一个数组或指向一个缓存区的指针
************************************************************
*/
_Bool I2C_1750_WriteBytes(unsigned char slaveAddr, unsigned char regAddr, unsigned char *buf, unsigned char num)
{

	unsigned char addr = 0;

	addr = slaveAddr << 1;		//IIC地址是7bit，这里需要左移1位，bit0：1-读	0-写
	
	IIC_1750_Start();				//起始信号
	
	IIC_1750_SendByte(addr);			//发送设备地址(写)
	if(IIC_1750_WaitAck(5000))		//等待应答
		return IIC_1750_Err;
	
	IIC_1750_SendByte(regAddr);		//发送寄存器地址
	if(IIC_1750_WaitAck(5000))		//等待应答
		return IIC_1750_Err;
	
	while(num--)				//循环写入数据
	{
		IIC_1750_SendByte(*buf);		//发送数据
		if(IIC_1750_WaitAck(5000))	//等待应答
			return IIC_1750_Err;
		
		buf++;					//数据指针偏移到下一个
		
		uDelay(10);
	}
	
	IIC_1750_Stop();					//停止信号
	
	return IIC_1750_OK;

}

/*
************************************************************
*	函数名称：	I2C_1750_ReadBytes
*
*	函数功能：	软件IIC读多个数据
*
*	入口参数：	slaveAddr：从机地址
*				regAddr：寄存器地址
*				buf：需要读取的数据缓存
*				num：数据长度
*
*	返回参数：	0-写入成功	1-写入失败
*
*	说明：		*buf是一个数组或指向一个缓存区的指针
************************************************************
*/
_Bool I2C_1750_ReadBytes(unsigned char slaveAddr, unsigned char regAddr, unsigned char *buf, unsigned char num)
{

	unsigned short addr = 0;

    addr = slaveAddr << 1;		//IIC地址是7bit，这里需要左移1位，bit0：1-读	0-写
	
	IIC_1750_Start();				//起始信号
	
	IIC_1750_SendByte(addr);			//发送设备地址(写)
	if(IIC_1750_WaitAck(5000))		//等待应答
		return IIC_1750_Err;
	
	IIC_1750_SendByte(regAddr);		//发送寄存器地址
	if(IIC_1750_WaitAck(5000))		//等待应答
		return IIC_1750_Err;
	
	IIC_1750_Start();				//重启信号
	
	IIC_1750_SendByte(addr + 1);		//发送设备地址(读)
	if(IIC_1750_WaitAck(5000))		//等待应答
		return IIC_1750_Err;
	
	while(num--)
	{
		*buf = IIC_1750_RecvByte();
		buf++;					//偏移到下一个数据存储地址
		
		if(num == 0)
        {
           IIC_1750_NAck();			//最后一个数据需要回NOACK
        }
        else
        {
          IIC_1750_Ack();			//回应ACK
		}
	}
	
	IIC_1750_Stop();
	
	return IIC_1750_OK;  

}


_Bool Cmd_Write_BH1750(unsigned int cmd)
{
    IIC_1750_Start();                  //起始信号
    IIC_1750_SendByte(BH1750_Addr+0);   //发送设备地址+写信号
	if(IIC_1750_WaitAck(5000))		//等待应答
	return IIC_1750_Err;
    IIC_1750_SendByte(cmd);    //内部寄存器地址
	if(IIC_1750_WaitAck(5000))		//等待应答
	return IIC_1750_Err;
    //BH1750_SendByte(REG_data);       //内部寄存器数据，
    IIC_1750_Stop();                   //发送停止信号
	uDelay(5000);
	return IIC_1750_OK;

}
void Start_BH1750(void)
{
	Cmd_Write_BH1750(BH1750_ON);	 //power on
	Cmd_Write_BH1750(BH1750_RSET);	//clear
	Cmd_Write_BH1750(BH1750_ONE);  //一次H分辨率模式，至少120ms，之后自动断电模式  
}
_Bool Read_BH1750(void)
{   	
    IIC_1750_Start();                          //起始信号
    IIC_1750_SendByte(BH1750_Addr+1);         //发送设备地址+读信号
	if(IIC_1750_WaitAck(5000))		//等待应答
	return IIC_1750_Err;
	BUF[0]=IIC_1750_RecvByte();  //发送ACK
	IIC_1750_Ack();			//回应ACK
	BUF[1]=IIC_1750_RecvByte();  //发送NACK
    IIC_1750_NAck();			//最后一个数据需要回NOACK
    IIC_1750_Stop();                          //停止信号
    uDelay(5000);
	return IIC_1750_OK;
}
void Convert_BH1750(void)
{
	
	result=BUF[0];
	result=(result<<8)+BUF[1];  //合成数据，即光照数据
	
	result_lx=(float)result/1.2;
	//printf("BH1750 DATA is: %f LUX\r\n",result_lx);

}

void BH1750_test(void)
{
  Start_BH1750();
	mDelay(200);
	Read_BH1750();
	Convert_BH1750();
	
}



