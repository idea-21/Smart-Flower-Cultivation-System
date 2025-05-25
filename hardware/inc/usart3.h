#ifndef __USART3_H
#define __USART3_H

#include "stm32f10x.h"

/*****************辰哥单片机设计******************
                                            STM32
 * 项目            :    JW01二氧化碳传感器实验                     
 * 版本            : V1.0
 * 日期            : 2024.9.14
 * MCU            :    STM32F103C8T6
 * 接口            :    参串口3                        
 * BILIBILI    :    辰哥单片机设计
 * CSDN            :    辰哥单片机设计
 * 作者            :    辰哥 

**********************BEGIN***********************/

extern uint8_t Usart3_RxPacket[6];                //定义接收数据包数组
extern uint8_t Usart3_RxFlag;

void USART3_Config(void);
uint8_t Usart3_GetRxFlag(void);

void CO2GetData(u16 *data);

#endif


