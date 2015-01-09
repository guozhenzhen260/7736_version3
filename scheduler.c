//#undef _DEBUG_TRACE
#define SCHEDULER_C
#include "device.h"
#include "common.h"
#include "scheduler.h"
#include "DataExchange.h"
#include "Casher.h"
#include "timer.h"
#include "string.h"
#include "debug.h"
#include "serial1.h"
#include "serial.h"
#include "procotol.h"
#include "Coiner.h"
#include "IOInput.h"
#include "global.h"
#include "mobile.h"
#include "lifterApi.h"



extern uchar xdata SystemStatus;
extern u_int  xdata HardWareErr;
uchar xdata ZhkCasherStatus;
uchar xdata CoinerStatus;

extern unsigned char VPMission_Button_RPT( unsigned char type, unsigned char value );
extern bit IfGameKeyOn( void );

///纸币器状态值
#define CASHER_STATUS_NULL  	0
#define CASHER_STATUS_ACCEPT  	1
#define CASHER_STATUS_ENABLE  	2
#define CASHER_STATUS_DISABLE  	3
#define CASHER_STATUS_REJECT 	4
#define CASHER_STATUS_QUERY 	5

///硬币器状态值
#define COINER_STATUS_NULL  	0
#define COINER_STATUS_ENABLE  	1
#define COINER_STATUS_DISABLE  	2
#define COINER_STATUS_RESET 	3

#define DOWNMSGACKTIMEOUT       30

uchar ZhkSchedulerAddTask( struct COMMTASK xdata* NewTask );
// 添加调度任务
bit ZhkSchedulerIsTaskNull( uchar TaskNum );
// 判断当前任务是否无任务
// 在人物列表中删除任务
uchar ZhkGetSchedulerClock( void );
// 读取调度时钟

bit ZhkHopper1Query( void );
bit ZhkHopper2Query( void );
bit ZhkHopper3Query( void );
//bit ZhkHubQuery( void );
bit ZhkCasherQuery( void );
//作业
bit ZhkNullTask( void );
bit ZhkCasherTask( struct COMMTASK xdata* TaskTemp );
bit ZhkHopperTask( struct COMMTASK xdata* TaskTemp );
bit ZhkLcdTask( struct COMMTASK xdata* TaskTemp );
//bit ZhkHubTask( struct COMMTASK xdata* TaskTemp );
//bit ZhkPrnTask( struct COMMTASK xdata* TaskTemp );
bit ZhkChannelTask( struct COMMTASK xdata* TaskTemp );

//处理
uchar ZhkHopperMsgProcess( void );
uchar ZhkCasherMsgProcess( void );
//uchar ZhkHubMsgProcess( void );
uchar ZhkLcdMsgProcess( void );
//uchar ZhkPrnMsgProcess( void );
uchar ZhkChannelMsgProcess( void );

bit ZhkCasherTxMsg( struct COMMTASK xdata* NewTask, uchar xdata Flag  );
bit ZhkHopperTxMsg( struct COMMTASK xdata* NewTask );
bit ZhkLcdTxMsg( struct COMMTASK xdata* NewTask );
//bit ZhkHubTxMsg( struct COMMTASK xdata* NewTask );
//bit ZhkPrnTxMsg( struct COMMTASK xdata* NewTask );
bit ZhkChannelTxMsg( struct COMMTASK xdata* NewTask );

//uchar CoinerMsgProcess( void );
//bit CoinerTxMsg( struct COMMTASK xdata* TaskTemp );
//bit CoinerTask( struct COMMTASK xdata* TaskTemp );

void ZhkChannelSwitch( uchar type );

struct AUTOTASK code ZhkAutoTaskList[] = 
{	
	ID_CASHER  , CASHER_QUERY   ,150 ,
	ID_COINER1 , COINER1_QUERY   ,100 
};	

//任务计时器
uchar xdata ZhkAutoTaskTimer[ sizeof( ZhkAutoTaskList ) / sizeof( struct AUTOTASK ) ];
//任务许可
uchar xdata ZhkAutoTaskPermission[ sizeof( ZhkAutoTaskList ) / sizeof( struct AUTOTASK ) ];



// 串口设备调度时钟，以秒为单位
uchar xdata ZhkSchedulerId;
// 当前设备任务的设备ID
uchar xdata ZhkSchedulerState;

struct COMMTASK xdata ZhkReadyTaskList[ 32 ];
// 就绪任务队列
struct COMMTASK xdata CurrentTask;
// 当前占用串口任务编号

uchar xdata	AlarmFlag = 0;//0为关,1为开

//是否轮询手机模块控制代码
#ifndef _DEBUG_NO_MOBILE
	uchar xdata IsPressMobile = 1;
#endif // _DEBUG_NO_MOBILE


uchar SchedulerInit( void )
{
	uchar data i;

	Hopper1.Id            = ID_HOPPER1;
	Hopper1.Sn            = 0;
	Hopper1.CommState     = 0;
	Hopper1.State         = 0;
	memset( Hopper1.ExtState, 0, sizeof( Hopper1.ExtState ) );
//	Hopper1.ExtState[ 0 ] = 0;
//	Hopper1.ExtState[ 1 ] = 0;

	Hopper2.Id            = ID_HOPPER2;
	Hopper2.Sn            = 0;
	Hopper2.CommState     = 0;
	Hopper2.State         = 0;
	memset( Hopper2.ExtState, 0, sizeof( Hopper2.ExtState ) );
//	Hopper2.ExtState[ 0 ] = 0;
//	Hopper2.ExtState[ 1 ] = 0;

	Hopper3.Id            = ID_HOPPER3;
	Hopper3.Sn            = 0;
	Hopper3.CommState     = 0;
	Hopper3.State         = 0;
	memset( Hopper3.ExtState, 0, sizeof( Hopper3.ExtState ) );
//	Hopper3.ExtState[ 0 ] = 0;
//	Hopper3.ExtState[ 1 ] = 0;
			
	Lcd.Id            = ID_LCD;
	Lcd.Sn            = 1;
	Lcd.CommState     = 0;
	Lcd.State         = 0;
	memset( Lcd.ExtState, 0, sizeof( Lcd.ExtState ) );
//	Lcd.ExtState[ 0 ] = 0;
//	Lcd.ExtState[ 1 ] = 0;
	
	Casher.Id            = ID_CASHER;
	Casher.Sn            = 1;
	Casher.CommState     = 0;
	Casher.State         = 0;
	memset( Casher.ExtState, 0, sizeof( Casher.ExtState ) );
//	Casher.ExtState[ 0 ] = 0;
//	Casher.ExtState[ 1 ] = 0;

//	Coiner.Id            = ID_COINER1;
//	Coiner.Sn            = 1;
//	Coiner.CommState     = 0;
//	Coiner.State         = 0;
//	memset( Coiner.ExtState, 0, sizeof( Coiner.ExtState ) );
//	Coiner.ExtState[ 0 ] = 0;
//	Coiner.ExtState[ 1 ] = 0;

//	Hub.Id            = ID_HUB;
//	Hub.Sn            = 1;
//	Hub.CommState     = 0;
//	Hub.State         = 0;
//	memset( Hub.ExtState, 0, sizeof( Hub.ExtState ) );
//	Hub.ExtState[ 0 ] = 0;
//	Hub.ExtState[ 1 ] = 0;

	Channel.Id            = ID_CHANNEL;
	Channel.Sn            = 1;
	Channel.CommState     = 0;
	Channel.State         = 0;
	memset( Channel.ExtState, 0, sizeof( Channel.ExtState ) );
//	Channel.ExtState[ 0 ] = 0;
//	Channel.ExtState[ 1 ] = 0;
    
    Selection1.Id         = ID_SELECTION1;
	Selection1.Sn         = 0;
	Selection1.CommState  = 0;
	Selection1.State      = 0;
	memset( Selection1.ExtState, 0, sizeof( Selection1.ExtState ) );

	Selection2.Id         = ID_SELECTION2;
	Selection2.Sn         = 0;
	Selection2.CommState  = 0;
	Selection2.State      = 0;
	memset( Selection2.ExtState, 0, sizeof( Selection2.ExtState ) );

	Selection3.Id         = ID_SELECTION3;
	Selection3.Sn         = 0;
	Selection3.CommState  = 0;
	Selection3.State      = 0;
	memset( Selection3.ExtState, 0, sizeof( Selection3.ExtState ) );
    
    ACDCModule.Id         = ID_ACDCMODULE;
	ACDCModule.Sn         = 0;
    ACDCModule.CommState  = 0;
    ACDCModule.State      = 0;
    memset( ACDCModule.ExtState, 0, sizeof( ACDCModule.ExtState ) );

	memset( ZhkAutoTaskPermission, 0, sizeof( ZhkAutoTaskPermission ) );
	ZhkCasherStatus = CASHER_STATUS_NULL;
	
//	AlarmTimer = 90;
//	AlarmFlag = 0;
	MaintFlag = 0;

	ZhkSchedulerId = ID_NULL;
	ZhkSchedulerState = TASK_NULL;	
	for( i = 0 ; i < sizeof( ZhkReadyTaskList ) / sizeof( struct COMMTASK ) ; i ++ )			
		memset( &ZhkReadyTaskList[ i ] , 0xFF , sizeof( struct COMMTASK ) );

#ifndef _DEBUG_NO_MOBILE
	ZhkMobileInit();
#endif // _DEBUG_NO_MOBILE

	return 1;
}

