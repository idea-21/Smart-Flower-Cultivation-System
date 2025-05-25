#ifndef __BSP_TIMER_MOTOR_H
#define __BSP_TIMER_MOTOR_H

#include "stm32f10x.h"



/* 此处以TIMER3为例 用户可自定义TIMERx
 * 详见STM32官方用户手册
 */
#define TIM_MOTOR                        TIM3
#define TIM_MOTOR_RCC_CLK_ENABLE()       RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE)

/* TIMX_CHANNEL引脚宏定义 
 * 该引脚用于输出占空比可调的PWM波 进而调节电机的转速
 * 与电机驱动模块 PWMA/B 相连
 */
#define MOTOR_PWMA_RCC_CLK_ENABLE()  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE)
#define TIM_OCxInit                  TIM_OC3Init
#define MOTOR_PWMA_CHANNEL           TIM_Channel_3
#define MOTOR_PWMA_GPIO_PORT         GPIOB
#define MOTOR_PWMA_GPIO_PIN          GPIO_Pin_0
#define TIMXCCRX                     TIM3->CCR3              

/* 触发源选择(TS)
 * 此处以内部触发(ITR)为例 用户可自定义触发源
 * 详见STM32官方用户手册
 */
#define TIMX_ITR                    TIM_TS_ITR2

/* 电机驱动模块 AIN1/2 引脚宏定义 
 * 该引脚用于控制电机的正/反转
 * 与 AIN1/2 或者 BIN1/2 相连 用户可自定义选择
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


