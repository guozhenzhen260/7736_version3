#define  VMC_PC_C
#include "debug.h" 
#include "device.h"
//#include "serial1.h"
#include "Serial.h"
//#include "procotol.h" 
//#include "scheduler.h"
#include "string.h"
#include "global.h"
#include "VMC_PC.h"
#include "CommonFunction.h"
#include "key.h"
#include "ITL.h"
#include "BusCard.h"
#include "IOInput.h"//by gzz 20110721
#include "Countermaint.h"//120419 by cq TotalSell


//------------------------------------------
//extern bit  data 	isShowLcd;
extern u_int  xdata CurrentPayed;
extern u_int  xdata HardWareErr;
extern u_int  xdata CurrentPayed;
extern u_int  xdata CurrentDispenseCoin;
extern u_char xdata SystemStatus;
extern struct HopperSet xdata HopperSetList[HOPPERNUM];  //0存放最大面值,1存放第二大面值,依此类推
extern unsigned char OutGoods( unsigned char type );
extern u_char DispenseCoin(unsigned char flag );
extern u_char DisableBillDev();
extern u_char StackTheBillAPI( void );
extern u_char DisplayCurrentMoney( u_int Money );
extern void SellAccumulateNum(unsigned long * ul_num);
extern void SellAccumulateMoney(unsigned long * ul_money, unsigned int ui_cost);
u_char IsCanChange(u_int    Price);

//==========================================

unsigned char xdata VPMsgBuf[255];
unsigned char xdata GoodsType[124];
	u_int     xdata Price[124]; 

/*
extern SERAILPARAM xdata ZhkSerialParam;
SERAILPARAM code VPSerialParam = 
{ 
	8, VERIFY_NULL, 1, 0x00, 0x96, 0x00 	
};
*/

//121121 by cq PC_VER_3
unsigned short calc_crc(unsigned char * msg, unsigned short len)
{
	unsigned short i,j;
	unsigned short crc = 0;
	unsigned short current;
	for(i = 0; i < len; i++)
	{
		current = msg[i] << 8;
		for(j = 0; j < 8; j++)
		{
			if((short)(crc ^ current) < 0)
				crc = (crc << 1) ^ 0x1021;
			else
				crc <<= 1;
			current <<= 1;
		}
	}
	return crc;
}

unsigned char g_msg[300];


void VPSerialInit( void )
{
	/*
    unsigned char data i;
	for( i=0; i<sizeof( SERAILPARAM ); i++ )
		ZhkSerialParam[i] = VPSerialParam[i];
	ZhkSerialInit();
    ES = 1;  //2009.03.02 added
	*/
	memset( VPMsgBuf, 0, sizeof(VPMsgBuf) );
}

//CHK and send the message
unsigned char VPBusTxMsg( void )
{
	unsigned char  data i   = 0;
	//unsigned char  data len = 0;
	//unsigned char  data sum = 0;
	//unsigned char xdata DataString[255];
	
	//1.caculate the CHK
	//sysVPMission.send.datLen = sysVPMission.send.len - 5;
    memset(g_msg,0x00,sizeof(g_msg));
	g_msg[0] = sysVPMission.send.sf;
	g_msg[1] = sysVPMission.send.len;
	g_msg[2] = sysVPMission.send.verFlag;
	g_msg[3] = sysVPMission.send.msgType;
	g_msg[4] = sysVPMission.send.sn;
	for( i = 0; i < sysVPMission.send.datLen; i++ )
	{
		g_msg[5+i] += sysVPMission.send.msg[i]; 
	}
	
	sysVPMission.send.chk = calc_crc(g_msg,sysVPMission.send.len);
	//2.send the message
	ZhkSerialPutCh( sysVPMission.send.sf );
	ZhkSerialPutCh( sysVPMission.send.len );
	ZhkSerialPutCh( sysVPMission.send.verFlag );
	ZhkSerialPutCh( sysVPMission.send.msgType );
	ZhkSerialPutCh( sysVPMission.send.sn );
    for( i=0; i<sysVPMission.send.datLen; i++  )
	{
		ZhkSerialPutCh( sysVPMission.send.msg[i] );
	}
	ZhkSerialPutCh( sysVPMission.send.chk/256 );
    ZhkSerialPutCh( sysVPMission.send.chk%256 );
	return 1;
}


unsigned char VPBusFrameUnPack( void )
{
	unsigned char data i=0, j=0, k=0, m=0;
	unsigned char data len = 0;
	unsigned int  data sum = 0;
	
	while( !ZhkSerialIsRxBufNull() )
	{
		for( i=0; i<sizeof( VPMsgBuf )-1; i++ )
		{
			VPMsgBuf[i] = VPMsgBuf[i+1];
		}
		VPMsgBuf[sizeof(VPMsgBuf)-1] = ZhkSerialGetCh();
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
			Trace( " %02bx", VPMsgBuf[32+j] );
		j = 0;
	}
#endif
*/
	for( i=0; i<=sizeof(VPMsgBuf)-7; i++ )
	{
	    //Check the SF
		if( VPMsgBuf[i] != VP_SF ) 
			continue;
		//DisplayStr( 0, 0, 1, "A1", 2 );  
		//WaitForWork(2000,NULL);
		//Check the len
		len = VPMsgBuf[i+1];
		if( !(len >=5) ) 
		    continue;
		//DisplayStr( 0, 0, 1, "A2", 2 );  
		//WaitForWork(2000,NULL);
		if( i+len+2 > sizeof( VPMsgBuf ) ) 
		    break;	
		//DisplayStr( 0, 0, 1, "A3", 2 );  
		//WaitForWork(2000,NULL);
		//Check the CHK
		sum = 0;
		sum = calc_crc(VPMsgBuf + i, len);
		if( (VPMsgBuf[i+len] != sum/256)||(VPMsgBuf[i+len+1] != sum%256)	)
		{
			continue;
	    }
		//DisplayStr( 0, 0, 1, "A4", 2 );  
		//WaitForWork(2000,NULL);
		//Check the message type
        if( !((VPMsgBuf[i+3]>=VP_MT_MIN_RECEIVE) && (VPMsgBuf[i+3]<=VP_MT_MAX_RECEIVE)) )   
            continue;     
		//Save the message
		sysVPMission.receive.sf      = VPMsgBuf[i];
		sysVPMission.receive.len     = VPMsgBuf[i+1];
        sysVPMission.receive.verFlag = VPMsgBuf[i+2];
 	    sysVPMission.receive.msgType = VPMsgBuf[i+3];
		sysVPMission.receive.sn      = VPMsgBuf[i+4];
        sysVPMission.receive.datLen  = sysVPMission.receive.len - 5;
		for( m=0,k=i+5; k<i+5+sysVPMission.receive.datLen; m++,k++)
		{
			sysVPMission.receive.msg[m] = VPMsgBuf[k];	
		}
		sysVPMission.receive.chk = sysVPMission.receive.msg[k]*256 + sysVPMission.receive.msg[k+1];
		memset( VPMsgBuf, 0, sizeof(VPMsgBuf) );
/*
#ifdef _DEBUG_TRACE		
		Trace( "\n Bus Return Unpack = ");
		for( j = 0; j < 32; j++ )
			Trace( " %02bx", VPMsgBuf[32+j] );		
#endif
*/       
		return 1;
	}	
	return 0;
}

/***********************************************************
vmc内部处理都是以分为单位，但是在上传给pc时需要转换

decimalPlaces=1以角为单位
	例如，需要上传给pc的是200分,
	200*10=2000
	2000/100=20角，即上传20角
	
decimalPlaces=2以分为单位
	例如，需要上传给pc的是200分,
	200*100=20000
	20000/100=200分，即上传200分	

decimalPlaces=0以元为单位
	例如，需要上传给pc的是200分,
	200=200
	200/100=2元，即上传2元	
***************************************************************/
u_int   MoneySend(u_int sendMoney)
{
	u_int tempMoney;
	
	//公式2: 上传ScaledValue = ActualValue元/(10-DecimalPlaces次方)
	if(sysVPMission.decimalPlaces==1)
	{
		//tempMoney = sendMoney*10;
		//tempMoney = tempMoney/100;
		tempMoney = sendMoney/10;
	}
	else if(sysVPMission.decimalPlaces==2)
	{
		tempMoney = sendMoney;
	}
	else if(sysVPMission.decimalPlaces==0)
	{
		//tempMoney = sendMoney;
		//tempMoney = tempMoney/100;
		tempMoney = sendMoney/100;
	}
	//例如:600分=6元
	return tempMoney;
}
u_long   MoneySendInfo(u_long sendMoney)
{
	u_long tempMoney;
	
	//公式2: 上传ScaledValue = ActualValue元/(10-DecimalPlaces次方)
	if(sysVPMission.decimalPlaces==1)
	{
		//tempMoney = sendMoney*10;
		//tempMoney = tempMoney/100;
		tempMoney = sendMoney/10;
	}
	else if(sysVPMission.decimalPlaces==2)
	{
		tempMoney = sendMoney;
	}
	else if(sysVPMission.decimalPlaces==0)
	{
		//tempMoney = sendMoney;
		//tempMoney = tempMoney/100;
		tempMoney = sendMoney/100;
	}
	//例如:600分=6元
	return tempMoney;
}

/***********************************************************
vmc内部处理都是以分为单位，所以接收pc传下来的数据时需要转换

decimalPlaces=1以角为单位
	例如，pc下传是20角,
	20*100=2000
	2000/10=200分，即接收200分
	
decimalPlaces=2以分为单位
	例如，pc下传是200分,
	200*100=20000
	20000/100=200分，即接收200分	

decimalPlaces=0以元为单位
	例如，pc下传是2元,
	2*100=200
	200=200分，即接收200分
***************************************************************/
u_int   MoneyRec(unsigned char recMoneyH,unsigned char recMoneyL)
{
	u_int tempMoney;
	
	tempMoney = recMoneyH*256+recMoneyL;
	//公式1:  ActualValue元 =下载ScaledValue*(10-DecimalPlaces次方)
	if(sysVPMission.decimalPlaces==1)
	{
		//tempMoney = tempMoney*100;
		//tempMoney = tempMoney/10;
		tempMoney = tempMoney*10;
	}
	else if(sysVPMission.decimalPlaces==2)
	{
		tempMoney = tempMoney;
	}
	else if(sysVPMission.decimalPlaces==0)
	{
		tempMoney = tempMoney*100;
		//tempMoney = tempMoney;
	}
	//例如:6元=600分	
	return tempMoney;
}


