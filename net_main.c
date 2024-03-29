/******************************************************************************/
/* 头文件包含*/
#include <stdio.h>
#include <string.h>
#include "CH57x_common.h"
#include "core_cm0.h"
#include "CH57xNET.H"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "MQTTPacket.H"
#include "stdbool.h"
#include "net_protocol.h"

#define KEEPLIVE_ENABLE                      1                                  /* 开启KEEPLIVE功能 */

/* 下面的缓冲区和全局变量必须要定义，库中调用 */
__align(16)UINT8    CH57xMACRxDesBuf[(RX_QUEUE_ENTRIES )*16];                   /* MAC接收描述符缓冲区，16字节对齐 */
__align(4) UINT8    CH57xMACRxBuf[RX_QUEUE_ENTRIES*RX_BUF_SIZE];                /* MAC接收缓冲区，4字节对齐 */
__align(4) SOCK_INF SocketInf[CH57xNET_MAX_SOCKET_NUM];                         /* Socket信息表，4字节对齐 */

UINT16 MemNum[8] = {CH57xNET_NUM_IPRAW,
                    CH57xNET_NUM_UDP,
                    CH57xNET_NUM_TCP,
                    CH57xNET_NUM_TCP_LISTEN,
                    CH57xNET_NUM_TCP_SEG,
                    CH57xNET_NUM_IP_REASSDATA,
                    CH57xNET_NUM_PBUF,
                    CH57xNET_NUM_POOL_BUF
                    };
UINT16 MemSize[8] = {CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_IPRAW_PCB),
                    CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_UDP_PCB),
                    CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_TCP_PCB),
                    CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_TCP_PCB_LISTEN),
                    CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_TCP_SEG),
                    CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_IP_REASSDATA),
                    CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_PBUF) + CH57xNET_MEM_ALIGN_SIZE(0),
                    CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_PBUF) + CH57xNET_MEM_ALIGN_SIZE(CH57xNET_SIZE_POOL_BUF)
                    };
__align(4)UINT8 Memp_Memory[CH57xNET_MEMP_SIZE];
__align(4)UINT8 Mem_Heap_Memory[CH57xNET_RAM_HEAP_SIZE];
__align(4)UINT8 Mem_ArpTable[CH57xNET_RAM_ARP_TABLE_SIZE];
/******************************************************************************/
/* 本演示程序的相关宏 */
#define RECE_BUF_LEN                          536                               /* 接收缓冲区的大小 */
/* CH57xNET库TCP的MSS长度为536字节，即一个TCP包里的数据部分最长为536字节 */
/* TCP协议栈采用滑动窗口进行流控，窗口最大值为socket的接收缓冲区长度。在设定 */
/* RX_QUEUE_ENTRIES时要考虑MSS和窗口之间的关系，例如窗口值为4*MSS，则远端一次会发送 */
/* 4个TCP包，如果RX_QUEUE_ENTRIES小于4，则必然会导致数据包丢失，从而导致通讯效率降低 */
/* 建议RX_QUEUE_ENTRIES要大于( 窗口/MSS ),如果多个socket同时进行大批量发送数据，则 */ 
/* 建议RX_QUEUE_ENTRIES要大于(( 窗口/MSS )*socket个数) 在多个socket同时进行大批数据收发时 */
/* 为了节约RAM，请将接收缓冲区的长度设置为MSS */
																		
/* CH579相关定义 */
UINT8 MACAddr[6] = {0x84,0xc2,0xe4,0x02,0x03,0x04};                             /* CH579MAC地址 */
static UINT8 IPAddr[4] = {192,168,1,200};                                                          /* CH579IP地址 */
static UINT8 GWIPAddr[4] = {192,168,1,1};                                              /* CH579网关 */
static UINT8 IPMask[4] = {255,255,255,0};                                              /* CH579子网掩码 */


static UINT8 DESIP[4] = {162,14,81,127};                                               /* 目的IP地址 */
static UINT16 aport=1000;											                   /* CH579源端口 */

