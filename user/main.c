//单片机头文件
#include "stm32f10x.h"
//网络协议层
#include "onenet.h"
//网络设备
#include "esp8266.h"
//硬件驱动
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "dht11.h"
#include "oled.h"
#include "bsp_adc.h"
#include "bsp_gpio_light.h"
#include "bsp_timer_motor.h"
#include "bsp_gpio_beep.h"
#include "MOSFET.h"
#include "usart3.h"

#include "sys.h"
//C库
#include <string.h>

#define ESP8266_ONENET_INFO        "AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n"

void Hardware_Init(void);
void Refresh_Data(void);
void Draw_Page_Labels(u8 page);

u8 g_light_manual_override = 0;
u8 g_motor_manual_override = 0;
u8 g_MOSFET_manual_override = 0;
uint32_t g_manual_light_override_timeout_ticks = 100;
uint32_t g_manual_motor_override_timeout_ticks = 100;
uint32_t g_manual_MOSFET_override_timeout_ticks = 100;
u8 temp,humi,temp_value= 30, humi_value = 50;
u16 soil_adc_raw=0, rain_adc_raw=0;
u16 co = 0;
u8 g_motor_status = 0;
u8 g_MOSFET_status = 0;
volatile u8 g_oled_current_page = 0;
volatile u8 g_key_pb15_pressed_flag = 0;
double rain_mm_calc=0.0;
extern __IO uint16_t ADC_ConvertedValue[NOFCHANEL];

#define OLED_PAGE_1 0
#define OLED_PAGE_2 1
#define OLED_NUM_PAGES 2

/*
************************************************************
*    函数名称：    main
*
*    函数功能：    
*
*    入口参数：    无
*
*    返回参数：    0
*
*    说明：        
************************************************************
*/
int main(void)
{
    unsigned short timeCount = 0;
    unsigned char *dataPtr = NULL;
    u8 key_val = 0;

    Hardware_Init();
    ESP8266_Init();

    OLED_Clear(); OLED_ShowString(0,0,"Connect MQTTs Server...",16);
    while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
        DelayXms(500);
    OLED_ShowString(0,4,"Connect MQTT Server Success",16); DelayXms(500);

    OLED_Clear(); OLED_ShowString(0,0,"Device login ...",16);
    while(OneNet_DevLink())
    {
        ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT");
        DelayXms(500);
    }

    OneNET_Subscribe();
    Draw_Page_Labels(g_oled_current_page);

    while(1)
    {
        if(g_key_pb15_pressed_flag)
        {
            g_key_pb15_pressed_flag = 0;

            g_oled_current_page = (g_oled_current_page + 1) % OLED_NUM_PAGES;
            Draw_Page_Labels(g_oled_current_page);
        }

        if(++timeCount >= 100)
        {
            DHT11_Read_Data(&temp,&humi);
            CO2GetData(&co);

            rain_adc_raw = ADC_ConvertedValue[0];
            soil_adc_raw = soil_num(ADC_ConvertedValue[1]);
            rain_mm_calc = rain_num(rain_adc_raw);

            OneNet_SendData();
            timeCount = 0;
            ESP8266_Clear();
        }

        dataPtr = ESP8266_GetIPD(0);
        if(dataPtr != NULL)
            OneNet_RevPro(dataPtr);

        Refresh_Data();

        DelayMs(10);
        if (g_light_manual_override && g_manual_light_override_timeout_ticks > 0)
        {
            g_manual_light_override_timeout_ticks--;
            if (g_manual_light_override_timeout_ticks == 0)
            {
                g_light_manual_override = 0; 
                UsartPrintf(USART_DEBUG, "Manual light override timed out. Reverting to auto.\r\n");
            }
        }

        if (!g_light_manual_override) 
        {
            Light_Task(); 
        }
         if(!g_motor_manual_override)
        {
            if(temp>temp_value)
            {
                BEEP_ON(BEEP_GPIO_PORT, BEEP_GPIO_PIN, BEEP_HIGH_TRIGGER);
                Motor_Wakeup();
                Motor_Set_Speed(30);
                Motor_Forward_Direction();
                g_motor_status = 1;
            }
            else
            {
                BEEP_OFF(BEEP_GPIO_PORT, BEEP_GPIO_PIN, BEEP_HIGH_TRIGGER);
                Motor_Stop_By_PWM();
                g_motor_status = 0;
            }
        }
        else
        {
            if(g_motor_status)
            {
                Motor_Wakeup();
                Motor_Set_Speed(30);
                Motor_Forward_Direction();
            }
            else
            {
                Motor_Stop_By_PWM();
            }
        }
        if(!g_MOSFET_manual_override)
        {
            if(soil_adc_raw<humi_value)
            {
                MOSFET_PIN = 1;            //on
                g_MOSFET_status = 1;
            }
            else
            {
                MOSFET_PIN = 0;            //off
                g_MOSFET_status = 0;
            }
        }
        else
        {
            if(g_MOSFET_status)
            {
                MOSFET_PIN = 1;
            }
            else
            {
                MOSFET_PIN = 0;
            }
        }
    }
}
/*
************************************************************
*    函数名称：    Hardware_Init
*
*    函数功能：    硬件初始化
*
*    入口参数：    无
*
*    返回参数：    无
*
*    说明：        初始化单片机功能以及外接设备
************************************************************
*/
void Hardware_Init(void)
{
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);    //中断控制器分组设置

    Delay_Init();                                    //systick初始化
    
    Usart1_Init(115200);                            //串口1，打印信息用
    
    Usart2_Init(115200);                            //串口2，驱动ESP8266用
    
    USART3_Config();
    
    Key_Init();
    
    Led_Init();                                    //LED初始化
    
    OLED_Init();                                    //初始化OLED  
    
    ADCx_Init();
    LIGHT_GPIO_Config();
    Motor_Direction_GPIO_Config();
    TIMX_Motor_Config(72, 100, 30);
    BEEP_GPIO_Config();
    Init_MOSFET_pin();
    
    while(DHT11_Init())
    {
//      UsartPrintf(USART_DEBUG, "DHT11 Error \r\n");
        OLED_ShowString(0,0,"DHT11 Error",16);
        DelayMs(1000);
    }
    