unsigned char VPMsgPackSend( unsigned char msgType, unsigned char flag, unsigned char snCtr )
{
    
    unsigned char data i=0,j=0;
    unsigned int data CurrentMoney=0;
	u_int tempMoney;
	unsigned char data tempSend=0;
	u_long tradeMoney=0;
	//u_char xdata str[20];
	//u_char xdata len = 0;
	u_char xdata salei,salej = 0;
	u_int xdata saleMoney=0;

	if( !((msgType>=VP_TEXT_MSG)&&(msgType<=VP_MT_MAX_SEND)) )
		return VP_ERR_PAR;
    if( !((flag>=0)&&(flag<=1)) )
	    return VP_ERR_PAR;

	switch( msgType )
	{
		case VP_TEXT_MSG:
			{
			}
			break;
		case VP_ACK_RPT:
            {
				sysVPMission.send.msgType = VP_ACK_RPT;
                sysVPMission.send.datLen = 0;
			}
			break;
		case VP_NAK_RPT:
		    {
				sysVPMission.send.msgType = VP_NAK_RPT;
                sysVPMission.send.datLen = 0;
			}
			break;
		case VP_POLL:
		    {
			    sysVPMission.send.msgType = VP_POLL;
                sysVPMission.send.datLen = 0;
			}
			break;
		case VP_STARTUP_RPT:
			{
				sysVPMission.send.msgType = VP_STARTUP_RPT;
				sysVPMission.send.datLen = 0;
			}
			break;
		//120419 by cq TotalSell
		/*
		case VP_STARTUP_RPT_1:
			{
				sysVPMission.send.msgType = VP_STARTUP_RPT_1;
				sysVPMission.send.datLen = 0;
			}
			break;
		*/	
		case VP_VMC_SETUP:
		    {
			    sysVPMission.send.msgType = VP_VMC_SETUP;
                sysVPMission.send.datLen = 10;       //3,5
                sysVPMission.send.msg[0] = sysVPMission.columnNumSet;
				sysVPMission.send.msg[1] = sysVPMission.selectionNumSet;				
				sysVPMission.send.msg[2] = VP_MAC_SET_H;				
				#ifdef   _SHUPING_
					sysVPMission.send.msg[3] = 10;
				#else
                	sysVPMission.send.msg[3] = VP_MAC_SET_L;
				#endif
				sysVPMission.send.msg[4] = 0x0;
				sysVPMission.send.msg[5] = 0x0;
				sysVPMission.send.msg[6] = 0x0;
				sysVPMission.send.msg[7] = 0x2;
				sysVPMission.send.msg[8] = 0x87;
				sysVPMission.send.msg[9] = 0x07;//0x87; //
			}
			break;
		case VP_PAYIN_RPT:
		    {
			    sysVPMission.send.msgType = VP_PAYIN_RPT;
                sysVPMission.send.datLen = 5;      //3,5
				sysVPMission.send.msg[0] = sysVPMission.payInDev;
				tempMoney = MoneySend(sysVPMission.payInMoney);
				sysVPMission.send.msg[1] = tempMoney/256;
				sysVPMission.send.msg[2] = tempMoney%256;
				//PAYIN_RPT协议中末尾添加两个字节
                //Total_Value：表示当前交易投币后，或者出货后，屏幕上显示的金额数 
				//by gzz 20110721
				if( sysITLMission.billHoldingFlag == 1 )
				{
					CurrentMoney = CurrentPayed + sysITLMission.billHoldingValue;
				}
				else
				{
					CurrentMoney = CurrentPayed;
				}
				tempMoney = MoneySend(CurrentMoney);
                sysVPMission.send.msg[3] = tempMoney/256;
                sysVPMission.send.msg[4] = tempMoney%256;
			}
			break;
		case VP_PAYOUT_RPT:
		    {
			    sysVPMission.send.msgType = VP_PAYOUT_RPT;
				sysVPMission.send.datLen = 6;   
                sysVPMission.send.msg[0] = sysVPMission.payOutDev;
				tempMoney = MoneySend(sysVPMission.payOutMoney1+sysVPMission.payOutMoney2);
				sysVPMission.send.msg[1] = tempMoney/256;
				sysVPMission.send.msg[2] = tempMoney%256; 				
				//PAYIN_RPT协议中末尾添加两个字节
                //Total_Value：表示当前交易投币后，或者出货后，屏幕上显示的金额数 
				//by gzz 20110721
				if( sysITLMission.billHoldingFlag == 1 )
				{
					CurrentMoney = CurrentPayed + sysITLMission.billHoldingValue;
				}
				else
				{
					CurrentMoney = CurrentPayed;
				}
				tempMoney = MoneySend(CurrentMoney);
                sysVPMission.send.msg[3] = tempMoney/256;
                sysVPMission.send.msg[4] = tempMoney%256;
				sysVPMission.send.msg[5] = sysVPMission.changeType;
				sysVPMission.changeType = 0;
			}
			break;
        case VP_COST_RPT://扣款操作;by gzz 20110823
		    {
			    sysVPMission.send.msgType = VP_COST_RPT;
				sysVPMission.send.datLen = 6;   
                sysVPMission.send.msg[0] = sysVPMission.costDev;
				tempMoney = MoneySend(sysVPMission.costMoney);
				sysVPMission.send.msg[1] = tempMoney/256;
				sysVPMission.send.msg[2] = tempMoney%256;                 
				//PAYIN_RPT协议中末尾添加两个字节
                //Total_Value：表示当前交易投币后，或者出货后，屏幕上显示的金额数 
				//by gzz 20110721
				if( sysITLMission.billHoldingFlag == 1 )
				{
					CurrentMoney = CurrentPayed + sysITLMission.billHoldingValue;
				}
				else
				{
					CurrentMoney = CurrentPayed;
				}
				tempMoney = MoneySend(CurrentMoney);
                sysVPMission.send.msg[3] = tempMoney/256;
                sysVPMission.send.msg[4] = tempMoney%256;
				sysVPMission.send.msg[5] = sysVPMission.costType;
				sysVPMission.costType = 0;
			}
			break;
			/*
		case VP_DEBT_RPT://上报欠条操作;by gzz 20110825
		    {
			    sysVPMission.send.msgType = VP_DEBT_RPT;
				sysVPMission.send.datLen = 4;   
                sysVPMission.send.msg[0] = sysVPMission.debtType;
				sysVPMission.send.msg[1] = sysVPMission.debtDev;
				sysVPMission.send.msg[2] = sysVPMission.debtMoney/256;
				sysVPMission.send.msg[3] = sysVPMission.debtMoney%256;   
			}
			break;	*/		
		case VP_VENDOUT_RPT:
		    {
				sysVPMission.send.msgType = VP_VENDOUT_RPT;
                sysVPMission.send.datLen = 9;
				sysVPMission.send.msg[0] = 0;
				sysVPMission.send.msg[1] = sysVPMission.vendSta;
				sysVPMission.send.msg[2] = sysVPMission.vendColumn;
				sysVPMission.send.msg[3] = sysVPMission.vendType;
				tempMoney = MoneySend(sysVPMission.vendCost);
				sysVPMission.send.msg[4] = tempMoney/256;
				sysVPMission.send.msg[5] = tempMoney%256;
				//PAYIN_RPT协议中末尾添加两个字节
                //Total_Value：表示当前交易投币后，或者出货后，屏幕上显示的金额数 
				//by gzz 20110721
				if( sysITLMission.billHoldingFlag == 1 )
				{
					CurrentMoney = CurrentPayed + sysITLMission.billHoldingValue;
				}
				else
				{
					CurrentMoney = CurrentPayed;
				}
				tempMoney = MoneySend(CurrentMoney);
                sysVPMission.send.msg[6] = tempMoney/256;
                sysVPMission.send.msg[7] = tempMoney%256;
				sysVPMission.send.msg[8] = sysVPMission.vendcolumnLeft;
			}
			break;
		case VP_REQUEST: 
            {
				sysVPMission.send.msgType = VP_REQUEST;
				sysVPMission.send.datLen = 1;
				sysVPMission.send.msg[0] = sysVPMission.request;
			}
			break;
		case VP_ADMIN_RPT:
		    {
			    sysVPMission.send.msgType =  VP_ADMIN_RPT;

				if( sysVPMission.ADM_Type == VP_ADMIN_COL_PRICE )
				{
				    sysVPMission.send.datLen = 4;
					sysVPMission.send.msg[0] = sysVPMission.ADM_Type;
					sysVPMission.send.msg[1] = sysVPMission.ADM_Dat[0];
					sysVPMission.send.msg[2] = sysVPMission.ADM_Dat[1];	
                    sysVPMission.send.msg[3] = sysVPMission.ADM_Dat[2];	
				}
				else if( sysVPMission.ADM_Type == VP_ADMIN_GOODSADDCOL )
				{
					sysVPMission.send.datLen = 3;
					sysVPMission.send.msg[0] = sysVPMission.ADM_Type;
					sysVPMission.send.msg[1] = sysVPMission.ADM_Dat[0];
					sysVPMission.send.msg[2] = sysVPMission.ADM_Dat[1];
				}
                else if( sysVPMission.ADM_Type == VP_ADMIN_GOODSBUHUO )
				{
					sysVPMission.send.datLen = 1;
					sysVPMission.send.msg[0] = sysVPMission.ADM_Type;
					//sysVPMission.send.msg[1] = sysVPMission.ADM_Dat[0];
					//sysVPMission.send.msg[2] = sysVPMission.ADM_Dat[1];
				}
				else if( sysVPMission.ADM_Type == VP_ADMIN_GOODSADDTRAY ) //Add the tray's goods
				{
				    sysVPMission.send.datLen = 2;
					sysVPMission.send.msg[0] = sysVPMission.ADM_Type;
					sysVPMission.send.msg[1] = sysVPMission.ADM_Dat[0];
				}
				else  //Add all columns' goods, chage, get bill
				{
				    sysVPMission.send.datLen = 1;
					sysVPMission.send.msg[0] = sysVPMission.ADM_Type;
				}
				
			}
			break;
        case VP_ACTION_RPT:		    
			sysVPMission.send.msgType = VP_ACTION_RPT;	
			if( (sysVPMission.ACT_Type==VP_ACT_HEART)||(sysVPMission.ACT_Type==VP_ACT_ONLINE) )
			{
				sysVPMission.send.datLen = 1;
				sysVPMission.send.msg[0] = sysVPMission.ACT_Type;
			}
			else if(sysVPMission.ACT_Type==VP_ACT_ADMIN)
			{
				sysVPMission.send.datLen = 2;
				sysVPMission.send.msg[0] = sysVPMission.ACT_Type;
				sysVPMission.send.msg[1] = sysVPMission.ACT_Value;
			}	
			else if(sysVPMission.ACT_Type==VP_ACT_CHUHUO)
			{
				sysVPMission.send.datLen = 8;
				sysVPMission.send.msg[0] = sysVPMission.ACT_Type;
				sysVPMission.send.msg[1] = 5;
				sysVPMission.send.msg[2] = sysVPMission.vendColumn+1;
				sysVPMission.send.msg[3] = sysVPMission.vendType;
				tempMoney = MoneySend(sysVPMission.vendCost);
				sysVPMission.send.msg[4] = tempMoney/256;
				sysVPMission.send.msg[5] = tempMoney%256;
				//PAYIN_RPT协议中末尾添加两个字节
                //Total_Value：表示当前交易投币后，或者出货后，屏幕上显示的金额数 
				//by gzz 20110721
				if( sysITLMission.billHoldingFlag == 1 )
				{
					CurrentMoney = CurrentPayed + sysITLMission.billHoldingValue;
				}
				else
				{
					CurrentMoney = CurrentPayed;
				}
				tempMoney = MoneySend(CurrentMoney);
                sysVPMission.send.msg[6] = tempMoney/256;
                sysVPMission.send.msg[7] = tempMoney%256;
			}
			else if(sysVPMission.ACT_Type==VP_ACT_PAYOUT)
			{
				sysVPMission.send.datLen = 7;
				sysVPMission.send.msg[0] = sysVPMission.ACT_Type;
				sysVPMission.send.msg[1] = CurrentDispenseCoin/50/2+1;				
				tempMoney = MoneySend(CurrentDispenseCoin);
				sysVPMission.send.msg[2] = tempMoney/256;
				sysVPMission.send.msg[3] = tempMoney%256;
				//PAYIN_RPT协议中末尾添加两个字节 
				//Total_Value：表示当前交易投币后，或者出货后，屏幕上显示的金额数 
				//by gzz 20110721
				if( sysITLMission.billHoldingFlag == 1 )
				{
					CurrentMoney = CurrentPayed + sysITLMission.billHoldingValue;
				}
				else
				{
					CurrentMoney = CurrentPayed;
				}
				if(CurrentMoney)
					tempMoney = MoneySend(CurrentMoney);
				else
					tempMoney = MoneySend(CurrentDispenseCoin);
				sysVPMission.send.msg[4] = tempMoney/256;
				sysVPMission.send.msg[5] = tempMoney%256;				
                sysVPMission.send.msg[6] = sysVPMission.changeType;
			}	
			break;
        case VP_BUTTON_RPT:		    
			sysVPMission.send.msgType = VP_BUTTON_RPT;
			if(sysVPMission.BUT_Type==VP_BUT_GOODS)
			{
				sysVPMission.send.datLen = 3;
				sysVPMission.send.msg[0] = sysVPMission.BUT_Type;
				sysVPMission.send.msg[1] = 0;
				sysVPMission.send.msg[2] = sysVPMission.BUT_Value;
			}
			else if(sysVPMission.BUT_Type==VP_BUT_GAME)
			{
				sysVPMission.send.datLen = 2;
				sysVPMission.send.msg[0] = sysVPMission.BUT_Type;
				sysVPMission.send.msg[1] = sysVPMission.BUT_Value;
			}	
			else if(sysVPMission.BUT_Type==VP_BUT_RETURN)
			{
				sysVPMission.send.datLen = 2;
				sysVPMission.send.msg[0] = sysVPMission.BUT_Type;
				sysVPMission.send.msg[1] = sysVPMission.BUT_Value;
			}	
			break;
		case VP_STATUS_RPT:
		    {
			    sysVPMission.send.msgType = VP_STATUS_RPT;
                //sysVPMission.send.datLen  = 15;
				sysVPMission.send.datLen  = 16;

				tempSend |= sysVPMission.STA_VMC;
				tempSend |= sysVPMission.STA_CoinA<<2;
				tempSend |= sysVPMission.STA_BillA<<4;
				tempSend |= sysVPMission.STA_ICCard<<6;				
				sysVPMission.send.msg[0]  = tempSend;
				#ifdef   _SHUPING_
					tempSend = 0xff;
				#else
					tempSend = 0;
					tempSend |= sysVPMission.STA_Tep1;
					tempSend |= sysVPMission.STA_Tep2<<2;
					tempSend |= sysVPMission.STA_Tep3<<4;					
				#endif
				sysVPMission.send.msg[1]  = tempSend;
				
				tempMoney = MoneySend(sysMDBMission.coinAllValue);
                sysVPMission.send.msg[2] = tempMoney/256;
                sysVPMission.send.msg[3] = tempMoney%256;
				
				sysVPMission.send.msg[4]  = sysVPMission.E_Tem1;
				sysVPMission.send.msg[5]  = sysVPMission.E_Tem2;
				sysVPMission.send.msg[6]  = sysVPMission.E_Tem2;
				sysVPMission.send.msg[7]  = sysVPMission.E_Tem2;
				
				tempSend = 0;
				tempSend |= sysVPMission.ESTA_Tem1;
				tempSend |= sysVPMission.ESTA_Tem2<<2;
				tempSend |= sysVPMission.ESTA_Tem2<<4;
				tempSend |= sysVPMission.ESTA_Tem2<<6;	
				sysVPMission.send.msg[8]  = tempSend;
				
				sysVPMission.send.msg[9]  = sysVPMission.Payout_Time;
				
				sysVPMission.send.msg[10] = sysVPMission.STA_ChangeNum1;
				sysVPMission.send.msg[11] = sysVPMission.STA_ChangeNum2;
				sysVPMission.send.msg[12] = 0;
				sysVPMission.send.msg[13] = 0;
				sysVPMission.send.msg[14] = 0;
				sysVPMission.send.msg[15] = 0;				
			}
			break;
        case VP_HUODAO_RPT:
            {
                sysVPMission.send.msgType = VP_HUODAO_RPT;
                sysVPMission.send.datLen  = COLUMN_NUM_SET+1;    
				sysVPMission.send.msg[0]  = 0;
                //
            }
            break;
        case VP_CARD_RPT:
            {
                sysVPMission.send.msgType = VP_CARD_RPT;  
                sysVPMission.send.datLen  = sysBCMission.msgLen + 1;
                sysVPMission.send.msg[0]  = 1;       
                for( i=0; i<sysBCMission.msgLen; i++ )
                {
                	sysVPMission.send.msg[i+1] = sysBCMission.msgBuf[i];    
                }
            }
            break;
		case VP_INFO_RPT://120419 by cq TotalSell 
			{
                sysVPMission.send.msgType = VP_INFO_RPT;  
				switch(sysVPMission.infoType)
				{
					case VP_INFO_TOTALVALUE:
						sysVPMission.send.datLen  = 3;
						sysVPMission.send.msg[0]  = VP_INFO_TOTALVALUE;
						//Total_Value：表示当前交易投币后，或者出货后，屏幕上显示的金额数 
						//by gzz 20110721
						if( sysITLMission.billHoldingFlag == 1 )
						{
							CurrentMoney = CurrentPayed + sysITLMission.billHoldingValue;
						}
						else
						{
							CurrentMoney = CurrentPayed;
						}
						tempMoney = MoneySend(CurrentMoney);
						//tempMoney = CurrentMoney;
		                sysVPMission.send.msg[1] = tempMoney/256;
		                sysVPMission.send.msg[2] = tempMoney%256;
						break;
					case VP_INFO_CHUHUO:
						sysVPMission.send.datLen  = 53;
						sysVPMission.send.msg[0]  = VP_INFO_CHUHUO;
						//全部出货数量
						sysVPMission.send.msg[1]  = H0UINT32(TradeCounter.vpSuccessNum);       
		                sysVPMission.send.msg[2]  = H1UINT32(TradeCounter.vpSuccessNum);        
		                sysVPMission.send.msg[3]  = L0UINT32(TradeCounter.vpSuccessNum);        
		                sysVPMission.send.msg[4]  = L1UINT32(TradeCounter.vpSuccessNum);   
						//全部出货金额						
						//tradeMoney = MoneySendInfo(TradeCounter.vpSuccessMoney); 
						//tradeMoney = TradeCounter.vpSuccessMoney; 
						sysVPMission.send.msg[5]  = H0UINT32(TradeCounter.vpSuccessMoney);  	 
						sysVPMission.send.msg[6]  = H1UINT32(TradeCounter.vpSuccessMoney);  	 
						sysVPMission.send.msg[7]  = L0UINT32(TradeCounter.vpSuccessMoney);  	 
						sysVPMission.send.msg[8]  = L1UINT32(TradeCounter.vpSuccessMoney);  
						
						//tradeMoney = MoneySendInfo(TradeCounter.vpSuccessMoney); 
						//sysVPMission.send.msg[5]  = tradeMoney/256/256/256;		 
						//sysVPMission.send.msg[6]  = tradeMoney/256/256%256;		 
						//sysVPMission.send.msg[7]  = tradeMoney/256%256;		 
						//sysVPMission.send.msg[8]  = tradeMoney%256;
						//len = sprintf( str, "%ld,%ld,%u,%u", TradeCounter.vpSuccessMoney, tradeMoney,sysVPMission.send.msg[7],sysVPMission.send.msg[8] );
						//DisplayStr( 0, 0, 1, str, len );
						//len = sprintf( str, "%u,%u", tradeMoney/256%256, tradeMoney%256 );
						//DisplayStr( 0, 1, 1, str, len );
						//WaitForWork( 5000, NULL );

						//现金出货数量
						sysVPMission.send.msg[9]  = H0UINT32(TradeCounter.vpCashNum); 	 
						sysVPMission.send.msg[10]  = H1UINT32(TradeCounter.vpCashNum); 		 
						sysVPMission.send.msg[11]  = L0UINT32(TradeCounter.vpCashNum); 			 
						sysVPMission.send.msg[12]  = L1UINT32(TradeCounter.vpCashNum); 	
						//现金出货金额	
						//tradeMoney = MoneySendInfo(TradeCounter.vpCashMoney); 
						//tradeMoney = TradeCounter.vpCashMoney; 
						sysVPMission.send.msg[13]  = H0UINT32(TradeCounter.vpCashMoney); 		 
						sysVPMission.send.msg[14]  = H1UINT32(TradeCounter.vpCashMoney); 	 
						sysVPMission.send.msg[15]  = L0UINT32(TradeCounter.vpCashMoney); 	 
						sysVPMission.send.msg[16]  = L1UINT32(TradeCounter.vpCashMoney); 
						//游戏出货数量 		
						sysVPMission.send.msg[17]  = H0UINT32(TradeCounter.vpGameNum); 
						sysVPMission.send.msg[18]  = H1UINT32(TradeCounter.vpGameNum); 	 
						sysVPMission.send.msg[19]  = L0UINT32(TradeCounter.vpGameNum); 		 
						sysVPMission.send.msg[20]  = L1UINT32(TradeCounter.vpGameNum); 
						//游戏出货金额
						tradeMoney = 0; 
						sysVPMission.send.msg[21]  = tradeMoney; 	 
						sysVPMission.send.msg[22]  = tradeMoney; 	 
						sysVPMission.send.msg[23]  = tradeMoney; 	 
						sysVPMission.send.msg[24]  = tradeMoney; 
						//刷卡出货数量
						sysVPMission.send.msg[25]  = H0UINT32(TradeCounter.vpCardNum); 		 
						sysVPMission.send.msg[26]  = H1UINT32(TradeCounter.vpCardNum); 			 
						sysVPMission.send.msg[27]  = L0UINT32(TradeCounter.vpCardNum); 			 
						sysVPMission.send.msg[28]  = L1UINT32(TradeCounter.vpCardNum); 	 
						//刷卡出货金额
						tradeMoney =0; 
						sysVPMission.send.msg[29]  = tradeMoney; 	 
						sysVPMission.send.msg[30]  = tradeMoney; 	 
						sysVPMission.send.msg[31]  = tradeMoney; 	 
						sysVPMission.send.msg[32]  = tradeMoney;
						//在线出货数量
						sysVPMission.send.msg[33]  = H0UINT32(TradeCounter.vpOnlineNum);	 
						sysVPMission.send.msg[34]  = H1UINT32(TradeCounter.vpOnlineNum);	 
						sysVPMission.send.msg[35]  = L0UINT32(TradeCounter.vpOnlineNum);	 		 
						sysVPMission.send.msg[36]  = L1UINT32(TradeCounter.vpOnlineNum);	 
						//在线出货金额
						tradeMoney = 0; 
						sysVPMission.send.msg[37]  = tradeMoney; 	 
						sysVPMission.send.msg[38]  = tradeMoney; 	 
						sysVPMission.send.msg[39]  = tradeMoney; 	 
						sysVPMission.send.msg[40]  = tradeMoney;
						//硬币投入
						//tradeMoney = MoneySendInfo(TradeCounter.CoinSum1y+TradeCounter.CoinSum5j); 
						tradeMoney = (TradeCounter.CoinSum1y+TradeCounter.CoinSum5j); 
						sysVPMission.send.msg[41]  = H0UINT32(tradeMoney);	 	 
						sysVPMission.send.msg[42]  = H1UINT32(tradeMoney);	 
						sysVPMission.send.msg[43]  = L0UINT32(tradeMoney);		 
						sysVPMission.send.msg[44]  = L1UINT32(tradeMoney);	
						//纸币投入
						//tradeMoney = MoneySendInfo(TradeCounter.CashSum); 
						//tradeMoney = TradeCounter.CashSum; 
						sysVPMission.send.msg[45]  = H0UINT32(TradeCounter.CashSum);	  
						sysVPMission.send.msg[46]  = H1UINT32(TradeCounter.CashSum);	 
						sysVPMission.send.msg[47]  = L0UINT32(TradeCounter.CashSum);	 
						sysVPMission.send.msg[48]  = L1UINT32(TradeCounter.CashSum);
						//硬币出币
						//tradeMoney = MoneySendInfo(TradeCounter.Hopper2Sum+TradeCounter.Hopper1Sum); 
						tradeMoney = TradeCounter.Hopper2Sum+TradeCounter.Hopper1Sum; 
						sysVPMission.send.msg[49]  = H0UINT32(tradeMoney);	 
						sysVPMission.send.msg[50]  = H1UINT32(tradeMoney);	 
						sysVPMission.send.msg[51]  = L0UINT32(tradeMoney); 	 
						sysVPMission.send.msg[52]  = L1UINT32(tradeMoney);
						break;
					case VP_INFO_HUODAO:
						sysVPMission.send.datLen  = COLUMN_NUM_SET + 2;
						sysVPMission.send.msg[0]  = VP_INFO_HUODAO;
						sysVPMission.send.msg[1]  = 0; 
						for( i=0; i<COLUMN_NUM_SET; i++ )
						{
							sysVPMission.send.msg[i+2]  = GoodsWaySetVal[i].GoodsType;
						}
						break;
					case VP_INFO_POSITION:
						sysVPMission.send.datLen  = GOODSTYPEMAX + 2;
						sysVPMission.send.msg[0]  = VP_INFO_POSITION;
						sysVPMission.send.msg[1]  = 0; 
						for( i=0; i<GOODSTYPEMAX; i++)
						{
							//sysVPMission.send.msg[i+2] = sysVPMission.selItem[i];
							sysVPMission.send.msg[i+2] = SystemParameter.selItem[i];
						}
						break;
					case VP_INFO_SALEPRICE:
						sysVPMission.send.datLen  = sysVPMission.GoodsRev*2+2;
						sysVPMission.send.msg[0]  = VP_INFO_SALEPRICE;
						sysVPMission.send.msg[1]  = 0; 						
						
						/*for( i=0,j=2; i<4;  i++,j+=2 )
						{
							//tempMoney = sysGoodsMatrix[i].Price;
							//tempMoney = GoodsWaySetVal[i].Price;
							//tempMoney = MoneySend(sysVPMission.selPrice[i]);
							tempMoney = MoneySend(sysGoodsMatrix[i].Price);
							sysVPMission.send.msg[j]  = HUINT16(tempMoney); 	 
							sysVPMission.send.msg[j+1]  = LUINT16(tempMoney);
						}	*/
						for( salei=0,salej=2; salei<sysVPMission.GoodsRev;  salei++,salej+=2 )
						{
							saleMoney = (sysVPMission.selPrice[salei]%10==0)?sysVPMission.selPrice[salei]:Price[salei];
							tempMoney = MoneySend(saleMoney);
							sysVPMission.send.msg[salej]  = HUINT16(tempMoney); 	 
							sysVPMission.send.msg[salej+1]  = LUINT16(tempMoney);
							//len = sprintf( str, "%02d=%02d,%02d", tempMoney,sysGoodsMatrix[salei].Price,sysVPMission.selPrice[salei]);						
							//DisplayStr( 0, 0, 1, str, len );
							//WaitForWork( 5000, NULL );							
						}						
						break;
					case VP_INFO_VMC:
						sysVPMission.send.datLen  = 4;
						sysVPMission.send.msg[0]  = VP_INFO_VMC;
						sysVPMission.send.msg[1]  = 0; 
						sysVPMission.send.msg[2]  = 0; 
						sysVPMission.send.msg[3]  = COLUMN_NUM_SET + 1;
						break;
					case VP_INFO_VER:
						sysVPMission.send.datLen  = 31;
						sysVPMission.send.msg[0]  = VP_INFO_VER;
						// unpeng-zonghe-1.0(2012-11-07)
						sysVPMission.send.msg[1]  = 'J'; 	 
						sysVPMission.send.msg[2]  = 'u'; 	 
						sysVPMission.send.msg[3]  = 'n'; 	 
						sysVPMission.send.msg[4]  = 'p';
						sysVPMission.send.msg[5]  = 'e'; 	 
						sysVPMission.send.msg[6]  = 'n'; 	 
						sysVPMission.send.msg[7]  = 'g'; 	 
						sysVPMission.send.msg[8]  = '-';
						sysVPMission.send.msg[9]  = 'z'; 	 
						sysVPMission.send.msg[10]  = 'o'; 	 
						sysVPMission.send.msg[11]  = 'n';
						sysVPMission.send.msg[12]  = 'g'; 	 
						sysVPMission.send.msg[13]  = 'h'; 	 
						sysVPMission.send.msg[14]  = 'e'; 	 
						sysVPMission.send.msg[15]  = '-';
						sysVPMission.send.msg[16]  = '1'; 	 
						sysVPMission.send.msg[17]  = '.'; 	 
						sysVPMission.send.msg[18]  = '0';
						sysVPMission.send.msg[19]  = '('; 	 
						sysVPMission.send.msg[20]  = '2'; 	 
						sysVPMission.send.msg[21]  = '0'; 	 
						sysVPMission.send.msg[22]  = '1';
						sysVPMission.send.msg[23]  = '4'; 	 
						sysVPMission.send.msg[24]  = '-'; 	 
						sysVPMission.send.msg[25]  = '0';
						sysVPMission.send.msg[26]  = '9'; 	 
						sysVPMission.send.msg[27]  = '-'; 	 
						sysVPMission.send.msg[28]  = '2'; 	 
						sysVPMission.send.msg[29]  = '2';
						sysVPMission.send.msg[30]  = ')';
						break;
					case VP_INFO_HARD:
						sysVPMission.send.datLen  = 8;
						sysVPMission.send.msg[0]  = VP_INFO_HARD;
						sysVPMission.send.msg[1]  = 'p'; 	 
						sysVPMission.send.msg[2]  = '8'; 	 
						sysVPMission.send.msg[3]  = '9'; 	 
						sysVPMission.send.msg[4]  = 'c';
						sysVPMission.send.msg[5]  = '6'; 	 
						sysVPMission.send.msg[6]  = '6'; 	 
						sysVPMission.send.msg[7]  = '9'; 	 
						break;
					case VP_INFO_BILL:
						sysVPMission.send.datLen  = 19+33;
						sysVPMission.send.msg[0]  = VP_INFO_BILL;
						sysVPMission.send.msg[1]  = sysMDBMission.Pc_billBuf[0];
						sysVPMission.send.msg[2]  = sysMDBMission.Pc_billBuf[1];	 
						sysVPMission.send.msg[3]  = sysMDBMission.Pc_billBuf[2]; 	 
						sysVPMission.send.msg[4]  = sysMDBMission.Pc_billBuf[3]; 	
						sysVPMission.send.msg[5]  = sysMDBMission.Pc_billBuf[4]; 		 
						sysVPMission.send.msg[6]  = sysMDBMission.Pc_billBuf[5]; 	 
						sysVPMission.send.msg[7]  = sysMDBMission.Pc_billBuf[6];  
						sysVPMission.send.msg[8]  = sysMDBMission.Pc_billBuf[7]; 
						sysVPMission.send.msg[9]  = sysMDBMission.Pc_billBuf[8];  	 
						sysVPMission.send.msg[10]  = sysMDBMission.Pc_billBuf[10]; 	 
						sysVPMission.send.msg[11]  = sysMDBMission.Pc_billBuf[11];
						sysVPMission.send.msg[12]  = sysMDBMission.Pc_billBuf[12]; 	 
						sysVPMission.send.msg[13]  = sysMDBMission.Pc_billBuf[13]; 	 
						sysVPMission.send.msg[14]  = sysMDBMission.Pc_billBuf[14]; 	 
						sysVPMission.send.msg[15]  = sysMDBMission.Pc_billBuf[15];
						sysVPMission.send.msg[16]  = sysMDBMission.Pc_billBuf[16]; 	
						sysVPMission.send.msg[17]  = sysMDBMission.Pc_billBuf[17];
						sysVPMission.send.msg[18]  = sysMDBMission.Pc_billBuf[18];
						for(i=0;i<33;i++)
						{
							sysVPMission.send.msg[19+i]  = sysMDBMission.billIDENTITYBuf[i];
						}
						break;
					case VP_INFO_COIN:
						sysVPMission.send.datLen  = 18+33;
						sysVPMission.send.msg[0]  = VP_INFO_COIN;
						sysVPMission.send.msg[1]  = sysMDBMission.Pc_coinBuf[0];
						sysVPMission.send.msg[2]  = sysMDBMission.Pc_coinBuf[1];	 
						sysVPMission.send.msg[3]  = sysMDBMission.Pc_coinBuf[2]; 	 
						sysVPMission.send.msg[4]  = sysMDBMission.Pc_coinBuf[3]; 	
						sysVPMission.send.msg[5]  = sysMDBMission.Pc_coinBuf[4]; 		 
						sysVPMission.send.msg[6]  = sysMDBMission.Pc_coinBuf[7]; 	 
						sysVPMission.send.msg[7]  = sysMDBMission.Pc_coinBuf[8];  
						sysVPMission.send.msg[8]  = sysMDBMission.Pc_coinBuf[9]; 
						sysVPMission.send.msg[9]  = sysMDBMission.Pc_coinBuf[10];  	 
						sysVPMission.send.msg[10]  = sysMDBMission.Pc_coinBuf[11]; 	 
						sysVPMission.send.msg[11]  = sysMDBMission.Pc_coinBuf[12];
						sysVPMission.send.msg[12]  = sysMDBMission.Pc_coinBuf[7]; 	 
						sysVPMission.send.msg[13]  = sysMDBMission.Pc_coinBuf[8]; 	 
						sysVPMission.send.msg[14]  = sysMDBMission.Pc_coinBuf[9]; 	 
						sysVPMission.send.msg[15]  = sysMDBMission.Pc_coinBuf[10];
						sysVPMission.send.msg[16]  = sysMDBMission.Pc_coinBuf[11]; 	
						sysVPMission.send.msg[17]  = sysMDBMission.Pc_coinBuf[12];
						for(i=0;i<33;i++)
						{
							sysVPMission.send.msg[18+i]  = sysMDBMission.coinIDENTITYBuf[i];
						}
						break;
					case VP_INFO_COMP:
						sysVPMission.send.datLen  = 1;
						sysVPMission.send.msg[0]  = VP_INFO_COMP;
						break;
					case VP_INFO_ERR:
						{	
							//sysVPMission.infoErr = HardWareErr;
						    i = 0;				
							//sysVPMission.send.datLen = 2;   
			                //sysVPMission.send.msg[0] = sysVPMission.infoType;
							//sysVPMission.send.msg[1] = sysVPMission.infoErr;
							sysVPMission.send.msg[i++] = VP_INFO_ERR;							
							//纸币器故障
							/*
							if(sysVPMission.STA_BillA==2)
							{
							    //errcode = 0xFB;
								//sprintf( errCodeStr, "%s", "纸币器故障" );			
								//sprintf( errCodeStr, "%s", "  " );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "纸币器故障" );
								sysVPMission.send.msg[i++] = 0x00;
								sysVPMission.send.msg[i++] = 0xFB;
							}
							*/
							//纸币器故障
							//具体故障情况
							//钞箱已满							
							if(sysMDBMission.billIsFull == 1)
							{
								sysVPMission.send.msg[i++] = 0x17;
								sysVPMission.send.msg[i++] = 0x78;
							}
							//钞箱移除报故障
							if(sysMDBMission.billCashBuf == 1)//钞箱移除报故障;by gzz 20121224
						    {
								if( sysITLMission.billHoldingFlag == 1 )
								{
									CurrentMoney = CurrentPayed + sysITLMission.billHoldingValue;
								}
								else
								{
									CurrentMoney = CurrentPayed;
								}
								if( (CurrentMoney == 0)&&( sysITLMission.billHoldingFlag == 0 ) )
								{
									sysVPMission.send.msg[i++] = 0x17;
									sysVPMission.send.msg[i++] = 0x77;
								}
							}
							//电机异常
							if(sysMDBMission.billOthErr & 0x01)
							{
								sysVPMission.send.msg[i++] = 0x17;
								sysVPMission.send.msg[i++] = 0x73;
							}
							//传感器异常
							if(sysMDBMission.billOthErr & 0x02)
							{
								sysVPMission.send.msg[i++] = 0x17;
								sysVPMission.send.msg[i++] = 0x74;
							}
							//卡币
							if(sysMDBMission.billOthErr & 0x08)
							{
								sysVPMission.send.msg[i++] = 0x17;
								sysVPMission.send.msg[i++] = 0x76;
							}
							
							//sysVPMission.send.msg[i++] = sysMDBMission.billOthErr;

							
							//硬币器故障
							/*
							if( ( HardWareErr & 0x0002 )||( sysVPMission.STA_CoinA == 2 ) )
							{
								//errcode = 0xF2;
								//sprintf( errCodeStr, "%s", "硬币器故障" );			
								//sprintf( errCodeStr, "%s", "  请用其它方式购物" );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "硬币器故障");
								sysVPMission.send.msg[i++] = 0x00;
								sysVPMission.send.msg[i++] = 0xF2;
								//sysVPMission.send.msg[i++] = 0x00;
								//sysVPMission.send.msg[i++] = 0xF3;
							}
							*/
							//硬币器故障
							//具体故障情况
							//添加币管没有锁紧故障;by gzz 20110827
							if( sysMDBMission.tubeRemoved == 1 )
							{
							    //errcode = 0xF0;//币管没有锁紧故障
								//sprintf( errCodeStr, "%s", "币管未关闭" );			
								//sprintf( errCodeStr, "%s", "  请用其它方式购物" );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "币管未关闭" );
								sysVPMission.send.msg[i++] = 0x13;
								sysVPMission.send.msg[i++] = 0x8c;
							}
							//传感器异常
							if(sysMDBMission.coinOthErr & 0x01)
							{
								sysVPMission.send.msg[i++] = 0x13;
								sysVPMission.send.msg[i++] = 0x8b;
							}
							//出币卡币
							if(sysMDBMission.coinOthErr & 0x02)
							{
								sysVPMission.send.msg[i++] = 0x13;
								sysVPMission.send.msg[i++] = 0x8d;
							}
							//进币通道出错
							if(sysMDBMission.coinOthErr & 0x08)
							{
								sysVPMission.send.msg[i++] = 0x13;
								sysVPMission.send.msg[i++] = 0x8f;
							}
							//投币卡币
							if(sysMDBMission.coinOthErr & 0x10)
							{
								sysVPMission.send.msg[i++] = 0x13;
								sysVPMission.send.msg[i++] = 0x90;
							}
							//找币器故障
							//if( ( HardWareErr & 0x0004 )||( sysVPMission.STA_CoinA == 2 ) )
							//{
								//errcode = 0xF3;
								//sprintf( errCodeStr, "%s", "找币器故障" );			
								//sprintf( errCodeStr, "%s", "  请用其它方式购物" );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "找币器故障");
								//sysVPMission.send.msg[i++] = 0x00;
								//sysVPMission.send.msg[i++] = 0xF3;
							//}
							//找币器缺币
							//if( HardWareErr & 0x0008 )
							//{
								//errcode = 0xF4;
					            //sprintf( errCodeStr, "%s", "找币器缺币" );			
								//sprintf( errCodeStr, "%s", "  请用其它方式购物" );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "找币器缺币");
								//sysVPMission.send.msg[i++] = 0x00;
								//sysVPMission.send.msg[i++] = 0xF4;
							//}
							//if( HardWareErr & 0x0010 )
							//{
								//errcode = 0xF5;			
								//sprintf( errCodeStr, "%s", "  " );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "所有货道无货");
								//sysVPMission.send.msg[i++] = 0x00;
								//sysVPMission.send.msg[i++] = 0xF5;
							//}
							//if( HardWareErr & 0x0020 )
							//{
								//errcode = 0xF6;
								//sprintf( errCodeStr, "%s", "所有货道故障" );			
								//sprintf( errCodeStr, "%s", "  " );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "所有货道故障");
								//sysVPMission.send.msg[i++] = 0x00;
								//sysVPMission.send.msg[i++] = 0xF6;
							//}
							if( HardWareErr & 0x0040 )
							{
								//errcode = 0xF7;	
								//sprintf( errCodeStr, "%s", "货道板故障" );			
								//sprintf( errCodeStr, "%s", "  " );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "货道板故障");
								sysVPMission.send.msg[i++] = 0x00;
								sysVPMission.send.msg[i++] = 0xF7;
							}
							if( HardWareErr & 0x0080 )
							{
								//errcode = 0xF8;				
								//sprintf( errCodeStr, "%s", "  " );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "所有货道不能工作");
								sysVPMission.send.msg[i++] = 0x00;
								sysVPMission.send.msg[i++] = 0xF8;
							}
							if( HardWareErr & 0x0100 )
							{
								//errcode = 0xF9;	
								//sprintf( errCodeStr, "%s", "系统参数故障" );			
								//sprintf( errCodeStr, "%s", "  " );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "系统参数故障");
								sysVPMission.send.msg[i++] = 0x00;
								sysVPMission.send.msg[i++] = 0xF9;
							}
							if( HardWareErr & 0x0200 )
							{
								//errcode = 0xFA;
								//sprintf( errCodeStr, "%s", "LCD 故障" );			
								//sprintf( errCodeStr, "%s", "  " );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "LCD 故障" );
								sysVPMission.send.msg[i++] = 0x00;
								sysVPMission.send.msg[i++] = 0xFA;
							}	

							/*
							//出货检测故障		
							if( ( HardWareErr & 0x0800 )||(sysVPMission.STA_ICCard==2) )
							{
							    //errcode = 0xFC;
								//sprintf( errCodeStr, "%s", "出货确认故障" );			
								//sprintf( errCodeStr, "%s", "  " );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "出货确认故障" );
								sysVPMission.send.msg[i++] = 0x00;
								sysVPMission.send.msg[i++] = 0xFC;
							}
							*/
							if( HardWareErr & 0x1000 )
							{
							    //errcode = 0xFD;
								//sprintf( errCodeStr, "%s", "选货模块故障" );			
								//sprintf( errCodeStr, "%s", "  " );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "选货模块故障" );
								sysVPMission.send.msg[i++] = 0x00;
								sysVPMission.send.msg[i++] = 0xFD;
							}
							if( HardWareErr & 0x2000 )
							{
							    //errcode = 0xFE;
								//sprintf( errCodeStr, "%s", "PC通信故障" );			
								//sprintf( errCodeStr, "%s", "  " );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "PC通信故障" );
								sysVPMission.send.msg[i++] = 0x1b;
								sysVPMission.send.msg[i++] = 0x5a;
							}
							//if( HardWareErr & 0x4000 )	
							//{
							    //errcode = 0xE1;
								//sprintf( errCodeStr, "%s", "pc离线" );			
								//sprintf( errCodeStr, "%s", "  " );

								//sprintf( sysVPMission.hardWareErrStr, "%s", "pc离线" );
								//sysVPMission.send.msg[i++] = 0x00;
								//sysVPMission.send.msg[i++] = 0xE1;
							//}					        
							sysVPMission.send.datLen = i;   
						}
						break;
				}
				/*
                sysVPMission.send.datLen  = 9;
                sysVPMission.send.msg[0]  = 6; 
				
                sysVPMission.send.msg[1]  = TradeCounter.TotalSellNum/256/256/256;       
                sysVPMission.send.msg[2]  = TradeCounter.TotalSellNum/256/256%256;       
                sysVPMission.send.msg[3]  = TradeCounter.TotalSellNum/256%256;       
                sysVPMission.send.msg[4]  = TradeCounter.TotalSellNum%256; 

				sysVPMission.send.msg[5]  = TradeCounter.TotalSellMoeny/256/256/256;		 
				sysVPMission.send.msg[6]  = TradeCounter.TotalSellMoeny/256/256%256;		 
				sysVPMission.send.msg[7]  = TradeCounter.TotalSellMoeny/256%256;		 
				sysVPMission.send.msg[8]  = TradeCounter.TotalSellMoeny%256;	
				*/

			}
			break;
			case VP_OFFLINEDATA_RPT:
            {
                sysVPMission.send.msgType = VP_OFFLINEDATA_RPT;  
                sysVPMission.send.datLen  = 5;
                sysVPMission.send.msg[0]  = 1;    
				tradeMoney = MoneySendInfo(TradeCounter.offLineMoney);
                //sysVPMission.send.msg[1]  = tempMoney/256;
				//sysVPMission.send.msg[2]  = tempMoney%256;
				sysVPMission.send.msg[1]  = tradeMoney/256%256;		 
				sysVPMission.send.msg[2]  = tradeMoney%256;
				sysVPMission.send.msg[3]  = TradeCounter.offLineNum/256;
				sysVPMission.send.msg[4]  = TradeCounter.offLineNum%256;
				TradeCounter.offLineMoney = 0;
				TradeCounter.offLineNum = 0;
            }
            break;
		default: break;
	}
	//
	sysVPMission.send.sf  = VP_SF;
	//
    sysVPMission.send.len = sysVPMission.send.datLen + 5;
	//
    sysVPMission.send.verFlag = VP_PROTOCOL_VER;
	if( flag == 1 )
	{
	    sysVPMission.send.verFlag += VP_PROTOCOL_ACK;
	}
	else
	{
		sysVPMission.send.verFlag += VP_PROTOCOL_NAK;
	}
	//
	if( snCtr == 1 )
	{
	    if( sysVPMission.sysStatus != 0 )
		{
			sysVPMission.send.sn += 1;
    	}
		else
		{
			sysVPMission.sysStatus |= VPM_STA_FIRSTCOM;
		}
	}
    sysVPMission.comLock = 1;
    VPBusTxMsg();
    sysVPMission.comLock = 0;
	return VP_ERR_NULL;
}

