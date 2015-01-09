#undef _DEBUG_TRACE
#define PROCOTOL_C
#include "debug.h" 
#include "device.h"
#include "serial1.h"
#include "procotol.h" 
#include "scheduler.h"
#include "string.h"
#include "Coiner.h"
#include "Casher.h"
#include "vti.h"

#define CMDHEAD_7652Hub 0xEE    //Added by Andy Zhang on Jan 17,2007
#define ACKHEAD_7652Hub 0xFE

unsigned char xdata DownMsgBuf[ 64 ];
unsigned char xdata	GoodsPacketLen = 0;
bit	 data GoodsWaitACKLog = 0;

SERAIL1PARAM code ZhkBusSerialParam = 
{ 
	8 , VERIFY_NULL , 1 , 0x00 , 0x96 , 0x00 	
};
void ZhkBusSerialInit( void )
{
	unsigned char data i;
	for( i = 0 ; i < sizeof( SERAIL1PARAM ) ; i ++ )
		ZhkSerial1Param[ i ] = ZhkBusSerialParam[ i ];
	ZhkSerial1Init();
	memset( DownMsgBuf, 0, sizeof( DownMsgBuf ) );
}

/*
//关闭宝联硬币器电源包
unsigned char ZyzBusTxMsg( struct COMMTASK xdata* Task )
{
	unsigned char  data i;
	unsigned char  data len;
	unsigned char  data sum;
	unsigned char xdata DataString[ 64 ];
	
//	Trace( "\n ZhkBusTxMsg" );
	len = Task->ParamLen + 6;
	DataString[ 0 ] = CMDHEAD_7652Hub;
	DataString[ 1 ] = len;
	DataString[ 2 ] = Task->Sn;
//	Trace( "\n Task->Sn = %bu", Task->Sn );
	DataString[ 3 ] = Task->Cmd;
	DataString[ 4 ] = Task->Id;
	for( i = 0 ; i <= len - 6 ; i ++ )
		DataString[ 5 + i ] = Task->Param[ i ];	
	sum = 0;
	for( i = 0 ; i < DataString[ 1 ] - 1 ; i ++ )
	{
		sum ^= DataString[ i ];
	}
	DataString[ len - 1 ] = sum;	
	for( i = 0 ; i < len ; i ++ )		
		ZhkSerial1PutCh( DataString[ i ] );			
	return 1;
}
*/
unsigned char ZhkBusTxMsg( struct COMMTASK xdata* Task )
{
	unsigned char  data i;
	unsigned char  data len;
	unsigned char  data sum;
	unsigned char xdata DataString[ 64 ];
	
//	Trace( "\n ZhkBusTxMsg" );
	len = Task->ParamLen + 6;
	DataString[ 0 ] = CMDHEAD;
	DataString[ 1 ] = len;
	DataString[ 2 ] = Task->Sn;
//	Trace( "\n Task->Sn = %bu", Task->Sn );
	DataString[ 3 ] = Task->Cmd;
	DataString[ 4 ] = Task->Id;
	for( i = 0 ; i <= len - 6 ; i ++ )
		DataString[ 5 + i ] = Task->Param[ i ];	
	sum = 0;
	for( i = 0 ; i < DataString[ 1 ] - 1 ; i ++ )
	{
		sum ^= DataString[ i ];
	}
	DataString[ len - 1 ] = sum;	
	for( i = 0 ; i < len ; i ++ )		
		ZhkSerial1PutCh( DataString[ i ] );			
	
#ifdef _DEBUG_TRACE
	if( Task->Id == ID_CHANNEL )//ID_HOPPER2
	{
		Trace( "\n Bus Snd str ");	
		for( i = 0 ; i < len ; i ++ )
			Trace( " %02bx,",DataString[ i ] );
		Trace( "\n" );
	}
#endif	

	return 1;
}

