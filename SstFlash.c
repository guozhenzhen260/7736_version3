#define SSTFLASH_C
#include "device.h"
#include "SstFlash.h"
#include "common.h"
#include <stdio.h>
#include "debug.h"

void SstFlashPutCh( unsigned long FlashAddr , unsigned char ch )small;
unsigned char SstFlashGetCh( unsigned long FlashAddr )small;
//void DelaySectorErase( void )small;
void DelayFlashOp( void )small;
/*
unsigned char SstFlashGetCh( unsigned long FlashAddr )		small
{
	unsigned int  data FlashAddrL;
	unsigned char data FlashAddrH;

	FlashAddrH = ( ( uchar data* )&FlashAddr )[ 1 ];
	FlashAddrL = FlashAddr & 0xFFFF;
	PinAddr18 = FlashAddrH & 0x04;
	PinAddr17 = FlashAddrH & 0x02;
	PinAddr16 = FlashAddrH & 0x01;
	return XBYTE[ FlashAddrL ];
}

bit SstFlashSelfCheck( void )		small
{
	unsigned char data ManufacturerID , DeviceID;	

	PinAddr18 = 0;
	PinAddr17 = 0;
	PinAddr16 = 0;
	XBYTE[ 0x555 ] = 0xAA;
	XBYTE[ 0x2AA ] = 0x55;
	XBYTE[ 0x555 ] = 0x90;

	ManufacturerID = XBYTE[ 0 ];	
	ManufacturerID = XBYTE[ 1 ];	
	if ( ManufacturerID != FLASHMANUFACTURERID || DeviceID != FLASHDEVICEID )
		return 0;
	return 1;
}
*/
bit SstFlashChipErase( void )		small
{

	XBYTE[ 0x555 ] = 0xAA;
	XBYTE[ 0x2AA ] = 0x55;
	XBYTE[ 0x555 ] = 0x80;
	XBYTE[ 0x555 ] = 0xAA;
	XBYTE[ 0x2AA ] = 0x55;
	XBYTE[ 0x555 ] = 0x10;
	while( (XBYTE[0] & 0x40) != (XBYTE[0] & 0x40) )
		_nop_();
	return 1;
}
bit SstFlashSectorErase( unsigned long FlashAddr )		small
{
	unsigned  int data FlashAddrL;
	unsigned char data FlashAddrH;
	unsigned char data i;

	XBYTE[ 0x555 ] = 0xAA;
	XBYTE[ 0x2AA ] = 0x55;
	XBYTE[ 0x555 ] = 0x80;
	XBYTE[ 0x555 ] = 0xAA;
	XBYTE[ 0x2AA ] = 0x55;	

	FlashAddrH = ( ( uchar data* )&FlashAddr )[ 1 ];
	FlashAddrL = FlashAddr & 0xFFFF;
	PinAddr18 = FlashAddrH & 0x04;
	PinAddr17 = FlashAddrH & 0x02;
	PinAddr16 = FlashAddrH & 0x01;	
	XBYTE[ FlashAddrL ] = 0x20;

	while( (XBYTE[0] & 0x40) != (XBYTE[0] & 0x40) )
		_nop_();

	FlashAddrL  = FlashAddr & 0xffffff80;
	for( i = 0 ; i < SECTORSIZE ; i ++ )
	{
		if ( XBYTE[ FlashAddrL ++ ] != 0xFF ) 
			return 0;
	}
	return 1;
}

bit SstFlashStringProgam( unsigned long FlashAddr , unsigned char idata* RamAddr , unsigned char len )		small
{
	unsigned  int data FlashAddrL;
	unsigned char data FlashAddrH;

	while( len -- )
	{		
		PinAddr18 = 0;
		PinAddr17 = 0;
		PinAddr16 = 0;
		XBYTE[ 0x555 ] = 0xAA;
		XBYTE[ 0x2AA ] = 0x55;
		XBYTE[ 0x555 ] = 0xA0;
		
		FlashAddrH = ( ( uchar data* )&FlashAddr )[ 1 ];
		FlashAddrL = FlashAddr & 0xFFFF;
		PinAddr18 = FlashAddrH & 0x04;
		PinAddr17 = FlashAddrH & 0x02;
		PinAddr16 = FlashAddrH & 0x01;
		XBYTE[ FlashAddrL ] = *RamAddr;
	
		while( (XBYTE[0] & 0x40) != (XBYTE[0] & 0x40) )
			_nop_();

		FlashAddr++;
		RamAddr++;
	}
	return 1;
}

bit SstFlashReadString( unsigned long FlashAddr , unsigned char idata* RamAddr , unsigned char len )		small
{
	unsigned int  data FlashAddrL;
	unsigned char data FlashAddrH;

	while( len -- )
	{	
		FlashAddrH = ( ( uchar data* )&FlashAddr )[ 1 ];
		FlashAddrL = FlashAddr & 0xFFFF;
		PinAddr18 = FlashAddrH & 0x04;
		PinAddr17 = FlashAddrH & 0x02;
		PinAddr16 = FlashAddrH & 0x01;
		*RamAddr ++ = XBYTE[ FlashAddrL ];
		FlashAddr ++;
	}
	return 1;
}
/*
void SstFlashPutCh( unsigned long FlashAddr , unsigned char ch )		small
{
	unsigned  int data FlashAddrL;
	unsigned char data FlashAddrH;
	
	FlashAddrH = ( ( uchar data* )&FlashAddr )[ 1 ];
	FlashAddrL = FlashAddr & 0xFFFF;
	PinAddr18 = FlashAddrH & 0x04;
	PinAddr17 = FlashAddrH & 0x02;
	PinAddr16 = FlashAddrH & 0x01;
	XBYTE[ FlashAddrL ] = ch;


}

//2005.11.08 liny 合并DelayChipErase和DelaySectorErase函数为DelayFlashOp。
void DelayFlashOp( void )		small
{
	//2005.11.08 liny 原赵汗可程序延时太短，现改为ToggleBit(DQ6)测试确定Flash是否擦除成功
	while( (XBYTE[0] & 0x40) != (XBYTE[0] & 0x40) )
		_nop_();
}
*/
