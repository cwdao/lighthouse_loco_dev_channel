
//#include "nRF_SDK/nrf_timer.h"
#include "timer_ts4231.h"
#include "nrf52840.h"
#include "nrf52840_bitfields.h"

void TIMER_init(void) {

  // 16MHz HFCLOCK enabled on board_init()

  TIMER3_init();
  TIMER4_init();
}

void TIMER3_init(void) {

  NRF_TIMER3->MODE = TIMER_MODE_MODE_Timer;
  // 2^0 = 1,16MHz
  NRF_TIMER3->PRESCALER = 0;
  NRF_TIMER3->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
  // clear timer before use
  NRF_TIMER3->TASKS_CLEAR = 1;
  timer3_start();
}

void TIMER4_init(void) {

  NRF_TIMER4->MODE = TIMER_MODE_MODE_Timer;
  // 2^0 = 1,16MHz
  NRF_TIMER4->PRESCALER = 0;
  NRF_TIMER4->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
  // clear timer before use
  NRF_TIMER4->TASKS_CLEAR = 1;
  timer4_start();
}