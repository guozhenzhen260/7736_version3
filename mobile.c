// 手机模块驱动程序
// 采用西门子m20手机模块
#define MOBILE_C
#define GPRS_C
#define SMS_C

#include "device.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "serial.h"
#include "mobile.h"
#include "timer.h"
#include "debug.h"
#include "global.h"

#define ZHKMOBILE_CONNECTTIMEOUT  	180
#define ZHKMOBILE_CMD_ACKTIMEOUT  	50
#define ZHKMOBILE_SMS_ACKTIMEOUT  	70
//存储收到的短信的最大量
#define SmsRxMaxNum	3

uchar ZhkAtCmd( uchar* AtCmd, uchar CmdFlag );
uchar ZhkAtCmdGetErr( void );
uchar xdata ZhkAtCmdBuffer[ 1024 ];
uint  xdata ZhkAtCmdBufWr;
uchar MobileInitProcess( void );
uchar ZhkMobileNullProcess( void );
void ZhkMobileSignalIn( void );	

//AT指令命令执行的结果
#define ATCMD_ERR_NULL 		0
#define ATCMD_ERR_BLOCK  	1
#define ATCMD_ERR_TIMEOUT 	2
#define ATCMD_ERR_ERR 		3
uchar xdata ZhkAtCmdErr = ATCMD_ERR_NULL;

///任务队列中操作任务定义
#define ZHKTASKCLAIM_DIAL  	0x01		// 拨号请求 
#define ZHKTASKCLAIM_HANG  	0x02		// 挂断请求 
#define ZHKTASKCLAIM_CONNECT  	0x04	// 建链请求 
#define ZHKTASKCLAIM_SHUT  	0x08		// 断链请求 
#define ZHKTASKCLAIM_TXSMSG  	0x10 	// 发送短信请求 
#define ZHKTASKCLAIM_RXTMSG  	0x40 	// 读短信息请求 
#define ZHKTASKCLAIM_QUERY  	0x80 	// 查询状态请求,在短信模式下用于查询手机模块的工作状态
uchar xdata ZhkTaskClaim;			// 手机通信任务请求

// 拨号状态
////手机模块当前执行的任务
#define MOBILE_NULL  		0x00  	// 空闲状态
#define MOBILE_DIALUP 		0x01	// 拨号中
#define MOBILE_HANGUP 		0x02	// 挂断中
#define MOBILE_TCP_CONNECT 	0x03	// 建链中
#define MOBILE_TXSHORTMSG	0x05	// 发送短信中
#define MOBILE_RXSHORTMSG  	0x06 	// 接收短信中
#define MOBILE_TCP_SHUT 	0x04	// 断链中
#define MOBILE_TXTCPMSG  	0x07 	// 正在发送TCP信息
#define MOBILE_RXTCPMSG     0x08	// 正在接收TCP信息
#define MOBILE_INIT      	0x09	// 正在初始化
#define MOBILE_QUERY      	0x0A	//查询
uchar xdata ZhkMobileCurrentTask = MOBILE_NULL;		// 手机模块状态

// 手机模块执行某个任务时的进度定义	
#define MOBILE_BEGIN 	0x00
#define MOBILE_STEP1 	0x01
#define MOBILE_STEP2 	0x02
#define MOBILE_STEP3 	0x03
#define MOBILE_STEP4 	0x04
#define MOBILE_STEP5 	0x05
#define MOBILE_STEP6 	0x06
#define MOBILE_STEP7 	0x07
#define MOBILE_STEP8 	0x08
#define MOBILE_STEP9 	0x09
#define MOBILE_STEP10 	0x0a
#define MOBILE_FINISH	0x10
#define MOBILE_TIMEOUT 	0x11
#define MOBILE_ERROR 	0x12	

///手机模块所处的通讯链路状态
#define GPRS_STATUS_BREAK  		1	//在SMS模下表示主控板与手机模块通讯断开
#define GPRS_STATUS_DIALUP  		2
#define GPRS_STATUS_CONNECTED  		3	//在SMS模下表示主控板与手机模块通讯正常
#define GPRS_STATUS_CONNECTING  	4	//在SMS模下表示主控板与手机模块正在连接
#define GPRS_STATUS_HANGUP  		5
uchar xdata ZhkGprsStatus     	= GPRS_STATUS_BREAK;

