#ifndef _CASHER_
#define _CASHER_

#ifndef CASHER_C
	#define ZhkCasherSourse extern
//	ZhkCasherSourse unsigned  int xdata CashAccept[ 8 ];
#else
	#define ZhkCasherSourse
//	ZhkCasherSourse unsigned  int xdata CashAccept[ 8 ] =
//	{
//		50 , 100 , 200 , 500 , 1000 , 2000 , -1 , -1
//	};
#endif

	ZhkCasherSourse unsigned  int code CashAccept[ 8 ];
	ZhkCasherSourse unsigned  char code CashAcceptEnableTable[ 8 ];

	#define VTISERIAL_FRAMEHEAD 	0x02
	#define VTISERIAL_FRAMETAIL  	0x03

	ZhkCasherSourse void ZhkCasherSerialInit( void );	
//	ZhkCasherSourse void ZhkCasherInit( void );
	ZhkCasherSourse void ZhkCasherEnable( void );
	ZhkCasherSourse void ZhkCasherDisable( void );
	ZhkCasherSourse void ZhkCasherAcceptCash( void );
	ZhkCasherSourse void ZhkCasherRejectCash( void );
	ZhkCasherSourse void ZhkCasherGetState( unsigned char Flag );
	ZhkCasherSourse void CasherDisableCasher( void );
	ZhkCasherSourse unsigned char ZhkCasherCmd( unsigned char xdata* cmdbuf , unsigned char len );
//	ZhkCasherSourse unsigned char ZhkCasherSerialProcess( void );
	
#endif