/* 网口灯定义 PB口低十六位有效 */
UINT16 CH57xNET_LEDCONN=0x0010;                                                 /* 连接指示灯 PB4 */
UINT16 CH57xNET_LEDDATA=0x0080;                                                 /* 通讯指示灯 PB7 */ 

static char *username  = "";							                               /* 设备名，每个设备唯一，可用”/“做分级 */
static char *password  = "";								                           /* 服务器登陆密码 */
//static char *sub_topic = "+/#";								                           /* 订阅的会话名，为了自发自收，应与发布的会话名相同 */
//static char *pub_topic = "";									                       /* 发布的会话*/
static size_t mqtt_keepalive=20;																			/*MQTT 的Keepalaive*/
static char *mqtt_clientid="CH579";																/*MQTT的clientid*/


static UINT8 SocketId;                                                                /* 保存socket索引，可以不用定义 */
static UINT8 SocketRecvBuf[RECE_BUF_LEN];                                             /* socket接收缓冲区 */
static UINT8 MyBuf[RECE_BUF_LEN];                                                     /* 定义一个临时缓冲区 */

static UINT8 con_flag=0;										                       /* 已连接MQTT服务器标志位 */
//static UINT8 pub_flag=1;											                   /* 已发布会话消息标志位 */
static UINT8 sub_flag=0;											                   /* 已订阅会话标志位 */
//static UINT8 tout_flag=0;											                   /* 超时标志位 */
static UINT16 packetid=0;											                   /* 包ID */






//extern const UINT16 *memp_num;

/*******************************************************************************
* Function Name  : IRQ_Handler
* Description    : IRQ中断服务函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ETH_IRQHandler( void )						                             	/* 以太网中断 */
{
	CH57xNET_ETHIsr();								                            /* 以太网中断中断服务函数 */
}

//void TMR0_IRQHandler( void ) 						                            /* 定时器中断 */
//{
//	CH57xNET_TimeIsr(CH57xNETTIMEPERIOD);                                       /* 定时器中断服务函数 */
//	R8_TMR0_INT_FLAG |= 0xff;						                            /* 清除定时器中断标志 */
//}



/*******************************************************************************
* Function Name  : mStopIfError
* Description    : 调试使用，显示错误代码
* Input          : iError 错误代码
* Output         : None
* Return         : None
*******************************************************************************/
void mStopIfError(UINT8 iError)
{
    if (iError == CH57xNET_ERR_SUCCESS) return;                                 /* 操作成功 */
    PRINT("mStopIfError: %02X\r\n", (UINT16)iError);                            /* 显示错误 */
}


	/*******************************************************************************
* Function Name	: Transport_Open
* Description	: 创建TCP连接
* Input			: None
* Output		: None
* Return		: None
*******************************************************************************/
UINT8 Transport_Open()
{
	UINT8 i;																														 
	SOCK_INF TmpSocketInf;									                            /* 创建临时socket变量 */

	memset((void *)&TmpSocketInf,0,sizeof(SOCK_INF));		                            /* 库内部会将此变量复制，所以最好将临时变量先全部清零 */
	memcpy((void *)TmpSocketInf.IPAddr,DESIP,4);		                             	/* 设置目的IP地址 */
	TmpSocketInf.DesPort = 1883;							                            /* 设置目的端口 */
	TmpSocketInf.SourPort = aport++;					                              	/* 设置源端口 */
	TmpSocketInf.ProtoType = PROTO_TYPE_TCP;			                             	/* 设置socekt类型 */
	TmpSocketInf.RecvStartPoint = (UINT32)SocketRecvBuf;                               	/* 设置接收缓冲区的接收缓冲区 */
	TmpSocketInf.RecvBufLen = RECE_BUF_LEN ;				                            /* 设置接收缓冲区的接收长度 */
	i = CH57xNET_SocketCreat(&SocketId,&TmpSocketInf);	                             	/* 创建socket，将返回的socket索引保存在SocketId中 */
	mStopIfError(i);									                               	/* 检查错误 */
	
	i = CH57xNET_SocketConnect(SocketId);					                            /* TCP连接 */
	mStopIfError(i);									                               	/* 检查错误 */
	return SocketId;
}


