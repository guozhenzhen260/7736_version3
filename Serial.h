#ifndef _SERIAL_
#define _SERIAL_
#ifndef SERIAL_C
	#define ZhkSerialSourse extern 
#else
	#define ZhkSerialSourse
#endif


#ifndef VERIFY_NULL
	#define VERIFY_NULL 	0
#endif
#ifndef	VERIFY_ODD
	#define VERIFY_ODD 	1
#endif 
#ifndef VERIFY_EVEN
	#define VERIFY_EVEN 	2
#endif

//---------------------------------------------------
//Added by Andy on 2009.03.01
typedef uchar SERAILPARAM[6];

#ifndef ZHKSERIAL_C
        extern SERAILPARAM xdata ZhkSerialParam;
#endif
//===================================================

ZhkSerialSourse uchar ZhkSerialInit( void );
ZhkSerialSourse uchar ZhkSerialPutCh( uchar ch );
ZhkSerialSourse uchar ZhkSerialGetCh( void );
ZhkSerialSourse uchar ZhkSerialIsRxBufNull( void );
ZhkSerialSourse uchar ZhkSerialTxMsg( uchar* TxBuf , uint len );

#endif