uchar SchedulerProcess( void )
{
	struct DEVICE xdata* data CurrentDevice;

#ifndef _DEBUG_NO_MOBILE
	if( IsPressMobile == 1 )
		ZhkMobileProcess();
#endif	
     
    //----------------------------------------------------------
    if( ( IfGameKeyOn() )&&(sysVPMission.comLock==0) )
	{
		VPMission_Button_RPT( 0, 0 );
	    //Beep();
	}
    //==========================================================
    
	//Trace( "SchedulerRouteDevice\n" );
	// 分析当前任务关联的设备


	switch( ZhkChkAddr( CurrentTask.Id ) )
	{
	case EQUIP_CASHER:
		CurrentDevice = &Casher;
		break;
//	case EQUIP_COINER:
//		CurrentDevice = &Coiner;
//		break;
	case EQUIP_LCD:
		CurrentDevice = &Lcd;
		break;
//	case EQUIP_HUB:
//		CurrentDevice = &Hub;
//		break;
	case EQUIP_HOPPER:		
		switch( CurrentTask.Id - ID_HOPPER1 )
		{
		case 0:
			CurrentDevice = &Hopper1;
			break;
		case 1:
			CurrentDevice = &Hopper2;
			break;
		case 2:
			CurrentDevice = &Hopper3;
			break;
		default:
			ZhkSchedulerState = TASK_NULL;
		}
		break;	
	case EQUIP_CHANNEL:
		CurrentDevice = &Channel;
		break;
	case EQUIP_SELECTION:		
		switch( CurrentTask.Id - ID_SELECTION1 )
		{
		case 0:
			CurrentDevice = &Selection1;
			break;
		case 1:
			CurrentDevice = &Selection2;
			break;
		case 2:
			CurrentDevice = &Selection3;
			break;
		default:
			ZhkSchedulerState = TASK_NULL;
		}
		break;
	case EQUIP_ACDCMODULE:
	    CurrentDevice = &ACDCModule;
	    break;
	default:
		ZhkSchedulerState = TASK_NULL;
	}	
	// 设备调度流程控制
	switch( ZhkSchedulerState )
	{
	case TASK_NULL:
		ZhkNullTask();
		break;
	case TASK_REDAY:
	case TASK_WAIT:
	case TASK_REWAIT:
	case TASK_ERROR:
	case TASK_FINISH:
		if ( CurrentDevice == -1 ) 
			ZhkSchedulerState = TASK_NULL;
		switch( CurrentTask.Id )
		{
		case ID_HOPPER1:
		case ID_HOPPER2:
		case ID_HOPPER3:
			ZhkHopperTask( &CurrentTask );
	//		Trace( "\n ZhkSchedulerState = %bu", ZhkSchedulerState );
			break;
		case ID_LCD:
			ZhkLcdTask( &CurrentTask );
			break;		
		case ID_CASHER:
			//ZhkCasherTask( &CurrentTask );
			break;	
//		case ID_HUB:
//			ZhkHubTask( &CurrentTask );
//			break;
//		case ID_COINER1:
//			CoinerTask( &CurrentTask );
//			break;
		case ID_CHANNEL:
			#ifdef   _SJJ_//升降机
				ZhkLifterTask(&CurrentTask );	
			#else         //普通货道
		    	ZhkChannelTask( &CurrentTask );		
			#endif
			
			break;
		case ID_SELECTION1:
		case ID_SELECTION2:
		case ID_SELECTION3:
			ZhkHopperTask( &CurrentTask );
	        //Trace( "\n ZhkSchedulerState = %bu", ZhkSchedulerState );
			break;
		case ID_ACDCMODULE:
		    ZhkHopperTask( &CurrentTask );
		    break;
		default:
			ZhkSchedulerState = TASK_NULL; 
		}
		break;
	default:
		ZhkSchedulerState = TASK_NULL; 
		break;
	}
  
	return ZhkSchedulerState;	
}

uchar ZhkSchedulerAddTask( struct COMMTASK xdata* NewTask )
{
	uchar data i;
//	Trace( "AddTask:Id=%bx,Cmd=%bx\n" , NewTask->Id,NewTask->Cmd );
	
	if( NewTask->Id == ID_CASHER )
	{
		if( Casher.CommState == COMM_TIMEOUT )
		{
			if( QueryCashTimer != 0 )
			{
//				Trace( "\n ZhkSchedulerAddTask ->Do not Add Query Commmand1" );
				ZhkCasherStatus = CASHER_STATUS_NULL;
				Casher.ExtState[13] = 1;
				return 1;
			}
			else
			{
				if( ( Casher.ExtState[ 15 ] == 1 ) && ( Casher.ExtState[ 14 ] != 1 ) )
				{
	//				Trace( "\n ZhkSchedulerAddTask ->Do not Add Query Commmand2" );
					ZhkCasherStatus = CASHER_STATUS_NULL;
					Casher.ExtState[13] = 1;
					return 1;
				}
			}	
		}	 			
//		Trace( "\n ZhkSchedulerAddTask ->Add Query Commmand" );
	}

	for( i = 0 ; i < sizeof( ZhkReadyTaskList ) / sizeof( struct COMMTASK ) ; i ++ )
	{
		if ( ZhkSchedulerIsTaskNull( i ) )
		{
			MovSramToSram( ( uchar xdata* )NewTask , 
				( uchar xdata* )&ZhkReadyTaskList[ i ] , 
				sizeof( struct COMMTASK ) );
			return 1;
		}
	}
	return 0;
}

bit ZhkSchedulerIsTaskNull( uchar TaskNum )
{
	if ( TaskNum >= sizeof( ZhkReadyTaskList ) / sizeof( struct COMMTASK ) )
		return 0;
	return ZhkReadyTaskList[ TaskNum ].Id == ID_NULL;
	
}

uchar ZhkGetSchedulerClock( void )
{
	uchar data ch;
	EA = 0;
	ch = ZhkSchedulerClock;
	ZhkSchedulerClock = 0;
	EA = 1;
	return ch;
}