/*******************************************************************************
* Function Name	: Transport_Close
* Description	: 关闭TCP连接
* Input			: None
* Output		: None
* Return		: None
*******************************************************************************/
UINT8 Transport_Close()
{
	UINT8 i;
	i=CH57xNET_SocketClose(SocketId,TCP_CLOSE_NORMAL);
	mStopIfError(i);
	return i;
}


/*******************************************************************************
* Function Name	: Transport_SendPacket
* Description	: 以太网发送数据
* Input			: UINT8 *buf 发送数据的首字节地址
				  UINT32 len 发送数据的长度
* Output		: None
* Return		: None
*******************************************************************************/
void Transport_SendPacket(UINT8 *buf,UINT32 len)
{
	UINT32 totallen;
	UINT8 *p=buf;
	
	totallen=len;
	while(1)
	{
		len = totallen;
		CH57xNET_SocketSend(SocketId,p,&len);				                         	/* 将MyBuf中的数据发送 */
		totallen -= len;					 				                          	/* 将总长度减去以及发送完毕的长度 */
		p += len;											                         	/* 将缓冲区指针偏移*/
		if(totallen)continue;								                         	/* 如果数据未发送完毕，则继续发送*/
		break;												                         	/* 发送完毕，退出 */
	}
}


/*******************************************************************************
* Function Name	: MQTT_Connect
* Description	: 创建MQTT连接
* Input			: char *username 设备名
				  char *password 服务器连接密码
* Output		: None
* Return		: None
*******************************************************************************/
void MQTT_Connect(char *username,char *password)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	UINT32 len;
	UINT8 buf[200];

	data.clientID.cstring = mqtt_clientid;
	data.keepAliveInterval = mqtt_keepalive;
	data.cleansession = 1;
	data.username.cstring = username;																		
	data.password.cstring = password;
	
	Net_Protocol_On_Connect(&data);
	 
	len=MQTTSerialize_connect(buf,sizeof(buf),&data);											
	Transport_SendPacket(buf,len);						
}


/*******************************************************************************
* Function Name	: MQTT_Subscribe
* Description	: MQTT订阅一个主题
* Input			: char *topic 订阅的主题名
* Output		: None
* Return		: None
*******************************************************************************/
void MQTT_Subscribe( char *topic)
{
	MQTTString topicString = MQTTString_initializer;
	int req_qos=0;
	UINT32 len;
	UINT32 msgid=1;
	UINT8 buf[200];
	
	topicString.cstring=topic;
	len=MQTTSerialize_subscribe(buf,sizeof(buf),0,msgid,1,&topicString,&req_qos);
	Transport_SendPacket(buf,len);
}


/*******************************************************************************
* Function Name	: MQTT_Unsubscribe
* Description	: MQTT取消订阅一个主题
* Input			: char *topic 取消订阅的主题名
* Output		: None
* Return		: None
*******************************************************************************/
void MQTT_Unsubscribe(char *topic)
{
	MQTTString topicString = MQTTString_initializer;
	UINT32 len;
	UINT32 msgid=1;
	UINT8 buf[200];
	
	topicString.cstring=topic;
	len=MQTTSerialize_unsubscribe(buf,sizeof(buf),0,msgid,1,&topicString);
	Transport_SendPacket(buf,len);
}


/*******************************************************************************
* Function Name	: MQTT_Subscribe
* Description	: MQTT订阅一个主题
* Input			: char *topic 订阅的主题名
* Output		: None
* Return		: None
*******************************************************************************/
void MQTT_Publish(char *topic,char *payload,size_t payloadlen)
{
	MQTTString topicString = MQTTString_initializer;
	UINT32 len;
	UINT8 buf[1024];	
	topicString.cstring=topic; 
	len= MQTTSerialize_publish(buf,sizeof(buf),0,0,0,packetid++,topicString,(uint8_t *)payload,payloadlen);	
	Transport_SendPacket(buf,len);
}


