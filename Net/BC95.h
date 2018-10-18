#ifdef BC95


#include <stdint.h>
#include <string.h>
#include "internal.h"

typedef enum
{
	MIPL_DEBUG_LEVEL_NONE = 0,
	MIPL_DEBUG_LEVEL_RXL,
	MIPL_DEBUG_LEVEL_RXL_RXD,
	MIPL_DEBUG_LEVEL_TXL_TXD,
} MIPL_DEBUG_LEVEL_E;


#define MIPL_BLOCK1  5
#define MIPL_BLOCK2  5
#define MIPL_BLOCK2TH  2
#define MIPL_PORT  0
#define MIPL_KEEPALIVE  300
#define MIPL_DEBUG  MIPL_DEBUG_LEVEL_NONE
#define MIPL_BOOT  0
#define MIPL_ENCRYPT  0

#define NBIOT_SOCK_BUF_SIZE 1024

typedef struct
{
	uint8_t boot;
	uint8_t encrypt;
	MIPL_DEBUG_LEVEL_E debug;
	uint16_t port;
	uint32_t keep_alive;
	size_t uri_len;
	const char* uri;
	size_t ep_len;
	const char* ep;
	uint8_t block1;	//COAP option BLOCK1(PUT or POST),0-6. 2^(4+n)  bytes
	uint8_t block2;	//COAP option BLOCK2(GET),0-6. 2^(4+n)  bytes
	uint8_t block2th;	//max size to trigger block-wise operation,0-2. 2^(8+n) bytes
} MIPL_T;

typedef void (*sendmsg)(uint8_t * buf, uint32_t len);

void netdev_init(void);

uint32_t ip_SendData(int8_t * buf, uint32_t len);

void netif_rx(uint8_t*buf,uint16_t *read);

void nb_addobj(uint16_t		objid,uint16_t		attrcount,uint16_t		actcount);
void nb_delobj(uint16_t 	 objid);
 size_t nb_register_request( uint8_t  *buffer,									    
                                size_t    buffer_len);

 size_t nb_register_update (uint16_t lifttime, 
 	                           uint8_t *buffer,									    
                               size_t  buffer_len);

 size_t nb_close_request( uint8_t  *buffer,									    
                             size_t    buffer_len);

void nb_notify_upload(const nbiot_uri_t uri,uint8_t type,char *data);

void nb_read_upload(const nbiot_uri_t uri,uint8_t type,char *data);

void nb_observe_rsp(int suc,const nbiot_uri_t uri);

void nb_discover_rsp(const nbiot_uri_t *uri,size_t lenth,char *value);
	
void nb_write_rsp(int suc,uint16_t ackid);

void nb_execute_rsp(int suc,uint16_t ackid);

void register_cmd_handler(sendmsg func,void *result_buf,volatile char *flag);
char SendCmd(char* cmd, uint8_t *result,uint16_t timeout,uint8_t retry,uint16_t waittime);
void SentData(char* cmd, uint8_t *result,uint16_t timeout);

#endif
