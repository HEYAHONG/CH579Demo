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
	ע��:����Э��ջ�����к�������Ҫ������Э��ջ���������µ��á�
*/
	
/*
	MQTT�Ƿ�����
*/
bool MQTT_Is_Connected(void);

/*
	MQTT����
*/
void MQTT_Subscribe( char *topic);	
/*
	MQTT����
*/
void MQTT_Publish(char *topic,char *payload,size_t payloadlen);
	
/*
	Э��ջ��ʼ��
*/
bool Net_Protocol_Init(void);
	
/*
	Э��ջ����MQTT����
*/
void Net_Protocol_On_Connect(MQTTPacket_connectData *cfg);

/*
	Э��ջ����MQTT������
*/
void Net_Protocol_On_Connected(void);

/*
	Э��ջ����MQTT�ѳɹ�����
*/
void Net_Protocol_On_Subscribe_Success(void);

/*
	Э��ջ����MQTT��Ϣ
*/
void Net_Protocol_On_Message(const char *topic,size_t topiclen,void *payload,size_t payloadlen,uint8_t qos,uint8_t retain);

#ifdef __cplusplus
}
#endif

#endif