/*******************************************************************************
* Function Name	: MQTT_Pingreq
* Description	: MQTT发送心跳包
* Input			: None
* Output		: None
* Return		: None
*******************************************************************************/
void MQTT_Pingreq()
{
	UINT32 len;
	UINT8 buf[200];
	
	len=MQTTSerialize_pingreq(buf,sizeof(buf));
	Transport_SendPacket(buf,len);
}



/*******************************************************************************
* Function Name	: MQTT_Disconnect
* Description	: 断开MQTT连接
* Input			: None
* Output		: None
* Return		: None
*******************************************************************************/
void MQTT_Disconnect()
{
	UINT32 len;
	UINT8 buf[50];
	len=MQTTSerialize_disconnect(buf,sizeof(buf));
	Transport_SendPacket(buf,len);
}



/*******************************************************************************
* Function Name  : CH57xNET_CreatTcpSocket
* Description    : 创建TCP Client socket
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_CreatTcpSocket(void)
{
   UINT8 i;                                                             
   SOCK_INF TmpSocketInf;                                                       /* 创建临时socket变量 */

   memset((void *)&TmpSocketInf,0,sizeof(SOCK_INF));                            /* 库内部会将此变量复制，所以最好将临时变量先全部清零 */
   memcpy((void *)TmpSocketInf.IPAddr,DESIP,4);                                 /* 设置目的IP地址 */
   TmpSocketInf.DesPort = 1000;                     
   TmpSocketInf.SourPort = 2000;                                                /* 设置源端口 */
   TmpSocketInf.ProtoType = PROTO_TYPE_TCP;                                     /* 设置socekt类型 */
   TmpSocketInf.RecvStartPoint = (UINT32)SocketRecvBuf;                         /* 设置接收缓冲区的接收缓冲区 */
   TmpSocketInf.RecvBufLen = RECE_BUF_LEN ;                                     /* 设置接收缓冲区的接收长度 */
   i = CH57xNET_SocketCreat(&SocketId,&TmpSocketInf);                           /* 创建socket，将返回的socket索引保存在SocketId中 */
   mStopIfError(i);                                                             /* 检查错误 */
#ifdef  KEEPLIVE_ENABLE
   CH57xNET_SocketSetKeepLive( SocketId, 1 );                                   /* 开启socket的KEEPLIVE功能（V06版本支持） */
#endif

   i = CH57xNET_SocketConnect(SocketId);                                        /* TCP连接 */
   mStopIfError(i);                                                             /* 检查错误 */
   i = CH57xNET_SetSocketTTL( SocketId,10 );
   mStopIfError(i);                                                             /* 检查错误 */
}


/*******************************************************************************
* Function Name  : net_initkeeplive
* Description    : keeplive初始化
* Input          : None      
* Output         : None
* Return         : None
*******************************************************************************/
#ifdef  KEEPLIVE_ENABLE
void net_initkeeplive(void)
{
    struct _KEEP_CFG  klcfg;

    klcfg.KLIdle = 20000;                                                       /* 空闲 */
    klcfg.KLIntvl = 10000;                                                      /* 间隔 */
    klcfg.KLCount = 5;                                                          /* 次数 */
    CH57xNET_ConfigKeepLive(&klcfg);
}
#endif

/*******************************************************************************
* Function Name  : CH57xNET_LibInit
* Description    : 库初始化操作
* Input          : ip      ip地址指针
*                ：gwip    网关ip地址指针
*                : mask    掩码指针
*                : macaddr MAC地址指针 
* Output         : None
* Return         : 执行状态
*******************************************************************************/
UINT8 CH57xNET_LibInit(/*const*/ UINT8 *ip,/*const*/ UINT8 *gwip,/*const*/ UINT8 *mask,/*const*/ UINT8 *macaddr)
{
    UINT8 i;
    struct _CH57x_CFG cfg;

    if(CH57xNET_GetVer() != CH57xNET_LIB_VER)return 0xfc;                       /* 获取库的版本号，检查是否和头文件一致 */
    CH57xNETConfig = LIB_CFG_VALUE;                                             /* 将配置信息传递给库的配置变量 */
    cfg.RxBufSize = RX_BUF_SIZE; 
    cfg.TCPMss   = CH57xNET_TCP_MSS;
    cfg.HeapSize = CH57x_MEM_HEAP_SIZE;
    cfg.ARPTableNum = CH57xNET_NUM_ARP_TABLE;
    cfg.MiscConfig0 = CH57xNET_MISC_CONFIG0;
    CH57xNET_ConfigLIB(&cfg);
    i = CH57xNET_Init(ip,gwip,mask,macaddr);
#ifdef  KEEPLIVE_ENABLE
    net_initkeeplive( );
#endif
    return (i);            
}