bit ZhkNullTask( void )
{
	uchar  data i;
	uchar  data clocktemp;
	uchar  data PriorityTemp;
	uchar  data TaskIdTemp;
	//Trace( "ZhkNullTask\n" );
	// 更新定时查询任务队列中各个任务的等待就绪时间计数
	clocktemp = ZhkGetSchedulerClock();
	for( i = 0 ; i < sizeof( ZhkAutoTaskList ) / sizeof( struct AUTOTASK ) ; i ++ )
	{
		if ( ZhkAutoTaskTimer[ i ] > clocktemp ) 
			ZhkAutoTaskTimer[ i ] -= clocktemp;
		else
		{
			ZhkAutoTaskTimer[ i ]  = 0;
			if ( ZhkAutoTaskPermission[ i ] == 1 ) 
			{
		//		Trace( "\n AutoTaskNum=%bu", i );
				ZhkAutoTaskTimer[ i ] = ZhkAutoTaskList[ i ].Cycle;
				switch( ZhkAutoTaskList[ i ].Id )
				{
				//	case ID_COINER1:	
				//		if( CoinerStatus == COINER_STATUS_NULL )
				//			CoinerQuery();
				//		break;			
					case ID_CASHER:	
						if( ZhkCasherStatus == CASHER_STATUS_NULL )
							ZhkCasherQuery();					
						break;
				}
			}
		}
	}
	//Trace( "PriorityTemp = 0xFF\n" );
	// 选择任务队列中已经满足就绪条件中的优先级最高的任务
	PriorityTemp = 0xFF;
	for( i = 0 ; i < sizeof( ZhkReadyTaskList ) / sizeof( struct COMMTASK ) ; i ++ )
	{
		if ( ZhkSchedulerIsTaskNull( i ) ) 
			continue;
		if ( PriorityTemp > ZhkReadyTaskList[ i ].Priority )
		{
			PriorityTemp = ZhkReadyTaskList[ i ].Priority;
			TaskIdTemp   = i;
		}
	}			
	// 没有就绪任务，返回0
	if ( PriorityTemp == 0xFF ) 
		return 0;
	
	memcpy( &CurrentTask , &ZhkReadyTaskList[ TaskIdTemp ] , sizeof( struct COMMTASK ) );
	ZhkSchedulerState = TASK_REDAY;	

	for( i = TaskIdTemp ; i < ( sizeof( ZhkReadyTaskList ) / sizeof( struct COMMTASK ) - 1 ) ; i ++ )	
		memcpy( &ZhkReadyTaskList[ i ] , &ZhkReadyTaskList[ i + 1 ] , sizeof( struct COMMTASK ) );
	
	memset( &ZhkReadyTaskList[ sizeof( ZhkReadyTaskList ) / sizeof( struct COMMTASK ) - 1 ] , 0xFF , sizeof( struct COMMTASK ) );
	return 1;
}
//////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////
bit ZhkHopperTask( struct COMMTASK xdata* TaskTemp )
{
	uchar data StateTemp;
	
	switch( ZhkSchedulerState )
	{
	case TASK_NULL:
		break;
	case TASK_REDAY:
        //Trace( "ZhkHopperTaskBegin\n" );
		ZhkChannelSwitch( EQUIP_HOPPER );
		if( ZhkHopperTxMsg( TaskTemp ) )
			ZhkSchedulerState = TASK_WAIT;
		else
			ZhkSchedulerState = TASK_FINISH;
	    //Trace( "\n ZhkHopperTxMsgOK" );		
		ZhkDownMsgAckTimer  = 80;
		switch( TaskTemp->Id - ID_HOPPER1 )
		{
		case 0:
			Hopper1.CommState = COMM_BUZY;
			break;
		case 1:
			Hopper2.CommState = COMM_BUZY;
			break;
		case 2:
			Hopper3.CommState = COMM_BUZY;
			break;
        case ID_SELECTION1:
		    Selection1.CommState = COMM_BUZY;
			break;
        case ID_SELECTION2:
		    Selection2.CommState = COMM_BUZY;
			break;
		case ID_SELECTION3:
		    Selection3.CommState = COMM_BUZY;
			break;
		case ID_ACDCMODULE:
		    ACDCModule.CommState = COMM_BUZY;
		    break;
		default:
			ZhkSchedulerState = TASK_NULL; 
		}
		break;
	case TASK_WAIT:
        //Trace( "\n HOPPERTASK_WAIT" );
		StateTemp = ZhkHopperMsgProcess();
	    //Trace( "\n StateTemp = %bu", StateTemp );
		if ( StateTemp == 1 )
		{
			ZhkSchedulerState = TASK_FINISH;
			break;
		}
		if( ZhkDownMsgAckTimer == 0 )
		{
            //Trace( "\n HOPPERTASK_WAIT" );	
			//发送			
			if ( ZhkHopperTxMsg( TaskTemp ) )
				ZhkSchedulerState = TASK_REWAIT;
			else
				ZhkSchedulerState = TASK_FINISH;
			
			if( TaskTemp->Cmd == HOPPER_OUTPUT )
				ZhkDownMsgAckTimer  = 80;           //DOWNMSGACKTIMEOUT;
			else if( TaskTemp->Cmd == HOPPER_QUERY )
				ZhkDownMsgAckTimer  = 80;
		}		
		break;
	case TASK_REWAIT:
	    //Trace( "\n HOPPER RETASK_WAIT" );
		StateTemp = ZhkHopperMsgProcess();
		if ( StateTemp == 1 )
		{
			ZhkSchedulerState = TASK_FINISH;
			break;
		}
		if ( ZhkDownMsgAckTimer == 0 )
		{
			ZhkSchedulerState = TASK_ERROR;
		}
		break;
	case TASK_FINISH:
        //Trace( "\n HOPPER TASK_FINISH" );		
		switch( TaskTemp->Id - ID_HOPPER1 )
		{
		case 0:
			Hopper1.CommState = COMM_COMMOK;
			break;
		case 1:
			Hopper2.CommState = COMM_COMMOK;
			break;
		case 2:
			Hopper3.CommState = COMM_COMMOK;
			break;
		case ID_SELECTION1:
		    Selection1.CommState = COMM_COMMOK;
			break;
        case ID_SELECTION2:
		    Selection2.CommState = COMM_COMMOK;
			break;
		case ID_SELECTION3:
		    Selection3.CommState = COMM_COMMOK;
			break;
		case ID_ACDCMODULE:
		    ACDCModule.CommState = COMM_COMMOK;
		    break;
		default:
			ZhkSchedulerState = TASK_NULL; 
		}
		ZhkSchedulerState = TASK_NULL; 
		break;
	case TASK_ERROR:
		switch( TaskTemp->Id - ID_HOPPER1 )
		{
		case 0:
			Hopper1.CommState = COMM_TIMEOUT;  //| COMM_COMMOK;
			break;
		case 1:
			Hopper2.CommState = COMM_TIMEOUT;  //| COMM_COMMOK;
			break;
		case 2:
			Hopper3.CommState = COMM_TIMEOUT;  //| COMM_COMMOK;
			break;
		case ID_SELECTION1:
		    Selection1.CommState = COMM_TIMEOUT;
			break;
        case ID_SELECTION2:
		    Selection2.CommState = COMM_TIMEOUT;
			break;
		case ID_SELECTION3:
		    Selection3.CommState = COMM_TIMEOUT;
			break;
		case ID_ACDCMODULE:
		    ACDCModule.CommState = COMM_TIMEOUT;
		    break;
		default:
			ZhkSchedulerState = TASK_NULL; 
		}
		ZhkSchedulerState = TASK_NULL; 
		break;
	default:
		ZhkSchedulerState = TASK_NULL;
	}
	return 1;
}


