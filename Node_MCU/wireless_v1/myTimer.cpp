/****************************************
* * * * * * I N C L U D E S * * * * * * * 
****************************************/
#include "myTimer.h"

/****************************************
* * * * * * * G L O B A L S * * * * * * * 
****************************************/
volatile int interrupts;
Ticker timer;

extern bool flagModbus;
/****************************************
* * * * * * F U N C T I O N S * * * * * * 
****************************************/
namespace myTimer
{
void ICACHE_RAM_ATTR onTime()
{
  interrupts++;

  // set flag to poll modbus slave
  flagModbus = true;

  // re-arm timer1 as using TIM_SINGLE
  // - TIM_DIV256 + 156250 ticks ... 0.5s
  // - TIM_DIV256 + 312500 ticks ... 1s
  // - TIM_DIV256 + 625000 ticks ... 2s
  #if defined(TIMER1_RE_ARM_HLF_S)
    timer1_write(TIMER1_TICKS_HLF_S);
  #elif defined(TIMER1_RE_ARM_ONE_S)
    timer1_write(TIMER1_TICKS_ONE_S);
  #elif defined(TIMER1_RE_ARM_TWO_S)
    timer1_write(TIMER1_TICKS_TWO_S);
  #else
    timer1_write(TIMER1_TICKS_ONE_S);
  #endif
}
} // enf of namespace

/****************************************
* * * * * * * * * E O F * * * * * * * * * 
****************************************/
// EOF
