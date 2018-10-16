#ifndef USART2_H_H
#define USART2_H_H
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


/*
 *  @brief USART2初始化函数
 */
extern void USART3_Init(void);
/*
 *  @brief USART2串口发送api
 */
void USART3_Write(uint8_t *Data, uint32_t len);

#endif