unsigned char ZhkBusFrameUnPack( struct COMMTASK xdata* Result  )
{
	unsigned char data i,j;
	unsigned char data len;
	unsigned char data sum;
	

	j = 0;
	while( ! ZhkSerial1IsRxBufNull() )
	{
		for( i = 0 ; i < sizeof( DownMsgBuf ) - 1 ; i ++ )
			DownMsgBuf[ i ] = DownMsgBuf[ i + 1 ];
		DownMsgBuf[ sizeof( DownMsgBuf ) - 1 ] = ZhkSerial1GetCh();
#ifdef _DEBUG_TRACE
		j ++ ;
#endif
	}
/*
#ifdef _DEBUG_TRACE
	if( j )
	{
		Trace( "\n Bus Return Str = ");
		for( j = 0; j < 32; j++ )
			Trace( " %02bx", DownMsgBuf[32+j] );
		j = 0;
	}
#endif
*/
	for( i = 0 ; i <= sizeof( DownMsgBuf ) - 6 ; i ++ )
	{
		if ( DownMsgBuf[ i ] != ACKHEAD ) 
			continue;	
		len = DownMsgBuf[ i + 1 ];	
		if ( i + len > sizeof( DownMsgBuf ) )
			break;	 
		sum = 0;
		for( j = i ; j < i + len ; j ++ )		
		{
			sum ^= DownMsgBuf[ j ];		
		}
		if ( sum != 0 )		
			continue;
	
		//完整的数据包
		if( ( DownMsgBuf[ i + 4 ] == ID_HOPPER1 ) || ( DownMsgBuf[ i + 4 ] == ID_HOPPER2 ) || ( DownMsgBuf[ i + 4 ] == ID_HOPPER3 ) || ( DownMsgBuf[ i + 4 ] == ID_SELECTION1 ) || ( DownMsgBuf[ i + 4 ] == ID_SELECTION2 ) || ( DownMsgBuf[ i + 4 ] == ID_SELECTION3 ) || ( DownMsgBuf[ i + 4 ] == ID_ACDCMODULE ) )
		{
            /*-------------------------------------------------
            //Disabled for the VMC reset by watch dog
			if( Result->Sn != DownMsgBuf[ i+2 ] )
			{
			#ifdef _DEBUG_TRACE						
				Trace( "\n Bus Return Unpack SN is error = ");
				for( j = i; j < i + len; j++ )
					Trace( " %02bx", DownMsgBuf[j] );					
			#endif	
				memset( Result, 0, sizeof( Result ) );
				sum = 0;
				for( sum = i; sum < i + len; sum ++ )
					DownMsgBuf[sum] = 0x00;
				return 0;
			}
            ==================================================*/
		}
		memset( Result, 0, sizeof( Result ) );
		Result->ParamLen = len - 6;
		Result->Sn       = DownMsgBuf[ i + 2 ];
		Result->State    = DownMsgBuf[ i + 3 ];		
		Result->Id       = DownMsgBuf[ i + 4 ];

		for( j = 0 ; j < len - 6 ; j ++ )		
			Result->Param[ j ] = DownMsgBuf[ i + j + 5 ];		
		sum = 0;
		for( sum = i; sum < i + len; sum ++ )
			DownMsgBuf[sum] = 0x00;
/*
#ifdef _DEBUG_TRACE		
		Trace( "\n Bus Return Unpack = ");
		for( j = 0; j < 32; j++ )
			Trace( " %02bx", DownMsgBuf[32+j] );		
#endif*/
		return 1;
	}	
	return 0;
}

