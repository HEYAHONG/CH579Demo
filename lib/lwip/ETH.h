#ifndef __ETH_H__
#define __ETH_H__

#include "CH57x_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ETH_MAX_HOSTNAME_LENGTH 16
	
//��ʼ��ETH
void ETH_Init(void);
	
//���ETH״̬
void ETH_CheckState(void);

	
#ifdef __cplusplus
}
#endif

#endif
