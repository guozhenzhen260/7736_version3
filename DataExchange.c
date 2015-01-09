#define DATAEXCHANGE_C
#include "debug.h"
#include "device.h"
#include "DataExchange.h"
#include "SstFlash.h"
#include "stdio.h"
#include "string.h"

bit MovDataFromSram( uchar xdata* SramAddr , uchar idata* RamAddr , uchar len )
{
	while(len --)
		*RamAddr ++ = *SramAddr ++;

	return 1;	
}

bit MovDataToSram( uchar xdata* SramAddr , uchar idata* RamAddr , uchar len )
{
	while(len --)
		*SramAddr ++ = *RamAddr ++;

	return 1;
}

bit MovSramToSram( uchar xdata* Sourse , uchar xdata* Object , uint len ) 
{
	while( len -- )
		*Object ++ = *Sourse ++;
	return 1;
}

bit PogramFlash( ulong FlashAddr , uchar xdata* SramAddr , uint len ) small
{
	uchar data lentemp;

	while( len )
	{
		lentemp = sizeof( Cache );
		if ( len < sizeof( Cache ) ) 
			lentemp = len;		

		MovDataFromSram( SramAddr , Cache , lentemp );
		if ( (FlashAddr & 0x80000) == 0) 			
			P1 = 0x09;
		else			
			P1 = 0x0a;

		SstFlashStringProgam( FlashAddr , Cache , lentemp );
		ZhkHardSramSelect();
		FlashAddr += lentemp;
		SramAddr  += lentemp;		
		len       -= lentemp;
	}	
	return 1;
}

bit MovSramToFlash( ulong FlashAddr , uchar xdata* SramAddr , uint len ) small
{
	uchar xdata TempBuf[ SECTORSIZE ];
	uint  data len1;
	uint  data len2;

//	Trace("\n FlashAddr2 = 0x%02lx", FlashAddr );
	while( len > 0)
	{
		len1 = FlashAddr % SECTORSIZE;
		len2 = SECTORSIZE - len1;

		if( len1 > 0 || len2 > len )
			MovFlashToSram( FlashAddr - len1, TempBuf, SECTORSIZE );

		len2 = (len > len2) ? len2 : len;
		MovSramToSram( SramAddr, TempBuf + len1, len2 );		

		if ( FlashAddr & 0x80000 == 0) 
			P1 = 0x09;
		else
			P1 = 0x0a;		

		SstFlashSectorErase( FlashAddr );
		ZhkHardSramSelect();		
		PogramFlash( FlashAddr - len1, TempBuf, SECTORSIZE );

		FlashAddr += len2;
		SramAddr += len2;
		len -= len2;
	}

	return 1;
}

bit MovFlashToSram( ulong FlashAddr , uchar xdata* SramAddr , uint len ) small
{
	uint data lentemp;
	
	while( len > 0 )
	{
		lentemp = (len > sizeof( Cache )) ? sizeof( Cache ) : len;
		////////////////////////
//		MovDataFromFlash( FlashAddr, Cache, lentemp );
/*		if ( (FlashAddr & 0x80000) == 0) 
			ZhkHardFlashSelect();			
		else
			ZhkHardFlash1Select();	
*/			
		if ( (FlashAddr & 0x80000) == 0) 			
			P1 = 0x09;
		else			
			P1 = 0x0a;

		SstFlashReadString( FlashAddr ,Cache , lentemp );
		ZhkHardSramSelect();		

		MovDataToSram( SramAddr,Cache,lentemp);	
//		memcpy( SramAddr,Cache,lentemp);	

		FlashAddr += lentemp;
		SramAddr += lentemp;
		len -= lentemp;
	}
	return 1;
}
/*
bit FlashDel( ulong FlashAddr , ulong len ) small
{
	ZhkHardSramSelect();
	FlashAddr = FlashAddr / SECTORSIZE * SECTORSIZE;
	
	while( len )
	{
		if ( FlashAddr % FLASHCHIPSIZE == 0 && len >= FLASHCHIPSIZE )
		{		
			if ( (FlashAddr & 0x80000) == 0) 			
				P1 = 0x09;
			else			
				P1 = 0x0a;
			SstFlashChipErase();
			ZhkHardSramSelect();
			FlashAddr += FLASHCHIPSIZE;
			if ( len < FLASHCHIPSIZE ) len = FLASHCHIPSIZE;
			len -= FLASHCHIPSIZE;
			continue;
		}	
		if ( (FlashAddr & 0x80000) == 0) 			
			P1 = 0x09;
		else			
			P1 = 0x0a;
		SstFlashSectorErase( FlashAddr );
		ZhkHardSramSelect();
		FlashAddr += SECTORSIZE;
		if ( len < SECTORSIZE ) len = SECTORSIZE;
		len -= SECTORSIZE;
	}
	return 1;
}*/
