////////////////////////////////////////////////////////////////////////////////////////////////
//
//                         串口2驱动程序     
//                  工作于华邦系列mcs51(w77e532)
//
//
////////////////////////////////////////////////////////////////////////////////////////////////
//#undef _DEBUG_TRACE
#define ZHKSERIAL1_C
#ifndef ZHK_INTERRUPT
#define ZHK_INTERRUPT
#endif
#include "device.h"
#include "Serial1.h"
#include "debug.h"
#include "common.h"
#include "global.h"

#define ZHKSERIAL1RXBUFLEN 	128
#define ZHKSERIAL1TXBUFLEN  	128


ZhkSerial1Sourse SERAIL1PARAM xdata ZhkSerial1Param = 
{ 
	8 , VERIFY_NULL , 1 , 0x00 , 0x96 , 0x00 	
};

#define ZhkBaudRate 	ZhkSerial1Param[ 4 ]
#define ZhkDataBit  	ZhkSerial1Param[ 0 ]
#define ZhkVerifyBit 	ZhkSerial1Param[ 1 ]
#define ZhkStopBit   	ZhkSerial1Param[ 2 ]

uchar xdata ZhkSerial1RxBuf[ ZHKSERIAL1RXBUFLEN ];
uchar data ZhkSerial1RxBufWd;
uchar data ZhkSerial1RxBufRd;

bit ZhkSerial1TxdEmpty;

uchar ZhkSerial1IsRxBufNull( void );
//uchar ZhkSerial1IsRxFullIsr( void );
uchar ZhkSerial1IsTxBufNull( void );
uchar ZhkSerial1IsTxBufNullIsr( void );





uchar ZhkSerial1Init( void )
{
	uint data dat;
	 data ch;
	S1STAT = 0x20 ; // 区分接收和发送中断
	S1CON = 0x00 ; //串口控制寄存器清零
	REN_1 = 1 ; //允许接收中断
	{ //进入8 位UART 模式设置
	SM0_1 = 0 ;
	SM1_1 = 1 ; //设定串口通讯方式为模式1
	}
	if ( ZhkVerifyBit != VERIFY_NULL && ZhkDataBit == 8 )
	{
		SM0_1 = 1;
	}
	// ZhkBaudRate 是波特率的 千位数和百位数组成的BCD码 
	ch       = ZhkBaudRate;
	if ( ch > 0x99 || ch % 16 > 9 ) return 0 ;
	
	dat      = ( ch - ch / 16 * 6 ) * 100;
	dat      = OSCRATE / dat - 16;
	BRGCON = 0X00 ; //关闭波特率发生器，从而设置波特率
	BRGR1 = dat >> 8 ;
	BRGR0 = dat & 0x0ff ;
	BRGCON = 0X01 ; //开启波特率发生器
	
	ES1R = 1 ; // 开放UART1 接收中断
	TI_1 = 1 ;
	DelayMs( 3 );
	ZhkSerial1RxBufWd = 0;
	ZhkSerial1RxBufRd = 0;
	return 1;
}

uchar ZhkSerial1PutCh( uchar ch )
{
//	uint data i;
	bit veri;
//	Trace( "ZhkSerial1PutCh(%bx)",ch );
	while( ! TI_1 )
	TI_1  = 0;
	delay( 250 );
	delay( 250 );
	ACC   = ch;
	veri  = P;
	if ( ZhkVerifyBit == VERIFY_ODD  ) veri = ! veri;
	if ( ZhkVerifyBit == VERIFY_NULL ) veri = 1;
	if ( ZhkDataBit < 8 )
	{
		if ( veri ) ch |= 1 << 7;
	}
	TB8_1 = veri;
	S1BUF = ch;
	return 0;
}
uchar ZhkSerial1GetCh( void )
{
	uchar data ch;
	ch = ZhkSerial1RxBuf[ ZhkSerial1RxBufRd ];
	if ( ZhkDataBit < 8 )
	{
		ch &= ~( 1 << 7 );
	}
	ZhkSerial1RxBufRd ++;
	if ( ZhkSerial1RxBufRd >= ZHKSERIAL1RXBUFLEN ) ZhkSerial1RxBufRd = 0;
	ZhkHardSetRTS();
//	Trace( "ZhkSerial1GetCh=%bxH\n",ch );
	return ch;
	
}
/*
uchar ZhkSerial1TxMsg( uchar xdata* TxBuf , uchar len )
{
	uchar data i;
	
	if ( len > ZHKSERIAL1TXBUFLEN ) return 1;
	if ( ! ZhkSerial1IsTxBufNull() ) return 1;
	
	ZhkSerial1TxBufWd = 0;
	ZhkSerial1TxBufRd = 0;
	for( i = 0 ; i < ZHKSERIAL1TXBUFLEN ; i ++ )
		ZhkSerial1TxBuf[ ZhkSerial1TxBufWd ++ ] = TxBuf[ i ];
	ZhkSerial1PutCh( ZhkSerial1TxBuf[ ZhkSerial1TxBufRd ++ ] );
	return 1;
}
*/
/////////////////////////////////////////////////////////////////////////////////////
uchar ZhkSerial1IsRxBufNull( void )
{
	if  ( ZhkSerial1RxBufRd - ZhkSerial1RxBufWd == 0 ) return 1;
	return 0;
}

