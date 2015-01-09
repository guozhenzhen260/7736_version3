#define COMMON_C
#include "device.h"
#include "common.h"

/////////////////////////////////////////////////////////////////////////////////
void delay( uchar Cycle )
{
	while( Cycle ) 
		Cycle --;
}
/////////////////////////////////////////////////////////////////////////////////

//void DelayMs( uchar MicroSecond )
void DelayMs( uint MicroSecond )
{
	uchar xdata iCycle;

	MicroSecond ++;
	while( -- MicroSecond )
	{
		iCycle = 224;
		while( iCycle ) 
			iCycle --;

		iCycle = 224;
		while( iCycle ) 
			iCycle --;		 
	}
}

void NumberToString( unsigned long dat , uchar * str , uchar len  )
{
	unsigned long code divisor[]=
	{
		1,
		10,
		100,
		1000,
		10000,
		100000,
		1000000,
		10000000,
		100000000
	};
	uchar data i,ch;
	unsigned long data temp1;
	unsigned  int data temp2;
	uchar data temp3;
	unsigned long data divisor1;
	unsigned  int data divisor2;
	
	if ( len > 8 ) len = 8;
	temp1    = dat;
	temp1   %= divisor[ len ];
	for( i = 8 ; i > 4 ; i -- )
	{
		divisor1 = divisor[ i - 1 ];
		for( ch = 0 ; ch < 9 ; ch ++ )
		{
			if ( temp1 < divisor1 ) break;
			temp1 -= divisor1;
		}
		if ( i <= len )
			str[ len - i ] = ch;
	}
	temp2    = temp1;
	for( i = 4 ; i > 2 ; i -- )
	{
		divisor2 = divisor[ i - 1 ];
		for( ch = 0 ; ch < 9 ; ch ++ )
		{
			if ( temp2 < divisor2 ) break;
			temp2 -= divisor2;
		}
		if ( i <= len )
			str[ len - i ] = ch;
	}
	temp3 = temp2;
	ch    = temp3 / 10;
	if ( 2 <= len )
		str[ len - 2 ] = ch;
	ch    = temp3 % 10;
	if ( 1 <= len )
		str[ len - 1 ] = ch;
	for( i = 0 ; i < len - 1 ; i ++ )
	{
		if ( str[ i ] != 0 ) break;
		str[ i ] = 0x20;
	}
	for( ; i < len ; i ++ )
	{
		str[ i ] += '0';
	}
}