/*//----------------------------------------------------------------------
//PC--VMC command mission
unsigned char VP_CMD_GoodsPar( void )
{
	
	unsigned char i = 0;
	unsigned char j = 0;
	u_int  tempMoney = 0;
	
	
	//if( sysVPMission.receive.datLen == (GOODSTYPEMAX+GOODSTYPEMAX*2)	)
	if( sysVPMission.receive.datLen <= 124*2+1)
	{
		
		if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK )
		{
			VPMsgPackSend( VP_ACK_RPT, 0, 0 );
		}

		//----------------------------------
		//	1...			  n
		//SP1_Price...Sn_Price
		sysVPMission.GoodsRev = (sysVPMission.receive.datLen - 1) / 2;	
		for( i=0,  j=1; i<sysVPMission.GoodsRev; i++, j+=2 )
		{
			sysGoodsMatrix[i].GoodsType = i+1;			
			if((sysVPMission.receive.msg[j]*256+sysVPMission.receive.msg[j+1])>=6550)
			{
				sysGoodsMatrix[i].Price  = 65000;
			}
			else
			{	
				if(sysGoodsMatrix[i].Price  != MoneyRec(sysVPMission.receive.msg[j],sysVPMission.receive.msg[j+1]))
					sysGoodsMatrix[i].Price  = MoneyRec(sysVPMission.receive.msg[j],sysVPMission.receive.msg[j+1]);				
			}
			//sysVPMission.selItem[i] 	= sysGoodsMatrix[i].GoodsType;
			sysVPMission.selPrice[i] = sysGoodsMatrix[i].Price;  
			if(sysVPMission.selPrice[i] >= tempMoney)
			{
				tempMoney = sysVPMission.selPrice[i];
				SystemParameter.BanknoteMax = sysVPMission.selPrice[i];
			}
		}	
		
		sysVPMission.sysStatus |= VPM_STA_GOODSPAR; 	
	}
	else
	{
		VPMsgPackSend( VP_NAK_RPT, 0, 0  );  //1, 0
		return VP_ERR_PAR;
	}
	
	return VP_ERR_NULL; 
}
*/

/*
{
    unsigned char i = 0;
    unsigned char j = 0;
    if( sysVPMission.receive.datLen == (GOODSTYPEMAX+GOODSTYPEMAX*2)  )
	{
        
	    if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK )
        {
	    	VPMsgPackSend( VP_ACK_RPT, 0, 0 );
        }
		//----------------------------------
		//  1....17,   19...               54
		//SP1....SP18, SP1_Price...SP18_Price
        for( i=0; i<GOODSTYPEMAX; i++)
		{
			sysGoodsMatrix[i].GoodsType = sysVPMission.receive.msg[i];
            sysVPMission.selItem[i]     = sysVPMission.receive.msg[i];
		}
		for( i=0, j=GOODSTYPEMAX; i<GOODSTYPEMAX; i++,j+=2 )
		{
			sysGoodsMatrix[i].Price  = sysVPMission.receive.msg[j]*256+sysVPMission.receive.msg[j+1]; 
            sysVPMission.selPrice[i] = sysVPMission.receive.msg[j]*256+sysVPMission.receive.msg[j+1];    
		}
		sysVPMission.sysStatus |= VPM_STA_GOODSPAR;
	}
	else
	{
	    VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 //1, 0
		return VP_ERR_PAR;
	}
    return VP_ERR_NULL;	
}*/

unsigned char VP_CMD_GoodsPos( void )
{
    unsigned char i = 0;
    unsigned char j = 0;
	/**/
    //if( sysVPMission.receive.datLen == (GOODSTYPEMAX+GOODSTYPEMAX*2)  )
    if( sysVPMission.receive.datLen == GOODSTYPEMAX+1)
	{
        
	    if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK )
        {
	    	VPMsgPackSend( VP_ACK_RPT, 0, 0 );
        }

		for( i=0; i<GOODSTYPEMAX; i++)
		{
			//sysVPMission.selItem[i] = sysVPMission.receive.msg[i+1];
			if(SystemParameter.selItem[i] != sysVPMission.receive.msg[i+1])
				SystemParameter.selItem[i] = sysVPMission.receive.msg[i+1];
		}
		SaveGlobalParam();
		WaitForWork( 100, NULL );
	}
	else
	{
	    VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 //1, 0
		return VP_ERR_PAR;
	}
    return VP_ERR_NULL;	
}


unsigned char VP_CMD_ColumnPar( void )
{
    unsigned char i = 0;
	unsigned char j = 0;
	if( sysVPMission.receive.datLen == COLUMN_NUM_SET+1 )
	{
        if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK )
        {
	    	VPMsgPackSend( VP_ACK_RPT, 0, 0 );
        }
		//-------------------------------------------------
		//1...       48  49...        96
		//colNum_1...48  colGoods_1...48
		for( i=0; i<COLUMN_NUM_SET; i++ )
		{
			if( (sysVPMission.receive.msg[i+1]==0xff)||(sysVPMission.receive.msg[i+1]==0x0) )
            {
                GoodsWaySetVal[i].GoodsCurrentSum = 0;
                GoodsWaySetVal[i].GoodsType = sysVPMission.receive.msg[i+1];
				GoodsWaySetVal[i].Price = 0;
            }
            else
            {
				//GoodsWaySetVal[i].GoodsCurrentSum = sysVPMission.receive.msg[i];
				if(GoodsWaySetVal[i].GoodsType != sysVPMission.receive.msg[i+1])
					GoodsWaySetVal[i].GoodsType = sysVPMission.receive.msg[i+1];
            }
			GoodsWaySetVal[i].IsUsed = 0;
		}
		sysVPMission.sysStatus |= VPM_STA_COLUMNPAR;
		
		
	}
	else
	{
		VPMsgPackSend( VP_NAK_RPT, 0, 0  );  //1,0
		return VP_ERR_PAR;
	}
    return VP_ERR_NULL;	
}	




unsigned char VP_CMD_ColumnNo( void )
{
    unsigned char i = 0;
	unsigned char j = 0;
	if( sysVPMission.receive.datLen == COLUMN_NUM_SET+1 )
	{
        if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK )
        {
	    	VPMsgPackSend( VP_ACK_RPT, 0, 0 );
        }
		//-------------------------------------------------
		//1...       48  49...        96
		//colNum_1...48  colGoods_1...48		
		for( i=0; i<COLUMN_NUM_SET; i++)
		{
			if( (GoodsWaySetVal[i].GoodsType!=0xff)&&(GoodsWaySetVal[i].GoodsType!=0x0) )
            {
				GoodsWaySetVal[i].GoodsCurrentSum = (sysVPMission.receive.msg[i+1]>63)?63:(sysVPMission.receive.msg[i+1]&0x3f);
				//GoodsWaySetVal[i].GoodsType = sysVPMission.receive.msg[i+1];
			}
		}		
		sysVPMission.sysStatus |= VPM_STA_COLUMNPAR;
		for( i=0; i<COLUMN_NUM_SET; i++ )
		{
	       if( GoodsWaySetVal[i].GoodsCurrentSum > 0 )
			{
			    GoodsWaySetVal[i].WayState = 0x01;
			}						
		}		
		SaveGoodsSet();	
		WaitForWork( 100, NULL );
	}
	else
	{
		VPMsgPackSend( VP_NAK_RPT, 0, 0  );  //1,0
		return VP_ERR_PAR;
	}
    return VP_ERR_NULL;	
}


unsigned char VP_CMD_Init_OK( void )
{
    unsigned char i = 0;
	unsigned char j = 0;
	if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK )
	{
    	VPMsgPackSend( VP_ACK_RPT, 0, 0 );
    }
    sysVPMission.sysStatus |= VPM_STA_INITOK;
	return VP_ERR_NULL;	
}

unsigned char VP_CMD_Vendout( void )
{
    unsigned char i = 0;
    unsigned char j = 0;
	unsigned int  sel = 0xffff;
	unsigned char col = 0xff;
    unsigned int  moneyBuf = 0;
	unsigned char dspUpdate = 0;
	unsigned char method = 0;
	uchar xdata chuSet = 0;
	//u_char xdata str[20];
	//u_char xdata len = 0;
    
	//1.Check the data
    if( sysVPMission.receive.datLen != 6  )
	{
		//DisplayStr( 0, 0, 1, "1", 1 ); 
		//WaitForWork(5000,NULL);
	    VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}
	
	if(sysVPMission.ErrFlag2 == 1)
	{
		//DisplayStr( 0, 0, 1, "2", 1 ); 
		//WaitForWork(5000,NULL);
	    VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}	

    //2.Check the goods code in goods matrix //or in column matrix
	//
	if(DeviceStatus.Driver64 != 0x00)
	{
		//DisplayStr( 0, 0, 1, "3", 1 ); 
		//WaitForWork(5000,NULL);
		VPMsgPackSend( VP_NAK_RPT, 0, 0  );	
	    return VP_ERR_PAR;
	}
	method = sysVPMission.receive.msg[1];

	if(method == 1)
	{
		/*
		for( i=0; i<sysVPMission.GoodsRev; i++ )
		{
			if( sysVPMission.receive.msg[2] == sysGoodsMatrix[i].GoodsType )
			{
		    	sel = i;
			    break;
			}
		}

	    if( sel == 0xffff )
		{
	    	//VPMsgPackSend( VP_NAK_RPT, 0, 0  );	
			//return VP_ERR_PAR;
			//The goods code isn't in the goods window, find in the column matrix
			for( i=0; i<COLUMN_NUM_SET; i++  )
			{
				if( GoodsWaySetVal[i].GoodsType == sysVPMission.receive.msg[2] )
	     	    {
					if( (GoodsWaySetVal[i].GoodsCurrentSum >= 1) && ( GoodsWaySetVal[i].WayState == 0x01 ) )
					{
						col = i;
	                    break;
					}
				}
			}
			if( i >= COLUMN_NUM_SET )
			{
				DisplayStr( 0, 0, 1, "4", 1 ); 
				WaitForWork(5000,NULL);
				VPMsgPackSend( VP_NAK_RPT, 0, 0  );	
			    return VP_ERR_PAR;
			}
	        sysVPMission.goodsType2 = GOODSTYPEMAX;

		}
	    else
		{
			if( ( sysGoodsMatrix[sel].ColumnNum == 0 )||( sysGoodsMatrix[sel].NextColumn == GOODS_MATRIX_NONE ) )
			{
				DisplayStr( 0, 0, 1, "5", 1 ); 
				WaitForWork(5000,NULL);
				VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
				return VP_ERR_PAR;		
			}
		    else
			{
				col = sysGoodsMatrix[sel].NextColumn;
				sysVPMission.goodsType2 = sel;
			}    
		}
		*/
		//1.判断是否有货物出货
		for( i=0; i<COLUMN_NUM_SET; i++  )
		{
			if( GoodsWaySetVal[i].GoodsType == sysVPMission.receive.msg[2] )
     	    {
				if( (GoodsWaySetVal[i].GoodsCurrentSum > 0) && (GoodStateLed(i)==1) )
				{
					col = i;
                    break;
				}
			}
		}
		//没有货物出货
		if( i >= COLUMN_NUM_SET )
		{
			//DisplayStr( 0, 0, 1, "4", 1 ); 
			//WaitForWork(5000,NULL);
			VPMsgPackSend( VP_NAK_RPT, 0, 0  );	
		    return VP_ERR_PAR;
		}
		//2.判断是这次轮流到哪个货道出货
		while(chuSet==0)
		{		
			for( i=0; i<COLUMN_NUM_SET; i++ )
		    {
		    	if( GoodsWaySetVal[i].GoodsType == sysVPMission.receive.msg[2] )
		    	{
		    		//len = sprintf( str, "1=%02bu,%02bu,%02bu", i,GoodsWaySetVal[i].GoodsCurrentSum,GoodsWaySetVal[i].IsUsed );
					//		DisplayStr( 0, 1, 1, str, len );
					//		WaitForWork( 2000, NULL );
					if( ( GoodsWaySetVal[i].GoodsCurrentSum > 0 )&&(GoodStateLed(i)==1) &&( GoodsWaySetVal[i].IsUsed == 0 ) )
				    {		        
			        	col = i;
						chuSet = 1;
						GoodsWaySetVal[i].IsUsed = 1;
						//len = sprintf( str, "2=%02bu", i );
						//	DisplayStr( 0, 1, 1, str, len );
						//	WaitForWork( 2000, NULL );	        
						break;
				    }
		    	}
		    }
			//都已经出货过一次了，重头开始出货
		    if(i>=COLUMN_NUM_SET)
			{
				for( i=0; i<COLUMN_NUM_SET; i++ )
			    {
			    	if( GoodsWaySetVal[i].GoodsType == sysVPMission.receive.msg[2] )
			    	{
			    		//len = sprintf( str, "%02bu,%02bu", sysVPMission.key,SystemParameter.selItem[sysVPMission.key] );
						//		DisplayStr( 0, 1, 1, str, len );
						//		WaitForWork( 2000, NULL );
						//CurrentStockCode = i;		
					    if( ( GoodsWaySetVal[i].GoodsCurrentSum > 0 )&&(GoodStateLed(i)==1) )
					    {		        
				        	GoodsWaySetVal[i].IsUsed = 0;	 
					    }
			    	}
			    }
		    }
		}
        sysVPMission.goodsType2 = GOODSTYPEMAX;
	}
	else if(method == 2)
	{
		col = sysVPMission.receive.msg[2]-1;
		
		if( (!(GoodsWaySetVal[col].WayState & 0x01))||( GoodsWaySetVal[col].WayState & 0x0A )||( GoodsWaySetVal[col].GoodsCurrentSum == 0 ) )
		{
			//DisplayStr( 0, 0, 1, "6", 1 ); 
			//WaitForWork(5000,NULL);
			VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
			return VP_ERR_PAR;			
		}
	}

    //3.Check the column's status
    
	//4.ACK
    sysVPMission.vendGoods = sysVPMission.receive.msg[2];
    sysVPMission.vendType  = sysVPMission.receive.msg[3];
    sysVPMission.vendCost  = MoneyRec(sysVPMission.receive.msg[4], sysVPMission.receive.msg[5]);
    sysVPMission.vendColumn = col;
	if(( sysVPMission.vendType > 0 )&&(sysVPMission.vendCost > 0))
	{
		//DisplayStr( 0, 0, 1, "7", 1 ); 
		//WaitForWork(5000,NULL);
		VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;			
	}
	if(( sysVPMission.vendType == 0 )&&(sysVPMission.vendCost == 0))
	{
		//DisplayStr( 0, 0, 1, "8", 1 ); 
		//WaitForWork(5000,NULL);
		VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;			
	}
    
    //---------------------------------------------------
    //2011.7.19 changed for 
    moneyBuf = CurrentPayed;
    if( sysITLMission.billHoldingFlag == 1 )
    {
        moneyBuf += sysITLMission.billHoldingValue;
    }
    //if( sysVPMission.vendCost > CurrentPayed )
    if( sysVPMission.vendCost > moneyBuf )
	{
		//DisplayStr( 0, 0, 1, "3", 1 ); 
		//WaitForWork(5000,NULL);
		VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;			
	}
	//扣钱后不能找零
	if(!(IsCanChange(sysVPMission.vendCost)))
	{
		//DisplayStr( 0, 0, 1, "4", 1 ); 
		//WaitForWork(5000,NULL);
    	VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}	
    //
    if( ( sysITLMission.billHoldingFlag == 1 )&&(sysVPMission.vendCost > CurrentPayed) )
    {
        StackTheBillAPI();
        dspUpdate = 1;			
    }
    if( sysVPMission.vendCost > CurrentPayed )
    {
    	//DisplayStr( 0, 0, 1, "5", 1 ); 
		//WaitForWork(5000,NULL);
		VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;	    
	}
	
	//正在进行出货，找零，或扣款操作
	if(( sysVPMission.vendCmd == 1 )||(sysVPMission.changeCmd == 1)||(sysVPMission.costCmd == 1))
	{
    	//DisplayStr( 0, 0, 1, "5", 1 ); 
		//WaitForWork(5000,NULL);
		VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;	    
	}	
	if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK )
	{
    	VPMsgPackSend( VP_ACK_RPT, 0, 0 );
	}
	if( dspUpdate == 1 )
	{
		  //payin report
		  if( sysVPMission.payInCoinFlag == 1 )
		  {
		      VPMission_Payin_RPT( VP_DEV_COIN, sysVPMission.payInCoinMoney );
		      sysVPMission.payInCoinFlag = 0;
		      sysVPMission.payInCoinMoney = 0;
						        
		  }
		  if( sysVPMission.payInBillFlag == 1 )
		  {
		      VPMission_Payin_RPT( VP_DEV_BILL, sysVPMission.payInBillMoney );
		      sysVPMission.payInBillFlag = 0;
		      sysVPMission.payInBillMoney = 0;      
		  }
		  //
		  if( sysVPMission.escrowOutFlag == 1 )
		  {
		      VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
		      sysVPMission.escrowOutFlag = 0;
		      sysVPMission.escrowMoney = 0;     
		  }
		  DisplayCurrentMoney( CurrentPayed );
	}
    //5.Vendout
    //OutGoods(1);   
	sysVPMission.vendCmd = 1;
	//6.Vendout_RPT
    //VPMission_Vendout_RPT( sysVPMission.vendSta, sysVPMission.vendColumn, sysVPMission.vendType, sysVPMission.vendCost );
    //WaitForWork( 300, NULL );
    return VP_ERR_NULL;	
}

