#ifndef _CLOCK_
#define _CLOCK_
#ifndef CLOCK_C
	#define ClockSourse extern
#else
	#define ClockSourse 
#endif
/*	struct time 					// 时间数据类型定义，各个字段均为BCD码表示
	{
		unsigned char year;			// 年
		unsigned char mon;			// 月
		unsigned char date;			// 日
		unsigned char hour;			// 时
		unsigned char min;			// 分
		unsigned char sec;			// 秒
	};
*/	
	ClockSourse bit ClockInit( void ) small;
	// 时钟初始化
//	ClockSourse bit GetCurrentTime( struct time xdata* CurrentTime ) small;
	// 读取当前时间
//	ClockSourse bit SetCurrentTime( struct time xdata* CurrentTime ) small;
	// 设置当前时间
	
#endif