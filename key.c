#define KEY_C
#include "device.h"
#include "string.h"
#include "key.h"
#include "IOInput.h"
#include "global.h"
#include "scheduler.h"
#include "CommonFunction.h"
#include "VMC_PC.h"

#define KEYBOARD11 KEY_5
#define KEYBOARD12 KEY_4
#define KEYBOARD13 KEY_8
#define KEYBOARD14 KEY_7

#define KEYBOARD21 KEY_1
#define KEYBOARD22 KEY_2
#define KEYBOARD23 KEY_3
#define KEYBOARD24 KEY_6

#define KEYBOARD31 KEY_DOWN
#define KEYBOARD32 KEY_CANCEL
#define KEYBOARD33 KEY_SUBMIT
#define KEYBOARD34 KEY_BACK

#define KEYBOARD41 KEY_9
#define KEYBOARD42 KEY_DOT
#define KEYBOARD43 KEY_0
#define KEYBOARD44 KEY_UP

//                      0            1            2            3            4            5            6            7            8           9            a            b            c            d           e            f			      	
uchar code Key1[] = { KEYBOARD11 , KEYBOARD11 , KEYBOARD11 , KEYBOARD11 , KEYBOARD11 , KEYBOARD11 , KEYBOARD11 , KEYBOARD11 , KEYBOARD12 , KEYBOARD12 , KEYBOARD12 , KEYBOARD12 , KEYBOARD13 , KEYBOARD13 , KEYBOARD14 , KEY_NULL };
uchar code Key2[] = { KEYBOARD21 , KEYBOARD21 , KEYBOARD21 , KEYBOARD21 , KEYBOARD21 , KEYBOARD21 , KEYBOARD21 , KEYBOARD21 , KEYBOARD22 , KEYBOARD22 , KEYBOARD22 , KEYBOARD22 , KEYBOARD23 , KEYBOARD23 , KEYBOARD24 , KEY_NULL };
uchar code Key3[] = { KEYBOARD31 , KEYBOARD31 , KEYBOARD31 , KEYBOARD31 , KEYBOARD31 , KEYBOARD31 , KEYBOARD31 , KEYBOARD31 , KEYBOARD32 , KEYBOARD32 , KEYBOARD32 , KEYBOARD32 , KEYBOARD33 , KEYBOARD33 , KEYBOARD34 , KEY_NULL };
uchar code Key4[] = { KEYBOARD41 , KEYBOARD41 , KEYBOARD41 , KEYBOARD41 , KEYBOARD41 , KEYBOARD41 , KEYBOARD41 , KEYBOARD41 , KEYBOARD42 , KEYBOARD42 , KEYBOARD42 , KEYBOARD42 , KEYBOARD43 , KEYBOARD43 , KEYBOARD44 , KEY_NULL };

uchar xdata ZhkKeyPermission;
uchar xdata ZhkKeyBuf[ 1 ];
uchar xdata ZhkKeyBufPtr;

void KeyboardInit( void )
{
	//uchar data i;
	memset( ZhkKeyBuf , KEY_NULL , sizeof( ZhkKeyBuf ));
	ZhkKeyPermission = 1;
	ZhkKeyBufPtr = 0;
}
bit KeyLock( void )
{
	ZhkKeyPermission = 0;
	return 1;
}
bit KeyUnLock( void )
{
	ZhkKeyPermission = 1;
	KeyboardInit();
	return 1;
}

