#ifndef _SerialProcess_VTI_
#define _SERIALPROCESS_VTI_
#ifndef SERIALPROCESS_VTI_C
	#define ZhkVtiSourse extern
#else
	#define ZhkVtiSourse 
#endif

	ZhkVtiSourse void ZhkVtiCasherInit( void );
	ZhkVtiSourse void ZhkVtiAcceptCash( void );
	ZhkVtiSourse void ZhkVtiRejectCash( void );
	ZhkVtiSourse void ZhkVtiDisable( void );
	ZhkVtiSourse void ZhkVtiEnable( void );
	ZhkVtiSourse void ZhkVtiAck( void );
	ZhkVtiSourse void ZhkVtiGetState( void );
	ZhkVtiSourse unsigned char ZhkVtiSerialProcess( void );
	ZhkVtiSourse void ZhkVtiSerialInit( void );
	ZhkVtiSourse void ZhkVtiSendCmd( unsigned char idata* buf , unsigned char style );

	
	#define VTISERIAL_FRAMEHEAD 	0x02
	#define VTISERIAL_FRAMETAIL  	0x03
	
	

#endif