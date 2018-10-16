/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#ifndef ONENET_PLATFORM_H_
#define ONENET_PLATFORM_H_

//#if defined(NBIOT_WIN) || defined(NBIOT_POSIX)
#include <stddef.h>   /* for size_t */
#include <stdint.h>   /* for integer types */
#include <stdbool.h>  /* for bool */
#include <time.h>     /* for time_t clock_t */
#include <float.h>    /* for DBL_MAX */
#include <inttypes.h> /* fro PRId64 */
//#endif

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NO_SOCKET
#define inline 
#endif	
/**
 * 初始化环境
**/
void nbiot_init_environment(void);

/**
 * 清理环境
**/
void nbiot_clear_environment( void );

/**
 * 分配内存
 * @param size 需要分配的内存字节数
 * @return 分配成功返回分配内存指针，否则返回NULL
**/
void *nbiot_malloc( size_t size );

/**
 * 释放由nbiot_malloc分配的内存
 * @param ptr 指向将要被释放的内存
**/
void nbiot_free( void *ptr );

/**
 * 获取当前时间
 * @return 返回当前距(00:00:00 UTC, January 1, 1970)的秒数
**/
time_t nbiot_time( void );

/**
 * 休眠
 * @param milliseconds 休眠时长
**/
void nbiot_sleep( int milliseconds );

/**
 * socket句柄，移植时自行定义其具体属性
**/


void nbiot_time_init(void);


void nbiot_reset(void);
/**
 * 发送数据
 * @param sock 指向UDP socket句柄的内存
 *        buff 指向将要被发送的数据
 *        size 将要被发送的数据字节数
 *        sent 实际发送成功的字节数
 *        dest 指向目标地址信息的内存
 * @return 发送数据正常返回NBIOT_ERR_OK
**/
int nbiot_udp_send( const void	*buff,
					           size_t			size);


/**
 * 接收数据
 * @param sock 指向UDP socket句柄的内存
 *        buff 指向存储接收数据的缓冲区
 *        size 接收数据缓存区的最大字节数
 *        read [OUT] 实际接收到的数据字节数
 *        src  [OUT] 指向源地址信息的内存,
 *                   如果接收成功，src=NULL时，自动创建nbiot_sockaddr_t结构
 * @return 接收数据正常返回NBIOT_ERR_OK
**/
int nbiot_udp_recv( void			  *buff,
					size_t			   size,
					size_t			  *read);



#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* ONENET_PLATFORM_H_ */
