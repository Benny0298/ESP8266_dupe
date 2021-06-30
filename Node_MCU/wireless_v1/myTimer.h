#pragma once
#ifndef _myTimer_h
#define _myTimer_h

/****************************************
* * * * * * I N C L U D E S * * * * * * * 
****************************************/
#include <Arduino.h>
#include <Ticker.h>

/****************************************
* * * * * * * D E F I N E S * * * * * * * 
****************************************/
// Timer1

// choose timer1 re-arm value
// - program defaults to TIMER1_RE_ARM_ONE_S if no value is choen
// #define TIMER1_RE_ARM_HLF_S
#define TIMER1_RE_ARM_ONE_S
// #define TIMER1_RE_ARM_TWO_S

// timer1 re-arm values
#define TIMER1_TICKS_HLF_S 156250
#define TIMER1_TICKS_ONE_S 312500
#define TIMER1_TICKS_TWO_S 625000

/****************************************
* * * * * P R O T O T Y P E S * * * * * * 
****************************************/
namespace myTimer
{
  void ICACHE_RAM_ATTR onTime();
} // end of namespace
/****************************************
* * * * * * * * * E O F * * * * * * * * * 
****************************************/
#endif