//  UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
    OLED_Clear(); OLED_ShowString(0,0,"Hardware init OK",16); DelayMs(1000);
}
void Draw_Page_Labels(u8 page)
{
    OLED_Clear();

    if(page == OLED_PAGE_1)
    {
        OLED_ShowCHinese(0,  0, 1); // 温
        OLED_ShowCHinese(18, 0, 2); // 度
        OLED_ShowCHinese(36, 0, 0); // ：
        OLED_ShowCHinese(100, 0, 3); // ℃

        OLED_ShowCHinese(0,  2, 4); // 湿
        OLED_ShowCHinese(18, 2, 5); // 度
        OLED_ShowCHinese(36, 2, 0); // ：
        OLED_ShowString(100, 2, "%", 16);

        OLED_ShowCHinese(0,  4, 10); // 土
        OLED_ShowCHinese(18, 4, 11); // 壤
        OLED_ShowCHinese(36, 4, 0); // ：
        OLED_ShowString(100, 4, "%", 16);

        OLED_ShowCHinese(0,  6, 12); // 雨
        OLED_ShowCHinese(18, 6, 13); // 量
        OLED_ShowCHinese(36, 6, 0); // ：
        OLED_ShowString(100, 6, "mm", 16);
    }
    else if(page == OLED_PAGE_2)
    {
        OLED_ShowString(0,0,"CO2:",16);
        OLED_ShowString(100,0,"ppm",16);

        OLED_ShowCHinese(0,  2, 7);  // 灯
        OLED_ShowCHinese(36, 2, 0);  // ：

        OLED_ShowCHinese(0,  4, 14); // 风
        OLED_ShowCHinese(18, 4, 15); // 扇
        OLED_ShowCHinese(36, 4, 0); // ：

        OLED_ShowCHinese(0,  6, 16); // 水
        OLED_ShowCHinese(18, 6, 17); // 泵
        OLED_ShowCHinese(36, 6, 0); // ：
    }
}
void Refresh_Data(void)
{
    char buf[10];

    if(g_oled_current_page == OLED_PAGE_1)
    {
        OLED_ShowString(60, 0, "   ", 16); 
        sprintf(buf, "%2d", temp);
        OLED_ShowString(60, 0, buf, 16);

        OLED_ShowString(60, 2, "   ", 16);
        sprintf(buf, "%2d", humi);
        OLED_ShowString(60, 2, buf, 16);

        OLED_ShowString(60, 4, "   ", 16);
        sprintf(buf, "%2d", soil_adc_raw);
        OLED_ShowString(60, 4, buf, 16);

        OLED_ShowString(60, 6, "     ", 16);
        sprintf(buf, "%3d", (uint16_t)rain_mm_calc);
        OLED_ShowString(60, 6, buf, 16);
    }
    else if(g_oled_current_page == OLED_PAGE_2)
    {
        OLED_ShowString(48, 0, "    ", 16);
        sprintf(buf, "%4d", co);
        OLED_ShowString(48, 0, buf, 16);

        OLED_ShowString(60, 2, "    ", 16);
        if(GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_5) == Bit_RESET)
             OLED_ShowString(60,2,"ON ",16);
        else
             OLED_ShowString(60,2,"OFF",16);

        OLED_ShowString(60, 4, "    ", 16);
        if(g_motor_status)
            OLED_ShowString(60, 4, "ON ", 16);
        else
            OLED_ShowString(60, 4, "OFF", 16);

        OLED_ShowString(60, 6, "    ", 16);
        if(g_MOSFET_status)
            OLED_ShowString(60, 6, "ON ", 16);
        else
            OLED_ShowString(60, 6, "OFF", 16);
    }
}