bit ZhkLcdTask( struct COMMTASK xdata* TaskTemp )
{
	uchar data StateTemp;
	
	switch( ZhkSchedulerState )
	{
	case TASK_NULL:
		break;
	case TASK_REDAY:
		ZhkChannelSwitch( EQUIP_LCD );
		if ( ZhkLcdTxMsg( TaskTemp ) )
			ZhkSchedulerState = TASK_WAIT;
		else
			ZhkSchedulerState = TASK_FINISH;
		ZhkDownMsgAckTimer = 60;
		Lcd.CommState   = COMM_BUZY;
		break;
	case TASK_WAIT:
		StateTemp = ZhkLcdMsgProcess();
		if ( StateTemp == 1 )
		{		
			ZhkSchedulerState = TASK_FINISH;
			break;
		}
		if ( ZhkDownMsgAckTimer == 0 )
		{
			Lcd.Sn --;
			if ( ZhkLcdTxMsg( TaskTemp ) )							
				ZhkSchedulerState = TASK_REWAIT;
		
			else			
				ZhkSchedulerState = TASK_FINISH;			
			ZhkDownMsgAckTimer = DOWNMSGACKTIMEOUT;
		}
		
		break;
	case TASK_REWAIT:		
		StateTemp = ZhkLcdMsgProcess();
		if ( StateTemp == 1 )
		{
			ZhkSchedulerState = TASK_FINISH;
			break;
		}
		if ( ZhkDownMsgAckTimer == 0 )
		{
			ZhkSchedulerState = TASK_ERROR;
		}
		break;
	case TASK_FINISH:		
		ZhkSchedulerState = TASK_NULL; 
		Lcd.CommState     = COMM_COMMOK;
		Lcd.ExtState[1] = 0;
		break;
	case TASK_ERROR:
		Lcd.CommState  = COMM_TIMEOUT;// | COMM_COMMOK;
		ZhkSchedulerState = TASK_NULL; 
		Lcd.ExtState[1] = 1;
		break;
	default:
		ZhkSchedulerState = TASK_NULL;
	}
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////

bit ZhkHopperTxMsg( struct COMMTASK xdata* NewTask )
{
	uchar data HopperNum;
	struct DEVICE xdata* data CurrentHopper;  //Why?
	
	// 选择出币机构
	HopperNum = NewTask->Id - ID_HOPPER1;
	switch( HopperNum )
	{
	case 0:
		CurrentHopper = &Hopper1;
	/*	if( Hopper1.Sn < 255 )
			Hopper1.Sn++;
		else
			Hopper1.Sn = 0;*/
		break;
	case 1:
	//	Trace("\n ZhkHopperTxMsg HopperOutput2");
		CurrentHopper = &Hopper2;
	/*	if( Hopper2.Sn < 255 )
			Hopper2.Sn++;
		else
			Hopper2.Sn = 0;*/
		break;
	case 2:
		CurrentHopper = &Hopper3;
	/*	if( Hopper3.Sn < 255 )
			Hopper3.Sn++;
		else
			Hopper3.Sn = 0;*/
		break;
	//----------------------------------------------
	case ID_SELECTION1:
	    CurrentHopper = &Selection1;
		break;
    case ID_SELECTION2:
	    CurrentHopper = &Selection2;
		break;
	case ID_SELECTION3:
	    CurrentHopper = &Selection3;
		break;
	case ID_ACDCMODULE:
	    CurrentHopper = &ACDCModule;
	    break;
	//===============================================
	default:
		return 0;
	}
//	Trace("\n ZhkHopperTxMsg" );
	ZhkBusTxMsg( NewTask );		
	return 1;	
}

bit ZhkLcdTxMsg( struct COMMTASK xdata* NewTask )
{
	// 更新sn计数
	Lcd.Sn ++;
	if ( Lcd.Sn == 0 ) Lcd.Sn ++;
	NewTask->Sn = Lcd.Sn;
	ZhkBusTxMsg( NewTask );	
	
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////
uchar ZhkHopperMsgProcess( void )
{	
    struct COMMTASK xdata AckTask;
	memset( &AckTask, 0, sizeof( struct COMMTASK ) );
	memcpy( &AckTask, &CurrentTask, sizeof( struct COMMTASK ) );

    //Trace( "\n AckTask.Sn = %bu", AckTask.Sn );
	if ( ZhkBusFrameUnPack( &AckTask ) == 0 )
	{
		return 0;	
	}
	switch( CurrentTask.Id - ID_HOPPER1 )
	{
		case 0:
	        //Trace( "\n AckTask.State = %02bx", AckTask.State );
			Hopper1.State = 0;
			Hopper1.State = AckTask.State;		
			break;
		case 1:
			Hopper2.State = 0;
			Hopper2.State = AckTask.State;
			break;
		case 2:
			Hopper3.State = 0;
			Hopper3.State = AckTask.State;
			break;
		case ID_SELECTION1:
		    Selection1.State = 0;
			Selection1.State = AckTask.State;
			break;
		case ID_SELECTION2:
		    Selection2.State = 0;
			Selection2.State = AckTask.State;
			break;
		case ID_SELECTION3:
		    Selection3.State = 0;
			Selection3.State = AckTask.State;
			break;
		case ID_ACDCMODULE:
		    //ACDCModule.State = 0;
			ACDCModule.State = AckTask.State;
		    break;
	}
	
    //Trace( "\n ZhkHopperMsgProcess OK");
	switch( CurrentTask.Cmd  )
	{
		case HOPPER_OUTPUT:		
			break;
		case HOPPER_QUERY:
			switch( CurrentTask.Id - ID_HOPPER1 )
			{
			case 0:
				Hopper1.ExtState[ 0 ] = AckTask.Param[ 0 ];
				Hopper1.ExtState[ 1 ] = AckTask.Param[ 1 ];
				break;
			case 1:
				Hopper2.ExtState[ 0 ] = AckTask.Param[ 0 ];
				Hopper2.ExtState[ 1 ] = AckTask.Param[ 1 ];
				break;
			case 2:
				Hopper3.ExtState[ 0 ] = AckTask.Param[ 0 ];
				Hopper3.ExtState[ 1 ] = AckTask.Param[ 1 ];
				break;

			case ID_SELECTION1:
				Selection1.ExtState[ 0 ] = AckTask.Param[ 0 ];
				Selection1.ExtState[ 1 ] = AckTask.Param[ 1 ];
				break;
			case ID_SELECTION2:
				Selection2.ExtState[ 0 ] = AckTask.Param[ 0 ];
				Selection2.ExtState[ 1 ] = AckTask.Param[ 1 ];
				break;
			case ID_SELECTION3:
				Selection3.ExtState[ 0 ] = AckTask.Param[ 0 ];
				Selection3.ExtState[ 1 ] = AckTask.Param[ 1 ];
				break;
            case ID_ACDCMODULE:
				ACDCModule.ExtState[ 0 ] = AckTask.Param[ 0 ];
				ACDCModule.ExtState[ 1 ] = AckTask.Param[ 1 ];
				break;
			}
			break;
		default:
			return 0;
	}
    //Trace( "ZhkHopperMsgProcessOver\n" );
	return 1;	
}


uchar ZhkLcdMsgProcess( void )
{
	struct COMMTASK xdata AckTask;
	
	
	if ( ZhkBusFrameUnPack( &AckTask ) == 0 )
		return 0;
	if ( CurrentTask.Id  != AckTask.Id  ) 
		return 0;
	if ( CurrentTask.Sn  != AckTask.Sn  ) 
		return 0;
	Lcd.State = AckTask.State;
	switch( CurrentTask.Cmd  )
	{
		case DISPKEY_DISP:		
		case DISPKEY_DISPS:
		case DISPKEY_SETCURSOR:
		case DISPKEY_CURSORSTYLE:
		case DISPKEY_DENOTE:
		case DISPKEY_SCROLL:
		case DISPKEY_CLEAR:
		case DISPKEY_LANGUAGE:
			break;		
		default:
			return 0;
	}
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

uchar TestDeviceBuzy( struct DEVICE xdata* Device )
{
	return( Device->CommState & COMM_BUZY );
}

uchar TestDeviceTimeOut( struct DEVICE xdata* Device )
{
	uchar data ch;

	ch = Device->CommState;
	Device->CommState &= ~ COMM_TIMEOUT;

	return( ch & COMM_TIMEOUT );
}

uchar TestDeviceException( struct DEVICE xdata* Device )
{
	if( ( Device == &Hopper1 ) || ( Device == &Hopper2 ) || ( Device == &Hopper3 ) )
	{		
		if( ( Device->State & 0x16 ) || ( Device->State == 0 ) )
			return 0;//正常
		else
			return 1;//故障
	}
	else
		return( Device->State & 0x40 );
}

uchar TestDeviceCommOK( struct DEVICE xdata* Device )
{
	uchar data ch;
	
	ch = Device->CommState;
	Device->CommState &= ~ COMM_COMMOK;
	return( ch & COMM_COMMOK );
}

////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                        
//         出币机构接口函数
//
////////////////////////////////////////////////////////////////////////////////////////////
bit Selection1Query( void )
{
	struct COMMTASK xdata TaskTemp;

	TaskTemp.Id 		  = ID_SELECTION1;
	TaskTemp.Cmd 		  = HOPPER_QUERY;
	TaskTemp.Sn		      = Selection1.Sn;
	TaskTemp.Priority     = PRIORITY_NORMAL;
	TaskTemp.ParamLen	  = 2;

    //if( HardWareErr > 0 )
    //只有暂停服务模式，才关闭指示灯;by gzz 20110727
    if( ( HardWareErr > 0 ) && (sysVPMission.SystemState != 1) )
	{
	    TaskTemp.Param[0] = 0; 
        TaskTemp.Param[1] = 0xff;   
	}
	else
	{
		TaskTemp.Param[0] = sysVPMission.sel1ReadyLed;
		TaskTemp.Param[1] = sysVPMission.sel1ErrLed;
    }

	return ZhkSchedulerAddTask( &TaskTemp );	
}

bit Selection2Query( void )
{
	struct COMMTASK xdata TaskTemp;

	TaskTemp.Id 		  = ID_SELECTION2;
	TaskTemp.Cmd 		  = HOPPER_QUERY;
	TaskTemp.Sn		      = Selection2.Sn;
	TaskTemp.Priority     = PRIORITY_NORMAL;
	TaskTemp.ParamLen	  = 2;

	//if( HardWareErr > 0 )
    //只有暂停服务模式，才关闭指示灯;by gzz 20110727
    if( ( HardWareErr > 0 ) && (sysVPMission.SystemState != 1) )
	{
	    TaskTemp.Param[0] = 0; 
        TaskTemp.Param[1] = 0xff;   
	}
	else
	{
		TaskTemp.Param[0] = sysVPMission.sel2ReadyLed;
		TaskTemp.Param[1] = sysVPMission.sel2ErrLed;
    }
	return ZhkSchedulerAddTask( &TaskTemp );	
}

bit Selection3Query( void )
{
	struct COMMTASK xdata TaskTemp;

	TaskTemp.Id 		  = ID_SELECTION3;
	TaskTemp.Cmd 		  = HOPPER_QUERY;
	TaskTemp.Sn		      = Selection3.Sn;
	TaskTemp.Priority     = PRIORITY_NORMAL;
	TaskTemp.ParamLen	  = 2;

	//if( HardWareErr > 0 )
    //只有暂停服务模式，才关闭指示灯;by gzz 20110727
    if( ( HardWareErr > 0 ) && (sysVPMission.SystemState != 1) )
	{
	    TaskTemp.Param[0] = 0; 
        TaskTemp.Param[1] = 0xff;   
	}
	else
	{
		TaskTemp.Param[0] = sysVPMission.sel3ReadyLed;
		TaskTemp.Param[1] = sysVPMission.sel3ErrLed;
    }

	return ZhkSchedulerAddTask( &TaskTemp );	
}

bit SelectionQuery( uchar itemNum )
{
	switch( itemNum )
	{
	 case 1:
		return Selection1Query();
		break;
	 case 2:
		return Selection2Query();
		break;
	 case 3:
		return Selection3Query();	
		break;
     case 4:
        return ACDCModuleQuery();
        break;
	}
	return 1;
}


bit ACDCModuleQuery( void )
{
	struct COMMTASK xdata TaskTemp;
	TaskTemp.Id 	  = ID_ACDCMODULE;
	TaskTemp.Cmd      = HOPPER_QUERY;
	TaskTemp.Sn		  = ACDCModule.Sn;
	TaskTemp.Priority = PRIORITY_NORMAL;
	TaskTemp.ParamLen = 2;
    /*
	if( HardWareErr > 0 )
	{
	    TaskTemp.Param[0] = 0; 
        TaskTemp.Param[1] = 0;   
	}
	else
	*/
	{
		TaskTemp.Param[0] = sysVPMission.ACDCLedCtr;
		TaskTemp.Param[1] = sysVPMission.ACDCCompressorCtr;
    }
	return ZhkSchedulerAddTask( &TaskTemp );	
}


bit ZhkHopper1Query( void )
{
	struct COMMTASK xdata TaskTemp;

	TaskTemp.Id 		= ID_HOPPER1;
	TaskTemp.Cmd 		= HOPPER_QUERY;
	TaskTemp.Sn		= Hopper1.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 2;
	TaskTemp.Param[ 0 ] 	= 0;
	TaskTemp.Param[ 1 ] 	= 0;
	return ZhkSchedulerAddTask( &TaskTemp );	
}

bit ZhkHopper2Query( void )
{
	struct COMMTASK xdata TaskTemp;

	TaskTemp.Id 		= ID_HOPPER2;
	TaskTemp.Cmd 		= HOPPER_QUERY;
	TaskTemp.Sn		= Hopper2.Sn;
	TaskTemp.Priority = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 2;
	TaskTemp.Param[ 0 ] 	= 0;
	TaskTemp.Param[ 1 ] 	= 0;
	return ZhkSchedulerAddTask( &TaskTemp );	
}

bit ZhkHopper3Query( void )
{
	struct COMMTASK xdata TaskTemp;

	TaskTemp.Id 		= ID_HOPPER3;
	TaskTemp.Cmd 		= HOPPER_QUERY;
	TaskTemp.Sn		= Hopper3.Sn;
	TaskTemp.Priority = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 2;
	TaskTemp.Param[ 0 ] 	= 0;
	TaskTemp.Param[ 1 ] 	= 0;
	return ZhkSchedulerAddTask( &TaskTemp );	
}

bit HopperQuery( uchar HopperNum )
{
	switch( HopperNum )
	{
	case 1:
		return ZhkHopper1Query();
		break;
	case 2:
		return ZhkHopper2Query();
		break;
	 case 3:
		return ZhkHopper3Query();	
		break;
	}
	return 1;
}


//---------------------------------------------
//Clear SN
bit ZhkHopper1ClearSn( void )
{
	struct COMMTASK xdata TaskTemp;
    
	Hopper1.Sn = 0;
	TaskTemp.Id    = ID_HOPPER1;
	TaskTemp.Cmd   = HOPPER_OUTPUT;
	TaskTemp.Sn	   = Hopper1.Sn;
	TaskTemp.Priority   = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 2;
	TaskTemp.Param[ 0 ] = 0;
	TaskTemp.Param[ 1 ] = 0;
	return ZhkSchedulerAddTask( &TaskTemp );	
}

bit ZhkHopper2ClearSn( void )
{
	struct COMMTASK xdata TaskTemp;

	Hopper2.Sn = 0;
	TaskTemp.Id 	= ID_HOPPER2;
	TaskTemp.Cmd 	= HOPPER_OUTPUT;
	TaskTemp.Sn		= Hopper2.Sn;
	TaskTemp.Priority = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 2;
	TaskTemp.Param[ 0 ] 	= 0;
	TaskTemp.Param[ 1 ] 	= 0;
	return ZhkSchedulerAddTask( &TaskTemp );	
}

bit ZhkHopper3ClearSn( void )
{
	struct COMMTASK xdata TaskTemp;

	Hopper3.Sn = 0;
	TaskTemp.Id 	= ID_HOPPER3;
	TaskTemp.Cmd 	= HOPPER_OUTPUT;
	TaskTemp.Sn		= Hopper3.Sn;
	TaskTemp.Priority = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 2;
	TaskTemp.Param[ 0 ] 	= 0;
	TaskTemp.Param[ 1 ] 	= 0;
	return ZhkSchedulerAddTask( &TaskTemp );	
}

bit HopperClearSn( uchar HopperNum )
{
	switch( HopperNum )
	{
	case 1:
		return ZhkHopper1ClearSn();
		break;
	case 2:
		return ZhkHopper2ClearSn();
		break;
	 case 3:
		return ZhkHopper3ClearSn();	
		break;
	}
	return 1;
}
//====================================================


bit HopperOutput( uchar HopperNum , uint CoinAmount )
{
	struct COMMTASK xdata TaskTemp;
//	Trace("\nHopperOutput");
	switch( HopperNum )
	{
	case 1:
		if ( Hopper1.CommState & COMM_BUZY ) 
			return 0;
	//	Hopper1.CommState = COMM_BUZY;
		if ( HopperNum > 8 ) 
			return 0;

		if( Hopper1.Sn < 255 )
			Hopper1.Sn++;
		else
			Hopper1.Sn = 0;

		TaskTemp.Id 		= ID_HOPPER1;
		TaskTemp.Cmd 		= HOPPER_OUTPUT ;
		TaskTemp.Sn		= Hopper1.Sn;
		TaskTemp.Priority       = PRIORITY1;
		TaskTemp.ParamLen	= 2;
		TaskTemp.Param[ 0 ] 	= CoinAmount % 256;
		TaskTemp.Param[ 1 ] 	= CoinAmount / 256;

		return ZhkSchedulerAddTask( &TaskTemp );
		break;
	case 2:			
		if ( Hopper2.CommState & COMM_BUZY ) 
			return 0;
	//	Hopper2.CommState = COMM_BUZY;
		if ( HopperNum > 8 ) 
			return 0;

		 if( Hopper2.Sn < 255 )
			Hopper2.Sn++;
		else
			Hopper2.Sn = 0;

		TaskTemp.Id 		= ID_HOPPER2;
		TaskTemp.Cmd 		= HOPPER_OUTPUT ;
		TaskTemp.Sn		= Hopper2.Sn;
		TaskTemp.Priority       = PRIORITY1;
		TaskTemp.ParamLen	= 2;
		TaskTemp.Param[ 0 ] 	= CoinAmount % 256;
		TaskTemp.Param[ 1 ] 	= CoinAmount / 256;
	//	Trace("\n HopperOutput2");
		return ZhkSchedulerAddTask( &TaskTemp );	
		break;
	case 3:
		if ( Hopper3.CommState & COMM_BUZY ) 
			return 0;
//		Hopper3.CommState = COMM_BUZY;
		if ( HopperNum > 8 ) 
			return 0;

		if( Hopper3.Sn < 255 )
			Hopper3.Sn++;
		else
			Hopper3.Sn = 0;

		TaskTemp.Id 		= ID_HOPPER3;
		TaskTemp.Cmd 		= HOPPER_OUTPUT ;
		TaskTemp.Sn		= Hopper3.Sn;
		TaskTemp.Priority       = PRIORITY1;
		TaskTemp.ParamLen	= 2;
		TaskTemp.Param[ 0 ] 	= CoinAmount % 256;
		TaskTemp.Param[ 1 ] 	= CoinAmount / 256;
		return ZhkSchedulerAddTask( &TaskTemp );	
	 	break;
	}
}

uint HopperGetCoinOut( uchar HopperNum )
{
	uint data ch;
	
	switch( HopperNum )
	{
	case 1:
		ch = Hopper1.ExtState[ 1 ] * 256 + Hopper1.ExtState[ 0 ];
		break;
	case 2:
		ch = Hopper2.ExtState[ 1 ] * 256 + Hopper2.ExtState[ 0 ];
		break;
	case 3:
		ch = Hopper3.ExtState[ 1 ] * 256 + Hopper3.ExtState[ 0 ];
		break;
	}
	return ch;
}
////////////////////////////////////////////////////////////////////////////////////////////
//
//       辅助模块hub
//
////////////////////////////////////////////////////////////////////////////////////////////
/*
bit DenoteLedLightOn( void )
{
	struct COMMTASK xdata TaskTemp;
	
	TaskTemp.Id 		= ID_HUB;
	TaskTemp.Cmd 		= HUB_DENOTE;
	TaskTemp.Sn		= Hub.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 1;
	TaskTemp.Param[ 0 ] 	= 1;
	return ZhkSchedulerAddTask( &TaskTemp );
}

bit DenoteLedLightOff( void )
{
	struct COMMTASK xdata TaskTemp;
	
	TaskTemp.Id 		= ID_HUB;
	TaskTemp.Cmd 		= HUB_DENOTE;
	TaskTemp.Sn		= Hub.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 1;
	TaskTemp.Param[ 0 ] 	= 0;
	return ZhkSchedulerAddTask( &TaskTemp );
}


bit SetAlertOn( void )
{
	struct COMMTASK xdata TaskTemp;
	
	TaskTemp.Id 		= ID_HUB;
	TaskTemp.Cmd 		= HUB_BUZZER;
	TaskTemp.Sn		= Hub.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 1;
	TaskTemp.Param[ 0 ] 	= 1;
	return ZhkSchedulerAddTask( &TaskTemp );
}

bit SetAlertOff( void )
{
	struct COMMTASK xdata TaskTemp;
	
	TaskTemp.Id 		= ID_HUB;
	TaskTemp.Cmd 		= HUB_BUZZER;
	TaskTemp.Sn		= Hub.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 1;
	TaskTemp.Param[ 0 ] 	= 0;
	return ZhkSchedulerAddTask( &TaskTemp );
}
*/
////////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////////
bit DisplaycharacterSet( uint CharSetCode )
{
/*	uchar code* CharSetCodeTable[] = 
	{
	"GB2312" ,
	"WIN1252"
	};*/
	struct COMMTASK xdata TaskTemp;
	
	memset( &TaskTemp, 0, sizeof(struct COMMTASK) );	
	TaskTemp.Id 		= ID_LCD;
	TaskTemp.Cmd 		= DISPKEY_LANGUAGE;
	TaskTemp.Sn		= Lcd.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	switch( CharSetCode )
	{
	case CS_GB2312:
		memcpy( TaskTemp.Param, "GB2312", 6 );
//		strcpy( TaskTemp.Param , CharSetCodeTable[ 0 ] );
		TaskTemp.ParamLen = 6;	//strlen( CharSetCodeTable[ 0 ] );
		break;
	case CS_WIN1252:		
//		strcpy( TaskTemp.Param , CharSetCodeTable[ 1 ] );
		memcpy( TaskTemp.Param, "WIN1252", 7 );
		TaskTemp.ParamLen = 7; //strlen( CharSetCodeTable[ 1 ] );
		break;
	default:
		return 0;
	}

	return ZhkSchedulerAddTask( &TaskTemp );
}

bit DisplayClear( void )
{
	struct COMMTASK xdata TaskTemp;
	
	memset( &TaskTemp, 0, sizeof(struct COMMTASK) );	
	TaskTemp.Id 		= ID_LCD;
	TaskTemp.Cmd 		= DISPKEY_CLEAR;
	TaskTemp.Sn		= Lcd.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 0;

	return ZhkSchedulerAddTask( &TaskTemp );
	
	
}

bit DisplayString( uchar * Str , const uchar len )
{
	struct COMMTASK xdata TaskTemp;

	memset( &TaskTemp, 0, sizeof(struct COMMTASK) );	
	TaskTemp.Id 		= ID_LCD;
	TaskTemp.Cmd 		= DISPKEY_DISP;
	TaskTemp.Sn		= Lcd.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= len;
	memcpy( TaskTemp.Param , Str , len );

	return ZhkSchedulerAddTask( &TaskTemp );
	
}
bit DisplayPosString( const uchar PosX , const uchar PosY , uchar * Str , const uchar len )
{
	struct COMMTASK xdata TaskTemp;
	
	memset( &TaskTemp, 0, sizeof(struct COMMTASK) );
	//Trace( "DisplayPosString( %bd , %bd , % bd )\n" , PosX , PosY , len );
	TaskTemp.Id 		= ID_LCD;
	TaskTemp.Cmd 		= DISPKEY_DISPS;
	TaskTemp.Sn		= Lcd.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= len + 2;
	TaskTemp.Param[ 0 ] 	= PosX;
	TaskTemp.Param[ 1 ] 	= PosY;
	memcpy( TaskTemp.Param + 2 , Str , len );

	return ZhkSchedulerAddTask( &TaskTemp );
}

bit DisplaySetCursorStyle( uchar style )
{
	struct COMMTASK xdata TaskTemp;

	memset( &TaskTemp, 0, sizeof(struct COMMTASK) );	
	TaskTemp.Id 		= ID_LCD;
	TaskTemp.Cmd 		= DISPKEY_CURSORSTYLE;
	TaskTemp.Sn		= Lcd.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 1;
	TaskTemp.Param[ 0 ] 	= style;

	return ZhkSchedulerAddTask( &TaskTemp );
}

bit DisplaySetCursorPos( uchar PosX , uchar PosY )
{
	struct COMMTASK xdata TaskTemp;

	memset( &TaskTemp, 0, sizeof(struct COMMTASK) );	
	TaskTemp.Id 		= ID_LCD;
	TaskTemp.Cmd 		= DISPKEY_SETCURSOR;
	TaskTemp.Sn		= Lcd.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 2;
	TaskTemp.Param[ 0 ] 	= PosX;
	TaskTemp.Param[ 1 ] 	= PosY;

	return ZhkSchedulerAddTask( &TaskTemp );
		
}

bit DisplayBkLightOn( void )
{
	struct COMMTASK xdata TaskTemp;

	TaskTemp.Id 		= ID_LCD;
	TaskTemp.Cmd 		= DISPKEY_DENOTE;
	TaskTemp.Sn		= Lcd.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 1;
	TaskTemp.Param[ 0 ] 	= 1;
	return ZhkSchedulerAddTask( &TaskTemp );
	
}

bit DisplayBkLightOff( void )
{
	struct COMMTASK xdata TaskTemp;

	TaskTemp.Id 		= ID_LCD;
	TaskTemp.Cmd 		= DISPKEY_DENOTE;
	TaskTemp.Sn		= Lcd.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 1;
	TaskTemp.Param[ 0 ] 	= 0;
	return ZhkSchedulerAddTask( &TaskTemp );
}

uchar IsBackLightOn( void )
{
	return Lcd.State & 0x10;
}

uchar GetDisplayState()
{
 	return Lcd.ExtState[1];
}

////////////////////////////////////////////////////////////////////////////////////////////
//纸币器处理
////////////////////////////////////////////////////////////////////////////////////////////
/*
bit ZhkCasherTask( struct COMMTASK xdata* TaskTemp )
{
	uchar xdata StateTemp = 0;
	//Trace( "ZhkCasherTask\n" );
	//Trace( "ZhkSchedulerState=%d\n",(int)ZhkSchedulerState );
	switch( ZhkSchedulerState )
	{
	case TASK_NULL:
		break;
	case TASK_REDAY:		
		ZhkChannelSwitch( EQUIP_CASHER );
		if( Casher.ExtState[ 14 ] == 0 )
			StateTemp = ZhkCasherTxMsg( TaskTemp, 1 );//改换纸币器数据包的ACK
		else
			StateTemp = ZhkCasherTxMsg( TaskTemp, 0 );//不改换纸币器数据包的ACK
		if ( StateTemp )
			ZhkSchedulerState = TASK_WAIT;
		else
			ZhkSchedulerState = TASK_FINISH;	
		
		ZhkDownMsgAckTimer  = 90;//DOWNMSGACKTIMEOUT;
		Casher.CommState = COMM_BUZY;
		break;
	case TASK_WAIT:	
//		Trace( "\n ZhkCasherTask TASK_WAIT" );	
		StateTemp = ZhkCasherMsgProcess();
		if ( StateTemp == 1 )
		{
			ZhkSchedulerState = TASK_FINISH;
			break;
		}
		if ( ZhkDownMsgAckTimer == 0 )
		{
			if ( ZhkCasherTxMsg( TaskTemp, 0 ) )
				ZhkSchedulerState = TASK_REWAIT;
			else
				ZhkSchedulerState = TASK_FINISH;
		//	ZhkSchedulerState  = TASK_REWAIT;
			ZhkDownMsgAckTimer = 90; //DOWNMSGACKTIMEOUT;
		}		
		break;
	case TASK_REWAIT:
//		Trace( "\n ZhkCasherTask TASK_REWAIT" );		
		StateTemp = ZhkCasherMsgProcess();
		if ( StateTemp == 1 )
		{
			ZhkSchedulerState = TASK_FINISH;
			break;
		}
		if ( ZhkDownMsgAckTimer == 0 )
		{
			ZhkSchedulerState = TASK_ERROR;
		}
		break;
	case TASK_FINISH:	
		Casher.CommState  = COMM_COMMOK;
		memset( &CurrentTask, 0, sizeof( struct COMMTASK ) );
		ZhkSchedulerState = TASK_NULL; 
		ZhkCasherStatus = CASHER_STATUS_NULL;
		Casher.ExtState[ 14 ] = 0; 
		Casher.ExtState[13] = 0;
		QueryCashTimer = 0;
		break;
	case TASK_ERROR:	
//		Trace( "\n ZhkCasherTask TASK_ERROR" );
		Casher.CommState = COMM_TIMEOUT;// | COMM_COMMOK;
		ZhkSchedulerState   = TASK_NULL; 
		ZhkCasherStatus = CASHER_STATUS_NULL;

		if( Casher.ExtState[ 14 ] < 240 )
			Casher.ExtState[ 14 ] ++;
		if( Casher.ExtState[ 14 ] >= 2 )
		{
			QueryCashTimer = 120;//定时二分钟
			Casher.ExtState[ 14 ] = 0;
			CasherDisableCasher();
			Casher.State	=  0 ;
			Casher.ExtState[ 0 ] = 0;
			Casher.ExtState[ 1 ] = 0 ;			
			Trace( "\n Casher outtime, Close query" );
		}
		memset( &CurrentTask, 0, sizeof( struct COMMTASK ) );
		break;
	default:
		ZhkSchedulerState = TASK_NULL;
	}	
	return 1;
}

bit ZhkCasherTxMsg( struct COMMTASK xdata* TaskTemp, uchar xdata Flag )
{
	switch( TaskTemp->Cmd )
	{
	case CASHER_PERMISSION:
		break;
	case CASHER_ACCEPT:
		// 接收纸币
		ZhkCasherAcceptCash();
		break;
	case CASHER_REJECT:
		// 拒收纸币
		ZhkCasherRejectCash();
		break;
	case CASHER_QUERY:
		// 查询纸币器状态
		ZhkCasherGetState( Flag );
		break;
	case CASHER_DISABLE:
		//禁能纸币器指令
		CasherDisableCasher();
		break;
	default:
		return  0;
	}
	return 1;	
}

uchar ZhkCasherMsgProcess( void )
{
	struct COMMTASK xdata AckTask;
	
	if ( ZhkCasherFrameUnPack( &AckTask ) == 0 )
 		return 0;
	Casher.State        = AckTask.State;
	Casher.ExtState[ 0 ] = AckTask.Param[ 0 ];
	Casher.ExtState[ 1 ] = AckTask.Param[ 1 ];
//	if ( CasherIsCashIn() )
 //		ZhkCasherStatus = CASHER_STATUS_NULL;
	return 1;
	
}
*/
bit ZhkCasherQuery( void )
{
	struct COMMTASK xdata TaskTemp;

	ZhkCasherStatus 	= CASHER_STATUS_QUERY;	
	TaskTemp.Id 		= ID_CASHER;
	TaskTemp.Cmd 		= CASHER_QUERY;
	TaskTemp.Sn			= Casher.Sn;
	TaskTemp.Priority   = PRIORITY4;
	TaskTemp.ParamLen	= 0;
//	Trace("\n check CashState");
	return ZhkSchedulerAddTask( &TaskTemp );
}

bit CasherQuery( void )
{	
//	if ( Casher.CommState == COMM_BUZY ) 
//		return 0;	
//	if( ZhkCasherStatus == CASHER_STATUS_NULL )
//	{			
		ZhkCasherQuery();
		return 1;
//	}	
//	return 0;
}

bit CasherAccept( void )
{
	struct COMMTASK xdata TaskTemp;

	ZhkCasherStatus = CASHER_STATUS_ACCEPT;
	TaskTemp.Id 		= ID_CASHER;
	TaskTemp.Cmd 		= CASHER_ACCEPT;
	TaskTemp.Sn		= Casher.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 0;
	return ZhkSchedulerAddTask( &TaskTemp );
}

bit CasherReject( void )
{
	struct COMMTASK xdata TaskTemp;
	
	Trace("\n CasherReject");
	ZhkCasherStatus = CASHER_STATUS_REJECT;
	TaskTemp.Id 		= ID_CASHER;
	TaskTemp.Cmd 		= CASHER_REJECT;
	TaskTemp.Sn		= Casher.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL; 
	TaskTemp.ParamLen	= 0;
	return ZhkSchedulerAddTask( &TaskTemp );
}

uchar CasherIsCashIn( void )
{
	return Casher.State & 0x10;
}

uchar CasherIsAcceptOK( void )
{
	return Casher.State & 0x20;
}

//退币是否成功
uchar CasherIsRejectOK( void )
{
	return Casher.State & 0x80;
}

uint GetCash( void )
{
	uchar data ch;

	ch = Casher.ExtState[ 1 ] % 8;
	Casher.ExtState[ 1 ] = 0;
	if ( ch == 0 ) 
		return 0;
	return CashAccept[ ch - 1 ];
}

uint GetComingCash( void )
{
	uchar data ch = 0;

	ch = Casher.ExtState[ 1 ] % 8;
	Casher.ExtState[ 1 ] = 0;
    /*
	if ( ( ch == 0 ) || ( CashAcceptEnableTable[ch - 1] != 1 ) )
		return 0;
	return CashAccept[ ch - 1 ];
    */
    if ( ch == 0 ) 
		return 0;
	return SystemParameter.billValue[ch-1];   //Changed by Andy on 2010.8.17!
}

bit CasherQueryEnable( void )
{
	ZhkAutoTaskPermission[ 0 ] = 1;
	memset( &Casher, 0, sizeof( Casher ) );
	return 1;
}

void CasherGetMachineState( uchar state )
{
	Casher.ExtState[ 15 ] = state;
}

uchar CasherRealTimeState( )
{
	return Casher.ExtState[ 13 ];
}


bit CasherQueryDisable( void )
{
	ZhkAutoTaskPermission[ 0 ] = 0;
	return 1;
}

void CasherResetOuttime( void )
{	
	Casher.CommState  = COMM_COMMOK;	
	QueryCashTimer = 0;
}

//////////////////////////////////////////////////////////
///货道处理
//////////////////////////////////////////////////////////
bit ZhkChannelTask( struct COMMTASK xdata* TaskTemp )
{
	u_char data StateTemp;

	#ifdef   _SJJ_//升降机
		return 0;		
	#endif
	
	switch( ZhkSchedulerState )
	{
	case TASK_NULL:
		break;
	case TASK_REDAY:		
		ZhkChannelSwitch( EQUIP_CHANNEL );		
		if ( ZhkChannelTxMsg( TaskTemp ) )
			ZhkSchedulerState = TASK_WAIT;
		else
			ZhkSchedulerState = TASK_FINISH;
		
		ZhkDownMsgAckTimer  = 800;
		Channel.CommState = COMM_BUZY;
		Channel.ExtState[2] = 0;
		break;	
	case TASK_WAIT:
		StateTemp = ZhkChannelMsgProcess();
		if ( StateTemp == 1 )
		{
			if( ( TaskTemp->Cmd == CHANNEL_EXEC ) || ( TaskTemp->Cmd == CHANNEL_TEST ) )
			{
			//等来了命令ACK，还要等命令执行结果啦
//				Trace("\n return command ACK");
				Channel.ExtState[2] = 1;
				ZhkSchedulerState = TASK_REWAIT;
				ZhkDownMsgAckTimer = 1500;				
			}	
			else
			{
				ZhkDownMsgAckTimer = 0;
				ZhkSchedulerState = TASK_FINISH;
			}
			break;
		}
		if ( ZhkDownMsgAckTimer == 0 )
		{
			if( ( TaskTemp->Cmd == CHANNEL_EXEC ) || ( TaskTemp->Cmd == CHANNEL_TEST ) )
			{
				//超时确没有命令ACK回应，只能重来了
//				Trace("\n no return command ACK");
				if( Channel.ExtState[2] == 0)
				{
					ZhkSchedulerState = TASK_ERROR;
					break;
				}
			}
			//如果是查询或清零命令，则可直接重试一次
			Channel.Sn --;
			if ( ZhkChannelTxMsg( TaskTemp ) )
			{
				ZhkSchedulerState = TASK_REWAIT;
				ZhkDownMsgAckTimer = 800;
			}
			else
				ZhkSchedulerState = TASK_FINISH;					
		}		
		break;		
	case TASK_REWAIT:
		StateTemp = ZhkChannelMsgProcess();
		if ( StateTemp == 1 )
		{
//			Trace("\n return mission ACK");
			if( ( TaskTemp->Cmd == CHANNEL_EXEC ) || ( TaskTemp->Cmd == CHANNEL_TEST ) )			
				Channel.ExtState[2] = 2;				
			ZhkSchedulerState = TASK_FINISH;
			break;
		}
		if ( ZhkDownMsgAckTimer == 0 )
		{
			ZhkSchedulerState = TASK_ERROR;
		}
		break;		
	case TASK_FINISH:
		ZhkSchedulerState = TASK_NULL; 		
		Channel.CommState   = COMM_COMMOK;
		memset( &CurrentTask, 0, sizeof( struct COMMTASK ) );
		ZhkDownMsgAckTimer = 0;
		break;
	case TASK_ERROR:
		Channel.CommState = COMM_TIMEOUT;// | COMM_COMMOK;
		ZhkSchedulerState   = TASK_NULL; 
		memset( &CurrentTask, 0, sizeof( struct COMMTASK ) );
		break;
	default:
		ZhkSchedulerState = TASK_NULL;
	}
	return 1;
}

bit ZhkChannelTxMsg( struct COMMTASK xdata* NewTask )
{
	u_char ChannelTemp;
	u_char sum;
	
//	Trace("\n ZhkChannelTxMsg" );
	ChannelTemp  = NewTask->Param[ 0 ] ;	
	
	if( ( NewTask->Cmd == CHANNEL_EXEC ) && ( NewTask->Param[1] == 1 ) )
		Channel.Sn--;
	if( Channel.Sn == 255 )
		Channel.Sn = 1;
	else
		Channel.Sn ++;

	ZhkSerial1PutCh( 0xED );
//	Trace(" ed" );
	sum  = 0xED;
	ZhkSerial1PutCh( 0x08 );
//	Trace(" 08" );
	sum ^= 0x08;
	if( NewTask->Cmd == CHANNEL_CLEAR )
	{
		ZhkSerial1PutCh( 0x00 );
//		Trace(" %02bx", 0x00 );
		sum ^= 0x00;
	}
	else
	{
		ZhkSerial1PutCh( Channel.Sn );
//		Trace(" %02bx", Channel.Sn );
		sum ^= Channel.Sn;		
		NewTask->Sn = Channel.Sn;
	}
	switch( NewTask->Cmd )
	{
		case CHANNEL_EXEC:	//命令ID	
			ZhkSerial1PutCh( 0x70 );
//			Trace(" 70" );
			sum ^= 0x70;
			ZhkSerial1PutCh( ChannelTemp );//货道编号		
			sum ^= ChannelTemp;
//			Trace(" %02bx", ChannelTemp );		
		break;	
		case CHANNEL_QUERY:	//命令ID	
			ZhkSerial1PutCh( 0x71 );
//			Trace(" 71" );
			sum ^= 0x71;
			ZhkSerial1PutCh( 0x00 );//货道编号
			sum ^= 0x00;		
			NewTask->Param[ 0 ] = 0;
//			Trace(" 00");			
		break;	
		case CHANNEL_CLEAR:	//命令ID	
			ZhkSerial1PutCh( 0x72 );
//			Trace(" 72" );
			sum ^= 0x72;
			ZhkSerial1PutCh( 0x00 );//货道编号
			sum ^= 0x00;
			NewTask->Param[ 0 ] = 0;
//			Trace(" 00");			
			Channel.Sn = 0;
			NewTask->Sn = 0;
		break;	
		case CHANNEL_TEST:	//命令ID	
			ZhkSerial1PutCh( 0x73 );
//			Trace(" 73" );
			sum ^= 0x73;
			ZhkSerial1PutCh( ChannelTemp );//货道编号
//			Trace(" %02bx", ChannelTemp );
			sum ^= ChannelTemp;		
		break;			
	default:
		return  0;
	}
	ZhkSerial1PutCh( 0x00 );		
    //Trace(" 00" );
	sum ^= 0x00;
	
    //----------------------------------------------
    //Changed for GOC dev, by Andy on 2010.10.21 
    //if( SystemParameter.GOCOn == 1 )
    if( (SystemParameter.GOCOn == 1)/*&&(DeviceStatus.GOCStatus==0x00)&&(SystemStatus==1)*/ )  //Changed by Andy 2011.7.11
    {
        
        if( (SystemStatus==1) )
        {
             if(DeviceStatus.GOCStatus==0x00)
             {
		    	 ZhkSerial1PutCh( 0x01 );
			     sum ^= 0x01;
             }
             //出货模块卡货时，每隔一段时间，予以检测;by cq 20110815
             else if((DeviceStatus.GOCStatus & 0x01) && (sysVPMission.GOCTestFlag == 1))
			 {
				 ZhkSerial1PutCh( 0x01 );//打开
				 sum ^= 0x01;
			 }  
             else
             {
             	 ZhkSerial1PutCh( 0x00 );
		    	 //Trace(" 00" );
				 sum ^= 0x00;
             }
        }
        else
        {
        	 ZhkSerial1PutCh( 0x01 );
		     sum ^= 0x01;               
        }
    }
    else
    {
    	ZhkSerial1PutCh( 0x00 );
    	//Trace(" 00" );
		sum ^= 0x00;
    }
    //==============================================

	ZhkSerial1PutCh( sum );
    //Trace(" %02bx", sum );
	return 1;	
}

u_char ZhkChannelMsgProcess( void )
{
	struct COMMTASK xdata AckTask;
	
	memcpy( &AckTask, &CurrentTask, sizeof( struct COMMTASK ) );
	if( ZhkChannelFrameUnPack( &AckTask ) == 0 ) 
		return 0;		
	if( AckTask.Cmd == CHANNEL_QUERY )//查询指令不能比较货道编号
	{
		Channel.State         = AckTask.State;
		Channel.ExtState[ 0 ] = AckTask.Param[ 0 ];//货道序列号
		Channel.ExtState[ 1 ] = AckTask.Param[ 1 ];//包序列号
		return 1;
	}
	
    //if( CurrentTask.Param[ 0 ] == AckTask.Param[ 0 ] )
	{
		Channel.State         = AckTask.State;
		Channel.ExtState[ 0 ] = AckTask.Param[ 0 ];//货道序列号
		Channel.ExtState[ 1 ] = AckTask.Param[ 1 ];//包序列号
	  //Channel.CommState     = COMM_COMMOK;
	}
    /*
	else
    {
		return 0;
    }
    */
	return 1;
	
}

////货道命令
//查询
bit ZhkChannelQuery( )
{
	struct COMMTASK xdata TaskTemp;
	
	Channel.CommState       = COMM_BUZY;
	TaskTemp.Id 		= ID_CHANNEL;
	TaskTemp.Cmd 		= CHANNEL_QUERY;
	TaskTemp.Sn		= Channel.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL; 
	TaskTemp.ParamLen	= 1;
	TaskTemp.Param[ 0 ]     = 0x00;
	return ZhkSchedulerAddTask( &TaskTemp );
}

//出货命令
bit ZhkChannelExec( u_char ChannelNum, u_char ReSendFlag )
{
	struct COMMTASK xdata TaskTemp;
	
	memset( &TaskTemp, 0, sizeof( struct COMMTASK ) );
	Channel.CommState       = COMM_BUZY;
	TaskTemp.Id 		= ID_CHANNEL;
	TaskTemp.Cmd 		= CHANNEL_EXEC;
	TaskTemp.Sn		= Channel.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL; 
	TaskTemp.ParamLen	= 2;
	TaskTemp.Param[ 0 ]     = ChannelNum;
	TaskTemp.Param[ 1 ]     = ReSendFlag;
	return ZhkSchedulerAddTask( &TaskTemp );
}

//清空SN命令
bit ZhkChannelClear( )
{
	struct COMMTASK xdata TaskTemp;
	
	Trace("\nZhkChannelClear" );
	Channel.CommState       = COMM_BUZY;
	TaskTemp.Id 		= ID_CHANNEL;
	TaskTemp.Cmd 		= CHANNEL_CLEAR;
	TaskTemp.Sn		= Channel.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL; 
	TaskTemp.ParamLen	= 1;
	TaskTemp.Param[ 0 ]     = 0x00;
	return ZhkSchedulerAddTask( &TaskTemp );
}

//检测货道命令
bit ZhkChannelTest( u_char ChannelNum )
{
	struct COMMTASK xdata TaskTemp;
	
	Channel.CommState       = COMM_BUZY;
	TaskTemp.Id 		= ID_CHANNEL;
	TaskTemp.Cmd 		= CHANNEL_TEST;
	TaskTemp.Sn		= Channel.Sn;
	TaskTemp.Priority       = PRIORITY_NORMAL; 
	TaskTemp.ParamLen	= 1;
	TaskTemp.Param[ 0 ]     = ChannelNum;
	return ZhkSchedulerAddTask( &TaskTemp );
}

//////////////////////////////////////////////////////////
///硬币器处理函数
//////////////////////////////////////////////////////////
//关闭硬币器电源
/*
bit CoinPowerOff( void )
{
	struct COMMTASK xdata TaskTemp;
	
	TaskTemp.Id 		= ID_HUB;
	TaskTemp.Cmd 		= HUB_COINPOWER;
	TaskTemp.Sn		    = Hub.Sn;
	TaskTemp.Priority   = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 1;
	TaskTemp.Param[ 0 ] = 0;
	Trace("\n CoinPowerOff()_AddTask");
	return ZhkSchedulerAddTask( &TaskTemp );
}
//打开硬币器电源
bit CoinPowerOn( void )
{
	struct COMMTASK xdata TaskTemp;
	
	TaskTemp.Id 		= ID_HUB;
	TaskTemp.Cmd 		= HUB_COINPOWER;
	TaskTemp.Sn		    = Hub.Sn;
	TaskTemp.Priority   = PRIORITY_NORMAL;
	TaskTemp.ParamLen	= 1;
	TaskTemp.Param[ 0 ] = 1;
	Trace("\n CoinPowerOn()_AddTask");
	return ZhkSchedulerAddTask( &TaskTemp );
	
}
*/
//////手机模块处理
uchar TestATcommand()
{
	uchar temp = 0;
	uchar TestTimer = 0;
#ifndef _DEBUG_NO_MOBILE
	IsPressMobile = 0;
	while( 1 )
	{
		temp = SendATcommandToMobile();
		if( temp == 3 )
		{
			IsPressMobile = 1;
			return 3;
		}
		if( temp == 2 )
		{
			SchedulerProcess();
			continue;
		}
		else
		{
			if( temp == 0 )
			{
				IsPressMobile = 1;
				return 0;			
			}
	 		if( TestTimer < 1 )
			{
				TestTimer++;
				continue;
			}
			else
			{
				IsPressMobile = 1;
				return 1;		
			}
		}
	}
	IsPressMobile = 1;
 #endif
 return 0;

}

/////////////////////////////////////////////
//HUB板通讯包处理
/*
bit ZhkHubTxMsg( struct COMMTASK xdata* NewTask )
{
	///	
	if( Hub.Sn < 255 )
		Hub.Sn ++;
	else
		Hub.Sn = 1;
	if ( Hub.Sn == 0 ) 
		Hub.Sn ++;
	NewTask->Sn = Hub.Sn;
	ZhkBusTxMsg( NewTask );	
	return 1;
}

uchar ZhkHubMsgProcess( void )
{
	struct COMMTASK xdata AckTask;

	if ( CtrlCoinPowerFrameUnPack( &AckTask ) == 0 )     //Changed by Andy Zhang on Jan 17,2007.
		return 0;
	if ( CurrentTask.Id  != AckTask.Id  ) 
		return 0;
	if ( CurrentTask.Sn  != AckTask.Sn  ) 
		return 0;
	Hub.State = AckTask.State;
	return 1;
}


bit ZhkHubTask( struct COMMTASK xdata* TaskTemp )
{
	uchar data StateTemp;
	//Trace( "ZhkHubTask\n" );
	//Trace( "ZhkSchedulerState=%d\n",(int)ZhkSchedulerState );
	switch( ZhkSchedulerState )
	{
	case TASK_NULL:
		break;
	case TASK_REDAY:
		Trace( "ZhkHubTaskBegin\n" );
		ZhkChannelSwitch( EQUIP_HUB );
		if ( ZhkHubTxMsg( TaskTemp ) )
			ZhkSchedulerState = TASK_WAIT;
		else
			ZhkSchedulerState = TASK_FINISH;
		Trace( "GUANYU ZhkSchedulerState = %bu\n, ZhkSchedulerState" );
		Trace( "ZhkHubTaskTxMsgOK\n" );
		ZhkDownMsgAckTimer = 60;//DOWNMSGACKTIMEOUT;
		Hub.CommState   = COMM_BUZY;
		break;
	case TASK_WAIT:
		StateTemp = ZhkHubMsgProcess();
		if ( StateTemp == 1 )
		{
			ZhkSchedulerState = TASK_FINISH;
			break;
		}
		if ( ZhkDownMsgAckTimer == 0 )
		{
			Hub.Sn --;	
			if ( ZhkHubTxMsg( TaskTemp ) )
				ZhkSchedulerState = TASK_WAIT;
			else
				ZhkSchedulerState = TASK_FINISH;
			ZhkDownMsgAckTimer = 60;//DOWNMSGACKTIMEOUT;
		}
		break;
	case TASK_REWAIT:
		StateTemp = ZhkHubMsgProcess();
		if ( StateTemp == 1 )
		{
			ZhkSchedulerState = TASK_FINISH;
			break;
		}
		if ( ZhkDownMsgAckTimer == 0 )
		{
			ZhkSchedulerState = TASK_ERROR;
		}
		break;
	case TASK_FINISH:
		ZhkSchedulerState = TASK_NULL; 
		Hub.CommState     = COMM_COMMOK;
		break;
	case TASK_ERROR:
		Hub.CommState  = COMM_TIMEOUT;
		ZhkSchedulerState = TASK_NULL; 
		break;
	default:
		ZhkSchedulerState = TASK_NULL;
	}
	return 1;
}
*/

void ZhkChannelSwitch( uchar type )
{
	//Trace( "ZhkChannelSwitch(%bd)\n" , type );
	ES1R = 0;
	switch( type )
	{
	case EQUIP_LCD:	
	//	ZhkHardSerialSwitch( 3 );
		ZhkBusSerialInit();
		DelayMs( 2 );
		ZhkHardSerialSwitch( 2 );
		DelayMs( 2 );
		break;
	case EQUIP_HOPPER:
  //case EQUIP_HUB:	
  //	ZhkHardSerialSwitch( 3 );
		ZhkBusSerialInit();
		DelayMs( 2 );
        /*
        if( sysVPMission.VPMode_Key == 1 )
            ZhkHardSerialSwitch( 2 );
        else
			ZhkHardSerialSwitch( 4 );
        */
        ZhkHardSerialSwitch( 4 );
		DelayMs( 2 );
		break;
	case EQUIP_CASHER:	
		//ZhkCasherSerialInit();
		DelayMs( 2 );
		ZhkHardSerialSwitch( 0 );
		DelayMs( 2 );
		break;
	case EQUIP_COINER:
		ZhkBusSerialInit();
		DelayMs( 2 );
        //ZhkHardSerialSwitch( 6 );//HUB板上J8口
		ZhkHardSerialSwitch( 1 );
		DelayMs( 2 );		
		break;	
	case EQUIP_POWER:
	case EQUIP_MODE:
		break;
	case EQUIP_CHANNEL:	//主板扩展口
        //ZhkHardSerialSwitch( 0 );
		ZhkBusSerialInit();
		DelayMs( 2 );
		ZhkHardSerialSwitch( 3 );
		DelayMs( 2 );
		break;		
	}
	ES1R = 1;
}
















