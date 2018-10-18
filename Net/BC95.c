
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "BC95.h"
#include "at_cmd.h"
#include "utils.h"
//#include "fifo.h"
//#include "led.h"


signed char dl_buf_id=-1;
static char cmd_buff[1024];

uint32_t ip_SendData(int8_t * buf, uint32_t len)
{
     SentData(buf,"OK",100);
     return len;
}

void netif_rx(uint8_t*buf,uint16_t *read)
{
     uint8_t *msg_p=NULL;
     uint8_t ptr[1024]={0};
     //*read=fifo_get(dl_buf_id,ptr);
     if(*read!=0)
     {
        if((msg_p = strstr((const char *)ptr, "+MIPL"))!=NULL)
        {              
           memcpy(buf,ptr,*read);
        }
				else{
        
           *read=0;
        }
     }
       
}

void netdev_init(void)
{
		SendCmd("AT+NRB\r\n", "OK", 5000,0,10); 
	  SendCmd("AT+CIMI\r\n", "OK", 2000,3,5);
	  SendCmd("AT+CMEE=1\r\n","OK", 2000,0,10);
    SendCmd("AT+CSCON=1\r\n","OK", 2000,0,10);  
    SendCmd("AT+CEREG=2\r\n","OK", 2000,0,10);
    SendCmd("AT+CGATT=1\r\n","OK", 2000,0,10);	
    SendCmd("AT+CEREG?\r\n","CEREG:2,1", 3000,0,5);
    SendCmd("AT+CEDRXS=0,5\r\n","OK", 3000,0,5); 
	  SendCmd("AT+CPSMS=0\r\n","OK", 3000,0,5); 	
	  SendCmd("AT+CSQ\r\n", "OK", 2000,0,5); 
    printf("connect NB-IoT sucess\r\n");
} 

