#define  __ITL_C__
#include "ITL.h"
#include "Serial1.h"
#include "global.h"
#include "debug.h" 
#include "device.h"
#include "string.h"
#include "common.h"
#include "scheduler.h"
#include "VMC_PC.h"
#include "IOInput.h"
//#include "CommonFunction.h"


//#define  ITL_DEBUG

unsigned char xdata ITLMsgBuf[255];  //255

SERAIL1PARAM  code ITLSerialParam =
{
	8, VERIFY_NULL, 1, 0x00, 0x96, 0x00
};

//const unsigned int code CrcTable[8*32]=
const unsigned int xdata CrcTable[8*32]=
{
   /*
   0x0000,0x8005,0x800F,0x000A,0x801B,0x001E,0x0014,0x8011,
   0x8033,0x0036,0x003C,0x8039,0x0028,0x802D,0x8027,0x0022,
   0x8063,0x0066,0x006C,0x8069,0x0078,0x807D,0x8077,0x0072,
   0x0050,0x8055,0x805F,0x005A,0x804B,0x004E,0x0044,0x8041,
   0x80C3,0x00C6,0x00CC,0x80C9,0x00D8,0x80DD,0x80D7,0x00D2,
   0x00F0,0x80F5,0x80FF,0x00FA,0x80EB,0x00EE,0x00E4,0x80E1,
   0x00A0,0x80A5,0x80AF,0x00AA,0x80BB,0x00BE,0x00B4,0x80B1,
   0x8093,0x0096,0x009C,0x8099,0x0088,0x808D,0x8087,0x0082,
   0x8183,0x0186,0x018C,0x8189,0x0198,0x819D,0x8197,0x0192,
   0x01B0,0x81B5,0x81BF,0x01BA,0x81AB,0x01AE,0x01A4,0x81A1,
   0x01E0,0x81E5,0x81EF,0x01EA,0x81FB,0x01FE,0x01F4,0x81F1,
   0x81D3,0x01D6,0x01DC,0x81D9,0x01C8,0x81CD,0x81C7,0x01C2,
   0x0140,0x8145,0x814F,0x014A,0x815B,0x015E,0x0154,0x8151,
   0x8173,0x0176,0x017C,0x8179,0x0168,0x816D,0x8167,0x0162,
   0x8123,0x0126,0x012C,0x8129,0x0138,0x813D,0x8137,0x0132,
   0x0110,0x8115,0x811F,0x011A,0x810B,0x010E,0x0104,0x8101,
   0x8303,0x0306,0x030C,0x8309,0x0318,0x831D,0x8317,0x0312,
   0x0330,0x8335,0x833F,0x033A,0x832B,0x032E,0x0324,0x8321,
   0x0360,0x8365,0x836F,0x036A,0x837B,0x037E,0x0374,0x8371,
   0x8353,0x0356,0x035C,0x8359,0x0348,0x834D,0x8347,0x0342,
   0x03C0,0x83C5,0x83CF,0x03CA,0x83DB,0x03DE,0x03D4,0x83D1,
   0x83F3,0x03F6,0x03FC,0x83F9,0x03E8,0x83ED,0x83E7,0x03E2,
   0x83A3,0x03A6,0x03AC,0x83A9,0x03B8,0x83BD,0x83B7,0x03B2,
   0x0390,0x8395,0x839F,0x039A,0x838B,0x038E,0x0384,0x8381,
   0x0280,0x8285,0x828F,0x028A,0x829B,0x029E,0x0294,0x8291,
   0x82B3,0x02B6,0x02BC,0x82B9,0x02A8,0x82AD,0x82A7,0x02A2,
   0x82E3,0x02E6,0x02EC,0x82E9,0x02F8,0x82FD,0x82F7,0x02F2,
   0x02D0,0x82D5,0x82DF,0x02DA,0x82CB,0x02CE,0x02C4,0x82C1,
   0x8243,0x0246,0x024C,0x8249,0x0258,0x825D,0x8257,0x0252,
   0x0270,0x8275,0x827F,0x027A,0x826B,0x026E,0x0264,0x8261,
   0x0220,0x8225,0x822F,0x022A,0x823B,0x023E,0x0234,0x8231,
   0x8213,0x0216,0x021C,0x8219,0x0280,0x820D,0x8207,0x0202,
   */
   0x0000,0x8005,0x800F,0x000A,0x801B,0x001E,0x0014,0x8011,
   0x8033,0x0036,0x003C,0x8039,0x0028,0x802D,0x8027,0x0022,
   0x8063,0x0066,0x006C,0x8069,0x0078,0x807D,0x8077,0x0072,
   0x0050,0x8055,0x805F,0x005A,0x804B,0x004E,0x0044,0x8041,
   0x80C3,0x00C6,0x00CC,0x80C9,0x00D8,0x80DD,0x80D7,0x00D2,
   0x00F0,0x80F5,0x80FF,0x00FA,0x80EB,0x00EE,0x00E4,0x80E1,
   0x00A0,0x80A5,0x80AF,0x00AA,0x80BB,0x00BE,0x00B4,0x80B1,
   0x8093,0x0096,0x009C,0x8099,0x0088,0x808D,0x8087,0x0082,
   0x8183,0x0186,0x018C,0x8189,0x0198,0x819D,0x8197,0x0192,
   0x01B0,0x81B5,0x81BF,0x01BA,0x81AB,0x01AE,0x01A4,0x81A1,
   0x01E0,0x81E5,0x81EF,0x01EA,0x81FB,0x01FE,0x01F4,0x81F1,
   0x81D3,0x01D6,0x01DC,0x81D9,0x01C8,0x81CD,0x81C7,0x01C2,
   0x0140,0x8145,0x814F,0x014A,0x815B,0x015E,0x0154,0x8151,
   0x8173,0x0176,0x017C,0x8179,0x0168,0x816D,0x8167,0x0162,
   0x8123,0x0126,0x012C,0x8129,0x0138,0x813D,0x8137,0x0132,
   0x0110,0x8115,0x811F,0x011A,0x810B,0x010E,0x0104,0x8101,
   0x8303,0x0306,0x030C,0x8309,0x0318,0x831D,0x8317,0x0312,
   0x0330,0x8335,0x833F,0x033A,0x832B,0x032E,0x0324,0x8321,
   0x0360,0x8365,0x836F,0x036A,0x837B,0x037E,0x0374,0x8371,
   0x8353,0x0356,0x035C,0x8359,0x0348,0x834D,0x8347,0x0342,
   0x03C0,0x83C5,0x83CF,0x03CA,0x83DB,0x03DE,0x03D4,0x83D1,
   0x83F3,0x03F6,0x03FC,0x83F9,0x03E8,0x83ED,0x83E7,0x03E2,
   0x83A3,0x03A6,0x03AC,0x83A9,0x03B8,0x83BD,0x83B7,0x03B2,
   0x0390,0x8395,0x839F,0x039A,0x838B,0x038E,0x0384,0x8381,
   0x0280,0x8285,0x828F,0x028A,0x829B,0x029E,0x0294,0x8291,
   0x82B3,0x02B6,0x02BC,0x82B9,0x02A8,0x82AD,0x82A7,0x02A2,
   0x82E3,0x02E6,0x02EC,0x82E9,0x02F8,0x82FD,0x82F7,0x02F2,
   0x02D0,0x82D5,0x82DF,0x02DA,0x82CB,0x02CE,0x02C4,0x82C1,
   0x8243,0x0246,0x024C,0x8249,0x0258,0x825D,0x8257,0x0252,
   0x0270,0x8275,0x827F,0x027A,0x826B,0x026E,0x0264,0x8261,
   0x0220,0x8225,0x822F,0x022A,0x823B,0x023E,0x0234,0x8231,
   0x8213,0x0216,0x021C,0x8219,0x0208,0x820D,0x8207,0x0202
 };

//-----------------------------------------------------------
void ITLUpdateCrc( const unsigned char num )
{
    unsigned char data addr = 0;
    addr = ( num ^ sysITLMission.crcH );
    sysITLMission.crcH = (unsigned char)(CrcTable[addr]>>8)^sysITLMission.crcL;
    sysITLMission.crcL = (unsigned char)(CrcTable[addr] & 0x00FF );
}

//-----------------------------------------------------------
void ITLResetCrc( void )
{
    sysITLMission.crcL = 0xFF;
    sysITLMission.crcH = 0xFF;
}

void ITLChnCtr()
{
    sysITLMission.chnCtr1 = 0;
	sysITLMission.chnCtr2 = 0;
    
    if( SystemParameter.billValue[0] != 0   )
	{
		sysITLMission.chnCtr1 |= 0x01;
	}
	if( SystemParameter.billValue[1] != 0   )
	{
		sysITLMission.chnCtr1 |= 0x02;
	}
	if( SystemParameter.billValue[2] != 0   )
	{
		sysITLMission.chnCtr1 |= 0x04;
	}
	if( SystemParameter.billValue[3] != 0   )
	{
		sysITLMission.chnCtr1 |= 0x08;
	}
	if( SystemParameter.billValue[4] != 0   )
	{
		sysITLMission.chnCtr1 |= 0x10;
	}
	if( SystemParameter.billValue[5] != 0   )
	{
		sysITLMission.chnCtr1 |= 0x20;
	}
	if( SystemParameter.billValue[6] != 0   )
	{
		sysITLMission.chnCtr1 |= 0x40;
	}
	if( SystemParameter.billValue[7] != 0   )
	{
		sysITLMission.chnCtr1 |= 0x80;
	}
    //---------------------------------------------------------
	#ifdef ITL_DEBUG
    	sysITLMission.chnCtr1 = 0xff;
		sysITLMission.chnCtr2 = 0xff;
	#endif
    //sysITLMission.chnCtr1 = 0xff;
	//sysITLMission.chnCtr2 = 0xff;
    //=========================================================

}

void ITLSerialInit( void )
{	
	unsigned char data i=0;
    
	//DelayMs(1);
	ZhkHardSerialSwitch( 0 );
	//DelayMs(1);
	for( i=0; i<sizeof( SERAIL1PARAM ); i ++ )
	{
		ZhkSerial1Param[ i ] = ITLSerialParam[ i ];
	}
	ZhkSerial1Init();
    memset( ITLMsgBuf, 0, sizeof(ITLMsgBuf) );   
}

