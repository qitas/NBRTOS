#ifndef _BH1750_H_
#define _BH1750_H_



#define BH1750_Addr 0x46
#define BH1750_ON   0x01
#define BH1750_CON  0x10
#define BH1750_ONE  0x20
#define BH1750_RSET 0x07





#define IIC_1750_OK		0
#define IIC_1750_Err		1



//SDA		PB12
//SCL		PB13
#define SDA1750_H	GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define SDA1750_L	GPIO_ResetBits(GPIOB, GPIO_Pin_12)
#define SDA1750_R	GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)

#define SCL1750_H	GPIO_SetBits(GPIOB, GPIO_Pin_13)
#define SCL1750_L	GPIO_ResetBits(GPIOB, GPIO_Pin_13)


typedef struct
{

	unsigned short speed;

} IIC_1750_INFO;

extern IIC_1750_INFO iic1750Info;




void IIC_1750_Init(void);

void IIC_1750_SpeedCtl(unsigned short speed);

_Bool I2C_1750_WriteByte(unsigned char slaveAddr, unsigned char regAddr, unsigned char byte);

_Bool I2C_1750_ReadByte(unsigned char slaveAddr, unsigned char regAddr, unsigned char *val);

_Bool I2C_1750_WriteBytes(unsigned char slaveAddr, unsigned char regAddr, unsigned char *buf, unsigned char num);

_Bool I2C_1750_ReadBytes(unsigned char slaveAddr, unsigned char regAddr, unsigned char *buf, unsigned char num);

void IIC_1750_Start(void);

void IIC_1750_Stop(void);

_Bool IIC_1750_WaitAck(unsigned int timeOut);

void IIC_1750_Ack(void);

void IIC_1750_NAck(void);

void IIC_1750_SendByte(unsigned char byte);

unsigned char IIC_1750_RecvByte(void);




void Init_BH1750(void);								  //IO初始化，
void Start_BH1750(void);			//上电，设置清除数据寄存器
_Bool Read_BH1750(void);                               //连续的读取内部寄存器数据
void Convert_BH1750(void);
_Bool Cmd_Write_BH1750(unsigned int cmd);
void BH1750_test(void);
#endif
