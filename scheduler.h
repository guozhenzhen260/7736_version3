#ifndef _SCHEDULER_
#define _SCHEDULER_

#ifndef SCHEDULER_C
	#define SchedulerSourse extern
#else
	#define SchedulerSourse
#endif
	#define CS_GB2312    	936
	#define CS_WIN1252  	1252
	SchedulerSourse uchar xdata ZhkSchedulerState;
	SchedulerSourse void ZhkChannelSwitch( uchar type );
	SchedulerSourse uchar ZhkSchedulerAddTask( struct COMMTASK xdata* NewTask );
	
	/** @name 通信设备轮询任务调度执行状态定义 
	 *
	 */
	 //@{
	/** 任务调度处于空闲状态 */
	#define TASK_NULL		0x00
	/** 当前任务处于就绪状态 */
	#define TASK_REDAY		0x01
	/** 当前任务处于执行等待状态 */
	#define TASK_WAIT		0x02
	/** 当前任务处于重新执行并等待状态 */
	#define TASK_REWAIT		0x03
	/** 当前任务正确执行结束 */
	#define TASK_FINISH  		0x04
	/** 当前任务错误 */
	#define TASK_ERROR		0x05
	//@}
	/** @name 通信设备轮询任务优先级定义
	 *  数值越小优先级越高
	 */
	 //@{
	/** 任务调度处于空闲状态 */
	// 
	// 数值越小优先级越高
	#define PRIORITY1  		0x41
	#define PRIORITY2  		0x42
	#define PRIORITY3  		0x43
	#define PRIORITY4  		0x44
	#define PRIORITY5  		0x45
	#define PRIORITY6  		0x46
	//@}
	/** 定义普通任务的优先级为次优先级 */
	#define PRIORITY_NORMAL  	PRIORITY2
	
	/** 通信任务类型定义 */
	struct COMMTASK
	{
		uchar 		Priority;		// 优先级，0 为最高级
		uchar 		Id;			// id号
		uchar 		Cmd;			// 指令
		uchar 		Sn;			// 
		uchar 		State;			//
		uchar 		ParamLen;		//
		uchar 		Param[32];		//
	};
	extern struct COMMTASK xdata CurrentTask;
	/**　自动执行任务的类型定义　*/
	struct AUTOTASK
	{
		uchar 		Id;
		uchar 		Cmd;
		uchar 		Cycle;
	};
	/** 设备类型定义　*/
	struct DEVICE					// 设备数据类型定义

	{
		uchar Id;			// 设备ID
		uchar Sn;			// 设备sn计数器
		uchar CommState;		// bit0 = Buzy , bit1 = TimeOut , bit2 = CommOK
		uchar State;			// 设备基本状态
		uchar ExtState[17];		// 设备扩展状态
	};
	/** @name设备通信状态定义
	 *
	 */
	//@{
	/** 忙标志,等于１表示当前设备通信状态处于忙状态，等于０表示不忙 */
	#define COMM_BUZY    		1
	
	/** 超时标志,
	 *  等于１时表示当前通信未能再预定时间内接收到应答信号，
	 *  等于０表示当前通信任务已经在预定时间内接收到应答信号 
	 */
	#define COMM_TIMEOUT 		2
	/** 通信任务结束标志 
	 *  等于１表示当前通信任务结束，包括超时和错误结束
	 *　等于０表示当前通信任务尚未结束　
	 */
	#define COMM_COMMOK 		4  		// bit2 = CommOK
	//@}
	
///////////////////////////////////////////////////////////////////////////////////////	
///////////////////////////////////////////////////////////////////////////////////////
	
	/** 通信设备轮询任务调度初始化 */
	SchedulerSourse uchar SchedulerInit( void );
	/** 设备调度入口函数 */
	SchedulerSourse uchar SchedulerProcess( void );


///////////////////////////////////////////////////////////////////////////////////////
//
//       
//
///////////////////////////////////////////////////////////////////////////////////////
	/** 检测设备是否忙 
	 *  @param
	 */
	SchedulerSourse uchar TestDeviceBuzy( struct DEVICE xdata* Device );
	// 检测设备是否忙 
	// 返回值：
	// 	1 表示忙，0表示正常

	SchedulerSourse uchar TestDeviceTimeOut( struct DEVICE xdata* Device );
	// 检测设备是否超时
	// 返回值：
	// 	1 表示超时，0表示正常

	SchedulerSourse uchar TestDeviceCommOK( struct DEVICE xdata* Device );
	// 检测设备是否通信完成
	// 返回值：
	// 	1 表示通信完成，0表示未完成

	SchedulerSourse uchar TestDeviceException( struct DEVICE xdata* Device );
	// 检测设备是否故障
	// 返回值：
	// 	1 表示故障，0表示正常


	
