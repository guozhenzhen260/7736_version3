#define CLOCK_C
#include "device.h" 
#include "clock.h"
#include "ds12887.h"
//#include "DataExchange.h"
bit ClockInit( void )			small
{
	bit c;
	ZhkHardDs12887Select();	
	c = Ds12887Init();
	ZhkHardSramSelect();
	return c;
}
/*
bit GetCurrentTime( struct time xdata* CurrentTime )		small
{
	struct time idata TempTime;

	ZhkHardDs12887Select();	
	Ds12887ReadTime( &TempTime );
	ZhkHardSramSelect();
	MovDataToSram( ( unsigned char xdata* )CurrentTime , ( unsigned char idata* )&TempTime , sizeof( struct time ) );
	return 1;
}
bit SetCurrentTime( struct time xdata* CurrentTime )		small
{
	struct time idata TempTime;
	
	ZhkHardSramSelect();
	MovDataFromSram( ( unsigned char xdata* )CurrentTime , ( unsigned char idata* )&TempTime , sizeof( struct time ) );
	ZhkHardDs12887Select();	
	Ds12887WriteTime( &TempTime );
	ZhkHardSramSelect();
	return 1;
}*/