unsigned char ITLBusUnpack( void )
{
	unsigned char xdata  i=0, j=0, k=0, m=0;
	unsigned char xdata  len = 0;
	unsigned char xdata  seq = 0;
	unsigned char xdata  position = 0;
	unsigned char xdata newMsg[255];
	
	//return 1;
    //-------------------------------------------------
	/*
    #ifdef ITL_DEBUG
        return 1;
    #endif
	*/
    //=================================================

	while( !ZhkSerial1IsRxBufNull() )
	{
		for( i=0; i<sizeof( ITLMsgBuf )-1; i++ )
		{
			ITLMsgBuf[i] = ITLMsgBuf[i+1];
		}
		ITLMsgBuf[sizeof(ITLMsgBuf)-1] = ZhkSerial1GetCh();
	}

    for( i=0; i<=sizeof(ITLMsgBuf)-6; i++ )
	{
	    //Check the SF
		if( ITLMsgBuf[i] != ITL_MSG_STX ) 
			continue;
		//Check the len
		seq = ITLMsgBuf[i+1];
		if( !((seq==ITL_MSG_SEQ_0)||(seq==ITL_MSG_SEQ_1)) ) 
		    continue;
		len = ITLMsgBuf[i+2];
		if( i+len+5 > sizeof( ITLMsgBuf ) ) 
		    break;

        //-------------------------------------------------
		/*
		#ifdef ITL_DEBUG
            return 1;
		#endif
		*/
        //=================================================

	    //Delete the extral "0x7F"
		memset( newMsg, 0, sizeof(newMsg) );
        position = i;
		k = 0;
		newMsg[k++] = ITLMsgBuf[i];
		newMsg[k++] = ITLMsgBuf[i+1];
        for( j=i+2; j<sizeof(ITLMsgBuf); j++)
		{
        	newMsg[k++] = ITLMsgBuf[j];
			if( ITLMsgBuf[j+1] == ITL_MSG_STX )
			{
				j += 1;
			}
		}
		//Check the CHK
		ITLResetCrc();
      //ITLUpdateCrc( newMsg[0] );
        ITLUpdateCrc( newMsg[1] );   //
	    ITLUpdateCrc( newMsg[2] );   //	
	    for( j=3; j<len+3; j++ )
	    {
    		ITLUpdateCrc( newMsg[j] );
	    }
		if( (sysITLMission.crcL != newMsg[j]) || (sysITLMission.crcH != newMsg[j+1]) )
			continue;
		//Check the message type
        if( !( (newMsg[2]>=1)&&((newMsg[3]>=ITL_ACK_MIN)&&(newMsg[3]<=ITL_ACK_MAX)) ) )   
            continue;     
		//Save the message
		sysITLMission.receive.stx = newMsg[0];
		sysITLMission.receive.seq = newMsg[1];
		sysITLMission.receive.len = newMsg[2];
		for( k=0,j=3; j<sysITLMission.receive.len+3; k++,j++ )
		{
			sysITLMission.receive.msg[k] = newMsg[j];	
		}
        sysITLMission.receive.crcL = newMsg[j];
		sysITLMission.receive.crcH = newMsg[j+1];
		memset( ITLMsgBuf, 0, sizeof(ITLMsgBuf) );
		return 1;
	}	
	return 0;
			
}

//Create the CHK and send the message
unsigned char ITLBusTxMsg( void )
{
	unsigned char i = 0;
	//1.Create the CHK
    ITLResetCrc();
    ITLUpdateCrc( sysITLMission.send.seq );   //
	ITLUpdateCrc( sysITLMission.send.len );	
	for( i=0; i<sysITLMission.send.len; i++ )
	{
    	ITLUpdateCrc( sysITLMission.send.msg[i] );
	}
    //--------------------------------------------
    #ifdef ITL_DEBUG
    	if( (sysITLMission.send.msg[0]==ITL_CMD_SETINHIBITS)&&(sysITLMission.send.seq==0x80) )
        {
        	sysITLMission.crcL = 0x25;
            sysITLMission.crcH = 0xa4;	
        }
        else if( (sysITLMission.send.msg[0]==ITL_CMD_ENABLE)&&(sysITLMission.send.seq==0x00) )
        {
            sysITLMission.crcL = 0x3C;
            sysITLMission.crcH = 0x08;
        }
        else if( (sysITLMission.send.msg[0]==ITL_CMD_POLL)&&(sysITLMission.send.seq==0x00) )
        {
            sysITLMission.crcL = 0x11;
            sysITLMission.crcH = 0x88;
        }
        else if( (sysITLMission.send.msg[0]==ITL_CMD_POLL)&&(sysITLMission.send.seq==0x80) )
        {
            sysITLMission.crcL = 0x12;
            sysITLMission.crcH = 0x02;
        }
    #endif
    //============================================
	sysITLMission.send.crcL = sysITLMission.crcL;
	sysITLMission.send.crcH = sysITLMission.crcH;
	//2.Send the message
    ZhkSerial1PutCh( sysITLMission.send.stx );
	delay( ITL_COM_DELAY );
	ZhkSerial1PutCh( sysITLMission.send.seq );
    delay( ITL_COM_DELAY );
    ZhkSerial1PutCh( sysITLMission.send.len );
	delay( ITL_COM_DELAY );
	if( sysITLMission.send.len == ITL_MSG_STX )
	{
		ZhkSerial1PutCh( sysITLMission.send.len );
		delay( ITL_COM_DELAY );
	}

	for( i=0; i<sysITLMission.send.len; i++ )
	{
		ZhkSerial1PutCh( sysITLMission.send.msg[i] );
		delay( ITL_COM_DELAY );
		if( sysITLMission.send.msg[i] == ITL_MSG_STX )
		{
			ZhkSerial1PutCh( sysITLMission.send.msg[i] );
			delay( ITL_COM_DELAY );
		}
	}

	ZhkSerial1PutCh( sysITLMission.send.crcL );
	delay( ITL_COM_DELAY );
	if( sysITLMission.send.crcL == ITL_MSG_STX )
	{
	    ZhkSerial1PutCh( sysITLMission.send.crcL );
		delay( ITL_COM_DELAY );
	}
    
	ZhkSerial1PutCh( sysITLMission.send.crcH );
	delay( ITL_COM_DELAY );
	if( sysITLMission.send.crcH == ITL_MSG_STX )
	{
	    ZhkSerial1PutCh( sysITLMission.send.crcH );
		delay( ITL_COM_DELAY );
	}
    
    return 1;
}

//msgType, seq 
unsigned char ITLMsgPackSend( unsigned char msgType, unsigned char flag )
{

    unsigned char i = 0;

    if( !((msgType>=ITL_CMD_MIN)&&(msgType<=ITL_CMD_MAX)) )
		return ITL_ERR_PAR;

    if( !((flag>=0)&&(flag<=1)) )
	    return ITL_ERR_PAR;

	sysITLMission.send.stx = ITL_MSG_STX;
	if( flag == 1 )
	{
		if( sysITLMission.send.seq == ITL_MSG_SEQ_0 )
		    sysITLMission.send.seq = ITL_MSG_SEQ_1;
		else 
		    sysITLMission.send.seq = ITL_MSG_SEQ_0;
	}

    switch( msgType )
	{
	    case ITL_CMD_RESET:
			{
				sysITLMission.send.len = 1;
				sysITLMission.send.msg[0] = ITL_CMD_RESET;
			}
			break;
		case ITL_CMD_SETINHIBITS:
		    {
			    sysITLMission.send.len = 3;
				sysITLMission.send.msg[0] = ITL_CMD_SETINHIBITS;
				sysITLMission.send.msg[1] = sysITLMission.chnCtr1;
				sysITLMission.send.msg[2] = sysITLMission.chnCtr2;
			}
		    break;
        case ITL_CMD_DSPON:
		    {
			    sysITLMission.send.len = 1;
				sysITLMission.send.msg[0] = ITL_CMD_DSPON;
			}
		    break;
		case ITL_CMD_DSPOFF:
		    {
			    sysITLMission.send.len = 1;
				sysITLMission.send.msg[0] = ITL_CMD_DSPOFF;
			}
		    break;
		case ITL_CMD_SETUP:
		    {
			     sysITLMission.send.len = 1;
				 sysITLMission.send.msg[0] = ITL_CMD_SETUP;
			}
		    break;
		case ITL_CMD_POLL:
		    {
			     sysITLMission.send.len = 1;
				 sysITLMission.send.msg[0] = ITL_CMD_POLL;
			}
		    break;
		case ITL_CMD_REJECT:
		    {
			     sysITLMission.send.len = 1;
				 sysITLMission.send.msg[0] = ITL_CMD_REJECT;
			}
		    break;
		case ITL_CMD_DISABLE:
		    {
			     sysITLMission.send.len = 1;
				 sysITLMission.send.msg[0] = ITL_CMD_DISABLE;
			}
		    break;
        case ITL_CMD_ENABLE:
		    {
				 sysITLMission.send.len = 1;
				 sysITLMission.send.msg[0] = ITL_CMD_ENABLE;
			}
		    break;
		case ITL_CMD_SYN:
		    {
			     sysITLMission.send.len = 1;
				 sysITLMission.send.msg[0] = ITL_CMD_SYN;
			}
		    break;
        case ITL_CMD_HOLD:
		    {
			     sysITLMission.send.len = 1;
				 sysITLMission.send.msg[0] = ITL_CMD_HOLD;
			}
		    break;
		default: break;
	}
    ITLBusTxMsg();
    DelayMs(10);
    return ITL_ERR_NULL;
}