//添加扣款函数;by gzz 20110823
unsigned char VP_CMD_Cost( void )
{
    unsigned char i = 0;
    unsigned char j = 0;
	unsigned int data CurrentMoney=0;
	//u_char xdata len = 0;
	//u_char xdata str[20];

    //1.Check the data
    if( sysVPMission.receive.datLen != 4  )
	{
	    VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}
    sysVPMission.costDev = sysVPMission.receive.msg[0];
	sysVPMission.costMoney = MoneyRec(sysVPMission.receive.msg[1], sysVPMission.receive.msg[2]);
	//2.得到当前投入的金额
	if( sysITLMission.billHoldingFlag == 1 )
	{
		CurrentMoney = CurrentPayed + sysITLMission.billHoldingValue;
	}
	else
	{
		CurrentMoney = CurrentPayed;
	}
	//3.用户投币金额小于扣款金额时，返回NAK_RPT    
    if( (sysVPMission.costDev != 0) || (sysVPMission.costMoney > CurrentMoney ) )
	{
		//len = sprintf( str, "1=%lu,%lu", sysVPMission.costMoney,CurrentMoney );
		//DisplayStr( 0, 0, 1, str, strlen(str) );  
		//WaitForWork(2000,NULL);
    	VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}
	//扣钱后不能找零
	if(!(IsCanChange(sysVPMission.costMoney)))
	{
		//DisplayStr( 0, 0, 1, "2", 1 );  
		//WaitForWork(2000,NULL);
    	VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}	

	//正在进行出货，找零，或扣款操作
	if(( sysVPMission.vendCmd == 1 )||(sysVPMission.changeCmd == 1)||(sysVPMission.costCmd == 1))
	{
    	//DisplayStr( 0, 0, 1, "5", 1 ); 
		//WaitForWork(5000,NULL);
		VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;	    
	}	
	
	sysVPMission.costType = sysVPMission.receive.msg[3];

	//2.ACK
	if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK )
	{
    	VPMsgPackSend( VP_ACK_RPT, 0, 0 );
	}
	

    //3.Set command flag
	sysVPMission.costCmd = 1;
    return VP_ERR_NULL;	
}

unsigned char VP_CMD_Reset( void )
{
    unsigned char i = 0;
    unsigned char j = 0;
    
	//1.Check the data
    if( sysVPMission.receive.datLen != 1  )
	{
	    VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}
    sysVPMission.resetDev = sysVPMission.receive.msg[0];
    //2.ACK
	if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK )
	{
    	VPMsgPackSend( VP_ACK_RPT, 0, 0 );
	}
    //3.Reset action......
	sysVPMission.resetCmd = 1;
	sysVPMission.VPInit = 1;
    return VP_ERR_NULL;	
}

unsigned char VP_CMD_Control( void )
{
    unsigned char i = 0;
    unsigned char j = 0;
    
	//1.Check the data
    if( sysVPMission.receive.datLen != 2  )
	{
	    VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}
	//if( !( ((sysVPMission.receive.msg[0]>=1)&&(sysVPMission.receive.msg[0]<=30))&&(sysVPMission.receive.msg[1]<=1) )  )
	if( !( (sysVPMission.receive.msg[0]==2)||(sysVPMission.receive.msg[0]==6)||(sysVPMission.receive.msg[0]==16)||(sysVPMission.receive.msg[0]==18) ))
	{
	    VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}
    sysVPMission.ctrType  = sysVPMission.receive.msg[0];
	sysVPMission.ctrValue = sysVPMission.receive.msg[1];
    //Judge the device status
    /*
     if( ((sysVPMission.ctrType==VP_DEV_CTR_LED1)||(sysVPMission.ctrType==VP_DEV_CTR_COMPRESSOR1))&&((SystemParameter.ACDCModule==0)||(DeviceStatus.ACDCModule!=0)) )
     {
    	 VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
     }
	*/
    //2.ACK	
	if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK )
    {
		VPMsgPackSend( VP_ACK_RPT, 0, 0 );
	}
    //3.Action......
	if( sysVPMission.ctrType == VP_DEV_BILLCOIN )
	{
    	if( sysVPMission.ctrValue == 0 )
		{
			sysVPMission.billCoinEnable = 0;
		}
		else
		{
			sysVPMission.billCoinEnable = 1;
		}
	}
	else if( sysVPMission.ctrType == VP_DEV_PAYOUT )
	{
		if( sysVPMission.ctrValue == 0 )
		{
			sysMDBMission.returnCmdFlag = 1;
		}		
	}
	else if( sysVPMission.ctrType == VP_DEV_GAME_LED )
	{
    	if( sysVPMission.ctrValue == 0 )
		{
			GameLedOff();
		}
		else
		{
			GameLedOn();
		}
	}
	else if( sysVPMission.ctrType == VP_DEV_SCALFACTOR )
	{
		if( sysVPMission.ctrValue >= 3  )
		{
			HardWareErr |= 0x2000;
		    VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
			return VP_ERR_PAR;
		}
		else
		{
			if(sysVPMission.decimalPlaces == sysVPMission.ctrValue)
			{
				HardWareErr &=  ~(0x2000);
    			sysVPMission.decimalPlaces = sysVPMission.ctrValue;
			}
			else
			{
				HardWareErr |= 0x2000;
			}
		}
	}
    return VP_ERR_NULL;	
}

unsigned char VP_CMD_GetStatus( void )
{
    unsigned char i = 0;
    unsigned char j = 0;

	//1.Check the data
    if( sysVPMission.receive.datLen != 0  )
	{
	    VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}
	if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK  )
	{
		VPMsgPackSend( VP_ACK_RPT, 0, 0  );
	}
	//2.
    VPMission_Status_RPT();
    return VP_ERR_NULL;	
}

unsigned char VP_CMD_Payout( void )
{
    unsigned char i = 0;
    unsigned char j = 0;
	u_int  xdata tempPrice;
	u_char xdata resultdisp[2] = {0, 0};

    //1.Check the data
    if( sysVPMission.receive.datLen != 4  )
	{
	    VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}
    sysVPMission.changeDev   = sysVPMission.receive.msg[0];
	sysVPMission.changeMoney = MoneyRec(sysVPMission.receive.msg[1],sysVPMission.receive.msg[2]);
	//if( (sysVPMission.changeDev != 0) || (sysVPMission.changeMoney < SystemParameter.HopperCoinPrice1 ) )
    if( (sysVPMission.changeDev != 0) || (sysVPMission.changeMoney > sysMDBMission.coinAllValue ) || (sysMDBMission.coinDeviceStatus!=0) 
		||( sysMDBMission.coinDeviceStatus != 0 )||( sysMDBMission.tubeRemoved == 1 ))
	{
    	VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}
	//扣钱后不能找零
	tempPrice = sysVPMission.changeMoney/10;
	if(!(changeCNY2(resultdisp,tempPrice)))
	{
    	VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}
	//正在进行出货，找零，或扣款操作
	if(( sysVPMission.vendCmd == 1 )||(sysVPMission.changeCmd == 1)||(sysVPMission.costCmd == 1))
	{
    	//DisplayStr( 0, 0, 1, "5", 1 ); 
		//WaitForWork(5000,NULL);
		VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;	    
	}
	sysVPMission.changeType = sysVPMission.receive.msg[3];

	//2.ACK
	if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK )
	{
    	VPMsgPackSend( VP_ACK_RPT, 0, 0 );
	}

    //3.Set command flag
	sysVPMission.changeCmd = 1;
    return VP_ERR_NULL;	
}


unsigned char VP_CMD_GetColumnSta( void )
{
    unsigned char i = 0;
    unsigned char j = 0;

	//1.Check the data
	/*
    if( sysVPMission.receive.datLen != 0  )
	{
	    VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 
		return VP_ERR_PAR;
	}
	if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK  )
	{
		VPMsgPackSend( VP_ACK_RPT, 0, 0  );
	}
	*/
	//2.
    VPMission_ColumnSta_RPT();
    return VP_ERR_NULL;	
}

//判断货道状态
unsigned char GoodStateLed(u_char i)
{
	if(( GoodsWaySetVal[i].WayState & 0x01 )!= 0x01)
	{
	    //未
	    return 0;
	}
	else if( GoodsWaySetVal[i].WayState & 0x0A )
	{
	    //故
	    return 2;
	}
	else if( (GoodsWaySetVal[i].WayState & 0x04) || (GoodsWaySetVal[i].GoodsCurrentSum == 0) )
	{
	    //缺
	    return 3;
	}
	else
	{
	    //正
	    return 1;
	}
}

//-----------------------------------------------------------------------------
unsigned char isLed_Goods( u_char ledSel )//这个灯对应的商品是否有货1有货,0无货
{
	u_char xdata i = 0;
	for( i=0; i<COLUMN_NUM_SET; i++ )
    {
    	if(GoodsWaySetVal[i].GoodsType == SystemParameter.selItem[ledSel])
    	{
    		if( ( GoodsWaySetVal[i].GoodsCurrentSum > 0 )&&( GoodsWaySetVal[i].Price > 0 )&&(GoodStateLed(i)==1) )
    		{
    			return 1;
    		}
    	}
	}
	return 0;
}

unsigned int isLed_Price( u_char ledSel )//这个灯对应的商品是否有货1有货,0无货
{
	u_char xdata i = 0;
	for( i=0; i<COLUMN_NUM_SET; i++ )
    {
    	if(GoodsWaySetVal[i].GoodsType == SystemParameter.selItem[ledSel])
    	{
    		return GoodsWaySetVal[i].Price;
    	}
	}	
}


unsigned char UpdateSelectionLed_GoodsSta( void )
{
	u_char xdata i = 0;

    sysVPMission.sel1ErrLed = 0;
    sysVPMission.sel1ReadyLed = 0;
	sysVPMission.sel2ErrLed = 0;
    sysVPMission.sel2ReadyLed = 0;
	sysVPMission.sel3ErrLed = 0;
    sysVPMission.sel3ReadyLed = 0;

	for( i=0; i<KEYEXTRAVAL; i++ )
    {
		if(isLed_Goods(i)==0)
        {
        	sysVPMission.sel1ErrLed |= 1<<i;
        }
        else
        {
            sysVPMission.sel1ReadyLed |= 1<<i;
        }
    }
    for( i=KEYEXTRAVAL; i<KEYEXTRAVAL*2; i++ )
    {
		if(isLed_Goods(i)==0)
        {
        	sysVPMission.sel2ErrLed |= 1<<(i-KEYEXTRAVAL);
        }
        else
        {
            sysVPMission.sel2ReadyLed |= 1<<(i-KEYEXTRAVAL);
        }
    }
	for( i=KEYEXTRAVAL*2; i<KEYEXTRAVAL*3; i++ )
    {
		if(isLed_Goods(i)==0)
        {
        	sysVPMission.sel3ErrLed |= 1<<(i-KEYEXTRAVAL*2);
        }
        else
        {
            sysVPMission.sel3ReadyLed |= 1<<(i-KEYEXTRAVAL*2);
        }
    }

    return VP_ERR_NULL;
}



/**
* 计算找零方案
* @param in length 数组的长度
 
* @param in coins
* @param in lefts
 
可找零币种
每种币剩余量
 
* @param out results 找零结果（返回结果"成功"时有效）
* @param in money
 
* @return
 
返回1：成功；返回0：失败
 
*/
u_char MakeChange2(int length, unsigned char* coins,
unsigned char* lefts, unsigned char* results, int money)
{
    unsigned char  i=0;
    unsigned char  count;
    //printf("allMoney=%d\n", money);

    for(i=0; i<length; ++i)
    {
        unsigned char coin = coins[i];
        unsigned char left = lefts[i];

        if(left <= 0 || coin > money)
        {
            continue;
        }
        count = money / coin;

        if(count > left)
        {
            count = left;
        }
        lefts[i] = left - count;

        //printf("%d=%d\n",coin,count);
        results[i] = count;

        money -= count*coin;
        if(money == 0)
        {
            break;
        }
    }

    if(money == 0)
    {
        return 1;
    }
    else
    {
        for(i=0; i<length; ++i)
        {
            results[i] = 0;
        }
        return 0;
    }
}

//找零人民币/*找零money元,即3.5元=35(money)*/
u_char changeCNY2(unsigned char* results, u_int money)
{
    //*可找零硬币面额（人民币角）（面额从大到小）*
    unsigned char xdata coins[2] = {10, 5};
    //*每种面额硬币剩余个数 *
    unsigned char xdata lefts[2] = {0, 0};
	//u_char xdata len = 0;
	//u_char xdata str[20];
	
	lefts[0] = sysMDBMission.coin1yNum;
	lefts[1] = sysMDBMission.coin5jNum;
    //unsigned char xdata results[2] = {0, 0};
    //len = sprintf( str, "1.1:%u.%02u,%u,%u", money/10,money%10,lefts[0],lefts[1] );
	//DisplayStr( 0, 0, 1, str, len );
	//WaitForWork( 5000, NULL );
    if(MakeChange2(2, coins, lefts, results, money))
    {
        
        //printf("CNY Success\n");
        //DisplayStr( 0, 0, 1, "1CNY Success",12 );
		//WaitForWork( 1000, NULL );
        return 1;
    }
    else
    {
        //printf("CNY Fail\n");
        //DisplayStr( 0, 0, 1, "1CNY Fail",9 );
		//WaitForWork( 1000, NULL );
        return 0;
    }

}


//是否可以找零1可找，0不可找
u_char IsCanChange(u_int    Price)
{	
	u_char xdata mission = 0;
	u_int  xdata tempPrice;
	u_char xdata resultdisp[2] = {0, 0};

	//硬币器故障时只能扣款或购买与暂存纸币等额的商品
	if( ( sysMDBMission.coinDeviceStatus != 0 )||( sysMDBMission.tubeRemoved == 1 ) )
	{
		if(Price > 0)
		{
			if(Price != (CurrentPayed + sysITLMission.billHoldingValue) )
			{
				return 0;
			}
			else
				return 1;
		}
		else
			return 1;
	}

	//硬币器正常时判断
	if(Price > CurrentPayed)
	{
		tempPrice = Price - CurrentPayed;
		if( sysITLMission.billHoldingFlag == 1 )
		{
			tempPrice = sysITLMission.billHoldingValue - tempPrice;
			tempPrice = tempPrice/10;
			mission =changeCNY2(resultdisp,tempPrice);
			if(mission == 0)	
			{				
				return 0;
			}
		}
	}
	else
	{		
		tempPrice = CurrentPayed - Price;
		tempPrice = tempPrice/10;
		mission =changeCNY2(resultdisp,tempPrice);
		if(mission == 0)		
		{			
			return 0;
		}
	}
	return 1;
}

unsigned char UpdateSelLed_Trade( void )
{   
    u_char xdata i = 0;
	u_int  xdata moneyBuf = 0;
	//uchar xdata flag = 0;
	//u_char xdata str[20];
    
	//----------------------------------------------------------------
	//2011.5.19 added for the bill holding status judgement
	if( sysITLMission.billHoldingFlag == 1 )
	{
		moneyBuf = CurrentPayed + sysITLMission.billHoldingValue;
	}
	else
	{
	    moneyBuf = CurrentPayed;
	}
	
    if( moneyBuf <= 0 )
	{
		return VP_ERR_NULL;	
	}
    //=================================================================

    //sysVPMission.sel1ErrLed = 0;
    sysVPMission.sel1ReadyLed = 0;
	//sysVPMission.sel2ErrLed = 0;
    sysVPMission.sel2ReadyLed = 0;
	//sysVPMission.sel3ErrLed = 0;
    sysVPMission.sel3ReadyLed = 0;
	

	for( i=0; i<KEYEXTRAVAL; i++ )
    {   
    	if( (isLed_Goods(i)) && ( moneyBuf >= isLed_Price(i) ) )
        {        
        	if(IsCanChange(isLed_Price(i)))
        	{
        		sysVPMission.sel1ReadyLed |= 1<<i;
				sysVPMission.sel1ErrLed   &= ~(1<<i);
        	}
        }
        //by gzz 20110611 
        else if(isLed_Goods(i)==0)
        {
        	sysVPMission.sel1ErrLed   |= 1<<i;
			sysVPMission.sel1ReadyLed &= ~(1<<i);
        }   
    }
	
    for( i=KEYEXTRAVAL; i<KEYEXTRAVAL*2; i++ )
    {
    	if( (isLed_Goods(i)) && ( moneyBuf >= isLed_Price(i) ) )
        {
        	if(IsCanChange(isLed_Price(i)))
    		{
        		sysVPMission.sel2ReadyLed |= 1<<(i-KEYEXTRAVAL);
				sysVPMission.sel2ErrLed   &= ~(1<<(i-KEYEXTRAVAL));
        	}
        }
        //by gzz 20110611 
        else if(isLed_Goods(i)==0)
        {
        	sysVPMission.sel2ErrLed |= 1<<(i-KEYEXTRAVAL);
			sysVPMission.sel2ReadyLed &= ~(1<<(i-KEYEXTRAVAL));
        }
    }

	for( i=KEYEXTRAVAL*2; i<KEYEXTRAVAL*3; i++ )
    {
    	if( (isLed_Goods(i)) && ( moneyBuf >= isLed_Price(i) ) )
        {
        	if(IsCanChange(isLed_Price(i)))
    		{
        		sysVPMission.sel3ReadyLed |= 1<<(i-KEYEXTRAVAL*2);
				sysVPMission.sel3ErrLed   &= ~(1<<(i-KEYEXTRAVAL*2));
        	}
        }
        //by gzz 20110611 
        else if(isLed_Goods(i)==0)
        {
        	sysVPMission.sel3ErrLed   |= 1<<(i-KEYEXTRAVAL*2);
			sysVPMission.sel3ReadyLed &= ~(1<<(i-KEYEXTRAVAL*2));
        }
    }
	
	
    return VP_ERR_NULL;
    
}

