#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "bc95.h"
#include "stm32f10x.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "utils.h"
#include "ringbuf.h"
#include "fifo.h"
#include "at_cmd.h"
#include "led.h"
volatile char rsp_ok=0;
struct ringbuf ring_fifo;
struct ringbuf ring_fifo1;
static uint8_t rx_fifo[512];
static uint8_t rx_fifo1[512];
int8_t dl_buf_id=-1;
FIFO(dl_buf,8,512); 
#define UART_DMA 1
#define MAX_RCV_LEN 512
#ifdef UART_DMA
#include "stm32f10x.h"
#include "stm32f10x_dma.h"
DMA_Channel_TypeDef *USARx_DMA[4]={NULL,DMA1_Channel5,DMA1_Channel6,DMA1_Channel3};
DMA_Channel_TypeDef *USATx_DMA[4]={NULL,DMA1_Channel4,DMA1_Channel7,DMA1_Channel2};
#endif
/*
 *  @brief USART3初始化函数
 */
 
 /*
*  @brief USART3串口发送api
*/
void USART3_Write(uint8_t *Data, uint32_t len)
{

#ifndef UART_DMA  
    uint32_t i;
    USART_ClearFlag(USART3, USART_FLAG_TC);
    for(i = 0; i < len; i++)
    {
        USART_SendData(USART3, *Data++);
        while( USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET );
    }		
		
#else	
    DMA_InitTypeDef DMA_InitStruct;
 //   DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);
    DMA_Cmd(USATx_DMA[3],DISABLE);
    DMA_InitStruct.DMA_PeripheralBaseAddr = (u32)(&USART3->DR);
    DMA_InitStruct.DMA_MemoryBaseAddr = (u32)Data;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStruct.DMA_BufferSize = len;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(USATx_DMA[3],&DMA_InitStruct);
    DMA_Cmd(USATx_DMA[3],ENABLE);  
#endif 
    rsp_ok=0;		
}
void USART3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	  USART_InitTypeDef USART_InitStructure;
   	NVIC_InitTypeDef NVIC_InitStructure;
 #ifdef UART_DMA
   DMA_InitTypeDef DMA_InitStructure;
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  
#endif	 
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
   	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); 
	  USART_DeInit(USART3); 
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure); 
   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure); 

	  USART_InitStructure.USART_BaudRate = 9600;
	  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	  USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
	  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
    
		ringbuf_init(&ring_fifo, rx_fifo, sizeof(rx_fifo));
		ringbuf_init(&ring_fifo1, rx_fifo1, sizeof(rx_fifo1));
    dl_buf_id=fifo_init(&dl_buf);
    register_cmd_handler(USART3_Write,&ring_fifo1,&rsp_ok);
		
    USART_Init(USART3, &USART_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	
	  NVIC_Init(&NVIC_InitStructure);	
  #ifdef UART_DMA 
    //DMA1通道4配置  
    DMA_DeInit(USARx_DMA[3]);  
    //外设地址  
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART3->DR);  
    //内存地址  
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rx_fifo;  
    //dma传输方向单向  
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  
    //设置DMA在传输时缓冲区的长度  
    DMA_InitStructure.DMA_BufferSize = MAX_RCV_LEN;  
    //设置DMA的外设递增模式，一个外设  
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
    //设置DMA的内存递增模式  
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
    //外设数据字长  
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
    //内存数据字长  
    DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;  
    //设置DMA的传输模式  
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
    //设置DMA的优先级别  
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;  
    //设置DMA的2个memory中的变量互相访问  
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
    DMA_Init(USARx_DMA[3],&DMA_InitStructure);  

    DMA_Cmd(USARx_DMA[3],ENABLE);  
    USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE); 
		USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE); 
    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
  #else
     USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
  #endif 
    USART_Cmd(USART3, ENABLE);                   
			
}

/*
 *  @brief USART3串口发送AT命令用
 */
void USART3_IRQHandler(void)
{	
     unsigned int data;
     uint8_t *msg_p=NULL;
    if(USART3->SR & 0x0F)
    {
        data = USART3->DR;
    }
#ifndef UART_DMA 		
    else if(USART3->SR & USART_FLAG_RXNE)  
    {		
            data = USART3->DR;
	    ringbuf_put(&ring_fifo,data);
	     if(ringbuf_elements(&ring_fifo)==1)
               USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);

			                    
    }
#endif		
		else if(USART3->SR & USART_FLAG_IDLE){
                 data=USART3->SR;
		             data=USART3->DR;
 #ifndef UART_DMA                 
                 USART_ITConfig(USART3, USART_IT_IDLE, DISABLE);    
#else
                 DMA_Cmd(USARx_DMA[3], DISABLE);  //先停止DMA才行设置缓冲区大小 
                 ring_fifo.put_ptr = MAX_RCV_LEN - DMA_GetCurrDataCounter(USARx_DMA[3]);  
#endif 
            if((msg_p= strstr((const char *)ring_fifo.data, "+MIPL"))!=NULL) {
                 fifo_put(dl_buf_id,ringbuf_elements(&ring_fifo),ring_fifo.data);                            
            }else if((msg_p= strstr((const char *)ring_fifo.data, "+CEREG:0"))!=NULL){
						     Led3_Set(LED_OFF);  
						}else if((msg_p= strstr((const char *)ring_fifo.data, "+CEREG:1"))!=NULL){
						     Led3_Set(LED_ON); 
						}else{
                  rsp_ok=1;
                  ring_fifo1.get_ptr=ring_fifo.get_ptr;
                  ring_fifo1.put_ptr= ring_fifo.put_ptr;
                  memcpy(ring_fifo1.data,ring_fifo.data,ringbuf_elements(&ring_fifo));
              } 
              ringbuf_clear(&ring_fifo);   
	}
#ifdef UART_DMA
       DMA_SetCurrDataCounter(USARx_DMA[3],MAX_RCV_LEN);   //重新设置DMA的读取缓冲区长度 
       DMA_Cmd(USARx_DMA[3], ENABLE);  //开启DMA    
#endif  		
}

#if 0
void USART1_IRQHandler(void)
{
     unsigned int data;
     uint8_t *msg_p=NULL;
    if(USART1->SR & 0x0F)
    {
        data = USART1->DR;
    }	
    else if(USART1->SR & USART_FLAG_RXNE)  
    {		
        data = USART1->DR;
   	    ringbuf_put(&ring_fifo,data);
	     if(ringbuf_elements(&ring_fifo)==1)
           USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

			                    
    }		
		else if(USART1->SR & USART_FLAG_IDLE){
                 data=USART1->SR;
		             data=USART1->DR;              
                 USART_ITConfig(USART1, USART_IT_IDLE, DISABLE);   
 
        if((msg_p= strstr((const char *)ring_fifo.data, "+MIPL"))!=NULL) {
              fifo_put(dl_buf_id,ringbuf_elements(&ring_fifo),ring_fifo.data);                            
            }
        else{
                rsp_ok=1;
                ring_fifo1.get_ptr=ring_fifo.get_ptr;
                ring_fifo1.put_ptr= ring_fifo.put_ptr;
                memcpy(ring_fifo1.data,ring_fifo.data,ringbuf_elements(&ring_fifo));
          } 
              ringbuf_clear(&ring_fifo);   
	 }	
 }
#endif