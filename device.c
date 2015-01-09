#define DEVICE_C
#include "device.h"
#include "debug.h"
#include "global.h"

uchar data ZhkHardIOOutputReg;		// IOÊä³ö×´Ì¬´æ´¢Æ÷

#define ZhkHardOutputLock() 	ZhkHardSramSelect()	

void ZhkHardSerialSwitch( uchar Route )
{
//	uchar data P0State;
	ZhkHardSramSelect();
	Route              &= 0x07;
	ZhkHardIOOutputReg &= 0xF8;
	ZhkHardIOOutputReg |= Route;
	EA = 0;
	ZhkHardNullSelect();
	P0 = ZhkHardIOOutputReg;
	ZhkHardIOOutputSelect();
//	P0State = P0;
	_nop_();
	_nop_();
	ZhkHardOutputLock();
	EA = 1;
//	Trace( "ZhkHardIOOutputReg=%bxH\n" , ZhkHardIOOutputReg );
//	Trace( "P0State=%bxH\n" , P0State );
}
void ZhkHardBuzzerOn( void )
{
	ZhkHardSramSelect();
	EA = 0;
	ZhkHardIOOutputReg &= ~ BitBuzzer;
	ZhkHardNullSelect();
	P0 = ZhkHardIOOutputReg;
	ZhkHardIOOutputSelect();
	ZhkHardOutputLock();
	EA = 1;
//	Trace( "ZhkHardIOOutputReg=%bxH\n" , ZhkHardIOOutputReg );
//	Trace( "P0State=%bxH\n" , P0State );
	
}
void ZhkHardBuzzerOff( void )
{
	ZhkHardSramSelect();
	EA = 0;
	ZhkHardIOOutputReg |= BitBuzzer;
	ZhkHardNullSelect();
	P0 = ZhkHardIOOutputReg;
	ZhkHardIOOutputSelect();
	ZhkHardOutputLock();
	EA = 1;
//	Trace( "ZhkHardIOOutputReg=%bxH\n" , ZhkHardIOOutputReg );
//	Trace( "P0State=%bxH\n" , P0State );
}
void ZhkHardCoinerDisable( void )
{
	ZhkHardSramSelect();
	EA = 0;
	ZhkHardIOOutputReg &= ~ BitCoinerEn;
	ZhkHardNullSelect();
	P0 = ZhkHardIOOutputReg;
	ZhkHardIOOutputSelect();
	ZhkHardOutputLock();
	EA = 1;
//	Trace( "ZhkHardIOOutputReg=%bxH\n" , ZhkHardIOOutputReg );
//	Trace( "P0State=%bxH\n" , P0State );
	
}
void ZhkHardCoinerEnable( void )
{
	ZhkHardSramSelect();
	EA = 0;
	ZhkHardIOOutputReg |= BitCoinerEn;
	ZhkHardNullSelect();
	P0 = ZhkHardIOOutputReg;
	ZhkHardIOOutputSelect();
	ZhkHardOutputLock();
	EA = 1;
//	Trace( "ZhkHardIOOutputReg=%bxH\n" , ZhkHardIOOutputReg );
//	Trace( "P0State=%bxH\n" , P0State );
}
void ZhkHardStatusOn( void )
{
	ZhkHardSramSelect();
	EA = 0;
	ZhkHardIOOutputReg &= ~ BitStatus;
	ZhkHardNullSelect();
	P0 = ZhkHardIOOutputReg;
	ZhkHardIOOutputSelect();
	ZhkHardOutputLock();
	EA = 1;
//	Trace( "ZhkHardIOOutputReg=%bxH\n" , ZhkHardIOOutputReg );
//	Trace( "P0State=%bxH\n" , P0State );
	
}


void ZhkHardPowerOn( void )
{
	ZhkHardSramSelect();
	EA = 0;
	ZhkHardIOOutputReg &= ~ BitPowerCtrl;
	ZhkHardNullSelect();
	P0 = ZhkHardIOOutputReg;
	ZhkHardIOOutputSelect();
	ZhkHardOutputLock();
	EA = 1;
  //Trace( "ZhkHardIOOutputReg=%bxH\n" , ZhkHardIOOutputReg );
  //Trace( "P0State=%bxH\n" , P0State );

}

void ZhkHardPowerOff( void )
{
	ZhkHardSramSelect();
	EA = 0;
	ZhkHardIOOutputReg |= BitPowerCtrl;
	ZhkHardNullSelect();
	P0 = ZhkHardIOOutputReg;
	ZhkHardIOOutputSelect();
	ZhkHardOutputLock();
	EA = 1;
  //Trace( "ZhkHardIOOutputReg=%bxH\n" , ZhkHardIOOutputReg );
  //Trace( "P0State=%bxH\n" , P0State );
}

void WaterKeyEnable( void )
{
	INT0 = 0;
}


void WaterKeyDisable( void )
{
	INT0 = 1;
    //P3 |= 0x04;
}


void GameLedOn( void )
{
	ZhkHardSramSelect();
	EA = 0;
	ZhkHardIOOutputReg |= BitPowerCtrl;
	ZhkHardNullSelect();
	P0 = ZhkHardIOOutputReg;
	ZhkHardIOOutputSelect();
	ZhkHardOutputLock();
	EA = 1;
    //Trace( "ZhkHardIOOutputReg=%bxH\n" , ZhkHardIOOutputReg );
    //Trace( "P0State=%bxH\n" , P0State );
}


void GameLedOff( void )
{
	ZhkHardSramSelect();
	EA = 0;
	ZhkHardIOOutputReg &= ~BitPowerCtrl;
	ZhkHardNullSelect();
	P0 = ZhkHardIOOutputReg;
	ZhkHardIOOutputSelect();
	ZhkHardOutputLock();
	EA = 1;
    //Trace( "ZhkHardIOOutputReg=%bxH\n" , ZhkHardIOOutputReg );
    //Trace( "P0State=%bxH\n" , P0State );
}

void WatchDogInit( void )
{
    /*
	CCAPM4 = 0x4c;
    CCAP4L = 0xff;
    CCAP4H = 0xff;
    //CMOD  |= 0x40;
	CMOD  &= 0xb0;
    */
	sysVPMission.watchDogTimer = 0;
	sysVPMission.watchDogCtr   = 1;
	sysVPMission.watchDogIng   = 0;
}

void WatchDogDisable( void )
{
    /*
	CCAPM4 = 0x4c;
    CCAP4L = 0xff;
    CCAP4H = 0xff;
    //CMOD  |= 0x40;
	CMOD  &= 0xb0;
    */
	sysVPMission.watchDogTimer = 0;
	sysVPMission.watchDogCtr   = 0;
	sysVPMission.watchDogIng   = 0;
}

void FeedWatchDog( void )
{
    
    if( sysVPMission.watchDogCtr == 1 )
    {
    	sysVPMission.watchDogTimer = 0;
    }

}

