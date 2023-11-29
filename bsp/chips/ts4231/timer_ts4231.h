#ifndef _TIMER_H_
#define _TIMER_H_

#include "nrf52840.h"

typedef void (*funcPtr_t)();

//class TimerClass {
//    public:
//        TimerClass(int timer = 1, int channel = 0);
//        void attachInterrupt(funcPtr_t callback, int microsec);
//        inline void detachInterrupt();
//    private:
//        NRF_TIMER_Type*        nrf_timer;
//        nrf_timer_cc_channel_t cc_channel;

//        void NVIC_set(IRQn_Type IRQn);
//};

extern NRF_TIMER_Type* nrf_timers[5];
//extern TimerClass* Timers[5];

void TIMER_init(void);
void TIMER3_init(void);
void TIMER4_init(void);

#endif // _TIMER_H_

