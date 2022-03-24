#include "libSMGS.h"
#include "net_protocol.h"
#include "CH57XNET.h"

static const char * TAG="CH579Demo";
static char GateWaySerialNumber[32]="C579";

/*
协议栈相关
*/

SMGS_device_context_t device_context;

bool  SMGS_Device_IsOnline(SMGS_device_context_t *ctx)
{
    //默认返回真
    return true;
}

bool SMGS_Device_Command(SMGS_device_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_cmdid_t *cmdid,uint8_t *cmddata,size_t cmddata_length,uint8_t *retbuff,size_t *retbuff_length,SMGS_payload_retcode_t *ret)
{
    bool _ret=false;
    PRINT("%s:Device_Command(CmdID=%04X)\r\n",TAG,(uint32_t)(*cmdid));
    return _ret;
}

bool SMGS_Device_ReadRegister(SMGS_device_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_register_address_t addr,uint64_t *dat,SMGS_payload_register_flag_t *flag)
{
    bool ret=false;
    PRINT("%s:Device_ReadRegister(Addr=%04X)\r\n",TAG,(uint32_t)addr);
    return ret;
}

bool SMGS_Device_WriteRegister(SMGS_device_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_register_address_t addr,uint64_t *dat,SMGS_payload_register_flag_t *flag)
{
    bool ret=false;
    PRINT("%s:Device_WriteRegister(Addr=%04X,Data=%016llX,Flag=%02X)\r\n",TAG,(uint32_t)addr,(*dat),(uint32_t)(flag->val));
    return ret;
}

bool SMGS_Device_ReadSensor(SMGS_device_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_sensor_address_t addr,uint64_t *dat,SMGS_payload_sensor_flag_t *flag)
{
    bool ret=false;
    PRINT("%s:Device_ReadSensor(Addr=%04X,Flag=%02X)\r\n",TAG,(uint32_t)addr,(uint32_t)(flag->val));
    return ret;
}



SMGS_gateway_context_t gateway_context;

bool SMGS_GateWay_Command(SMGS_gateway_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_cmdid_t *cmdid,uint8_t *cmddata,size_t cmddata_length,uint8_t *retbuff,size_t *retbuff_length,SMGS_payload_retcode_t *ret)
{
    bool _ret=false;
    PRINT("%s:GateWay_Command(CmdID=%04X)\r\n",TAG,(uint32_t)(*cmdid));
    return _ret;
}

bool SMGS_GateWay_ReadRegister(SMGS_gateway_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_register_address_t addr,uint64_t *dat,SMGS_payload_register_flag_t *flag)
{
    bool ret=false;
    PRINT("%s:GateWay_ReadRegister(Addr=%04X)\r\n",TAG,(uint32_t)addr);
    return ret;
}

bool SMGS_GateWay_WriteRegister(SMGS_gateway_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_register_address_t addr,uint64_t *dat,SMGS_payload_register_flag_t *flag)
{
    bool ret=false;
    PRINT("%s:GateWay_WriteRegister(Addr=%04X,Data=%016llX,Flag=%02X)\r\n",TAG,(uint32_t)addr,(*dat),(uint32_t)(flag->val));
    return ret;
}

bool SMGS_GateWay_ReadSensor(SMGS_gateway_context_t *ctx,SMGS_topic_string_ptr_t plies[],SMGS_payload_sensor_address_t addr,uint64_t *dat,SMGS_payload_sensor_flag_t *flag)
{
    bool ret=false;
    PRINT("%s:GateWay_ReadSensor(Addr=%04X,Flag=%02X)\r\n",TAG,(uint32_t)addr,(uint32_t)(flag->val));
    return ret;
}


//设备查询函数
SMGS_device_context_t * SMGS_Device_Next(struct __SMGS_gateway_context_t *ctx,SMGS_device_context_t * devctx)
{
    if(devctx==NULL)
    {
        return &device_context;//返回第一个设备
    }

    //由于只有一个设备，直接返回NULL

    return NULL;
}


