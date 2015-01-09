#ifndef _MOBILE_
#define _MOBILE_
#ifndef MOBILE_C
	#define MobileSourse extern
#else
	#define MobileSourse
#endif
	#include "device.h"
	#include "gprs.h"
	#include "ShortMsg.h"
			
	
////////////////////////////////////////////////////////////////////////////////////
	MobileSourse uchar ZhkMobileProcess( void );
	MobileSourse uchar ZhkMobileInit( void );
	MobileSourse uchar SendATcommandToMobile();
//	MobileSourse bit   SMSMobileStatus(void);
	
#endif