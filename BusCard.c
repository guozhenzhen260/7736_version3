#define  __BUSCARD_C__
#include "BusCard.h"
#include "Serial1.h"
#include "global.h"
#include "debug.h" 
#include "device.h"
#include "string.h"
#include "common.h"
#include "scheduler.h"


unsigned char xdata BCMsgBuf[255];  

SERAIL1PARAM  code BCSerialParam =
{
	8, VERIFY_NULL, 1, 0x00, 0x96, 0x00
};

void BCSerialInit( void )
{	
	unsigned char xdata i=0;
	//DelayMs(1);
	ZhkHardSerialSwitch( 1 );
	//DelayMs(1);
	for( i=0; i<sizeof( SERAIL1PARAM ); i ++ )
	{
		ZhkSerial1Param[ i ] = BCSerialParam[ i ];
	}
	ZhkSerial1Init();
    memset( BCMsgBuf, 0, sizeof(BCMsgBuf) );   
}


//CRC caculate  
unsigned int BC_CalCRC( unsigned char *ptr, unsigned char len )
{
	unsigned char i;
    unsigned int  crc = 0;
    while( len-- != 0 )
    {
    	for( i=0x80; i!=0; i/=2 )
        {
        	if( (crc&0x8000)!=0 )
            {
            	crc *= 2;
                crc ^= 0x1021;
            }
            else       //余式CRC乘以2再求CRC
            {
            	crc *= 2;
			}

            if((*ptr&i)!=0)
            {
                crc ^= 0x1021;       //再加上本位的CRC
            }   
        }
		ptr++;
    }
    return ( crc );
}


//由HEX转换成H-ASCII 或 由BCD转换成B-ASCII函数
unsigned char BC_Hex_to_HAscii( unsigned char hex )
{
    unsigned char xdata m = 0;
    m = hex >> 4;
    if( m < 0x0a )
    {
    	m += 0x30;
    }
    else
    {
    	m += 0x37;
    }
    return m;
}


unsigned char BC_Hex_to_LAscii( unsigned char hex )
{
	unsigned char xdata m = 0;
    m = hex & 0x0f;
    if( m < 0x0a )
    {
    	m += 0x30;
    }
    else
    {
    	m += 0x37;
    }
    return m;
}

//由H-ASCII转换成HEX 或 由B-ASCII转换成BCD函数
unsigned char BC_Ascii_to_Hex( unsigned char highBit, unsigned char lowBit )
{

	unsigned char xdata hexByte = 0;
    //
    if( ( highBit > 0x2f ) && ( highBit < 0x3a ) )
    {
    	highBit -= 0x30;
    }
    else
    {
    	highBit -= 0x37;    	
    }
    //
    if( ( lowBit > 0x2f ) && ( lowBit < 0x3a ) )
    {
    	lowBit -= 0x30;
    }
    else
    {
    	lowBit -= 0x37;
    }
    //
    hexByte = ( highBit << 4 ) | lowBit;
    return hexByte;
}


unsigned char BCBusUnpack( void )
{
	unsigned char xdata  i=0, j=0, k=0, m=0;
	unsigned char xdata  len = 0;
	unsigned char xdata  seq = 0;
	unsigned char xdata  position    = 0;
	unsigned int  xdata crcValueCal = 0;
	unsigned int  xdata crcValueRec = 0;
	unsigned char xdata crcByteL    = 0;
	unsigned char xdata crcByteH    = 0;
    unsigned char xdata len1 = 0;
	//unsigned char xdata newMsg[255];
	
	//return 1;
    //-------------------------------------------------
	/*
    #ifdef BC_DEBUG
        return 1;
    #endif
	*/
    //=================================================

	while( !ZhkSerial1IsRxBufNull() )
	{
		for( i=0; i<sizeof( BCMsgBuf )-1; i++ )
		{
			BCMsgBuf[i] = BCMsgBuf[i+1];
		}
		BCMsgBuf[sizeof(BCMsgBuf)-1] = ZhkSerial1GetCh();
	}

    for( i=0; i<=sizeof(BCMsgBuf)-10; i++ )
	{
	    //Check the STX
		if( BCMsgBuf[i] != BC_MSG_STX ) 
			continue;
	    //Check the len
		len  = BC_Ascii_to_Hex( BCMsgBuf[i+1], BCMsgBuf[i+2] );    
		len1 = len;
		if( !( (len==0x02) || (len==0x42) ) )
		    continue;
		if( i+2+len1+4+1 > sizeof( BCMsgBuf ) ) 
		    break;
	
		//Check the CHK
	    crcValueCal = BC_CalCRC( &BCMsgBuf[i+3], len1 );
        crcByteL    = BC_Ascii_to_Hex( BCMsgBuf[i+2+len1+1], BCMsgBuf[i+2+len1+2] );
		crcByteH    = BC_Ascii_to_Hex( BCMsgBuf[i+2+len1+3], BCMsgBuf[i+2+len1+4] );
		crcValueRec = crcByteL + crcByteH*256;
        if( crcValueCal != crcValueRec )
			break;
		if( BCMsgBuf[i+2+len1+5] != BC_MSG_ETX )
            break;
		//Save the message
        sysBCMission.recDatLen = len/2;       //get the actul data
		for( j=0; j<sysBCMission.recDatLen; j++ )
		{
			sysBCMission.receive.msg[j] = BC_Ascii_to_Hex( BCMsgBuf[i+3+2*j], BCMsgBuf[i+3+2*j+1] );
		}
		sysBCMission.recACK = sysBCMission.receive.msg[0];
		sysBCMission.balance = 0x00;
		if( len == 0x42 )
		{
			sysBCMission.balance = sysBCMission.receive.msg[10]+sysBCMission.receive.msg[11]*0x100+sysBCMission.receive.msg[12]*0x10000+sysBCMission.receive.msg[13]*0x1000000;
		}
		//------------------------------------------------
		sysBCMission.msgLen = 1+2+len+4+1;
		for( j=0; j<sysBCMission.msgLen; j++ )
		{
			sysBCMission.msgBuf[j] = BCMsgBuf[i+j];
		}
		memset( BCMsgBuf, 0, sizeof(BCMsgBuf) );
		return 1;
	}	
	return 0;
			
}

