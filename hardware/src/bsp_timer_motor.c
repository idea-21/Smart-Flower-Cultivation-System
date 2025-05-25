/**
 ******************************************************************************
 * @file    bsp_timer_motor.c
 * @author  fire
 * @version V1.0
 * @date    2024-xx-xx
 * @brief   
 ******************************************************************************
 * @attention
 *
 * 实验平台:野火 F103 STM32 开发板
 * 论坛    :http://www.firebbs.cn
 * 淘宝    :https://fire-stm32.taobao.com
 *
 ******************************************************************************
 */
#include "bsp_timer_motor.h"

GPIO_InitTypeDef gpio_initstruct;
TIM_TimeBaseInitTypeDef tim_base_initstruct;
TIM_OCInitTypeDef tim_oc_initstruct;

static void GPIO_Config(void)
{
    /* 使能GPIO时钟 */
    MOTOR_PWMA_RCC_CLK_ENABLE();

    /* 初始化GPIO */
    gpio_initstruct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_initstruct.GPIO_Pin = MOTOR_PWMA_GPIO_PIN;
    gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_PWMA_GPIO_PORT, &gpio_initstruct);
}

void TIMX_Motor_Config(uint16_t psc, uint16_t period, uint16_t pulse)
{
    /* 使能TIMX时钟 */
    TIM_MOTOR_RCC_CLK_ENABLE();

    /* =============================配置TIMX时基单元============================ */
    /* 初始化TIMX */
    tim_base_initstruct.TIM_ClockDivision = TIM_CKD_DIV1;
    tim_base_initstruct.TIM_CounterMode = TIM_CounterMode_Up;
    tim_base_initstruct.TIM_Period = period - 1;
    tim_base_initstruct.TIM_Prescaler = psc - 1;
    tim_base_initstruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM_MOTOR,&tim_base_initstruct);

    /* =============================配置TIMX比较输出单元========================== */
    tim_oc_initstruct.TIM_OCMode = TIM_OCMode_PWM1;
    tim_oc_initstruct.TIM_OutputState = TIM_OutputState_Enable;
    tim_oc_initstruct.TIM_OutputNState = TIM_OutputNState_Disable;
    tim_oc_initstruct.TIM_OCIdleState = TIM_OCIdleState_Set;
    tim_oc_initstruct.TIM_OCPolarity = TIM_OCPolarity_High;
    tim_oc_initstruct.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    tim_oc_initstruct.TIM_OCNPolarity = TIM_OCNPolarity_High;
    tim_oc_initstruct.TIM_Pulse = pulse - 1;

    TIM_OCxInit(TIM_MOTOR, &tim_oc_initstruct);

    /* =============================配置TIMX主从模式========================== */
    TIM_SelectInputTrigger(TIM_MOTOR,TIMX_ITR);
    TIM_SelectSlaveMode(TIM_MOTOR,TIM_SlaveMode_Reset);

    TIM_SelectOutputTrigger(TIM_MOTOR,TIM_TRGOSource_Update);
    TIM_SelectMasterSlaveMode(TIM_MOTOR,TIM_MasterSlaveMode_Enable);

    /* 配置GPIO引脚 */
    GPIO_Config();

    /* 使能TIMX */
    TIM_Cmd(TIM_MOTOR,ENABLE);

}

void Motor_Direction_GPIO_Config(void)
{
    MOTOR_DRIVER_AIN_CLK_ENABLE();
    MOTOR_DRIVER_STBY_CLK_ENABLE();

    gpio_initstruct.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_initstruct.GPIO_Pin = MOTOR_DRIVER_AIN1_PIN | MOTOR_DRIVER_AIN2_PIN;
    gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(MOTOR_DRIVER_AIN1_PORT, &gpio_initstruct);
    
    gpio_initstruct.GPIO_Pin = MOTOR_DRIVER_STBY_PIN;
    GPIO_Init(MOTOR_DRIVER_STBY_PORT, &gpio_initstruct);
    
    GPIO_SetBits(MOTOR_DRIVER_STBY_PORT,MOTOR_DRIVER_STBY_PIN);//STBY引脚上拉保持工作模式
}

/* 电机正转 */
void Motor_Forward_Direction(void)
{
    GPIO_SetBits(MOTOR_DRIVER_AIN2_PORT, MOTOR_DRIVER_AIN2_PIN);
    GPIO_ResetBits(MOTOR_DRIVER_AIN1_PORT, MOTOR_DRIVER_AIN1_PIN);
}

/* 电机反转 */
void Motor_Backward_Direction(void)
{
    GPIO_SetBits(MOTOR_DRIVER_AIN1_PORT, MOTOR_DRIVER_AIN1_PIN);
    GPIO_ResetBits(MOTOR_DRIVER_AIN2_PORT, MOTOR_DRIVER_AIN2_PIN);
}

void Motor_Set_Speed(uint16_t pulse)
{
    // 获取当前定时器的周期值 (ARR - Auto-Reload Register)
    // TIM_Period 在初始化时设置为 period - 1, 所以 ARR = period - 1
    uint16_t max_pulse = TIM_MOTOR->ARR; // 读取 TIM3->ARR

    // 限制 pulse 值不超过最大值 (ARR 的值)
    // 如果 pulse 等于 period (即 ARR+1), 占空比会是 100%
    // 如果 pulse 大于 period, 行为可能未定义或保持 100%
    // 这里我们限制 pulse 最大为 ARR 的值，以确保可控
    if (pulse > max_pulse)
    {
        pulse = max_pulse;
    }

    // 设置 TIM3 的通道 3 比较寄存器值 (CCR3)
    // TIMXCCRX 在 H 文件中定义为 TIM3->CCR3
    TIMXCCRX = pulse;

    // 或者，也可以使用标准外设库函数 (效果相同):
    // TIM_SetCompare3(TIM_MOTOR, pulse);
}

/**
  * @brief  通过将 PWM 占空比设置为 0 来平滑停止电机
  * @param  None
  * @retval None
  */
void Motor_Stop_By_PWM(void)
{
    // 调用设置速度的函数，将脉冲宽度设置为 0
    // 这将使 PWM 输出的占空比变为 0%，从而停止向电机供电
    Motor_Set_Speed(0);
}

void Motor_Run_By_PWM(void)
{
    // 调用设置速度的函数，将脉冲宽度设置为 0
    // 这将使 PWM 输出的占空比变为 0%，从而停止向电机供电
    Motor_Set_Speed(50);
}

void Motor_Wakeup(void)
{
    GPIO_SetBits(MOTOR_DRIVER_STBY_PORT, MOTOR_DRIVER_STBY_PIN);
}
/*********************************************END OF FILE**********************/
