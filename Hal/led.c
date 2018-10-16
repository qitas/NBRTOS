/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	led.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		LED初始化，亮灭LED
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//LED头文件
#include "led.h"
#include "delay.h"




LED_STATUS ledStatus;


/*
************************************************************
*	函数名称：	Led_Init
*
*	函数功能：	LED初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		LED4-PC7	LED5-PC8	LED6-PA12	LED7-PC10
				高电平关灯		低电平开灯
************************************************************
*/
void Led_Init(void)
{	
		GPIO_InitTypeDef gpioInitStrcut;	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC , ENABLE);	//打开GPIOA和GPIOC的时钟
		
		gpioInitStrcut.GPIO_Mode = GPIO_Mode_Out_PP;									//设置为推挽输出模式
		gpioInitStrcut.GPIO_Pin = GPIO_Pin_12;												//初始化Pin12
		gpioInitStrcut.GPIO_Speed = GPIO_Speed_50MHz;									//承载的最大频率
		GPIO_Init(GPIOA, &gpioInitStrcut);
		
		gpioInitStrcut.GPIO_Mode = GPIO_Mode_Out_PP;									//设置为推挽输出模式
		gpioInitStrcut.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 |GPIO_Pin_10;					//初始化Pin8、9、10
		gpioInitStrcut.GPIO_Speed = GPIO_Speed_50MHz;									//承载的最大频率
		GPIO_Init(GPIOC, &gpioInitStrcut);												//初始化GPIOC

	  Led1_Set(LED_OFF);
    Led2_Set(LED_OFF);
    Led3_Set(LED_OFF);
    Led4_Set(LED_OFF);

}

/*
************************************************************
*	函数名称：	Led1_Set
*
*	函数功能：	LED1控制
*
*	入口参数：	status：LED_ON-开灯	LED_OFF-关灯
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Led1_Set(LED_ENUM status)
{

	GPIO_WriteBit(GPIOC, GPIO_Pin_10, status == LED_ON ?Bit_RESET:Bit_SET);	//status如果不等于LED_ON则返回Bit_SET，否则返回Bit_RESET。下同
	ledStatus.Led1Sta = status;

}

/*
************************************************************
*	函数名称：	Led2_Set
*
*	函数功能：	LED2控制
*
*	入口参数：	status：LED_ON-开灯	LED_OFF-关灯
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Led2_Set(LED_ENUM status)
{

	GPIO_WriteBit(GPIOA, GPIO_Pin_12, status == LED_ON ?Bit_RESET:Bit_SET);
	ledStatus.Led2Sta = status;

}

/*
************************************************************
*	函数名称：	Led3_Set
*
*	函数功能：	LED3控制
*
*	入口参数：	status：LED_ON-开灯	LED_OFF-关灯
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Led3_Set(LED_ENUM status)
{

	GPIO_WriteBit(GPIOC, GPIO_Pin_8, status == LED_ON ?Bit_RESET:Bit_SET);
	ledStatus.Led3Sta = status;

}

/*
************************************************************
*	函数名称：	Led4_Set
*
*	函数功能：	LED4控制
*
*	入口参数：	status：LED_ON-开灯	LED_OFF-关灯
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Led4_Set(LED_ENUM status)
{

	GPIO_WriteBit(GPIOC, GPIO_Pin_7, status == LED_ON ?Bit_RESET:Bit_SET);
	ledStatus.Led4Sta = status;

}

/*
************************************************************
*	函数名称：	Led_test
*
*	函数功能：	流水灯
*
*	入口参数：	延时时间
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Led_test(int ms)
{

		Led1_Set(LED_OFF);mDelay(ms);Led1_Set(LED_ON);		//点亮LED4，并延时500ms，然后熄灭LED4
		
		Led2_Set(LED_OFF);mDelay(ms);Led2_Set(LED_ON);		//点亮LED5，并延时500ms，然后熄灭LED5
		
		Led3_Set(LED_OFF);mDelay(ms);Led3_Set(LED_ON);		//点亮LED6，并延时500ms，然后熄灭LED6
		
		Led4_Set(LED_OFF);mDelay(ms);Led4_Set(LED_ON);		//点亮LED7，并延时500ms，然后熄灭LED7

}
