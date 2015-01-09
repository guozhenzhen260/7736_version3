//#undef _DEBUG_MOBIL_DEVICE_TRACE
#include "debug.h"
#include "CommModule.h"
#include "mainflow.h"
#include "ShortMsg.h"

unsigned char data g_byCommRunningStatus = STA_COMM_MODULE_INIT;

#ifdef _MOBILE_SMS_1
	struct SMSModuleInfo xdata g_SMSCommModuleInfo;
#endif

//uchar ReTryTime = 0;

//运行标志，用bit位指示相关的运行状态
//bit0:为1表示正在进行发送短信测试
uchar xdata RunFlag = 0;

void DoCommunication()
{		

}

//Init
unsigned char staCommModuleInit()
{
	Trace("\n staCommModuleInit " );
#ifdef _MOBILE_SMS_1
	memset( &g_SMSCommModuleInfo, 0, sizeof(g_SMSCommModuleInfo) );
	ReTryTime = 0;
	g_SMSCommModuleInfo.m_byCommStatus = COMM_STATUS_INITING;
#endif		
	return STA_CHECK_COMM_STATUS;
}

void commInitGPRS()
{
	
}

//检查通讯状态
unsigned char staCheckCommStatus()
{
    return 0;
}
/*
unsigned char ScanSMSErrorCode()
{
//	Trace( "\n staCheckCommStatus" );
	switch( SMSErrorCode )
	{	
		case 0 :
			Trace( "\n staCheckCommStatus SMS_ERR_NULL " );
		case SMS_ERR_SEND_SUCCEED:
			Trace( "\n staCheckCommStatus SMS_ERR_SEND_SUCCEED " );
		case SMS_ERR_SENDFAIL:
			Trace( "\n staCheckCommStatus SMS_ERR_SENDFAIL " );
		case SMS_ERR_READFAIL:		
			g_SMSCommModuleInfo.m_byCommStatus = COMM_STATUS_OK;			
			return	STA_SMS_NULL;
			break;
		case SMS_ERR_BLOCK:		
			Trace( "\n staCheckCommStatus SMS_ERR_BLOCK" );			
			return STA_CHECK_COMM_STATUS;	
			break;
		case SMS_ERR_TIMEOUT:		
		default:
			Trace( "\n staCheckCommStatus  SMS_ERR_TIMEOUT " );
			commInitGPRS();	
			return STA_CHECK_COMM_STATUS;	
		break;
	}	
}
*/
#ifdef	_MOBILE_SMS_1

	void commResetAlarmFlag()
	{
		g_SMSCommModuleInfo.m_byAlarm = 0;
	}
	
	unsigned char TestSMSSend( struct SHORTMSG* Msg )
	{		
		if( g_SMSCommModuleInfo.m_byCommStatus != COMM_STATUS_OK )
			return 1;
		Trace("\n TestSMSSend");
		if( PutShortMsg( Msg) == 1 )
			return 1;
		Trace("\n TestSMSSend Sending");
		g_SMSCommModuleInfo.m_byCommStatus = COMM_STATUS_SMS_SENDING;
		g_byCommRunningStatus = STA_CHECK_SMS_SEND_RESULT;
		RunFlag |= 0x01;
		return 0;
	}

	///取短信发送结果
	//0－OK,1－fail,2-continue
	unsigned char GetTestSMSSendResult(  )
	{
		Trace("\n GetTestSMSSendResult RunFlag = %bu", RunFlag );
		if( ( RunFlag & 0x01 ) == 0x01 )
			return 2; 
		if( ( RunFlag & 0x02 ) == 0x02 )
		{ 
			RunFlag &= 0xfb; //清除bit0和bit1，置为0
			return 1;
		}
		else
		{
			RunFlag &= 0xfb; //清除bit0和bit1，置为0
			return 0;
		}
	}

	///取AT指令故障码
	unsigned int  GetATCMDResult()
	{
		return SMSErrorCode;
	}

	//检查短信发送结果
	unsigned char staCheckSMSSendResult()
	{	
		return 0;
	}

	//将SMS放入发送缓冲
	//pSMS必须已经是经过切割处理，保证长度不大于MAX_SMS_LEN（使用PDU模式下的8位编码方式，最大是140个字符）
	void commAdd2SMSSendBuffer( unsigned char* pSMS, unsigned char byLen )
	{
		unsigned char data byEndIndex = g_SMSCommModuleInfo.m_SMSBuffer.m_byBeginIndex
			+ g_SMSCommModuleInfo.m_SMSBuffer.m_byCount;
		if( byEndIndex >= MAX_BUFFERED_SMS_COUNT )
		{
			byEndIndex -= MAX_BUFFERED_SMS_COUNT;
		}
		
		if( byLen > MAX_SMS_LEN )
		{
			byLen = MAX_SMS_LEN;				
		}
		Trace("\n commAdd2SMSSendBuffer");
		memcpy( g_SMSCommModuleInfo.m_SMSBuffer.m_bySMSs[ byEndIndex ]
			, pSMS, byLen );
		g_SMSCommModuleInfo.m_SMSBuffer.m_byLens[ byEndIndex ] = byLen;		
		
		if( g_SMSCommModuleInfo.m_SMSBuffer.m_byCount == MAX_BUFFERED_SMS_COUNT )	//drop first one
		{
			if( ++g_SMSCommModuleInfo.m_SMSBuffer.m_byBeginIndex == MAX_BUFFERED_SMS_COUNT )
			{
				g_SMSCommModuleInfo.m_SMSBuffer.m_byBeginIndex = 0;
			}
		}
		else
		{
			++g_SMSCommModuleInfo.m_SMSBuffer.m_byCount;
		}
	}
	
	//从SMS缓冲中提取SMS进行真正发送
	void commDoSendSMS()
	{
		struct SHORTMSG xdata struSMS;	

		if( g_SMSCommModuleInfo.m_byCommStatus != COMM_STATUS_OK )
		{
		//	Trace("\n commDoSendSMS SENDING");
			return;
		}	

		if( !g_SMSCommModuleInfo.m_SMSBuffer.m_byCount || !SystemParameter.GSMNO[ 0 ] )		
			return;		
		
		if( IsMessagerBuzy() )		
			return ;		

		if( SendSMSTimer != 0 )
			return;

		Trace("\n commDoSendSMS Add Msg To Send ");
		memcpy( struSMS.phone, SystemParameter.GSMNO, sizeof(struSMS.phone) );
		struSMS.len = g_SMSCommModuleInfo.m_SMSBuffer.m_byLens[ g_SMSCommModuleInfo.m_SMSBuffer.m_byBeginIndex ];
		memcpy( struSMS.msg
			, g_SMSCommModuleInfo.m_SMSBuffer.m_bySMSs[ g_SMSCommModuleInfo.m_SMSBuffer.m_byBeginIndex ]
			, struSMS.len );		
		PutShortMsg( &struSMS );
		g_SMSCommModuleInfo.m_byCommStatus = COMM_STATUS_SMS_SENDING;

		g_byCommRunningStatus = STA_CHECK_SMS_SEND_RESULT;

		return ;
	}

	//检查发送报警请求
/*	unsigned char staCheckAlarm()
	{
	#ifdef PRINT_TRACE
		ZhkSerial1Printf( "%s:%d:staCheckAlarm\n", __FILE__, __INT_LINE__ );
	#endif
		if( g_SMSCommModuleInfo.m_byAlarm )
		{
	#ifdef PRINT_DEBUG
		ZhkSerial1Printf( "%s:%d:g_CommModuleInfo.m_byAlarm\n", __FILE__, __INT_LINE__ );
	#endif
			return STA_PACK_STATUS_PACKET;
		}
		
		return STA_CHECK_COMM_HEARTBEAT;
	}
*/
#endif
