// gprs 服务接口
// 采用非阻塞异步通信方式
#ifndef _GPRS_
#define _GPRS_

#ifndef GPRS_C
#define GprsSourse extern
#else
#define GprsSourse
#endif

// 拨号
// 立即返回
GprsSourse void GprsDailUp( uchar* VPN );

// 挂断
// 立即返回
GprsSourse void GprsHangUp( void );


// 建立连接
// 立即返回
GprsSourse void GprsConnect( uchar* IP , uint Port );

// 切断连接
// 立即返回
GprsSourse void GprsShut( void );

// 发送信息
// 返回值
// -1 表示错误
// >=0 表示实际写入缓冲区的数量
GprsSourse int GprsSendMsg( uchar* GprsMsg , int len );

// 接收信息
// len 表示请求接收的信息长度
// 返回值表示
// >=0 实际接收到的信息长度
// -1 错误
GprsSourse uchar GprsInit( void );
GprsSourse int GprsRecvMsg( uchar *GprsMsg , int len );

// 检查设备状态
// 返回值为设备状态，掩码表示
GprsSourse uchar GprsSelect( void );

// 设备状态定义
#define GPRS_WRITEABLE 		1
#define GPRS_READABLE 		2
#define GPRS_ERROR 		4


// 取前一次操作的错误信息
// 返回值错误类型，
GprsSourse uchar GprsGetLastErr( void );

// 错误类型定义
#define GPRS_ERR_NULL 		0
#define GPRS_ERR_BLOCK 		1
#define GPRS_ERR_HANGUP  	2
#define GPRS_ERR_CONNECTBREAK	3
#define GPRS_ERR_TIMEOUT 	4
#define GPRS_ERR_SENDFAIL 	5
#define GPRS_ERR_CONNECTING  	6

#endif 

