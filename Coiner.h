#ifndef _COINER_
#define _COINER_

#ifndef COINER_C
	#define CoinerSourse extern
#else
	#define CoinerSourse	
#endif

//	CoinerSourse unsigned char xdata CoinAccept[ 8 ];	
	CoinerSourse void HykCoinerReset( void );
	CoinerSourse void HykCoinerEnable( void );
	CoinerSourse void HykCoinerDisable( void );
	CoinerSourse void HykCoinerGetState( void );

#define HYKSERIAL_FRAMEHEAD 	0x10	
#define COINER_TYPE  1	

#endif