unsigned char UpdateGoodsMatrixStatus( unsigned char goodsNum )
{
    u_char xdata i = 0;
	u_char xdata j = 0;
	u_char xdata k = 0;
	u_char xdata oldOne = 0;
	u_char xdata position = 0;
	
	goodsNum=1;
	/*
	//-------------------------------------------------------------------------------------------------------------------
	//Update the information for vending
	if( goodsNum == 0xff )
	{
		for( i=0; i<GOODSTYPEMAX; i++)
		{
		    //-------------------------------------------------
		    //oldOne = sysGoodsMatrix[i].NextColumn;
			oldOne = sysGoodsMatrix[i].ColumnMatrix[0];
			for( k=0; k<sysGoodsMatrix[i].ColumnNum; k++ )
			{
				if( sysGoodsMatrix[i].NextColumn == sysGoodsMatrix[i].ColumnMatrix[k] )
				{
					oldOne = k;
					break;
				}
			}
			//=================================================
			if( oldOne >= sysGoodsMatrix[i].ColumnNum-1 )
			{
			    //1.find in all stage
				sysGoodsMatrix[i].NextColumn = 0xff;
		  		for( j=0; j<sysGoodsMatrix[i].ColumnNum; j++)
				{
				    k = sysGoodsMatrix[i].ColumnMatrix[j];
					if( /*(GoodsWaySetVal[k].Price > 0) && *n/(GoodsWaySetVal[k].GoodsCurrentSum > 0) && (GoodsWaySetVal[k].WayState & 0x01) && ( (GoodsWaySetVal[k].WayState & 0x0A ) == 0x00 ) )  //0x01, 0x02
					{
						sysGoodsMatrix[i].NextColumn = k;
						break;
					}
				}	
			}
			else
			{
			    //2.find in ascending order stage
				sysGoodsMatrix[i].NextColumn = 0xff;
				position = oldOne+1;
		  		for( j=position; j<sysGoodsMatrix[i].ColumnNum; j++)
				{
				    k = sysGoodsMatrix[i].ColumnMatrix[j];
					if( /*(GoodsWaySetVal[k].Price > 0) && * /(GoodsWaySetVal[k].GoodsCurrentSum > 0) && (GoodsWaySetVal[k].WayState & 0x01) && ( (GoodsWaySetVal[k].WayState & 0x0A ) == 0x00 ) )  //0x01, 0x02
					{
						sysGoodsMatrix[i].NextColumn = k;
						break;
					}
				}
				//3.find in descending order stage
				if( sysGoodsMatrix[i].NextColumn == GOODS_MATRIX_NONE )
				{
					sysGoodsMatrix[i].NextColumn = 0xff;
			  		for( j=0; j<sysGoodsMatrix[i].ColumnNum; j++)
					{
					    k = sysGoodsMatrix[i].ColumnMatrix[j];
						if( /*(GoodsWaySetVal[k].Price > 0) && * /(GoodsWaySetVal[k].GoodsCurrentSum > 0) && (GoodsWaySetVal[k].WayState & 0x01) && ( (GoodsWaySetVal[k].WayState & 0x0A ) == 0x00 ) )  //0x01, 0x02
						{
							sysGoodsMatrix[i].NextColumn = k;
							break;
						}
					}	
				}
			}
		    
		}
		
    }

	//
	if( (goodsNum>=0)&&(goodsNum<GOODSTYPEMAX) )
	{    
	    i = goodsNum;
		//-------------------------------------------------
	    //oldOne = sysGoodsMatrix[i].NextColumn;
		oldOne = sysGoodsMatrix[i].ColumnMatrix[0];
		for( k=0; k<sysGoodsMatrix[i].ColumnNum; k++ )
		{
			if( sysGoodsMatrix[i].NextColumn == sysGoodsMatrix[i].ColumnMatrix[k] )
			{
				oldOne = k;
				break;
			}
		}
		//=================================================
		if( oldOne >= sysGoodsMatrix[i].ColumnNum-1 )
		{
		    //1.find in all stage
			sysGoodsMatrix[i].NextColumn = 0xff;
	  		for( j=0; j<sysGoodsMatrix[i].ColumnNum; j++)
			{
			    k = sysGoodsMatrix[i].ColumnMatrix[j];
				if( /*(GoodsWaySetVal[k].Price > 0) && * /(GoodsWaySetVal[k].GoodsCurrentSum > 0) && (GoodsWaySetVal[k].WayState & 0x01) && ( (GoodsWaySetVal[k].WayState & 0x0A ) == 0x00 ) )  //0x01, 0x02
				{
					sysGoodsMatrix[i].NextColumn = k;
					break;
				}
			}	
		}
		else
		{
		    //2.find in ascending order stage
			sysGoodsMatrix[i].NextColumn = 0xff;
			position = oldOne+1;
	  		for( j=position; j<sysGoodsMatrix[i].ColumnNum; j++)
			{
			    k = sysGoodsMatrix[i].ColumnMatrix[j];
				if( /*(GoodsWaySetVal[k].Price > 0) && * /(GoodsWaySetVal[k].GoodsCurrentSum > 0) && (GoodsWaySetVal[k].WayState & 0x01) && ( (GoodsWaySetVal[k].WayState & 0x0A ) == 0x00 ) )  //0x01, 0x02
				{
					sysGoodsMatrix[i].NextColumn = k;
					break;
				}
			}
			//3.find in descending order stage
			if( sysGoodsMatrix[i].NextColumn == GOODS_MATRIX_NONE )
			{
				sysGoodsMatrix[i].NextColumn = 0xff;
		  		for( j=0; j<sysGoodsMatrix[i].ColumnNum; j++)
				{
				    k = sysGoodsMatrix[i].ColumnMatrix[j];
					if( /*(GoodsWaySetVal[k].Price > 0) && * /(GoodsWaySetVal[k].GoodsCurrentSum > 0) && (GoodsWaySetVal[k].WayState & 0x01) && ( (GoodsWaySetVal[k].WayState & 0x0A ) == 0x00 ) )  //0x01, 0x02
					{
						sysGoodsMatrix[i].NextColumn = k;
						break;
					}
				}	
			}
		}
		//--------------------------------------------------------------------------------------
		for( i=0; i<GOODSTYPEMAX; i++ )
	    {
	    	if( sysGoodsMatrix[i].GoodsType == sysGoodsMatrix[goodsNum].GoodsType )
			{
				sysGoodsMatrix[i].NextColumn = sysGoodsMatrix[goodsNum].NextColumn;
				//
			}
	    }
	}
    //------------------------------------------------------------------------------------------
    /*
	for( i=0; i<GOODSTYPEMAX-1; i++ )
	{
		for( j=i+1; j<GOODSTYPEMAX; j++ )
		{
			if( sysGoodsMatrix[i].GoodsType == sysGoodsMatrix[j].GoodsType )
			{
                if( sysGoodsMatrix[i].NextColumn != GOODS_MATRIX_NONE )
                {
					sysGoodsMatrix[j].NextColumn = sysGoodsMatrix[i].NextColumn;
				  //sysGoodsMatrix[j].ColumnNum  = sysGoodsMatrix[i].ColumnNum;
                }
                else if( sysGoodsMatrix[j].NextColumn != GOODS_MATRIX_NONE )
                {
                    sysGoodsMatrix[i].NextColumn = sysGoodsMatrix[j].NextColumn;
                }
			}
		}
	}
    * /
    
    //------------------------------------------------------------------------------------------
	for( i=0; i<GOODSTYPEMAX; i++)
	{
		//Trace("\n sysGoodsMatrix[%bu].GoodsType = %u", i, sysGoodsMatrix[i].GoodsType );
		//Trace("\n sysGoodsMatrix[%bu].ColumnNumber = %bu", i, sysGoodsMatrix[i].ColumnNumber );
		//Trace("\n sysGoodsMatrix[%bu].NextColumn = %u", i, sysGoodsMatrix[i].NextColumn );
		for( j=0; j<sysGoodsMatrix[i].ColumnNum; j++ )
		{
			//Trace("\n sysGoodsMatrix[%bu].ColumnMatrix[%bu] = %u", i, j, sysGoodsMatrix[i].ColumnMatrix[j] );
		}
	}
    */
    //Update the selection led status
	//2011.5.19 added judgement for the trade mode
    if( (CurrentPayed+sysITLMission.billHoldingValue) == 0 )
	{
		UpdateSelectionLed_GoodsSta();
	}
    return VP_ERR_NULL;
}

unsigned char VP_Update_ColumnGoodsPar( void )
{
    unsigned char i = 0;
	unsigned char j = 0;
	u_int  tempMoney = 0;
	//DisplayStr( 0, 0, 1, "1", 1 );  
	//WaitForWork(2000,NULL);
	//1.配置商品price
	if( sysVPMission.receive.datLen <= 124*2+1)
	{
		
		if( sysVPMission.receive.verFlag & VP_PROTOCOL_ACK )
		{
			VPMsgPackSend( VP_ACK_RPT, 0, 0 );
		}

		//----------------------------------
		//	1...			  n
		//SP1_Price...Sn_Price
		sysVPMission.GoodsRev = (sysVPMission.receive.datLen - 1) / 2;	
		for( i=0,  j=1; i<sysVPMission.GoodsRev; i++, j+=2 )
		{
			GoodsType[i] = i+1;			
			if((sysVPMission.receive.msg[j]*256+sysVPMission.receive.msg[j+1])>=6550)
			{
				Price[i]  = 65000;
			}
			else
			{	
				if(Price[i]  != MoneyRec(sysVPMission.receive.msg[j],sysVPMission.receive.msg[j+1]))
					Price[i]  = MoneyRec(sysVPMission.receive.msg[j],sysVPMission.receive.msg[j+1]);				
			}
			//sysVPMission.selItem[i] 	= sysGoodsMatrix[i].GoodsType;
			sysVPMission.selPrice[i] = Price[i];  
			if(sysVPMission.selPrice[i] >= tempMoney)
			{
				tempMoney = sysVPMission.selPrice[i];
				SystemParameter.BanknoteMax = sysVPMission.selPrice[i];
			}
		}	
		
		sysVPMission.sysStatus |= VPM_STA_GOODSPAR; 	
	}
	else
	{
		VPMsgPackSend( VP_NAK_RPT, 0, 0  );  //1, 0
		return VP_ERR_PAR;
	}
	
	//2.Update the storage status
    for( i=0; i<COLUMN_NUM_SET; i++ )
	{
        //GoodsWaySetVal[i].WayState = 0x01;  //Clear the error status 
		if( GoodsWaySetVal[i].GoodsCurrentSum > 0 )
		{
		    GoodsWaySetVal[i].WayState = 0x01;
		}		
		GoodsWaySetVal[j].Price = 0;//所有货道单价都设为0
	}
	//DisplayStr( 0, 0, 1, "2", 1 );  
	//WaitForWork(2000,NULL);
	//2.Update the goods matrix and column's price
	//for( i=0; i<GOODSTYPEMAX; i++ )
	//{
	//    sysGoodsMatrix[i].ColumnNum = 0;
	//}
	//3.
	for( j=0; j<COLUMN_NUM_SET; j++)
	{
		for( i=0; i<sysVPMission.GoodsRev; i++ )
		{		    
			if( GoodsWaySetVal[j].GoodsType == GoodsType[i] )
			{
            	//sysGoodsMatrix[i].ColumnMatrix[sysGoodsMatrix[i].ColumnNum++] = j;
				//if(sysGoodsMatrix[i].Price>=65535)
				//{
				//	GoodsWaySetVal[j].Price = 0;
				//}
				//else
				//{
				GoodsWaySetVal[j].Price = Price[i];
				//}
				break;
			}
		}
	}
	//DisplayStr( 0, 0, 1, "3", 1 );  
	//WaitForWork(2000,NULL);
	/*
	for( i=0; i<sysVPMission.GoodsRev; i++ )
	{
	    sysGoodsMatrix[i].ColumnNum = 0;
		for( j=0; j<COLUMN_NUM_SET; j++)
		{
			if( GoodsWaySetVal[j].GoodsType == sysGoodsMatrix[i].GoodsType )
			{
            	sysGoodsMatrix[i].ColumnMatrix[sysGoodsMatrix[i].ColumnNum++] = j;
				//GoodsWaySetVal[j].Price = sysGoodsMatrix[i].Price;
			}
		}
	}
	*/
	
	//3.Update goods matirx trade status
	UpdateGoodsMatrixStatus( 0xff );
    //4.Update flash status
    //-------------------------------------------------------------------
    //SaveGoodsSet();
    //SaveGoodsParSet();
    //===================================================================
	return VP_ERR_NULL;
}	
/*{
    unsigned char i = 0;
	unsigned char j = 0;

	//1.Update the storage status
    for( i=0; i<COLUMN_NUM_SET; i++ )
	{
        //GoodsWaySetVal[i].WayState = 0x01;  //Clear the error status 
		if( GoodsWaySetVal[i].GoodsCurrentSum > 0 )
		{
		    GoodsWaySetVal[i].WayState &= 0xFB;
		}
		else
		{
			GoodsWaySetVal[i].WayState |= 0x04;
		}
	}
	//2.Update the goods matrix and column's price
	for( i=0; i<GOODSTYPEMAX; i++ )
	{
	    sysGoodsMatrix[i].ColumnNum = 0;
		for( j=0; j<COLUMN_NUM_SET; j++)
		{
			if( GoodsWaySetVal[j].GoodsType == sysGoodsMatrix[i].GoodsType )
			{
            	sysGoodsMatrix[i].ColumnMatrix[sysGoodsMatrix[i].ColumnNum++] = j;
				GoodsWaySetVal[j].Price = sysGoodsMatrix[i].Price;
			}
		}
	}
	//3.Update goods matirx trade status
	UpdateGoodsMatrixStatus( 0xff );
    //4.Update flash status
    //-------------------------------------------------------------------
    //SaveGoodsSet();
    //SaveGoodsParSet();
    //===================================================================
	return VP_ERR_NULL;
}*/
//=======================================================================

//-----------------------------------------------------------------------
//VMC--PC command mission

//120419 by cq TotalSell
unsigned char VPMission_Startup_RPT_1( void )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

    retry = VP_COM_RETRY;
TAB_VPM_STARPT_RETRY:
	flag = VPMsgPackSend( VP_STARTUP_RPT_1, 1, 1);
	//flag = VPMsgPackSend( VP_STARTUP_RPT, 1, 1);
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		if( VPBusFrameUnPack() )
		{
		    sysVPMission.comErrNum = 0;
			goto TAB_VPM_STARPT_COM_OK;
		}
	}
	
	//if( retry-- )//120419 by cq TotalSell	
	if(0)
	{
		goto TAB_VPM_STARPT_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//
        return VP_ERR_COM;
	}
TAB_VPM_STARPT_COM_OK:
    switch( sysVPMission.receive.msgType )
	{
		default:
		    break;
	}
	return VP_ERR_NULL;
}


unsigned char VPMission_Startup_RPT( void )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

    retry = VP_COM_RETRY;
TAB_VPM_STARPT_RETRY:
	flag = VPMsgPackSend( VP_STARTUP_RPT, 1, 1);
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		if( VPBusFrameUnPack() )
		{
		    sysVPMission.comErrNum = 0;
			goto TAB_VPM_STARPT_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_VPM_STARPT_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//
        return VP_ERR_COM;
	}
TAB_VPM_STARPT_COM_OK:
    switch( sysVPMission.receive.msgType )
	{
		default:
		    break;
	}
	return VP_ERR_NULL;
}

unsigned char VPMission_Setup_RPT( void )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

    retry = VP_COM_RETRY;
TAB_VPM_SETUP_RETRY:
    //-------------------------------------------
	//Fixed by VMC or controlled by PC?
    //sysVPMission.proVer = VP_PRO_VER;
	sysVPMission.columnNumSet = COLUMN_NUM_SET;
	#ifdef   _SHUPING_
		sysVPMission.selectionNumSet = 0;
	#else
		sysVPMission.selectionNumSet = GOODSTYPEMAX;
	#endif
	
	//Added the machine VER
    //sysVPMission.send.msg[3] = VP_MAC_SET_H;
    //sysVPMission.send.msg[4] = VP_MAC_SET_L;
	//===========================================
	flag = VPMsgPackSend( VP_VMC_SETUP, 1, 1);
	//DisplayStr( 0, 0, 1, "2", 1 );  
	//WaitForWork(2000,NULL);
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		//DisplayStr( 0, 0, 1, "3", 1 );  
		WaitForWork(1000,NULL);
		if( VPBusFrameUnPack() )
		{
			//DisplayStr( 0, 0, 1, "4", 1 );  
			//WaitForWork(2000,NULL);
		    sysVPMission.comErrNum = 0;
			goto TAB_VPM_SETUP_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_VPM_SETUP_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//
        return VP_ERR_COM;
	}
TAB_VPM_SETUP_COM_OK:
    switch( sysVPMission.receive.msgType )
	{
		default:
		    break;
	}
	return VP_ERR_NULL;
}


unsigned char VPMission_Payin_RPT( unsigned char dev, unsigned int money )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

	if(sysVPMission.offLineFlag == 1)
	{		
		return VP_ERR_NULL;
	}
	//
    retry = VP_COM_RETRY;
//TAB_VPM_PAYIN_RETRY:
    //-------------------------------------------
    sysVPMission.payInDev = dev;
	sysVPMission.payInMoney = money;
	//===========================================
	flag = VPMsgPackSend( VP_PAYIN_RPT, 0, 1);
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	/*
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		if( VPBusFrameUnPack() )
		{			
		    sysVPMission.comErrNum = 0;
			if(sysVPMission.offLineFlag == 1)
			{
				sysVPMission.offLineFlag = 0;
				VPMission_Act_RPT(VP_ACT_ONLINE,0);
			}
			goto TAB_VPM_PAYIN_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_VPM_PAYIN_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//-----------------------------------
		//3 次PC仍无响应，则跳过该消息，同时打印“PAYIN_RPT Err, device=类型,val=金额”
		if((  sysVPMission.comErrNum >= VP_TIME_OUNT_NUM )&&(sysVPMission.offLineFlag != 1) )
		{
			sysVPMission.offLineFlag = 1;
		}
        
        return VP_ERR_COM;
	}
TAB_VPM_PAYIN_COM_OK:
    switch( sysVPMission.receive.msgType )
	{
		default:
		    break;
	}
	*/
	return VP_ERR_NULL;
}


unsigned char VPMission_Payout_RPT( unsigned char dev, unsigned int money1, unsigned int money2 )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

	if(sysVPMission.offLineFlag == 1)
	{
		return VP_ERR_NULL;
	}
	//
    retry = VP_COM_RETRY;
TAB_VPM_PAYOUT_RETRY:
    //-------------------------------------------
    sysVPMission.payOutDev = dev;
	sysVPMission.payOutMoney1 = money1;
	sysVPMission.payOutMoney2 = money2;
	//===========================================
	flag = VPMsgPackSend( VP_PAYOUT_RPT, 1, 1);
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		if( VPBusFrameUnPack() )
		{
		    sysVPMission.comErrNum = 0;
			if(sysVPMission.offLineFlag == 1)
			{
				sysVPMission.offLineFlag = 0;
				VPMission_Act_RPT(VP_ACT_ONLINE,0);
			}
			goto TAB_VPM_PAYOUT_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_VPM_PAYOUT_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//-----------------------------------
		//?
        if((  sysVPMission.comErrNum >= VP_TIME_OUNT_NUM )&&(sysVPMission.offLineFlag != 1) )
		{
			sysVPMission.offLineFlag = 1;
		}
        return VP_ERR_COM;
	}
TAB_VPM_PAYOUT_COM_OK:
    switch( sysVPMission.receive.msgType )
	{
		default:
		    break;
	}
	return VP_ERR_NULL;
}


unsigned char VPMission_Payout_RPTNOACK( unsigned char dev, unsigned int money1, unsigned int money2 )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

	if(sysVPMission.offLineFlag == 1)
	{
		return VP_ERR_NULL;
	}
	//
    retry = VP_COM_RETRY;
//TAB_VPM_PAYOUT_RETRY:
    //-------------------------------------------
    sysVPMission.payOutDev = dev;
	sysVPMission.payOutMoney1 = money1;
	sysVPMission.payOutMoney2 = money2;
	//===========================================
	flag = VPMsgPackSend( VP_PAYOUT_RPT, 0, 1);
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	/*
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		if( VPBusFrameUnPack() )
		{
		    sysVPMission.comErrNum = 0;
			if(sysVPMission.offLineFlag == 1)
			{
				sysVPMission.offLineFlag = 0;
				VPMission_Act_RPT(VP_ACT_ONLINE,0);
			}
			goto TAB_VPM_PAYOUT_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_VPM_PAYOUT_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//-----------------------------------
		//?
        if((  sysVPMission.comErrNum >= VP_TIME_OUNT_NUM )&&(sysVPMission.offLineFlag != 1) )
		{
			sysVPMission.offLineFlag = 1;
		}
        return VP_ERR_COM;
	}
TAB_VPM_PAYOUT_COM_OK:
    switch( sysVPMission.receive.msgType )
	{
		default:
		    break;
	}
	*/
	return VP_ERR_NULL;
}


//发送pc扣款后金额;by gzz 20110823
unsigned char VPMission_Cost_RPT( unsigned char dev, unsigned int money,unsigned char type )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

	if(sysVPMission.offLineFlag == 1)
	{
		return VP_ERR_NULL;
	}
	//
    retry = VP_COM_RETRY;
TAB_VPM_COST_RETRY:  
    //-------------------------------------------
    sysVPMission.costDev = dev;
	sysVPMission.costMoney = money;
    sysVPMission.costType = type;
	//全部扣款
	SellAccumulateMoney(&TradeCounter.vpSuccessMoney,sysVPMission.costMoney);
	//现金出货
	SellAccumulateMoney(&TradeCounter.vpCashMoney,sysVPMission.costMoney);
	//累计销售
	//SellAccumulateMoney(&TradeCounter.TotalSellMoeny,sysVPMission.costMoney);	
	//===========================================
	flag = VPMsgPackSend( VP_COST_RPT, 1, 1);
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		if( VPBusFrameUnPack() )
		{
		    sysVPMission.comErrNum = 0;
			if(sysVPMission.offLineFlag == 1)
			{
				sysVPMission.offLineFlag = 0;
				VPMission_Act_RPT(VP_ACT_ONLINE,0);
			}
			goto TAB_VPM_COST_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_VPM_COST_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//-----------------------------------
		//?
        if((  sysVPMission.comErrNum >= VP_TIME_OUNT_NUM )&&(sysVPMission.offLineFlag != 1) )
		{
			sysVPMission.offLineFlag = 1;
		}
        return VP_ERR_COM;
	}
TAB_VPM_COST_COM_OK:
    switch( sysVPMission.receive.msgType )
	{
		default:
		    break;
	}
	return VP_ERR_NULL;
}




//当上报欠款时，上报pc端;by gzz 20110825
unsigned char VPMission_Debt_RPT( unsigned char type, unsigned char dev, unsigned int money )
{
	

    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;
    
	//
    retry = VP_COM_RETRY;
//TAB_VPM_DEBT_RETRY:  
    //-------------------------------------------
    sysVPMission.debtType = type;
    sysVPMission.debtDev = dev;
	sysVPMission.debtMoney = money;
	//===========================================
	//flag = VPMsgPackSend( VP_DEBT_RPT, 0, 1);//120419 by cq TotalSell
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}	
	return VP_ERR_NULL;
	
}


//120521 by cq SellAccumulate
void SellAccumulateNum(unsigned long * ul_num)
{
	(*ul_num)++;
	if(*ul_num >= 100000)
		*ul_num -= 100000;
}

//120521 by cq SellAccumulate
void SellAccumulateMoney(unsigned long * ul_money, unsigned int ui_cost)
{
	*ul_money += ui_cost;
	if(*ul_money >= 10000000)
		*ul_money -= 10000000;
}


void changeFailedBeep()
{
	Beep();
	WaitForWork(200,NULL);
	Beep();
}

