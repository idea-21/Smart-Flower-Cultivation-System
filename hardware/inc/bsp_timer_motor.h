#ifndef __BSP_TIMER_MOTOR_H
#define __BSP_TIMER_MOTOR_H

#include "stm32f10x.h"



/* �˴���TIMER3Ϊ�� �û����Զ���TIMERx
 * ���STM32�ٷ��û��ֲ�
 */
#define TIM_MOTOR                        TIM3
#define TIM_MOTOR_RCC_CLK_ENABLE()       RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE)

/* TIMX_CHANNEL���ź궨�� 
 * �������������ռ�ձȿɵ���PWM�� �������ڵ����ת��
 * ��������ģ�� PWMA/B ����
 */
#define MOTOR_PWMA_RCC_CLK_ENABLE()  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE)
#define TIM_OCxInit                  TIM_OC3Init
#define MOTOR_PWMA_CHANNEL           TIM_Channel_3
#define MOTOR_PWMA_GPIO_PORT         GPIOB
#define MOTOR_PWMA_GPIO_PIN          GPIO_Pin_0
#define TIMXCCRX                     TIM3->CCR3              

/* ����Դѡ��(TS)
 * �˴����ڲ�����(ITR)Ϊ�� �û����Զ��崥��Դ
 * ���STM32�ٷ��û��ֲ�
 */
#define TIMX_ITR                    TIM_TS_ITR2

/* �������ģ�� AIN1/2 ���ź궨�� 
 * ���������ڿ��Ƶ������/��ת
 * �� AIN1/2 ���� BIN1/2 ���� �û����Զ���ѡ��
 */
#define MOTOR_DRIVER_AIN_CLK_ENABLE()   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE)
#define MOTOR_DRIVER_AIN1_PORT          GPIOA
#define MOTOR_DRIVER_AIN1_PIN           GPIO_Pin_4

#define MOTOR_DRIVER_AIN2_PORT          GPIOA
#define MOTOR_DRIVER_AIN2_PIN           GPIO_Pin_5

#define MOTOR_DRIVER_STBY_CLK_ENABLE()  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE)
#define MOTOR_DRIVER_STBY_PORT          GPIOA
#define MOTOR_DRIVER_STBY_PIN           GPIO_Pin_7


void TIMX_Motor_Config(uint16_t psc, uint16_t period, uint16_t pulse);
void Motor_Direction_GPIO_Config(void);
void Motor_Forward_Direction(void);
void Motor_Backward_Direction(void);
void Motor_Set_Speed(uint16_t pulse);
void Motor_Stop_By_PWM(void);
void Motor_Run_By_PWM(void);
void Motor_Wakeup(void);


#endif