/*******************************************************************************
* Function Name  : CH57xNET_HandleSockInt
* Description    : Socket中断处理函数
* Input          : sockeid  socket索引
*                ：initstat 中断状态
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_HandleSockInt(UINT8 sockeid,UINT8 initstat)
{
    UINT32 len;
    //UINT8 i;
    unsigned char dup;
	unsigned short packetid;

	int qos;
	unsigned char retained;
	MQTTString topicName={0};
	unsigned char* payload;
	int payloadlen;
	//unsigned char *p=payload;

    if(initstat & SINT_STAT_RECV)                                                   /* 接收中断 */
    {
    len = CH57xNET_SocketRecvLen(sockeid,NULL);		                         	/* 查询长度 */
		CH57xNET_SocketRecv(sockeid,MyBuf,&len);		                          	/* 将接收缓冲区的数据读到MyBuf中*/
		switch(MyBuf[0]>>4)
		{
			case FLAG_CONNACK:
				PRINT("connack\r\n");
				con_flag=1;
        //MQTT_Subscribe(sub_topic);
			  Net_Protocol_On_Connected();
				break;

			case FLAG_PUBLISH:
			{
				MQTTDeserialize_publish(&dup,&qos,&retained,&packetid,&topicName,&payload,&payloadlen,MyBuf,len);
				/*
			  PRINT("qos=%d,retained=%d,packetid=%d,payloadlen=%d\r\n",qos,retained,packetid,(UINT16)payloadlen);
				
				p=(uint8_t *)topicName.lenstring.data;
				printf("topic=");
				for(i=0;i<topicName.lenstring.len;i++)
				{
					PRINT("%c",(UINT16)*p);
					p++;
				}
				PRINT("\r\n");
				
				printf("payload:");
				p=payload;
				for(i=0;i<payloadlen;i++)
				{
					PRINT("%c",(UINT16)*p);
					p++;
				}
				PRINT("\r\n");
				*/
				Net_Protocol_On_Message(topicName.lenstring.data,topicName.lenstring.len,payload,payloadlen,qos,retained);
			}
			break;
				
			case FLAG_SUBACK:
			{
				if(sub_flag!=1)
				{
					sub_flag=1;	
					Net_Protocol_On_Subscribe_Success();
				}
				PRINT("suback\r\n");
			  
			}
			break;

			default:

				break;
		}
    }
    if(initstat & SINT_STAT_CONNECT)                                            /* TCP连接中断 */
    {                                                                           /* 产生此中断表示TCP已经连接，可以进行收发数据 */
        PRINT("TCP Connect Success\n");   
        MQTT_Connect(username, password);        
    }
    if(initstat & SINT_STAT_DISCONNECT)                                         /* TCP断开中断 */
    {                                                                           /* 产生此中断，CH579库内部会将此socket清除，置为关闭*/
        PRINT("TCP Disconnect\n");                                              /* 应用层可以重新创建连接 */
        Transport_Close();
				Transport_Open();
				con_flag=0;
				sub_flag=0;
    }
    if(initstat & SINT_STAT_TIM_OUT)                                            /* TCP超时中断 */
    {                                                                           /* 产生此中断，CH579库内部会将此socket清除，置为关闭*/
        PRINT("TCP Timout\n");                                                  /* 应用层需可以重新创建连接 */
        Transport_Close();
				Transport_Open();
				con_flag=0;
			  sub_flag=0;
    }
}


