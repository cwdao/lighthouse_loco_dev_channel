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
void gpiote_init(void) {

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

void GPIOTE_IRQHandler(void) {

  if ((NRF_GPIOTE->EVENTS_IN[0] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN0_Msk)) {
    NRF_GPIOTE->EVENTS_IN[0] = 0;
  }

  // else
}

void ppi_set(void) {
  NRF_PPI->CH[0].EEP = (uint32_t)(&NRF_TIMER3->EVENTS_COMPARE[0]);
  NRF_PPI->CH[0].TEP = (uint32_t)(&NRF_TIMER3->EVENTS_COMPARE[0]);
}