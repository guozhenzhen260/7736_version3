#ifndef _ZHKSERIAL1_
#define _ZHKSERIAL1_
#ifndef ZHKSERIAL1_C
	#define ZhkSerial1Sourse extern
#else
	#define ZhkSerial1Sourse 
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
typedef unsigned char SERAIL1PARAM[6];
#ifndef ZHKSERIAL1_C
extern SERAIL1PARAM xdata ZhkSerial1Param;
#endif
ZhkSerial1Sourse unsigned char ZhkSerial1Init( void );
ZhkSerial1Sourse unsigned char ZhkSerial1PutCh( unsigned char ch );
ZhkSerial1Sourse unsigned char ZhkSerial1GetCh( void );
ZhkSerial1Sourse unsigned char ZhkSerial1IsRxBufNull( void );
ZhkSerial1Sourse unsigned char ZhkSerial1TxMsg( unsigned char xdata* TxBuf , unsigned char len );
ZhkSerial1Sourse unsigned char ZhkSerial1_MDB_Init( void );
ZhkSerial1Sourse unsigned char  ZhkSerial1_MDB_PutCh( unsigned char ch,unsigned char mode);
ZhkSerial1Sourse unsigned char  ZhkSerial1_MDB_GetCh( unsigned char  *byte, unsigned char  *bMode );

#endif