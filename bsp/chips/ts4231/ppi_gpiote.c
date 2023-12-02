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
#include "timer_ts4231.h"
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
void gpiote_init(void) {
  //下降沿表示一个波的开始
  NRF_GPIOTE->CONFIG[0] =
      (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos) |
      (TS4231_N1_E_PIN << GPIOTE_CONFIG_PSEL_Pos) |
      (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);
  //上升沿表示一个波的结束
  NRF_GPIOTE->CONFIG[1] =
      (GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos) |
      (TS4231_N1_E_PIN << GPIOTE_CONFIG_PSEL_Pos) |
      (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);

  NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos;
  NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN1_Set << GPIOTE_INTENSET_IN1_Pos;
  NVIC_EnableIRQ(GPIOTE_IRQn);
}
// t_0:用于计算每个光脉冲持续时间
// t_1:用于计算两脉冲间隔
uint32_t t_0_start = 0x00;
uint32_t t_0_end = 0x00;
uint32_t t_opt_pulse = 0x00;
uint32_t t_opt_pulse_us = 0x00;
uint32_t t_1_start = 0x00;
int8_t flag_start = 0;
int32_t flag_opt = 0;
bool flag_sweep = 0;
int32_t count_sweep = 0;
int8_t flag_sync = 0;

bool cal_distance = 0;

uint32_t loca_duration = 0;
int8_t loca_x = 0;
uint32_t L_X = 0;
uint32_t L_Y = 0;

void GPIOTE_IRQHandler(void) {
  //下降沿中断
  if ((NRF_GPIOTE->EVENTS_IN[0] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN0_Msk)) {
    NRF_GPIOTE->EVENTS_IN[0] = 0;
    //
    switch (flag_start) {
    case 0:
      t_0_start = timer3_getCapturedValue(0);

      // t_1_start = timer4_getCapturedValue(0);
      flag_start++;

      if (cal_distance == 1) {
        loca_duration = t_0_start - t_1_start;
      }
      if (loca_x==1){
      L_X = loca_duration;
      }
      else{
      L_Y = loca_duration;
      }


      t_1_start = t_0_start;
      break;
    case 1:
      //计个数
      flag_opt++;
      break;
    default:
      break;
    }
  }

  //上升沿中断
  if ((NRF_GPIOTE->EVENTS_IN[1] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN1_Msk)) {
    NRF_GPIOTE->EVENTS_IN[1] = 0;
    //

    switch (flag_start) {
    case 0:
      break;
    case 1:
      t_0_end = timer3_getCapturedValue(1);
      flag_start = 0;
      t_opt_pulse = t_0_end - t_0_start;
      //以50us为界划分:0.000,050/(1/16M) = 800 = 0x320
      (t_opt_pulse < 800) ? (flag_sweep = 1) : (flag_sweep = 0);
      t_opt_pulse_us = t_opt_pulse/16;
      switch (flag_sweep) {
      //超过50us则肯定是sync，那么下次下降沿中断就需要计算位置了
      case 0:
        cal_distance = 1;
        //凡十位数为偶数表示X轴，为奇数则是Y轴
        ((t_opt_pulse_us/10)%2==0)?(loca_x=1):(loca_x=0);


        break;
      case 1:
      cal_distance = 0;
        // if (cal_distance)
        break;
      }
      break;
    default:
      break;
    }
  }
  // else
}

//测量角度首先需要从灯光判断开始。ts4231处于watch state 时，E pin常高，当有红外光照射时，E pin 会被拉低。
//此刻开启两个定时器，使用第一个TIMER（TIMER3）用来判断此灯光持续时间，应于第一个上升沿来临时记录数值一次，记为t0。
//按照规律，第二个下降沿是扫射激光到达，因此第二个定时器（TIMER4）应此时计时。
//综上，TIMER3在每次边沿都触发，TIMER4只在下降沿触发。
//当然,更多逻辑需要在GPIO中断里执行，以识别出更多的信息并确保可靠。
// 1s 16M tick,设置一个最大允许时间，略超过8ms，取10ms ，160，000

void ppi_init(void) {
  // GPIOTE[0]与E pin(0,3)连接，并通往TIMER3计数任务.
  // TIME 初始化时已 start, 会持续自增，因此此处只需要获取当前数值
  NRF_PPI->CH[0].EEP = (uint32_t)(&NRF_GPIOTE->EVENTS_IN[0]);
  NRF_PPI->CH[0].TEP = (uint32_t)(&NRF_TIMER3->TASKS_CAPTURE[0]);
  // 用于计算两波间隔时间的第二个计数器TIMER4计数任务，同时启动.
  NRF_PPI->CH[1].EEP = (uint32_t)(&NRF_GPIOTE->EVENTS_IN[0]);
  NRF_PPI->CH[1].TEP = (uint32_t)(&NRF_TIMER4->TASKS_CAPTURE[0]);

  //在上升沿到来时，捕获计数值至对应CC[n]
  //也不需要使用stop，因为32bit的寄存器远远超过几十ms,因此不用考虑溢出问题
  NRF_PPI->CH[2].EEP = (uint32_t)(&NRF_GPIOTE->EVENTS_IN[1]);
  NRF_PPI->CH[2].TEP = (uint32_t)(&NRF_TIMER3->TASKS_CAPTURE[1]);
  //按照规律，第二个下降沿表示扫射激光，因此第二个定时器此时停止。
  NRF_PPI->CH[3].EEP = (uint32_t)(&NRF_GPIOTE->EVENTS_IN[0]);
  NRF_PPI->CH[3].TEP = (uint32_t)(&NRF_TIMER4->TASKS_CAPTURE[0]);

  // enable channel 0,1,2,3
  NRF_PPI->CHENSET = 0x0f;
}