unsigned char ITLMission_Syn( void )
{    
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;
    
	//1.
    ITLSerialInit();
	//2.
    retry = ITL_COM_RETRY;
TAB_ITLM_Syn_RETRY:
	flag = ITLMsgPackSend( ITL_CMD_SYN, 1);
    if( flag != ITL_ERR_NULL )
    {
		return ITL_ERR_PAR;
	}
	sysITLMission.msTimer2 = ITL_TIME_OUT;
	while( sysITLMission.msTimer2 )
	{
		if( ITLBusUnpack() )
		{
		    sysITLMission.comErrNum = 0;
			goto TAB_ITLM_Syn_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_ITLM_Syn_RETRY;
	}
	else
	{
		sysITLMission.comErrNum++;
        return ITL_ERR_COM;
	}
TAB_ITLM_Syn_COM_OK:
    sysITLMission.billESta = sysITLMission.receive.msg[0];
    return ITL_ERR_NULL;

}


unsigned char ITLMission_Setup( void )
{    
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;
    
	//1.
    ITLSerialInit();
	//2.
    retry = ITL_COM_RETRY;
TAB_ITLM_Setup_RETRY:
	flag = ITLMsgPackSend( ITL_CMD_SETUP, 1);
    if( flag != ITL_ERR_NULL )
    {
		return ITL_ERR_PAR;
	}
	sysITLMission.msTimer2 = ITL_TIME_OUT;
	while( sysITLMission.msTimer2 )
	{
		if( ITLBusUnpack() )
		{
		    sysITLMission.comErrNum = 0;
			goto TAB_ITLM_Setup_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_ITLM_Setup_RETRY;
	}
	else
	{
		sysITLMission.comErrNum++;
        return ITL_ERR_COM;
	}
TAB_ITLM_Setup_COM_OK:
    sysITLMission.billESta = sysITLMission.receive.msg[0];
    return ITL_ERR_NULL;

}

unsigned char ITLMission_SetInhibit( void )
{    
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;
  
	//1.
    ITLSerialInit();
	//2.
    ITLChnCtr();
    retry = ITL_COM_RETRY;
TAB_ITLM_SetInhibit_RETRY:
	flag = ITLMsgPackSend( ITL_CMD_SETINHIBITS, 1);
    if( flag != ITL_ERR_NULL )
    {
		return ITL_ERR_PAR;
	}
	sysITLMission.msTimer2 = ITL_TIME_OUT;
	while( sysITLMission.msTimer2 )
	{
		if( ITLBusUnpack() )
		{
		    sysITLMission.comErrNum = 0;
			goto TAB_ITLM_SetInhibit_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_ITLM_SetInhibit_RETRY;
	}
	else
	{
		sysITLMission.comErrNum++;
        return ITL_ERR_COM;
	}
TAB_ITLM_SetInhibit_COM_OK:
    sysITLMission.billESta = sysITLMission.receive.msg[0];
    return ITL_ERR_NULL;

}

unsigned char ITLMission_Reset( void )
{    
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

	//1.
    ITLSerialInit();
	//2.
    retry = ITL_COM_RETRY;
TAB_ITLM_Reset_RETRY:
	flag = ITLMsgPackSend( ITL_CMD_RESET, 1);
    if( flag != ITL_ERR_NULL )
    {
		return ITL_ERR_PAR;
	}
	sysITLMission.msTimer2 = ITL_TIME_OUT;
	while( sysITLMission.msTimer2 )
	{
		if( ITLBusUnpack() )
		{
		    sysITLMission.comErrNum = 0;
			goto TAB_ITLM_Reset_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_ITLM_Reset_RETRY;
	}
	else
	{
		sysITLMission.comErrNum++;
        return ITL_ERR_COM;
	}
TAB_ITLM_Reset_COM_OK:
    sysITLMission.billESta = sysITLMission.receive.msg[0];
    return ITL_ERR_NULL;

}

unsigned char ITLMission_Dspon( void )
{    
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

	//1.
    ITLSerialInit();
	//2.
    retry = ITL_COM_RETRY;
TAB_ITLM_Dspon_RETRY:
	flag = ITLMsgPackSend( ITL_CMD_DSPON, 1);
    if( flag != ITL_ERR_NULL )
    {
		return ITL_ERR_PAR;
	}
	sysITLMission.msTimer2 = ITL_TIME_OUT;
	while( sysITLMission.msTimer2 )
	{
		if( ITLBusUnpack() )
		{
		    sysITLMission.comErrNum = 0;
			goto TAB_ITLM_Dspon_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_ITLM_Dspon_RETRY;
	}
	else
	{
		sysITLMission.comErrNum++;
        return ITL_ERR_COM;
	}
TAB_ITLM_Dspon_COM_OK:
    sysITLMission.billESta = sysITLMission.receive.msg[0];
    return ITL_ERR_NULL;

}

unsigned char ITLMission_Dspoff( void )
{    
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

	//1.
    ITLSerialInit();
	//2.
    retry = ITL_COM_RETRY;
TAB_ITLM_Dspoff_RETRY:
	flag = ITLMsgPackSend( ITL_CMD_DSPOFF, 1);
    if( flag != ITL_ERR_NULL )
    {
		return ITL_ERR_PAR;
	}
	sysITLMission.msTimer2 = ITL_TIME_OUT;
	while( sysITLMission.msTimer2 )
	{
		if( ITLBusUnpack() )
		{
		    sysITLMission.comErrNum = 0;
			goto TAB_ITLM_Dspoff_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_ITLM_Dspoff_RETRY;
	}
	else
	{
		sysITLMission.comErrNum++;
        return ITL_ERR_COM;
	}
TAB_ITLM_Dspoff_COM_OK:
    sysITLMission.billESta = sysITLMission.receive.msg[0];
    return ITL_ERR_NULL;

}

unsigned char ITLMission_Hold( void )
{    
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

	//1.
    ITLSerialInit();
	//2.
    retry = ITL_COM_RETRY;
TAB_ITLM_Hold_RETRY:
	flag = ITLMsgPackSend( ITL_CMD_HOLD, 1);
    if( flag != ITL_ERR_NULL )
    {
		return ITL_ERR_PAR;
	}
	sysITLMission.msTimer2 = ITL_TIME_OUT;
	while( sysITLMission.msTimer2 )
	{
		if( ITLBusUnpack() )
		{
		    sysITLMission.comErrNum = 0;
			goto TAB_ITLM_Hold_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_ITLM_Hold_RETRY;
	}
	else
	{
		sysITLMission.comErrNum++;
        return ITL_ERR_COM;
	}
TAB_ITLM_Hold_COM_OK:
    sysITLMission.billESta = sysITLMission.receive.msg[0];
    return ITL_ERR_NULL;

}


unsigned char ITLMission_Enable( void )
{    
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;
  
	//1.
    ITLSerialInit();
	//2.
    retry = ITL_COM_RETRY;
TAB_ITLM_Enable_RETRY:
	flag = ITLMsgPackSend( ITL_CMD_ENABLE, 1);
    if( flag != ITL_ERR_NULL )
    {
		return ITL_ERR_PAR;
	}
	sysITLMission.msTimer2 = ITL_TIME_OUT;
	while( sysITLMission.msTimer2 )
	{
		if( ITLBusUnpack() )
		{
		    sysITLMission.comErrNum = 0;
			goto TAB_ITLM_Enable_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_ITLM_Enable_RETRY;
	}
	else
	{
		sysITLMission.comErrNum++;
        return ITL_ERR_COM;
	}
TAB_ITLM_Enable_COM_OK:
    sysITLMission.enableFlag = 1;
	sysITLMission.billESta = sysITLMission.receive.msg[0];
    return ITL_ERR_NULL;

}

unsigned char ITLMission_Disable( void )
{    
    /*
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

    
	//1.
    ITLSerialInit();
	//2.
    retry = ITL_COM_RETRY;
TAB_ITLM_Disable_RETRY:
	flag = ITLMsgPackSend( ITL_CMD_DISABLE, 1);
    if( flag != ITL_ERR_NULL )
    {
		return ITL_ERR_PAR;
	}
	sysITLMission.msTimer2 = ITL_TIME_OUT;
	while( sysITLMission.msTimer2 )
	{
		if( ITLBusUnpack() )
		{
		    sysITLMission.comErrNum = 0;
			goto TAB_ITLM_Disable_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_ITLM_Disable_RETRY;
	}
	else
	{
		sysITLMission.comErrNum++;
        return ITL_ERR_COM;
	}
TAB_ITLM_Disable_COM_OK:
    sysITLMission.enableFlag = 0;
	sysITLMission.billESta = sysITLMission.receive.msg[0];
    return ITL_ERR_NULL;
   */
   return ITL_ERR_NULL;
}

unsigned char ITLMission_Reject( void )
{    
    /*
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

	//1.
    ITLSerialInit();
	//2.
    retry = ITL_COM_RETRY;
TAB_ITLM_Reject_RETRY:
	flag = ITLMsgPackSend( ITL_CMD_REJECT, 1);
    if( flag != ITL_ERR_NULL )
    {
		return ITL_ERR_PAR;
	}
	sysITLMission.msTimer2 = ITL_TIME_OUT;
	while( sysITLMission.msTimer2 )
	{
		if( ITLBusUnpack() )
		{
		    sysITLMission.comErrNum = 0;
			goto TAB_ITLM_Reject_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_ITLM_Reject_RETRY;
	}
	else
	{
		sysITLMission.comErrNum++;
        return ITL_ERR_COM;
	}
TAB_ITLM_Reject_COM_OK:
    sysITLMission.billESta = sysITLMission.receive.msg[0];
    return ITL_ERR_NULL;
	*/
	return ITL_ERR_NULL;

}


unsigned char ITLMission_Poll( void )
{   
    /* 
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;
    
	//1.
    ITLSerialInit();
	//2.
    retry = ITL_COM_RETRY;
TAB_ITLM_P_RETRY:
	flag = ITLMsgPackSend( ITL_CMD_POLL, 1);
    if( flag != ITL_ERR_NULL )
    {
		return ITL_ERR_PAR;
	}
	sysITLMission.msTimer2 = ITL_TIME_OUT;
	while( sysITLMission.msTimer2 )   //sysVPMission.msTimer2
	{
		if( ITLBusUnpack() )
		{
		    sysITLMission.comErrNum = 0;
			goto TAB_ITLM_P_COM_OK;
		}
        //------------------------------------------------------------------
        if( IfGameKeyOn() )
		{
			VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
	        //Beep();
		}
        //===================================================================
	}

	if( retry-- )
	{
		goto TAB_ITLM_P_RETRY;
	}
	else
	{
		sysITLMission.comErrNum++;
		if( sysITLMission.comErrNum >= ITL_TIME_OUT_NUM )
		{
			DeviceStatus.BillAccepter |= 0x01;
		}
        return ITL_ERR_COM;
	}
TAB_ITLM_P_COM_OK:
    //Check the the bill status
	sysITLMission.billSta  = ITL_BILL_NULL;
    sysITLMission.billESta = ITL_BILL_NULL;
	sysITLMission.status   = ITL_DEV_NULL;
    for( i=0; i<sysITLMission.receive.len; i++ )
	{
		if( sysITLMission.receive.msg[i] == ITL_ACK_READ )
		{
			if( (sysITLMission.receive.msg[i+1]>=1)&&(sysITLMission.receive.msg[i+1]<=8) )
			{
				sysITLMission.billSta  |= ITL_BILL_READ;
				sysITLMission.billChn   = sysITLMission.receive.msg[i+1];
				sysITLMission.billValue = SystemParameter.billValue[sysITLMission.billChn-1];
			}
		}
		else if( sysITLMission.receive.msg[i] == ITL_ACK_STACKING )
		{
			sysITLMission.billSta |= ITL_BILL_STACKING;
		}
		else if( sysITLMission.receive.msg[i] == ITL_ACK_CREDIT )
		{
			if( (sysITLMission.receive.msg[i+1]>=1)&&(sysITLMission.receive.msg[i+1]<=8) )
			{
				sysITLMission.billSta  |= ITL_BILL_CREDIT;
				sysITLMission.billChn   = sysITLMission.receive.msg[i+1];
				sysITLMission.billValue = SystemParameter.billValue[sysITLMission.billChn-1];
			}
		}
		else if( sysITLMission.receive.msg[i] == ITL_ACK_STACKED )
		{
			sysITLMission.billSta  |= ITL_BILL_STACKED;
		}
        else if( sysITLMission.receive.msg[i] == ITL_ACK_REJECTING )
		{
			sysITLMission.billSta  |= ITL_BILL_REJECTING;
		}
		else if( sysITLMission.receive.msg[i] == ITL_ACK_REJECTED )
		{
			sysITLMission.billSta  |= ITL_BILL_REJECTED;
		}
		else if( sysITLMission.receive.msg[i] == ITL_ACK_SJAM )
		{
			sysITLMission.billSta  |= ITL_BILL_SJAM;   //ERR
		}
        else if( sysITLMission.receive.msg[i] == ITL_ACK_UJAM )
		{
			sysITLMission.billSta  |= ITL_BILL_UJAM;  //ERR
		}
		//
		else if( sysITLMission.receive.msg[i] == ITL_ACK_RESET )
		{
		    sysITLMission.status |= ITL_DEV_RESET;
		}
		else if( sysITLMission.receive.msg[i] == ITL_ACK_DISABLED )
		{
		    sysITLMission.status |= ITL_DEV_DISABLED;
		}
        else if( sysITLMission.receive.msg[i] == ITL_ACK_NCFFAR )
		{
		    sysITLMission.status |= ITL_DEV_NCFFAR;
		}
        else if( sysITLMission.receive.msg[i] == ITL_ACK_NCICBAR )
		{
		    sysITLMission.status |= ITL_DEV_NCICBAR;
		}
        else if( sysITLMission.receive.msg[i] == ITL_ACK_CBREM )
		{
		    sysITLMission.status |= ITL_DEV_CBREM;
		}
        else if( sysITLMission.receive.msg[i] == ITL_ACK_CBREP )
		{
		    sysITLMission.status |= ITL_DEV_CBREP;
		}
		else if( sysITLMission.receive.msg[i] == ITL_ACK_STAFULL )
		{
		    sysITLMission.status |= ITL_DEV_STAFULL;
		}
		else if( sysITLMission.receive.msg[i] == ITL_ACK_FATTN )
		{
		    sysITLMission.status |= ITL_DEV_FATTN;
		}
        else if( sysITLMission.receive.msg[i] == ITL_ACK_FAIL )
		{
		    sysITLMission.status |= ITL_DEV_FAIL;
		}
        //Andy added 2011.4.15.
        else if( sysITLMission.receive.msg[i] == ITL_ACK_CMDAPL )
        {
        	sysITLMission.billESta = ITL_ACK_CMDAPL;
        }
        else if( sysITLMission.receive.msg[i] == ITL_ACK_OK )
        {
        	sysITLMission.billESta = ITL_ACK_OK;
        }

	} 
    return ITL_ERR_NULL;
	*/
	return ITL_ERR_NULL;
}

unsigned char ITLMission_Init_1( void )
{
    /*
    //ITLMission_Reset();
    //DelayMs( ITL_TIME_CMD_INTEVEL );          
    //ITLMission_Syn();
    //DelayMs( ITL_TIME_CMD_INTEVEL );
    ITLMission_Disable();
    DelayMs( ITL_TIME_CMD_INTEVEL_1 );
    ITLMission_SetInhibit();
    DelayMs( ITL_TIME_CMD_INTEVEL_1 );
    ITLMission_Dspon();
    DelayMs( ITL_TIME_CMD_INTEVEL_1 );
	ITLMission_Enable();
    DelayMs( ITL_TIME_CMD_INTEVEL_1 );
    return ITL_ERR_NULL;
	*/
	//MDBMission_Bill_Init();
    return ITL_ERR_NULL;
}

unsigned char ITLMission_Init( void )
{
    /* 
    ITLMission_Reset();
    DelayMs( ITL_TIME_CMD_INTEVEL );          //30000,10000,4000,3000
    //ITLMission_Syn();
    //DelayMs( ITL_TIME_CMD_INTEVEL );
    ITLMission_SetInhibit();
    DelayMs( ITL_TIME_CMD_INTEVEL );
    ITLMission_Dspon();
    DelayMs( ITL_TIME_CMD_INTEVEL );
	ITLMission_Enable();
    DelayMs( ITL_TIME_CMD_INTEVEL );
    return ITL_ERR_NULL;
	*/
	//MDBMission_Bill_Init();
    return ITL_ERR_NULL;
}

unsigned char ITLMission_HB_Stack( void )
{
    unsigned char flag = 0;
	if( sysITLMission.billHoldingFlag == 1 )
	{
	    flag = ITLMission_Poll(); 
        sysITLMission.msPoll = ITL_TIME_POLL;
		sysITLMission.sTimer2 = ITL_TIME_STACK;
		while( sysITLMission.sTimer2 )
		{

		    if( sysITLMission.billSta & ITL_BILL_STACKED )
			{
				sysITLMission.billStaCtr |= ITL_BILL_STACKED;
                sysITLMission.billSta &= ~ITL_BILL_STACKED;
			}

            if( sysITLMission.billSta & ITL_BILL_CREDIT )
			{
				sysITLMission.billStaCtr |= ITL_BILL_CREDIT;
				sysITLMission.billSta &= ~ITL_BILL_CREDIT;
			}

		    //if( ( sysITLMission.billSta & ITL_BILL_STACKED ) )
 		    //if( ( sysITLMission.billSta & ITL_BILL_CREDIT )&&( sysITLMission.billStaCtr&ITL_BILL_READ ) )
			//if( ( sysITLMission.billSta & ITL_BILL_CREDIT )&&( sysITLMission.billSta&ITL_BILL_STACKED )&&( sysITLMission.billStaCtr&ITL_BILL_READ ) )
			if( ( sysITLMission.billStaCtr&ITL_BILL_CREDIT )&&( sysITLMission.billStaCtr&ITL_BILL_STACKED )&&( sysITLMission.billStaCtr&ITL_BILL_READ ) )
         	{    
			    sysITLMission.billStaCtr &= ~ITL_BILL_READ;
			    sysITLMission.billStaCtr &= ~ITL_BILL_CREDIT;
                sysITLMission.billStaCtr &= ~ITL_BILL_STACKED;
				break;
			}
            else if( sysITLMission.msPoll == 0 )
			{
				flag = ITLMission_Poll(); 
        		sysITLMission.msPoll = ITL_TIME_POLL;
			}
		}
        //Stack fail
		if( sysITLMission.sTimer2 == 0 )
		{
			return 1;
		}
	}
	return ITL_ERR_NULL;
}



unsigned char ITLMission_HB_Reject( void )
{
    unsigned char flag = 0;
    if( sysITLMission.billHoldingFlag == 1 )
	{
    	if( SystemParameter.RefundPermission==1 )
		{
			flag = ITLMission_Reject();
			sysITLMission.msPoll = ITL_TIME_POLL;
			sysITLMission.sTimer2 = ITL_TIME_REJECT;
			while( sysITLMission.sTimer2 )
			{
				if( ( sysITLMission.billSta & ITL_BILL_REJECTED ))
				{
				    sysITLMission.billSta &= ~ITL_BILL_REJECTED;
					//...
					break;
				}
	            else if( sysITLMission.msPoll == 0 )
				{
					flag = ITLMission_Reject(); 
	        		sysITLMission.msPoll = ITL_TIME_POLL;
				}

			}
			//
			if( sysITLMission.sTimer2 == 0 )
			{
				return 1;
			}
		}
	}
	return ITL_ERR_NULL;
}





/*****************************************************************************
*****************************************************************************
***********************************MDB纸币器*********************************
*****************************************************************************
*****************************************************************************/



void MDBSerialInit( void )
{	
	unsigned char data i=0;

	for( i=0; i<sizeof( SERAIL1PARAM ); i ++ )
	{
		ZhkSerial1Param[ i ] = ITLSerialParam[ i ];
	} 
	DelayMs(10);
	ZhkSerial1_MDB_Init();
	DelayMs(10);
    
    DelayMs(2);
	ZhkHardSerialSwitch( 0 );
	DelayMs(2);

    //memset( ITLMsgBuf, 0, sizeof(ITLMsgBuf) );   
}


/*****************************************************************************
*UART块传送
*入口参数:add-VMC发送的地址指令字节,dat[]-VMC发送的数据块,count-数据块的大小
*出口参数:0-外设应答ACK,非0且小于0x80-外设应答的数据块的大小,大于等于0x80-会话中出错
*****************************************************************************/
uchar Session(uchar add,uchar const dat[ ],uchar count)
{
	uchar check,i,j,err;
	uchar mode;
	
	sysMDBMission.port0_counter=0;
	for(j=0;j<5;j++)                      //最多重复发送命令5次
	{
	  check=0;
	  err=0;
	  ZhkSerial1_MDB_PutCh(add,1);                //发送地址字节
	  check +=add;                        //计算CHK
	  for(i=0;i<count;i++)                //发送数据字节块
	  {
	    ZhkSerial1_MDB_PutCh(dat[i],0);
	    check +=dat[i];
	  }
	  ZhkSerial1_MDB_PutCh(check,0);              //发送CHK
	
	  for(i=0,check=0,mode=0;!mode && i<MDB_MAX_BLOCK_SIZF && !err;i++)  //反复接收   字节直到方式位为1或出错
	  {
	    //while( !ZhkSerial1IsRxBufNull() )
		{
		    if(!ZhkSerial1_MDB_GetCh(&sysMDBMission.buff[i],&mode))  //超时,外设可用超时表示NAK
		      //err=i?MDB_ERR_TIME_OUT:MDB_NAK;
              err=MDB_ERR_TIME_OUT;  //2011.7.9 changed for 0x80 error code
		    else if (i==0 && sysMDBMission.buff[i]==MDB_NAK && mode)  //收到NAK
		      err = MDB_NAK;
		    else if (!mode)          //方式位为0表示还有数据
		      check +=sysMDBMission.buff[i];
		}
	  } 
	  if(!err)                            //未发生错误
	  {
	    if(!mode)                         //收完36个字节还未结束
	    {
	      ZhkSerial1_MDB_PutCh(MDB_NAK,0);
	      err=MDB_ERR_NO_MODE_BIT;
	    }
	    else if (i>1)                   //收到数据块
	    {
	      if(check !=sysMDBMission.buff[i-1])         //校验和错
	      {
	          ZhkSerial1_MDB_PutCh(MDB_NAK,0);
	          err=MDB_ERR_CHECKSUM;
	      }
	      else                        //一切正常，发送ACK后跳出循环
	      {
	          ZhkSerial1_MDB_PutCh(MDB_ACK,0);
	          break;
	      }
	    }
	    else                          //收到外设传来的ACK
	          break;
	  }
	//  Wait(T_RESPONSE);                   //防止与外设数据冲突
	}
	sysMDBMission.port0_counter=err?err:(i-1);
	return err?err:(i-1);
}






//以下是硬纸币机的函数

//-------------------------------------
//-------------------------------------
//纸币器启动
unsigned char MDBMission_Bill_Init( void )
{
	u_char flag;
	u_int  BillDecimal = 0;
	u_char BillScale = 0,i = 0;

	DelayMs( 100 );
    MDBSerialInit();	
	DelayMs( 100 );

	flag=MDB_Bill_Reset();
	if(flag!=0)
    {
		//sysMDBMission.billDeviceStatus|=0x01;
	}
	else
	{
        sysMDBMission.billDeviceStatus&=0xfe;
	}

	DelayMs( 500 );
	MDB_Bill_Poll( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
	DelayMs( 100 );
	flag=MDB_Bill_Setup( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
	if(flag!=0)
		sysMDBMission.billDeviceStatus|=0x01;
	else
	{
		//计算各通道面值
		BillDecimal = 100;
	    for(i = 0; i < sysMDBMission.billBuf[5]; i++) 
	    {
		   BillDecimal /= 10;
	    }
		BillScale = ((((u_int)sysMDBMission.billBuf[3]) << 8) | sysMDBMission.billBuf[4]) * BillDecimal;
		for(i=0;i<8;i++)
		{
			if(sysMDBMission.billBuf[11+i] == 0xFF) 
			{
			     break;
			}
			
			SystemParameter.billValue[i] = (u_int)sysMDBMission.billBuf[i+11] * BillScale;	
			if( (SystemParameter.billValue[i]>5000)&&(SystemParameter.billValue[i]!=10000) )
				SystemParameter.billValue[i]=0;
		}	
		
        sysMDBMission.billDeviceStatus&=0xfe;
	}

	DelayMs( 100 );
	flag=MDB_Bill_EXP_Identification( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
	if(flag!=0)
		sysMDBMission.billDeviceStatus|=0x01;
	else
        sysMDBMission.billDeviceStatus&=0xfe;

	DelayMs( 100 );
	flag=MDB_Bill_Stacker( &sysMDBMission.billBuf[0], &sysMDBMission.billBuf[1] );
	DelayMs( 100 );
	if(flag!=0)
		sysMDBMission.billDeviceStatus|=0x01;
	else
        sysMDBMission.billDeviceStatus&=0xfe;

	flag=MDBMission_Bill_Enable();
	if(flag!=0)
		sysMDBMission.billDeviceStatus|=0x01;
	else
        sysMDBMission.billDeviceStatus&=0xfe;

	DelayMs( 100 );
	flag = MDB_Bill_Poll( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
	if(flag==0)
		sysMDBMission.billDeviceStatus&=0xfe;
	else
	    sysMDBMission.billDeviceStatus|=0x01;
	DelayMs( 100 );
	return ITL_ERR_NULL;
}

/*
//测试使用的纸币器使能
unsigned char MDBMission_Bill_InitForTest( void )
{
	u_char flag;
	uchar typesec=0;

	DelayMs( 100 );
    MDBSerialInit();	
	DelayMs( 100 );

	flag=MDB_Bill_Reset();
	if(flag!=0)
    {
		//sysMDBMission.billDeviceStatus|=0x01;
	}
	else
	{
        sysMDBMission.billDeviceStatus&=0xfe;
	}

	DelayMs( 500 );
	MDB_Bill_Poll( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
	DelayMs( 100 );
	flag=MDB_Bill_Setup( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
	if(flag!=0)
		sysMDBMission.billDeviceStatus|=0x01;
	else
        sysMDBMission.billDeviceStatus&=0xfe;

	DelayMs( 100 );
	flag=MDB_Bill_Stacker( &sysMDBMission.billBuf[0], &sysMDBMission.billBuf[1] );
	DelayMs( 100 );
	if(flag!=0)
		sysMDBMission.billDeviceStatus|=0x01;
	else
        sysMDBMission.billDeviceStatus&=0xfe;

	//3
    if(SystemParameter.billValue[0]>0)
		typesec |= 0x01;
	if(SystemParameter.billValue[1]>0)
		typesec |= 0x02;
	if(SystemParameter.billValue[2]>0)
		typesec |= 0x04;
	if(SystemParameter.billValue[3]>0)
		typesec |= 0x08;
	if(SystemParameter.billValue[4]>0)
		typesec |= 0x10;
	if(SystemParameter.billValue[5]>0)
		typesec |= 0x20;
	if(SystemParameter.billValue[6]>0)
		typesec |= 0x40;
	if(SystemParameter.billValue[7]>0)
		typesec |= 0x80;
	
	flag=MDB_Bill_Type(  0x00, typesec, 0xff, 0xff );
	if(flag!=0)
		sysMDBMission.billDeviceStatus|=0x01;
	else
        sysMDBMission.billDeviceStatus&=0xfe;

	DelayMs( 100 );
	flag = MDB_Bill_Poll( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
	if(flag==0)
		sysMDBMission.billDeviceStatus&=0xfe;
	else
	    sysMDBMission.billDeviceStatus|=0x01;
	DelayMs( 100 );
	return ITL_ERR_NULL;
}
*/

//纸币机使能
unsigned char MDBMission_Bill_Enable( void )
{
    uchar  mission;
	uchar typesec=0;
    //1.
    MDBSerialInit();  
    //添加是否开启硬币器的控制;by gzz 20111019
    if(SystemParameter.BillNo==1)
   	{
	    if(SystemParameter.billValue[0]>0)
			typesec |= 0x01;
		if(SystemParameter.billValue[1]>0)
			typesec |= 0x02;
		if(SystemParameter.billValue[2]>0)
			typesec |= 0x04;
		if(SystemParameter.billValue[3]>0)
			typesec |= 0x08;
		if(SystemParameter.billValue[4]>0)
			typesec |= 0x10;
		if(SystemParameter.billValue[5]>0)
			typesec |= 0x20;
		if(SystemParameter.billValue[6]>0)
			typesec |= 0x40;
		if(SystemParameter.billValue[7]>0)
			typesec |= 0x80;
    }
	
	mission=MDB_Bill_Type(  0x00, typesec, 0xff, 0xff );    
	//3.
    if( mission == 0x00 )
    { 
       sysMDBMission.billIsEnable = 1;
       sysMDBMission.billDeviceStatus &= 0xfe;
	   return 0;
    }
    else
    {	   
       sysMDBMission.billDeviceStatus |= 0x01;
	   return 0x80;
    }		
}



//纸币机禁能
unsigned char MDBMission_Bill_Disable( void )
{
   uchar mission;
   //1.
   MDBSerialInit();
   //2.
   mission=MDB_Bill_Type(  0x00, 0x00, 0xff, 0xff );
   //3.
   if( mission == 0x00 )
   {
       sysMDBMission.billIsEnable = 0;
       sysMDBMission.billDeviceStatus &= 0xfe;
   }
   else
   {
       sysMDBMission.billDeviceStatus |= 0x01;
   }
   return 0;
}

//纸币机复位。
//0x80: comms error
//0x00: ok
uchar MDB_Bill_Reset( void )   //Reset 命令
{
	uchar Reset_r[1]={0};
	return Session(0x30,Reset_r,0);  //复位纸币机,
}
//-------------------------------------
//------------------------------------

uchar MDB_Bill_Setup( uchar *buf, uchar *len )   //Setup 命令
{
	uchar Reset_r[1]={0};
	uchar candu;
	uchar i;
	candu=Session(0x31,Reset_r,0);
	if(candu>=0x80) return candu;
	else
	{
		for(i=0;i<candu;i++)
		{
		    buf[i]=sysMDBMission.buff[i];
			//*buf++ = sysMDBMission.buff[i];
			if(i>10)
				sysMDBMission.Pc_billBuf[i]=(buf[i]>100)?0:buf[i];
			else
				sysMDBMission.Pc_billBuf[i]=buf[i];	
		}
		*len=candu;
		return 0x00;
	}
}
//-------------------------------------
//------------------------------------

//0x80 = com error
//0x00 = ok
uchar MDB_Bill_Poll(uchar *buf, uchar *len)   //轮询
{
	uchar Poll_r[1]={0};
	uchar	candu;
	uchar	i;
	candu=Session(0x33,Poll_r,0);  //轮询纸币机,poll
	if(candu>=0x80) return	0x80;
	else
	{
		for(i=0;i<candu;i++)
			buf[i]=sysMDBMission.buff[i];
		*len=candu;
		return 0x00;
	}
	
}


uchar MDB_Bill_Poll_API( void ) 
{
    uchar mission = 0;
    //1.
    MDBSerialInit();
    //2.
    mission = MDB_Bill_Poll( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
	//3.
    if( mission == 0x00 )
    {
       sysMDBMission.billDeviceStatus &= 0xfe;
    }
    else
    {
       sysMDBMission.billDeviceStatus |= 0x01;
    }
    return 0;    
}

//------------------------------------
//0x80 = com error
//0x00 = not, 0x01=stacked,0x02=其他状态码
uchar MDB_Bill_IfStacked( uchar *buf, uchar *len ) //是否已压钞
{
	uchar IfStacked_r[1]={0};
	uchar	candu;
	uchar	i;
	
    MDBSerialInit();
	candu=Session(0x33,IfStacked_r,0);  //轮询纸币机,poll
	if( candu >= 0x80 ) 
	{
	    return candu;
	}
	else
	{
		*len=candu;
		if(candu==0) return 0x00;
		for(i=0;i<candu;i++)
		{
			buf[i]=sysMDBMission.buff[i];
		}
		//*billType=sysMDBMission.buff[0]&0x0f;
	    if( sysMDBMission.buff[0] <= 0x7f )
		{
			return  0x02;   //other message(maybe error status)
		}
		else if((sysMDBMission.buff[0]&0xf0)==0x80) 
		{
			return	0x01;
		}
		return	0x00;
	}
}
//-------------------------------------
//------------------------------------

//0x80 = com error
//0x00 = not, 0x01=secrow,0x02=其他状态码
uchar MDB_Bill_IfSecrow( uchar *buf, uchar *len, uchar *billType )  //是否暂存纸币
{
	uchar IfSecrow_r[1]={0};
	uchar	candu;
	uchar	i;
	static uchar errFlag=0;
		
    MDBSerialInit();
	candu=Session(0x33,IfSecrow_r,0);  //轮询纸币机,poll
	if( candu >= 0x80 ) 
	{		
		if(errFlag>=5)
		{
	    	sysMDBMission.billDeviceStatus |= 0x01;
		}
		else
		{
			errFlag++;
		}
	    return candu;
	}
	else
	{
		sysMDBMission.billDeviceStatus &= 0xfe;
		errFlag=0;		
		*len=candu;
		if(candu==0)
		{
			sysMDBMission.billCashBuf = 0;//钞箱移除报故障;by gzz 20121224 
			sysMDBMission.billOthErr = 0;
			return 0x00;
		}
		for(i=0;i<candu;i++)
		{
			buf[i]=sysMDBMission.buff[i];
		}
		for(i = 0; i < candu; i++) 
		{			
		    if( sysMDBMission.buff[i] <= 0x7f )
			{
				/*
				//钞箱移除报故障;by gzz 20121224
				if( ( sysMDBMission.buff[0] == 0x08 )||( sysMDBMission.buff[0] == 0x09 ) )
				//if( sysMDBMission.buff[0] == 0x08 )
				{
					sysMDBMission.billCashBuf = 1;
				}
				*/
				switch(sysMDBMission.buff[i]) 
				{
		            case 0x01:			                 //defective motor    
		            	//TraceBill("\r\n Drvbill motor");
				        sysMDBMission.billOthErr |= 0x01;
				        break;
			
			        case 0x02:			                 //sensor problem
			        	//TraceBill("\r\n Drvbill sensor");
					    sysMDBMission.billOthErr |= 0x02;
				        break;
			
			        case 0x03:			                 //validator busy
			        	//TraceBill("\r\n Drvbil busy");
			        	break;
			
			        case 0x04:			                 //rom chksum err
			        	//TraceBill("\r\n Drvbill chksum");
				    	sysMDBMission.billOthErr |= 0x04;
			        break;
			
			        case 0x05:			                 //validator jammed
			        	//TraceBill("\r\n Drvbill jammed");
					    sysMDBMission.billOthErr |= 0x08;			       
				        break;
			
			        case 0x06:			                 //validator was reset
			        	//TraceBill("\r\n Drvbil reset");
						break;
				 
			        case 0x07:			                 //bill removed	
			        	//TraceBill("\r\n Drvbil removed");
			        	break;
			 
			        case 0x08:			                 //cash box out of position
			        	//TraceBill("\r\n Drvbill removeCash");
					    sysMDBMission.billCashBuf = 1;
				        break;
			
			        case 0x09:			                 //validator disabled	
			        	//TraceBill("\r\n Drvbill disabled");
			        	sysMDBMission.billOthErr |= 0x10;
						break;
			
			        case 0x0A:			                 //invalid escrow request
			        	//TraceBill("\r\n Drvbil invalid");
			       		break;
			
			        case 0x0B:			                 //bill rejected
			        	//TraceBill("\r\n Drvbil rejected");
			        	break;
			
			        case 0x0C:			                 //possible credited bill removal
			        	//TraceBill("\r\n Drvbill cashErr");
					    sysMDBMission.billOthErr |= 0x20;	
				        break;
			
			        default:
						//TraceBill("\r\n Drvbill default");	
						sysMDBMission.billOthErr = 0;
						sysMDBMission.billCashBuf = 0;
					    break;
		         }
				//return  0x02;   //other message(maybe error status)
			}
			else if((sysMDBMission.buff[i]&0xf0)==0x90) 
			{
				*billType=sysMDBMission.buff[i]&0x0f;
				return	0x01;
			}
		}
		return	0x00;
	}	
}

//-------------------------------------
//------------------------------------
//0x80 = com error
//0x00 = not, 0x01=returned,0x02=其他状态码
uchar MDB_Bill_IfReturned( uchar *buf, uchar *len, uchar *billType )  //是否已退币
{
	uchar IfReturned_r[1]={0};
	uchar	candu;
	uchar	i;
		
    MDBSerialInit();
	candu=Session(0x33,IfReturned_r,0);  //轮询纸币机,poll
	if( candu >= 0x80 ) 
	{
	    return candu;
	}
	else
	{
		*len=candu;
		if(candu==0) return 0x00;
		for(i=0;i<candu;i++)
		{
			buf[i]=sysMDBMission.buff[i];
		}
		*billType=sysMDBMission.buff[0]&0x0f;
	    if( sysMDBMission.buff[0] <= 0x7f )
		{
			return  0x02;   //other message(maybe error status)
		}
		else if((sysMDBMission.buff[0]&0xf0)==0xa0) 
		{
			return	0x01;
		}
		return	0x00;
	}	
}

//-------------------------------------
//------------------------------------
//0x80 = com error
//0x00 = not, 0x01=DBReject,0x02=其他状态码
uchar MDB_Bill_IfDBReject( uchar *buf, uchar *len, uchar *billType )  //是否退出过禁用纸币
{
	uchar IfDBReject_r[1]={0};
	uchar	candu;
	uchar	i;
		
    MDBSerialInit();
	candu=Session(0x33,IfDBReject_r,0);  //轮询纸币机,poll
	if( candu >= 0x80 ) 
	{
	    return candu;
	}
	else
	{
		*len=candu;
		if(candu==0) return 0x00;
		for(i=0;i<candu;i++)
		{
			buf[i]=sysMDBMission.buff[i];
		}
		*billType=sysMDBMission.buff[0]&0x0f;
	    if( sysMDBMission.buff[0] <= 0x7f )
		{
			return  0x02;   //other message(maybe error status)
		}
		else if((sysMDBMission.buff[0]&0xf0)==0xc0) 
		{
			return	0x01;
		}
		return	0x00;
	}	
}
//-------------------------------------
//------------------------------------
//压钞与退钞控制
//0x80: comms error
//0x00: ok
uchar MDB_Bill_EscrowCtr( uchar control )  
{
    uchar EscrowCtr_r[1];
	uchar	candu;
		
    MDBSerialInit();
    EscrowCtr_r[0]=control;
	candu=Session(0x35,EscrowCtr_r,1);  //
	if(candu>=0x80) return	0x80;
	else  return	0x00;

}

//-------------------------------------
//------------------------------------


//纸币类型控制，保护纸币使能与暂存功能
//0x80: comms error
//0x00: ok
uchar MDB_Bill_Type( uchar enable1, uchar enable2, uchar escrow1, uchar escrow2 )  
{
     uchar TYPE_r[4];
	 TYPE_r[0]=enable1;
	 TYPE_r[1]=enable2;
     TYPE_r[2]=escrow1;
	 TYPE_r[3]=escrow2;
    return Session(0x34,TYPE_r,4); 
}
//-------------------------------------
//-------------------------------------
//钞箱状态查询（钞箱被取出，或设备上电复位后纸币数量会被清零 ）
//0x80: comms error
//0x00: ok
//(Fxxxxxxx)  (xxxxxxxx) 
//F = 1 if stacker is full, 0 if not.
//xxxxxxxxxxxxxxx  =  The number of bills in the stacker.
uchar MDB_Bill_Stacker( uchar * dat1, uchar *dat2 )  
{
     
    uchar Stacker_r[1]={0};
	uchar	candu;
		
    MDBSerialInit();
	candu=Session(0x36,Stacker_r,0);  //
	*dat1=sysMDBMission.buff[0]&0x7f;
	*dat2=sysMDBMission.buff[1];


    //添加钞箱是否已满的查询;by gzz 20111017
    if(sysMDBMission.buff[0]&0x80)
    	sysMDBMission.billIsFull = 1;
    else
        sysMDBMission.billIsFull = 0;

	if(candu>=0x80) return	0x80;
	else  return	0x00;
}


//------------------------------------------------------------------------------
//0x80: comms error
//0x00: ok
//z1-z33
uchar MDB_Bill_EXP_Identification(uchar *buf, uchar *len)
{
	uchar VMCdata[1]={0x00};
	uchar length;
	uchar i;
	
    //MDBSerialInit();
	length=Session(0x37,VMCdata,1);
	if(length>=0x80) return length;
    else
    {
	  for(i=0;i<length;i++)
	  {
	    buf[i]=sysMDBMission.buff[i];
		sysMDBMission.billIDENTITYBuf[i] = buf[i];
	  }
	  *len=length;
	  return 0x00;
    }    
}



//以下是硬币机的函数
//------------------------------------------------------------------------------
//硬币器启用
unsigned char MDBMission_Coin_Init( void )
{
	
    uchar flag = 0;
	u_int  CoinDecimal = 0;
	u_char CoinScale = 0,i = 0;
	
	DelayMs( 100 );
    MDBSerialInit();
	DelayMs( 100 );

	flag=MDB_Coin_Reset();
	if( flag != 0 )
	{
		//sysMDBMission.coinDeviceStatus |=0x01;
	}
	else
	{
        sysMDBMission.coinDeviceStatus &=0xfe;
	}

	DelayMs( 1000 );
	flag = MDB_Coin_Poll( sysMDBMission.coinBuf, &sysMDBMission.coinBufLen);
    if( flag != 0 )
		sysMDBMission.coinDeviceStatus |=0x01;
	else
        sysMDBMission.coinDeviceStatus &=0xfe;
	DelayMs( 100 );
	flag=MDB_Coin_Setup( sysMDBMission.coinBuf, &sysMDBMission.coinBufLen);
	if(flag!=0)
	{
		sysVPMission.scalFactor = 50;
		sysVPMission.decimalPlaces = 1;
		sysMDBMission.coinDeviceStatus|=0x01;
	}
	else
	{
		sysVPMission.scalFactor = sysMDBMission.coinBuf[3];
		sysVPMission.decimalPlaces = sysMDBMission.coinBuf[4];
		//计算各通道面值
		
		CoinDecimal = 100;
	    for(i = 0; i < sysMDBMission.coinBuf[4]; i++) 
	    {
		   CoinDecimal /= 10;
	    }
		CoinScale = sysMDBMission.coinBuf[3] * CoinDecimal;
		for(i=0;i<6;i++)
		{
			if(sysMDBMission.coinBuf[7+i] == 0xFF)
			{
			     break;
			}			
			SystemParameter.coinValue[i] = (u_int)sysMDBMission.coinBuf[7+i] * CoinScale;
			if( (SystemParameter.coinValue[i]>5000)&&(SystemParameter.coinValue[i]!=10000) )
				SystemParameter.coinValue[i]=0;
		}	
		/**/
        sysMDBMission.coinDeviceStatus&=0xfe;
	}
	DelayMs( 100 );
	flag = MDB_Coin_EXP_Identification( sysMDBMission.coinBuf, &sysMDBMission.coinBufLen);
    if( flag != 0 )
		sysMDBMission.coinDeviceStatus |=0x01;
	else
        sysMDBMission.coinDeviceStatus &=0xfe;
	DelayMs( 100 );
    //
	flag=MDB_Coin_TubeSta_API();
	if(flag!=0)
    {
		sysMDBMission.coinDeviceStatus|=0x01;
    }
	else
    {
        sysMDBMission.coinDeviceStatus&=0xfe;
    }
	DelayMs( 100 );
    //
	flag=MDBMission_Coin_Enable();
	if(flag!=0)
		sysMDBMission.coinDeviceStatus |= 0x01;
	else
        sysMDBMission.coinDeviceStatus &= 0xfe;
	DelayMs( 100 );
	flag=MDB_Coin_Poll( sysMDBMission.coinBuf, &sysMDBMission.coinBufLen );
	if( flag == 0 )
	{
		sysMDBMission.coinDeviceStatus &=0xfe;
	}
	else
	{
		sysMDBMission.coinDeviceStatus |=0x01;
	}
	DelayMs( 100 );     
    flag = MDB_Coin_EXP_FeatureEnable( 0xff, 0xff, 0xff, 0xff );
	if( flag == 0 )
	{
		sysMDBMission.coinDeviceStatus &=0xfe;
	}
	else
	{
		sysMDBMission.coinDeviceStatus |=0x01;
	}
	DelayMs( 100 );
	return ITL_ERR_NULL;
}



//硬币器使能
uchar MDBMission_Coin_Enable(void)
{
   uchar mission;
   uchar typesec = 0;
   //1.
   MDBSerialInit();
   //2.
   //添加是否开启硬币器的控制;by gzz 20111019
   if(SystemParameter.coinOn==1)
   {
	   if(SystemParameter.coinValue[0]>0)
	    	typesec |= 0x01;
	   if(SystemParameter.coinValue[1]>0)
	    	typesec |= 0x02;
	   if(SystemParameter.coinValue[2]>0)
	    	typesec |= 0x04;
	   if(SystemParameter.coinValue[3]>0)
	    	typesec |= 0x08;
	   if(SystemParameter.coinValue[4]>0)
	    	typesec |= 0x10;
	   if(SystemParameter.coinValue[5]>0)
	    	typesec |= 0x20;
   }

   mission=MDB_Coin_Type(  0x00, typesec, 0xff, 0xff );
   if(mission!=0)
   {
		sysMDBMission.coinDeviceStatus |= 0x01;
		return 0x80;
   }
   else
   {
        sysMDBMission.coinIsEnable = 1;
        sysMDBMission.coinDeviceStatus &= 0xfe;
		return 0x00;
   }  
}


//------------------------------------------------------------------------------
//硬币器禁能
uchar MDBMission_Coin_Disable(void)
{
   uchar mission;
   //1.
   MDBSerialInit();
   //2.
   mission=MDB_Coin_Type(  0x00, 0x00, 0xff, 0xff );
  //3.
   if( mission == 0x00 )
   {
        sysMDBMission.coinIsEnable = 0;
        sysMDBMission.coinDeviceStatus &= 0xfe;
   		return 0x00;
   }
   else
   {
   		sysMDBMission.coinDeviceStatus |= 0x01;//by gzz 20120323
   		return 0x80;
   }
}


//------------------------------------------------------------------------------
//Reset 命令
//0x80 = com error
//0x00 = ok
uchar MDB_Coin_Reset(void)
{
    uchar VMCdata[1]={0};
    return Session(0x08,VMCdata,0);  //复位纸币机,
}

//------------------------------------------------------------------------------
//Setup 命令
//0x80 = com error
//0x00 = ok
uchar MDB_Coin_Setup(uchar *buf,uchar *len)
{
	uchar VMCdata[1]={0};
	uchar length;
	uchar i;

	length=Session(0x09,VMCdata,0);
	if(length>=0x80) return length;
    else
    {
	  for(i=0;i<length;i++)
	  {
	    buf[i]=sysMDBMission.buff[i];
		if(i>6)
			sysMDBMission.Pc_coinBuf[i]=(buf[i]>100)?0:buf[i];
		else
			sysMDBMission.Pc_coinBuf[i]=buf[i];	
	  }
	  *len=length;
	  return 0x00;
    }
}

//------------------------------------------------------------------------------
//查询，各个找币斗的当前状态
//After reset more 500 ms, 
//0x80: comms error
//0x00: ok
uchar MDB_Coin_TubeSta(uchar *buf,uchar *len)  
{
	uchar VMCdata[1]={0};
	uchar length;
	uchar i;
	
	//MDBSerialInit();
	length=Session(0x0a,VMCdata,0);
	if(length>=0x80)
    {
 		return length;
    }
	else
	{  
	  for(i=0;i<length;i++)
      {
	    buf[i]=sysMDBMission.buff[i];
	    *len=length;
      }
	  return 0x00;
    }    
}


uchar MDB_Coin_TubeSta_API( void )  
{
	uchar mission;

	MDBSerialInit();
    mission = MDB_Coin_TubeSta( sysMDBMission.coinBuf, &sysMDBMission.coinBufLen);
    if( mission == 0x00 )
    {
	    if( sysMDBMission.coinBufLen >= 4 )
		{
	    	sysMDBMission.coin5jNum = sysMDBMission.coinBuf[2];
	        sysMDBMission.coin1yNum = sysMDBMission.coinBuf[3];
	        sysMDBMission.coin5jValue = sysMDBMission.coin5jNum*SystemParameter.coinValue[0];
	        sysMDBMission.coin1yValue = sysMDBMission.coin1yNum*SystemParameter.coinValue[1];
            sysMDBMission.coinAllValue = sysMDBMission.coin5jValue + sysMDBMission.coin1yValue;
			
        }
		sysMDBMission.coinDeviceStatus &= 0xfe;
		return 0;
    }
	else
	{
	    sysMDBMission.coinDeviceStatus |= 0x01;
	    return mission;	
	}

}

//------------------------------------------------------------------------------
//轮询
//0x80 = com error
//0x00 = ok
uchar MDB_Coin_Poll(uchar *buf,uchar *len)
{
	uchar VMCdata[1]={0};
	uchar length;
	uchar i;

    length=Session(0x0b,VMCdata,0);
    if(length>=0x80) return length;
    else
    {
	  for(i=0;i<length;i++)
	    buf[i]=sysMDBMission.buff[i];
	  *len=length;
	  return 0x00;
    }    
}

uchar MDB_Coin_Poll_API( void )
{
    uchar mission = 0;
	//1.
    MDBSerialInit();
    //2.
	mission =   MDB_Coin_Poll( sysMDBMission.coinBuf, &sysMDBMission.coinBufLen );
	if( mission == 0x00 )
	{
	    sysMDBMission.coinDeviceStatus &= 0xfe; 
	}
	else
	{
		sysMDBMission.coinDeviceStatus |= 0x01;
	}
    return 0;
}

//------------------------------------------------------------------------------
//是否退币
//0x80 = com error
//0x00 = not, 0x01=dispensed, 0x02=其他状态码
uchar MDB_Coin_IfDispensed(uchar *buf,uchar *len,uchar *coinType,uchar *coinDispensedNum,uchar *coinStock)
{
	uchar VMCdata[1]={0};
	uchar length;
	uchar i;

	MDBSerialInit();
	length=Session(0x0b,VMCdata,0);
	if(length>=0x80) return length;
    else
    {
		  *len=length;
		  if(length==0) return 0x00;
		  for(i=0;i<length;i++)
		    buf[i]=sysMDBMission.buff[i];
		  if(sysMDBMission.buff[0]<0x40) return	0x02;
		  if(sysMDBMission.buff[0]&0x80)
	      {
		    *coinDispensedNum=(sysMDBMission.buff[0]&0x70)/0x10;
		    *coinType=sysMDBMission.buff[0]&0x0f;
		    *coinStock=sysMDBMission.buff[1];
		    return 0x01;
	      }
	      return 0x00;
	 }    
}

//------------------------------------------------------------------------------
//是否接收到硬币
//0x80 = com error
//0x00 = not, 0x01=escrow, 0x02=其他状态码
uchar MDB_Coin_IfDeposited(uchar *buf,uchar *len,uchar *coinType,uchar *coinStock )  
{
  
  uchar VMCdata[1]={0};
  uchar length;
  uchar i;
  static uchar errFlag=0;
  //u_char xdata str[20];
  //u_char xdata lens = 0;

  MDBSerialInit();
  DelayMs( 100 );
  length=Session(0x0b,VMCdata,0);
  if(length>=0x80) 
  {
  	if(errFlag>=5)
	{
    	sysMDBMission.coinDeviceStatus |= 0x01;
	}
	else
	{
		errFlag++;
	}    
	//DisplayStr( 0, 0, 1, "coinerr", 7 );
	//WaitForWork( 2000, NULL );
  	return length;
  }
  else
  {
  	  sysMDBMission.coinDeviceStatus &= 0xfe;
	  errFlag=0;	
	  *len=length;
	  if(length==0) 
	  {
	  	sysMDBMission.coinOthErr = 0;
	  	return 0x00;
	  }
      for(i=0;i<length;i++)
      {
      	buf[i]=sysMDBMission.buff[i];
      }
	  //lens = sprintf( str, "coin=%x,%x" , buf[0],buf[1]);	
	  //DisplayStr( 0, 0, 1, str, lens );
	  //WaitForWork( 2000, NULL );			
	  for(i = 0; i < length; i++) 
	  { 
		  //report changer activity
		  //coins dispensed manually手动退币
		  if(sysMDBMission.buff[i] & 0x80) 
		  {        	      
				i++; //忽略第2字节
		  } 
		  else if((sysMDBMission.buff[i] & 0xC0) == 0x40) 
		  {	 
				//coins deposited有硬币投入
				if(((sysMDBMission.buff[i] & 0x30) == 0x10) || ((sysMDBMission.buff[i] & 0x30) == 0)) //in tubes
				{
				    *coinType=sysMDBMission.buff[i]&0x0f;
				    *coinStock=sysMDBMission.buff[i+1];
					//DisplayStr( 0, 0, 1, "getcoin", 7 );
				    //WaitForWork( 2000, NULL );
			        return 0x01;
			    }
				i++; //忽略第2字节
		  } 
		  else if((sysMDBMission.buff[i] & 0xE0) == 0x20) 
		  {	 //slug
		  }
	      else
	      {
	      	/*
	          //------------------------
	          if( sysMDBMission.buff[i] == 0x01 )
	          {
	          	  //DisplayStr( 0, 0, 1, "1", 1 );
		  		  //WaitForWork( 2000, NULL );
	              sysMDBMission.returnKeyFlag = 1;
	          }
			  */
			    switch(sysMDBMission.buff[i]) 
				{						
				   case 0x01:			                 //escrow request	
				   	   sysMDBMission.returnKeyFlag = 1;
					   break;

				   case 0x02:			                 //changer pay out busy
					   break;

				   case 0x03:			                 //no credit
					   //不能进入对应通道
					   break;

				   case 0x04:			                 //defective tube sensor				   	   	
					   //传感器故障
					   sysMDBMission.coinOthErr |= 0x01;
					   break;

				   case 0x05:			                 //double arriver
				   	   break;

				   case 0x06:			                 //acceptor unplugged						   
					   break;

				   case 0x07:			                 //tube jam
					   //出币卡币
					   sysMDBMission.coinOthErr |= 0x02;
					   break;

				   case 0x08:			                 //rom chksum err
					   //ROM出错
					   sysMDBMission.coinOthErr |= 0x04;
					   break;

				   case 0x09:			                 //coin routing err
					   //进币通道出错
					   sysMDBMission.coinOthErr |= 0x08;
					   break;

				   case 0x0A:			                 //changer busy
				   	   break;

				   case 0x0B:			                 //changer was reset
					   //刚复位
					   break;

				   case 0x0C:			                 //coin jam
					   //投币卡币
					   sysMDBMission.coinOthErr |= 0x10;
					   break;

				   case 0x0D:		              	     //possible credited coin removal
					   //试图移除硬币
					   sysMDBMission.coinOthErr |= 0x20;
					   break;

				   default:	
				   	   sysMDBMission.coinOthErr = 0;	
					   break;
				}
	          	//return 0x02;
	      }	      
	  }
      return 0x00;
  }    
}


//------------------------------------------------------------------------------
//硬币类型控制，各通道硬币收币与退币允许功能
//0x80: comms error
//0x00: ok
uchar MDB_Coin_Type(uchar typeCtr1,uchar typeCtr2,uchar dispenseCtr1,uchar dispenseCtr2)  
{
	uchar VMCdata[4];
	VMCdata[0]=typeCtr1;
	VMCdata[1]=typeCtr2;
	VMCdata[2]=dispenseCtr1;
	VMCdata[3]=dispenseCtr2;
	return Session(0x0c,VMCdata,4);     
}

//硬币器,纸币器禁用,但是启用退币器
uchar MDB_Coin_HopperEnable(void)
{
   uchar mission;
   DelayMs( 100 );
   //1.
   MDBSerialInit();
   DelayMs( 100 );
   //2.
   mission=MDB_Coin_Type(  0x00, 0x00, 0xff, 0xff );
   DelayMs( 100 );
   mission=MDB_Bill_Type(  0x00, 0x00, 0x00, 0x00 );
   DelayMs( 100 );
   //3.
   ITLSerialInit();
   if(mission&0x80)
   		return ITL_ERR_COM;
   else
   		return ITL_ERR_NULL;
}


//------------------------------------------------------------------------------
//手动找币
//0x80: comms error
//0x00: ok
uchar MDB_Coin_Dispense( uchar type, uchar num ) 
{
    
	uchar VMCdata[1];	
    MDBSerialInit();
	VMCdata[0] =  type&0x0f;
    VMCdata[0] |= ((num<<4)&0xf0);
	return Session(0x0d,VMCdata,1);
}

//------------------------------------------------------------------------------
//0x80: comms error
//0x00: ok
//z1-z33
uchar MDB_Coin_EXP_Identification(uchar *buf, uchar *len)
{
	uchar VMCdata[1]={0x00};
	uchar length;
	uchar i;
	
    //MDBSerialInit();
	length=Session(0x0f,VMCdata,1);
	if(length>=0x80) return length;
    else
    {
	  for(i=0;i<length;i++)
	  {
	    buf[i]=sysMDBMission.buff[i];
		sysMDBMission.coinIDENTITYBuf[i] = buf[i];
	  }
	  *len=length;
	  return 0x00;
    }    
}

//------------------------------------------------------------------------------
//0x80: comms error
//0x00: ok
//none
uchar MDB_Coin_EXP_FeatureEnable(uchar dat1,uchar dat2,uchar dat3,uchar dat4) 
{
	uchar VMCdata[5]={0x01, 0x00, 0x00, 0x00, 0x00 };
	VMCdata[1]=dat1;
	VMCdata[2]=dat2;
	VMCdata[3]=dat3;
	VMCdata[4]=dat4;	
    //MDBSerialInit();
    return Session(0x0f,VMCdata,5);
}

//------------------------------------------------------------------------------
//0x80: comms error
//0x00: ok
//none
uchar MDB_Coin_EXP_Payout(uchar dat) 
{
    uchar VMCdata[2]={0x02, 0x00 };
    VMCdata[1]=dat;
    MDBSerialInit();
    return Session(0x0f,VMCdata,2);
}

//------------------------------------------------------------------------------
//0x80: comms error
//0x00: busy, none coin payouted
//0x01: There is/are coin(s) payouted
uchar MDB_Coin_EXP_PayoutStatus(uchar *buf,uchar *len) 
{
	uchar VMCdata[1]={0x03};
	uchar length;
	uchar i,j;
	
    MDBSerialInit();
    length=Session(0x0f,VMCdata,1);
    if(length>=0x80) return length;
    else
    {
	    for(i=0;i<length;i++)
	    {
	    	buf[i]=sysMDBMission.buff[i];
	        if(sysMDBMission.buff[i]!=0)
	        	j=0xff;
	    }
        *len=length;
        if(j) return 0x01;
        else return 0x00; 
    }    
}

//------------------------------------------------------------------------------
//0x80: comms error
//0x00: ACK(Payout finished)
//0x01: There is/are coin(s) payouted  
uchar MDB_Coin_EXP_PayoutValuePoll(uchar *dat)
{
    uchar VMCdata[1]={0x04};
    uchar length;
	
    MDBSerialInit();
    length=Session(0x0f,VMCdata,1);
    if(length>=0x80) return length;
    else
    {
  		*dat=sysMDBMission.buff[0];
        if(length==0) return 0x00;
        else return 0x01;
    }    
}


//------------------------------------------------------------------------------
//0x80: comms error
//0x00: not
//0x01: Get status finished
//查找币管是否没有锁紧;by gzz 20110825
uchar MDB_Coin_EXP_TubeRemoved(void)
{
    uchar VMCdata[1]={0x05};
    uchar length;
	
    MDBSerialInit();
    length=Session(0x0f,VMCdata,1);
    if(length>=0x80) 
        return length;
    else
    {  	
	    sysMDBMission.tubeRemoved = 0;
        if(length==0) 
        	return 0x00;
        else
        {
            if(sysMDBMission.buff[0]==0x15)
            {
                if(sysMDBMission.buff[1]==0x02)
                {
	                sysMDBMission.tubeRemoved = 1;	        		
                }                 
            }
            return 0x01;
        }        
    }    
}
