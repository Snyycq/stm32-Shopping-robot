#ifndef __TCRT5000_H
#define	__TCRT5000_H

#include "stm32f4xx.h"

#define TCRT5000_1  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_5)    //��ȡѭ��������1  --PE5
#define TCRT5000_2  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)   //��ȡѭ��������2  --PB15
#define TCRT5000_0  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)   //��ȡѭ��������0  --PB14

void TCRT5000_config(void);

#endif
