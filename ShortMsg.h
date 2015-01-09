#ifndef _SHORTMSG_
#define _SHORTMSG_
#ifndef SMS_C
	#define ShortMsgSourse extern
#else
	#define ShortMsgSourse
#endif

	#define MAX_SMS_LEN 160
	struct SHORTMSG 
	{
		// 手机号码,收发时都为BCD格式,收短讯时,此号码中包括国家代码,
		uchar phone[16];		
		// 短信长度	
		uint len;			
		// 收短信和发短信时是ASC码表示
		uchar msg[MAX_SMS_LEN];			
	};

	// 判断是否受到短信
	// 如果收到短信返回非0
	// 如果未收到短信返回0
	//ShortMsgSourse bit IsShortMsgIn( void );	
	
	// 读取收到的短信
	// Message 存储目标位置
	ShortMsgSourse uchar GetShortMsg( struct SHORTMSG* Msg );	
	
	// 发送短信（非独占方式，即不是等待发送完才返回）	
	ShortMsgSourse uchar PutShortMsg( struct SHORTMSG* Msg );	
	
	ShortMsgSourse bit IsMessagerBuzy( void );

	ShortMsgSourse uchar SMSGetLastErr( void );

	// 错误类型定义
	#define SMS_ERR_NULL 		0
	#define SMS_ERR_BLOCK 		1
	#define SMS_ERR_CONNECTBREAK	2
	#define SMS_ERR_TIMEOUT 	3	
	#define SMS_ERR_SENDFAIL 	4	//但和手机模块的通讯还是正常的
	#define SMS_ERR_READFAIL 	5	//但和手机模块的通讯还是正常的
	#define SMS_ERR_CONNECTING  6
	#define SMS_ERR_SEND_SUCCEED 7
	#define SMS_ERR_REND_SUCCEED 8

	ShortMsgSourse	uint 	SMSErrorCode;	//手机模块返回的错误码

	//短信重读或重发的次数,不管是收还是发，重试3次即丢弃此信息
	//记录发信息的次数	
	ShortMsgSourse uchar SMSSendTime;
	//记录收信息的次数
	ShortMsgSourse uchar SMSReceiveTime;
	//重发或重收时的次数
	#define RETRYTIME  5
	//重发或重收时间隔的时间，以秒为单位
	#define INTERVAL_TIME 30

	//当发送信息或接收短信时出现错误时重试的次数,
	//bit0、bit2、bit3用于记录重发的次数
	//此变量的bit7和bit6两位用于记录网络状态
	//当此娈量的bit7为0,bit6位为1,且值（不含bit6和bit7位）大于5时则丢弃此条短信,并置bit7为1,bit6为0;
	//如果bit7为1,bit6为1,且值（不含bit6和bit7位）大于5时,表示连续两条短信的发送都不成功，
	//此时停止短信发送
	//如果有成功收到一条短信或中间成功发送过一次短信，则置此变量
	ShortMsgSourse	uchar ReTryTime;

	//发送或接收短信重试指定次数还没有成功，则手机模块下电重启
	ShortMsgSourse	uchar DownMobilePowerFlag;	
#endif