uchar GetKey_M2( void )
{
    uchar data i=0;
	uchar data ch;
	uchar data key = KEY_NULL;
    
    //---------------------------------------------------------
    if( IfGameKeyOn() )
	{
		VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
        //Beep();
	}
    //=========================================================
    sysVPMission.VPMode_Key = 1;
    //Scan selection 1
	if( DeviceStatus.Selection[0] == 0 )
	{
		ch = GetSelectionState( 1, &Selection1 );
	    if( ch == 0 )
		{
		    DeviceStatus.Selection[0] &= 0xfe;
	        key = Selection1.ExtState[0];
	        if( key != KEY_NULL )
		    {
			    Beep();
				return key;
			}
		}
		else
		{
			DeviceStatus.Selection[0] |= 0x01;
		}
	}
    //---------------------------------------------------------
    if( IfGameKeyOn() )
	{
		VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
        //Beep();
	}
    //=========================================================
	//Scan selection 2
	if( DeviceStatus.Selection[1] == 0 )
	{
		ch = GetSelectionState( 2, &Selection2 );
	    if( ch == 0 )
		{
		    DeviceStatus.Selection[1] &= 0xfe; 
	        key = Selection2.ExtState[0];
	        if( key != KEY_NULL )
		    {
			    Beep();
				return (key+KEYEXTRAVAL);
			}
		}
		else
		{
			DeviceStatus.Selection[1] |= 0x01;
		}
	}
    //---------------------------------------------------------
    if( IfGameKeyOn() )
	{
		VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
        //Beep();
	}
    //=========================================================
	//Scan selection 3
    if( DeviceStatus.Selection[2] == 0 )
	{
		ch = GetSelectionState( 3, &Selection3 );
	    if( ch == 0 )
		{
		    DeviceStatus.Selection[2] &= 0xfe; 
	        key = Selection3.ExtState[0];
	        if( key != KEY_NULL )
		    {
			    Beep();
				return (key+KEYEXTRAVAL*2);
			}	
		}
		else
		{
			DeviceStatus.Selection[2] |= 0x01;
		}
	}
    sysVPMission.VPMode_Key = 0;
    return key;

}

uchar GetKey( void )
{
	uchar data i;
	uchar data ch;
	uchar data key = KEY_NULL;

    if( sysVPMission.VPMode == 1 )
	{
		#ifdef   _SHUPING_
			if ( ! ZhkKeyPermission ) return KEY_NULL;
			ch = ZhkKeyBuf[ 0 ];
			for( i = 0 ; i < sizeof( ZhkKeyBuf ) - 1 ; i ++ )
				ZhkKeyBuf[ i ] = ZhkKeyBuf[ i + 1 ];
			ZhkKeyBuf[ i ] = KEY_NULL;
			if ( ZhkKeyBufPtr > 0 ) ZhkKeyBufPtr --;
			if ( ch != KEY_NULL ) Beep();
			return ch;
		#else
		    key = GetKey_M2();
	        if( key != KEY_NULL )
	        {
	        	//VPMission_Button_RPT( VP_BUT_GOODS, key+1 );
	        }
	        return key;   //2011.1.21
        #endif
	}
	else
	{
		if ( ! ZhkKeyPermission ) return KEY_NULL;
		ch = ZhkKeyBuf[ 0 ];
		for( i = 0 ; i < sizeof( ZhkKeyBuf ) - 1 ; i ++ )
			ZhkKeyBuf[ i ] = ZhkKeyBuf[ i + 1 ];
		ZhkKeyBuf[ i ] = KEY_NULL;
		if ( ZhkKeyBufPtr > 0 ) ZhkKeyBufPtr --;
		if ( ch != KEY_NULL ) Beep();
		return ch;
	}
}


uchar ScanfKeyFun( void ) 	using	1
{
	uchar data key = KEY_NULL;
	uchar data KeyIOTemp;

	ZhkHardKeyBoardLine1Select();
	P0 = 0xFF;
	_nop_();
	_nop_();
	_nop_();
	KeyIOTemp = P0;
	ZhkHardSramSelect();
	key = Key1[ KeyIOTemp / 0x10 ];if ( key != KEY_NULL ) return( key );
	key = Key2[ KeyIOTemp % 0x10 ];if ( key != KEY_NULL ) return( key );

	ZhkHardKeyBoardLine2Select();
	P0 = 0xFF;
	_nop_();
	_nop_();
	_nop_();
	KeyIOTemp = P0;
	ZhkHardSramSelect();
	key = Key3[ KeyIOTemp / 0x10 ];if ( key != KEY_NULL ) return( key );
	key = Key4[ KeyIOTemp % 0x10 ];if ( key != KEY_NULL ) return( key );
	return( key );
}

void ZhkKeyScanIsr( void ) using 1
{
	static uchar data KeyValue,KeyDownTime;
	static bit NewKey;
	uchar data key;

	key = ScanfKeyFun();
	if ( KeyValue != key )
	{
		KeyValue    = key;
		KeyDownTime = 0;
		NewKey      = 0;
	}
	else if ( key != KEY_NULL )
	{
		KeyDownTime ++;
		if ( KeyDownTime % 8 == 2 ) 
		{
			NewKey = 1;
			if ( ZhkKeyBufPtr >= sizeof( ZhkKeyBuf ) )
				ZhkKeyBufPtr = sizeof( ZhkKeyBuf );
			else
				ZhkKeyBuf[ ZhkKeyBufPtr ++ ] = KeyValue;
		}
	}
	
}



