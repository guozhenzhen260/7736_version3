#ifndef __COMM_MODULE_H__
#define __COMM_MODULE_H__

//
//目前只有应用才有回答包处理，通讯模块发送的请求包（状态上送）的回答包处理必须特殊处理（而不能通过WaitAnswer机制）
//

#include <string.h>

#include "global.h"
#include "mobile.h"
#include "crc.h"
#include "clock.h"
#include "timer.h"
#include "gprs.h"

#include "communication.h"

/////任务执行步骤定义

#define STA_COMM_MODULE_INIT 0
//检查通讯状态以及全局通讯参数和局部通讯参数(如发生变化，拷贝全局通讯参数到局部)
#define STA_CHECK_COMM_STATUS 1

///查询短信发送结果
#define STA_CHECK_SMS_SEND_RESULT 17

///发送短信空操作,这个空操作使流程转到判断是不有短信需要发送
#define STA_SMS_NULL 18

extern unsigned char data g_byCommRunningStatus;

//外围函数得到的通讯链路所处的状态
#define COMM_STATUS_HANGUP 0
#define COMM_STATUS_DIALING 1
#define COMM_STATUS_BROKEN 2		//dial ok, but tcp broken
#define COMM_STATUS_CONNECTING 3
#define COMM_STATUS_OK 4
#define COMM_STATUS_BROKEING 5
#define COMM_STATUS_HANGING 6
#define COMM_STATUS_INITING 7		//正在初始化
#define COMM_STATUS_SMS_SENDING 8		//正在发送短信


////通用函数
//Init	
unsigned char staCommModuleInit();	
//检查通讯状态以及全局通讯参数和局部通讯参数(如发生变化，拷贝全局通讯参数到局部)	
unsigned char staCheckCommStatus();


#define VMID_LEN 5

#ifdef _MOBILE_SMS_1		//用短信进行处理

	#define MAX_BUFFERED_SMS_COUNT 10
	struct SMSBuffer
	{
		unsigned char m_bySMSs[ MAX_BUFFERED_SMS_COUNT ][ MAX_SMS_LEN ];
		unsigned char m_byLens[ MAX_BUFFERED_SMS_COUNT ];		
		unsigned char m_byBeginIndex;
		unsigned char m_byCount;
	};
	struct SMSModuleInfo
	{		
		struct SMSBuffer m_SMSBuffer;
		unsigned char m_byAlarm;
		unsigned char m_byCommStatus;
	};
	extern struct SMSModuleInfo xdata g_SMSCommModuleInfo;
	//检查发送报警请求
	#define STA_CHECK_ALARM 6
	unsigned char staCheckAlarm();
	//从SMS缓冲中提取SMS进行真正发送
	void commDoSendSMS();
	//将SMS放入发送缓冲
	//pSMS必须已经是经过切割处理，保证长度不大于MAX_SMS_LEN（使用PDU模式下的8位编码方式，最大是140个字符）
	void commAdd2SMSSendBuffer( unsigned char* pSMS, unsigned char byLen );
	//检查短信发送结果
	unsigned char staCheckSMSSendResult();
#endif
#endif //__COMM_MODULE_H__