unsigned char ZhkCasherFrameUnPack( struct COMMTASK xdata* Result )
{
	unsigned char  xdata i,j,sum;
	unsigned char  xdata buf[ 3 ];
	
	
//	Trace( "\n Get Cash Return Begin ");
//	Trace( "\n Cash Return Str = ");
	j = 0;
	while( ! ZhkSerial1IsRxBufNull() )
	{
		for( i = 0 ; i <= sizeof( DownMsgBuf ) - 1 ; i ++ )
			DownMsgBuf[ i ] = DownMsgBuf[ i + 1 ];
		DownMsgBuf[ sizeof( DownMsgBuf ) - 1 ] = ZhkSerial1GetCh();
	//	Trace( " %02bx", DownMsgBuf[ sizeof( DownMsgBuf ) - 1 ] );
#ifdef _DEBUG_TRACE
		j ++ ;
#endif
	}		
/*
#ifdef _DEBUG_TRACE
	if( j )
	{
		Trace( "\n Casher Return Str = ");
		for( j = 0; j < 32; j++ )
			Trace( " %02bx", DownMsgBuf[32+j] );
		j = 0;
	}
#endif	
*/
//	Trace( "\n Get Cash Return End ");
	while( 1 )
	{
		for( i = 0; i <= sizeof( DownMsgBuf ) - 0x0B; i ++ )
		{
			if( ( DownMsgBuf[i] == VTISERIAL_FRAMEHEAD ) && ( DownMsgBuf[ i + 1 ] == 0x0B ) )
			{
				//找到包头
				break;
			}
		}
		if( i > sizeof( DownMsgBuf ) - 0x0B )//没有找到包头退出
			return 0;
		//找到包头处理	
		if ( DownMsgBuf[ i + 9 ] != VTISERIAL_FRAMETAIL ) 
		{
			//包头和包尾不配对,应该乱包，除去包头标志
		//	for( sum = i; sum < sizeof( DownMsgBuf ); sum ++ )
		//		DownMsgBuf[sum] = DownMsgBuf[sum + 2 ];
		   	DownMsgBuf[ i ] = 0x00;
		   	DownMsgBuf[ i + 1 ] = 0x00;		
		}
		else
			break;
	
		//包头、尾配对,查校验		
		sum = 0;
		for( j = i + 1 ; j < i + 8 ; j ++ )
		{
			sum ^= DownMsgBuf[ j ]; 		
		}
		if ( DownMsgBuf[ i + 10 ] != sum ) 
		{
			//包校验不对，乱包，除去包头
			sum = 0;
//			for( sum = i; sum < sizeof( DownMsgBuf ); sum ++ )
//				DownMsgBuf[sum] = DownMsgBuf[sum + 2 ];
		   	DownMsgBuf[ i ] = 0x00;
		   	DownMsgBuf[ i + 1 ] = 0x00;		
		}
		else
			break;
	}
	//是完整而正确的包
	#ifdef _DEBUG_TRACE
		Trace( "\n Cash Return OK Str");
		for( j = i; j <= i + 10 ; j ++ )		
			Trace( " 0x%02bx",DownMsgBuf[ j ] );	
		Trace( "\n" );
	#endif
	buf[ 0 ] = 0;
	buf[ 1 ] = 0;
	buf[ 2 ] = 0;
	
	if ( DownMsgBuf[ i + 3 + 0 ] & 0x04 ) buf[ 0 ] |= 0x10;//有钱进入
	if ( DownMsgBuf[ i + 3 + 0 ] & 0x10 ) buf[ 0 ] |= 0x20;//已存入
	if ( DownMsgBuf[ i + 3 + 0 ] & 0x40 ) buf[ 0 ] |= 0x80;//已返回
	if ( DownMsgBuf[ i + 3 + 2 ] & 0x02 ) buf[ 0 ] |= 0x04;//命令错误
	if ( DownMsgBuf[ i + 3 + 2 ] & 0x04 ) buf[ 0 ] |= 0x40;//故障
	buf[ 1 ]  = DownMsgBuf[ i + 3 + 1 ] & 0x7F;//纸币器和钱箱状态
	buf[ 2 ]  = DownMsgBuf[ i + 3 + 2 ] / 8;
	buf[ 2 ] &= 0x07;//收到的币种编号
	
//	Trace( "\n Result->State = %02bx", buf[ 0 ] );
	Result->State      = buf[ 0 ];//纸币状态
	Result->Param[ 0 ] = buf[ 1 ];//纸币器状态
	Result->Param[ 1 ] = buf[ 2 ];////收到的币种编号
	Result->ParamLen   = 3;
	sum = 0;
	for( sum = i; sum < i + 11; sum ++ )
		DownMsgBuf[sum] = 0x00;
//	for( sum = sizeof( DownMsgBuf ) - 11; sum < sizeof( DownMsgBuf ); sum ++ )
//		DownMsgBuf[sum] = 0x00;				
	return 1;	
}

