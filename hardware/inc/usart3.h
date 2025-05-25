#ifndef __USART3_H
#define __USART3_H

#include "stm32f10x.h"

/*****************���絥Ƭ�����******************
                                            STM32
 * ��Ŀ            :    JW01������̼������ʵ��                     
 * �汾            : V1.0
 * ����            : 2024.9.14
 * MCU            :    STM32F103C8T6
 * �ӿ�            :    �δ���3                        
 * BILIBILI    :    ���絥Ƭ�����
 * CSDN            :    ���絥Ƭ�����
 * ����            :    ���� 

**********************BEGIN***********************/

extern uint8_t Usart3_RxPacket[6];                //����������ݰ�����
extern uint8_t Usart3_RxFlag;

void USART3_Config(void);
uint8_t Usart3_GetRxFlag(void);

void CO2GetData(u16 *data);

#endif


