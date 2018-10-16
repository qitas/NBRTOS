/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	key.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		按键IO初始化，按键功能判断
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//按键头文件
#include <string.h>
#include "key.h"
//硬件驱动
#include "delay.h"
#include "beep.h"
#include "sht20.h"
#include "stdio.h"
#include "nbiot.h"
#include "platform.h"
#include "utils.h"
/*
************************************************************
*	函数名称：	Key_Init
*
*	函数功能：	按键IO初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		SW2-PD2		SW3-PC11	SW4-PC12	SW5-PC13	
*				按下为低电平		释放为高电平
************************************************************
*/
void Key_Init(void)
{
	GPIO_InitTypeDef gpioInitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;  
  NVIC_InitTypeDef NVIC_InitStruct;  
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);	//打开GPIOC
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE); 
	
	gpioInitStructure.GPIO_Mode = GPIO_Mode_IPU;									//设置为上拉输入模式
	gpioInitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;			//初始化Pin11、12、13
	gpioInitStructure.GPIO_Speed = GPIO_Speed_50MHz;								//承载的最大频率
	
	GPIO_Init(GPIOC, &gpioInitStructure);											//初始化GPIOC
	
    NVIC_InitStruct.NVIC_IRQChannel =EXTI15_10_IRQn;  
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;  
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 4;  
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;  
      
    NVIC_Init(&NVIC_InitStruct);  
	
    EXTI_ClearITPendingBit(EXTI_Line11);  
	//	EXTI_DeInit();
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource11);  
    EXTI_InitStructure.EXTI_Line=EXTI_Line11;  
    EXTI_InitStructure.EXTI_LineCmd =ENABLE;  
    EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;  
    EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling;  
  
    EXTI_Init(&EXTI_InitStructure);  
	
}

/*
************************************************************
*	函数名称：	KeyScan
*
*	函数功能：	按键电平扫描
*
*	入口参数：	GPIOX：需要扫描的GPIO组	NUM：该GPIO组内的编号
*
*	返回参数：	IO电平状态
*
*	说明：		
************************************************************
*/
_Bool KeyScan(GPIO_TypeDef* GPIOX, unsigned int NUM)
{
	
	if(GPIOX == GPIOC)
	{
		if(!GPIO_ReadInputDataBit(GPIOC, NUM))	//按下  为低
		{
			return KEYDOWN;
		}
		else									//弹起  为高
		{
			return KEYUP;
		}
	}

		
	return KEYUP;								//默认返回按键释放
	
}

/*
************************************************************
*	函数名称：	Keyboard
*
*	函数功能：	按键功能检测
*
*	入口参数：	
*
*	返回参数：	IO电平状态
*
*	说明：		
************************************************************
*/
unsigned char Keyboard(void)
{
	
	if(KeyScan(GPIOC, KEY0) == KEYDOWN)
	{
		while(KeyScan(GPIOC, KEY0) != KEYUP);		//等待释放
		return KEY0DOWN;
	}
	
	if(KeyScan(GPIOC, KEY1) == KEYDOWN)
	{
		while(KeyScan(GPIOC, KEY1) != KEYUP);		//等待释放
		return KEY1DOWN;
	}
	
	if(KeyScan(GPIOC, KEY2) == KEYDOWN)
	{
		while(KeyScan(GPIOC, KEY2) != KEYUP);		//等待释放
		return KEY2DOWN;
	}
	
	if(KeyScan(GPIOC, KEY3) == KEYDOWN)
	{
		while(KeyScan(GPIOC, KEY3) != KEYUP);		//等待释放
		return KEY3DOWN;
	}
	
	return KEYNONE;
	
}

/*
************************************************************
*	函数名称：	Key_test
*
*	函数功能：	按键测试
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		SW0-PC0		SW1-PC1	SW2-PC2	SW3-PC3	
*				按下为低电平		释放为高电平
************************************************************
*/
extern nbiot_value_t dicr;
void Key_test(void)
{

			 // printf("hello\r\n");
	     if(!(dicr.flag&NBIOT_UPDATED)){
			  dicr.flag |= NBIOT_UPDATED;
        nbiot_free(dicr.value.as_buf.val);
				dicr.value.as_buf.len = strlen("trigger");
        dicr.value.as_buf.val = nbiot_strdup("trigger", dicr.value.as_buf.len);
			}	 

}