uchar xdata	ZhkGprsLastErr       	= GPRS_ERR_BLOCK;
//手机模块死机控制
#define GPRSMDeadJValue  3        //无应答次数
#define GPRSMResetValue  90      //掉电时长 
bit data GPRSModuleDeadFlag = 0;	//是否死机标志
bit data LoadPowerDeleAllSMS = 0;	//系统上电上后是否删除了所有以前的短信
uchar xdata GPRSDeadTimer = 0;	  	//无回应次数


#ifdef _MOBILE_SMS_1	
	
	#define SMS_MOBILE_STATUS_OK 1
	#define SMS_MOBILE_STATUS_BROKEN 2

	uchar xdata	SMSLastErr       	= SMS_ERR_BLOCK;	
//	uchar xdata SMSMobileStatus = SMS_MOBILE_STATUS_BROKEN;
	// 短信接收缓冲区
	struct SHORTMSG xdata ZhkSmsRxBuf[SmsRxMaxNum];
	uchar xdata ZhkSmsRxNum = 0;
	char xdata NewCommingSmsSit[16];
	// 短信发送缓冲区
	struct SHORTMSG xdata ZhkSmsTxBuf;
	uchar xdata ZhkSmsTxNum = 0;
	

	uchar ZhkMobileSmsRxProcess( void );
	uchar ZhkMobileSmsTxProcess( void );
	uchar ZhkMobileSmsGetTxMsg( struct SHORTMSG* Msg );
	uint ZhkMobileSmsUnpack( struct SHORTMSG xdata* Msg, uchar xdata* DataBuf , uint DataLen );
	uint ZhkMobileSmsPack( struct SHORTMSG xdata* Msg, uchar xdata* DataBuf );
	uint ZhkGSMToAsc( uchar xdata* GSMCode , uchar xdata* AscCode , uint len );
//	uchar ZhkAscToGSM( uchar xdata* GSMCode , uchar xdata* AscCode , uchar len );	
#endif

//公共函数
/////当空闲时处理任务队列中的任务
uchar ZhkMobileNullProcess( void )
{
	
	return 0;
}

//公共函数
////初始化手机模块
uchar MobileInitProcess( void )
{
    return 0;
}

//公共函数
///循环监控手机模块
uchar ZhkMobileProcess( void )
{	
	
	return 0;
}

uchar GprsInit( void )
{

	return 0;
}
//公共函数
///串口的信号进行，然后判断是什么信号
void ZhkMobileSignalIn( void )
{
	#define SMSHEAD  "+CMTI:"			
	#define GPRSHEAD "+IPD"
	#define GPRSCLOSE "CLOSED"
	
}

//控制模块代码初始化
uchar ZhkMobileInit( void )
{	
	
	return 0;
}

uchar SendATcommandToMobile()
{
    return 0;
}

