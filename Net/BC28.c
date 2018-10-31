#ifdef BC28

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "ringbuf.h"
#include "utils.h"
#include "fifo.h"
#include "BC28.h"



signed char dl_buf_id=-1;
static char cmd_buff[1024];


sendmsg callback=NULL;
volatile char *flag_ok=NULL;
struct ringbuf *result_ptr=NULL;



void nbiot_sleep( int milliseconds)
{
	osDelay(milliseconds);
}

uint32_t nbiot_time(void)
{
	//(RTC->CNTL | RTC->CNTH << 16);
   return xTaskGetTickCount();
}


uint32_t ip_SendData(int8_t * buf, uint32_t len)
{
     SentData(buf,"OK",100);
     return len;
}

void netif_rx(uint8_t*buf,uint16_t *read)
{
     uint8_t *msg_p=NULL;
     uint8_t ptr[1024]={0};
     *read=fifo_get(dl_buf_id,ptr);
     if(*read!=0)
     {
        if((msg_p = strstr((const char *)ptr, "+MIPL"))!=NULL)
        {              
           memcpy(buf,ptr,*read);
        }
		else
		{
           *read=0;
        }
     }     
}

void netdev_init(void)
{
	SendCmd("AT+CFUN?\r\n", "OK", 2000,1,5);//模块射频功能是否打开，值为1
	SendCmd("AT+CIMI\r\n", "OK", 2000,3,5);	//查询IMSI
	SendCmd("AT+CMEE=1\r\n","OK", 2000,0,10);	// Report UE Error
    SendCmd("AT+CSCON=1\r\n","OK", 2000,0,10);  //查询当前网络连接状态
    SendCmd("AT+CEREG=2\r\n","OK", 2000,0,10);	//网络注册1=registered, 2=searching 
    SendCmd("AT+CGATT=1\r\n","OK", 2000,0,10);	//启动网络附着
    SendCmd("AT+CEREG?\r\n","CEREG:2,1", 3000,0,5);	//返回0为入网失败，返回1为入网成功
    SendCmd("AT+CEDRXS=0,5\r\n","OK", 3000,0,5); //关闭eDRX
	SendCmd("AT+CPSMS=0\r\n","OK", 3000,0,5); 	//关闭PSM
	SendCmd("AT+CESQ\r\n", "OK", 2000,0,5); 	//查看信号强度
    printf("BC26:connect sucess\r\n");
} 

void nb_addobj(uint16_t	objid,uint16_t	attrcount,uint16_t	actcount)
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

void nb_delobj(uint16_t objid)
{
	 char tmp[10];
	 memset(cmd_buff,0,50);
	 memcpy(cmd_buff,"AT+MIPLDELOBJ=0,",strlen("AT+MIPLDELOBJ=0,"));
	 nbiot_itoa(objid,tmp,10);
	 strcat(cmd_buff,tmp);
	 strcat(cmd_buff,"\r\n");
	 SendCmd(cmd_buff,"OK",300,0,5);
 }


size_t nb_register_request( uint8_t *buffer,	size_t buffer_len)
{   
    size_t  len=0;
	char status=0;
	len=strlen("AT+MIPLOPEN=0,2000\r\n")+1;
	if(len<buffer_len)
	{
		memcpy(buffer,"AT+MIPLOPEN=0,2000\r\n",len);
		status=SendCmd("AT+MIPLOPEN=0,2000\r\n","OK",1000,0,5);
		if(status==2) SendCmd("AT+MIPLCLOSE=0\r\n","OK",300,0,5); 
		return len;
	}
	return 0;
}