static bool SMGS_MessagePublish(struct __SMGS_gateway_context_t *ctx,const char * topic,void * payload,size_t payloadlen,uint8_t qos,int retain)
{
	 if(!MQTT_Is_Connected())
	 {
		 return false;
	 }
	 
   MQTT_Publish((char *)topic,(char *)payload,payloadlen);
	 CH57xNET_MainTask();//处理网络任务
	
   return true;
}
static char subtopic[32]="";
bool Net_Protocol_Init()
{
	
	 {
		 uint8_t mac[6]={0};
		 GetMACAddress(mac);
		 for(size_t i=0;i<sizeof(mac);i++)
		 {
			 char buff[10]={0};
			 sprintf(buff,"%02X",mac[i]);
			 strcat(GateWaySerialNumber,buff);
		 }
		 
		 PRINT("GateWaySerialNumber Is %s\r\n",GateWaySerialNumber);
	 }
	 
	 {
		 strcat(subtopic,GateWaySerialNumber);
		 strcat(subtopic,"/#");
	 }
	
	 {
        //初始化设备上下文
        SMGS_Device_Context_Init(&device_context);

        //填写设备上下文
        device_context.DeviceName=TAG;
        device_context.DevicePosNumber=1;
        device_context.DeviceSerialNumber=GateWaySerialNumber;//默认序列号同网关
        device_context.IsOnline=SMGS_Device_IsOnline;
        device_context.Command=SMGS_Device_Command;
        device_context.ReadRegister=SMGS_Device_ReadRegister;
        device_context.WriteRegister=SMGS_Device_WriteRegister;
        device_context.ReadSensor=SMGS_Device_ReadSensor;

    }

    {

        //初始化网关上下文
        SMGS_GateWay_Context_Init(&gateway_context,GateWaySerialNumber,SMGS_MessagePublish);

        //填写网关上下文
        gateway_context.GateWayName=TAG;
        gateway_context.Command=SMGS_GateWay_Command;
        gateway_context.ReadRegister=SMGS_GateWay_ReadRegister;
        gateway_context.WriteRegister=SMGS_GateWay_WriteRegister;
        gateway_context.ReadSensor=SMGS_GateWay_ReadSensor;
        gateway_context.Device_Next=SMGS_Device_Next;
    }


		return true;
}


static uint8_t willbuff[256]= {0};
static SMGS_gateway_will_t will= {0};
void Net_Protocol_On_Connect(MQTTPacket_connectData *cfg)
{
	cfg->clientID.cstring=GateWaySerialNumber;
	cfg->username.cstring=GateWaySerialNumber;
	cfg->password.cstring=GateWaySerialNumber;
	
  SMGS_GateWay_Will_Encode(&gateway_context,&will,willbuff,sizeof(willbuff));

  cfg->will.topicName.cstring=(char *)will.topic;
  cfg->will.qos=will.qos;
  cfg->will.message.lenstring.data=(char *)will.payload;
  cfg->will.message.lenstring.len=will.payloadlen;
  cfg->will.retained=will.ratain;
  cfg->willFlag=1;	
}

void Net_Protocol_On_Connected(void)
{
	MQTT_Subscribe(subtopic);
}

void Net_Protocol_On_Subscribe_Success(void)
{
	{
    //发送网关上线消息
    uint8_t buff[512]= {0};
    SMGS_GateWay_Online(&gateway_context,buff,sizeof(buff),0,0);
  }	
}

static uint8_t buff[1024]={0};
void Net_Protocol_On_Message(const char *topic,size_t topiclen,void *payload,size_t payloadlen,uint8_t qos,uint8_t retain)
{
	SMGS_GateWay_Receive_MQTT_MSG(&gateway_context,topic,topiclen,payload,payloadlen,qos,retain,buff,sizeof(buff));
}
