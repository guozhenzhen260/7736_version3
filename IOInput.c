#define IOINPUT_C
#include "device.h"
#include "IOInput.h"
#include "timer.h"
#include "common.h"
#include "global.h"
#include "STRING.H"
#include "VMC_PC.h"
//#include "CommonFunction.h"


#ifndef _COIN_INPUT_PARALLEL
	struct ZHKCOINER
	{
		uchar State;			// 硬币器状态 bit6 = 故障 bit5 = 使能
		uchar Count; 			// 当前接收到硬币的数量
	};
#else 	
	struct ZHKCOINER
    {
	    uchar State;			// 硬币器状态 bit6 = 故障 bit5 = 使能
	    uchar Count; 			// 当前接收到硬币的数量        
        uchar CountChn1;
        uchar CountChn2;
        uchar CountChn3;
        uchar CountChn4;
        uchar CountChn5;
        uchar CountChn6;
    };  	
	struct COINAPCCEPTOR xdata CoinSet;
#endif

uchar xdata ZhkModeState;		// 0 表示模式开关处于关闭状态
						// 1 表示模式开关处于打开状态

uchar xdata ZhkPowerState;		// 0 表示系统处于正常供电状态
						// 1 表示系统处于电池供电状态

uchar xdata HardShakeAlarmState;// 0 表示系统有震动触发
                                // 1 表示系统无震动触发
                                // Added on 2006.06.02

struct ZHKCOINER xdata ZhkCoiner;			


bit IsModeOff( void )
{
	#ifdef   _SHUPING_
		return ZhkModeState == 1;
	#else
		return ZhkModeState == 0;
	#endif
}


//Power off or game key
bit IsPowerOff( void )
{
	return ZhkPowerState == 1;
}

//Return key
bit IfShakeAlarm( void )                // Added on 2006.06.02  
{
 	return HardShakeAlarmState == 1;    //return "1", identify "Shake alrm" !
}

