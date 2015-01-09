#define TIMER_C
#define ZHK_INTERRUPT
#include "device.h"
#include "timer.h"
#include "global.h"
//#include "IOInput.h"
//#include "VMC_PC.h"

extern unsigned char VPMission_Button_RPT( unsigned char type, unsigned char value );
extern bit IfGameKeyOn( void );
extern void ZhkKeyScanIsr( void );
extern void ZhkIOInputIsr( void );

#ifdef _COIN_INPUT_PARALLEL
	extern void ZyzChkCoinChnSta( void ); 
#else
	extern void ZhkChkCoinInIsr();
#endif


uchar xdata Tick;
uchar xdata Second;

char xdata lifterTimer;
void ZhkTimerInit( void )
{
	TMOD &= 0xF0;
	TMOD |= 0x01;
	TL0   = ( uint )( 0x10000 + 16 - 10.0 * 1000 * OSCRATE / 1000000 / MACHINECYCLE / 12 ) % 256;
	TH0   = ( uint )( 0x10000 + 16 - 10.0 * 1000 * OSCRATE / 1000000 / MACHINECYCLE / 12 ) / 256;
	TR0   = 1;
	ET0   = 1;
	SgpSecTimer = 0;
	SgwSecTimer = 0;
	Tick = 0;
	Second = 0;
	YgpSecTimer = 0;
	LzjSecTimer = 0;
	ZhkDownMsgAckTimer = 0;
	ZhkBuzzerTimer = 0;
	QueryCashTimer = 0;
	ZhkMobileSecTimer = 0;
	AlarmSpacing = 0;
	GPRSModuleResetTimer = 0;
	SendSMSTimer = 0;
	ReceiveSMSTimer = 0;
	QueryTimer = 0;
	sysVPMission.watchDogCtr = 0;
    sysVPMission.watchDogTimer = 0;
    sysVPMission.watchDogIng = 0;
    sysVPMission.hopperOutTimer = 0;
}
void Zhktmr0( void ) interrupt 1 using 1 
{
	ZhkSaveChipSelect();
	ZhkHardSramSelect();
	TL0   = ( uint )( 0x10000 + 16 - 10.0 * 1000 * OSCRATE / 1000000 / MACHINECYCLE ) % 256;
	TH0   = ( uint )( 0x10000 + 16 - 10.0 * 1000 * OSCRATE / 1000000 / MACHINECYCLE ) / 256;
	
	Tick ++ ;
	if ( Tick % 5 == 0 ) 
		ZhkKeyScanIsr();
	// 时钟周期(10ms)延时区
	if ( LzjMSecTimer > 10 ) 
		LzjMSecTimer -= 10;
	else
		LzjMSecTimer = 0;
//	if ( SerialAckTimer ) SerialAckTimer --;
	ZhkSchedulerClock ++;
	if ( ZhkDownMsgAckTimer ) 
		ZhkDownMsgAckTimer --;
	if(lifterTimer)
		lifterTimer--;
	if ( ZhkBuzzerTimer > 0 ) 
		ZhkBuzzerTimer --;
	else
	{
		ZhkHardBuzzerOff();
	}
	
    if( sysMDBMission.MDBMSTimer > 0 )
        sysMDBMission.MDBMSTimer--;

	//-------------------------------------------
	if( sysVPMission.msTimer1 > 0 )
		sysVPMission.msTimer1--;
	if( sysVPMission.msTimer2 > 0 )
	    sysVPMission.msTimer2--;
    if( sysVPMission.msTimer3 > 0 )
	    sysVPMission.msTimer3--;    
    if( sysVPMission.msGameTimer1 > 0 )
        sysVPMission.msGameTimer1 --;
    if( sysITLMission.msPoll > 0 )
        sysITLMission.msPoll --;
    if( sysITLMission.msTimer2 > 0 )
        sysITLMission.msTimer2 --;
    if( sysBCMission.msPoll > 0 )
        sysBCMission.msPoll --;
    if( sysBCMission.msTimer2 > 0 )
        sysBCMission.msTimer2 --;
	if(sysVPMission.msActTimer)
		sysVPMission.msActTimer--;
    sysVPMission.dspTimer1 ++; 
	//sysVPMission.sWeihuTimer++;
    //===========================================

	if ( Tick >= 100 )
	{
		Tick = 0;
		Second ++;
		//---------------------------------------
        //watch dog
        if( sysVPMission.watchDogCtr == 1 )
        {
        	if( sysVPMission.watchDogTimer >= WATCHDOGTIMEOUT )
            {
			    if( sysVPMission.watchDogIng == 0 )
				{
            	    //CMOD |= 0x40;
                    //WDCON  = 0x07; 
					WDTRST = 0x1E;
					WDTRST = 0xe1;
				}
				else
				{
					sysVPMission.watchDogIng = 1;
				}
            }
            else
            {
                sysVPMission.watchDogTimer++;
            }
        }
        //=======================================
        if( sysVPMission.menuTimer )
            sysVPMission.menuTimer--;
		if( SgpSecTimer ) 
			SgpSecTimer --;
		if( SgwSecTimer )
			SgwSecTimer--;
		if( YgpSecTimer ) 
			YgpSecTimer --;
		if( LzjSecTimer ) 
			LzjSecTimer --;
		if(( QueryCashTimer > 0 ) && ( QueryCashTimer < 240 ) )
			QueryCashTimer--;
		if ( ZhkMobileSecTimer ) 
				ZhkMobileSecTimer --;
      //if ( g_byCommTimeOutSeconds ) g_byCommTimeOutSeconds --;
		
		if( SendSMSTimer )
			SendSMSTimer --;		
		if( ReceiveSMSTimer )
			ReceiveSMSTimer --;
		if( QueryTimer )
			QueryTimer--;        
        

        //sysVPMission.sTimerHopperEmp++;//20110502

        //----------------------------------------------------------
        if( ( sysVPMission.sTimer1 > 0 )&&(sysVPMission.sTimer1 < 255) )
        	sysVPMission.sTimer1--;
        if( sysVPMission.sTimer2 > 0 )
            sysVPMission.sTimer2--;
        if( sysVPMission.billSTimer > 0 )
            sysVPMission.billSTimer--;
        if( sysVPMission.hopperTimer > 0 )
		    sysVPMission.hopperTimer--;
		if( sysVPMission.hopperOutTimer ) 
			sysVPMission.hopperOutTimer --;
		if(sysVPMission.sTimerHeart)
			sysVPMission.sTimerHeart--;
		//if(sysVPMission.sWeihuTimer)
		//	sysVPMission.sWeihuTimer--;

		if( sysVPMission.VPDevCtr & 0x01 )
		{
            sysVPMission.sTimerBC++;
  	    }

        //if( sysVPMission.VPDevCtr & 0x02 )
		{
            sysVPMission.sTimerChanger1++;
  	    }
        
        if( sysVPMission.VPDevCtr & 0x04 )
		{
            sysVPMission.sTimerChanger2++;
  	    }
        
        if( sysVPMission.VPDevCtr & 0x08 )
		{
            sysVPMission.sTimerChanger3++;
  	    }
        
        if( sysVPMission.VPDevCtr & 0x10 )
		{
            sysVPMission.sTimerGOC++;
  	    }

        sysVPMission.sTimertubeRemoved++;//币管没有关紧检测;by gzz 20110827 

		if( sysVPMission.sTimerTubeUpdate > 0 )
            sysVPMission.sTimerTubeUpdate --; 

        if( sysITLMission.sTimer1 > 0 )
            sysITLMission.sTimer1 --; 

        if( sysITLMission.sTimer2 > 0 )
            sysITLMission.sTimer2 --;

        if( sysITLMission.reEnableFlag == 1 )
        {
            sysITLMission.reEnableTime ++;
        }

        if( sysITLMission.billJamedFlag == 1 )
        {
        	sysITLMission.billJamedTime ++;
        }
        
        if( sysITLMission.restartTime )
        {
            sysITLMission.restartTime--;
        }
        //==========================================================


		// 秒级延时区域
		if ( Second >= 60 )
		{
			Second = 0;
			if( AlarmSpacing < 255 )
				AlarmSpacing ++ ;
			else
				AlarmSpacing = 0;				
			// 分钟级延时区域
		//	if ( g_byCommHeartBeatMins ) g_byCommHeartBeatMins --;		
		}		
		if( GPRSModuleResetTimer > 0 )
			GPRSModuleResetTimer--;
	}
    //#ifndef _DEBUG_MOBILE_TEST
	ZhkIOInputIsr();

#ifdef _COIN_INPUT_PARALLEL
	ZyzChkCoinChnSta();
#else
	ZhkChkCoinInIsr();
#endif
    //----------------------------------------------------------
	/*
    if( ( IfGameKeyOn() )&&(sysVPMission.comLock==0) )
	{
		VPMission_Button_RPT( 0, 0 );
	    //Beep();
	}
	*/
    //==========================================================
 
    //if ( Tick % 5 == 0 ) ZhkKeyScanIsr();
    //#endif	// _DEBUG_MOBILE_TEST	
	ZhkLoadChipSelect();
}