unsigned char VPMission_Vendout_RPT( unsigned char status, unsigned char column, unsigned char type, unsigned int cost, unsigned char columnLeft )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;	

	
	//--------------------------------------------
	/*
	#ifndef MACHINE_SET_VMC_PC
	   return VP_ERR_NULL;
	#endif
	*/
    //============================================

	//120419 by cq TotalSell
	//120521 by cq SellAccumulate
	if(status == 0)
	{
		/*
		TradeCounter.TotalSellNum++;
		if(TradeCounter.TotalSellNum > 9999999)
			TradeCounter.TotalSellNum = 0;
		
		TradeCounter.TotalSellMoeny += cost;
		if(TradeCounter.TotalSellMoeny > 999999999)
			TradeCounter.TotalSellMoeny = 0;
		*/
		
		//累计销售
		//SellAccumulateNum(&TradeCounter.TotalSellNum);
		//SellAccumulateMoney(&TradeCounter.TotalSellMoeny,cost);

		//全部出货
		SellAccumulateNum(&TradeCounter.vpSuccessNum);
		if(type == 0)
		{
			SellAccumulateMoney(&TradeCounter.vpSuccessMoney,cost);
		}

		if(type == 0)//现金出货
		{	
			SellAccumulateNum(&TradeCounter.vpCashNum);
			SellAccumulateMoney(&TradeCounter.vpCashMoney,cost);
		}
		else if(type == 1)//游戏出货
		{
			SellAccumulateNum(&TradeCounter.vpGameNum);
			//SellAccumulateMoney(&TradeCounter.vpGameMoney,cost);
		}		
		//else if(type == 5 || (type >= 101 && type <= 255))//刷卡出货
		else if(type == 5 || type >= 101 )//刷卡出货
		{
			SellAccumulateNum(&TradeCounter.vpCardNum);
			//SellAccumulateMoney(&TradeCounter.vpCardMoney,cost);
		}
		//else if((type >= 2 && type <= 4) || (type >= 6 && type <= 100))//在线出货
		else//在线出货
		{
			SellAccumulateNum(&TradeCounter.vpOnlineNum);
			//SellAccumulateMoney(&TradeCounter.vpOnlineMoney,cost);
		}		
	}
	else if(status == 2)
	{
		changeFailedBeep();
		//离线时不上报
		if(sysVPMission.offLineFlag == 0)
		{
			VP_CMD_GetColumnSta();
		}
	}

	//离线时不上报
	if(sysVPMission.offLineFlag == 1)
	{
		return VP_ERR_NULL;
	}
    retry = VP_COM_RETRY;
TAB_VPM_VENDOUT_RETRY:
    //-------------------------------------------
    sysVPMission.vendSta = status;
	sysVPMission.vendColumn = column+1;
	sysVPMission.vendType = type;
	sysVPMission.vendCost = cost;
	sysVPMission.vendcolumnLeft = columnLeft;
	

	
	//TradeCounter.TotalSellNum = 1234567;//120419 by cq TotalSell
	//TradeCounter.TotalSellMoeny = 123456789;//120419 by cq TotalSell
	//TradeCounter.TotalSellNum = 9999997;//120419 by cq TotalSell
	//TradeCounter.TotalSellMoeny = 999999989;//120419 by cq TotalSell
	
	//===========================================
	flag = VPMsgPackSend( VP_VENDOUT_RPT, 1, 1);
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		if( VPBusFrameUnPack() )
		{
		    sysVPMission.comErrNum = 0;
			if(sysVPMission.offLineFlag == 1)
			{
				sysVPMission.offLineFlag = 0;
				VPMission_Act_RPT(VP_ACT_ONLINE,0);
			}
			goto TAB_VPM_VENDOUT_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_VPM_VENDOUT_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//-----------------------------------
		//?
        if((  sysVPMission.comErrNum >= VP_TIME_OUNT_NUM )&&(sysVPMission.offLineFlag != 1) )
		{
			sysVPMission.offLineFlag = 1;
		}
        return VP_ERR_COM;
	}
TAB_VPM_VENDOUT_COM_OK:
    switch( sysVPMission.receive.msgType )
	{
		default:
		    break;
	}
	return VP_ERR_NULL;
}


unsigned char VPMission_Request( unsigned char type )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

	if(sysVPMission.offLineFlag == 1)
	{
		return VP_ERR_NULL;
	}
    retry = VP_COM_RETRY;
TAB_VPM_REQUEST_RETRY:
    //-------------------------------------------
    sysVPMission.request = type;
	//===========================================
	flag = VPMsgPackSend( VP_REQUEST, 1, 1);
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		if( VPBusFrameUnPack() )
		{
		    sysVPMission.comErrNum = 0;
			if(sysVPMission.offLineFlag == 1)
			{
				sysVPMission.offLineFlag = 0;
				VPMission_Act_RPT(VP_ACT_ONLINE,0);
			}
			goto TAB_VPM_REQUEST_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_VPM_REQUEST_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//-----------------------------------
		//?
        if((  sysVPMission.comErrNum >= VP_TIME_OUNT_NUM )&&(sysVPMission.offLineFlag != 1) )
		{
			sysVPMission.offLineFlag = 1;
		}
        return VP_ERR_COM;
	}
TAB_VPM_REQUEST_COM_OK:
    switch( sysVPMission.receive.msgType )
	{
		default:
		    break;
	}
	return VP_ERR_NULL;
}

//unsigned char VPMission_Admin_RPT( unsigned char type, unsigned char column, unsigned char num )
unsigned char VPMission_Admin_RPT( unsigned char type, unsigned char column, unsigned int num )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;
	//u_char xdata str[20];
	//u_char xdata len = 0;

	if(sysVPMission.offLineFlag == 1)
	{
		return VP_ERR_NULL;
	}
	//
    retry = VP_COM_RETRY;
TAB_VPM_ADMIN_RETRY:
    //-------------------------------------------
	if( type == VP_ADMIN_COL_PRICE )    //price
	{
		sysVPMission.ADM_Type   = type;
		sysVPMission.ADM_Dat[0] = column;
		sysVPMission.ADM_Dat[1] = num/256;
        sysVPMission.ADM_Dat[2] = num%256;
	}
	else  //the single column's goods, all columns' goods, or the tray columns'goods
	{
    	sysVPMission.ADM_Type   = type;
		sysVPMission.ADM_Dat[0] = column;
		sysVPMission.ADM_Dat[1] = num;
	}
	//===========================================
	flag = VPMsgPackSend( VP_ADMIN_RPT, 1, 1);
	//DisplayStr( 0, 0, 1, "01", 2 );  
	WaitForWork(2000,NULL);
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		//DisplayStr( 0, 0, 1, "02", 2 );  
		//WaitForWork(2000,NULL);
		if( VPBusFrameUnPack() )
		{
			//len = sprintf( str, "01:%02bu", sysVPMission.receive.msgType );
			//DisplayStr( 0, 0, 1, str, strlen(str) ); 
			//WaitForWork(2000,NULL);
		    sysVPMission.comErrNum = 0;
			if(sysVPMission.offLineFlag == 1)
			{
				sysVPMission.offLineFlag = 0;
				VPMission_Act_RPT(VP_ACT_ONLINE,0);
			}
			goto TAB_VPM_ADMIN_COM_OK;
		}
		//DisplayStr( 0, 0, 1, "04", 2 );  
		//WaitForWork(2000,NULL);
	}
	if( retry-- )
	{
		goto TAB_VPM_ADMIN_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//?
		if((  sysVPMission.comErrNum >= VP_TIME_OUNT_NUM )&&(sysVPMission.offLineFlag != 1) )
		{
			sysVPMission.offLineFlag = 1;
		}
        return VP_ERR_COM;
	}
TAB_VPM_ADMIN_COM_OK:
    switch( sysVPMission.receive.msgType )
	{   		
        case VP_NAK:
			changeFailedBeep();
            return VP_NAK;
        	break;
		case VP_ACK:
		    return VP_ACK;
		    break;
		case VP_HOUDAO_IND:
			//DisplayStr( 0, 0, 1, "2", 1 );  
			//WaitForWork(2000,NULL);
		    VP_CMD_ColumnPar();
			//if( sysVPMission.sysStatus & VPM_STA_PCPAROK )
			{
				//UpdateGoodsMatrixStatus( 0xff );
               // VP_Update_ColumnGoodsPar();
			}
		    break;
		case VP_HUODAO_SET_IND: 
			//DisplayStr( 0, 0, 1, "02", 2 );  
			//WaitForWork(2000,NULL);
			VP_CMD_ColumnNo();	
			//len = sprintf( str, "1.1:%02bu",sysVPMission.sysStatus);
    		//DisplayStr( 0, 0, 1, str, strlen(str) ); 
			//WaitForWork(2000,NULL);
			break;	
		case VP_POSITION_IND:
			//DisplayStr( 0, 0, 1, "4", 1 );  
			//WaitForWork(2000,NULL);
		    VP_CMD_GoodsPos();			
			break;	
		case VP_SALEPRICE_IND:
			//DisplayStr( 0, 0, 1, "5", 1 );  
			//WaitForWork(2000,NULL);
			//VP_CMD_GoodsPar();
			//if( sysVPMission.sysStatus & VPM_STA_PCPAROK )
			{
				//UpdateGoodsMatrixStatus( 0xff );
                VP_Update_ColumnGoodsPar();
				SaveGoodsSet();
				WaitForWork( 100, NULL );
			}
			break;	
		default:
		    break;
	}
	return VP_ERR_NULL;

}



unsigned char VPMission_Button_RPT( unsigned char type, unsigned char value )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

	if(sysVPMission.offLineFlag == 1)
	{
		if(type==VP_BUT_GAME)//游戏按键蜂鸣器响;by gzz 20110721
	    {   
	    	Beep();
	    }
		return VP_ERR_NULL;
	}
	//
    retry = VP_COM_RETRY;
//TAB_VPM_BUTTON_RETRY:
    //-------------------------------------------
    sysVPMission.BUT_Type   = type;
	sysVPMission.BUT_Value  = value;
    if(type==VP_BUT_GAME)//游戏按键蜂鸣器响;by gzz 20110721
    {  
    	//return VP_ERR_PAR;
    	Beep();
    }
	//===========================================
    //1-0: button message, not need ACK
	flag = VPMsgPackSend( VP_BUTTON_RPT, 0, 1);   
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
    /*
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		if( VPBusFrameUnPack() )
		{
			goto TAB_VPM_BUTTON_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_VPM_BUTTON_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//-----------------------------------
		//?
        
        return VP_ERR_COM;
	}
TAB_VPM_BUTTON_COM_OK:
    switch( sysVPMission.receive.msgType )
	{
		default:
		    break;
	}
    */
	return VP_ERR_NULL;

}

unsigned char VPMission_Status_RPT( void )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;
	unsigned int data CurrentMoney=0;

	if(sysVPMission.offLineFlag == 1)
	{
		return VP_ERR_NULL;
	}
    retry = VP_COM_RETRY;
	//------------------------------------------------------
    //1.硬币器状态cc_st
	//if( DeviceStatus.CoinAccepter != 0 )
	if(sysVPMission.billCoinEnable == 0)
	{
        sysVPMission.STA_CoinA = 1;
    }	
    else if( sysMDBMission.coinDeviceStatus != 0 )
	{
        sysVPMission.STA_CoinA = 3;
    }
    //币管没关紧，上报故障;by gzz 20110902
	else if( sysMDBMission.tubeRemoved == 1 )
	{
		sysVPMission.STA_CoinA = 2;
	}
	//硬币器其他故障
	else if(sysMDBMission.coinOthErr != 0)
	{
		sysVPMission.STA_CoinA = 2;
	}	
	else if(sysMDBMission.coinIsEnable == 0)
	{
        sysVPMission.STA_CoinA = 1;
    }	
	else
	{
		sysVPMission.STA_CoinA = 0;
	}

    //2.机器中当前可用于找零的5角硬币量cnt5j
    //if( DeviceStatus.ChangeUnit1 == 0 )
    if( ( sysMDBMission.coinDeviceStatus == 0x00 )&&(sysMDBMission.coin5jNum > 0) )
    	sysVPMission.STA_ChangeNum1 = sysMDBMission.coin5jNum;   //VP_STA_CHANGE_ENOUGH;
    else
	    sysVPMission.STA_ChangeNum1 = 0;

    //3.机器中当前可用于找零的1元硬币量cnt1y
    //if( DeviceStatus.ChangeUnit2 == 0 )
    if( ( sysMDBMission.coinDeviceStatus == 0x00 )&&(sysMDBMission.coin1yNum > 0) )
    	sysVPMission.STA_ChangeNum2 = sysMDBMission.coin1yNum;   //VP_STA_CHANGE_ENOUGH;
    else
	    sysVPMission.STA_ChangeNum2 = 0;

    //4.纸币器状态bv_st
	//1. 
	if( SystemParameter.BillNo == 1 )
	{
	    //if( DeviceStatus.BillAccepter == 0 )
	    if(sysVPMission.billCoinEnable == 0)
		{
	        sysVPMission.STA_BillA = 1;
	    }	
		//钞箱已满
		else if(sysMDBMission.billIsFull == 1)
		{
			sysVPMission.STA_BillA = 2;	
		}
		else if(sysMDBMission.billCashBuf == 1)//钞箱移除报故障;by gzz 20121224
	    {
			if( sysITLMission.billHoldingFlag == 1 )
			{
				CurrentMoney = CurrentPayed + sysITLMission.billHoldingValue;
			}
			else
			{
				CurrentMoney = CurrentPayed;
			}
			if( (CurrentMoney == 0)&&( sysITLMission.billHoldingFlag == 0 ) )
			{
				sysVPMission.STA_BillA = 2;	
			}
			else
			{
				sysVPMission.STA_BillA = 0;	
			}				
		}		
	    else if( sysMDBMission.billDeviceStatus == 0 )
		{
			if(sysMDBMission.billIsEnable == 0)
			{
				sysVPMission.STA_BillA = 1;			
			}
			else
			{
				sysVPMission.STA_BillA = 0;
			}
            
			//sysVPMission.STA_BillCount = 0;//by gzz 20110905
		}
		//纸币器其他故障
		else if(sysMDBMission.billOthErr != 0)
		{
			sysVPMission.STA_BillA = 2;			
		}
		/*
        //1)如果发现此次纸币器和硬币器同时故障，则全部上报
        //2)pc轮询时，如果只有纸币器故障，给予累加，只有连续故障次数大于5次时，才上报
		//by gzz 20110905
		else if(( sysMDBMission.coinDeviceStatus != 0 )||( sysVPMission.STA_BillCount >= 1 ))
		{
			sysVPMission.STA_BillA = 2;			
		}		
		else//by gzz 20110905 
		{
		    sysVPMission.STA_BillA = 0;
			sysVPMission.STA_BillCount ++;
		}
		*/
		else//by gzz 20110905 
		{
		    sysVPMission.STA_BillA = 3;
		}
	}
	else
	{
		sysVPMission.STA_BillA = 3;
	}

    //5.找零设备状态ch1_st,ch2_st
	//2.
    if( SystemParameter.HopperCoinPrice1 > 0 )
	{
		//if( DeviceStatus.ChangeUnit1 == 1 )
        if( sysMDBMission.coinDeviceStatus != 0 )
		{
			sysVPMission.STA_Changer1 = 2;
		}
		//币管没关紧，上报故障;by gzz 20110901
		else if( sysMDBMission.tubeRemoved == 1 )
		{
			sysVPMission.STA_Changer1 = 2;
		}
		else
		{
			sysVPMission.STA_Changer1 = 0;
		}
	}
	else
	{
		sysVPMission.STA_Changer1 = 3;
	}
    //
	if( SystemParameter.HopperCoinPrice2 > 0 )
	{
	    
		//if( DeviceStatus.ChangeUnit2 == 1 )
        if( sysMDBMission.coinDeviceStatus != 0 )
		{
			sysVPMission.STA_Changer2 = 2;
		}
		//币管没关紧，上报故障;by gzz 20110901
		else if( sysMDBMission.tubeRemoved == 1 )
		{
			sysVPMission.STA_Changer2 = 2;
		}
		else
		{
			sysVPMission.STA_Changer2 = 0;
		}

	}
	else
	{
		sysVPMission.STA_Changer2 = 3;
	}
    
    //6.主控器状态vmc_st
	//只有机器因故障而不能工作后，才上报故障信息;by gzz 20110902
	//if( HardWareErr == 0 )
	if(( HardWareErr & 0x0010 )||( HardWareErr & 0x0080 ))
    {
	    sysVPMission.STA_VMC = 1;
	}
	//else if(( HardWareErr & 0x0020 )||( HardWareErr & 0x0040 )||( HardWareErr & 0x0100 )
	//	   ||( HardWareErr & 0x0200 )||( HardWareErr & 0x1000 )||( HardWareErr & 0x2000 ))
	else if( HardWareErr != 0)
    {
	    sysVPMission.STA_VMC = 2;
	}
	else if(sysVPMission.adminType == 1)
	{
		sysVPMission.STA_VMC = 3;
	}
    else
	{
		sysVPMission.STA_VMC = 0;
	}
	
	
	
    //7.选货1,选货2,选货3状态
	//sysVPMission.STA_Tep1    = 0x7f;
    //sysVPMission.STA_Tep2    = 0x7f;
	if( DeviceStatus.Selection[0] == 0 )
	{
		sysVPMission.STA_Tep1 = 0;
	}
	else
	{
		sysVPMission.STA_Tep1 = 2;	
	}
    //
    if( DeviceStatus.Selection[1] == 0 )
	{
		sysVPMission.STA_Tep2 = 0;
	}
	else
	{
		sysVPMission.STA_Tep2 = 2;	
	}
    
	//sysVPMission.STA_Bank   = 3;
	if( DeviceStatus.Selection[2] == 0 )
	{
		sysVPMission.STA_Tep3 = 0;
	}
	else
	{
		sysVPMission.STA_Tep3 = 2;	
	}
    

    

	//8.出货检测
	//sysVPMission.STA_ICCard    = 3;
	if( SystemParameter.GOCOn == 1 )
	{
        //只有确认出货检测故障，才发送故障信息给pc端;by cq 20110815
        /*	            
        if( DeviceStatus.GOCStatus & 0x01 )
		{
			sysVPMission.STA_ICCard = 2;
		}
		else 
		{
			sysVPMission.STA_ICCard = 0;
		} 
        */
        if( DeviceStatus.GOCStatus == 0 )
        {
        	sysVPMission.STA_ICCard = 0;
            sysVPMission.STA_ICCardCount = 0;//by gzz 20111012
        }
        else if(( DeviceStatus.GOCStatus & 0x01 )&&( sysVPMission.STA_ICCardCount > 5 ))
        {
			sysVPMission.STA_ICCard = 2;
		}
        else 
		{
			sysVPMission.STA_ICCard = 0;
            sysVPMission.STA_ICCardCount ++;
		} 
	}
	else
	{
		sysVPMission.STA_ICCard = 1;
	}
	/*
	//8.公交一卡通gj
	if( SystemParameter.busCardOn == 1 )
	{
	    if( DeviceStatus.BusCard == 0 )
		{
			sysVPMission.STA_Bus = 0;
		}
        else if( DeviceStatus.BusCard & BC_DEV_ERR_BUSY )
        {
            sysVPMission.STA_Bus = 1;
        }
		else
		{
			sysVPMission.STA_Bus = 2;	
		}
	}
	else
	{
		sysVPMission.STA_Bus = 3;
	}
	*/
	//9.货仓1状态
	if(sysVPMission.ACDCCompressorCtr == 1)
		sysVPMission.ESTA_Tem1 = 1;
	else
		sysVPMission.ESTA_Tem1 = 0;
	//10.货仓1温度tem1
	if( SystemParameter.ACDCModule == 1 )
	{
        if( DeviceStatus.ACDCModule == 0x00 )
		{
			sysVPMission.E_Tem1 = 0xfd;//0xfd表示温度无意义;by gzz 20110721
		}
		else
		{
			sysVPMission.E_Tem1 = 0xff;//故障
		}
	}
	else
	{  
		sysVPMission.E_Tem1 = 0xfe;//不存在此货仓
	}
	//11.货仓2,3,4状态
	sysVPMission.ESTA_Tem2 = 3;//不存在此货仓
	//12，货仓2,3,4温度tem2,表示
	sysVPMission.E_Tem2 = 0xfe;//不存在此货仓

    //13交易时间
	sysVPMission.Payout_Time = SystemParameter.tradeTime;

	//==============================================================
//TAB_VPM_STATUS_RETRY:
    flag = VPMsgPackSend( VP_STATUS_RPT, 0, 1);  //1-0, not need ACK
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	/*
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		if( VPBusFrameUnPack() )
		{
		    sysVPMission.comErrNum = 0;
			goto TAB_VPM_STATUS_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_VPM_STATUS_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//? 
        return VP_ERR_COM;
	}
TAB_VPM_STATUS_COM_OK:
    switch( sysVPMission.receive.msgType )
	{
		default:
		    break;
	}
	*/
	return VP_ERR_NULL;

}

unsigned char VPMission_ColumnSta_RPT( void )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;  
	
    retry = VP_COM_RETRY;
	//--------------------------------------------
    for(i=0; i<COLUMN_NUM_SET+1; i++)
	{
		sysVPMission.send.msg[i] = 0;
	}
    for(i=0; i<COLUMN_NUM_SET; i++)
	{
        if( !(GoodsWaySetVal[i].WayState & 0x01) )
        {
		    sysVPMission.send.msg[i+1] |= 0x40;   //Error flag Added by Andy 2011.5.5	
        }
		if( GoodsWaySetVal[i].WayState & 0x0A )
 		{
			sysVPMission.send.msg[i+1] |= 0x40;   //Error flag 
		}
		/*
        if( ( GoodsWaySetVal[i].WayState & 0x04 )&&( GoodsWaySetVal[i].GoodsCurrentSum >= 1 )  )
        {
            sysVPMission.send.msg[i] |= 0x80;    
        }
        */
		sysVPMission.send.msg[i+1] += GoodsWaySetVal[i].GoodsCurrentSum & 0x3F; 
	}
	//============================================
//TAB_VPM_COLUMNSTA_RETRY:	
	flag = VPMsgPackSend( VP_HUODAO_RPT, 0, 1);   //Not need ACK
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	/*
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		if( VPBusFrameUnPack() )
		{
			goto TAB_VPM_COLUMNSTA_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_VPM_COLUMNSTA_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//   
        return VP_ERR_COM;
	}
TAB_VPM_COLUMNSTA_COM_OK:
    switch( sysVPMission.receive.msgType )
	{
		default:
		    break;
	}
	*/
	return VP_ERR_NULL;

}

unsigned char VPMission_Card_RPT( void )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;  
    retry = VP_COM_RETRY;
    //1-0: button message, not need ACK
	flag = VPMsgPackSend( VP_CARD_RPT, 0, 1);   
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	return VP_ERR_NULL;

}

