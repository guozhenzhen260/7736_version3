#include "device.h"
#include "stdio.h"
#include "stdarg.h"
#include "serial1.h"
#include "serial.h"
#include "string.h"
/*
void debug_output ( int level, int category, char *fmt, ...)  
{
  va_list arg_ptr;

  if((level <= _DEBUG_TRACE_LEVEL) || (category & _DEBUG_TRACE_CAT == 0))
  	return;

  va_start (arg_ptr, fmt);           // format string 
  vprintf (fmt, arg_ptr);
  va_end (arg_ptr);
}
void debug_output_category (int category, char *fmt, ...)
{
  va_list arg_ptr;

  if(category & _DEBUG_TRACE_CAT == 0)
  	return;

  va_start(arg_ptr, fmt);           // format string 
  vprintf (fmt, arg_ptr);
  va_end(arg_ptr);
}
*/
/*#ifdef _DEBUG_TRACE
void Trace( uchar *format , ... )
{
	va_list arg_ptr;
	char xdata StringTemp[40];
	char xdata format2[ 20 ];
	int xdata i;
	int xdata len;

	strncpy(format2, format, 20);

	va_start(arg_ptr, format);
	len = vsprintf( StringTemp , format2 , arg_ptr );
	va_end(arg_ptr);	

	for(i = 0; i < len; ++i)
	  putchar(StringTemp[i]);
}
#endif
*/
#ifdef _DEBUG_MOBIL_DEVICE_TRACE
void Trace( uchar *format , ... )
{
	va_list arg_ptr;
	uchar xdata StringTemp[ 256 ];
	uint  xdata i;
	uint  xdata len;
	while( ZhkSerialPutCh( 0x1D ) );
	ZhkSerialPutCh( 0x12 );
	va_start(arg_ptr, format);
	len = vsprintf( StringTemp , format , arg_ptr );
	va_end(arg_ptr);
	for( i = 0 ; i < len ; i ++ )
	{
		ZhkSerialPutCh( StringTemp[ i ] );
	}
	ZhkSerialPutCh( 0x12 );
	ZhkSerialPutCh( 0x1D );
	
}
#endif // _DEBUG_MOBIL_DEVICE_TRACE
uint ZhkSerial1Printf( uchar *format , ... )
{
	va_list arg_ptr;
	uchar StringTemp[ 256 ];
	uint  i;
	uint  len;
	va_start(arg_ptr, format);
	len = vsprintf( StringTemp , format , arg_ptr );
	va_end(arg_ptr);
	for( i = 0 ; i < len ; i ++ )
	{
		ZhkSerial1PutCh( StringTemp[ i ] );
	}
	return 0;
}

void PrintTest( uchar *format , ... )
{
	va_list arg_ptr;
	uchar xdata StringTemp[ 1024 ];
	uint  xdata i;
	uint  xdata len;
	while( ZhkSerialPutCh( 0x1D ) );
	ZhkSerialPutCh( 0x12 );
	va_start(arg_ptr, format);
	len = vsprintf( StringTemp , format , arg_ptr );
	va_end(arg_ptr);
	for( i = 0 ; i < len ; i ++ )
	{
		ZhkSerialPutCh( StringTemp[ i ] );
	}
	ZhkSerialPutCh( 0x12 );
	ZhkSerialPutCh( 0x1D );
	
}

uchar getPcCmd(uchar xdata *MsgTemp)
{
	uchar xdata MobileMsgTemp[ 20 ];
	uchar xdata i=0;
	while( ! ZhkSerialIsRxBufNull() )
	{
		if ( i >= sizeof( MobileMsgTemp ) )
	 		i = 0;
		MobileMsgTemp[ i ++ ] = ZhkSerialGetCh();
		MobileMsgTemp[ i ] = '\0';
		
		if ( ( i >= 2 ) && ( MobileMsgTemp[ i - 2 ] == 'E') && ( MobileMsgTemp[ i - 1 ] == 'T' ) )
		{				
			i-=2;
			memcpy( MsgTemp , &MobileMsgTemp[0], i );			
			//clearReadSerial();									
		}
	}	
	return i;
}

