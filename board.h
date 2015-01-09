#ifndef _BOARD_
#define _BOARD_
//#include "global.h"
// IO 输出锁存
sbit PinLock 		= P1^4;
// IO 输出片选
//sbit PinCsOut 		= P3^5;	
// 键盘片选
//sbit PinCsKey		= P1^4;
// 硬币脉冲输入
sbit PinCoinIn 		= P3^3;	
#define HardIsCoinIn() ( PinCoinIn == 0 )

//振动器脉冲
sbit PinAlarmSIn    = P3^2;
#define HardAlarmCheck() ( PinAlarmSIn == 0 ) 

// 硬件留控管脚定义
sbit PinRTS		= P3^4;
sbit PinCTS		= P3^5;

// 扩展地址线定义
sbit PinAddr16 		= P1^5;
sbit PinAddr17 		= P1^6;
sbit PinAddr18 		= P1^7;

// IO 输入定义
// 当硬件选择为IO时的数据IO定义
sbit PinModeSw  	= P0^0;
sbit PinPowerOff 	= P0^1;

#define BitModeSw  	( 1 << 0 )
#define BitPowerOff 	( 1 << 1 )

// IO 输出定义
// 当硬件处于IO输出时的数据IO定义
sbit PinSa		= P0^0;
sbit PinSb		= P0^1;
sbit PinSc		= P0^2;
sbit PinBuzzer 		= P0^3;
sbit PinCoinerEn	= P0^4;
sbit PinStatus 		= P0^5;
sbit PinPowerCtrl	= P0^6;

#define BitSa  		( 1 << 0 )
#define BitSb  		( 1 << 1 )
#define BitSc   	( 1 << 2 )
#define BitBuzzer   	( 1 << 3 )
#define BitCoinerEn  	( 1 << 4 )
#define BitStatus   	( 1 << 5 )
#define BitPowerCtrl  	( 1 << 6 )

//#ifdef _COIN_INPUT_PARALLEL
#define BitCoinChn1     0x04
#define BitCoinChn2     0x08
#define BitCoinChn3     0x10
#define BitCoinChn4     0x20
#define BitCoinChn5     0x40
#define BitCoinChn6     ( 1 << 7 )
//#endif

#endif