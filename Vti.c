#define SERIALPROCESS_VTI_C
#include "device.h"
#include "serial1.h"
#include "casher.h"
#include "common.h"
#include "vti.h"
#include "debug.h"

SERAIL1PARAM  code ZhkVtiSerialParam =
{
	7 , VERIFY_EVEN , 1 , 0x00 , 0x96 , 0x00
};


void ZhkVtiSerialInit( void )
{
	unsigned char data i;
	for( i = 0 ; i < sizeof( SERAIL1PARAM ) ; i ++ )
		ZhkSerial1Param[ i ] = ZhkVtiSerialParam[ i ];
	ZhkSerial1Init();
}

void ZhkVtiSendCmd( unsigned char idata* buf , unsigned char style )
{
	static unsigned char data state = 0x10;
	unsigned char data i,sum;

	if ( state != 0x10 ) state = 0x11;
	if ( style > 1 ) return;
	if ( style == 1 ) state ^= 0x01;

	// 发送帧头
	ZhkSerial1PutCh( VTISERIAL_FRAMEHEAD );
	// 发送数据长度
	sum = 0x08;
	ZhkSerial1PutCh( sum );

	// 发送状态字
	sum ^= state;
	ZhkSerial1PutCh( state );
	for( i = 0 ; i < 3 ; i ++ )
	{
		ZhkSerial1PutCh( buf[ i ] );		
		sum ^= buf[ i ];
	}
	ZhkSerial1PutCh( VTISERIAL_FRAMETAIL );	
	ZhkSerial1PutCh( sum );
}

void ZhkVtiAcceptCash( void )
{
	unsigned char data buf[ 3 ];
	
	buf[ 0 ] = 0x03;
    buf[ 1 ] = 0x34;
    buf[ 2 ] = 0x00;
	
	ZhkVtiSendCmd( buf , 1 );
}

void ZhkVtiRejectCash( void )
{
	unsigned char data buf[ 3 ];

	buf[ 0 ] = 0x00;
    buf[ 1 ] = 0x54;
    buf[ 2 ] = 0x00;

	ZhkVtiSendCmd( buf , 1 );
}

void ZhkVtiAck( void )
{
	unsigned char data buf[ 3 ];
	
	buf[ 0 ] = 0x03;
    buf[ 1 ] = 0x14;
    buf[ 2 ] = 0x00; 

	ZhkVtiSendCmd( buf , 1 );
}