//Create the CHK and send the message
unsigned char BCBusTxMsg( void )
{
	unsigned char i = 0;
	unsigned char sendBuf[255];
	unsigned char sendLen = 0;
	unsigned char lenBuf[2];
	unsigned char crcBuf[4];

	//1.Pack the message
	memset( sendBuf, 0, sizeof(sendBuf) );
	sysBCMission.send.stx = BC_MSG_STX;
	lenBuf[0] = BC_Hex_to_HAscii( sysBCMission.send.len*2 );  //*2
	lenBuf[1] = BC_Hex_to_LAscii( sysBCMission.send.len*2 );  //*2  
	for( i=0; i<sysBCMission.send.len; i++ )
	{
    	sendBuf[2*i]    = BC_Hex_to_HAscii( sysBCMission.send.msg[i] );
		sendBuf[2*i+1]  = BC_Hex_to_LAscii( sysBCMission.send.msg[i] );
	}

    //sysBCMission.send.crc = BC_CalCRC( sysBCMission.send.msg, sysBCMission.send.len );
    sysBCMission.send.crc = BC_CalCRC( sendBuf, sysBCMission.send.len*2 );
  
	crcBuf[0] = BC_Hex_to_HAscii( sysBCMission.send.crc%256 );
	crcBuf[1] = BC_Hex_to_LAscii( sysBCMission.send.crc%256 );
	crcBuf[2] = BC_Hex_to_HAscii( sysBCMission.send.crc/256 );
	crcBuf[3] = BC_Hex_to_LAscii( sysBCMission.send.crc/256 );
    sysBCMission.send.etx = BC_MSG_ETX;

	//2.Send the message
    ZhkSerial1PutCh( sysBCMission.send.stx );
	//delay( BC_COM_DELAY );
    ZhkSerial1PutCh( lenBuf[0] );
	//delay( BC_COM_DELAY );
    ZhkSerial1PutCh( lenBuf[1] );
	//delay( BC_COM_DELAY );
	for( i=0; i<sysBCMission.send.len*2; i++ )
	{
		ZhkSerial1PutCh( sendBuf[i] );
		//delay( ITL_COM_DELAY );	
	}

	ZhkSerial1PutCh( crcBuf[0] );
	//delay( ITL_COM_DELAY );
    ZhkSerial1PutCh( crcBuf[1] );
	//delay( ITL_COM_DELAY );
    ZhkSerial1PutCh( crcBuf[2] );
	//delay( ITL_COM_DELAY );
    ZhkSerial1PutCh( crcBuf[3] );
	//delay( ITL_COM_DELAY );

	ZhkSerial1PutCh( sysBCMission.send.etx );
	//delay( ITL_COM_DELAY );
    return 1;
}

unsigned char BCInqMsgPackSend( void )
{
   
   sysBCMission.send.len    = 0x01;
   sysBCMission.send.msg[0] = BC_CMD_INQ; 
   BCBusTxMsg();
   return BC_ERR_NULL;
	
}

