#define DS12887_C
#include "device.h"
#include "clock.h"
#include "Ds12887.h"
bit Ds12887Init( void )		small
{
	XBYTE[ 0x32 ]  = 0x14;
	XBYTE[ 0x0B ] |= 0x80;
	XBYTE[ 0x01 ]  = 0x00;
	XBYTE[ 0x03 ]  = 0xEF;
	XBYTE[ 0x05 ]  = 0xEF;
	XBYTE[ 0x0B ] &= 0x7F;
	XBYTE[ 0x0A ]  = 0x20;
	XBYTE[ 0x0B ]  = 0x2A;
	return XBYTE[ 0x0D ] & 0x80;
	
}
/*
bit Ds12887WriteTime( struct time idata* TimeCache )		small
{
	XBYTE[ 0x0B ] |= 0x80;
	XBYTE[ 0x09 ]  = TimeCache->year ;
	XBYTE[ 0x08 ]  = TimeCache->mon;
	XBYTE[ 0x07 ]  = TimeCache->date;
	XBYTE[ 0x04 ]  = TimeCache->hour;
	XBYTE[ 0x02 ]  = TimeCache->min;
	XBYTE[ 0x00 ]  = TimeCache->sec;
	XBYTE[ 0x0B ] &= 0x7F;
	return 1;
}
bit Ds12887ReadTime( struct time idata* TimeCache )		small
{
	XBYTE[ 0x0B ] |= 0x80;
	TimeCache->year = XBYTE[ 9 ];
	TimeCache->mon  = XBYTE[ 8 ];
	TimeCache->date = XBYTE[ 7 ];
	TimeCache->hour = XBYTE[ 4 ];
	TimeCache->min  = XBYTE[ 2 ];
	TimeCache->sec  = XBYTE[ 0 ];
	XBYTE[ 0x0B ] &= 0x7F;
	return 1;
}
*/


