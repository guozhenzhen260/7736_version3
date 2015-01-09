#ifndef _DEVICE_
#define _DEVICE_
#ifndef DEVICE_C
	#define BasicSourse extern
#else
	#define BasicSourse
#endif

#include <reg669.h>
#include <ABSACC.h>
#include <intrins.h>
#include "board.h"
#ifdef __REG669_H__

    #define ES      	ES0
    
    #define SM0  	SM0_0
    #define FE  	FE_0
    #define SM1  	SM1_0
    #define SM2  	SM2_0
    #define REN  	REN_0
    #define TB8  	TB8_0
    #define RB8  	RB8_0
    #define TI  	TI_0
    #define RI  	RI_0
#endif
typedef unsigned char uchar;
typedef unsigned int  uint;
typedef unsigned long ulong;


#define OSCRATE 		11059200
#define MACHINECYCLE   		6			// 机器周期与晶振周期的比例关系

//#define DOWNMSGACKTIMEOUT   	30

// flash地址分配

// 系统参数,从第三个扇区开始,占用二个扇区
#define ADDRF_SYSTEMPARAM 	0x80180

// 货道参数设置,最大64道设计,总共320bytes,占用三个扇区
//开始地址:ADDRF_SYSTEMPARAM+0x00100
#define ADDRF_GOODSWAYS_SET  0x80280
//空闲一个页面不用0x80400-80480
//交易统计参数，总计22bytes,分配一个扇区的空间，即128byte = 0x80
//开始地址:ADDRF_GOODSWAYS_SET + 0x00180
//#define ADDRF_TRADECOUNTER  	0x80400 // +0x80
#define ADDRF_TRADECOUNTER  	0x80500
//作备份页面
#define ADDRF_TRADECOUNTERCOPY 	0x80600

//货道交易统计参数,每货道交易记录长5byte,
//最大64道设计，共用320byte,分配三个扇区的空间，即384byte = 0x180
//开始地址:ADDRF_TRADECOUNTER + 0x00080
//#define ADDRF_WAYSTARDE_LOG		0x80480 //+180

//层架设置参数,每层交易记录长3byte,
//最大8层设计，共用24byte,分配二个扇区的空间，即256byte = 0x100
//中间跳过两个扇区
#define ADDRF_RACK_SET		0x80700 //+0x100

//各层货道参数设置,每货道参数设置长度2byte,
//最大64道设计，共用128byte,分配二个扇区的空间，即256byte = 0x100
#define ADDRF_RACK_COLUMN_SET	0x80800 //+100

//----------------------------------------------------------------------------------
#define ADDRF_GOODSPAR_SET      0x81000
//==================================================================================

////////////////////////////////////////////////////////////////////////////////////

#ifdef ZHK_INTERRUPT
// 片选控制线现场保存处理
#define ZhkLoadChipSelect() _pop_( P1 )
// 只能用宏函数 
#define ZhkSaveChipSelect() _push_ ( P1 )
// 只能用宏函数 
#endif


#define ZhkHardNullSelect()     	P1   = 0x04
#define ZhkHardSramSelect()  		P1   = 0x08
#define ZhkHardFlashSelect()  		P1   = 0x09
#define ZhkHardFlash1Select()   	P1   = 0x0A
#define ZhkHardDs12887Select()  	P1   = 0x0B
#define ZhkHardKeyBoardLine1Select()  	P1   = 0x0E
#define ZhkHardKeyBoardLine2Select()  	P1   = 0x0F
#define ZhkHardIOInputSelect()  	P1   = 0x0D
#define ZhkHardIOOutputSelect()  	P1   = 0x14

#define ZhkHardSetRTS() 		PinRTS = 0
#define ZhkHardClrRTS()  		PinRTS = 1
#define ZhkHardIsCTS()     		~ PinCTS

extern void ZhkHardSerialSwitch( unsigned char Route );
extern void ZhkHardBuzzerOn( void );
extern void ZhkHardBuzzerOff( void );
extern void ZhkHardCoinerDisable( void );
extern void ZhkHardCoinerEnable( void );
extern void ZhkHardStatusOn( void );
//extern void ZhkHardStatusOff( void );
extern void ZhkHardPowerOn( void );
extern void ZhkHardPowerOff( void );
extern void WaterKeyEnable( void );
extern void WaterKeyDisable( void );
extern void GameLedOn( void );
extern void GameLedOff( void );
extern void WatchDogInit( void );
extern void WatchDogDisable( void );
extern void FeedWatchDog( void );

#endif