///////////////////////////////////////////////////////////////////////////////////////
//
//      设备状态查询使能控制
//
///////////////////////////////////////////////////////////////////////////////////////	
//	SchedulerSourse bit Hopper1QueryEnable( void );
	// 找币模块1状态查询使能
	SchedulerSourse bit Hopper1QueryDisable( void );
	// 找币模块1状态查询禁能
	SchedulerSourse bit CasherQueryEnable( void );
	// 纸币器状态查询使能
	SchedulerSourse bit CasherQueryDisable( void );
	// 纸币器状态查询禁能

///////////////////////////////////////////////////////////////////////////////////////
//
//      hub模块相关接口函数定义
//	
///////////////////////////////////////////////////////////////////////////////////////	
//	SchedulerSourse bit DenoteLedLightOn( void );
	// 点亮出货指示灯
//	SchedulerSourse bit DenoteLedLightOff( void );
	// 熄灭出货指示灯	
	
	//报警器开
//	SchedulerSourse bit SetAlertOn( void );
	//报警器关
//	SchedulerSourse bit SetAlertOff( void );

///////////////////////////////////////////////////////////////////////////////////////	
//
//      除币机构接口函数 
//	
///////////////////////////////////////////////////////////////////////////////////////	
	SchedulerSourse bit HopperOutput( uchar HopperNum , uint CoinAmount );
	// 出币模块出币控制
	// 参数：
	// HopperNum	出币模块编号，取值范围1-2
	// CoinAmount  	出币数量(单位：枚)

	SchedulerSourse bit HopperQuery( uchar HopperNum );
	// 出币模块状态更新
	// 参数：
	// HopperNum	出币模块编号，取值范围1-2
	// 功能：更新当前Hopper状态,
	// 返回值保留
	
	SchedulerSourse uint HopperGetCoinOut( uchar HopperNum );
	// 出币模块已出币数量
	// 参数：
	// HopperNum	出币模块编号，取值范围1-2
	// 返回值：
	// 已出币数量

	SchedulerSourse bit SelectionQuery( uchar itemNum );
    SchedulerSourse bit ACDCModuleQuery( void );
    SchedulerSourse bit HopperClearSn( uchar HopperNum );
///////////////////////////////////////////////////////////////////////////////////////	
//
//      液晶显示接口函数 
//	
///////////////////////////////////////////////////////////////////////////////////////	
	SchedulerSourse bit DisplaycharacterSet( uint CharSetCode );
	// 选择字符集
	SchedulerSourse bit DisplayClear( void );
	// 清屏
	SchedulerSourse bit DisplayString( uchar * Str , const uchar len );
	// 以当前光标位置为起点显示一串字符，如果字符串超过屏幕宽度将自动换行
	// 光标最终停留在最后一个显示的字符的下一个位置
	SchedulerSourse bit DisplayPosString( const uchar PosX , const uchar PosY , uchar * Str , const uchar len );
	// 定位显示一串字符，如果字符超过屏幕宽度，超出部分不显示
	// 光标最终停留在最后一个显示的字符的下一个位置，如果有超出屏幕宽度，光标停留在当前行的末尾
	// 参数
	// PosX 起始点的横坐标
	// PosY 起始点的纵坐标
	// Str  字符串首地址
	// len  字符串长度

//ygp close	
/*	SchedulerSourse bit DisplayScroll( uchar Direct );
	// 屏幕滚动
	// 参数：
	// Direct 屏幕滚动的方向取值范围0-3
	// 	0表示向下滚动
	// 	1表示向上滚动
	// 	2表示向左滚动
	// 	3表示向右滚动
*/
	
	SchedulerSourse bit DisplaySetCursorStyle( uchar style );
	// 设置光标类型
	// 参数：
	// style 光标显示类型
	// 	0表示不无光标，
	// 	1表示字节光标，
	// 	2表示整行光标。

	SchedulerSourse bit DisplaySetCursorPos( uchar PosX , uchar PosY );
	// 设置光标位置
	// 参数：
	// PosX 横坐标
	// PosY 纵坐标
	SchedulerSourse bit DisplayBkLightOn( void );
	// 点亮液晶背光
	SchedulerSourse bit DisplayBkLightOff( void );
	// 熄灭液晶背光

	SchedulerSourse uchar IsBackLightOn( void );
	// 液晶背光是否处于点亮状态

	SchedulerSourse uchar GetDisplayState();