void bc95_addobj(uint16_t	objid,uint16_t	attrcount,uint16_t	actcount)
{
	char tmp[10];
	memset(cmd_buff,0,50);
	memcpy(cmd_buff,"AT+MIPLADDOBJ=0,",strlen("AT+MIPLADDOBJ=0,"));
	nbiot_itoa(objid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(1,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(1,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(attrcount,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(actcount,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,"\r\n");
	SendCmd(cmd_buff,"OK",300,0,5);
}

void bc95_delobj(uint16_t 	 objid)
{
	 char tmp[10];
	 memset(cmd_buff,0,50);
	 memcpy(cmd_buff,"AT+MIPLDELOBJ=0,",strlen("AT+MIPLDELOBJ=0,"));
	 nbiot_itoa(objid,tmp,10);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,"\r\n");
	 SendCmd(cmd_buff,"OK",300,0,5);
 }


 size_t bc95_register_request( uint8_t *buffer,size_t    buffer_len)
{   
		size_t  len=0;
		char status=0;
		len=strlen("AT+MIPLOPEN=0,2000\r\n")+1;
		if(len<buffer_len){
		memcpy(buffer,"AT+MIPLOPEN=0,2000\r\n",len);
		status=SendCmd("AT+MIPLOPEN=0,2000\r\n","OK",1000,0,5);
		if(status==2) SendCmd("AT+MIPLCLOSE=0\r\n","OK",300,0,5); 
		return len;
		}
		return 0;
}

size_t bc95_register_update(uint16_t lifttime, uint8_t *buffer,size_t  buffer_len)
{   
    size_t  len=0;
		char ative[6]={0};
		nbiot_itoa(lifttime,ative,6);
		memcpy(buffer,"AT+MIPLUPDATE=0,",sizeof("AT+MIPLUPDATE=0,"));
		strcat(buffer,ative);
		strcat(buffer,",0\r\n");
		len=strlen(buffer)+1;
		if(len<buffer_len){
			 SendCmd(buffer,"OK",300,0,5);
			 return len;
		}
		return 0;
}

 size_t bc95_close_request( uint8_t  *buffer,									    
                             size_t    buffer_len)
{   
    size_t  len=0;
	len=strlen("AT+MIPLCLOSE=0\r\n")+1;
	if(len<buffer_len){
	 memcpy(buffer,"AT+MIPLCLOSE=0\r\n",len);
	 SendCmd("AT+MIPLCLOSE=0\r\n","OK",300,0,5);
	 return len;
   }
	return 0;
}  


 void bc95_notify_upload(const nbiot_uri_t uri,uint8_t type,char *data)
{
	
  char tmp[10];
	uint8_t len = 0;
	memset(cmd_buff,0,sizeof(cmd_buff));
	memcpy(cmd_buff,"AT+MIPLNOTIFY=0",strlen("AT+MIPLNOTIFY=0"));
	strcat(cmd_buff,",");
	nbiot_itoa(uri.msgid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(uri.objid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(uri.instid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(uri.resid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
  nbiot_itoa(type,tmp,1);
  strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	if(type==1)
	{
		len = strlen(data);
		nbiot_itoa(len,tmp,10);
	}
	else if(type==2)
	{
		len = strlen(data);
		nbiot_itoa(len,tmp,10);
	}
	else if(type==3)
	{
		len = 2;
		nbiot_itoa(len,tmp,1);
	}
	else if(type==4)
	{
		len = 4;
		nbiot_itoa(len,tmp,1);
	}
	else
	{
		len = 1;
		nbiot_itoa(len,tmp,1);
	}
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	if(type==1||type==2)
	{
	strcat(cmd_buff,"\"");
	strcat(cmd_buff,data);
	strcat(cmd_buff,"\"");
	}
	else
	{
	strcat(cmd_buff,data);	
	}
	strcat(cmd_buff,",");
	nbiot_itoa(0,tmp,1);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,",");
  nbiot_itoa(0,tmp,1);
	strcat(cmd_buff,tmp);
  strcat(cmd_buff,"\r\n");
    printf("send data:");
    printf("%s\r\n",cmd_buff);
	  SentData(cmd_buff,"OK",100);
}

 void bc95_read_upload(const nbiot_uri_t uri,uint8_t type,char *data)
{
	
  char tmp[10];
	uint8_t len = 0;
	memset(cmd_buff,0,sizeof(cmd_buff));
	memcpy(cmd_buff,"AT+MIPLREADRSP=0,",strlen("AT+MIPLREADRSP=0,"));
	nbiot_itoa(uri.msgid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(1,tmp,1);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(uri.objid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(uri.instid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	nbiot_itoa(uri.resid,tmp,10);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
  nbiot_itoa(type,tmp,1);
  strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	if(type==1)
	{
		len = strlen(data);
		nbiot_itoa(len,tmp,10);
	}
	else if(type==2)
	{
		len = strlen(data);
		nbiot_itoa(len,tmp,10);
	}
	else if(type==3)
	{
		len = 2;
		nbiot_itoa(len,tmp,1);
	}
	else if(type==4)
	{
		len = 4;
		nbiot_itoa(len,tmp,1);
	}
	else
	{
		len = 1;
		nbiot_itoa(len,tmp,1);
	}
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
	if(type==1||type==2)
	{
	strcat(cmd_buff,"\"");
	strcat(cmd_buff,data);
	strcat(cmd_buff,"\"");
	}
	else
	{
	strcat(cmd_buff,data);	
	}
	strcat(cmd_buff,",");
	nbiot_itoa(0,tmp,1);
	strcat(cmd_buff,tmp);
	strcat(cmd_buff,",");
  nbiot_itoa(0,tmp,1);
	strcat(cmd_buff,tmp);
  strcat(cmd_buff,"\r\n");
	printf("read rsp:");
  printf("%s\r\n",cmd_buff);
	SentData(cmd_buff,"OK",100);
}

void bc95_observe_rsp(int suc,const nbiot_uri_t uri)
{
   char tmp[10];
	 memset(cmd_buff,0,50);
   memcpy(cmd_buff,"AT+MIPLOBSERVERSP=0,",strlen("AT+MIPLOBSERVERSP=0,"));
	 nbiot_itoa(uri.msgid,tmp,10);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,",");
	 nbiot_itoa(suc,tmp,1);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,"\r\n");
	 printf("observe rsp:");
   printf("%s\r\n",cmd_buff);
	 SentData(cmd_buff,"OK",100);
}

void bc95_discover_rsp(const nbiot_uri_t *uri,size_t lenth,char *value)
{
   char tmp[10];
	 memset(cmd_buff,0,50);
   memcpy(cmd_buff,"AT+MIPLDISCOVERRSP=0,",strlen("AT+MIPLDISCOVERRSP=0,"));
	 nbiot_itoa(uri->msgid,tmp,10);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,",");
	 strcat(cmd_buff,"1");
	 strcat(cmd_buff,",");
	 nbiot_itoa(lenth,tmp,10);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,",");
	 strcat(cmd_buff,"\"");
	 strcat(cmd_buff,value);
	 strcat(cmd_buff,"\"");
	 strcat(cmd_buff,"\r\n");
	 printf("discover rsp:");
   printf("%s\r\n",cmd_buff);
	 SentData(cmd_buff,"OK",100);
}
void bc95_write_rsp(int suc,uint16_t ackid)
{
   char tmp[10];
	 memset(cmd_buff,0,50);
   memcpy(cmd_buff,"AT+MIPLWRITERSP=0,",strlen("AT+MIPLWRITERSP=0,"));
	 nbiot_itoa(ackid,tmp,10);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,",");
	 nbiot_itoa(suc,tmp,1);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,"\r\n");
	 printf("write rsp:");
   printf("%s\r\n",cmd_buff);
	 SentData(cmd_buff,"OK",100);
  
}

void bc95_execute_rsp(int suc,uint16_t ackid)
{
   char tmp[10];
	 memset(cmd_buff,0,50);
	 memcpy(cmd_buff,"AT+MIPLEXECUTERSP=0,",strlen("AT+MIPLEXECUTERSP=0,"));
	 nbiot_itoa(ackid,tmp,10);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,",");
	 nbiot_itoa(suc,tmp,1);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,"\r\n");
	 printf("execute rsp:");
   printf("%s\r\n",cmd_buff);
   SentData(cmd_buff,"OK",100);
}

 