/*******************************************************************************
* Function Name  : CH57xNET_HandleGloableInt
* Description    : 全局中断处理函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH57xNET_HandleGlobalInt(void)
{
    UINT8 initstat;
    UINT8 i;
    UINT8 socketinit;
    initstat = CH57xNET_GetGlobalInt();                                         /* 读全局中断状态并清除 */
    if(initstat & GINT_STAT_UNREACH)                                            /* 不可达中断 */
    {
        PRINT("UnreachCode ：%d\n",CH57xInf.UnreachCode);                       /* 查看不可达代码 */
        PRINT("UnreachProto ：%d\n",CH57xInf.UnreachProto);                     /* 查看不可达协议类型 */
        PRINT("UnreachPort ：%d\n",CH57xInf.UnreachPort);                       /* 查询不可达端口 */      
    }
   if(initstat & GINT_STAT_IP_CONFLI)                                           /* IP冲突中断 */
   {
   
   }
   if(initstat & GINT_STAT_PHY_CHANGE)                                          /* PHY改变中断 */
   {
       i = CH57xNET_GetPHYStatus();                                             /* 获取PHY状态 */
       PRINT("GINT_STAT_PHY_CHANGE %02x\n",i);  
   }
   if(initstat & GINT_STAT_SOCKET)                                              /* Socket中断 */
   {
       for(i = 0; i < CH57xNET_MAX_SOCKET_NUM; i ++)                     
       {
           socketinit = CH57xNET_GetSocketInt(i);                               /* 读socket中断并清零 */
           if(socketinit)CH57xNET_HandleSockInt(i,socketinit);                  /* 如果有中断则清零 */
       }    
   }
}

/*******************************************************************************
* Function Name  : Timer0Init
* Description    : 定时器1初始化
* Input          : time 定时时间
* Output         : None
* Return         : None
*******************************************************************************/
//void Timer0Init(UINT32 time)
//{
//	R8_TMR0_CTRL_MOD = RB_TMR_ALL_CLEAR;	                                    /* 清除所有计数值 */
//	R8_TMR0_CTRL_MOD = 0;					                                	/* 设置定时器模式 */
//	R32_TMR0_CNT_END = FREQ_SYS/1000000*time;	                                /* 设置定时时间 */
//	R8_TMR0_INT_FLAG = R8_TMR0_INT_FLAG;		                                /* 清除标志 */
//	R8_TMR0_INTER_EN = RB_TMR_IE_CYC_END;	                                 	/* 定时中断 */
//	R8_TMR0_CTRL_MOD |= RB_TMR_COUNT_EN;
//	NVIC_EnableIRQ(TMR0_IRQn);	
//}


/*******************************************************************************
* Function Name  : CH57xNET_DHCPCallBack
* Description    : DHCP回调函数
* Input          : None
* Output         : None
* Return         : 执行状态
*******************************************************************************/
UINT8 CH57xNET_DHCPCallBack(UINT8 status,void *arg)
{
   UINT8 *p;
   
   if(!status){                                                                  /* 成功*/
       p = arg;                                                                  /* 产生此中断，CH57xNET库内部会将此socket清除，置为关闭*/
       PRINT("DHCP Success\n");
       memcpy(IPAddr,p,4);
       memcpy(GWIPAddr,&p[4],4);
       memcpy(IPMask,&p[8],4);                                                   /* 产生此中断，CH57xNET库内部会将此socket清除，置为关闭*/
       PRINT("IPAddr = %d.%d.%d.%d \n",(UINT16)IPAddr[0],(UINT16)IPAddr[1],
              (UINT16)IPAddr[2],(UINT16)IPAddr[3]);
       PRINT("GWIPAddr = %d.%d.%d.%d \n",(UINT16)GWIPAddr[0],(UINT16)GWIPAddr[1],
              (UINT16)GWIPAddr[2],(UINT16)GWIPAddr[3]);
       PRINT("IPAddr = %d.%d.%d.%d \n",(UINT16)IPMask[0],(UINT16)IPMask[1],
              (UINT16)IPMask[2],(UINT16)IPMask[3]);
       PRINT("DNS1: %d.%d.%d.%d\n",p[12],p[13],p[14],p[15]);
       PRINT("DNS2: %d.%d.%d.%d\n",p[16],p[17],p[18],p[19]);
//     CH57xNET_CreatTcpSocket();                                                /* 可以在此处，在DHCP完成后进行TCP连接 */
//	   PRINT("creat tcp sockrt!\r\n"); 
   }
   else{ 
                                                                                 /* 产生此中断，CH57xNET库内部会将此socket清除，置为关闭*/
      PRINT("DHCP Fail %02x\n",status);
   }                                                          
   return 0;
}