///////////////////////////////////////////////////////////////////////////////////////	
//
//      纸币器接口函数 
//	
///////////////////////////////////////////////////////////////////////////////////////	

	SchedulerSourse bit CasherQuery( void );

	SchedulerSourse bit CasherAccept( void );
	// 接收纸币
	SchedulerSourse bit CasherReject( void );
	// 拒收纸币
	SchedulerSourse uchar CasherIsCashIn( void );
	// 是否有纸币进入等待接收
	SchedulerSourse uchar CasherIsAcceptOK( void );
	// 是否纸币接受成功
	SchedulerSourse uint GetCash( void );
	// 读取已接收到纸币面额
	SchedulerSourse uint GetComingCash( void );
	// 读取已检测到的纸币面额
	SchedulerSourse bit CasherEnable( void );
	SchedulerSourse bit CasherDisable( void );
	SchedulerSourse bit CasherReset( void );
	SchedulerSourse bit ZhkCasherQuery( void );
	//机器的工作状态传入纸币器作业流程
	SchedulerSourse void CasherGetMachineState( uchar state );
	//向外发布纸币器的实时状态,0表示正常，1表示故障
	SchedulerSourse uchar CasherRealTimeState( );
	//退币是否成功
	SchedulerSourse uchar CasherIsRejectOK( void );
	//重置超时控制
	SchedulerSourse void CasherResetOuttime( void );
	///////////////////////////////////////////////////////////////////////////////////////	
	//
	//      硬币器接口函数 
	//	
	///////////////////////////////////////////////////////////////////////////////////////	
	//关闭硬币器电源
//	SchedulerSourse bit CoinPowerOff( void );
	//打开硬币器电源
//	SchedulerSourse bit CoinPowerOn( void );

	/*
	// 硬币器状态查询
	SchedulerSourse bit CoinerQuery( void );	
	// 是否有硬币进入
	SchedulerSourse uchar CoinerIsCoinIn( void );	
	// 读取已接收到纸币面额	
	SchedulerSourse uint GetCoin( void );	
	// 硬币器使能
	SchedulerSourse bit CoinerEnable( void );
	// 硬币器禁能
	SchedulerSourse bit CoinerDisable( void );
	// 硬币器复位
	SchedulerSourse bit CoinerReset( void );
	//硬币器查询使能
	SchedulerSourse bit CoinerQueryEnable( void );
	//硬币器查询禁能
	SchedulerSourse bit CoinerQueryDisable( void );
*/
	//////////////////////////////////////////////
	//			货道接口				//////////
	//////////////////////////////////////////////
	//货道状态查询
	SchedulerSourse bit ZhkChannelQuery();
	// 货道驱动模块查询指令
	// ChannelNum 货道号 1-64
	// 当查询成功后
	// 状态存储在Channel.State
	//ChannelNum 从1开始

	//出货命令
	SchedulerSourse bit ZhkChannelExec(  unsigned char ChannelNum,  unsigned char ReSendFlag );
	// 货道驱动模块出货指令
	// ChannelNum 货道号 1-64
	//ChannelNum 从1开始
	
	//货道测试
	SchedulerSourse bit ZhkChannelTest(  unsigned char ChannelNum );
	
	//货道命令SN清零
	SchedulerSourse bit ZhkChannelClear();

	///测试手机模块的连接状态
	//返回0－OK,1－失败,3－手机模块忙
	SchedulerSourse uchar TestATcommand();


//	SchedulerSourse bit ZhkLedScrset( uchar LEDWIDE , uint CharSum , void* ScrMsg );
	// 更新广告屏显示内容
	// LEDWIDE 屏幕宽度，8的倍数
	// CharSum 显示内容的字节长度
	// ScrMsg 显示内容的首地址

	// 设备定义
	SchedulerSourse struct DEVICE xdata Hopper1;	
	SchedulerSourse struct DEVICE xdata Hopper2;
	SchedulerSourse struct DEVICE xdata Hopper3;

    SchedulerSourse struct DEVICE xdata Selection1;
    SchedulerSourse struct DEVICE xdata Selection2;
    SchedulerSourse struct DEVICE xdata Selection3;
    SchedulerSourse struct DEVICE xdata ACDCModule;

	//扩展状态中的ExtState[13]记录纸币器的实时状态,0为正常，1为故障
	//扩展状态中的ExtState[14]记录纸币器连续回应超时次数,
	//扩展状态中的ExtState[15]记录机器的当前状态:空闲(0)或服务状态(1)
	SchedulerSourse struct DEVICE xdata Casher;
	SchedulerSourse struct DEVICE xdata Lcd;
	SchedulerSourse struct DEVICE xdata Channel;
//	SchedulerSourse struct DEVICE xdata Hub;
//	SchedulerSourse struct DEVICE xdata Coiner;	
	SchedulerSourse uchar xdata MaintFlag;	
	
	
////////////////////////////////////////////////////////////////////////////////////////



#endif
