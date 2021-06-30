#pragma once
#ifndef _wireless_v1_h
#define _wireless_v1_h


#include "myMQTT.h"
#include "myModbusRTU.h"
#include "myTimer.h"


extern Ubidots ubidotsClient;
extern ModbusRTU mb;
extern volatile int interrupts;

#endif
