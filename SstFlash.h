#ifndef _SSTFLASH_
#define _SSTFLASH_
#ifndef SSTFLASH_C
	#define SstFlashSourse extern 
#else
	#define SstFlashSourse
#endif

	#define SST29SF512 		1
	#define SST29VF512 		2
	#define SST29SF010 		3
	#define SST29VF010 		4
	#define SST29SF020 		5
	#define SST29VF020 		6
	#define SST29SF040 		7
	#define SST29VF040 		8
	
	#define SECTORSIZE 		128
	#define FLASHMANUFACTURERID 	0xBF
	
	#define FLASHDEVICE SST29VF040
	#if ( FLASHDEVICE == SST29SF512 )
		#define FLASHDEVICEID 	0x20
	#elif( FLASHDEVICE == SST29VF512 )
		#define FLASHDEVICEID 	0x21
	#elif( FLASHDEVICE == SST29SF010 )
		#define FLASHDEVICEID 	0x22
	#elif( FLASHDEVICE == SST29VF010 )
		#define FLASHDEVICEID 	0x23
	#elif( FLASHDEVICE == SST29SF020 )
		#define FLASHDEVICEID 	0x24
	#elif( FLASHDEVICE == SST29VF020 )
		#define FLASHDEVICEID 	0x25
	#elif( FLASHDEVICE == SST29SF040 )
		#define FLASHDEVICEID 	0x13
	#elif( FLASHDEVICE == SST29VF040 )
		#define FLASHDEVICEID 	0x14
	#endif
	 
	#if ( FLASHDEVICE == SST29SF512 )
		#define FLASHCHIPSIZE 	0x10000
	#elif( FLASHDEVICE == SST29VF512 )
		#define FLASHCHIPSIZE 	0x10000
	#elif( FLASHDEVICE == SST29SF010 )
		#define FLASHCHIPSIZE 	0x20000
	#elif( FLASHDEVICE == SST29VF010 )
		#define FLASHCHIPSIZE 	0x20000
	#elif( FLASHDEVICE == SST29SF020 )
		#define FLASHCHIPSIZE 	0x40000
	#elif( FLASHDEVICE == SST29VF020 )
		#define FLASHCHIPSIZE 	0x40000
	#elif( FLASHDEVICE == SST29SF040 )
		#define FLASHCHIPSIZE 	0x80000
	#elif( FLASHDEVICE == SST29VF040 )
		#define FLASHCHIPSIZE 	0x80000
	#endif

	SstFlashSourse bit SstFlashSelfCheck( void )		small;
	SstFlashSourse bit SstFlashChipErase( void )		small;
	SstFlashSourse bit SstFlashSectorErase( unsigned long FlashAddr )		small;
	SstFlashSourse bit SstFlashIsSectorIsNull( unsigned long FlashAdd )		small;
	SstFlashSourse bit SstFlashByteProgram( unsigned long FlashAddr , unsigned char ch )		small;
	SstFlashSourse bit SstFlashStringProgam( unsigned long FlashAddr , unsigned char idata* RamAddr , unsigned char len )		small;
//	SstFlashSourse bit SstFlashWriteString( unsigned long FlashAddr , unsigned char idata* RamAddr , unsigned char len )		small; 
	SstFlashSourse bit SstFlashReadString( unsigned long FlashAddr , unsigned char idata* RamAddr , unsigned char len )		small; 

#endif