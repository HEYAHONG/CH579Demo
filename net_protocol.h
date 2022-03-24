#ifndef __NET_PROTOCOL_H__
#define __NET_PROTOCOL_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "CH57x_common.h"
#include "libSMGS.h"
#include "MQTTPacket.H"	

/*
	注意:网络协议栈的所有函数均需要在网络协议栈所在任务下调用。
*/
	
/*
	MQTT是否连接
*/
bool MQTT_Is_Connected(void);

/*
	MQTT订阅
*/
void MQTT_Subscribe( char *topic);	
/*
	MQTT发布
*/
void MQTT_Publish(char *topic,char *payload,size_t payloadlen);
	
/*
	协议栈初始化
*/
bool Net_Protocol_Init(void);
	
/*
	协议栈处理MQTT连接
*/
void Net_Protocol_On_Connect(MQTTPacket_connectData *cfg);

/*
	协议栈处理MQTT已连接
*/
void Net_Protocol_On_Connected(void);

/*
	协议栈处理MQTT已成功订阅
*/
void Net_Protocol_On_Subscribe_Success(void);

/*
	协议栈处理MQTT消息
*/
void Net_Protocol_On_Message(const char *topic,size_t topiclen,void *payload,size_t payloadlen,uint8_t qos,uint8_t retain);

#ifdef __cplusplus
}
#endif

#endif
