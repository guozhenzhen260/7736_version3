#undef _DEBUG_TRACE
#undef _DEBUG_MOBIL_DEVICE_TRACE
#define CASHER_C
#include "device.h"
#include "casher.h"
#include "serial1.h"
#include "common.h"
#include "global.h"   //Added by Andy on Oct 9,2008
#include "debug.h" 

/*
SERAIL1PARAM  code ZhkVtiSerialParam =
{
	7 , VERIFY_EVEN , 1 , 0x00 , 0x96 , 0x00
};

unsigned  int code CashAccept[ 8 ] =
{
   #ifdef _CHINA_	
   		1, 2, 5, 10, 20, 50, 100, 0
   #else
  		10, 20, 50, 100, 0, 0, 0, 0
   #endif
   
};

unsigned  char code CashAcceptEnableTable[ 8 ] =
{
   #ifdef _CHINA_	
		0, 0, 1, 1, 1, 0, 0, 0
   #else
   		1, 1, 1, 1, 0, 0, 0, 0   
   #endif
   
};

unsigned char bdata CasherBit;
sbit CBit0 = CasherBit^0;
sbit CBit1 = CasherBit^1;
sbit CBit2 = CasherBit^2;
sbit CBit3 = CasherBit^3;
sbit CBit4 = CasherBit^4;
sbit CBit5 = CasherBit^5;
sbit CBit6 = CasherBit^6;
sbit CBit7 = CasherBit^7;

void ZhkVtiSendCmd( unsigned char idata* buf , unsigned char style )
{
	static unsigned char data state = 0x10;
	unsigned char xdata i,sum;

	if ( style > 1 ) 
		return;
	if ( state != 0x10 ) 
		state = 0x11;	
	if ( style == 1 ) 
		state ^= 0x01;

	Trace("\n Send date to casher ");
	// 发送帧头
	ZhkSerial1PutCh( VTISERIAL_FRAMEHEAD );
	Trace(" 0x%02bx", VTISERIAL_FRAMEHEAD );
	// 发送数据长度
	sum = 0x08;
	ZhkSerial1PutCh( sum );
	Trace(" 0x%02bx", sum );

	// 发送状态字
	sum ^= state;
	ZhkSerial1PutCh( state );
	Trace(" 0x%02bx", state );
	for( i = 0 ; i < 3 ; i ++ )
	{
		ZhkSerial1PutCh( buf[ i ] );		
		Trace(" 0x%02bx", buf[ i ] );
		sum ^= buf[ i ];
	}
	ZhkSerial1PutCh( VTISERIAL_FRAMETAIL );	
	Trace(" 0x%02bx", VTISERIAL_FRAMETAIL );
	ZhkSerial1PutCh( sum );
	Trace(" 0x%02bx \n", sum );
}

void ZhkCasherSerialInit( void )
{	
	unsigned char xdata i;

	for( i = 0 ; i < sizeof( SERAIL1PARAM ) ; i ++ )
		ZhkSerial1Param[ i ] = ZhkVtiSerialParam[ i ];
	ZhkSerial1Init();
    
   
    
    if( SystemParameter.billValue[0] != 0   )
	{
		CBit0 = 1;
	}
	if( SystemParameter.billValue[1] != 0   )
	{
		CBit1 = 1;
	}
	if( SystemParameter.billValue[2] != 0   )
	{
		CBit2 = 1;
	}
	if( SystemParameter.billValue[3] != 0   )
	{
		CBit3 = 1;
	}
	if( SystemParameter.billValue[4] != 0   )
	{
		CBit4 = 1;
	}
	if( SystemParameter.billValue[5] != 0   )
	{
		CBit5 = 1;
	}
	if( SystemParameter.billValue[6] != 0   )
	{
		CBit6 = 1;
	}
	if( SystemParameter.billValue[7] != 0   )
	{
		CBit7 = 1;
	}

}

void ZhkCasherAcceptCash( void )
{	
	unsigned char data buf[ 3 ];
	
	buf[ 0 ] = CasherBit;
    buf[ 1 ] = 0x30;              //0x34, 0x30
    buf[ 2 ] = 0x00;

	ZhkVtiSendCmd( buf , 1 );
}

void ZhkCasherRejectCash( void )
{	
	unsigned char data buf[ 3 ];

	buf[ 0 ] = 0x00;
    buf[ 1 ] = 0x50;             //0x54, 0x50 
    buf[ 2 ] = 0x00;

	ZhkVtiSendCmd( buf , 1 );
}

void ZhkCasherGetState( unsigned char Flag )
{	
	unsigned char data buf[ 3 ];

	buf[ 0 ] = CasherBit;
    buf[ 1 ] = 0x14;
    buf[ 2 ] = 0x00; 

	ZhkVtiSendCmd( buf , Flag );
}

//禁能纸币器就是不让它收任何币种即可
void CasherDisableCasher( void )
{
	unsigned char data buf[ 3 ];

	buf[ 0 ] = 0x00;
    buf[ 1 ] = 0x14;
    buf[ 2 ] = 0x00; 

	ZhkVtiSendCmd( buf , 1 );
}
*/
