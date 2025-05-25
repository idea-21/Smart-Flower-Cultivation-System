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
 * ʵ��ƽ̨:Ұ�� F103 STM32 ������
 * ��̳    :http://www.firebbs.cn
 * �Ա�    :https://fire-stm32.taobao.com
 *
 ******************************************************************************
 */
#include "bsp_timer_motor.h"

GPIO_InitTypeDef gpio_initstruct;
TIM_TimeBaseInitTypeDef tim_base_initstruct;
TIM_OCInitTypeDef tim_oc_initstruct;

static void GPIO_Config(void)
{
    /* ʹ��GPIOʱ�� */
    MOTOR_PWMA_RCC_CLK_ENABLE();

    /* ��ʼ��GPIO */
    gpio_initstruct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_initstruct.GPIO_Pin = MOTOR_PWMA_GPIO_PIN;
    gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_PWMA_GPIO_PORT, &gpio_initstruct);
}

void TIMX_Motor_Config(uint16_t psc, uint16_t period, uint16_t pulse)
{
    /* ʹ��TIMXʱ�� */
    TIM_MOTOR_RCC_CLK_ENABLE();

    /* =============================����TIMXʱ����Ԫ============================ */
    /* ��ʼ��TIMX */
    tim_base_initstruct.TIM_ClockDivision = TIM_CKD_DIV1;
    tim_base_initstruct.TIM_CounterMode = TIM_CounterMode_Up;
    tim_base_initstruct.TIM_Period = period - 1;
    tim_base_initstruct.TIM_Prescaler = psc - 1;
    tim_base_initstruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM_MOTOR,&tim_base_initstruct);

    /* =============================����TIMX�Ƚ������Ԫ========================== */
    tim_oc_initstruct.TIM_OCMode = TIM_OCMode_PWM1;
    tim_oc_initstruct.TIM_OutputState = TIM_OutputState_Enable;
    tim_oc_initstruct.TIM_OutputNState = TIM_OutputNState_Disable;
    tim_oc_initstruct.TIM_OCIdleState = TIM_OCIdleState_Set;
    tim_oc_initstruct.TIM_OCPolarity = TIM_OCPolarity_High;
    tim_oc_initstruct.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    tim_oc_initstruct.TIM_OCNPolarity = TIM_OCNPolarity_High;
    tim_oc_initstruct.TIM_Pulse = pulse - 1;

    TIM_OCxInit(TIM_MOTOR, &tim_oc_initstruct);

    /* =============================����TIMX����ģʽ========================== */
    TIM_SelectInputTrigger(TIM_MOTOR,TIMX_ITR);
    TIM_SelectSlaveMode(TIM_MOTOR,TIM_SlaveMode_Reset);

    TIM_SelectOutputTrigger(TIM_MOTOR,TIM_TRGOSource_Update);
    TIM_SelectMasterSlaveMode(TIM_MOTOR,TIM_MasterSlaveMode_Enable);

    /* ����GPIO���� */
    GPIO_Config();

    /* ʹ��TIMX */
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
    
    GPIO_SetBits(MOTOR_DRIVER_STBY_PORT,MOTOR_DRIVER_STBY_PIN);//STBY�����������ֹ���ģʽ
}

/* �����ת */
void Motor_Forward_Direction(void)
{
    GPIO_SetBits(MOTOR_DRIVER_AIN2_PORT, MOTOR_DRIVER_AIN2_PIN);
    GPIO_ResetBits(MOTOR_DRIVER_AIN1_PORT, MOTOR_DRIVER_AIN1_PIN);
}

/* �����ת */
void Motor_Backward_Direction(void)
{
    GPIO_SetBits(MOTOR_DRIVER_AIN1_PORT, MOTOR_DRIVER_AIN1_PIN);
    GPIO_ResetBits(MOTOR_DRIVER_AIN2_PORT, MOTOR_DRIVER_AIN2_PIN);
}

void Motor_Set_Speed(uint16_t pulse)
{
    // ��ȡ��ǰ��ʱ��������ֵ (ARR - Auto-Reload Register)
    // TIM_Period �ڳ�ʼ��ʱ����Ϊ period - 1, ���� ARR = period - 1
    uint16_t max_pulse = TIM_MOTOR->ARR; // ��ȡ TIM3->ARR

    // ���� pulse ֵ���������ֵ (ARR ��ֵ)
    // ��� pulse ���� period (�� ARR+1), ռ�ձȻ��� 100%
    // ��� pulse ���� period, ��Ϊ����δ����򱣳� 100%
    // ������������ pulse ���Ϊ ARR ��ֵ����ȷ���ɿ�
    if (pulse > max_pulse)
    {
        pulse = max_pulse;
    }

    // ���� TIM3 ��ͨ�� 3 �ȽϼĴ���ֵ (CCR3)
    // TIMXCCRX �� H �ļ��ж���Ϊ TIM3->CCR3
    TIMXCCRX = pulse;

    // ���ߣ�Ҳ����ʹ�ñ�׼����⺯�� (Ч����ͬ):
    // TIM_SetCompare3(TIM_MOTOR, pulse);
}

/**
  * @brief  ͨ���� PWM ռ�ձ�����Ϊ 0 ��ƽ��ֹͣ���
  * @param  None
  * @retval None
  */
void Motor_Stop_By_PWM(void)
{
    // ���������ٶȵĺ�����������������Ϊ 0
    // �⽫ʹ PWM �����ռ�ձȱ�Ϊ 0%���Ӷ�ֹͣ��������
    Motor_Set_Speed(0);
}

void Motor_Run_By_PWM(void)
{
    // ���������ٶȵĺ�����������������Ϊ 0
    // �⽫ʹ PWM �����ռ�ձȱ�Ϊ 0%���Ӷ�ֹͣ��������
    Motor_Set_Speed(50);
}

void Motor_Wakeup(void)
{
    GPIO_SetBits(MOTOR_DRIVER_STBY_PORT, MOTOR_DRIVER_STBY_PIN);
}
/*********************************************END OF FILE**********************/