unsigned char BCTradeMsgPackSend( unsigned int price, unsigned int sn )
{
    sysBCMission.send.len    = 0x0e;
	sysBCMission.send.msg[0] = BC_CMD_TRADE;
	sysBCMission.send.msg[1] = 0x00;
	//price
	sysBCMission.send.msg[2] = price%256;
	sysBCMission.send.msg[3] = price/256;
	sysBCMission.send.msg[4] = 0x00;
	sysBCMission.send.msg[5] = 0x00;
	//sn
    sysBCMission.send.msg[6]  = 0x00;   //0x01 for test
	sysBCMission.send.msg[7]  = 0x00;
	sysBCMission.send.msg[8]  = 0x00;
	sysBCMission.send.msg[9]  = 0x00;
	sysBCMission.send.msg[10] = 0x00;
    sysBCMission.send.msg[11] = 0x00;
	//
	sysBCMission.send.msg[12] = sn/256;
    sysBCMission.send.msg[13] = sn%256;
	//sysBCMission.send.msg[12] = 0x00;
    //sysBCMission.send.msg[13] = 0x00;
    
    BCBusTxMsg();
	return BC_ERR_NULL;


}

unsigned char BCMission_Inq( void )
{
    unsigned char retry = 0;
    unsigned char i = 0;
    unsigned char flag = 0;   
    //1.
    BCSerialInit();
    //2.
    retry = BC_COM_RETRY;
TAB_BCM_INQ_RETRY:
    flag = BCInqMsgPackSend();
    if( flag != BC_ERR_NULL )
    {
		return BC_ERR_PAR;
	}
    //sysBCMission.msTimer2 = SystemParameter.BCReadTime*1000/10;
    sysBCMission.msTimer2 = 300;
	while( sysBCMission.msTimer2 )   
	{
		if( BCBusUnpack() )
		{
		    sysBCMission.comErrNum = 0;
			goto TAB_BCM_INQ_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_BCM_INQ_RETRY;
	}
	else
	{
		sysBCMission.comErrNum++;
		if( sysBCMission.comErrNum >= BC_TIME_OUT_NUM )
		{
			DeviceStatus.BusCard |= BC_DEV_ERR_COM;
		}
        return BC_ERR_COM;
	}
TAB_BCM_INQ_COM_OK:

    DeviceStatus.BusCard &= ~BC_DEV_ERR_COM;

    if( sysBCMission.recACK == BC_ACK_OK )
	{
		DeviceStatus.BusCard = BC_DEV_ERR_NULL;
	}
	else if( (sysBCMission.recACK==BC_ACK_NOSIGNED)||(sysBCMission.recACK==BC_ACK_COMMERR)||(sysBCMission.recACK==BC_ACK_DEVERR) )
	{
	    DeviceStatus.BusCard |=  BC_DEV_ERR_HARDWARE;
	}
	else if( (sysBCMission.recACK==BC_ACK_BUSY) )
    {
    	DeviceStatus.BusCard |= BC_DEV_ERR_BUSY;
    }
	else
    {
	    DeviceStatus.BusCard = BC_DEV_ERR_NULL;
	}
    return BC_ERR_NULL;
}


unsigned char BCMission_Trade( unsigned int price, unsigned int sn )
{
    unsigned char retry = 0;
    unsigned char i = 0;
    unsigned char flag = 0;   
    //1.
    BCSerialInit();
    //2.
    retry = BC_COM_RETRY;
TAB_BCM_TRADE_RETRY:
    flag = BCTradeMsgPackSend( price, sn );
    if( flag != BC_ERR_NULL )
    {
		return BC_ERR_PAR;
	}
    //sysBCMission.msTimer2 = SystemParameter.BCReadTime*1000/10;
	sysBCMission.msTimer2 = 300;
	while( sysBCMission.msTimer2 )   
	{
		if( BCBusUnpack() )
		{
		    sysBCMission.comErrNum = 0;
			goto TAB_BCM_TRADE_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_BCM_TRADE_RETRY;
	}
	else
	{
		sysBCMission.comErrNum++;
		if( sysBCMission.comErrNum >= BC_TIME_OUT_NUM )
		{
			DeviceStatus.BusCard |= BC_DEV_ERR_COM;
		}
        return BC_ERR_COM;
	}
TAB_BCM_TRADE_COM_OK:
    DeviceStatus.BusCard &= ~BC_DEV_ERR_COM;
    if( sysBCMission.recACK == BC_ACK_OK )
	{
		DeviceStatus.BusCard = BC_DEV_ERR_NULL;
	}
	else if( (sysBCMission.recACK==BC_ACK_NOSIGNED)||(sysBCMission.recACK==BC_ACK_COMMERR)||(sysBCMission.recACK==BC_ACK_DEVERR) )
	{
	    DeviceStatus.BusCard |=  BC_DEV_ERR_HARDWARE;
	}
	else if( (sysBCMission.recACK==BC_ACK_BUSY) )
    {
    	DeviceStatus.BusCard |= BC_DEV_ERR_BUSY;
    }
	else
    {
	    DeviceStatus.BusCard = BC_DEV_ERR_NULL;
	}
    return BC_ERR_NULL;
}





