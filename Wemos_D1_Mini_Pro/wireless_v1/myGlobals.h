#pragma once
#ifndef _myGlobals_h
#define _myGlobals_h

/****************************************
* * * * * * I N C L U D E S * * * * * * * 
****************************************/
#include <SoftwareSerial.h>

/****************************************
* * * * * * * G L O B A L S * * * * * * * 
****************************************/
SoftwareSerial usbSerial(3, 1); // debugging info via usb
int count = 0;
int randNum = 0;
float pos = 0;
uint16_t posReg = 0;
uint16_t statusReg = 0;
uint32_t eSource = 0;
bool flagModbus = false;
bool dataChanged = false;
/****************************************
* * * * * * * * * E O F * * * * * * * * * 
****************************************/
#endif
