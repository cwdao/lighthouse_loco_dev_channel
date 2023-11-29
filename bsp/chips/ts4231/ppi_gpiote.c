/**
\brief TS4231 driver.
\based on Official lib at https://github.com/TriadSemi/TS4231
\If you want to use different pins, please change these define:
\E_Pin P(0.3)
#define TS4231_N1_E_GPIO_PORT 0
#define TS4231_N1_E_GPIO_PIN 3
\D_Pin P(0,4)
#define TS4231_N1_D_GPIO_PORT 0
#define TS4231_N1_D_GPIO_PIN 4
\author Cheng Wang <cwang199@connect.hkust-gz.edu.cn>, Nov 2023.
\This file is also based on bmx160 driver, author Tengfei Chang <tengfei.chang@gmail.com>, Nov 2021.
*/

#include "ppi_gpiote.h"
#include "SEGGER_RTT.h"
#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "stdbool.h"
#include "ts4231.h"

// #include "ts_photosensors.h"
// #include "ts_pulse.h"

// NRF_SDK_V17.1, maybe in future
// #include "nrf_gpio.h"

//=========================== define ==========================================

//// E_Pin P(1,0)0.3
//#define TS4231_N1_E_GPIO_PORT 0
//#define TS4231_N1_E_GPIO_PIN 3
//// D_Pin P(0,4)
//#define TS4231_N1_D_GPIO_PORT 0
//#define TS4231_N1_D_GPIO_PIN 4

#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin)&0x1F))

//#define TS4231_N1_D_PIN NRF_GPIO_PIN_MAP(TS4231_N1_D_GPIO_PORT, TS4231_N1_D_GPIO_PIN) // Data signal pin P0.04

//// for GPIO mode
//#define MODE_INPUT 0
//#define MODE_OUTPUT 1
//// for GPIO output
//#define OUTPUT_LOW 0
//#define OUTPUT_HIGH 1

// we have use E:P(0,3) and D:P(0,4), and config them as input in ts4231.c
void gpiote_init(void)
{

    NRF_GPIOTE->CONFIG[0] =
        (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos) |
        (TS4231_N1_E_PIN << GPIOTE_CONFIG_PSEL_Pos) |
        (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);

    NRF_GPIOTE->CONFIG[1] =
        (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos) |
        (TS4231_N1_D_PIN << GPIOTE_CONFIG_PSEL_Pos) |
        (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);

    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos;
    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN1_Set << GPIOTE_INTENSET_IN1_Pos;
    NVIC_EnableIRQ(GPIOTE_IRQn);
}

void GPIOTE_IRQHandler(void)
{

    if ((NRF_GPIOTE->EVENTS_IN[0] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN0_Msk))
    {
        NRF_GPIOTE->EVENTS_IN[0] = 0;
    }

    // else
}

//测量角度首先需要从灯光判断开始。ts4231处于watch state 时，E pin常高，当有红外光照射时，E pin 会被拉低。
//此刻开启两个定时器，第一个用来判断此灯光持续时间，应于第一个上升沿来临时停止。
//按照规律，第二个下降沿是扫射激光到达，因此第二个定时器应此时停止。第二个定时器的数值就是速度。
//当然我们也会在中断里判断这个持续时间，以识别出更多的信息并确保可靠。
1s 16M tick,设置一个最大允许时间，略超过8ms，取10ms ，160，000

void ppi_init(void)
{
    // GPIOTE[0]与E pin(0,3)连接，并通往TIMER3计数任务.
    NRF_PPI->CH[0].EEP = (uint32_t)(&NRF_GPIOTE->EVENTS_IN[0]);
    NRF_PPI->CH[0].TEP = (uint32_t)(&NRF_TIMER3->TASKS_COUNT);
    // 用于计算两波间隔时间的第二个计数器TIMER3计数任务，同时启动.
    NRF_PPI->CH[0].EEP = (uint32_t)(&NRF_GPIOTE->EVENTS_IN[0]);
    NRF_PPI->CH[0].TEP = (uint32_t)(&NRF_TIMER4->TASKS_COUNT);

    //按照规律，第二个下降沿表示扫射激光，因此第二个定时器此时停止。
    NRF_PPI->CH[0].EEP = (uint32_t)(&NRF_GPIOTE->EVENTS_IN[0]);
    NRF_PPI->CH[0].TEP = (uint32_t)(&NRF_TIMER3->TASKS_COUNT);
    // nrf_drv_ppi_channel_assign（ppi_channel2, nrf_drv_gpiote_in_event_addr_get（输入），
    //  nrf_drv_timer_task_address_get（&timer0，NRF_TIMER_TASK_COUNT））；

    // enable channel 0,1,2,3
    NRF_PPI->CHENSET = 0x0f;
}