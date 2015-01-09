#define INITIALIZATION_C
#include "device.h"
#include "serial.h"
#include "serial1.h"
#include "timer.h"
#include "SstFlash.h"
#include "Clock.h"
#include "scheduler.h"
#include "key.h"
#include "IOInput.h"
#include "DataExchange.h"


uchar ZhkInit( void )
{
	ZhkHardSramSelect();
	ZhkIOInit();
	ZhkTimerInit();
//	SstFlashSelfCheck();
	ClockInit();
	ZhkSerialInit();
	ZhkSerial1Init();
	EA = 1;
	SchedulerInit();	
	KeyboardInit();
	KeyUnLock();
	ZhkHardSramSelect();
	return 0;
}