/////////////////////////////////////////////////////////////////////
//	函数返回值注释													//
//0：继续等待;1：响应正确											//
//
//10：应答正确;														//
//11：应答表明序列号重复;调整序列后号可重发;						//
//12：应答表明发送了错误的命令或经校验是错误的命令;此次出货失败		//
//////////////////////////////////////////////////////////////////////
unsigned char ZhkChannelFrameUnPack( struct COMMTASK xdata* Result  )
{
	unsigned char xdata i;
	unsigned char xdata ret = 0;
	unsigned char xdata sum;
	unsigned char xdata j = 0;
//	unsigned char data packetHeard;	
		
	j = 0;
	while( ! ZhkSerial1IsRxBufNull() )
	{
		for( i = 0 ; i <= sizeof( DownMsgBuf ) - 1 ; i ++ )
			DownMsgBuf[ i ] = DownMsgBuf[ i + 1 ];
		DownMsgBuf[ sizeof( DownMsgBuf ) - 1 ] = ZhkSerial1GetCh();			
#ifdef _DEBUG_TRACE
		j++;
#endif
	}

#ifdef _DEBUG_TRACE
	if( j != 0 )
	{
		Trace( "\n Channel Return Str = " );		
		for( i = 0; i < j  ; i ++ )
			Trace( " %02bx", DownMsgBuf[ 64 - j + i ] );		
	}
	j = 0;
#endif

	//找应答帧
	for( i = 0; i <= sizeof( DownMsgBuf ) - 8 ; i++ ) 
	{			
		if( ( ( DownMsgBuf[ i ] == 0xee ) && ( DownMsgBuf[ i + 1 ] == 0x08 ) ) ) 				
		{
		//	Trace( "\n Channel Find Heard1" );		
		//	Trace( "\n Result->Sn = %bu, DownMsgBuf[ i + 2 ] = %bu",Result->Sn, DownMsgBuf[ i + 2 ]);		
			if( Result->Sn == DownMsgBuf[ i + 2 ] ) 
			{
			//	Trace( "\n Channel Find SN" );		
				//是本次的应答包				
				sum = 0;
				for( ret = 0; ret < 7; ret ++ )
					sum ^= DownMsgBuf[ i + ret ];			
				if( sum == DownMsgBuf[ i + 7 ] )//完整的响应包				
				{
					memset( Result, 0, sizeof( Result ) );
					Result->State = DownMsgBuf[ i + 5 ];//状态码
					Result->Param[0] = DownMsgBuf[ i + 4 ];//货道号
					Result->Param[1] = DownMsgBuf[ i + 2 ];//包序列号
					
				//	memset( DownMsgBuf + GoodsPacketLen - i - 8, 0, i + 8 );
				//	GoodsPacketLen = GoodsPacketLen - i - 8;	
			#ifdef _DEBUG_TRACE
					Trace( "\n Channel Return Unpack = " );		
					for( j = 0; j < 8  ; j ++ )
						Trace( " %02bx", DownMsgBuf[ i + j ] );							
			#endif
					//除去前面的所有字节应答包
					for( sum = 0; sum < i + 8; sum++ )
						DownMsgBuf[ sum ] = 0x00;

					return 1;
				}													
			}											
		}
	}
	return 0;
}