bit IfGameKeyOn( void )
{
    if( sysVPMission.gameKey == 1 )
	{
		sysVPMission.gameKey = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}

bit IfReturnKeyOn( void )
{
    uchar xdata flag = 0;
    /*
    if( sysVPMission.returnKey == 1 )
	{
		sysVPMission.returnKey = 0;
		return 1;
	}
	else
	{
		return 0;
	}
    */   
	
    if( sysVPMission.returnKey == 1 )
    {
        sysVPMission.returnKey = 0;
    	flag = 1;
    }
    if( sysMDBMission.returnKeyFlag == 1 )
    {
    	sysMDBMission.returnKeyFlag = 0;
    	flag = 1;
    }
	if( sysMDBMission.returnCmdFlag == 1 )
    {
    	sysMDBMission.returnCmdFlag = 0;
    	flag = 2;
    }
	
    if( flag == 1 ) 
    {
    	VPMission_Button_RPT( VP_BUT_RETURN, VP_BUT_RETURNNO );
		//DisplayStr( 0, 0, 1, "1", 1 );  
		//WaitForWork(2000,NULL);
    	return 1;
    }
	else if( flag == 2 ) 
    {
    	return 1;
    }
    else
    {
        return 0;
    }       
}

/*
void AlertOn( void )
{
	ZhkBuzzerTimer = -1;
	ZhkHardBuzzerOn();	
}
*/
void Beep( void )
{
	ZhkBuzzerTimer = 20;
	ZhkHardBuzzerOn();
}

void ZhkIOInputIsr( void ) using 1
{

	#define HardIsPowerOff()     !( IOState & BitPowerOff )
	#define HardIsModeSwitchOn() !( IOState & BitModeSw )
	#define HardIsAlertOn()      !( IOState & BitAlert )
    
	#define PWRCHKTIMES   1   //2,1
	#define ALARMCHKTIMES 2   //
    #define ISPCHKTIMES   10

    static uchar xdata ModeSwitchTimer=0,PowerTimer=0,ShakeAlarmTimer=0;
    //static bit ModeTemp=0,PowerTemp=0,ShakeAlarmFlag=0;

	//static uchar xdata ModeSwitchTimer;
	static bit ModeTemp = 0;

	uchar data IOState;
	
	
	ZhkHardIOInputSelect();
	_nop_();
	_nop_();
	_nop_();
	IOState = P0;
	ZhkHardSramSelect();	
	
	//掉电检测 		
    if( (HardIsPowerOff())/*&&(sysVPMission.gameKeyLock==0)*/ )
	{
	    if( sysVPMission.gameKeyLock==0 )
		{
			PowerTimer++;
			if( PowerTimer >= PWRCHKTIMES )
			{
				PowerTimer = 0;
				sysVPMission.gameKey = 1;
				sysVPMission.gameKeyLock = 1;
			}
		}
	}
	else
	{
		sysVPMission.gameKeyLock = 0;
	}
    
	//震动信号检测 & Return key  
    if( (HardAlarmCheck())/*&&(sysVPMission.returnKeyLock==0)*/ )
	{
	    if( (sysVPMission.returnKeyLock==0) )
		{
			ShakeAlarmTimer++;
			if( ShakeAlarmTimer >= ALARMCHKTIMES )
			{
				ShakeAlarmTimer = 0;
				sysVPMission.returnKey = 1;
				sysVPMission.returnKeyLock = 1;
			}
		}
	}
	else
	{
		sysVPMission.returnKeyLock = 0;
	}

	//模式开关检测
	if ( HardIsModeSwitchOn() )
	{
		if ( ModeTemp )
		{
			if ( ModeSwitchTimer )
				ModeSwitchTimer --;
			else
				ZhkModeState = 1;
		}
		else
		{
			ModeTemp = 1;
			ModeSwitchTimer = ISPCHKTIMES;
		}
	}
	else
	{
		ZhkModeState = 0;
		ModeTemp     = 0;
	}

}


bit CoinerEnable( void )
{
	ZhkCoiner.State &= ~ 0x20;
	ZhkHardCoinerDisable();	
#ifdef  _COIN_INPUT_PARALLEL
	CoinSet.Line1 = 50 ;
	CoinSet.Line2 = 50 ;
	CoinSet.Line3 = 100 ;
	CoinSet.Line4 = 100 ;	
	CoinSet.Line5 = 0 ;
	CoinSet.Line6 = 0 ;		
#endif
	return 1;
}
bit CoinerDisable( void )
{
	ZhkCoiner.State |= 0x20;
	ZhkHardCoinerEnable();
//	ZhkCoiner.State &= ~ 0x20;
//	ZhkHardCoinerDisable();
	return 1;
}
bit IsCoinerException( void )
{
	return ZhkCoiner.State & 0x40;
}

#ifndef _COIN_INPUT_PARALLEL
	void ZhkIOInit( void )
	{	
	    PinCoinIn = 1;
        PinAlarmSIn = 1;
		ZhkHardPowerOff();
		ZhkHardBuzzerOff();
		ZhkHardStatusOn();
		ZhkHardSramSelect();
		DelayMs( 250 );
		DelayMs( 250 );
	//	DelayMs( 250 );
	//	DelayMs( 250 );
		ZhkHardPowerOn();
		ZhkHardSramSelect();
		DelayMs( 250 );
		DelayMs( 250 );
		DelayMs( 250 );
		DelayMs( 250 );	
		ZhkCoiner.Count = 0;
		ZhkModeState    = 0;
		ZhkPowerState   = 0;
		ZhkBuzzerTimer  = 0;
	}
	
	void ZhkChkCoinInIsr( void ) using 1
	{
		static uchar xdata PinLowCount;
		static bit PinLow,PinPulse;
		
		if ( HardIsCoinIn() )
		{
			if ( PinLow )
			{
				if ( PinLowCount < 250 ) PinLowCount ++;
				if ( PinLowCount >= 3 ) PinPulse = 1;
			}
			else
			{
				PinLow   = 1;
				PinPulse = 0;
				PinLowCount = 0;
			}
			
		}
		else
		{
			if ( PinLow )
			{
				PinLowCount = 0;
				PinLow      = 0;
			}	
			else
			{
				PinLowCount ++;
				if ( PinLowCount >= 3 && PinPulse )
				{
					ZhkCoiner.Count ++;
					PinPulse = 0;
					PinLowCount = 0;
				}
			}
		}
		if ( PinLowCount >= 250 ) 
			ZhkCoiner.State |= 1 << 6;
		else
			ZhkCoiner.State &= ~( 1 << 6 );
	}
	
	uint GetCoin( void )
	{
		uint data ch;
		
		EA = 0;
		//ch = ZhkCoiner.Count;
        ch = ZhkCoiner.Count * SystemParameter.coinValue[0];   // In single line puluse mode, just use SystemParameter.coinValue[0].
		ZhkCoiner.Count = 0;
		EA = 1;
		return ch;
	}

#else
	
	void ZhkIOInit( void )
	{	
	    PinCoinIn = 1;
        PinAlarmSIn = 1;
		ZhkHardPowerOff();
		ZhkHardBuzzerOff();
		ZhkHardStatusOn();
		ZhkHardCoinerEnable();
		ZhkHardSramSelect();
	
		DelayMs( 250 );
		DelayMs( 250 );
	//	DelayMs( 250 );
	//	DelayMs( 250 );
		ZhkHardPowerOn();
		ZhkHardSramSelect();
		DelayMs( 250 );
		DelayMs( 250 );
		DelayMs( 250 );
		DelayMs( 250 );
	//	ZhkCoiner.State = 0;
		ZhkCoiner.Count = 0;
		ZhkCoiner.CountChn1 = 0;
		ZhkCoiner.CountChn2 = 0;
		ZhkCoiner.CountChn3 = 0;
		ZhkCoiner.CountChn4 = 0;
		ZhkCoiner.CountChn5 = 0;
		ZhkCoiner.CountChn6 = 0;
		///硬币器通道面值
		memset( &CoinSet, 0, sizeof(struct COINAPCCEPTOR));
		ZhkModeState    = 0;
		ZhkPowerState   = 0;
		ZhkBuzzerTimer  = 0;
	}
	
	void ScanSPCoin( void )
	{
		static uchar xdata PinLowCount;
		static bit PinLow,PinPulse;
        #define CoinTimes_SP  2

		if( HardIsCoinIn() )
		{
			if ( PinLow )
			{
				if ( PinLowCount < 250 ) PinLowCount ++;
				if ( PinLowCount >= CoinTimes_SP ) PinPulse = 1;
			}
			else
			{
				PinLow   = 1;
				PinPulse = 0;
				PinLowCount = 0;
			}
			
		}
		else
		{
			if ( PinLow )
			{
				PinLowCount = 0;
				PinLow      = 0;
			}	
			else
			{
				PinLowCount ++;
				if( (PinLowCount >= CoinTimes_SP)&&(PinPulse) )
				{
					ZhkCoiner.Count ++;
					PinPulse = 0;
					PinLowCount = 0;
				}
			}
		}
		if( PinLowCount >= 250 ) 
			ZhkCoiner.State |= 1 << 6;
		else
			ZhkCoiner.State &= ~( 1 << 6 );
	}

	void ZyzChkCoinChnSta( void ) using 1
	{
		//------------------------------------------------------
	    //Added by Andy on 20071027 for test02 .
	    #define HardIsCoinChn1In()   !( CountIOState & BitCoinChn1 )
		#define HardIsCoinChn2In()   !( CountIOState & BitCoinChn2 )
	    #define HardIsCoinChn3In()   !( CountIOState & BitCoinChn3 )
		#define HardIsCoinChn4In()   !( CountIOState & BitCoinChn4 )
	    #define HardIsCoinChn5In()   !( CountIOState & BitCoinChn5 )
		#define HardIsCoinChn6In()   !( CountIOState & BitCoinChn6 )
	    #define CoinTimes  2
	    static uchar xdata PinLowCount1 = 0;
		static uchar xdata PinLowCount2 = 0;
		static uchar xdata PinLowCount3 = 0;
		static uchar xdata PinLowCount4 = 0;
		static uchar xdata PinLowCount5 = 0;
		static uchar xdata PinLowCount6 = 0;
		static bit PinLow1   = 0;
		static bit PinLow2   = 0;
		static bit PinLow3   = 0;
		static bit PinLow4   = 0;
		static bit PinLow5   = 0;
		static bit PinLow6   = 0;
	    static bit PinPulse1 = 0;
	    static bit PinPulse2 = 0;
		static bit PinPulse3 = 0;
	    static bit PinPulse4 = 0;
	    static bit PinPulse5 = 0;
	  	static bit PinPulse6 = 0;
	  //static bit PowerOn1MinFlag = 0;
	    static uchar xdata CurrentChannel = 0; 
		uchar data CountIOState;

		/*
	    if( (PowerOn1MinFlag == 0) && ( ColumnTestTimer != 0 ) )
		{
		    return;
		}
		else
	    {
		    PowerOn1MinFlag = 1;
		}
	    */
        ScanSPCoin();  //Scan the single pulse interface coin acceptor...
		ZhkHardIOInputSelect();
		_nop_();
		_nop_();
		_nop_();
		CountIOState = P0;
		ZhkHardSramSelect();
	    if( !( CountIOState & 0x04 ) )
	    {
		    //CurrentChannel |= 0x01;
			CurrentChannel = 0x01;
		}
		else if( !( CountIOState & 0x08 ) )
		{
		    CurrentChannel = 0x02;
		}
		else if( !( CountIOState & 0x10 ) )
		{
		    CurrentChannel = 0x04;
		}
		else if( !( CountIOState & 0x20 ) )
		{
		    CurrentChannel = 0x08;
		}
		else if( !( CountIOState & 0x40 ) )
		{
		    CurrentChannel = 0x10;
		}
		else if( !( CountIOState & 0x80 ) )
		{
	    	CurrentChannel = 0x20;
		}

		//******************************************************************
		//Chn1 
        if( HardIsCoinChn1In() )   //Just for coin hopper full sign!
		{
			if( PinLow1 )
			{
				if( PinLowCount1 < 255 ) PinLowCount1 ++;
				if( PinLowCount1 >= 250 )  
                {
                	PinPulse1 = 1;
                    ZhkCoiner.CountChn1 = 1;
                }
			}
			else
			{
				PinLow1      = 1;
				PinPulse1    = 0;
				PinLowCount1 = 0;
			}
			
		}
		else
		{
		    PinLow1      = 0;
			PinPulse1    = 0;
			PinLowCount1 = 0;
	        ZhkCoiner.CountChn1 = 0;
		}

        /*
        if( CurrentChannel & 0x01 )
		{
			if( HardIsCoinChn1In() )
			{
				if ( PinLow1 )
				{
					if ( PinLowCount1 < 250 ) PinLowCount1 ++;
					if ( PinLowCount1 >= CoinTimes )  PinPulse1 = 1;
				}
				else
				{
					PinLow1      = 1;
					PinPulse1    = 0;
					PinLowCount1 = 0;
				}
				
			}
			else
			{
				if ( PinLow1 )
				{
					PinLowCount1 = 0;
					PinLow1      = 0;
				}	
				else
				{
					PinLowCount1 ++;
					if ( PinLowCount1 >= CoinTimes && PinPulse1 )
					{
						ZhkCoiner.CountChn1 ++;
						PinPulse1 = 0;
						PinLowCount1 = 0;
	                    CurrentChannel &= 0xfe;
					}
				}
			}
			if ( PinLowCount1 >= 250 ) 
				ZhkCoiner.State |= 1 << 6;
			else
				ZhkCoiner.State &= ~( 1 << 6 );    
		}
		else
 	    */
		if( CurrentChannel & 0x02 )
		{
			if( HardIsCoinChn2In() )
			{
				if ( PinLow2 )
				{
					if ( PinLowCount2 < 250 ) PinLowCount2 ++;
					if ( PinLowCount2 >= CoinTimes )  PinPulse2 = 1;
				}
				else
				{
					PinLow2      = 1;
					PinPulse2    = 0;
					PinLowCount2 = 0;
				}
				
			}
			else
			{
				if ( PinLow2 )
				{
					PinLowCount2 = 0;
					PinLow2      = 0;
				}	
				else
				{
					PinLowCount2 ++;
					if ( PinLowCount2 >= CoinTimes && PinPulse2 )
					{
						ZhkCoiner.CountChn2 ++;
						PinPulse2 = 0;
						PinLowCount2 = 0;
	                    CurrentChannel &= 0xfd;
					}
				}
			}
			if ( PinLowCount2 >= 250 ) 
				ZhkCoiner.State |= 1 << 6;
			else
				ZhkCoiner.State &= ~( 1 << 6 );    
		}
	    else if( CurrentChannel & 0x04 )
		{
			if( HardIsCoinChn3In() )
			{
				if ( PinLow3 )
				{
					if ( PinLowCount3 < 250 ) PinLowCount3 ++;
					if ( PinLowCount3 >= CoinTimes )  PinPulse3 = 1;
				}
				else
				{
					PinLow3      = 1;
					PinPulse3    = 0;
					PinLowCount3 = 0;
				}
				
			}
			else
			{
				if ( PinLow3 )
				{
					PinLowCount3 = 0;
					PinLow3      = 0;
				}	
				else
				{
					PinLowCount3 ++;
					if ( PinLowCount3 >= CoinTimes && PinPulse3 )
					{
						ZhkCoiner.CountChn3 ++;
						PinPulse3 = 0;
						PinLowCount3 = 0;
	                    CurrentChannel &= 0xfb;
					}
				}
			}
			if ( PinLowCount3 >= 250 ) 
				ZhkCoiner.State |= 1 << 6;
			else
				ZhkCoiner.State &= ~( 1 << 6 );    
		}
	    else if( CurrentChannel & 0x08 )
		{
			if( HardIsCoinChn4In() )
			{
				if ( PinLow4 )
				{
					if ( PinLowCount4 < 250 ) PinLowCount4 ++;
					if ( PinLowCount4 >= CoinTimes )  PinPulse4 = 1;
				}
				else
				{
					PinLow4      = 1;
					PinPulse4    = 0;
					PinLowCount4 = 0;
				}
				
			}
			else
			{
				if ( PinLow4 )
				{
					PinLowCount4 = 0;
					PinLow4      = 0;
				}	
				else
				{
					PinLowCount4 ++;
					if ( PinLowCount4 >= CoinTimes && PinPulse4 )
					{
						ZhkCoiner.CountChn4 ++;
						PinPulse4 = 0;
						PinLowCount4 = 0;
	                    CurrentChannel &= 0xf7;
					}
				}
			}
			if ( PinLowCount4 >= 250 ) 
				ZhkCoiner.State |= 1 << 6;
			else
				ZhkCoiner.State &= ~( 1 << 6 );    
		}
	    else if( CurrentChannel & 0x10 )
		{
			if( HardIsCoinChn5In() )
			{
				if ( PinLow5 )
				{
					if ( PinLowCount5 < 250 ) PinLowCount5 ++;
					if ( PinLowCount5 >= CoinTimes )  PinPulse5 = 1;
				}
				else
				{
					PinLow5      = 1;
					PinPulse5    = 0;
					PinLowCount5 = 0;
				}
				
			}
			else
			{
				if ( PinLow5 )
				{
					PinLowCount5 = 0;
					PinLow5      = 0;
				}	
				else
				{
					PinLowCount5 ++;
					if ( PinLowCount5 >= CoinTimes && PinPulse5 )
					{
						ZhkCoiner.CountChn5 ++;
						PinPulse5 = 0;
						PinLowCount5 = 0;
	                    CurrentChannel &= 0xef;
					}
				}
			}
			if ( PinLowCount5 >= 250 ) 
				ZhkCoiner.State |= 1 << 6;
			else
				ZhkCoiner.State &= ~( 1 << 6 );    
		}
		else if( CurrentChannel & 0x20 )
		{
			if( HardIsCoinChn6In() )
			{
				if ( PinLow6 )
				{
					if ( PinLowCount6 < 250 ) PinLowCount6 ++;
					if ( PinLowCount6 >= CoinTimes )  PinPulse6 = 1;
				}
				else
				{
					PinLow6      = 1;
					PinPulse6    = 0;
					PinLowCount6 = 0;
				}
				
			}
			else
			{
				if ( PinLow6 )
				{
					PinLowCount6 = 0;
					PinLow6      = 0;
				}	
				else
				{
					PinLowCount6 ++;
					if ( PinLowCount6 >= CoinTimes && PinPulse6 )
					{
						ZhkCoiner.CountChn6 ++;
						PinPulse6 = 0;
						PinLowCount6 = 0;
	                    CurrentChannel &= 0xdf;
					}
				}
			}
			if ( PinLowCount6 >= 250 ) 
				ZhkCoiner.State |= 1 << 6;
			else
				ZhkCoiner.State &= ~( 1 << 6 );    
		}
	    
	}
    
	uchar IfCoinHopperFull( void )
	{
		if( ZhkCoiner.CountChn1 >= 1 )
 	    {
			return 1;
		}
		else
		{
			return 0;
		}
	
	}

	uint GetCoin( void )
	{
		uint data ch=0;	
		EA = 0;
		if( ZhkCoiner.Count >= 1 )
		{
		    //ch = ZhkCoiner.Count * 25;
            ch = ZhkCoiner.Count * SystemParameter.coinValue[0];
	    	ZhkCoiner.Count = 0;
		}
        /*
		else if( ZhkCoiner.CountChn1 >= 1 )
		{
		    //ch = ZhkCoiner.CountChn1 * 1;
			//ch = ZhkCoiner.CountChn1 * CoinSet.Line1;
            ch = ZhkCoiner.CountChn1 * SystemParameter.coinValue[0];
			ZhkCoiner.CountChn1 = 0;
		}
	    else if( ZhkCoiner.CountChn2 >= 1 )
		{
		    //ch = ZhkCoiner.CountChn2 * 2;
			//ch = ZhkCoiner.CountChn2 * CoinSet.Line2;
            ch = ZhkCoiner.CountChn2 * SystemParameter.coinValue[1];
			ZhkCoiner.CountChn2 = 0;
		}
		else if( ZhkCoiner.CountChn3 >= 1 )
		{
		    //ch = ZhkCoiner.CountChn3 * 5;
			//ch = ZhkCoiner.CountChn3 * CoinSet.Line3;
            ch = ZhkCoiner.CountChn3 * SystemParameter.coinValue[2];
			ZhkCoiner.CountChn3 = 0;
		}
		else if( ZhkCoiner.CountChn4 >= 1 )
		{
		    //ch = ZhkCoiner.CountChn4 * SystemParameter.Exchange;    //30,
			//ch = ZhkCoiner.CountChn4 * CoinSet.Line4;
            ch = ZhkCoiner.CountChn4 * SystemParameter.coinValue[3];
			ZhkCoiner.CountChn4 = 0;
		}
		else if( ZhkCoiner.CountChn5 >= 1 )
		{
		    //ch = ZhkCoiner.CountChn5 * SystemParameter.Exchange;    //60,
			//ch = ZhkCoiner.CountChn5 * CoinSet.Line5;
            ch = ZhkCoiner.CountChn5 * SystemParameter.coinValue[4];
			ZhkCoiner.CountChn5 = 0;
		}
		else if( ZhkCoiner.CountChn6 >= 1 )
		{
		    //ch = ZhkCoiner.CountChn6 * SystemParameter.Exchange;
			//ch = ZhkCoiner.CountChn6 * CoinSet.Line6;
            ch = ZhkCoiner.CountChn6 * SystemParameter.coinValue[5];
			ZhkCoiner.CountChn6 = 0;
		}
        */
		EA = 1;
		return ch;          
	}
#endif

