#ifndef __BSP_GPIO_LIGHT_H
#define __BSP_GPIO_LIGHT_H

#include "stm32f10x.h"

/* 定义 LIGHT 连接的GPIO端口, 用户只需要修改下面的代码即可改变控制的 LIGHT 引脚 */

//LIGHT_DO
#define LIGHT_DO_GPIO_PORT          GPIOA                           /* GPIO端口 */
#define LIGHT_DO_GPIO_CLK_PORT      RCC_APB2Periph_GPIOA            /* GPIO端口时钟 */
#define LIGHT_DO_GPIO_PIN           GPIO_Pin_11                     /* 对应PIN脚 */

void LIGHT_GPIO_Config(void);
BitAction LIGHT_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void Light_Task(void);

#endif /* __BSP_GPIO_LIGHT_H */
