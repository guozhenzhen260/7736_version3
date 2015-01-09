#define SERIAL_C
 #define ZHKSERIAL_C
#ifndef ZHKSTORAGETYPE 
#define ZHKSTORAGETYPE xdata 
#endif
#ifndef ZHK_INTERRUPT
#define ZHK_INTERRUPT
#endif 
#include "device.h"
#include "Serial.h"
#include "Serial1.h"
#include "string.h"
#include "debug.h"
#define ZHKSERIALRXBUFLEN 	384
#define ZHKSERIALTXBUFLEN  	384
#ifndef ZHKLINEMAX
	#define ZHKLINEMAX 128
#endif

//typedef uchar SERAILPARAM[6];

ZhkSerialSourse SERAILPARAM code ZhkSerialParam = 
{ 
  //8 , VERIFY_NULL , 1 , 0x03 , 0x84 , 0x00
 	8 , VERIFY_NULL , 1 , 0x00 , 0x96 , 0x00
};

#define ZhkBaudRate 	ZhkSerialParam[ 4 ] + ZhkSerialParam[ 3 ] * 256
#define ZhkDataBit  	ZhkSerialParam[ 0 ]
#define ZhkVerifyBit 	ZhkSerialParam[ 1 ]
#define ZhkStopBit   	ZhkSerialParam[ 2 ]

uchar ZHKSTORAGETYPE ZhkSerialRxBuf[ ZHKSERIALRXBUFLEN ];
uint ZHKSTORAGETYPE ZhkSerialRxBufWr;
uint ZHKSTORAGETYPE ZhkSerialRxBufRd;

uchar ZHKSTORAGETYPE ZhkSerialTxBuf[ ZHKSERIALTXBUFLEN ];
uint  ZHKSTORAGETYPE ZhkSerialTxBufWr;
uint  ZHKSTORAGETYPE ZhkSerialTxBufRd;

bit ZhkSerialTxdEmpty;

uchar ZhkSerialIsRxBufNull( void );
//uchar ZhkSerialIsRxFullIsr( void );
uchar ZhkSerialIsTxBufNull( void );
//uchar ZhkSerialIsTxBufNullIsr( void );

uchar ZhkSerialInit( void )
{
	uint  data dat;
	uchar data ch;

	T2CON   = 0x34;
	SCON    = 0xD0;
  	
	ch = ZhkDataBit + ZhkStopBit;
	if ( ZhkVerifyBit != VERIFY_NULL ) ch ++;
	if ( ch != 9 && ch != 10 ) return 0;
	if ( ZhkVerifyBit == VERIFY_NULL || ZhkDataBit < 8 )
	{
		SM0 = 0;
	}
	
	/* ZhkBaudRate 是波特率/100 用BCD码表示 */
	dat      = ZhkBaudRate;
	dat     -= dat / 4096 * 6 * 256;
	dat     -= dat /  256 * 6 *  16;
	dat     -= dat /   16 * 6;
	dat      = ( OSCRATE / 32 / 100  * 12 / MACHINECYCLE ) / dat;
	dat      = - dat;
	RCAP2H   = dat / 256;
	RCAP2L   = dat % 256;

//	S0STAT   = 0x20;
	ES       = 1;
	#ifdef _DEBUG_TRACE
	ES = 0;
	#endif
	TI       = 1;

	ZhkSerialTxdEmpty = 1;
	ZhkSerialTxBufWr  = 0;
	ZhkSerialTxBufRd  = 0;
	ZhkSerialRxBufWr  = 0;
	ZhkSerialRxBufRd  = 0;
	return 1;
	
}

uchar ZhkSerialPutCh( uchar ch )
{

	uint data i;
	bit data veri;
	for( i = 0 ; i < 2000 ; i ++ )	if( ZhkSerialTxdEmpty )
	{ 
		ZhkSerialTxdEmpty = 0;
		ACC   = ch;
		veri  = P;
		if ( ZhkVerifyBit == VERIFY_ODD  ) veri = ! veri;
		if ( ZhkVerifyBit == VERIFY_NULL ) veri = 1;
		TB8 = veri;
	 	SBUF = ch;
		return 0;
	}
	return 1 ;

}
uchar ZhkSerialGetCh( void )
{
	uchar data ch;
	ch = ZhkSerialRxBuf[ ZhkSerialRxBufRd ];
	ZhkSerialRxBufRd ++;
	if ( ZhkSerialRxBufRd >= ZHKSERIALRXBUFLEN ) ZhkSerialRxBufRd = 0;
	ZhkHardSetRTS();
	return ch;
	
}
uchar ZhkSerialTxMsg( uchar* TxBuf , uint len )
{
	uchar i;
	if ( len > ZHKSERIALTXBUFLEN ) 
		return 1;
	while ( ! ZhkSerialIsTxBufNull() )
			;

//	Trace("\n ZhkSerialTxMsg len = %u", len );
	memcpy( ZhkSerialTxBuf , TxBuf , len );
	ZhkSerialPutCh( ZhkSerialTxBuf[ 0 ] );
	ZhkSerialTxBufRd = 1;
	ZhkSerialTxBufWr = len;
	#ifdef _DEBUG_MOBILE_TRACE
		for( i = 0 ; i < len ; i ++ )
			ZhkSerial1PutCh( TxBuf[ i ] );
	#endif
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
uchar ZhkSerialIsRxBufNull( void )
{
	if ( ZhkSerialRxBufRd - ZhkSerialRxBufWr == 0 ) 
		return 1;
	_nop_();
	return 0;
}
uchar ZhkSerialIsTxBufNull( void )
{
	if  ( ZhkSerialTxBufRd - ZhkSerialTxBufWr == 0 ) 
		return 1;
	return 0;
}

void ZhkSerial_Isr( void ) interrupt 4 using 1 
{
	bit data veri;
	ZhkSaveChipSelect();
	ZhkHardSramSelect();
	if ( TI )
	{
		TI = 0;
		if ( /*! ZhkHardIsCTS() || */ZhkSerialIsTxBufNull() )
		{
			ZhkSerialTxdEmpty = 1;
		}
		else
		{
			SBUF = ZhkSerialTxBuf[ ZhkSerialTxBufRd ++ ];
			if ( ZhkVerifyBit == VERIFY_ODD  ) veri = ! veri;
			if ( ZhkVerifyBit == VERIFY_NULL ) veri = 1;
			TB8  = veri;	
		}
	}
	if ( RI )
	{
		RI = 0;
		ZhkSerialRxBuf[ ZhkSerialRxBufWr ] = SBUF;
		
		ZhkSerialRxBufWr ++;
		if ( ZhkSerialRxBufWr >= ZHKSERIALRXBUFLEN )
			ZhkSerialRxBufWr = 0;
	}
	ZhkLoadChipSelect();
}