//控制宝连硬币器的电源
/*
unsigned char CtrlCoinPowerFrameUnPack( struct COMMTASK xdata* Result  )
{
	unsigned char data i,j;
	unsigned char data len;
	unsigned char data sum;
	

	j = 0;
	while( ! ZhkSerial1IsRxBufNull() )
	{
		for( i = 0 ; i < sizeof( DownMsgBuf ) - 1 ; i ++ )
			DownMsgBuf[ i ] = DownMsgBuf[ i + 1 ];
		DownMsgBuf[ sizeof( DownMsgBuf ) - 1 ] = ZhkSerial1GetCh();
#ifdef _DEBUG_TRACE
		j ++ ;
#endif
	}

#ifdef _DEBUG_TRACE
	if( j )
	{
		Trace( "\n CtrlCoinPowerFrameUnPack Bus Return Str = ");
		for( j = 0; j < 32; j++ )
			Trace( " %02bx", DownMsgBuf[32+j] );
		j = 0;
	}
#endif

	for( i = 0 ; i <= sizeof( DownMsgBuf ) - 6 ; i ++ )
	{
		if ( DownMsgBuf[ i ] != ACKHEAD_7652Hub ) 
			continue;	
		len = DownMsgBuf[ i + 1 ];	
		if ( i + len > sizeof( DownMsgBuf ) )
			break;	 
		sum = 0;
		for( j = i ; j < i + len ; j ++ )		
		{
			sum ^= DownMsgBuf[ j ];		
		}
		if ( sum != 0 )		
			continue;
	

		memset( Result, 0, sizeof( Result ) );
		Result->ParamLen = len - 6;
		Result->Sn       = DownMsgBuf[ i + 2 ];
		Result->State    = DownMsgBuf[ i + 3 ];		
		Result->Id       = DownMsgBuf[ i + 4 ];


		for( j = 0 ; j < len - 6 ; j ++ )		
			Result->Param[ j ] = DownMsgBuf[ i + j + 5 ];		
		sum = 0;
		for( sum = i; sum < i + len; sum ++ )
			DownMsgBuf[sum] = 0x00;
	//	for( sum = sizeof( DownMsgBuf ) - len; sum < sizeof( DownMsgBuf ); sum ++ )
	//		DownMsgBuf[sum] = 0x00;				
//		for( j = 0 ; j < sizeof( DownMsgBuf ) ; j ++ )
//			DownMsgBuf[ j ] = 0;
        Trace( "ZhkHubRevDataOK\n" );
		return 1;
	}	
	return 0;
}
*/
/*
unsigned char CoinerFrameUnPack( struct COMMTASK xdata* Result )
{
	unsigned char  xdata i;
	unsigned char  xdata j;
	unsigned char  xdata sum;
 	unsigned char  xdata len;	

	j = 0;
	while( ! ZhkSerial1IsRxBufNull() )
	{
		for( i = 0 ; i <= sizeof( DownMsgBuf ) - 1 ; i ++ )
			DownMsgBuf[ i ] = DownMsgBuf[ i + 1 ];
		DownMsgBuf[ sizeof( DownMsgBuf ) - 1 ] = ZhkSerial1GetCh();	
#ifdef _DEBUG_TRACE
		j++;	
#endif
	}
	while( 1 )
	{
		for( i = 0; i <= sizeof( DownMsgBuf ) - 0x03; i ++ )
		{
			if( DownMsgBuf[i] == HYKSERIAL_FRAMEHEAD )
				//找到包头
				break;			
		}
	//	Trace( "\n Find Pack head1 ");
		if( i > sizeof( DownMsgBuf ) - 0x03 )//没有找到包头退出
			return 0;
		
		len = DownMsgBuf[i + 1];
		if( len == 6 )
			len = 5;
		if( ( i + len ) > sizeof( DownMsgBuf ) )//找到包头,但包不完整,去掉前面的数据				
			return 0;
		
		//查校验		
		sum = 0;
		for( j = i; j < ( i + len - 1 ); j ++ )		
			sum += DownMsgBuf[ j ]; 		
		
	//	Trace( "\n sum = %02bx", sum );	
		if ( DownMsgBuf[ i + len - 1 ] != sum ) 
		{
			//包校验不对，乱包，除去包头
			sum = 0;
			for( sum = i; sum < sizeof( DownMsgBuf ); sum ++ )
				DownMsgBuf[sum] = DownMsgBuf[sum + 1 ];		   	
		   	DownMsgBuf[ sum - 1 ] = 0x00;			
		}
		else
			break;
	}
	//是完整而正确的包
	j = 0;
	len = 0;
	sum = 0;
//	Trace("\n Result->Cmd = %02bx", Result->Cmd );
	if( Result->Cmd == COINER1_QUERY )//查询
	{
	//	Trace( "\n Cash Query1");
		if( ( DownMsgBuf[i + 2] != 0x61 ) && ( DownMsgBuf[i + 3] != 0x76 ) )
		{
			//状态查询成功
			j = 1;
			len = DownMsgBuf[i + 2];
			sum = DownMsgBuf[i + 3];			
		//	if( ( len == 0x11 ) || ( len == 0x12 ) )
		//		sum &= 0x07;			
	//	   Trace( "\n Cash Query2");
		}
		else
			j = 2;//无效
	}	
	else
	{
		if( ( DownMsgBuf[i + 2] == 0x61 ) && ( DownMsgBuf[i + 3] == 0x76 ) )//无效的命令
			j = 2;//无效
		else		
			j = 1;//有效		
	}
//	Trace( "\n Cash Query3");	
	Result->State      = j;//硬币器回应
	Result->Param[ 0 ] = len;//硬币器状态
	Result->Param[ 1 ] = sum;////收到的币种编号
	Result->ParamLen   = 3;
	sum = 0;
	len = DownMsgBuf[i + 1];
	if( len == 6 )
		len = 5;
	for( sum = i; sum < i + 5; sum ++ )
		DownMsgBuf[sum] = 0x00;
//	for( ; sum < sizeof( DownMsgBuf ); sum ++ )
//		DownMsgBuf[sum] = 0x00;				
//	for( sum = i; sum < sizeof( DownMsgBuf ); sum ++ )
//		DownMsgBuf[sum] = DownMsgBuf[sum + len ];
//	for( ; sum < sizeof( DownMsgBuf ); sum ++ )
//		DownMsgBuf[sum] = 0x00;				
	return 1;	
}
*/
unsigned char ZhkChkAddr( unsigned char Addr )
{
	switch( Addr )
	{
	case ID_HOPPER1:
	case ID_HOPPER2:
	case ID_HOPPER3:
	case ID_HOPPER4:
	case ID_HOPPER5:
	case ID_HOPPER6:
	case ID_HOPPER7:
	case ID_HOPPER8:
		return EQUIP_HOPPER;
	case ID_SELECTION1:
	case ID_SELECTION2:
	case ID_SELECTION3:
	    return EQUIP_SELECTION;
    case ID_ACDCMODULE:
        return EQUIP_ACDCMODULE;
//	case ID_HUB:
//		return EQUIP_HUB;
	case ID_LCD:
		return EQUIP_LCD;
	case ID_POWER:
		return EQUIP_POWER;
	case ID_MODE:
		return EQUIP_MODE;
	case ID_PRN:
		return EQUIP_PRN;
	case ID_CASHER:
		return EQUIP_CASHER;
	case ID_COINER1:
	case ID_COINER2:
	case ID_COINER3:
	case ID_COINER4:
		return EQUIP_COINER;
	case ID_MESSAGER:
		return EQUIP_MESSAGER;
	case ID_SERVER:
		return EQUIP_SEVER;
	case ID_CHANNEL:
		return EQUIP_CHANNEL;
//	case ID_LEDSCR:
//		return EQUIP_LEDSCR;
	case ID_NULL:
	default:
		return EQUIP_NULL;
	}
}