#ifdef _MOBILE_SMS_1
	
	bit IsShortMsgIn( void )
	{
	//	Trace("\n IsShortMsgIn");
		return ! ( ZhkSmsRxNum == 0 );
	}
	bit IsMessagerBuzy( void )
	{
		return ZhkSmsTxNum;
	}
	
	bit	SMSMobileStatus()
	{
		if( SMSMobileStatus == SMS_MOBILE_STATUS_OK )
			return 1;
		else
			return 0;
	}

	uchar GetShortMsg( struct SHORTMSG* Msg )
	{
		if( Msg == 0);	
		return 0;	
	}
	
	uchar PutShortMsg( struct SHORTMSG* Msg )
	{
	    if( Msg == 0);
		ZhkTaskClaim |= ZHKTASKCLAIM_TXSMSG;
		return ZhkSmsTxBuf.len;
	}

	uchar SMSGetLastErr( void )
	{
		return SMSLastErr;
	}	
	
	uchar ZhkMobileSmsTxProcess( void )
	{
		// AT+CMGS=<length><cr><lf>
		// waitfor '>'
		// SendMsg
		// <^Z><cr><lf>
		// waitfor OK
		return 0;
	    //return MsgTxStatus;
	}

	uchar ASCToInt( uchar val )
	{
		if ((val >= '0') && (val <= '9'))
        	return  val - '0';
        return 0;
	}

	uchar ZhkMobileSmsRxProcess( void )
	{
		// AT+CMGR=n
		// getline index "+cgml" and 编号n
		// getline unpack
		// waitfor ok
		// AT+CMGD=n
		// waitfor ok
		return 	0;
	}

	// 返回值等于0表示操作成功从发送缓冲区取到短信
	// 返回值非零表示操作失败
	uchar ZhkMobileSmsGetTxMsg( struct SHORTMSG* Msg )
	{
		if ( ZhkSmsTxNum == 0) 
			return 1;
		memcpy( Msg , &ZhkSmsTxBuf, sizeof( struct SHORTMSG ) );
	//	ZhkSmsTxNum --;
		return 0;
	}
	
	//////////////////////////
	/////如果进来的短信是7位编码，则要用此函数转化为8位编码
	// GSM字符串(每个字符7位)转化为Asc字符串(每个字符8位)
	// GSMCode：GSM字符串(每个字符7位)，
	// AscCode：Asc字符串(每个字符8位)
	// len: 字符串长度，即字符个数为
	uint ZhkGSMToAsc( uchar xdata* GSMCode , uchar xdata* AscCode , uint len )
	{
	   GSMCode = 0;
	   AscCode = 0;
       len = 0;
	   return 0;
	}
		
	// GSMCode：GSM字符串(每个字符8位)，
	// MessageCode：消息字符串(每个字符8位)
	// len: 字符串长度，即字符个数
	// 返回转换结果的长度
	//此函数是把短信内容变为7位码的函数
	uint ZhkMessageToGSM( uchar xdata* GSMCode , uchar xdata* MessageCode , uint len )
	{
	    GSMCode = 0;
        MessageCode = 0;
        len = 0;
		return 0;
	}

	// 手机短信编码
	// 返回编码后去掉短信中心号码剩下部分的字符长度
	uint ZhkMobileSmsPack( struct SHORTMSG xdata* Msg, uchar xdata* DataBuf )
	{
	    Msg = 0;
		DataBuf = 0;
	    return 0;
	}

	// 返回值
	// 短信的信息长度
	uint ZhkMobileSmsUnpack( struct SHORTMSG xdata* Msg, uchar xdata* DataBuf, uint DataLen ) 
	{
	    Msg = 0;
		DataBuf = 0;
		DataLen = 0;
		return 0;
	}	
#endif

