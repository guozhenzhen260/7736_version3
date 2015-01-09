#ifndef _DATAEXCHANGE_
#define _DATAEXCHANGE_
#include "device.h"
#ifndef DATAEXCHANGE_C
	#define DataExchangeSourse extern
#else
	#define DataExchangeSourse
#endif
	DataExchangeSourse bit MovDataFromFlash( ulong FlashAddr , uchar idata* RamAddr , uchar len ) small;
	// 从Flash中移动一串数据到片内
	// 参数：
	// FlashAddr flash内部地址，取值范围与硬件结构相关
	// RamAddr   片内内存地址
	// len	     长度
	
	//DataExchangeSourse bit MovDataToFlash( ulong FlashAddr , uchar idata* RamAddr , uchar len ) small;
	// 从片内移动一串数据到Flash
	// 参数：
	// FlashAddr flash内部地址，取值范围与硬件结构相关
	// RamAddr   片内内存地址
	// len	     长度

	DataExchangeSourse bit MovDataFromSram( uchar xdata* SramAddr , uchar idata* RamAddr , uchar len );
	// 从片外内存移动一串数据到片外内存
	// SramAddr  片外内部地址，取值范围与硬件结构相关
	// RamAddr   片内内存地址
	// len	     长度
	
	DataExchangeSourse bit MovDataToSram( uchar xdata* SramAddr , uchar idata* RamAddr , uchar len );
	// 从片内内存移动一串数据到片外内存
	// SramAddr  片外内部地址，取值范围与硬件结构相关
	// RamAddr   片内内存地址
	// len	     长度
	
	
	DataExchangeSourse bit MovSramToFlash( ulong FlashAddr , uchar xdata* SramAddr , uint len ) small;
	// 从片外内存移动一串字符flash
	// FlashAddr flash内部地址，取值范围与硬件结构相关
	// SramAddr  片外内部地址，取值范围与硬件结构相关
	// len	     长度

	DataExchangeSourse bit MovFlashToSram( ulong FlashAddr , uchar xdata* SramAddr , uint len ) small;
	// 从flash移动一串字符到片外内存
	// FlashAddr flash内部地址，取值范围与硬件结构相关
	// SramAddr  片外内部地址，取值范围与硬件结构相关
	// len	     长度

	DataExchangeSourse bit PogramFlash( ulong FlashAddr , uchar xdata* SramAddr , uint len ) small;
	// 把片外内存中的一串字符写入到flash，没有经过擦除flash
	// FlashAddr flash内部地址，取值范围与硬件结构相关
	// SramAddr  片外内部地址，取值范围与硬件结构相关
	// len	     长度

	DataExchangeSourse bit MovSramToSram( uchar xdata* Sourse , uchar xdata* Object , uint len );
	// 在两段片外内存间移动字符串
	// Sourse    源地址
	// Object    目标地址
	// len       字符串的长度 
	
	DataExchangeSourse bit FlashDel( ulong FlashAddr , ulong len ) small;
	// 清除flash中的内容
	// FlashAddr表示清除区域的首地址
	// len 表示清除区域长度
	
	DataExchangeSourse uchar idata Cache[ 64 ];
	// 计算缓冲区
	
#endif