size_t nb_register_update (uint16_t lifttime, uint8_t *buffer,size_t  buffer_len)
{   
    size_t  len=0;
	char ative[6]={0};
	nbiot_itoa(lifttime,ative,6);
	memcpy(buffer,"AT+MIPLUPDATE=0,",sizeof("AT+MIPLUPDATE=0,"));
	strcat(buffer,ative);
	strcat(buffer,",0\r\n");
	len=strlen(buffer)+1;
	if(len<buffer_len)
	{
		SendCmd(buffer,"OK",300,0,5);
		return len;
	}
	return 0;
}

 size_t nb_close_request( uint8_t  *buffer,size_t  buffer_len)
{   
    size_t  len=0;
	len=strlen("AT+MIPLCLOSE=0\r\n")+1;
	if(len<buffer_len)
	{
		 memcpy(buffer,"AT+MIPLCLOSE=0\r\n",len);
		 SendCmd("AT+MIPLCLOSE=0\r\n","OK",300,0,5);
		 return len;
	}
	return 0;
}  


 void nb_notify_upload(const nbiot_uri_t uri,uint8_t type,char *data)
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

 void nb_read_upload(const nbiot_uri_t uri,uint8_t type,char *data)
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

void nb_observe_rsp(int suc,const nbiot_uri_t uri)
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

void nb_discover_rsp(const nbiot_uri_t *uri,size_t lenth,char *value)
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
void nb_write_rsp(int suc,uint16_t ackid)
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

void nb_execute_rsp(int suc,uint16_t ackid)
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

 


void register_cmd_handler(sendmsg func,void *result_buf, volatile char *flag)
{
	callback=func;
	result_ptr=(struct ringbuf *)result_buf;
	flag_ok=flag;
}

char SendCmd(char* cmd, uint8_t *result,uint16_t timeout,uint8_t retry,uint16_t waittime)
{	
	char *msg_p=NULL;
	uint8_t retry_num=0,retryflag=0;
	uint32_t  nowtime=0,newtime=0,sum=0;
	
	if(callback==NULL||result_ptr==NULL||flag_ok==NULL) return 2 ;
	*flag_ok=0;
	ringbuf_clear(result_ptr);//清除之前可能残留的信息
	printf("cmd:%s\r\n",cmd);
	callback((uint8_t*)cmd, strlen((const char *)cmd));
	nowtime= nbiot_time();
	while(1)
	{
		if(sum>waittime)
		{
			if(++retry_num>retry) return 0; 
			retryflag=1;
		}
		if(*flag_ok==1)
		{
			printf("cmd_rsp:%s",result_ptr->data);
			*flag_ok=0;
			msg_p=strstr(result_ptr->data,result);
			if(msg_p!=NULL)
			{
				ringbuf_clear(result_ptr);
				break;
			}
			else
			{		   
				msg_p=strstr(result_ptr->data,"ERROR");
				ringbuf_clear(result_ptr);	
				if(msg_p!=NULL) return 2; 
				/*	 
				if(retry==1){
				   osDelay(3000);
				printf("retry cmd:%s",cmd);
				callback((uint8_t*)cmd, strlen((const char *)cmd));							   
				}	
				*/						 
			}
		}
		else
		{
			newtime=nbiot_time();
			sum=newtime-nowtime;					
		}
		if(retryflag==1){
			retryflag=0;
			sum=0;
			nowtime=nbiot_time();
			if(retry_num>0&&retry_num<retry+1)
			{		
				printf("retry cmd:%s",cmd);
				callback((uint8_t*)cmd, strlen((const char *)cmd));	
			}
		}	
	}
	osDelay(timeout);
	return 1;
}


void SentData(char* cmd, uint8_t *result,uint16_t timeout)
{
  	char *msg_p=NULL;
	uint32_t  nowtime=0,newtime=0,sum=0;
	if(callback==NULL||result_ptr==NULL||flag_ok==NULL) return;
	*flag_ok=0;
	ringbuf_clear(result_ptr);	//清除之前可能残留的信息
	printf("Data:%s\r\n",cmd);
	callback((uint8_t*)cmd, strlen((const char *)cmd));
	nowtime= nbiot_time();  
	while(1)
	{
		if(sum>2) break;
		if(*flag_ok==1)
		{			 
			*flag_ok=0;
			printf("data_rsp:%s\r\n",result_ptr->data);
				/*
         msg_p=strstr(result_ptr->data,result);
         if(msg_p!=NULL){
			   	ringbuf_clear(result_ptr);
          break;
				 }*/
			break;
		}
		else{		
			newtime=nbiot_time();
			sum=newtime-nowtime;
		} 
	}
	osDelay(timeout);
}

#endif