#ifdef _MOBILE_GPRS		
	uchar ZhkMobileGprsConnectProcess( void )
	{
		// AT+CIPSTART="TCP","IP","PORT"
		// waitfor ok
		// waitfor connect ok
		static uchar xdata ConnectStatus = MOBILE_BEGIN;
		uchar xdata CmdStr[ 128 ];
		uchar  data len;
		uchar  data temp;
		switch( ConnectStatus )
		{
		case MOBILE_BEGIN:	
			len = sprintf( CmdStr , "AT+CIPSTART=\"TCP\",\"%d.%d.%d.%d\",\"%ld\"\0" ,  
				( int )ZhkGprsIP[ 0 ] , ( int )ZhkGprsIP[ 1 ] , 
				( int )ZhkGprsIP[ 2 ] , ( int )ZhkGprsIP[ 3 ] , (long)ZhkGprsPort );
			ZhkAtCmd( CmdStr, 0 );
			ConnectStatus     = MOBILE_STEP1;
			break;
		case MOBILE_STEP1:			// waitfor OK
			temp = ZhkAtCmdGetErr();
			switch( temp )
			{
			case ATCMD_ERR_NULL:
				// 手机模块正确接收指令，
				ZhkMobileSecTimer = ZHKMOBILE_CONNECTTIMEOUT;
				ConnectStatus     = MOBILE_STEP2;
				break;
			case ATCMD_ERR_BLOCK:
				// 阻塞状态直接返回
				break;						
			case ATCMD_ERR_TIMEOUT:
				// 手机模块接收指令超时，
				ConnectStatus     = MOBILE_BEGIN;
				ZhkGprsStatus     = GPRS_STATUS_CONNECTING;
				ZhkGprsLastErr    = GPRS_ERR_CONNECTING;
				return MOBILE_TIMEOUT;
			case ATCMD_ERR_ERR:
				// 手机模块接收指令出错，
				ConnectStatus     = MOBILE_BEGIN;
				ZhkGprsStatus     = GPRS_STATUS_CONNECTING;
				ZhkGprsLastErr    = GPRS_ERR_CONNECTING;
				return MOBILE_ERROR;
			}
			break;	
		case MOBILE_STEP2:  			// wait for connectok
			// 阻塞状态直接返回
			if ( ZhkAtCmdBufWr == 0 )
			{
				if ( ZhkMobileSecTimer == 0 )
				{
					ConnectStatus     = MOBILE_BEGIN;
					ZhkGprsStatus     = GPRS_STATUS_CONNECTING;
					ZhkGprsLastErr    = GPRS_ERR_CONNECTING;
					return MOBILE_TIMEOUT;
				}
				 break;	
			}
			ZhkAtCmdBufWr = 0;
			// 连接成功
			if ( strcmp( ZhkAtCmdBuffer , "CONNECT OK" ) == 0 )
			{
				ConnectStatus     = MOBILE_BEGIN;
				ZhkGprsStatus     = GPRS_STATUS_CONNECTED;
				ZhkGprsLastErr    = GPRS_ERR_NULL;
				return MOBILE_FINISH;
			}
			if ( strcmp( ZhkAtCmdBuffer , "CONNECT FAIL" ) == 0 )
			{
				ConnectStatus     = MOBILE_BEGIN;
				ZhkGprsStatus     = GPRS_STATUS_CONNECTING;
				ZhkGprsLastErr    = GPRS_ERR_CONNECTING;
				return MOBILE_ERROR;
			}
			break;
		default:
			ConnectStatus = MOBILE_BEGIN;
			return MOBILE_ERROR;
		}
		return ConnectStatus;
	}
	
	uchar ZhkMobileGprsShutProcess( void )
	{
		// AT+CIPSHUT
		// waitfor shut ok or error
		static uchar xdata ShutStatus = MOBILE_BEGIN;
		uchar  data temp;
		
		switch( ShutStatus )
		{
		case MOBILE_BEGIN:	
			ZhkAtCmd( "AT+CIPSHUT", 0 );
			ShutStatus        = MOBILE_STEP1;
			break;
		case MOBILE_STEP1:			// waitfor OK
			temp = ZhkAtCmdGetErr();
			ZhkGprsMsgRxBufWr = 0;
			switch( temp )
			{
			case ATCMD_ERR_NULL:
				// 断链成功的标志
				ShutStatus        = MOBILE_BEGIN;
				ZhkGprsStatus     = GPRS_STATUS_DIALUP;
				ZhkGprsLastErr    = GPRS_ERR_NULL;
				return MOBILE_FINISH;
			case ATCMD_ERR_BLOCK:
				// 阻塞状态直接返回
				break;						
			case ATCMD_ERR_TIMEOUT:
				// 挂断超时处理
				// 超时处理
				ShutStatus        = MOBILE_BEGIN;
				ZhkGprsLastErr    = GPRS_ERR_TIMEOUT;
				return MOBILE_TIMEOUT;
			case ATCMD_ERR_ERR:
				// 断链成功的标志
				ShutStatus        = MOBILE_BEGIN;
				ZhkGprsStatus     = GPRS_STATUS_DIALUP;
				ZhkGprsLastErr    = GPRS_ERR_NULL;
				return MOBILE_FINISH;
			}
			break;
		default:
			ShutStatus        = MOBILE_BEGIN;
			return MOBILE_ERROR;
		}
		return ShutStatus;
	}

	uchar ZhkMobileGprsTxProcess( void )
	{
	
		return 0;
	}
	
	uchar ZhkMobileGprsRxProcess( void )
	{
		return 0;
	}

#endif

uchar ZhkAtCmd( uchar* AtCmd, uchar CmdFlag )
{
	if( *AtCmd == CmdFlag )  CmdFlag = 0;
	return 0;
}
uchar ZhkAtCmdGetErr( void )
{

	return ATCMD_ERR_BLOCK;
}