////ygp close //////////////
/*
uchar ZhkSerial1IsRxFullIsr( void ) using 1
{

	if ( ( ZhkSerial1RxBufRd - ZhkSerial1RxBufWd ) % ZHKSERIAL1RXBUFLEN == 1 )
		return 1;
	return 0;
}
*/
/////////////////////////////
/*
uchar ZhkSerial1IsTxBufNull( void )
{
	if ( ZhkSerial1TxBufRd - ZhkSerial1TxBufWd == 0 ) return 1;;
	return  0;
}

uchar ZhkSerial1IsTxBufNullIsr( void ) using 1
{
	if ( ZhkSerial1TxBufRd - ZhkSerial1TxBufWd == 0 ) return 1;
	return 0;
}
*/
void ZhkSerial1_Isr( void ) interrupt 10 using 1 
{
	ZhkSaveChipSelect();
	ZhkHardSramSelect();
/*
	if ( TI_1 )
	{
		TI_1 = 0;
		ZhkSerial1TxdEmpty = 1;
	}
*/
	if ( RI_1 )
	{
		RI_1 = 0;
		ZhkSerial1RxBuf[ ZhkSerial1RxBufWd ] = S1BUF;
		ZhkSerial1RxBufWd ++;
		if ( ZhkSerial1RxBufWd >= ZHKSERIAL1RXBUFLEN )
			ZhkSerial1RxBufWd = 0;
		//--------------------------------------------
		if( RB8_1 == 1 )
		{
		    sysMDBMission.modeBit = 1;
		}
		else
		{
		    sysMDBMission.modeBit = 0;
		}
		//============================================
	}
	ZhkLoadChipSelect();

}


uchar ZhkSerial1_MDB_Init( void )
{
	uint  data dat;
	//uchar 
    data ch;

	S1STAT = 0x20; // 区分接收和发送中断
	S1CON = 0x00;  //串口控制寄存器清零
	REN_1 = 1 ;    //允许接收中断
	{   //进入8 位UART 模式设置
		SM0_1 = 1 ;
		SM1_1 = 1 ; //设定串口通讯方式为模式3
	}
	// ZhkBaudRate 是波特率的 千位数和百位数组成的BCD码 
	ch = ZhkBaudRate;
	if ( ch > 0x99 || ch % 16 > 9 ) return 0 ;
	dat      = ( ch - ch / 16 * 6 ) * 100;
	dat      = OSCRATE / dat - 16;
	BRGCON = 0X00 ; //关闭波特率发生器，从而设置波特率
	BRGR1 = dat >> 8 ;
	BRGR0 = dat & 0x0ff ;
	BRGCON = 0X01 ; //开启波特率发生器
	
	ES1R = 1 ; // 开放UART1 接收中断
	TI_1 = 1 ;
	DelayMs( 3 );
	ZhkSerial1RxBufWd = 0;
	ZhkSerial1RxBufRd = 0;
	return 1;

}

uchar ZhkSerial1_MDB_PutCh( uchar ch,uchar mode)
{
    
    //uint data i;
	//bit veri;

	while( !TI_1 )
	TI_1  = 0;
	delay( 250 );
	//delay( 250 );
	//ACC   = ch;
	if( mode == 0 )
    {
		TB8_1 = 0;  
    }
    else
    {
    	TB8_1 = 1;
    }	
	S1BUF = ch;
    delay( 250 );
	return 0;

}
uchar ZhkSerial1_MDB_GetCh( uchar *byte, uchar *bMode )
{
	uchar data ch;

    sysMDBMission.MDBMSTimer = 3;
	while( (ZhkSerial1IsRxBufNull())&&(sysMDBMission.MDBMSTimer>0) );
    if( sysMDBMission.MDBMSTimer == 0 ) return 0;
    
	ch = ZhkSerial1RxBuf[ ZhkSerial1RxBufRd ];
	ZhkSerial1RxBufRd ++;  
	if ( ZhkSerial1RxBufRd >= ZHKSERIAL1RXBUFLEN ) ZhkSerial1RxBufRd = 0;
	ZhkHardSetRTS();
    //Trace( "ZhkSerial1GetCh=%bxH\n",ch );
    //if( RB8_1==1 )
	if( sysMDBMission.modeBit == 1 )
	{
        *bMode = 1;
	}
	else
	{
	    *bMode = 0;
	}
	*byte = ch;
	return 1;

		
}
