#ifndef _TIMER_
#define _TIMER_
#ifndef TIMER_C
	#define TimerSourse extern
#else
	#define TimerSourse
#endif
#include "device.h"

TimerSourse void ZhkTimerInit( void );

//TimerSourse uchar data g_byCommHeartBeatMins;
// 分钟定时器林哲专用
//TimerSourse uchar data g_byCommTimeOutSeconds;
// 秒定时器林哲专用
TimerSourse uchar xdata LzjSecTimer;

TimerSourse uchar xdata SgwSecTimer;
// 秒定时器林智杰专用
TimerSourse uchar xdata SgpSecTimer;
// 秒定时器吴章飞专用
TimerSourse uchar xdata YgpSecTimer;
// 秒定时器叶国平专用



TimerSourse uchar xdata LzjMSecTimer;
// 毫秒定时器林智杰专用
//TimerSourse uchar data MSecTimer2;
// 毫秒定时器2

//再次查询纸币器的时间间隔，秒级
TimerSourse uchar xdata QueryCashTimer;

//发短信间隔时间
TimerSourse uchar xdata SendSMSTimer;
//收短信重试间隔时间
TimerSourse uchar xdata ReceiveSMSTimer;

//手机模块指令回应超时时间
TimerSourse uchar xdata ZhkMobileSecTimer;
//短信报警的间隔时长,分钟计时器
TimerSourse uchar  xdata AlarmSpacing;
//手机模块激活与掉电计时器,当手机模块正常时用于定时测试手机模块，
//当手机模块需要掉电重置时用于记录掉电时长，秒级
TimerSourse uint xdata GPRSModuleResetTimer; 

//顾客查询等待时间,秒级
TimerSourse uchar xdata QueryTimer;

////////////////////////////////////////////////////////////////////////////////////////

TimerSourse uchar xdata ZhkSchedulerClock;	
//TimerSourse uchar data ZhkDownMsgAckTimer;
TimerSourse uint xdata ZhkDownMsgAckTimer;
TimerSourse  char xdata ZhkBuzzerTimer;

extern  char xdata lifterTimer;
 

#endif