//120419 by cq TotalSell
unsigned char VPMission_Info_RPT( unsigned char type )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0; 
	#ifdef   _SHUPING_
			unsigned char unuseType[14] = {1,2,4,5,7,8,9,11,18,19,20,21,22,254};//未回复的type类型
		    retry = VP_COM_RETRY;	
			sysVPMission.infoType = type;	
			for(i=0;i<14;i++)
			{
				if(sysVPMission.infoType == unuseType[i])
					break;
			}
			if(i >= 14)	
			{
			    //1-0: button message, not need ACK
				flag = VPMsgPackSend( VP_INFO_RPT, 0, 1);   
			    if( flag != VP_ERR_NULL )
			    {
					return VP_ERR_PAR;
				}
			}
	#else
			unsigned char unuseType[13] = {1,2,4,5,7,8,9,18,19,20,21,22,254};//未回复的type类型
		    retry = VP_COM_RETRY;	
			sysVPMission.infoType = type;	
			for(i=0;i<13;i++)
			{
				if(sysVPMission.infoType == unuseType[i])
					break;
			}
			if(i >= 13)	
			{
			    //1-0: button message, not need ACK
				flag = VPMsgPackSend( VP_INFO_RPT, 0, 1);   
			    if( flag != VP_ERR_NULL )
			    {
					return VP_ERR_PAR;
				}
			}
	#endif
	//else
	//{
	//	VPMsgPackSend( VP_NAK_RPT, 0, 0  );	 //1, 0
	//}
	return VP_ERR_NULL;
}


unsigned char VPMission_Act_RPT( unsigned char type, unsigned char value )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;
    
	//
    retry = VP_COM_RETRY;
//TAB_VPM_BUTTON_RETRY:
    //-------------------------------------------
    sysVPMission.ACT_Type   = type;
	sysVPMission.ACT_Value  = value;    
	//===========================================
    //1-0: button message, not need ACK
	flag = VPMsgPackSend( VP_ACTION_RPT, 0, 1);   
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	return VP_ERR_NULL;
}


