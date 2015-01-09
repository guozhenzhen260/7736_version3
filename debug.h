#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <device.h>
#include <stdio.h>
#include "serial1.h"
extern uint ZhkSerial1Printf( uchar *format , ... );
extern void PrintTest( uchar *format , ... );
extern uchar getPcCmd(uchar xdata *MsgTemp);


//#define _DEBUG_MOBILE_TRACE

#ifdef _DEBUG_TRACE

  //extern void Trace( uchar *format , ... ); 
  #define Trace printf
  #define TraceInit()		{ZhkHardSramSelect();ZhkSerialInit();ES = 0;TI = 1;}

#else
#ifdef _DEBUG_MOBILE_TRACE
	#define Trace ZhkSerial1Printf
	#define TraceInit()  	{ZhkHardSerialSwitch( 0 );ZhkSerial1Init();}

#else
#ifdef _DEBUG_MOBIL_DEVICE_TRACE
	extern void Trace( uchar *format , ... ); 
	#define TraceInit()
#else
#define __InnerComment__ /
#define __Comment__ __InnerComment__##/
#define Trace ;__Comment__ while(0)
#define TraceInit()
#endif // _DEBUG_MOBIL_DEVICE_TRACE
#endif // _DEBUG_MOBILE_TRACE
#endif // _DEBUG_TRACE


#endif //__DEBUG_H__