/*******************************************************************************
* Function Name  : GetMacAddr
* Description    : 系统获取MAC地址
* Input          : pMAC:指向用来存储Mac地址的缓冲
* Output         : None
* Return         : None
*******************************************************************************/
void GetMacAddr(UINT8 *pMAC)
{
	UINT8 transbuf[6],i;
	
	GetMACAddress(transbuf);
	for(i=0;i<6;i++)
	{
		pMAC[5-i]=transbuf[i];
	
	}
}

/*******************************************************************************
* Function Name	: MQTT_CheckKeepalive
* Description	: MQTT 检查keepalive
* Input			: None
* Output		: None
* Return		: None
*******************************************************************************/

static TickType_t last_ping_tick=0;

void MQTT_CheckKeepalive()
{
	TickType_t current_tick=xTaskGetTickCount();
	if(mqtt_keepalive!=0 && con_flag!=0)
	{
		if(current_tick-last_ping_tick>=pdMS_TO_TICKS(1000*mqtt_keepalive))
		{
			last_ping_tick=current_tick;
			MQTT_Pingreq();//仅发送ping
			
		}
	}		
}

/*******************************************************************************
* Function Name  : net_main
* Description    : 网络主任务及定时器回调
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Net_Timer( TimerHandle_t xTimer )
{
	vTaskSuspendAll();
	CH57xNET_TimeIsr(CH57xNETTIMEPERIOD);                                       /* 定时器中断服务函数 */
	xTaskResumeAll();
}

void  net_main_task(void *arg) 
{
  UINT8 i = 0;
	GetMacAddr(MACAddr);                                                        /* 获取MAC地址 */
  i = CH57xNET_LibInit(IPAddr,GWIPAddr,IPMask,MACAddr);                       /* 库初始化 */
  mStopIfError(i);                                                            /* 检查错误 */
  PRINT("CH57xNETLibInit Success\r\n");  
//	Timer0Init( 10000 );		                                                /* 初始化定时器:10ms */
	{
		xTimerStart(xTimerCreate("Net",pdMS_TO_TICKS(CH57xNETTIMEPERIOD),pdTRUE,NULL,Net_Timer),0);	//初始化定时器
	}
	NVIC_EnableIRQ(ETH_IRQn);	
	
	Transport_Open();
	
	while ( CH57xInf.PHYStat < 2 )
	{
		vTaskDelay(pdMS_TO_TICKS(50));
	}
	if(!i)
	{
		CH57xNET_DHCPStart(CH57xNET_DHCPCallBack);                            /* 启动DHCP */
  }		
  PRINT("CH579 dhcp client create！\r\n");
  
	Net_Protocol_Init();
	
  while(1)
  {
			vTaskSuspendAll();
      CH57xNET_MainTask();		/* CH57xNET库主任务函数，需要在主循环中不断调用 */
      if(CH57xNET_QueryGlobalInt())CH57xNET_HandleGlobalInt();                /* 查询中断，如果有中断，则调用全局中断处理函数 */
			{
				MQTT_CheckKeepalive();			
			}
			xTaskResumeAll();
			taskYIELD();//任务切换
  }
}
/*********************************** 其它函数 **********************************/


bool MQTT_Is_Connected(void)
{
	return con_flag!=0 && sub_flag!=0;
}


/*********************************** endfile **********************************/
