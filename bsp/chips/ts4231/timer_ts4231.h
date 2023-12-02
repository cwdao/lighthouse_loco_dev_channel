#ifndef _TIMER_H_
#define _TIMER_H_

#include "nrf52840.h"

//typedef void (*funcPtr_t)();

////class TimerClass {
////    public:
////        TimerClass(int timer = 1, int channel = 0);
////        void attachInterrupt(funcPtr_t callback, int microsec);
////        inline void detachInterrupt();
////    private:
////        NRF_TIMER_Type*        nrf_timer;
////        nrf_timer_cc_channel_t cc_channel;

////        void NVIC_set(IRQn_Type IRQn);
////};

//extern NRF_TIMER_Type* nrf_timers[5];
////extern TimerClass* Timers[5];

void TIMER_init(void);
void TIMER3_init(void);
void TIMER4_init(void);

__STATIC_INLINE void timer3_start(void)
{

    NRF_TIMER3->TASKS_START = 1;
}

__STATIC_INLINE void timer3_stop(void)
{

    NRF_TIMER3->TASKS_STOP = 1;
}

__STATIC_INLINE void timer4_start(void)
{

    NRF_TIMER4->TASKS_START = 1;
}

__STATIC_INLINE void timer4_stop(void)
{

    NRF_TIMER4->TASKS_STOP = 1;
}

__STATIC_INLINE uint32_t timer3_getCapturedValue(uint8_t compare_id) {

    return NRF_TIMER3->CC[compare_id];
}

__STATIC_INLINE uint32_t timer4_getCapturedValue(uint8_t compare_id) {

    return NRF_TIMER4->CC[compare_id];
}
#endif // _TIMER_H_

