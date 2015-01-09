#ifndef _DS12887_
#define _DS12887_
#ifndef DS12887_C
	#define Ds12887Sourse extern
#else
	#define Ds12887Sourse
#endif
	Ds12887Sourse bit Ds12887Init( void )		small;
//	Ds12887Sourse bit Ds12887WriteTime( struct time idata* TimeCache )		small;
//	Ds12887Sourse bit Ds12887ReadTime( struct time idata* TimeCache )		small;

#endif