unsigned char VPMission_OfflineData_RPT( void )
{
	unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

	if(sysVPMission.offLineFlag == 1)
	{
		return VP_ERR_NULL;
	}
	//
	retry = VP_COM_RETRY;
TAB_VPM_OFFDATA_RETRY:
	//-------------------------------------------
	//sysVPMission.payInDev = dev;
	//sysVPMission.payInMoney = money;
	//===========================================
	flag = VPMsgPackSend( VP_OFFLINEDATA_RPT, 1, 1);
	if( flag != VP_ERR_NULL )
	{
		return VP_ERR_PAR;
	}
	sysVPMission.msTimer2 = VP_TIME_OUT;
	while( sysVPMission.msTimer2 )
	{
		if( VPBusFrameUnPack() )
		{
			sysVPMission.comErrNum = 0;
			if(sysVPMission.offLineFlag == 1)
			{
				sysVPMission.offLineFlag = 0;
				VPMission_Act_RPT(VP_ACT_ONLINE,0);
			}
			goto TAB_VPM_OFFDATA_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_VPM_OFFDATA_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//-----------------------------------
		//3 次PC仍无响应，则跳过该消息，同时打印“PAYIN_RPT Err, device=类型,val=金额”
		if((  sysVPMission.comErrNum >= VP_TIME_OUNT_NUM )&&(sysVPMission.offLineFlag != 1) )
		{
			sysVPMission.offLineFlag = 1;
		}
		return VP_ERR_COM;
	}
TAB_VPM_OFFDATA_COM_OK:
	switch( sysVPMission.receive.msgType )
	{
		default:
			break;
	}
	return VP_ERR_NULL;
}




unsigned char VPMission_Poll( void )
{
    unsigned char retry = 0;
	unsigned char i = 0;
	unsigned char flag = 0;

	if(sysVPMission.offLineFlag == 1)
	{		
		if(sysVPMission.sTimerOffLine < 3)
		{	
			sysVPMission.sTimerOffLine++;
			return VP_ERR_NULL;
		}
		sysVPMission.sTimerOffLine = 0;
	}	

	if(sysVPMission.offLineFlag == 0)
	{
		if(sysVPMission.sTimerHeart == 0)
		{
			sysVPMission.sTimerHeart = 10;
			VPMission_Act_RPT(VP_ACT_HEART,0);
			WaitForWork(1000,NULL);
		}
	}
	if( ( sysVPMission.vendCmd == 1 )||(sysVPMission.changeCmd == 1)||(sysVPMission.costCmd == 1)||(sysVPMission.returnCmd == 1) )
		return VP_ERR_NULL;
	
    retry = VP_COM_RETRY;
	//DisplayStr( 0, 0, 1, "1", 1 );  
	//WaitForWork(2000,NULL);
TAB_VPM_P_RETRY:
	flag = VPMsgPackSend( VP_POLL, 1, 1);
	//DisplayStr( 0, 0, 1, "2", 1 );  
	//WaitForWork(2000,NULL);
    if( flag != VP_ERR_NULL )
    {
		return VP_ERR_PAR;
	}
	sysVPMission.msTimer2 = VP_TIME_POLL;
	while( sysVPMission.msTimer2 )
	{
		//DisplayStr( 0, 0, 1, "System 1", 10 );  
		//WaitForWork(2000,NULL);
		if( VPBusFrameUnPack() )
		{
		    sysVPMission.comErrNum = 0;
			if(sysVPMission.offLineFlag == 1)
			{
				sysVPMission.offLineFlag = 0;
				VPMission_Act_RPT(VP_ACT_ONLINE,0);
			}
			goto TAB_VPM_P_COM_OK;
		}
	}
	if( retry-- )
	{
		goto TAB_VPM_P_RETRY;
	}
	else
	{
		sysVPMission.comErrNum++;
		//
		if((  sysVPMission.comErrNum >= VP_TIME_OUNT_NUM )&&(sysVPMission.offLineFlag != 1) )
		{
			sysVPMission.offLineFlag = 1;
		}
        return VP_ERR_COM;
	}
TAB_VPM_P_COM_OK:
	//DisplayStr( 0, 0, 1, "System 2", 10 );  
	//WaitForWork(2000,NULL);
    switch( sysVPMission.receive.msgType )
	{
	    case VP_INITIAL_OK:
		    VP_CMD_Init_OK();
			break;
		case VP_VENDOUT_IND:
		    VP_CMD_Vendout();
		    break;
		case VP_RESET_IND:
		    VP_CMD_Reset();
		    break;
		case VP_CONTROL_IND:
		    VP_CMD_Control();
		    break;
		case VP_GET_STATUS:
		    VP_CMD_GetStatus();
		    break;
		case VP_HOUDAO_IND:
		    VP_CMD_ColumnPar();
			//if( sysVPMission.sysStatus & VPM_STA_PCPAROK )
			{
				//UpdateGoodsMatrixStatus( 0xff );
               // VP_Update_ColumnGoodsPar();
			}
		    break;
		case VP_HUODAO_SET_IND: 
			VP_CMD_ColumnNo();			
			break;	
		case VP_POSITION_IND:
		    VP_CMD_GoodsPos();			
			break;	
		case VP_SALEPRICE_IND:
			//VP_CMD_GoodsPar();
			//if( sysVPMission.sysStatus & VPM_STA_PCPAROK )
			{
				//UpdateGoodsMatrixStatus( 0xff );
                VP_Update_ColumnGoodsPar();
				SaveGoodsSet();
				WaitForWork( 100, NULL );
			}
			break;				
	    case VP_PAYOUT_IND:
		    VP_CMD_Payout();
		    break;
        case VP_GET_HUODAO:
            VP_CMD_GetColumnSta();
            break;
        case VP_COST_IND://添加扣款函数;by gzz 20110823
		    VP_CMD_Cost();
		    break;
		case VP_GETINFO_IND://120419 by cq TotalSell 			
			VPMission_Info_RPT(sysVPMission.receive.msg[0]);
			break;	
		//case VP_SALETIME_IND:
		//	VPMission_Info_RPT();
		//	break;			
		case VP_GET_SETUP_IND: 
			//VPMission_Info_RPT();
			//DisplayStr( 0, 0, 1, "1", 1 );  
			//WaitForWork(2000,NULL);
			VPMission_Setup_RPT();
			break;	
		case VP_GET_OFFLINEDATA_IND:
			VPMission_OfflineData_RPT();
			break;	
		default:
		    break;
	}
	return VP_ERR_NULL;
}

unsigned char VPMission_Init( void )
{
    unsigned char i = 0;
	unsigned char j = 0,time=0;
	unsigned char result = 0;
	
   
#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "系统\xd5\xfd在启动, 请稍等", 20 );   
	ClearDisplayLine( 2 );	
#else		
	DisplayStr( 0, 0, 1, "System reset...wait!", 20 );  
	ClearDisplayLine( 2 );	
#endif	
	VPSerialInit();

	
    //2.disable the cash or cashless device
    DisableBillDev();

	sysVPMission.sel1ReadyLed = 0x00;
    sysVPMission.sel1ErrLed   = 0xff;
    sysVPMission.sel2ReadyLed = 0x00;
    sysVPMission.sel2ErrLed   = 0xff;
    sysVPMission.sel3ReadyLed = 0x00;
    sysVPMission.sel3ErrLed   = 0xff;
    GetSelectionState( 1, &Selection1 );
	GetSelectionState( 2, &Selection2 );
	GetSelectionState( 3, &Selection3 );

    
	


	//120419 by cq TotalSell
	//result = VPMission_Startup_RPT_1();
	//if( result == VP_ERR_NULL)
		//InitAllCounter_1();	

	//1.startup
//TAB_VPM_INIT_STARTUP:
	//do
	//{
    //	result = VPMission_Startup_RPT();
	//}while( result != VP_ERR_NULL );
    
	//2.setup
	//result = VPMission_Setup_RPT();
	//if( result != VP_ERR_NULL ) goto TAB_VPM_INIT_STARTUP;

    /*
	//3.wait goods set
	result = VPMission_Poll();
    if( result != VP_ERR_NULL ) goto TAB_VPM_INIT_STARTUP;	
	//4.wait column set
	result = VPMission_Poll();
	if( result != VP_ERR_NULL ) goto TAB_VPM_INIT_STARTUP;
	//5.wait initial ok
	result = VPMission_Poll();
	if( result != VP_ERR_NULL ) goto TAB_VPM_INIT_STARTUP;
    if( (sysVPMission.sysStatus & VPM_STA_GOODSPAR)&&(sysVPMission.sysStatus & VPM_STA_COLUMNPAR) )
	{
		VP_Update_ColumnGoodsPar();
        sysVPMission.sysStatus &= ~VPM_STA_COLUMNPAR;
        sysVPMission.sysStatus &= ~VPM_STA_GOODSPAR;
        sysVPMission.VPMode = 1;
	}
    else
    {
    	goto TAB_VPM_INIT_STARTUP;
    }
    */
        
    sysVPMission.resetCmd = 0;
    //启动不成功，重试三次;by gzz 20111025
    i=0;
    if(( SystemParameter.BillNo == 1 )&&( sysITLMission.ITLSet == 1 ))
    {
        do
        {
    		MDBMission_Bill_Init();
            i++;             
        }
        while((sysMDBMission.billDeviceStatus!=0)&&(i<3));
    }

    i=0;
	//if( SystemParameter.BillNo == 1 )
	{
	    do
	    {
		    MDBMission_Coin_Init();
		    i++;         
	    }
	    while((sysMDBMission.coinDeviceStatus!=0)&&(i<3));
	}
	sysVPMission.billCoinEnable = 1;



	VPMission_Act_RPT(VP_ACT_ONLINE,0);
	sysVPMission.offLineFlag = 0;
	sysVPMission.sysStatus = 0;  
	while( (sysVPMission.VPInit == 0)&&(sysVPMission.offLineFlag == 0)&&(time<5) )
    {
	    
    	result = VPMission_Poll();
        //if( result == VP_ERR_NULL ) break;//goto TAB_VPM_INIT_STARTUP;
        /*
        if( (sysVPMission.sysStatus & VPM_STA_INITOK)&&(sysVPMission.sysStatus & VPM_STA_GOODSPAR)&&(sysVPMission.sysStatus & VPM_STA_COLUMNPAR) )
        {
        	VP_Update_ColumnGoodsPar();
        	sysVPMission.sysStatus &= ~VPM_STA_COLUMNPAR;
        	sysVPMission.sysStatus &= ~VPM_STA_GOODSPAR;
            sysVPMission.sysStatus &= ~VPM_STA_INITOK;
        	sysVPMission.VPMode = 1;
            break;
        }
        */
        if( (sysVPMission.sysStatus & VPM_STA_GOODSPAR)&&(sysVPMission.sysStatus & VPM_STA_COLUMNPAR) )
		{
        	//VP_Update_ColumnGoodsPar();
        	sysVPMission.sysStatus &= ~VPM_STA_COLUMNPAR;
        	sysVPMission.sysStatus &= ~VPM_STA_GOODSPAR;
            //sysVPMission.sysStatus &= ~VPM_STA_INITOK;
        	sysVPMission.VPMode = 1;
			sysVPMission.VPInit = 1;
            break;
        }
		time++;
		WaitForWork(2000,NULL);

    }
	sysVPMission.sysStatus &= ~VPM_STA_COLUMNPAR;
    sysVPMission.sysStatus &= ~VPM_STA_GOODSPAR;
	sysVPMission.VPMode = 1;

    sysVPMission.dspUpdateFlag = 1;
    sysVPMission.dspTimer1  = VP_DSP_TIME1;
	sysVPMission.sysStatus |= VPM_STA_PCPAROK;

	
    //Open the watch dog
    WatchDogInit();
    //isShowLcd = 1;
	DeviceStatus.Selection[0] = 0;
	DeviceStatus.Selection[1] = 0;
	DeviceStatus.Selection[2] = 0;
	return VP_ERR_NULL;
}

//Flag=1更新主机参数，Flag=2更新从机参数;by gzz 20110509
unsigned char VPAddSingleColGoods( unsigned char col, unsigned char num, unsigned char oldNum, unsigned char Flag )
{   
    u_char xdata i    = 0;
	u_char xdata key  = 0;
	u_char xdata flag = 0;

	/*
#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "    你确定更改吗?", 17 );   
	DisplayStr( 0, 1, 1, "确定ENTER 取消CANCEL", 20 );	
#else		
	DisplayStr( 0, 0, 1, "   Are you sure ?", 17 );  
	DisplayStr( 0, 1, 1, "   ENTER   CANCEL", 17 );	
#endif													
	while( 1 )
	{
		key = GetKey();
		if( ( key == KEY_CANCEL ) || (key == KEY_SUBMIT) )
			break;
		WaitForWork( 100, NULL );	
	}
	if( key == KEY_CANCEL ) 
	{
		if(Flag == 1)
	    	GoodsWaySetVal[col].GoodsCurrentSum = oldNum;
		return 0;
	}
	*/

	//Display the message
#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "\xd5\xfd在更改商品余量...", 19 );    
	ClearDisplayLine( 2 );	
#else		
	DisplayStr( 0, 0, 1, "Please wait...", 14 );    
	ClearDisplayLine( 2 );
#endif
    WaitForWork( 200, NULL );		//1000, 500, 200
    while( 1 )
    {
		if(Flag == 1)
			flag = VPMission_Admin_RPT( VP_ADMIN_GOODSADDCOL, col+1, num );
		else if(Flag == 2)
			flag = VPMission_Admin_RPT( VP_ADMIN_GOODSBUHUO, col+1, num );

		if( (flag==VP_ERR_NULL)||(flag==VP_ACK)||(flag==VP_NAK) ) break;
        if( flag == VP_ERR_COM ) break;
        WaitForWork( 500, NULL );
	}

    if( flag == VP_ERR_COM )
	{
		changeFailedBeep();
    	#ifdef _CHINA_
			DisplayStr( 0, 0, 1, "商品余量更改失败!", 17 );    
			ClearDisplayLine( 2 );	
		#else		
			DisplayStr( 0, 0, 1, "Operate fail!", 13 );    
			ClearDisplayLine( 2 );
		#endif
		if(Flag == 1)
        	GoodsWaySetVal[col].GoodsCurrentSum = oldNum; 
		WaitForWork( 2000, NULL );
	}
	else if( sysVPMission.receive.msgType == VP_ACK )
	{
		/*
        //by gzz 20120223
	    VPMission_Request(1);
	    sysVPMission.sysStatus &= ~VPM_STA_COLUMNPAR;
	    while( 1 )
	    {
			flag = VPMission_Poll();
			if( flag == VP_ERR_COM ) break;
			if( (sysVPMission.sysStatus & VPM_STA_COLUMNPAR) )
		    {
				VP_Update_ColumnGoodsPar();
				break;
		    }
	        WaitForWork( 500, NULL );
		}
		*/
		
	    //GoodsSetSave = 1;
		//if(Flag == 1)
		{
			GoodsSetSave = 1;
			//VP_Update_ColumnGoodsPar();	

		/*
#ifdef _CHINA_
			DisplayStr( 0, 0, 1, "商品余量更改成功!", 17 );    
			ClearDisplayLine( 2 );	
#else		
			DisplayStr( 0, 0, 1, "Operate successfully", 20 );    
			ClearDisplayLine( 2 );
#endif*/
		}
		
    }
	else
	{
		//changeFailedBeep();
		#ifdef _CHINA_
			DisplayStr( 0, 0, 1, "商品余量更改失败!", 17 );    
			ClearDisplayLine( 2 );	
		#else		
			DisplayStr( 0, 0, 1, "Operate fail!", 13 );    
			ClearDisplayLine( 2 );
		#endif
		if(Flag == 1)
        	GoodsWaySetVal[col].GoodsCurrentSum = oldNum;
		WaitForWork( 2000, NULL );
	}
    //WaitForWork( 3000, NULL );	
	return 0;

}

unsigned char VPChangeColPrice( unsigned char col, unsigned int price, unsigned int oldPrice )
{   
    u_char xdata i    = 0;
	u_char xdata key  = 0;
	u_char xdata flag = 0;

#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "    你确定更改吗?", 17 );   
	DisplayStr( 0, 1, 1, "确定ENTER 取消CANCEL", 20 );	
#else		
	DisplayStr( 0, 0, 1, "   Are you sure ?", 17 );  
	DisplayStr( 0, 1, 1, "   ENTER   CANCEL", 17 );	
#endif													
	while( 1 )
	{
		key = GetKey();
		if( ( key == KEY_CANCEL ) || (key == KEY_SUBMIT) )
			break;
		WaitForWork( 100, NULL );	
	}
	if( key == KEY_CANCEL ) 
	{
	    GoodsWaySetVal[col].Price = oldPrice;
		return 0;
	}
	//Display the message
#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "\xd5\xfd在更改商品价格...", 19 );    
	ClearDisplayLine( 2 );	
#else		
	DisplayStr( 0, 0, 1, "Please wait...", 14 );    
	ClearDisplayLine( 2 );
#endif
    WaitForWork( 200, NULL );		//1000-200
    while( 1 )
    {
		flag = VPMission_Admin_RPT( VP_ADMIN_COL_PRICE, col+1, price );
        if( (flag==VP_ERR_NULL)||(flag==VP_ACK)||(flag==VP_NAK) ) break;
        if( flag == VP_ERR_COM ) break;
        WaitForWork( 500, NULL );
	}  
 
	if( flag == VP_ERR_COM )
	{
	    #ifdef _CHINA_
			DisplayStr( 0, 0, 1, "商品价格更改失败!", 17 );    
			ClearDisplayLine( 2 );	
		#else		
			DisplayStr( 0, 0, 1, "Operate fail!", 13 );    
			ClearDisplayLine( 2 );
		#endif
        GoodsWaySetVal[col].Price = oldPrice;
	}
    else if( sysVPMission.receive.msgType == VP_ACK )
	{
		/*
        VPMission_Request(1);
	    sysVPMission.sysStatus &= ~VPM_STA_COLUMNPAR;
	    sysVPMission.sysStatus &= ~VPM_STA_GOODSPAR;
	    while( 1 )
	    {
			VPMission_Poll();
			if( (sysVPMission.sysStatus & VPM_STA_GOODSPAR)&&(sysVPMission.sysStatus & VPM_STA_COLUMNPAR) )
		    {
				VP_Update_ColumnGoodsPar();
				break;
		    }
	        WaitForWork( 500, NULL );
		}
		*/
	    GoodsSetSave = 1;
		#ifdef _CHINA_
			DisplayStr( 0, 0, 1, "商品价格更改成功!", 17 );    
			ClearDisplayLine( 2 );	
		#else		
			DisplayStr( 0, 0, 1, "Operate successfully", 20 );    
			ClearDisplayLine( 2 );
		#endif	           
    }
	else
	{
		#ifdef _CHINA_
			DisplayStr( 0, 0, 1, "商品价格更改失败!", 17 );    
			ClearDisplayLine( 2 );	
		#else		
			DisplayStr( 0, 0, 1, "Operate fail!", 13 );    
			ClearDisplayLine( 2 );
		#endif
        GoodsWaySetVal[col].Price = oldPrice;
	}
    WaitForWork( 3000, NULL );	
	return 0;

}

unsigned char VPAddAllColGoods( void )
{   
    u_char xdata i    = 0;
	u_char xdata key  = 0;
	u_char xdata flag = 0;
	u_char xdata tryFlag = 3;

#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "    你确定添满吗?", 17 );   
	DisplayStr( 0, 1, 1, "确定ENTER 取消CANCEL", 20 );	
#else		
	DisplayStr( 0, 0, 1, "   Are you sure ?", 17 );  
	DisplayStr( 0, 1, 1, "   ENTER   CANCEL", 17 );	
#endif													
	while( 1 )
	{
		key = GetKey();
		if( ( key == KEY_CANCEL ) || (key == KEY_SUBMIT) )
			break;
		WaitForWork( 100, NULL );	
		if(sysVPMission.msActTimer == 0)
		{
			sysVPMission.msActTimer = 100;
			VPMission_Act_RPT(VP_ACT_ADMIN,VP_ACT_ENTERADMIN);
		}
	}
	if( key == KEY_CANCEL ) return 0;
	//---------------------------------------------------------------------------------
    /*						
	for( key = 0; key < GOODSWAYNUM; key ++ )
	{	
		if( InputGoodsWayList[key].UseState != 1 )
        {
			continue;
        }
		else
		{
			if( InputGoodsWayList[key].GoodsWayNo == GoodsWaySetVal[key].WayNo )
			{
                //--------------------------------------------------------------------
                if( SystemParameter.GOCOn == 0x01 )
                {
                	GoodsWaySetVal[key].WayState &= 0xfb;
                }
                else
				{
					GoodsWaySetVal[key].GoodsCurrentSum = InputGoodsWayList[key].GoodsMax;
				    if( ( GoodsWaySetVal[key].WayState & 0x04 ) == 0x04 )
						GoodsWaySetVal[key].WayState &= 0xfb;
				}
                //====================================================================
			}
			else
            {
				GoodsWaySetVal[key].GoodsCurrentSum = 0;
            }
		}								
	}
    */
	//Display the message
#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "\xd5\xfd在添满...", 11 );    
	ClearDisplayLine( 2 );	
#else		
	DisplayStr( 0, 0, 1, "Please waiting...", 17 );    
	ClearDisplayLine( 2 );
#endif
    WaitForWork( 1000, NULL );	
	sysVPMission.sysStatus &= ~VPM_STA_COLUMNPAR;    
    //sysVPMission.requestFlag = 1;
	//VPMission_Request(1);    
    while( tryFlag )
    {
    	tryFlag--;
		//flag = VPMission_Poll();
		VPMission_Admin_RPT( VP_ADMIN_GOODSADDALL, 0, 0);

		//if( flag == VP_ERR_COM ) break;
		//DisplayStr( 0, 0, 1, "03", 2 );  
		//WaitForWork(2000,NULL);

		if( /*(sysVPMission.sysStatus & VPM_STA_GOODSPAR)&&*/ (sysVPMission.sysStatus & VPM_STA_COLUMNPAR) )
	    {
			//VP_Update_ColumnGoodsPar();
			//DisplayStr( 0, 0, 1, "05", 2 );  
			//WaitForWork(2000,NULL);
			break;
	    }
		//DisplayStr( 0, 0, 1, "04", 2 );  
		//WaitForWork(2000,NULL);
        WaitForWork( 1000, NULL );
	}

    if( tryFlag == 0  )
	{
		changeFailedBeep();
	#ifdef _CHINA_
		DisplayStr( 0, 0, 1, "添满操作失败!", 13 );    
		ClearDisplayLine( 2 );	
	#else		
		DisplayStr( 0, 0, 1, "Operate fail!", 13 );    
		ClearDisplayLine( 2 );
	#endif

	}
	else
	{
	    GoodsSetSave = 1;
	#ifdef _CHINA_
		DisplayStr( 0, 0, 1, "添满操作成功!", 13 );    
		ClearDisplayLine( 2 );	
	#else		
		DisplayStr( 0, 0, 1, "Operate successfully", 20 );    
		ClearDisplayLine( 2 );
	#endif
    }
    WaitForWork( 3000, NULL );
		
	return 0;

}

//填满层架货道;by gzz 20110429
unsigned char VPAddLayerColGoods( void )
{
	u_char xdata i    = 0;
	u_char xdata key  = 0;
	u_char xdata flag = 0;
	u_char xdata RackNo = 0;
    u_char xdata Tempstr[8];
    u_char xdata len = 0;
	u_char xdata tryFlag = 3;

    ClearKey();
	ClearDisplayLine( 1 );
	ClearDisplayLine( 2 );
    while( 1 )
	{		
		//ClearDisplayLine( 1 );
		//ClearDisplayLine( 2 );		
		len = 1;
		memset( Tempstr,0, sizeof( Tempstr ) );	
	 #ifdef _CHINA_
	 	key = GetLine( "输入层架编号:", 13 , 0, Tempstr, &len );
	 #else
		key = GetLine( "Input Tray code:", 16 , 0, Tempstr, &len );
	 #endif
		WaitForWork( 500, NULL );	
		ClearKey();				
		if( key == 1 )//Enter	
		{			
			DisplayCursorState( 0, 0, 0, 0, 1 );			
			for( i = 0; i < len; i ++ )
			{
				if( ( Tempstr[i] >= 0 ) && ( Tempstr[i] <= 9 ) )
				{
					RackNo = Tempstr[i];				
					break;
				}
			}				
			ClearKey();			
			if( ( RackNo <= 0 ) || ( RackNo > 6 ))
	 	    {
				#ifdef _CHINA_
					DisplayStr( 0, 0, 1, "编号输入错误", 12 );
					DisplayStr( 0, 1, 1, "请输入其它编号", 14 );
				#else
					DisplayStr( 0, 0, 1, "Input code error", 16 );
					DisplayStr( 0, 1, 1, "Input other code", 16 );				
				#endif					
				WaitForWork( 2000, NULL );
				ClearDisplayLine( 1 );
		        ClearDisplayLine( 2 );
				continue;
			}
			else
				break;
	    }
		else
 	    {
			return 0;
		}
    } 
	ClearKey();	
#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "你确定添满此层架吗?", 19 );   
	DisplayStr( 0, 1, 1, "确定ENTER 取消CANCEL", 20 );	
#else		
	DisplayStr( 0, 0, 1, "   Are you sure ?", 17 );  
	DisplayStr( 0, 1, 1, "   ENTER   CANCEL", 17 );	
#endif													
	while( 1 )
	{
		key = GetKey();
		if( ( key == KEY_CANCEL ) || (key == KEY_SUBMIT) )
			break;
		WaitForWork( 100, NULL );	
		if(sysVPMission.msActTimer == 0)
		{
			sysVPMission.msActTimer = 100;
			VPMission_Act_RPT(VP_ACT_ADMIN,VP_ACT_ENTERADMIN);
		}
	}
	if( key == KEY_CANCEL ) return 0;
#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "\xd5\xfd在添满当前层架...", 19 );    
	ClearDisplayLine( 2 );	
#else		
	DisplayStr( 0, 0, 1, "Adding the tray...", 18 );    
	ClearDisplayLine( 2 );
#endif
    WaitForWork( 1000, NULL );	
    //--------------------------------------------------------------
    sysVPMission.sysStatus &= ~VPM_STA_COLUMNPAR;    
    //sysVPMission.requestFlag = 1;
	//VPMission_Request(1);    
    while( tryFlag )
    {    	
    	tryFlag--;
		//flag = VPMission_Poll();
		VPMission_Admin_RPT( VP_ADMIN_GOODSADDTRAY, RackNo, 0);
		//if( flag == VP_ERR_COM ) break;
		if( /*(sysVPMission.sysStatus & VPM_STA_GOODSPAR)&&*/ (sysVPMission.sysStatus & VPM_STA_COLUMNPAR) )
	    {
			//VP_Update_ColumnGoodsPar();
			break;
	    }
        WaitForWork( 500, NULL );
	}
	//==============================================================
	if( tryFlag == 0  )
	{
		changeFailedBeep();
	#ifdef _CHINA_
		DisplayStr( 0, 0, 1, "层架添满操作失败!", 17 );    
		ClearDisplayLine( 2 );	
	#else		
		DisplayStr( 0, 0, 1, "Add the tray fail!", 18 );    
		ClearDisplayLine( 2 );
	#endif

	}
	else
	{
	    GoodsSetSave = 1;
	#ifdef _CHINA_
		DisplayStr( 0, 0, 1, "层架添满操作成功!", 17 );    
		ClearDisplayLine( 2 );	
	#else		
		DisplayStr( 0, 0, 1, "Operate successfully", 20 );    
		ClearDisplayLine( 2 );
	#endif
    }
    WaitForWork( 3000, NULL );
	return 0;

}

//by gzz 20110506
unsigned char VPAddChanges( uchar xdata testDev )
{  
    u_char xdata i    = 0;
	u_char xdata key  = 0;
	u_char xdata flag = 0;
	u_char xdata DevStr[10];
	u_char xdata str[20];
	u_char xdata len = 0;

	if(testDev == 1)
	{
		#ifdef _CHINA_
			sprintf( DevStr, "%s", "补硬币" );
		#else
			sprintf( DevStr, "%s", "Coin" );
		#endif		
	}
	else if(testDev == 2)
	{
		#ifdef _CHINA_
			sprintf( DevStr, "%s", "取纸币" );
		#else
			sprintf( DevStr, "%s", "Cash" );
		#endif		
	}

#ifdef _CHINA_ 
    len = sprintf( str, "  你确定%s吗?", DevStr );     
	DisplayStr( 0, 0, 1, str, len );   
	DisplayStr( 0, 1, 1, "确定ENTER 取消CANCEL", 20 );	
#else		
	DisplayStr( 0, 0, 1, "   Are you sure ?", 17 );  
	DisplayStr( 0, 1, 1, "   ENTER   CANCEL", 17 );	
#endif	
												
	while( 1 )
	{
		key = GetKey();
		if( ( key == KEY_CANCEL ) || (key == KEY_SUBMIT) )
			break;
		WaitForWork( 100, NULL );	
		if(sysVPMission.msActTimer == 0)
		{
			sysVPMission.msActTimer = 100;
			VPMission_Act_RPT(VP_ACT_ADMIN,VP_ACT_ENTERADMIN);
		}
	}
	if( key == KEY_CANCEL ) return 0;
	//Display the message
#ifdef _CHINA_	 
    len = sprintf( str, "  \xd5\xfd在%s", DevStr );
	DisplayStr( 0, 0, 1, str, len ); 
	ClearDisplayLine( 2 );	
#else		
	DisplayStr( 0, 0, 1, "Please wait...", 14 );    
	ClearDisplayLine( 2 );
#endif
    WaitForWork( 1000, NULL );		
    while( 1 )
    {
		if(testDev == 1)
		{
			flag = VPMission_Admin_RPT( VP_ADMIN_CHANGEADD, 0, 0 );
		}
		else if(testDev == 2)
		{
			flag = VPMission_Admin_RPT( VP_ADMIN_GETBILL, 0, 0 );
		}
		if(sysVPMission.offLineFlag == 1)
		{
			flag =VP_ERR_COM;
		}
		if( (flag==VP_ERR_NULL)||(flag==VP_ACK)||(flag==VP_NAK) ) break;
		if( flag == VP_ERR_COM ) break;
        WaitForWork( 500, NULL );
	}

	if( flag == VP_ERR_COM )
    {
    	changeFailedBeep();
		#ifdef _CHINA_			 
  		    len = sprintf( str, "%s失败!", DevStr );
	        DisplayStr( 0, 0, 1, str, len ); 
			ClearDisplayLine( 2 );	
		#else		
			DisplayStr( 0, 0, 1, "Operate fail!", 13 );    
			ClearDisplayLine( 2 );
		#endif
	}
    else if( sysVPMission.receive.msgType == VP_ACK )
	{
		#ifdef _CHINA_			
  		    len = sprintf( str, "%s成功!", DevStr );
	        DisplayStr( 0, 0, 1, str, len ); 
			ClearDisplayLine( 2 );	
		#else		
			DisplayStr( 0, 0, 1, "Operate successfully", 20 );    
			ClearDisplayLine( 2 );
		#endif
		/*
        if(testDev == 1)
		{				
            TradeCounter.CoinSum5jBack = TradeCounter.CoinSum5j;			
			TradeCounter.CoinSum5jID++;
            TradeCounter.CoinSum1yBack = TradeCounter.CoinSum1y;			
			TradeCounter.CoinSum1yID++;
           
            TradeCounter.Hopper1SumBack = TradeCounter.Hopper1Sum;			
			TradeCounter.Hopper1SumID++;
            TradeCounter.Hopper2SumBack = TradeCounter.Hopper2Sum;			
			TradeCounter.Hopper2SumID++;
            TradeCounter.Hopper3SumBack = TradeCounter.Hopper3Sum;			
			TradeCounter.Hopper3SumID++;
		}
		else if(testDev == 2)
		{
			TradeCounter.CashSumBack = TradeCounter.CashSum;			
			TradeCounter.CashSumID++;
		}    
		*/
        SaveTradeParam();//by gzz 20110610
    }
	else
	{
		#ifdef _CHINA_			  
 		    len = sprintf( str, "%s失败!", DevStr );
	        DisplayStr( 0, 0, 1, str, len ); 
			ClearDisplayLine( 2 );	
		#else		
			DisplayStr( 0, 0, 1, "Operate fail!", 13 );    
			ClearDisplayLine( 2 );
		#endif
	}
    WaitForWork( 3000, NULL );		
	return 0;
  
}

unsigned char VPSynGoodsCol( void )
{   
    u_char xdata i = 0;
	u_char xdata key = 0;
    u_char xdata flag = 0;

	if(sysVPMission.offLineFlag == 1)
	{	
		#ifdef _CHINA_
			DisplayStr( 0, 0, 1, "    同步失败", 12 ); 
			DisplayStr( 0, 1, 1, "                    ", 20 );	
		#else		
			DisplayStr( 0, 0, 1, "   Fail", 7 ); 
			DisplayStr( 0, 1, 1, "                    ", 20 );	
		#endif
		WaitForWork( 2000, NULL );
		changeFailedBeep();	
		return VP_ERR_NULL;
	}
#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "    你确定同步吗?", 17 );   
	DisplayStr( 0, 1, 1, "确定ENTER 取消CANCEL", 20 );	
#else		
	DisplayStr( 0, 0, 1, "   Are you sure ?", 17 );  
	DisplayStr( 0, 1, 1, "   ENTER   CANCEL", 17 );	
#endif
													
	while( 1 )
	{
		key = GetKey();
		if( ( key == KEY_CANCEL ) || (key == KEY_SUBMIT) )
			break;
		WaitForWork( 100, NULL );	
		if(sysVPMission.msActTimer == 0)
		{
			sysVPMission.msActTimer = 100;
			VPMission_Act_RPT(VP_ACT_ADMIN,VP_ACT_ENTERADMIN);
		}
	}
	if( key == KEY_CANCEL ) return 0;
	
	//Display the message
#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "\xd5\xfd在同步, 请稍等...", 19 );    
	ClearDisplayLine( 2 );	
#else		
	DisplayStr( 0, 0, 1, "Please waiting...", 17 );    
	ClearDisplayLine( 2 );
#endif
    WaitForWork( 1000, NULL );

	/*
    flag = VPMission_Admin_RPT( VP_ADMIN_GOODSCOLUMN, 0, 0);
    if( flag != VP_ERR_NULL )
    {
    	#ifdef _CHINA_
			DisplayStr( 0, 0, 1, "操作失败!", 9 );    
			ClearDisplayLine( 2 );	
		#else		
			DisplayStr( 0, 0, 1, "Operate fail!", 13 );    
			ClearDisplayLine( 2 );
		#endif
	    WaitForWork( 2000, NULL );		
	    return 0;	
    }
    */

    while( 1 )
	{
		 flag = VPMission_Admin_RPT( VP_ADMIN_GOODSCOLUMN, 0, 0);
		 if( flag == VP_ACK )  break;
		 //if(flag == VP_NAK)
		 //	changeFailedBeep();
         WaitForWork( 800, NULL );
	}
	/*
    //sysVPMission.requestFlag = 1;
	//VPMission_Request(1);
    sysVPMission.sysStatus &= ~VPM_STA_COLUMNPAR;
    sysVPMission.sysStatus &= ~VPM_STA_GOODSPAR;
    while( 1 )
    {
		VPMission_Poll();
		if( (sysVPMission.sysStatus & VPM_STA_GOODSPAR)&&(sysVPMission.sysStatus & VPM_STA_COLUMNPAR) )
	    {
			VP_Update_ColumnGoodsPar();
			break;
	    }
        WaitForWork( 500, NULL );
	}
    GoodsSetSave = 1;
    */
#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "同步操作成功!", 13 );    
	ClearDisplayLine( 2 );	
#else		
	DisplayStr( 0, 0, 1, "Operate successfully", 20 );    
	ClearDisplayLine( 2 );
#endif
    WaitForWork( 2000, NULL );		
	return 0;

}

unsigned char VPBuhuoCol( void )
{  
	u_char xdata i	  = 0;
	u_char xdata key  = 0;
	u_char xdata flag = 0;
	

#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "    你确定补货吗?", 17 );   
	DisplayStr( 0, 1, 1, "确定ENTER 取消CANCEL", 20 );	
#else		
	DisplayStr( 0, 0, 1, "   Are you sure ?", 17 );  
	DisplayStr( 0, 1, 1, "   ENTER   CANCEL", 17 );	
#endif
												
	while( 1 )
	{
		key = GetKey();
		if( ( key == KEY_CANCEL ) || (key == KEY_SUBMIT) )
			break;
		WaitForWork( 100, NULL );	
		if(sysVPMission.msActTimer == 0)
		{
			sysVPMission.msActTimer = 100;
			VPMission_Act_RPT(VP_ACT_ADMIN,VP_ACT_ENTERADMIN);
		}
	}
	if( key == KEY_CANCEL ) return 0;
	
	//Display the message
#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "\xd5\xfd在补货, 请稍等...", 19 );    
	ClearDisplayLine( 2 );	
#else		
	DisplayStr( 0, 0, 1, "Please waiting...", 17 );    
	ClearDisplayLine( 2 );
#endif
    WaitForWork( 1000, NULL );		
	while( 1 )
	{		
		
		flag = VPMission_Admin_RPT( VP_ADMIN_GOODSBUHUO, 0, 0 );
		
		if( (flag==VP_ERR_NULL)||(flag==VP_ACK)||(flag==VP_NAK) ) break;
		if( flag == VP_ERR_COM ) break;
		WaitForWork( 500, NULL );
	}

	if( flag == VP_ERR_COM )
	{
		changeFailedBeep();
	#ifdef _CHINA_
		DisplayStr( 0, 0, 1, "补货操作失败!", 13 );    
		ClearDisplayLine( 2 );	
	#else		
		DisplayStr( 0, 0, 1, "Operate successfully", 20 );    
		ClearDisplayLine( 2 );
	#endif
	}
	else if( sysVPMission.receive.msgType == VP_ACK )
	{
		WaitForWork( 500, NULL );
		VPMission_Status_RPT();
		WaitForWork( 500, NULL );
		VPMission_Info_RPT( VP_INFO_CHUHUO);
	#ifdef _CHINA_
		DisplayStr( 0, 0, 1, "补货操作成功!", 13 );    
		ClearDisplayLine( 2 );	
	#else		
		DisplayStr( 0, 0, 1, "Operate successfully", 20 );    
		ClearDisplayLine( 2 );
	#endif		
	}
	else
	{
	#ifdef _CHINA_
		DisplayStr( 0, 0, 1, "补货操作失败!", 13 );    
		ClearDisplayLine( 2 );	
	#else		
		DisplayStr( 0, 0, 1, "Operate successfully", 20 );    
		ClearDisplayLine( 2 );
	#endif
	}
	WaitForWork( 3000, NULL );		
	return 0;
  
}



unsigned char VPCountCoin( unsigned char hopperNum )
{   
    u_char xdata i = 0;
	u_char xdata key = 0;
    u_char xdata flag = 0;
	u_char xdata ret = 0;
	u_char xdata coin1flag = 0; 
    u_int  xdata coinNum   = 0;
    u_int  xdata coinMoney = 0;
    u_int  xdata coinPrice = 0;
	u_char xdata str[20];
	u_char xdata len = 0;
    struct DEVICE xdata *HopperDev;
    u_char xdata *devHopperSta;

	//if( !((hopperNum>=1)&&(hopperNum<=2)) )  return 0;
    
	if( hopperNum == 1 )
	{
		HopperDev = &Hopper1;
        coinPrice = SystemParameter.HopperCoinPrice1;
        devHopperSta = &DeviceStatus.ChangeUnit1;	
	}
	else if( hopperNum == 2 )
	{
	    HopperDev = &Hopper2;
        coinPrice = SystemParameter.HopperCoinPrice2;
        devHopperSta = &DeviceStatus.ChangeUnit1;	
	}
	else
	{
		return 0;
	}

#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "  你确定清点吗?", 15 );   
	DisplayStr( 0, 1, 1, "确定ENTER 取消CANCEL", 20 );	
#else		
	DisplayStr( 0, 0, 1, "   Are you sure?", 16 );  
	DisplayStr( 0, 1, 1, "   ENTER   CANCEL", 17 );	
#endif
													
	while( 1 )
	{
		key = GetKey();
		if( ( key == KEY_CANCEL ) || (key == KEY_SUBMIT) )
			break;
		WaitForWork( 100, NULL );	
	}
	if( key == KEY_CANCEL ) return 0;
	
	//Display the message
#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "\xd5\xfd在清点, 请稍等...", 19 );    
	ClearDisplayLine( 2 );	
#else		
	DisplayStr( 0, 0, 1, "Please waiting...", 17 );    
	ClearDisplayLine( 2 );
#endif
    //WaitForWork( 1000, NULL );
    if( HopperOutCoin( 1000, hopperNum, HopperDev ) == 0 )		
	{   
        WaitForWork( 1000, NULL );
		//查询出币结果	
		ret = 0;
		while( ret < 2 )
		{
			if( GetHardWareDeviceState( hopperNum, HopperDev ) != 1 ) //更新状态成功
			{
				if( HopperDev->State &  0x50 )   //红外线传感有问题或系统故障
					coin1flag |= 4;              //硬件故障			
				if( HopperDev->State &  0x20 )   //实际无币或光测无币
					coin1flag |= 2;              //币不足
				//--------------------------------------------------------
				if( HopperDev->State & 0x08 )    //Empty
				    coin1flag |= 8;
				//========================================================
				coinNum = HopperGetCoinOut( hopperNum );
                coinMoney = coinNum*coinPrice;
				Trace( "\n outMoney1 = %d", coinNum );		
				ret = 0;
				break;
			}
			else    //查询状态超时
			{
				ret++;
				WaitForWork( 1000, NULL );	
			}
		}
		if( ret >= 2 )  coin1flag |= 4;
	}
	else
	{
		coin1flag |= 4;
	}

    //update the hopper status
    for( i = 0; i < 3; i ++ )
	{
		if( HopperSetList[i].HopperIndex == hopperNum)					
		{
			if( coin1flag & 4 )	
			{
				//DeviceStatus.ChangeUnit1 = 1;
                *devHopperSta = 1;
				HopperSetList[i].HopperState = 2;
			}
			else if( coin1flag & 8 )                 //Empty
			{
			    //DeviceStatus.ChangeUnit1 = 4;      //Empty
                *devHopperSta = 4;
				HopperSetList[i].HopperState = 2;
			}
			else if( coin1flag & 2 ) 
			{
				//DeviceStatus.ChangeUnit1 = 2;
                *devHopperSta = 2; 
				HopperSetList[i].HopperState = 2;
			}
			break;
		}			
	}

	//display the counting result		
#ifdef _CHINA_
	DisplayStr( 0, 0, 1, "清点完毕，请核对!", 17 );
	//DisplayStr( 0, 1, 1, "硬币金额: ", 10 );
#else
	DisplayStr( 0, 0, 1, "Pls check the result", 20 );
	//DisplayStr( 0, 1, 1, "Coin:", 5 );
#endif
		
    //if( flag == 1 )
	{	
		switch( SystemParameter.curUnit )
		{
			case 1:
		#ifdef _CHINA_
				len = sprintf( str, "清点金额: %u", coinMoney );	
		#else
				len = sprintf( str, "Payout: %u", coinMoney );		
		#endif	
			break;				
			case 10:
		#ifdef _CHINA_
				len = sprintf( str, "清点金额: %u.%u", coinMoney/SystemParameter.curUnit, coinMoney%SystemParameter.curUnit );	
		#else
				len = sprintf( str, "Payout: %u.%u", coinMoney/SystemParameter.curUnit, coinMoney%SystemParameter.curUnit );		
		#endif	
			break;
			case 100:
		#ifdef _CHINA_
				len = sprintf( str, "清点金额: %u.%02u", coinMoney/SystemParameter.curUnit, coinMoney%SystemParameter.curUnit );	
		#else
				len = sprintf( str, "Payout: %u.%02u", coinMoney/SystemParameter.curUnit, coinMoney%SystemParameter.curUnit );		
		#endif	
			break;
			default:
				len = 0;
				memset( str, 0, sizeof( str ) );
			break;
		}
		if( len != 0 )
		{
			DisplayStr( 0, 1, 1, str, len );
		}
	}
    //
	while( 1 )
	{
		key = GetKey();		
		if( ( key  == KEY_SUBMIT ) || ( key  == KEY_CANCEL ) )
	    {
			break;
	    }
    }
	return 0;
}









