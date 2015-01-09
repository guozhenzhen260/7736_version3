#ifndef _IOINPUT_
#define _IOINPUT_
//#include "global.h"

#ifndef IOINPUT_C
	#define IOInputSourse extern
#else
	#define IOInputSourse
#endif

	IOInputSourse struct COINAPCCEPTOR
	{
		unsigned char Enable;
		unsigned int  Line1;
		unsigned int  Line2;
		unsigned int  Line3;
		unsigned int  Line4;
		unsigned int  Line5;
		unsigned int  Line6;
		unsigned char check;
	};
	
	// 是否模式开关处于关闭状态
	IOInputSourse bit IsModeOff( void );

	// 硬币器使能
	IOInputSourse bit CoinerEnable( void );

	// 硬币器禁能
	IOInputSourse bit CoinerDisable( void );

	// 硬币器是否故障
	IOInputSourse bit IsCoinerException( void );

    IOInputSourse unsigned char IfCoinHopperFull( void );

	// 读取接收到的硬币金额
	IOInputSourse unsigned int GetCoin( void );

	// 是否处于调电状态
	IOInputSourse bit IsPowerOff( void );

	// 鸣响警报器
	IOInputSourse bit IfShakeAlarm( void );
	
	// 滴一声
	IOInputSourse void Beep( void );
	
/////////////////////////////////////////////////////////////////////////////////////	

	IOInputSourse void ZhkIOInit( void );

//#ifdef _COIN_INPUT_PARALLEL
//	IOInputSourse void ZyzChkCoinChnSta( void ) using 1
//#endif
    
    IOInputSourse bit IfGameKeyOn( void );
    IOInputSourse bit IfReturnKeyOn( void );

#endif 