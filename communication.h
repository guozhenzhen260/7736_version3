#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

//#define PRINT_DEBUG

unsigned int ZhkSerial1Printf( unsigned char *format , ... );

//
//-----------------------------------注意----------------------------
//
//目前短信处理都是针对unsigned char，而非unicode
//
//目前发送包不能大于4096（包括包头和CRC的长度）
//
//如果等待回答包超时，强烈建议关闭链路。否则可能发生应用准备发送第二个包的时候，收到第一个包的回答，
//应用和通讯模块同时使用全局缓冲区（机器所限，应用的发送和接收缓冲区为同一个），导致混乱
//
//由于VMID是通讯模块打包的，所以要保证系统参数全局变量中的VMID正确（当发送INIT的时候，其实为9999+安装码）
//-------------------------------------------------------------------
//

//更改
//2005-03-07
//	增加缓冲区大小COMM_PACKET_BUFFER_SIZE的宏定义
//
//2005-03-05
//	增加SendSMS函数，用于SMS发送。因为现在发送短信为非独占式，而正在发送的时候无法再发送下一条。所以通讯模块进行了缓冲处理。
//	ResponsePacketArrived函数增加一个参数，用于返回收到的回答包报文体长度

//2005-03-01
//	更改调试信息处理宏定义

//2005-02-28
//	增加管理命令函数补充说明：
//		注意，是否有管理命令的函数，返回有管理命令后，该条管理命令信息就标志为已通知应用，不再保留了
//	DLCardParamCommandExist函数增加一个参数
//////////////////////////////////////////////////////


//#define PRINT_DEBUG
#define __INT_LINE__ ((unsigned int)__LINE__)
#include <stdio.h>
#include "global.h"


//
//通讯后台任务执行函数
//该函数不会长时间占用CPU，应用应有机会让其运行
//
void DoCommunication();
//清除短信缓存区内的所有短消息
//void ClearAllSMSInSMSBuffer();

unsigned char TestSMSSend( struct SHORTMSG* Msg );
///取短信发送结果
//0－OK,1－fail,2-continue
unsigned char  GetTestSMSSendResult();

///取AT指令故障码
unsigned int  GetATCMDResult();

#ifdef _MOBILE_SMS_1
	//将SMS放入发送缓冲
	void SendSMS( unsigned char* pSMS, unsigned char byLen );
	//发送状态报警（为设备模块报警，不包含非法入侵）
	void SendStatusWarnning();
	//获取通讯和短信缓冲状态,是否可以发送短信
	//1 - OK	0 - Not OK
	bit SMSCommStatus();	
#endif

#endif //__COMMUNICATION_H__