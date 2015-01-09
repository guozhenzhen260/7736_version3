//#undef _DEBUG_TRACE
#include "Debug.h"
#include "device.h"
//#include "global.h"
#include "mainflow.h"
#include "IOInput.h"
#include "DataExchange.h"
#include "scheduler.h"
#include "key.h"
#include "Clock.h"
#include "common.h"
#include "language.h"
#include "CommonFunction.h"
#include "string.h"
#include "timer.h"
#include "maintmenu.h"
#include "devicecheck.h"
#include "initialization.h"
#include "serial.h"
#include <math.h>
#include "procotol.h"
//#include "Mobile.h"
#include "communication.h"
#include "ShortMsg.h"
#include "VMC_PC.h"
#include "ITL.h"
#include "BusCard.h"
#include "Countermaint.h"//120419 by cq TotalSell



#define MAX_CHAR_NUMBER	20
#define DEFAULT_COMM_TIMEOUT  60
#define DEFAULT_INPUT_CODE_TIMEOUT 30//输入货道超时控制
#define DEFAULT_INSERT_BILL_TIMEOUT 40
#define DEFAULT_GET_BILL_TIMEOUT 10
#define DEFAULT_DISPENSE_COIN_TIMEOUT 30
#define LIGHT_BLINK_TIME 3	/* 出货灯闪烁次数，每次持续２秒钟 */
#define DEFAULT_PRESSKEY_TIMEOUT 15
#define __LINE0__	 0
#define __LINE1__	 1


struct DEVICESTATUS xdata DeviceStatus;
struct SYSTEMPARAMETER xdata SystemParameter;//
struct RackSet	xdata iRackSet[RACKNUM];//货架配置
struct RackColumnSet xdata iRackColumnSet[RACKNUM][RACKCOLUMNNUM]; //每层货道启用配置
struct WaySet  xdata GoodsWaySetVal[GOODSWAYNUM]; //各货道的参数设置
struct HopperSet xdata HopperSetList[HOPPERNUM];  //0存放最大面值,1存放第二大面值,依此类推
struct TRADECOUNTER xdata TradeCounter;
//struct TRADELOG xdata TradeLog[GOODSWAYNUM];//各货道的交易统计
struct COLUMNTEST xdata WayCheck;

u_char xdata COLUMN_NUM_SET = 0;

//----------------------------------------------------------------
//struct GoodsMatrix xdata sysGoodsMatrix[124];
struct VP_Mission  xdata sysVPMission;
struct ITL_Mission xdata sysITLMission;
struct BC_Mission  xdata sysBCMission;
struct MDB_Mission xdata sysMDBMission;
//================================================================

//u_char xdata CurrentTransType = 0; /* 1 = 正在交易， 0 = 空闲*/
//u_char xdata LightBlinkFlag = 0;

//是否有短信进来,0为无，1为有
u_char IsShortMessageIn;

//短信查询命令
//bit0为1表示是查设备状态
//bit1为1表示查询交易统计记录
//bit2为1表示已发送了开机自检或维护后自检警报
//bit6为1表示警报短信有分页,需要再次组短信包
//bit7为1表示查设备状态有分页,需要再次组短信包
u_char xdata SMSQuery = 0;

//货币设备报警验证字节，各bit为1表示对应的故障已报警,具体定义为：
//bit7验证纸币器，bit6验证硬币器
//bit0-bit1验证hopper1,bit0验证缺币、bit1验证设备故障,下面类推
//bit2-bit3验证hopper2
//bit4-bit5验证hopper3
unsigned char xdata  MoneyDevAlarm;

//其它故障报警验证字节，各bit为1表示对应的故障已报警,具体定义为：
//bit0验证暂停服务,bit1验证系统参数校验,bit2验证主板与显示板通讯
//bit3验证所有货道无货,bit4验证所有货道故障,
//bit5验证所有货道均不能正常工作,bit6验证主板与货道驱动板通讯故障
//bit7保留
unsigned char xdata  SystemErrorAlarm;//货道参数保存

//待发的短信队列
struct SMSMessageList xdata SMSMessage[MAX_SMS_LIST];
//短信序列号
//unsigned char xdata ListIndex = 0;

const struct StrAndLen code QueryCmd[]=    
{	
	 {"VMDEV", 5 }, //状态查询
	 {"VMREC", 5 }	//交易记录查询
};


u_char xdata CurrentStockCode;	//当前货道编号
u_char xdata CurrentInputStockCode;
u_int xdata  CurrentPayed = 0;
u_int xdata  CurrentDispenseCoin;		/*该变量用作出币总额*/
u_char xdata SystemStatus = 1;          //1-正常服务等待状态、2-暂停服务
u_char xdata DisplayBuffer [ MAX_CHAR_NUMBER ];
u_char xdata DisplayLine=0;
u_char xdata UpdataDisp;
u_char xdata goodsCodeBack;

extern u_char xdata ZhkKeyPermission;

/*
故障位定义： 
// ff: undefinition error
// bit0－f1：纸、码币器全故障0x0001
// bit1－f2：硬币器故障；0x0002 
// bit2－f3：hopper故障；0x0004
// bit3－f4：hopper无币；0x0008
// bit4－f5：所有货道无货；0x0010
// bit5－f6：所有货道故障；0x0020
// bit6－f7：所有货道驱动板与主板通讯故障0x0040
// bit7--f8：所有货道不能工作0x0080
// bit8--f9：参数校验错误0x0100
// bit9--fa：LCD通讯故障0x0200
// bit10-fb：开启的某个货道单价设置为0 0x0400
// bit11-fc: GOC dev error:   0x0800
// bit12-fd: selection error: 0x1000 ---- 0xEFFF
// bit13-fe: PC com error:    0x2000 ---- 0xDFFF
// bit14-e1: ACDC error:      0x4000 ---- 0xBFFF 
// ---------------------------------------------
// System error definition( 2011.7.18 )
// F1: 纸币器、硬币器故障（通信）
// F2: 硬币器(找币器)故障（通信）
// F4: 找零不足，低于投币上限则自动关闭纸币器，(扣除下限值)5角低于3枚或总的找币金额小于5元则退出服务
// F7: 货道驱动板通信故障
// F8: 无可用货道
// F9: 系统参数故障
// FA: LCD通信故障
// FD: 选货按键板故障
// FE: PC通信故障
// F0: 多找币故障//by gzz 20110810
*/

u_int  xdata HardWareErr=0; 

u_char xdata AlarmTime = 0;      //短信报警的次数

bit  data  	KeyLockFlag = 0;     //键盘锁定标志
//bit  data  SystemSave = 0;     //系统是否保存
bit  data  	DisableFlag = 0;     //收币器禁能标志
bit  data  	IsCanInCashFlag = 1; //收纸币标志
bit  data  	IOUFlag = 0;         //本次退出服务时是否有欠钱
u_char xdata errInputMoney = 0;
bit	 data	QueryFlag = 0;       //记录是否有顾客想查询货道状态

u_char DispWelOrOutServ();
u_char DispenseCoin(unsigned char flag );
void GetHopperList();
u_char CheckGoodsSumAndLoad();
bit Selection1Query( void );
bit Selection2Query( void );
bit Selection3Query( void );
u_char changeCNY(unsigned char* results, u_int money);


//--------------------------------------------------
u_char VP_GameKeyPoll( u_int xdata msTime )
{
    
    #define VP_GAMEKEY_POLL  30

    u_char xdata result = 0;
    u_int  xdata i = 0;
    u_int  xdata pollTime = 0;

	pollTime = msTime/3;
    if( pollTime < 1 ) pollTime = 1;

	sysVPMission.msTimer3  =  VP_GAMEKEY_POLL;

    //sysVPMission.msGameTimer1 = time*1000/10;
    sysVPMission.msGameTimer1 = msTime/10;
	while( sysVPMission.msGameTimer1 )
	{
	    //----------------------------------------------------
		//Send the game key and poll pc in speciall time
	    if( IfGameKeyOn() )
		{
			VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
	        //Beep();
		}

        /*
		i++;
		if( i >= pollTime )
		{
		    i = 0;
			result = VPMission_Poll();
		}
		*/
        if( sysVPMission.msTimer3 == 0 )
		{
		    //sysVPMission.msTimer3  =  VP_GAMEKEY_POLL;
		    //if( ( sysVPMission.vendCmd == 0 )&&(sysVPMission.changeCmd == 0)&&(sysVPMission.costCmd == 0)&&(sysVPMission.returnCmd == 0) )
				result = VPMission_Poll();
		}
		//===============================
	}
    return 0;
}

u_char VP_UpdateChnagerStatus( void )
{
    u_char xdata flag = 0;
    
    #ifdef MACHINE_SET_MDB
        if( sysVPMission.sTimerChanger1 >= 10 )
		{
			sysVPMission.sTimerChanger1 = 0;
			flag = MDB_Coin_TubeSta_API();
			sysVPMission.tubeCoinMoney = sysMDBMission.coinAllValue;//保存硬币斗可找零金额
		}
	#else
		//1. 
	    if( SystemParameter.HopperCoinPrice1 >= 1 )
		{
		    //||(DeviceStatus.ChangeUnit1 == 0x01);by gzz 20110506
			//if(sysVPMission.hopperComErr[0]!=1)   
		    if( ((DeviceStatus.ChangeUnit1 == 0x02) ||(DeviceStatus.ChangeUnit1 == 0x04) ||(DeviceStatus.ChangeUnit1 == 0x01))&&(sysVPMission.hopperComErr[0]!=1) )   //low coin	
			{
				if( sysVPMission.VPDevCtr & 0x02 )
				{
				    if( sysVPMission.sTimerChanger1 >= VPM_DEVUPDATE_CHANGER1 )
					{
						sysVPMission.sTimerChanger1 = 0;
						flag = GetHardWareDeviceState( 1, &Hopper1 );			
						if( flag != 1 )
						{					
							if( Hopper1.State & 0x50 )        //系统故障
							{																			
								DeviceStatus.ChangeUnit1 = 1;
							}
							else if( Hopper1.State & 0x08 )
							{
							    DeviceStatus.ChangeUnit1 = 4;
							}
							else if( Hopper1.State & 0x20 )   //光测无币或实际无币
							{	
								DeviceStatus.ChangeUnit1 = 2;  
							}
							else
							{
								DeviceStatus.ChangeUnit1 = 0;
								//HopperSetList[0].HopperState = 1;
	                            sysVPMission.hopperComErr[0] = 0;
							}
						}	
						else
						{
							DeviceStatus.ChangeUnit1 = 1;
							sysVPMission.hopperComErr[0] = 1;
						}
	                    GetHopperList();
	 
					}
				}
				else
				{
					sysVPMission.VPDevCtr |= 0x02;
				}
			}
		}
	
		//2.
		if( SystemParameter.HopperCoinPrice2 >= 1 )
		{
		    //|| (DeviceStatus.ChangeUnit2 == 0x01);by gzz 20110506
	 	    //if(sysVPMission.hopperComErr[1]!=1)
		    if( ((DeviceStatus.ChangeUnit2 == 0x02) || (DeviceStatus.ChangeUnit2 == 0x04) || (DeviceStatus.ChangeUnit2 == 0x01))&&(sysVPMission.hopperComErr[1]!=1) )   //low coin   
			{
				if( sysVPMission.VPDevCtr & 0x04 )
				{
				    if( sysVPMission.sTimerChanger2 >= VPM_DEVUPDATE_CHANGER2 )
					{
						sysVPMission.sTimerChanger2 = 0;
						flag = GetHardWareDeviceState( 2, &Hopper2 );			
						if( flag != 1 )
						{					
							if( Hopper2.State & 0x50 )        //系统故障
							{																			
								DeviceStatus.ChangeUnit2 = 1;
							}
							else if( Hopper2.State & 0x08 )
							{
							    DeviceStatus.ChangeUnit2 = 4;
							}
							else if( Hopper2.State & 0x20 )   //光测无币或实际无币
							{	
								DeviceStatus.ChangeUnit2 = 2; 
							}
							else
							{
								DeviceStatus.ChangeUnit2 = 0;
								//HopperSetList[1].HopperState = 1;
	                            sysVPMission.hopperComErr[1] = 0;
							}
						}	
						else
						{
							DeviceStatus.ChangeUnit2 = 1;
							sysVPMission.hopperComErr[1]=1;
						}
	                    GetHopperList();
	 
					}
				}
				else
				{
					sysVPMission.VPDevCtr |= 0x04;
				}
			}
	
		}
	
	    //3.
	    if( SystemParameter.HopperCoinPrice3 >= 1 )
		{
		    //||(DeviceStatus.ChangeUnit3==0x01);by gzz 20110506
			//if(sysVPMission.hopperComErr[2]!=1)
		    if( ((DeviceStatus.ChangeUnit3==0x02) ||(DeviceStatus.ChangeUnit3==0x04) ||(DeviceStatus.ChangeUnit3==0x01))&&(sysVPMission.hopperComErr[2]!=1) )   //low coin 
			{
				if( sysVPMission.VPDevCtr & 0x08 )
				{
				    if( sysVPMission.sTimerChanger3 >= VPM_DEVUPDATE_CHANGER3 )
					{
						sysVPMission.sTimerChanger3 = 0;
						flag = GetHardWareDeviceState( 3, &Hopper3 );			
						if( flag != 1 )
						{					
							if( Hopper3.State & 0x50 )        //系统故障
							{																			
								DeviceStatus.ChangeUnit3 = 1;
							}
							else if( Hopper3.State & 0x08 )
							{
							    DeviceStatus.ChangeUnit3 = 4; 
							}
							else if( Hopper3.State & 0x20 )   //光测无币或实际无币
							{	
								DeviceStatus.ChangeUnit3 = 2; 
							}
							else
							{
								DeviceStatus.ChangeUnit3 = 0;
								//HopperSetList[2].HopperState = 1;
	                            sysVPMission.hopperComErr[2] = 0;
							}
						}	
						else
						{
							DeviceStatus.ChangeUnit3 = 1;
							sysVPMission.hopperComErr[2]=1;
						}
	                    GetHopperList();
					}
				}
				else
				{
					sysVPMission.VPDevCtr |= 0x08;
				}
			}
	
		}
	#endif
    return 0;

}
//==================================================

/////////////////////////////////////////////
///		公用涵数						/////
/////////////////////////////////////////////
u_char CheckHardWare( u_char i)
{
	u_char xdata j = 0;
//#ifndef _DEBUG_TRACE
//#ifdef _HAVE_CASHER	
//	Trace( "\n CheckHardWare i= %bu", i );
	if( SystemParameter.BillNo == 1 )
	{
		if( i == 3 )
		{	
			if( HardWareErr & 0x0001)			
				GetDeviceStatus( 0  );//取纸币器状态		
			else if( HardWareErr & 0x0004 )
			{
				GetDeviceStatus( 2  );//#1退币器测试				
				GetDeviceStatus( 3  );//#2退币器测试		
				GetDeviceStatus( 4  );//#3退币器测试
				i = 0;
			}	
		}
		else
		{
			if( i == 0 )
				GetDeviceStatus( 0  );//取纸币器状态
			else if( IsCanInCashFlag == 1 /*HopperSetList[0].HopperState == 1*/ )
				GetDeviceStatus( 0  );//取纸币器状态
			if ( i == 0 )
			{
				GetDeviceStatus( 2  );//#1退币器测试
				GetDeviceStatus( 3  );//#2退币器测试
				GetDeviceStatus( 4  );//#3退币器测试
			}
		}
		/*
		if( ( DeviceStatus.BillAccepter != 0 ) && ( DeviceStatus.CoinAccepter != 0 ) )
		//	HardWareErr = ERR_BILL_COIN;				
			HardWareErr |= 0x0001;
		else if( ( ( DeviceStatus.BillAccepter != 0 ) || ( IsCanInCashFlag == 0 ) ) && ( DeviceStatus.CoinAccepter != 0 ) )
		{
			//HardWareErr = ERR_COIN;				
			HardWareErr |= 0x0002;
		}
		else if( ( DeviceStatus.CoinAccepter == 0 ) || ( ( DeviceStatus.BillAccepter == 0 ) && ( IsCanInCashFlag == 1 ) ) )
		{
			//HardWareErr = 0;  //2011.1.22
		}
		*/
	}
	else
	{
		if( i == 3 )
		{				
//			if( HardWareErr & 0x0002 )
//				GetDeviceStatus( 1  );//取硬币器状态
			//else if( HardWareErr == ERR_HOPPER )
			if( HardWareErr & 0x0004 )
			{
				GetDeviceStatus( 2  );//#1退币器测试				
				GetDeviceStatus( 3  );//#2退币器测试		
				GetDeviceStatus( 4  );//#3退币器测试
			}	
		}
		else
		{		
//			GetDeviceStatus( 1  );//取硬币器状态
			if ( i == 0 )
			{
				GetDeviceStatus( 2  );//#1退币器测试
				GetDeviceStatus( 3  );//#2退币器测试
				GetDeviceStatus( 4  );//#3退币器测试
			}
		}				
	
		 if( DeviceStatus.CoinAccepter != 0 )
		 {
		//	HardWareErr = ERR_COIN;	
		//	HardWareErr |= 0x0002;
		 }
		 else
		 {		 	
			//	HardWareErr = 0;	
			if( HardWareErr & 0x0002 )			
				HardWareErr &= ( 0xffff - 0x00002 );		
		 }
//		 Trace( "\n CheckHardWare HardWareErr2= %bu", HardWareErr );
	}
	if ( i == 0 )
	{			
		for( i = 0; i < 3; i ++ )
		{
			switch( HopperSetList[i].HopperIndex )
			{
				case 1:
					if( DeviceStatus.ChangeUnit1 == 2 )
						continue;	//break;
					if( DeviceStatus.ChangeUnit1 == 1 )
						HopperSetList[i].HopperState = 2;
                    else if( (DeviceStatus.ChangeUnit1==4)||(DeviceStatus.ChangeUnit1==2) )
                        HopperSetList[i].HopperState = 2;
					else
						HopperSetList[i].HopperState = 1;
				break;
				case 2:
					if( DeviceStatus.ChangeUnit2 == 2 )
						continue;	//break;
					if( DeviceStatus.ChangeUnit2 == 1 )
						HopperSetList[i].HopperState = 2;
                    else if( (DeviceStatus.ChangeUnit2==4)||(DeviceStatus.ChangeUnit2==2) )
                        HopperSetList[i].HopperState = 2;
					else
						HopperSetList[i].HopperState = 1;
				break;
				case 3:
					if( DeviceStatus.ChangeUnit3 == 2 )
						continue;	//break;
					if( DeviceStatus.ChangeUnit3 == 1 )
						HopperSetList[i].HopperState = 2;
                    else if( (DeviceStatus.ChangeUnit3==4)||(DeviceStatus.ChangeUnit3==2) )
                        HopperSetList[i].HopperState = 2;
					else
						HopperSetList[i].HopperState = 1;
				break;
				default:
					continue;	
			}
		}
		for( j = 0; j < 3; j ++ )
		{
			if( HopperSetList[j].price == 0 )
				break;
		}
		if( ( j > 0 ) && ( HopperSetList[j - 1].HopperState != 1 ) )		
			IsCanInCashFlag = 0;
	}
//	Trace( "\n CheckHardWare HardWareErr = %bu", HardWareErr );	
//#endif

    //----------------------------------------------------------------------
	//
    //if( (SystemParameter.busCardOn==0x01)&&(DeviceStatus.BusCard==0x00) )
    //{
    //	BCMission_Inq();
    //}
    //=======================================================================

	return( 0 );
}

u_char RejectBill()
{	
	u_char xdata i = 0;
	u_char xdata j = 0;
	u_char xdata flag = 0;
    
	if( sysITLMission.ITLSet == 1 )
	{
	    flag = ITLMission_Reject();
		if( flag == ITL_ERR_NULL )
		{
		    return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		i = CasherReject();
		while( j < 3 )
		{
			if( i == 0 )
			{
				WaitForWork( 500, NULL );
				i = CasherReject();
				j++;
			}
			else
				break;
		}
		if( ( j >= 3 ) && ( i == 0 ) )
			return 1;
		while( ! ( i || j ) )
		{
			WaitForWork( 100, NULL );
			i = TestDeviceTimeOut( &Casher );
			j = TestDeviceCommOK( &Casher );		
		}
		if( i ) 
			return 1;//超时，故障
	}
	return 0;//正常	
}

//禁能纸、硬币器
u_char DisableBillDev()
{
	//DisplayStr( 0, 0, 1, "2", 1 );  
	//WaitForWork(2000,NULL);
	//if(sysVPMission.billCoinEnable == 0)
	{
		//DisplayStr( 0, 0, 1, "3", 1 );  
	    //WaitForWork(2000,NULL);
	    #ifdef MACHINE_SET_MDB		    
			if(( SystemParameter.BillNo == 1 )&&( sysITLMission.ITLSet == 1 ))	
			{ 
				if( sysMDBMission.billIsEnable == 1 )
				{
             		MDBMission_Bill_Disable();
				}
	        }

			if(sysMDBMission.coinIsEnable == 1)
			{
				MDBMission_Coin_Disable();
			}
		#else
			//if( !DisableFlag )    //2011.1.20 changed.
			{
				if( SystemParameter.BillNo == 1 )
				{
				    if( sysITLMission.ITLSet == 1 )
					{
						ITLMission_Disable();    
					}
				    else
					{
						CasherQueryDisable();
					}
				}
				//CoinerDisable();	
				MDBMission_Coin_Disable(); 
				WaitForWork( 20 , NULL );
				DisableFlag = 1;
			}
	    #endif
	}
	return 0;
}


//使能纸、硬币器
u_char EnableBillDev()
{	

	if(sysVPMission.billCoinEnable == 1)
	{
	    //static u_char xdata cFullFlag = 0;
		#ifdef MACHINE_SET_MDB
			//DisplayStr( 0, __LINE0__ , 1 , "4", 1 );
			//WaitForWork( 2000, NULL );  //500-50-100-200		    
            //纸币器使能操作；by gzz 20110811
			//if(( sysMDBMission.billDeviceStatus == 0x00 )&&( SystemParameter.BillNo == 1 )&&( sysITLMission.ITLSet == 1 ))
			if(( SystemParameter.BillNo == 1 )&&( sysITLMission.ITLSet == 1 ))
            {
                //只有纸币器目前是禁能或者是故障时，才予以重新使能;by gzz 20110920
                if(( sysMDBMission.billDeviceStatus == 0x01 )||( sysMDBMission.billIsEnable == 0 ))
                {
					MDBMission_Bill_Enable();
                }	
			}			

            //只有硬币器目前是禁能或者是故障时，才予以重新使能;by gzz 20110920
            if(( sysMDBMission.coinDeviceStatus == 0x01 )||( sysMDBMission.coinIsEnable == 0 ))
			{
				MDBMission_Coin_Enable();	
			}
	    #else
			if( (DisableFlag)&&(HardWareErr==0) )
			{		
				if( SystemParameter.BillNo == 1 )
				{	
				    if( sysITLMission.ITLSet == 1 )
					{
					    if( DeviceStatus.BillAccepter == 0 )
						{
							//ITLMission_Enable();
							if((sysITLMission.billSta&ITL_BILL_SJAM)||(sysITLMission.billSta& ITL_BILL_UJAM ))
							{
								sysITLMission.billJamedFlag = 1;
								if( sysITLMission.billJamedTime >= ITL_TIME_JAMED_DISABLE )
								{
									sysITLMission.billJamedTime = 0;
		                            ITLMission_Disable();
								}
							}
							else
							{
							    sysITLMission.billJamedFlag = 0;
								ITLMission_Init_1();
							}
		    			}
					}
				    else
				    {
						if( DeviceStatus.BillAccepter == 0 )
						{
							CasherQueryEnable();
						}
					}
				}
		
				if( DeviceStatus.CoinAccepter == 0 )
				{
					//CoinerEnable();           //使能硬币器
					MDBMission_Coin_Init();
				}
				else
				{
					//CoinerDisable();//禁能硬币器
					MDBMission_Coin_Disable();
				}
				WaitForWork( 20 , NULL );
				DisableFlag = 0;
			}
		    //
		    if( IfCoinHopperFull() )
		    {
		        if( cFullFlag == 0 )
		        {
		    		cFullFlag = 1;
		        }
		    }
		    else
		    {
		    	if( cFullFlag == 1 )
		        {
		        	cFullFlag = 0;
		            //CoinerEnable();
					MDBMission_Coin_Init();//使能硬币器
		        }
		    }
		#endif
	}
	if(sysVPMission.ErrFlag2 == 0)
	{
		if(sysVPMission.ErrFlag==1)
		{
			sysVPMission.ErrFlag=0;
			sysMDBMission.billCashBuf = 0;
			sysVPMission.DevErrFlag = 0;
			VPMission_Status_RPT();
			MDB_Coin_TubeSta_API();
		}
	}
	return 0;
}

u_char SetOffFlag()
{
    //Trace( "\n SetOffFlag");
	SystemStatus = 2; /* 标志设为OUT_OF_SERVICES状态 */
	//DisableBillDev();	
	//DisplayStr( 0, __LINE0__ , 1 , "1", 1 );
	//WaitForWork( 2000, NULL );  //500-50-100-200
	KeyLock();
	return 1;
}

//设为非现金交易模式;by gzz 20110727
u_char SetCashLessFlag()
{
    //Trace( "\n SetOffFlag");
	SystemStatus = 2; /* 标志设为OUT_OF_SERVICES状态 */
	sysVPMission.SystemState = 1; 
	//DisableBillDev();
	//DisplayStr( 0, __LINE0__ , 1 , "2", 1 );
	//WaitForWork( 2000, NULL );  //500-50-100-200
	//KeyLock();
	return 1;
}

u_char LogicOpen()
{
    //Trace( "\n LogicOpen");
	SystemStatus = 1;
    //isShowLcd = 1;
	if( sysVPMission.VPMode == 0 )
	{
		if( KeyLockFlag == 1 )
		{
			KeyUnLock(); /* 打开键盘，允许客户进行交易,硬币和纸币在运行过程中再打开 */
			KeyLockFlag = 0;
		}
	}
	if(sysVPMission.SystemState == 1)
		sysVPMission.SystemState = 0;
	return 1;

}


/*
u_char CheckKeyPress()
{
	if (  SystemStatus == 2 ) 
		return 0; // 系统关闭，按键不理,当做没按 
	if ( GetKey() != KEY_NULL ) 
		return 1;
	else
		return 0;
}
*/

//用于显示确定或取消并返回用记选择值,返回值，确定返回0,取消返回1,超时返回2
u_char ChooseReturn( )
{
//	u_char xdata length = 0;
	u_char xdata pt= 0;
	u_char xdata MyKey = 0;

	ClearKey();	
	//显示字符	
	DisplayLine = STR_OK_NOT;	 
	DisplayStr( 0, __LINE1__ , 1, DispInformationTable[	DisplayLine ].str , DispInformationTable[ DisplayLine ].len );
    
	//等待选择
	LzjSecTimer= DEFAULT_PRESSKEY_TIMEOUT;
	while( LzjSecTimer )
	{
		SchedulerProcess();
		MyKey = GetKey();
		if ( MyKey == KEY_NULL ) 
			continue;
		switch ( MyKey )
	 	{			
			case 	KEY_SUBMIT:			
				return 0 ;			
			case KEY_CANCEL:				
				return 1 ;					
			default:
				continue;
		}	
	}
	return 2;
}

u_char Cal( u_int Money , u_char* m_Hopper1, u_char* m_Hopper2, u_char* m_Hopper3 )
{
	u_char xdata m_L = 0;
	u_char xdata m_H = 0;	

    //Trace( "\n in Cal");	
    //Trace( "\n Money = %d", Money );	

	if( Money < HopperSetList[2].price )//无法配币
		return 1;
	
	m_L = Money / HopperSetList[0].price;			
	m_H = m_L;
    //Trace("\n HopperSetList[0].HopperState = %bu", HopperSetList[0].HopperState );
    //Trace("\n HopperSetList[0].HopperIndex = %bu", HopperSetList[0].HopperIndex );
	if( HopperSetList[0].HopperState == 1 )
	{
		switch( HopperSetList[0].HopperIndex )
		{
			case 1:											
				*m_Hopper1 = m_L;			
			break;
			case 2:			
				*m_Hopper2 = m_L;
			break;
			case 3:				
				*m_Hopper3 = m_L;
			break;
		}
	}
	else
	{
		switch( HopperSetList[0].HopperIndex )
		{
			case 1:						
				*m_Hopper1 = 0;
				m_H = 0;		
			break;
			case 2:
				*m_Hopper2 = 0;
				m_H = 0;
			break;
			case 3:
				*m_Hopper3 = 0;
				m_H = 0;
			break;
		}	
	}

	Money = Money - m_H * HopperSetList[0].price;			
	m_H = 0;
	m_L = 0;

	m_L = Money / HopperSetList[1].price;			
	m_H = m_L;
//	Trace("\n HopperSetList[1].HopperState = %bu", HopperSetList[1].HopperState );
//	Trace("\n HopperSetList[1].HopperIndex = %bu", HopperSetList[1].HopperIndex );
	if( HopperSetList[1].HopperState == 1 )
	{
		switch( HopperSetList[1].HopperIndex )
		{
			case 1:			
				*m_Hopper1 = m_L;
			break;
			case 2:				
				*m_Hopper2 = m_L;
			break;
			case 3:				
				*m_Hopper3 = m_L;
			break;
		}
	}
	else
	{
		switch( HopperSetList[1].HopperIndex )
		{
			case 1:						
				*m_Hopper1 = 0;
				m_H = 0;		
			break;
			case 2:
				*m_Hopper2 = 0;
				m_H = 0;
			break;
			case 3:
				*m_Hopper3 = 0;
				m_H = 0;
			break;
		}	
	}

	Money = Money - m_H * HopperSetList[1].price;			
	m_H = 0;
	m_L = 0;
	
//	Trace("\n HopperSetList[2].HopperState = %bu", HopperSetList[2].HopperState );
//	Trace("\n HopperSetList[2].HopperIndex = %bu", HopperSetList[2].HopperIndex );
	m_L = Money / HopperSetList[2].price;
	if( HopperSetList[2].HopperState == 1 )
	{			
		switch( HopperSetList[2].HopperIndex )
		{
			case 1:						
				*m_Hopper1 = m_L;
			break;
			case 2:							
				*m_Hopper2 = m_L;
			break;
			case 3:						
				*m_Hopper3 = m_L;
			break;
		}
	}
	else
	{
		switch( HopperSetList[2].HopperIndex )
		{
			case 1:						
				*m_Hopper1 = 0;
				m_H = 0;		
			break;
			case 2:
				*m_Hopper2 = 0;
				m_H = 0;
			break;
			case 3:
				*m_Hopper3 = 0;
				m_H = 0;
			break;
		}	
	}
		

//	Trace( "\n *Hopper1 = %bu", *m_Hopper1 );
//	Trace( "\n *Hopper2 = %bu", *m_Hopper2 );
//	Trace( "\n *Hopper3 = %bu", *m_Hopper3 );	
	if (( *m_Hopper1 == 0) && ( *m_Hopper2 == 0 ) && ( *m_Hopper3 == 0) ) 
	// 配币错误 
		return 1;	
	return 0 ;		
}

u_char DisplayCurrentMoney( u_int Money )
{
	u_char xdata length = 0;
	u_int  xdata moneyBuf= 0;

	if( sysITLMission.billHoldingFlag == 1 )
	{
		Money += sysITLMission.billHoldingValue;
	}

	if( Money <= 0 )
	{
		return 0;
	}
    //DisplayLine = STR_CUSTOM_PAYED;   
	DisplayLine = STR_BALANCE;
	memset( DisplayBuffer , 0 , sizeof( DisplayBuffer ));
	length = DispInformationTable[DisplayLine].len;
	memcpy( DisplayBuffer , DispInformationTable[DisplayLine].str , DispInformationTable[DisplayLine].len );
	DisplayBuffer[ length ] = ' ';	
	switch( SystemParameter.curUnit )
	{
		case 1:
			length += sprintf( DisplayBuffer + length + 1 , "%u" , Money );	
		break;				
		case 10:
			length += sprintf( DisplayBuffer + length + 1 , "%u.%u" , (Money / SystemParameter.curUnit),( Money %SystemParameter.curUnit));	
		break;
		case 100:
			length += sprintf( DisplayBuffer + length + 1 , "%u.%02u" , (Money / SystemParameter.curUnit),( Money %SystemParameter.curUnit));	
		break;		
	}		
	length++;	
	DisplayStr( 0, 0, 1, DisplayBuffer, length );
    
	if( Money  >= SystemParameter.BanknoteMax )
	{
	    DisplayLine = STR_VP_SELECTGOODS;
        DisplayStr( 0, 1, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
		//
		if( sysITLMission.billHoldingFlag == 1 )
		{
			
            MDBMission_Coin_Disable();
		}

	}
	else
	{
	    DisplayLine = STR_INPUT_GOODS;
		DisplayStr( 0, 1, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );  //-1	
	}
	//WaitForWork( 100 , &Lcd );

    //----------------------------------------------------------------------------------
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
	if( sysVPMission.escrowInFlag == 1 )
    {
	    VPMission_Payin_RPT( VP_DEV_ESCROWIN, sysVPMission.escrowMoney );
		sysVPMission.escrowInFlag = 0;
        sysVPMission.escrowMoney = 0;     
	}
	if( sysVPMission.escrowOutFlag == 1 )
    {
	    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
		sysVPMission.escrowOutFlag = 0;
        sysVPMission.escrowMoney = 0;     
	}
	//==================================================================================
	return( 0 );
}

void GetHopperList()
{		
	u_int xdata i = 0;
	u_int xdata j = 0;
	u_int xdata z = 0;	

	 i = SystemParameter.HopperCoinPrice1;
	 j = SystemParameter.HopperCoinPrice2;
	 z = SystemParameter.HopperCoinPrice3;
	 
	//找出最大的斗		
	if( ( i >= j ) && ( i >= z ) )
	{	
		HopperSetList[0].price = i;
		HopperSetList[0].HopperIndex = 1;
		if( j >= z )
		{
			HopperSetList[1].price = j;
			HopperSetList[1].HopperIndex = 2;		
			HopperSetList[2].price = z;
			HopperSetList[2].HopperIndex = 3;
		}
		else
		{		
			HopperSetList[1].price = z;
			HopperSetList[1].HopperIndex = 3;		
			HopperSetList[2].price = j;
			HopperSetList[2].HopperIndex = 2;
		}
	}
	else if( ( i >= j ) && ( i <= z ) )
	{
		HopperSetList[0].price = z;
		HopperSetList[0].HopperIndex = 3;
		HopperSetList[1].price = i;
		HopperSetList[1].HopperIndex = 1;	
		HopperSetList[2].price = j;
		HopperSetList[2].HopperIndex = 2;
	}
	else if( ( j >= i ) && ( i >= z ) )
	{	
		HopperSetList[0].price = j;
		HopperSetList[0].HopperIndex = 2;	
		HopperSetList[1].price = i;
		HopperSetList[1].HopperIndex = 1;
		HopperSetList[2].price = z;
		HopperSetList[2].HopperIndex = 3;
	}
	else if( ( j >= i ) && ( i <= z ) )
	{		
		HopperSetList[2].price = i;
		HopperSetList[2].HopperIndex = 1;
		if( j >= z )
		{
			HopperSetList[0].price = j;
			HopperSetList[0].HopperIndex = 2;
			if( (DeviceStatus.ChangeUnit2==1)||(DeviceStatus.ChangeUnit2==2)||(DeviceStatus.ChangeUnit2==4) )
				HopperSetList[0].HopperState = 2;
			HopperSetList[1].price = z;
			HopperSetList[1].HopperIndex = 3;
			if( (DeviceStatus.ChangeUnit2==1)||(DeviceStatus.ChangeUnit2==2)||(DeviceStatus.ChangeUnit2==4) )
				HopperSetList[0].HopperState = 2;
		}
		else
		{
			HopperSetList[0].price = z;
			HopperSetList[0].HopperIndex = 3;			
			HopperSetList[1].price = j;
			HopperSetList[1].HopperIndex = 2;
		}
	}

	for( i = 0; i < 3; i ++ )
	{
		if( HopperSetList[i].price == 0 )		
			break;
	}
	if( i > 0 )
		i--;
	for( j = 0; j < 3; j ++ )
	{
		switch( HopperSetList[j].HopperIndex )
		{
			case 1:
				if( SystemParameter.HopperCoinPrice1 == 0 )
					break;			
				if( DeviceStatus.ChangeUnit1 == 1 )
					HopperSetList[j].HopperState = 2;
				//---------------------------------------------------
				else if( (DeviceStatus.ChangeUnit1 == 4)||(DeviceStatus.ChangeUnit1 == 2) )
				    HopperSetList[j].HopperState = 2;
				//===================================================
				else
					HopperSetList[j].HopperState = 1;
				if( ( j == i ) && ( DeviceStatus.ChangeUnit1 != 0 ) )
					IsCanInCashFlag = 0;				
			break;
			case 2:
				if( SystemParameter.HopperCoinPrice2 == 0 )
					break;			
				if( DeviceStatus.ChangeUnit2 == 1 )
					HopperSetList[j].HopperState = 2;
				//---------------------------------------------------
				else if( (DeviceStatus.ChangeUnit2 == 4)||(DeviceStatus.ChangeUnit2 == 2) )
				    HopperSetList[j].HopperState = 2;
				//===================================================
				else
					HopperSetList[j].HopperState = 1;
				if( ( j == i ) && ( DeviceStatus.ChangeUnit2 != 0 ) )
					IsCanInCashFlag = 0;
			break;
			case 3:
				if( SystemParameter.HopperCoinPrice3 == 0 )
					break;			
				if( DeviceStatus.ChangeUnit3 == 1 )
					HopperSetList[j].HopperState = 2;
				//---------------------------------------------------
				else if( (DeviceStatus.ChangeUnit3==4)||(DeviceStatus.ChangeUnit3==2) )
				    HopperSetList[j].HopperState = 2;
				//===================================================
				else
					HopperSetList[j].HopperState = 1;
				if( ( j == i ) && ( DeviceStatus.ChangeUnit3 != 0 ) )
					IsCanInCashFlag = 0;
			break;
			default:
				continue;	
		}
	}
	//如果HOPPER内的面值为0则此HOPPER不能参数配币
	for( i = 0; i < 3; i ++ )
	{
		if( HopperSetList[i].price == 0 )		
			HopperSetList[i].HopperState = 2;				
	}
	//如果HOPPER内的面值为0则此HOPPER不能参数配币
	for( i = 0; i < 3; i ++ )
	{
		if( HopperSetList[i].price == 0 )		
			break;
	}
	if( ( i > 0 )&& ( HopperSetList[i - 1].HopperState != 1 ) )
		IsCanInCashFlag = 0;

	if( HardWareErr & 0x0004 )
	{
		//HardWareErr = 0;  //2011.1.22
	}

}


//得到面值最小的使能HHOPPER
u_char GetMinHopper()
{
	if( HopperSetList[2].price != 0 )
		return 2;
	else if( HopperSetList[1].price != 0 )
		return 1;
	else
		return 0;
}

//是否还能找币
u_char IsCanChange()
{
	u_char i = 0;

	for(i = 0; i < 3; i ++ )
	{
		if( ( HopperSetList[i].HopperState == 1 ) && ( HopperSetList[i].price != 0 ) )
			break;
	}
	if( i == 3 )
		return 0;
	else
		return 1;
}

//是否还有货道可服务
//by gzz 20110614
u_char ColnumIsCanServer()
{
	u_char i = 0;    
	for( i = 0; i < GOODSWAYNUM; i ++ )
	{
        if( SystemParameter.GOCOn != 0x01 )
		{
			if(( GoodsWaySetVal[i].GoodsCurrentSum != 0 ) && ( ( GoodsWaySetVal[i].WayState & 0x0A ) == 0x00 ) && ( GoodsWaySetVal[i].Price != 0 ) )
				break;
		}
		else
		{
		    //if(( (GoodsWaySetVal[i].WayState & 0x04) == 0 )&&( ( GoodsWaySetVal[i].WayState & 0x0A ) == 0x00 )&&( GoodsWaySetVal[i].Price != 0 ) )
			if( ( GoodsWaySetVal[i].GoodsCurrentSum != 0 ) && ( (GoodsWaySetVal[i].WayState & 0x04) == 0 )&&( ( GoodsWaySetVal[i].WayState & 0x0A ) == 0x00 )&&( GoodsWaySetVal[i].Price != 0 ) )
				break;
		}
        //----------------------------------------------------
        //原先用于判断是否有货道可用，如果有，则跳出循环
		//if( ( ( GoodsWaySetVal[i].WayState & 0x01 ) == 0x01 ) && ( ( GoodsWaySetVal[i].WayState & 0x0A ) == 0x00 ) && ( GoodsWaySetVal[i].GoodsCurrentSum != 0 ) /*&& ( GoodsWaySetVal[i].Price != 0 )*/ )
		//	break;
        //====================================================
	}
	if( i == GOODSWAYNUM )
    {
		return 0;
    }
    //-----------------------------------------------------
    /*
    //UpdateGoodsMatrixStatus();
	for( i=0; i<GOODSTYPEMAX; i++ )
	{
		//if( sysGoodsMatrix[i].NextColumn != 0xff )
		//	break;
        if( (sysGoodsMatrix[i].NextColumn != GOODS_MATRIX_NONE)&&(sysGoodsMatrix[i].Price != 0) )
		{
			break;
		}
	}
	if( i >= GOODSTYPEMAX )
	{
		return 0;
	}
	*/
	//=====================================================	
	return 1;
}

u_char	ScanIsSendAlarm()
{
	return 0;
}

//查询回应信息打包函数
//查询命令回应信息打包函数
u_char	ScanDeviceStatus()
{
	return 0;
}

//交易记录信息
u_char	TradePack()
{
	return 0;
}




/////////////////////////////////
///		流 程 函 数			/////
/////////////////////////////////
// 系统启动
u_char SystemInit()
{    
	u_char xdata i = 0;  

	memset( &SystemParameter, 0, sizeof( SystemParameter ) );
    //-------------------------------------------------------
    memset( &sysVPMission, 0, sizeof( sysVPMission ) );
	//memset( &sysGoodsMatrix, 0, sizeof( sysGoodsMatrix ) );
    //sysVPMission.VPMode = 1;
    sysVPMission.returnKeyLock = 1;
    sysVPMission.gameKeyLock = 1;
    memset( &sysITLMission, 0, sizeof( sysITLMission ) );
    sysITLMission.ITLSet = 1;
	memset( &sysBCMission, 0, sizeof( sysBCMission ) );
	//=======================================================
	WaitForWork( 100, NULL );
	//memset(&sysMDBMission.coinBuf,0,sizeof(sysMDBMission.coinBuf));
	//memset(&sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));
    memset( &sysMDBMission, 0, sizeof(sysMDBMission) );

	
	//-------------------------------------------------------
	//ClearDisplayLine( 1 );
    //ClearDisplayLine( 2 );
	DisplayCursorState( 0, 0, 1, 0, 1 );//无光标
	DisplayCursorState( 1, 0, 1, 0, 1 );//无光标
	//=======================================================
    //Trace( "\n Begin" );
	DisplayStr( 0, 0, 1, DispInformationTable[ STR_TYPE_VER ].str, DispInformationTable[ STR_TYPE_VER ].len );	
	DisplayStr( 0, 1, 1, DispInformationTable[ STR_INSERT_MONEY ].str, DispInformationTable[ STR_INSERT_MONEY ].len );	
	WaitForWork( 100, NULL );
	return 0;
}

// 初始化设备工作状态(如允许查询状态，自动开启广告灯等等)
u_char DeviceDefaultSetting()
{	
	u_char xdata i = 0;
    u_char xdata j = 0;
	u_char xdata x = 0 ;
	u_char xdata y = 0;
	u_char xdata z = 0;
	u_char xdata iRack = 0;

	WaitForWork( 100, NULL );
	memset( &WayCheck, 0, sizeof( WayCheck ));
	memset( &DeviceStatus, 0, sizeof( DeviceStatus ) );
    //----------------------------------------------------------
    //Trace( "\n SysInitParam" );
	for( i = 0; i < 50; i++ )
		CoreProcessCycle();

	IsCanInCashFlag = 1;
	// 如果不是新机器,这里要从flash里把东西读出来,变量初始化 
	memset( HopperSetList, 0, sizeof( HopperSetList ) );	
	LoadGlobalParam(); // 获取全程变量参数

	//检验系统参数的校验位
	i = 0;
	i = sizeof( struct SYSTEMPARAMETER ) - 1;
	x = 0;
	for( j = 0; j < i; j++ )	
		x ^= (( u_char* )&SystemParameter)[j];		
	if( x != SystemParameter.Checkbyte )
	{
		//参数校验错
		Trace( "\n SysParam check error" );
		HardWareErr |= 0x0100;//ERR_SYSTEM_CHECK;//ERR_SYSPARAMETER_ERR;	
	}
    //根据层架设置来配置货道设置
	memset( InputGoodsWayList, 0, sizeof( InputGoodsWayList )*GOODSWAYNUM );
	z = 0;

	///根据flash值确定物理货道与逻辑货道的关系
	for( i = 0; i < RACKNUM; i++ )//货架层数
		iRackSet[ i ].RackNo = i + 1;
	iRack = 0;
	for( i = 0; i < RACKNUM; i++ )//货架层数
	{
		if( iRackSet[ i ].UseState == 1 )//如果此层开启了
		{
			j = iRackSet[ i ].GoodsMax;//此层中各货道最大存货量
			x = 1;//辑逻货道编号的个位	
			for( y = 0; y < RACKCOLUMNNUM; y ++ )//检索每层中开启的货道数
			{
				if( iRackColumnSet[i][y].UseState == 1)
				{
					InputGoodsWayList[ z ].GoodsWayNo = ( iRack + 1 ) * 10 + x;//逻辑货道  			
					InputGoodsWayList[ z ].SetArrayNo = 64 - i*RACKCOLUMNNUM - 8 + y + 1;//物理货道
					InputGoodsWayList[ z ].UseState = 1;//开启状态
					InputGoodsWayList[ z ].GoodsMax = j;//最大存货量
					x += 1;
					z += 1;
				}
			}
			iRack++;
		}
	}	

	//货道数组中的货道代码初始化,确认逻辑货道的参数值	
	for( i = 0; i < GOODSWAYNUM; i++ )
	{		
		GoodsWaySetVal[i].WayNo = InputGoodsWayList[i].GoodsWayNo;	
#ifdef _DEBUG_TRACE
		GoodsWaySetVal[i].WayState = 0x01;	
#else	
        //-----------------------------------------------------------------------------
        //In GOC check mode, clear the column's error flag when power on.	
        if( SystemParameter.GOCOn == 1 )
        {
        	GoodsWaySetVal[i].WayState &= 0xfb;
        }
        //=============================================================================		
		if( (SystemParameter.GOCOn != 0x01)&&( GoodsWaySetVal[i].GoodsCurrentSum != 0 )&&( ( GoodsWaySetVal[i].WayState & 0x0B ) == 0x01 ) )
			GoodsWaySetVal[i].WayState = 0x01;
		else if( (SystemParameter.GOCOn==0x01) && ( ( GoodsWaySetVal[i].WayState & 0x0B ) == 0x01 ) )
		    GoodsWaySetVal[i].WayState = 0x01;
		else
			GoodsWaySetVal[i].WayState = ( GoodsWaySetVal[i].WayState & 0x0F );
#endif
	}		
	/*
	for( i = 0; i < GOODSWAYNUM; i ++ )	
	{
		if( ( GoodsWaySetVal[i].WayNo == InputGoodsWayList[i].GoodsWayNo ) && 
			( InputGoodsWayList[ i ].UseState == 1 ) &&
			( GoodsWaySetVal[i].Price == 0 ) )
		{
			HardWareErr |= 0x0400;
			break;
		}
	}
    */
    IOUFlag = 0;	
	UpdataDisp = 0;
	SystemErrorAlarm = 0;
	MoneyDevAlarm = 0;
	AlarmTime = 0;
	QueryFlag = 0;
	memset( SMSMessage, 0, sizeof( struct SMSMessageList )* MAX_SMS_LIST );
    //ListIndex = 0;
	
    //==========================================================		
	return 0;
}

//显示退出服务
u_char DisplayOutServer()
{	
	SetOffFlag();
    //DispWelOrOutServ();	
	KeyLock();              //锁定键盘，不让客户按键
	KeyLockFlag = 1;
    if( HardWareErr != 0 )
    {
    	if( IsModeOff() )   //是否模式开关处于关闭状态
    	//if( 0 )   //是否模式开关处于关闭状态
	    {	
            KeyUnLock();
			return 1;
	    }	
    }
	return 0;
}

//初始化系统参数
u_char SysInitParam()
{	
	u_char xdata i = 0;
	u_char xdata j = 0;
	u_char xdata x = 0 ;
	u_char xdata y = 0;
	u_char xdata z = 0;
	u_char xdata iRack = 0;
	
	GetHopperList();
	CheckHardWare( 0 );
	GetHopperList();      //获取Hopper的排序列表.

	//if( ( ( DeviceStatus.BillAccepter != 0 ) || ( IsCanInCashFlag == 0 ) ) && ( DeviceStatus.CoinAccepter != 0 ) )
	//	HardWareErr |= 0x0002;

	//清货道控制板的SN
	if( ChannelTask( 0, CHANNEL_CLEAR ) == 2 )
	{
		//执行命令超时,应是主板与货道驱动板通讯故障
		Trace("\n Channel are break");
		/*
		//Disabled by Andy on 2010.10.26
		for( i = 0; i < GOODSWAYNUM; i ++ )	
			GoodsWaySetVal[i].WayState = 0x09;
		*/
		HardWareErr |= 0x0040;
		DeviceStatus.Driver64 |= 0x01;
	}
    else
    {
        //ChannelTask( 0, CHANNEL_QUERY );
    }
	//ITLMission_Init();
	if( SystemParameter.ACDCModule == 1 )
	{
		sysVPMission.ACDCLedCtr = 1;         //Open by default.
		if(SystemParameter.CompModule == 1)
	    	sysVPMission.ACDCCompressorCtr = 1;  //Open by default.
	    else
			sysVPMission.ACDCCompressorCtr = 0;  //Open by default.	
		ACDCMission();
	}
	//获取货道数
	for( i=0,j=0; i<RACKNUM; i++ )
	{
		if(iRackSet[i].UseState == 1)
        	j++;
	}
	COLUMN_NUM_SET=j*8;
	
	return 0;	

}

//加快按键反应速度;by gzz 20110721  
u_char GetKeyOperStatus()
{	
    u_char xdata key;

    //-------------------------------------------------
    //Feed the watch dog
	FeedWatchDog();
	#ifdef   _SHUPING_        
		if( IsModeOff() )            //是否模式开关处于关闭状态
	    {	
			return 1;
	    }
		sysVPMission.VPMode = 0;
		ZhkKeyPermission = 1;
		key = 0;
	#else	
	    //================================================= 
	    //if( (sysVPMission.menuKeyNum==3)&&(sysVPMission.menuTimer>0)&&((sysVPMission.menuKey[0]==KEY_BACK)&&(sysVPMission.menuKey[1]==KEY_DOT)&&(sysVPMission.menuKey[2]==KEY_SUBMIT)) )
		//if( (sysVPMission.menuKeyNum==1)&&(sysVPMission.menuTimer>0)&&((sysVPMission.menuKey[0]==KEY_SUBMIT)) )
		if( (sysVPMission.menuKeyNum==5)&&(sysVPMission.menuTimer>0)&&((sysVPMission.menuKey[0]==KEY_1)&&(sysVPMission.menuKey[1]==KEY_2)&&(sysVPMission.menuKey[2]==KEY_3)&&(sysVPMission.menuKey[3]==KEY_4)&&(sysVPMission.menuKey[4]==KEY_SUBMIT)) )
		{
		    sysVPMission.menuTimer  = 0;
			sysVPMission.menuKeyNum = 0;
	        sysVPMission.menuKey[0] = KEY_NULL;
	        sysVPMission.menuKey[1] = KEY_NULL;
			sysVPMission.menuKey[2] = KEY_NULL;
			sysVPMission.menuKey[3] = KEY_NULL;
			sysVPMission.menuKey[4] = KEY_NULL;
			return 1;
		}
		else
		{
		    
		    if(sysVPMission.menuKeyNum==1)
			{
				if(sysVPMission.menuKey[0]!=KEY_1)
		   	    {
					sysVPMission.menuTimer = 0;
					sysVPMission.menuKeyNum = 0;
			        sysVPMission.menuKey[0] = KEY_NULL;
			        sysVPMission.menuKey[1] = KEY_NULL;
					sysVPMission.menuKey[2] = KEY_NULL;
					sysVPMission.menuKey[3] = KEY_NULL;
		        	sysVPMission.menuKey[4] = KEY_NULL;
					return 0;	
				}
			}
	        if(sysVPMission.menuKeyNum==2)
			{
				if(sysVPMission.menuKey[1]!=KEY_2)
		   	    {
					sysVPMission.menuTimer = 0;
					sysVPMission.menuKeyNum = 0;
			        sysVPMission.menuKey[0] = KEY_NULL;
			        sysVPMission.menuKey[1] = KEY_NULL;
					sysVPMission.menuKey[2] = KEY_NULL;
					sysVPMission.menuKey[3] = KEY_NULL;
		        	sysVPMission.menuKey[4] = KEY_NULL;
					return 0;	
				}
			}
			if(sysVPMission.menuKeyNum==3)
			{
				if(sysVPMission.menuKey[2]!=KEY_3)
		   	    {
					sysVPMission.menuTimer = 0;
					sysVPMission.menuKeyNum = 0;
			        sysVPMission.menuKey[0] = KEY_NULL;
			        sysVPMission.menuKey[1] = KEY_NULL;
					sysVPMission.menuKey[2] = KEY_NULL;
					sysVPMission.menuKey[3] = KEY_NULL;
		        	sysVPMission.menuKey[4] = KEY_NULL;
					return 0;	
				}
			}
	        if(sysVPMission.menuKeyNum==4)
			{
				if(sysVPMission.menuKey[3]!=KEY_4)
		   	    {
					sysVPMission.menuTimer = 0;
					sysVPMission.menuKeyNum = 0;
			        sysVPMission.menuKey[0] = KEY_NULL;
			        sysVPMission.menuKey[1] = KEY_NULL;
					sysVPMission.menuKey[2] = KEY_NULL;
					sysVPMission.menuKey[3] = KEY_NULL;
		        	sysVPMission.menuKey[4] = KEY_NULL;
					return 0;	
				}
			}
	        		
			
		    sysVPMission.VPMode = 0;
			ZhkKeyPermission = 1;
			key = GetKey();
			sysVPMission.VPMode = 1;
		  //if( (sysVPMission.menuKeyNum<3)&&((key==KEY_BACK)||(key==KEY_DOT)||(key==KEY_SUBMIT)) )
			if( (sysVPMission.menuKeyNum<5)&&((key==KEY_1)||(key==KEY_2)||(key==KEY_3)||(key==KEY_4)||(key==KEY_SUBMIT)) )
			{   
				sysVPMission.menuKey[sysVPMission.menuKeyNum++] = key;
	            sysVPMission.menuTimer  = 6;
	            ClearKey();	
			}        
			else if(key!=KEY_NULL)
			{
				sysVPMission.menuTimer  = 0;
				sysVPMission.menuKeyNum = 0;
		        sysVPMission.menuKey[0] = KEY_NULL;
		        sysVPMission.menuKey[1] = KEY_NULL;
				sysVPMission.menuKey[2] = KEY_NULL;
				sysVPMission.menuKey[3] = KEY_NULL;
		        sysVPMission.menuKey[4] = KEY_NULL;
	            return 0;
			}  
	        if(sysVPMission.menuTimer==0)
			{
				sysVPMission.menuTimer = 0;
				sysVPMission.menuKeyNum = 0;
		        sysVPMission.menuKey[0] = KEY_NULL;
		        sysVPMission.menuKey[1] = KEY_NULL;
				sysVPMission.menuKey[2] = KEY_NULL;
				sysVPMission.menuKey[3] = KEY_NULL;
	        	sysVPMission.menuKey[4] = KEY_NULL;
				return 0;
			}      
		}
	#endif
	return 0;
}

u_char EndMission()
{
	    CurrentPayed = 0;            //用户付款计数清零 
	    //DisplayStr( 0, __LINE0__ , 1 , "1", 1 );
		//WaitForWork( 2000, NULL );  //500-50-100-200	
	    sysVPMission.inBillMoney = 0;
        //CurrentTransType = 0;      //设置空闲状态 	
	    CasherGetMachineState( 0 );  //空闲状态
	    return 0;
}	     
//查看机器是否维护状态
u_char CheckOperStatus()
{	
    u_char xdata key;

    //-------------------------------------------------
    //Feed the watch dog
	FeedWatchDog();
    //=================================================
    //
    {
	    //CurrentPayed = 0;            //用户付款计数清零 
	    sysVPMission.inBillMoney = 0;
        //CurrentTransType = 0;      //设置空闲状态 	
	    CasherGetMachineState( 0 );  //空闲状态
	    key = 0;
	    WaitForWork( 10, NULL );
    }
	//if( IsModeOff() )            //是否模式开关处于关闭状态
    //{	
	//	return 1;
    //}

	#ifdef   _SHUPING_
		if( IsModeOff() )            //是否模式开关处于关闭状态
	    {	
			return 1;
	    }
		sysVPMission.VPMode = 0;
		ZhkKeyPermission = 1;
	#else	
	    //if( (sysVPMission.menuKeyNum==3)&&(sysVPMission.menuTimer>0)&&((sysVPMission.menuKey[0]==KEY_BACK)&&(sysVPMission.menuKey[1]==KEY_DOT)&&(sysVPMission.menuKey[2]==KEY_SUBMIT)) )
		//if( (sysVPMission.menuKeyNum==1)&&(sysVPMission.menuTimer>0)&&((sysVPMission.menuKey[0]==KEY_SUBMIT)) )
		if( (sysVPMission.menuKeyNum==5)&&(sysVPMission.menuTimer>0)&&((sysVPMission.menuKey[0]==KEY_1)&&(sysVPMission.menuKey[1]==KEY_2)&&(sysVPMission.menuKey[2]==KEY_3)&&(sysVPMission.menuKey[3]==KEY_4)&&(sysVPMission.menuKey[4]==KEY_SUBMIT)) )
		{
		    sysVPMission.menuTimer  = 0;
			sysVPMission.menuKeyNum = 0;
	        sysVPMission.menuKey[0] = KEY_NULL;
	        sysVPMission.menuKey[1] = KEY_NULL;
			sysVPMission.menuKey[2] = KEY_NULL;
			sysVPMission.menuKey[3] = KEY_NULL;
			sysVPMission.menuKey[4] = KEY_NULL;
			return 1;
		}
		else
		{
		    
		    if(sysVPMission.menuKeyNum==1)
			{
				if(sysVPMission.menuKey[0]!=KEY_1)
		   	    {
					sysVPMission.menuTimer = 0;
					sysVPMission.menuKeyNum = 0;
			        sysVPMission.menuKey[0] = KEY_NULL;
			        sysVPMission.menuKey[1] = KEY_NULL;
					sysVPMission.menuKey[2] = KEY_NULL;
					sysVPMission.menuKey[3] = KEY_NULL;
		        	sysVPMission.menuKey[4] = KEY_NULL;
					return 0;	
				}
			}
	        if(sysVPMission.menuKeyNum==2)
			{
				if(sysVPMission.menuKey[1]!=KEY_2)
		   	    {
					sysVPMission.menuTimer = 0;
					sysVPMission.menuKeyNum = 0;
			        sysVPMission.menuKey[0] = KEY_NULL;
			        sysVPMission.menuKey[1] = KEY_NULL;
					sysVPMission.menuKey[2] = KEY_NULL;
					sysVPMission.menuKey[3] = KEY_NULL;
		        	sysVPMission.menuKey[4] = KEY_NULL;
					return 0;	
				}
			}
			if(sysVPMission.menuKeyNum==3)
			{
				if(sysVPMission.menuKey[2]!=KEY_3)
		   	    {
					sysVPMission.menuTimer = 0;
					sysVPMission.menuKeyNum = 0;
			        sysVPMission.menuKey[0] = KEY_NULL;
			        sysVPMission.menuKey[1] = KEY_NULL;
					sysVPMission.menuKey[2] = KEY_NULL;
					sysVPMission.menuKey[3] = KEY_NULL;
		        	sysVPMission.menuKey[4] = KEY_NULL;
					return 0;	
				}
			}
	        if(sysVPMission.menuKeyNum==4)
			{
				if(sysVPMission.menuKey[3]!=KEY_4)
		   	    {
					sysVPMission.menuTimer = 0;
					sysVPMission.menuKeyNum = 0;
			        sysVPMission.menuKey[0] = KEY_NULL;
			        sysVPMission.menuKey[1] = KEY_NULL;
					sysVPMission.menuKey[2] = KEY_NULL;
					sysVPMission.menuKey[3] = KEY_NULL;
		        	sysVPMission.menuKey[4] = KEY_NULL;
					return 0;	
				}
			}
	        		
			
		    sysVPMission.VPMode = 0;
			ZhkKeyPermission = 1;
			key = GetKey();
			sysVPMission.VPMode = 1;
		  //if( (sysVPMission.menuKeyNum<3)&&((key==KEY_BACK)||(key==KEY_DOT)||(key==KEY_SUBMIT)) )
			if( (sysVPMission.menuKeyNum<5)&&((key==KEY_1)||(key==KEY_2)||(key==KEY_3)||(key==KEY_4)||(key==KEY_SUBMIT)) )
			{   
				sysVPMission.menuKey[sysVPMission.menuKeyNum++] = key;
	            sysVPMission.menuTimer  = 6;
	            ClearKey();	
			}        
			else if(key!=KEY_NULL)
			{
				sysVPMission.menuTimer  = 0;
				sysVPMission.menuKeyNum = 0;
		        sysVPMission.menuKey[0] = KEY_NULL;
		        sysVPMission.menuKey[1] = KEY_NULL;
				sysVPMission.menuKey[2] = KEY_NULL;
				sysVPMission.menuKey[3] = KEY_NULL;
		        sysVPMission.menuKey[4] = KEY_NULL;
	            return 0;
			}  
	        if(sysVPMission.menuTimer==0)
			{
				sysVPMission.menuTimer = 0;
				sysVPMission.menuKeyNum = 0;
		        sysVPMission.menuKey[0] = KEY_NULL;
		        sysVPMission.menuKey[1] = KEY_NULL;
				sysVPMission.menuKey[2] = KEY_NULL;
				sysVPMission.menuKey[3] = KEY_NULL;
	        	sysVPMission.menuKey[4] = KEY_NULL;
				return 0;
			}      
		}
	#endif
	
	return 0;
}

//查询硬件状态值,硬件包括键盘、液晶屏、电源，无故障返回0,有故障返回1并退出服务
u_char CheckHardwareState()	
{	   
    //Trace( "\n HardWareErr1 = %u", HardWareErr );	
    //#ifndef _DEBUG_TRACE	

	/*
	if( CasherRealTimeState() == 1 )
		DeviceStatus.BillAccepter = 1;
	else 
		DeviceStatus.BillAccepter = 0;
    */
    u_char xdata i = 0;
	u_char xdata ch = 0;
	//u_char xdata errStat = 0;
	static u_char xdata errGOC = 0,errCoin = 0,errTube = 0,errCash = 0,errBill = 0;
	//unsigned int xdata CurrentMoney=0;

	UpdateSelectionLed_GoodsSta();//空闲时更新

	


	
	if( (SystemParameter.GOCOn==0x01)&&(DeviceStatus.GOCStatus != 0x00) /*&&(SystemParameter.SVC_GOCErr==0)*/ )
	{
		//HardWareErr |= 0x0800;   //Disabled by Andy 2011.7.17
		//errStat = 1;
		if(errGOC==0)
			errGOC = 1;
	}
	else
	{
	    HardWareErr &= 0xF7FF;
		if(errGOC==2)
			errGOC = 3;
	}
    
	//------------------------------------------------------
	if( IfGameKeyOn() )
	{
		VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
        //Beep();
	}
	//If coin hopper full
	if( IfCoinHopperFull() )
	{
		sysVPMission.coinFull = 1;
		DeviceStatus.CoinAccepter |= 0x02;
	}
	else
	{
		sysVPMission.coinFull = 0;
		DeviceStatus.CoinAccepter &= 0xFD;
	}/*
    //If PC error
    if(  sysVPMission.comErrNum >= VP_TIME_OUNT_NUM )
    {    	
		 HardWareErr |= 0x2000; 
		 
         MDB_Bill_EscrowCtr(0);  //Added 2011.7.19
		 if( ( CurrentPayed > 0 ) )
		 {
		    CurrentDispenseCoin = CurrentPayed;	
			CurrentPayed = 0;
            DisableBillDev();
			DispenseCoin();
            UpdateSelectionLed_GoodsSta();
		 }
		 
	}
	else
	{
    	 if( sysVPMission.comErrNum == 0 )  HardWareErr &= 0xdfff;
	}*/
	//If selection error
    //if( (DeviceStatus.Selection[0] != 0) && (DeviceStatus.Selection[1] != 0) && (DeviceStatus.Selection[2] != 0) )
    if( (DeviceStatus.Selection[0] != 0) || (DeviceStatus.Selection[1] != 0) || (DeviceStatus.Selection[2] != 0) )
    {
    	#ifdef   _SHUPING_
			HardWareErr &= 0xEFFF;
		#else
			HardWareErr |= 0x1000;			

			ch = GetSelectionState( 1, &Selection1 );
		    if( ch == 0 )
			{
			    DeviceStatus.Selection[0] &= 0xfe;			
			}
			else
			{
				DeviceStatus.Selection[0] |= 0x01;			
			}
			ch = GetSelectionState( 2, &Selection2 );
		    if( ch == 0 )
			{
			    DeviceStatus.Selection[1] &= 0xfe;			
			}
			else
			{
				DeviceStatus.Selection[1] |= 0x01;			
			}
			ch = GetSelectionState( 3, &Selection3 );
		    if( ch == 0 )
			{
			    DeviceStatus.Selection[2] &= 0xfe;			
			}
			else
			{
				DeviceStatus.Selection[2] |= 0x01;			
			}
		#endif
	}
	else
	{
		HardWareErr &= 0xEFFF;
	}

	//
	/*
    if( SystemParameter.ACDCModule == 1 )
    {
    	if( DeviceStatus.ACDCModule != 0 )
        {
        	//HardWareErr |= 0x4000;
        }
        else
        {
			HardWareErr &= 0xBFFF;
        }
    }
	*/
	if(sysVPMission.offLineFlag==1)
	{
    	HardWareErr |= 0x4000;
    }
	else
    {
		HardWareErr &= 0xBFFF;
    }

	//
	//MDB_Coin_TubeSta_API();
    if( SystemParameter.SVC_NoChange != 1 )
	{
        //by gzz 20110502
		//if(( DeviceStatus.ChangeUnit1 != 0 ) && ( sysVPMission.sTimerHopperEmp >= VPM_DEVUPDATE_HOPPEREMP )) 
        //{
		//	sysVPMission.sTimerHopperEmp = 0;
		//	GetDeviceStatus(2);//测试此时的找零器;by gzz 20110430
		//}

		#ifdef MACHINE_SET_MDB
		    //if( (sysMDBMission.coinDeviceStatus!=0) || (sysMDBMission.coin5jNum < 3 ) || (sysMDBMission.coinAllValue<500) )
		    if( (sysMDBMission.coinDeviceStatus!=0) || (sysMDBMission.coinAllValue<500) )
	        {
	        	//HardWareErr |= 	0x0008;   
	        	//errStat = 1;
	        }
			else
			{                
				HardWareErr &= 	0xFFF7;
			}
		#else
		    if( DeviceStatus.ChangeUnit1 != 0 )
			{
				//HardWareErr |= 	0x0008;
				//errStat = 1;
			}
			else
			{
				HardWareErr &= 	0xFFF7;
			}
		#endif

	}
	CheckGoodsSumAndLoad();  //2011.5.11 added
	
	//F1 system error
	if( (sysMDBMission.billDeviceStatus != 0)&&(sysMDBMission.coinDeviceStatus!= 0 ) )
	{
		//HardWareErr |= 0x0001;
		//errStat = 1;
	}
	else
	{
	    HardWareErr &= 0xfffe;
	}
    //F2 system error
	if( sysMDBMission.coinDeviceStatus != 0 )
	{
		//HardWareErr |= 0x0002;
		//errStat = 1;
		if(errCoin==0)
			errCoin = 1;
	}
	else
	{
	    HardWareErr &= 0xfffd;
		if(errCoin==2)
			errCoin = 3;
	}

    //F0 system error
	if( sysMDBMission.tubeRemoved != 0 )
	{
		//HardWareErr |= 0x8000;
		//errStat = 1;
		if(errTube==0)
			errTube = 1;
	}
	else
	{
	    HardWareErr &= 0x7fff;
		if(errTube==2)
		{
			errTube = 3;
			sysVPMission.sTimerTubeUpdate = 15;
		}
		else if((errTube==3)&&(sysVPMission.sTimerTubeUpdate==0))
		{
			errTube=4;
		}
	}

	//纸币器故障
	if(sysMDBMission.billCashBuf == 1)//钞箱移除报故障;by gzz 20121224
    {/*
    	//由于投币后，不会调用这个函数,因此不用判断暂存纸币
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
			errStat = 1;
		}*/	
		//errStat = 1;
		if(errCash==0)
			errCash = 1;
	}
	else
	{
		if(errCash==2)
			errCash = 3;
	}
	
	if( sysMDBMission.billDeviceStatus != 0 )
	{
		//errStat = 1;
		if(errBill==0)
			errBill = 1;
	}	
	else
	{
		if(errBill==2)
			errBill = 3;
	}
	//===========================================================

	//设备有故障时及时上报start_rpt		
	if(errGOC == 1)
	{
		VPMission_Status_RPT();
		errGOC = 2;
	}
	else if(errGOC == 3)
	{
		VPMission_Status_RPT();
		errGOC = 0;
	}
	
	if(errCoin == 1)
	{
		VPMission_Status_RPT();
		errCoin = 2;
	}
	else if(errCoin == 3)
	{
		VPMission_Status_RPT();
		errCoin = 0;
	}

	if(errTube == 1)
	{
		VPMission_Status_RPT();
		errTube = 2;
	}
	else if(errTube == 4)
	{
		VPMission_Status_RPT();
		errTube = 0;
	}

	if(errCash == 1)
	{
		VPMission_Status_RPT();
		errCash = 2;
	}
	else if(errCash == 3)
	{
		VPMission_Status_RPT();
		errCash = 0;
	}	

	if(errBill == 1)
	{
		VPMission_Status_RPT();
		errBill = 2;
	}
	else if(errBill == 3)
	{
		VPMission_Status_RPT();
		errBill = 0;
	}	
	/*if(errStat == 1)
	{
		if(sysVPMission.DevErrFlag == 0)
		{
			//DisplayStr( 0, 0, 1, "err=1",5 ); 
			VPMission_Status_RPT();
			sysVPMission.DevErrFlag = 1;
		}
	}
	else
	{
		if(sysVPMission.DevErrFlag == 1)
		{
			//DisplayStr( 0, 0, 1, "err=0",5 );  
			VPMission_Status_RPT();
			sysVPMission.DevErrFlag = 0;
		}
	}*/
	//WaitForWork(2000,NULL);
	
	
	//是否进入故障页面
   	if( HardWareErr == 0 )  
	{
	    //Added for the column' 0 status error. 2011.5.12
        /*
	    for( i=0; i<48; i++ )
		{
			if( (GoodsWaySetVal[i].WayState&0x01)==0 )
 			{
				GoodsWaySetVal[i].WayState |= 0x01;
			}
		}
        */
        sysVPMission.hardWareErrShow = 0;//清除运行故障码;by gzz 20111028
		LogicOpen();		
		if(sysVPMission.ErrFlag2 == 1)
		{
			/*if( SystemParameter.ACDCModule == 1 )
			{
				sysVPMission.ACDCLedCtr = 1;         //Open by default.
				if(SystemParameter.CompModule == 1)
			    	sysVPMission.ACDCCompressorCtr = 1;  //Open by default.
			    else
					sysVPMission.ACDCCompressorCtr = 0;  //Open by default.	
				ACDCMission();
			}*/
			sysVPMission.billCoinEnable = 1;
			sysVPMission.ErrFlag2 = 0;
			//if(sysVPMission.ErrFlag==1)
			//{
				//sysVPMission.ErrFlag=0;
				//VPMission_Status_RPT();
			//}
		}		
		return 0;
	}
	/*
	//添加非现金交易模式;by gzz 20110727
	//else if(( HardWareErr & 0x0001 )||( HardWareErr & 0x0002 )||( HardWareErr & 0x0004 )||( HardWareErr & 0x0008 ))
    //添加f0币管没锁紧;by gzz 20110825
	else if(( HardWareErr & 0x0001 )||( HardWareErr & 0x0002 )||( HardWareErr & 0x0004 )||( HardWareErr & 0x0008 )||( HardWareErr & 0x8000 ))
    {
	    if( sysVPMission.systemErr != HardWareErr )
		{
			GetKey();   //Close the selection ready led
            sysVPMission.systemErr = HardWareErr;
		}
	    SetCashLessFlag();
		//if(sysVPMission.ErrFlag==0)
		//{
		//	sysVPMission.ErrFlag=1;
		//	VPMission_Status_RPT();
		//}
		return 1;
	}
	*/
	else  
	{
	    if( sysVPMission.systemErr != HardWareErr )
		{
			GetKey();   //Close the selection ready led
            sysVPMission.systemErr = HardWareErr;
		}
	    SetOffFlag();
		//if(sysVPMission.ErrFlag==0)
		//{
		//	sysVPMission.ErrFlag=1;
		//	VPMission_Status_RPT();
		//}		
		return 1;
	}

}


u_char CheckQureyCmd()
{
	return 0;
}

u_char CycleSendSMS()
{
	return 0;
}

//判断清除友宝销售记录
u_char IsTradeClear()
{
	u_char xdata str[20];
	u_char xdata cmdlen=0;
	
	//1.看是否需要清除友宝销售记录120521 by cq SellAccumulate    
    PrintTest( "IsNeedClear");
	WaitForWork( 3000, NULL );	
	cmdlen=getPcCmd(str);
	if(cmdlen>0)
	{
		//PrintTest( "\n get,%s",str);			
		if((str[0]=='c')&&(str[1]=='l')&&(str[2]=='e')&&(str[3]=='a')&&(str[4]=='r'))
		{
			//PrintTest( "\n get clear");			
			TradeCounter.vpSuccessNum = 0;
			TradeCounter.vpSuccessMoney = 0;
			TradeCounter.vpCashNum = 0;
			TradeCounter.vpCashMoney = 0;
			TradeCounter.vpGameNum = 0;
			//TradeCounter.vpGameMoney = 0;
			TradeCounter.vpCardNum = 0;
			//TradeCounter.vpCardMoney = 0;
			TradeCounter.vpOnlineNum = 0;
			//TradeCounter.vpOnlineMoney = 0;							
			//memset( &TradeCounter, 0, sizeof(TradeCounter) );	

		    //总交易记录
			TradeCounter.CoinSum1y = 0;
			TradeCounter.CoinSum5j = 0;
			TradeCounter.CashSum = 0;
			TradeCounter.Hopper1Sum = 0;
			TradeCounter.Hopper2Sum = 0;
			//区间交易记录
			TradeCounter.CoinSum1yBack = 0;
			TradeCounter.CoinSum5jBack = 0;
			TradeCounter.CashSumBack = 0;
			TradeCounter.Hopper1SumBack = 0;
			TradeCounter.Hopper2SumBack = 0;
			SaveTradeParam();//by gzz 20110610
			PrintTest( "ClearOK");
		}			
	}
	return 0;
}

//判断货道状态
unsigned char GoodState(u_char i)
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


unsigned char isEmpty_Goods()//判断整个机器是否有货可以购买,是返回1,否返回0
{
	u_char xdata i = 0,j=0;	
	/*
	for( j=0; j<GOODSTYPEMAX; j++ )
	{
		for( i=0; i<COLUMN_NUM_SET; i++ )
	    {
	    	if(GoodsWaySetVal[i].GoodsType == SystemParameter.selItem[j])
	    	{
	    		if( ( GoodsWaySetVal[i].GoodsCurrentSum > 0 )&&( GoodsWaySetVal[i].Price > 0 ) )
	    		{
	    			return 1;
	    		}
	    	}
		}
	}*/
	for( i=0; i<COLUMN_NUM_SET; i++ )
    {
		if( ( GoodsWaySetVal[i].GoodsCurrentSum > 0 )&&( GoodState(i) == 1 ) )
		{
			return 1;
		}
	}
	return 0;
}


//判断货道库存和货道状态
u_char CheckGoodsSumAndLoad()
{
	u_char xdata i = 0;	
	u_char xdata j = 0;
	/*
    //Trace( "\n CheckCardSumAndLoad" );
	for( i = 0; i < GOODSWAYNUM; i ++ )
	{
	    if( SystemParameter.GOCOn != 0x01 )
		{
			if(( GoodsWaySetVal[i].GoodsCurrentSum != 0 ) && ( ( GoodsWaySetVal[i].WayState & 0x0A ) == 0x00 ) && 
							( GoodsWaySetVal[i].Price != 0 ) )
				break;
		}
		else
		{
		  //if(( (GoodsWaySetVal[i].WayState & 0x04) == 0 )&&( ( GoodsWaySetVal[i].WayState & 0x0A ) == 0x00 )&&( GoodsWaySetVal[i].Price != 0 ) )
			if( ( GoodsWaySetVal[i].GoodsCurrentSum != 0 ) && ( (GoodsWaySetVal[i].WayState & 0x04) == 0 )&&( ( GoodsWaySetVal[i].WayState & 0x0A ) == 0x00 )&&( GoodsWaySetVal[i].Price != 0 ) )
				break;
		}
	}

	if( i == GOODSWAYNUM )
	{
        //Trace( "\n HardWareErr |= 0x0080 " );
		HardWareErr |= 0x0080;
		for( i = 0; i < GOODSWAYNUM; i ++ )
		{
		    if( SystemParameter.GOCOn != 0x01 )
			{
				if( GoodsWaySetVal[i].GoodsCurrentSum != 0 ) 
					break;
			}
			else
			{
			    if( (GoodsWaySetVal[i].WayState & 0x04) == 0 )
				    break;
			}
		}
	
		if( i == GOODSWAYNUM )
		{
			HardWareErr |= 0x0010;		
            //Trace( "\n CheckCardSumAndLoad all no goods" );
		}
		else
		{
			for( i = 0; i < GOODSWAYNUM; i ++ )
			{
				if( ( GoodsWaySetVal[i].WayState & 0x0A ) == 0x00 )
					break;
			}

			if( i == GOODSWAYNUM )
			{			
                //Trace( "\n CheckCardSumAndLoad colnum no ok" );
				HardWareErr |= 0x0020;
			}
		}		
	}
	*/
    //-----------------------------------------------------
    //UpdateGoodsMatrixStatus();
    /*
	for( i=0; i<GOODSTYPEMAX; i++ )
	{
		//if( sysGoodsMatrix[i].NextColumn != 0xff )
		//	break;
        //添加price=0的货道判断;by gzz 20110614
        if( (sysGoodsMatrix[i].NextColumn != GOODS_MATRIX_NONE)&&(sysGoodsMatrix[i].Price != 0) )
		{
			break;
		}
	}
	*/
	//if( i >= GOODSTYPEMAX )
	if(isEmpty_Goods()==0)
	{
		HardWareErr |= 0x0080;
	}
	//=====================================================
    if( HardWareErr > 0 )
	{
		//if(sysVPMission.ErrFlag==0)
		//{
		//	sysVPMission.ErrFlag=1;
		//	VPMission_Status_RPT();
		//}
	    return 1;
	}
	else
	{
	    sysVPMission.hardWareErrShow = 0;//清除运行故障码;by gzz 20111028 	    
		if(sysVPMission.ErrFlag2 == 1)
		{
			/*if( SystemParameter.ACDCModule == 1 )
			{
				sysVPMission.ACDCLedCtr = 1;         //Open by default.
				if(SystemParameter.CompModule == 1)
			    	sysVPMission.ACDCCompressorCtr = 1;  //Open by default.
			    else
					sysVPMission.ACDCCompressorCtr = 0;  //Open by default.	
				ACDCMission();
			}*/
			sysVPMission.billCoinEnable = 1;
			sysVPMission.ErrFlag2 = 0;
			//if(sysVPMission.ErrFlag==1)
			//{
				//sysVPMission.ErrFlag=0;
				//VPMission_Status_RPT();
			//}
		}
		return 0;
	}
}

//检查是否有币插入
u_char  IsInsertBill()
{	
	u_int  xdata CoinSum = 0;
	u_char xdata flag = 0;
	u_char xdata mission = 0;
    u_char xdata holdFlag = 0;
	//u_int  xdata coinMoney = 0;
    u_char xdata coinFlag = 0;
	u_int  xdata CurrentDomain = 0;
    u_int  xdata BankMaxBuf = 0;
    u_char xdata BankMaxFlag = 0;
	u_char xdata tempTradeTime = 15;
	//u_char xdata m_str[20];
	u_char xdata len = 0;
    u_char xdata length = 0;
	u_char xdata line[26];
	u_int  xdata Money = 0;
	
	if(SystemParameter.tradeTime==255)
		tempTradeTime = 255;
	else if(SystemParameter.tradeTime>=240)
		tempTradeTime = 254;
	else
		tempTradeTime += SystemParameter.tradeTime;

	if( SystemParameter.BillNo == 0 )
		return 0;
	
	mission = MDB_Coin_IfDeposited( sysMDBMission.coinBuf, &sysMDBMission.coinBufLen, &sysMDBMission.coinType, &sysMDBMission.coinStock );
    if( mission == 1 )
	{
		//DisplayStr( 0, 1, 1, "coin2", 4 );
		//WaitForWork( 2000, NULL );
		CoinSum = SystemParameter.coinValue[sysMDBMission.coinType];
		//Update the coin type's coin num and value
		if( sysMDBMission.coinType == 0 )
		{
			sysMDBMission.coin5jNum = sysMDBMission.coinStock;
            sysMDBMission.coin5jValue = sysMDBMission.coin5jNum*SystemParameter.coinValue[sysMDBMission.coinType];
			sysMDBMission.coinAllValue = sysMDBMission.coin5jValue + sysMDBMission.coin1yValue;
		}
		else if( sysMDBMission.coinType == 1 )
		{
		    sysMDBMission.coin1yNum = sysMDBMission.coinStock;
            sysMDBMission.coin1yValue = sysMDBMission.coin1yNum*SystemParameter.coinValue[sysMDBMission.coinType];
			sysMDBMission.coinAllValue = sysMDBMission.coin5jValue + sysMDBMission.coin1yValue;
		}
        coinFlag = 1;
		flag = 1;
        #ifdef _COIN_INPUT_PARALLEL	
			CurrentPayed += CoinSum;
			//TradeCounter.TrueCurrencySum += CoinSum;
			//TradeCounter.CoinSum += CoinSum;
			/*
	        if( CoinSum == SystemParameter.coinValue[0] )
	        {
	        	TradeCounter.CoinSum5j += CoinSum;
	        }
	        else
	        {
	            TradeCounter.CoinSum1y += CoinSum;
	        }
			*/
			//------------------------------------------------------
			sysVPMission.payInCoinFlag = 1;
			sysVPMission.payInCoinMoney += CoinSum;			
			//======================================================
		#else
			CurrentPayed += CoinSum * COINMULTIPLE ;
			//TradeCounter.TrueCurrencySum += CoinSum * COINMULTIPLE;
			//TradeCounter.CoinSum += CoinSum * COINMULTIPLE;
			/*
	        if( ( CoinSum*COINMULTIPLE ) == SystemParameter.coinValue[0] )
	        {
	        	TradeCounter.CoinSum5j += CoinSum*COINMULTIPLE;
	        }
	        else
	        {
	            TradeCounter.CoinSum1y += CoinSum*COINMULTIPLE;
	        }
			*/
			//------------------------------------------------------
			sysVPMission.payInCoinFlag = 1;
			sysVPMission.payInCoinMoney += CoinSum*COINMULTIPLE;			
			//======================================================
		#endif
		//WaitForWork(200,NULL);
		MDB_Coin_TubeSta_API();
		//WaitForWork(200,NULL);
        //reset the trade timer
	    sysVPMission.sTimer1 = tempTradeTime;
	}
    //
    if( coinFlag == 1 )
	{
		if( SystemParameter.coinValue[sysMDBMission.coinType] == SystemParameter.coinValue[0] )
		{
			//总
        	//TradeCounter.CoinSum5j += SystemParameter.coinValue[sysMDBMission.coinType];
        	 SellAccumulateMoney(&TradeCounter.CoinSum5j,SystemParameter.coinValue[sysMDBMission.coinType]);
			//区间
        	//TradeCounter.CoinSum5jBack += SystemParameter.coinValue[sysMDBMission.coinType];
        	SellAccumulateMoney(&TradeCounter.CoinSum5jBack,SystemParameter.coinValue[sysMDBMission.coinType]);
        }
        else if( SystemParameter.coinValue[sysMDBMission.coinType] == SystemParameter.coinValue[1] )
        {
        	//总
            //TradeCounter.CoinSum1y += SystemParameter.coinValue[sysMDBMission.coinType];
            SellAccumulateMoney(&TradeCounter.CoinSum1y,SystemParameter.coinValue[sysMDBMission.coinType]);
			//区间
            //TradeCounter.CoinSum1yBack += SystemParameter.coinValue[sysMDBMission.coinType];
            SellAccumulateMoney(&TradeCounter.CoinSum1yBack,SystemParameter.coinValue[sysMDBMission.coinType]);
        }
		memset(sysMDBMission.coinBuf,0,sizeof(sysMDBMission.coinBuf));
		sysMDBMission.coinBufLen=0;
		sysMDBMission.coinType=0;
		sysMDBMission.coinStock=0;
	}


    //Trace( "\n IsInsertBill" );	
	if( SystemParameter.BillNo == 1 )
	{
	    if( sysITLMission.ITLSet == 1 )
		{
            
			
			mission = MDB_Bill_IfSecrow( sysMDBMission.billBuf, &sysMDBMission.billBufLen, &sysMDBMission.billType );
            if( mission == 1 )
	        {
             	sysITLMission.billSta &= ~ITL_BILL_READ;    //2011.04.13 added
                sysITLMission.billStaCtr |= ITL_BILL_READ;  //2011.04.13 added
                //添加不找零模式的判断;by gzz 20111019
                //if((sysMDBMission.coin5jValue+sysMDBMission.coin1yValue)>=SystemParameter.BanknoteMax)
                if( sysVPMission.tubeCoinMoney > SystemParameter.BanknoteMax )
				{
                 	BankMaxBuf = SystemParameter.BanknoteMax;
                    BankMaxFlag = 0;
                }
                else
                {
                	BankMaxBuf = sysVPMission.tubeCoinMoney;
                    BankMaxFlag = 1;
                }
				//硬币器故障纸币全部暂存
				if( ( sysMDBMission.coinDeviceStatus != 0 )||( sysMDBMission.tubeRemoved == 1 ) )
				{
					BankMaxBuf = 0;
				}
				/*
                if( CurrentPayed+SystemParameter.billValue[sysMDBMission.billType] > BankMaxBuf )
                {
					mission = MDB_Bill_EscrowCtr( 0 );
                    sysITLMission.billStaCtr &= ~ITL_BILL_READ;  //Added by Andy 2011.5.31
                    if(BankMaxFlag == 0)
                    {					
	                    DisplayLine = STR_DOMAIN_ERROR;
						DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
						ClearDisplayLine( 2 );
						WaitForWork( 1500, NULL );					
                    }
                    else
                    {					
	                    DisplayLine = STR_COIN_NOT_ENOUGH;
						DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
						DisplayLine = STR_DOMAIN_ERROR;
						DisplayStr( 0, __LINE1__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
						WaitForWork( 1500, NULL );					
                    }
					DisplayCurrentMoney( CurrentPayed );
				}
                else
                {
                */  
                //len = sprintf( m_str, "bill=%u.%02u,max=%u.%02u", (CurrentPayed+SystemParameter.billValue[sysMDBMission.billType])/SystemParameter.curUnit,(CurrentPayed+SystemParameter.billValue[sysMDBMission.billType])%SystemParameter.curUnit,BankMaxBuf/SystemParameter.curUnit,BankMaxBuf%SystemParameter.curUnit );
				//DisplayStr( 0, 1, 1, m_str, strlen(m_str) );
				//WaitForWork( 500, NULL );
				
                if((CurrentPayed+SystemParameter.billValue[sysMDBMission.billType]) >= BankMaxBuf)
                {  
                	sysITLMission.billHoldingFlag = 1;
					sysITLMission.billHoldingValue = SystemParameter.billValue[sysMDBMission.billType];
					sysVPMission.escrowInFlag = 1;
					sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
                    UpdateSelLed_Trade();
					DisplayCurrentMoney( CurrentPayed/*+sysITLMission.billHoldingValue*/ );								
                    //reset the trade timer
					sysVPMission.sTimer1 = tempTradeTime;
            		flag = 1;
					sysITLMission.restartTime = ITL_TIME_RESTART; //reset the timer!
					//----------------------------------------------------------------------
					    //Send the holding message to PC in here!
                               
				}
                //正常压钞模式
                else
                {
                    sysITLMission.billSta    &= ~ITL_BILL_CREDIT;
		            sysITLMission.billStaCtr &= ~ITL_BILL_READ;
		            mission = MDB_Bill_EscrowCtr( 1 );
					if(mission==0)
					{
					    sysVPMission.billSTimer = TIME_BILL_STACK;
					    while( sysVPMission.billSTimer )
						{
                            mission = MDB_Bill_IfStacked( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
							if( mission == 1 )
							{
							    CurrentDomain = SystemParameter.billValue[sysMDBMission.billType];
								Trace( "\n Get in Cash Ok" );
								//CurrentDomain = GetCash();
								Trace( "\n CurrentDomain = %d",CurrentDomain );		
								CurrentPayed += CurrentDomain * BILLMULTIPLE;	
								sysVPMission.inBillMoney += CurrentDomain * BILLMULTIPLE;//保存纸币压抄金额;
								//TradeCounter.TrueCurrencySum += CurrentDomain * BILLMULTIPLE;
								//TradeCounter.CashSum += CurrentDomain * BILLMULTIPLE;
								//TradeCounter.CashSumBack += CurrentDomain * BILLMULTIPLE;//区间
								SellAccumulateMoney(&TradeCounter.CashSum,CurrentDomain * BILLMULTIPLE);
								SellAccumulateMoney(&TradeCounter.CashSumBack,CurrentDomain * BILLMULTIPLE);
								//------------------------------------------------------
								sysVPMission.payInBillFlag = 1;
								sysVPMission.payInBillMoney += CurrentDomain * BILLMULTIPLE;
								//reset the trade timer
						        sysVPMission.sTimer1 = tempTradeTime;
								//======================================================
								sysITLMission.restartTime = ITL_TIME_RESTART; //reset the timer!
								memset(sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));
								sysMDBMission.billBufLen=0;
								sysMDBMission.billType=0;
								sysMDBMission.billStock=0;
								flag = 1;
                                break;//压钞成功后，直接跳出时间循环等待;by gzz 20110721 
							}
							else
							{
							    VP_GameKeyPoll( 200 );
							}
						}
                        //钞箱满，钞箱压钞后就能时，还可以认到纸币;by gzz 20111017
                        if( sysVPMission.billSTimer == 0 )
                        {							    
							//检测纸币器钞箱是否已满;by gzz 20111014 
							WaitForWork(2000,NULL);
                            MDB_Bill_Stacker( &sysMDBMission.billBuf[0], &sysMDBMission.billBuf[1] ); 
                        	if(sysMDBMission.billIsFull==1)
                            {
							    CurrentDomain = SystemParameter.billValue[sysMDBMission.billType];
								Trace( "\n Get in Cash Ok" );
								//CurrentDomain = GetCash();
								Trace( "\n CurrentDomain = %d",CurrentDomain );		
								CurrentPayed += CurrentDomain * BILLMULTIPLE;	
								//TradeCounter.TrueCurrencySum += CurrentDomain * BILLMULTIPLE;
								//TradeCounter.CashSum += CurrentDomain * BILLMULTIPLE;
								//TradeCounter.CashSumBack += CurrentDomain * BILLMULTIPLE;//区间
								SellAccumulateMoney(&TradeCounter.CashSum,CurrentDomain * BILLMULTIPLE);
								SellAccumulateMoney(&TradeCounter.CashSumBack,CurrentDomain * BILLMULTIPLE);
								//------------------------------------------------------
								sysVPMission.payInBillFlag = 1;
								sysVPMission.payInBillMoney += CurrentDomain * BILLMULTIPLE;
								//reset the trade timer
						        sysVPMission.sTimer1 = tempTradeTime;
								//======================================================
								sysITLMission.restartTime = ITL_TIME_RESTART; //reset the timer!
								memset(sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));
								sysMDBMission.billBufLen=0;
								sysMDBMission.billType=0;
								sysMDBMission.billStock=0;
								flag = 1;                                    
							}
                        }
		            }
					
                }
                
            }
			
		}
		else
		{
			if ( CasherIsCashIn())
			{
				Trace( "\n Casher" ); 
				flag = 1;
			}
			WaitForWork( 10, NULL );
		}
	}
    
		
	//WaitForWork( 10, NULL );
	if( flag == 0 )
	{	
        //Trace( "\n NOT Bill Insert" );
		if( ( sysVPMission.dspCtr & 0x01 ) )
		{
			sysVPMission.dspCtr &= 0xfe;
			if( sysITLMission.billHoldingFlag == 1 )
			{
				if( (CurrentPayed+sysITLMission.billHoldingValue) >= 1 )
				{
					DisplayCurrentMoney( CurrentPayed );
				}
			}
			else
			{
				if( CurrentPayed >= 1 )
				{
					DisplayCurrentMoney( CurrentPayed );
				}
			}
		}

		Money = CurrentPayed;
		if( sysITLMission.billHoldingFlag == 1 )
		{
			Money += sysITLMission.billHoldingValue;
		}
		
		if( (sysVPMission.sTimer1 < 255)&&(Money > 0) )
		{
			length = sprintf( line , "%s 倒计时[%u]" , DispInformationTable[STR_VP_SELECTGOODS].str,sysVPMission.sTimer1 );
			DisplayStr( 0, 1, 1,line, strlen(line) );  
			//WaitForWork(200,NULL);
		}
		return 2;
	}
	else
	{
        //Trace( "\n Have Bill Insert" );
		return	1;
	}
}

//有数字键输入，返回1,否则返回2
u_char CheckKeyPress()
{
	u_char xdata key = 0;

    //Trace( "\n CheckKeyPress" );
	//if( SystemStatus != 1 )
	if( (SystemStatus != 1) && (sysVPMission.SystemState != 1) )//只有暂停服务模式，才做这个;by gzz 20110727
    { 
	    //Added to close the ready led
		//2011.7.17 But isn't a good solution here! 
	    //Selection1Query();
		//Selection2Query();
		//Selection3Query();
	    return 2;             //系统关闭，按键不理,当做没按
    }		
	key = GetKey();
    //Trace( "\n CheckKeyPress key1 = %bu", key );
	//--------------------------------------------------------------
	//if( ( key > 0 ) && ( key <= 9 ) )
#ifdef   _SHUPING_
	if( ( key > 0 ) && ( key <= 9 ) )
	{
//		Trace( "\n CheckKeyPress key2 = %bu", key );
		CurrentStockCode = key + CurrentStockCode*10;
		if( QueryFlag == 0 )
		{
			QueryFlag = 1;
			QueryTimer = 30;
		}
		return 1;
	}
#else
	if( ( key >= SELECTION_VAL_MIN )&&( key <= SELECTION_VAL_MAX ) )
	{
        //Trace( "\n CheckKeyPress key2 = %bu", key );
		//CurrentStockCode = key + CurrentStockCode*10;
		//CurrentStockCode = key;
        //sysVPMission.key = sysVPMission.selItem[key - 1];
		//sysVPMission.key = SystemParameter.selItem[key - 1];
		sysVPMission.key = key;
		sysVPMission.goodsType = key;   //2011.2.23 reenabled for the card trade bug.
	    //===============================================================
	    /*---------------------------------------------------------------
		Disabled by 2011.6.14
		if( QueryFlag == 0 )
		{
			QueryFlag = 1;
			QueryTimer = 30;
		}
		==================================================================
		*/
		return 1;
	}
#endif
	else if( key == KEY_CANCEL )
	{
		DisplayCursorState( 0, 1, 1, 0, 1 );
		QueryFlag = 0;
		QueryTimer = 0;
		CurrentStockCode = 0;
		UpdataDisp = 6;
		return 2;
	}		
	else
	{
		if( ( QueryFlag == 1 ) && ( QueryTimer == 0 ) )
		{
			DisplayCursorState( 0, 1, 1, 0, 1 );
			QueryFlag = 0;
			QueryTimer = 0;
			CurrentStockCode = 0;
			UpdataDisp = 6;
		}
		return 2;	
	}
	return 2;
}

//显示输入的货道，并判断输入是否正确，是否存在此货道,输入正确返回0,输入失败返回1
u_char QueryDisplayInputCode()
{
	u_char xdata MyKey = 0 ;
	u_char xdata len = 0;

	Trace("\n QueryDisplayInputCode" );
	//--------------------------------------------------
	#ifdef   _SHUPING_
		memset( DisplayBuffer, 0, sizeof( DisplayBuffer ) );															
		DisplayLine = STR_INPUT_GOODSNO;	
		if( CurrentStockCode != 0 )
		{
			len = sprintf( DisplayBuffer,"%s%bu", DispInformationTable[DisplayLine].str, CurrentStockCode );		
			DisplayStr( 0, __LINE0__, 1, DisplayBuffer, len + 1 );
			if( CurrentStockCode < 10 )
			{
				ClearDisplayLine( 2 );
				DisplayCursorState( len, __LINE0__, 1, 1, 1 );					
				return 1;
			}
			else
				DisplayCursorState( 0, 1, 1, 0, 1 );
		}
		else
		{
			DisplayCursorState( 0, 1, 1, 0, 1 );
			QueryFlag = 0;
			QueryTimer = 0;
			CurrentStockCode = 0;
			UpdataDisp = 6;
			return 1;
		}

		ClearKey();
		if( ( CurrentStockCode > 99 ) || ( CurrentStockCode <= 0 ) )
		{
			DisplayCursorState( 0, 1, 1, 0, 1 );
			// 客户输入错误 		
			DisplayLine = STR_INVAILD_CHOICE;
			DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
			DisplayLine = STR_RE_INPUT_GOODS;
			DisplayStr( 0, __LINE1__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
			WaitForWork( 2000 , NULL );
			QueryFlag = 0;
			QueryTimer = 0;
			CurrentStockCode = 0;
			UpdataDisp = 6;
			return 1 ;
		}	
		//输入规则正确	
		MyKey = 0;
		for( len = 0; len < GOODSWAYNUM; len ++ )
		{
			if( CurrentStockCode == InputGoodsWayList[len].GoodsWayNo )
			{
				CurrentStockCode = len;//InputGoodsWayList[len].SetArrayNo;
				MyKey = 1;
				break;
			}
		}
		if( MyKey == 0 )
		{
			DisplayCursorState( 0, 1, 1, 0, 1 );
			// 客户输入错误 		
			DisplayLine = STR_INVAILD_CHOICE;
			DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
			DisplayLine = STR_RE_INPUT_GOODS;
			DisplayStr( 0, __LINE1__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
			WaitForWork( 2000 , NULL );
			QueryFlag = 0;
			QueryTimer = 0;
			CurrentStockCode = 0;
			UpdataDisp = 6;
			return 1;
		}
		Trace( "\n QueryDisplayInputCode CurrentStockCode = %bu", CurrentStockCode );	
	#endif
	return 0;
	//==================================================
}

//在屏幕上显示各个货道的状态提示
void DispGoodsState(uchar state)
{
	switch(state)
	{
		//货道故障:
		//{"  故障", 8 },                //42			
		case 1:
			DisplayLine = STR_COLUMN_ERROR;
			DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
			break;
		//货没有开启或商品id为ff或商品id为0	
		// {"  该货道暂停服务", 16    },      //57
		case 2:
			DisplayLine = STR_GOODS_WAY_ERR3;	
			DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );			
			break;
		//单价为0	
		//{"  非现金售卖", 16    },      //58
		case 3:
			DisplayLine = STR_GOODS_WAY_NOSALE;	
			DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
			break;
		//缺货	
		//{"  商品已售完", 12 },//14	
		case 4:
			Beep();
			WaitForWork(200,NULL);
			Beep();
			DisplayLine = STR_GOODS_SOLDOUT;
			DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
			break;
	}
	//{"  请选择其它商品", 16 },       //25
	DisplayLine = STR_INPUT_WAY_NO;
	DisplayStr( 0, __LINE1__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );		
	WaitForWork( 2000, NULL );   //3000, 2000,
}
//判断货道是否正常并显示
u_char QueryIsGoodsWay()
{
	uchar xdata i = 0;
	uchar xdata len = 0;
    uchar xdata dspLine = 0;
	uchar xdata flag = 0;
	//u_char xdata str[20];

	//ClearDisplayLine( 1 );
	Trace( "\n QueryIsGoodsWay" );	
	Trace("\n GoodsWayTest GoodsWaySetVal[%bu].WayState = %02bx", CurrentStockCode,GoodsWaySetVal[ CurrentStockCode ].WayState );
	Trace("\n GoodsWaySetVal[%bu].Price = %u", CurrentStockCode,GoodsWaySetVal[CurrentStockCode].Price );
	memset( DisplayBuffer, 0, sizeof( DisplayBuffer ) );
	//----------------------------------------------------------------------------------------------------------------------
	/*
    for( len = 0; len < GOODSTYPEMAX; len ++ )
	{
		if( ( CurrentStockCode == sysGoodsMatrix[len].GoodsType ) && ( sysGoodsMatrix[len].ColumnNum > 0 ) )
		{
		    if( sysGoodsMatrix[len].NextColumn == 0xff ) //If none good column left in this matrix, with the defualt one.
			{
            	CurrentStockCode = sysGoodsMatrix[len].ColumnMatrix[0];    
			}
			else
			{
				CurrentStockCode = sysGoodsMatrix[len].NextColumn;
			}
			break;
		}
	}
    
    for( i=0; i<GOODSTYPEMAX; i++ )
    {
    	if(sysGoodsMatrix[i].GoodsType == SystemParameter.selItem[sysVPMission.key])
    	{
    		len = sprintf( str, "%02bu,%02bu", sysVPMission.key,SystemParameter.selItem[sysVPMission.key] );
					DisplayStr( 0, 1, 1, str, len );
					WaitForWork( 2000, NULL );
		    if( sysGoodsMatrix[i].ColumnNum > 0 )
		    {
		        if( sysGoodsMatrix[i].NextColumn != GOODS_MATRIX_NONE )
		        {
		        	CurrentStockCode = sysGoodsMatrix[i].NextColumn;
					len = sprintf( str, "%02bu", CurrentStockCode );
					DisplayStr( 0, 1, 1, str, len );
					WaitForWork( 2000, NULL );
		        }        
		    }
		    else
		    {
		    	sysGoodsMatrix[i].NextColumn = GOODS_MATRIX_NONE;
		    }
			break;
    	}
    }
	//len = sprintf( str, "%02bu,%02bu,%02bu", sysVPMission.key,SystemParameter.selItem[sysVPMission.key],CurrentStockCode );
	//	DisplayStr( 0, 1, 1, str, len );
	//	WaitForWork( 500, NULL );
	len = sprintf( str, "%02bu,%02bu", i,sysGoodsMatrix[i].NextColumn );
		DisplayStr( 0, 0, 1, str, len );		
	len = sprintf( str, "%02bu,%02bu,%02bu", sysVPMission.key,SystemParameter.selItem[sysVPMission.key],CurrentStockCode );
		DisplayStr( 0, 1, 1, str, len );
		WaitForWork( 2000, NULL );
	*/	
	#ifdef   _SHUPING_
		//按下购物按钮时，予以上报
		VPMission_Button_RPT( VP_BUT_GOODS, CurrentStockCode+1 );
	#else
		for( i=0; i<COLUMN_NUM_SET; i++ )
	    {
	    	if(GoodsWaySetVal[i].GoodsType == SystemParameter.selItem[sysVPMission.key])
	    	{
	    		//len = sprintf( str, "%02bu,%02bu", sysVPMission.key,SystemParameter.selItem[sysVPMission.key] );
				//		DisplayStr( 0, 1, 1, str, len );
				//		WaitForWork( 2000, NULL );
				//CurrentStockCode = i;		
			    if( ( GoodsWaySetVal[i].GoodsCurrentSum > 0 )&&(GoodStateLed(i)==1) )
			    {		        
		        	CurrentStockCode = i;
					//len = sprintf( str, "%02bu", CurrentStockCode );
					//( 0, 1, 1, str, len );
					//WaitForWork( 2000, NULL );	        
					break;
			    }
	    	}
	    }
		// = sprintf( str, "%02bu", i );
		//	DisplayStr( 0, 0, 1, str, len );		
		//len = sprintf( str, "%02bu,%02bu,%02bu", sysVPMission.key,SystemParameter.selItem[sysVPMission.key],CurrentStockCode );
		//	DisplayStr( 0, 1, 1, str, len );
		//	WaitForWork( 500, NULL );
	    //=====================================================================================================================
	    /*
		if( (SystemParameter.GOCOn != 0x01)&&(GoodsWaySetVal[CurrentStockCode].GoodsCurrentSum==0) ) //Added by Andy on 2010.10.26
		{	
			DisplayLine = STR_COLUMN;	
			i = sprintf( DisplayBuffer, "%s %bu - %s", DispInformationTable[DisplayLine].str, GoodsWaySetVal[CurrentStockCode].WayNo,
		 			DispInformationTable[STR_GOODS_OUT].str );				
		}
		else if( (SystemParameter.GOCOn==0x01)&&(GoodsWaySetVal[CurrentStockCode].WayState&0x04) ) //Added by Andy on 2010.10.26
		{
			DisplayLine = STR_COLUMN;	
			i = sprintf( DisplayBuffer, "%s %bu - %s", DispInformationTable[DisplayLine].str, GoodsWaySetVal[CurrentStockCode].WayNo,
		 			DispInformationTable[STR_GOODS_OUT].str );
		}
		else if( ( ( GoodsWaySetVal[CurrentStockCode].WayState & 0x0A ) != 0x00 ) || 
				( GoodsWaySetVal[CurrentStockCode].Price == 0 ) || ( ( GoodsWaySetVal[CurrentStockCode].WayState & 0x01 ) != 0x01 ) )
				//货道故障或单价为0		
		{
			DisplayLine = STR_COLUMN;	
			i = sprintf( DisplayBuffer, "%s %bu - %s", DispInformationTable[DisplayLine].str, GoodsWaySetVal[CurrentStockCode].WayNo,
		 			DispInformationTable[STR_COLUMN_ERROR].str );			
		}
		*/

	    //if( sysGoodsMatrix[i].NextColumn == GOODS_MATRIX_NONE )
	    if(i>=COLUMN_NUM_SET)
	    {
	    	/*
		    DisplayLine = STR_COLUMN;	
			i = sprintf( DisplayBuffer, "%s%s", DispInformationTable[DisplayLine].str, DispInformationTable[STR_GOODS_OUT].str );
	        */
	        DispGoodsState(4);
			ClearKey();
			QueryFlag = 0;
			QueryTimer = 0;
			CurrentStockCode = 0;
			UpdataDisp = 6;
			return 0;

	    }
		VPMission_Button_RPT( VP_BUT_GOODS, CurrentStockCode+1 );
	#endif
	/*
	else if( GoodsWaySetVal[CurrentStockCode].GoodsCurrentSum == 0 )
	{	
	    
		DisplayLine = STR_GOODS_SOLDOUT;
		DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
		DisplayLine = STR_INPUT_WAY_NO;
		DisplayStr( 0, __LINE1__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );		
		WaitForWork( 2000, NULL );
		ClearKey();
		QueryFlag = 0;
		QueryTimer = 0;
		CurrentStockCode = 0;
		UpdataDisp = 6;
		return 0;
	}
	else if( ( ( GoodsWaySetVal[CurrentStockCode].WayState & 0x0A ) != 0x00 ) || 
			( ( GoodsWaySetVal[CurrentStockCode].WayState & 0x01 ) != 0x01 ) )
			//货道故障或单价为0		
	{
		
		DisplayLine = STR_GOODS_SOLDOUT;
		DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
		DisplayLine = STR_INPUT_WAY_NO;
		DisplayStr( 0, __LINE1__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );		
		WaitForWork( 2000, NULL );
		ClearKey();
		QueryFlag = 0;
		QueryTimer = 0;
		CurrentStockCode = 0;
		UpdataDisp = 6;
		return 0;
	}
	*/
	//货道故障:
	if( GoodsWaySetVal[CurrentStockCode].WayState & 0x0A )
	{
		DispGoodsState(1);
	}
	//货没有开启或商品id为ff或商品id为0
	else if(( GoodsWaySetVal[CurrentStockCode].WayState == 0 ) || ( GoodsWaySetVal[CurrentStockCode].GoodsType == 0xff )|| ( GoodsWaySetVal[CurrentStockCode].GoodsType == 0x00 ))
	{
		DispGoodsState(2);		
	}
	//单价为0
	else if( GoodsWaySetVal[CurrentStockCode].Price == 0 )
	{
		DispGoodsState(3);	
	}
	//缺货
	else if( (GoodsWaySetVal[CurrentStockCode].WayState & 0x04) || (GoodsWaySetVal[CurrentStockCode].GoodsCurrentSum == 0) )
    {	
    	DispGoodsState(4);				
	}	
	else
	{		
		DisplayLine = STR_PRICE;
		switch( SystemParameter.curUnit )
		{
			case 1:
				i = sprintf( DisplayBuffer, "%s %u", DispInformationTable[DisplayLine].str,	GoodsWaySetVal[CurrentStockCode].Price );
				break;				
			case 10:
				i = sprintf( DisplayBuffer, "%s %u.%u", DispInformationTable[DisplayLine].str,
					GoodsWaySetVal[CurrentStockCode].Price/SystemParameter.curUnit,GoodsWaySetVal[CurrentStockCode].Price%SystemParameter.curUnit );				
				break;
			case 100:
				i = sprintf( DisplayBuffer, "%s %u.%02u", DispInformationTable[DisplayLine].str,
					GoodsWaySetVal[CurrentStockCode].Price/SystemParameter.curUnit,GoodsWaySetVal[CurrentStockCode].Price%SystemParameter.curUnit );				
			break;		
		}
        //--------------------------------------------------------------------------------------------
        //Here is for SELECTION_PAY(Cash or Card)_DISPENSE_MODE
		
		ClearKey();
		QueryFlag = 0;
		QueryTimer = 0;
		//CurrentStockCode = 0;
		UpdataDisp = 6;
		/*
	  //if( ( SystemParameter.busCardOn == 1 ) && ( DeviceStatus.BusCard == 0x00 ) )
        if( ( SystemParameter.busCardOn == 1 ) && ( (DeviceStatus.BusCard & 0xfb) == 0x00 ) && ( GoodsWaySetVal[CurrentStockCode].Price > 0 ) )
		{
        	DisplayStr( 0, __LINE1__, 1, DisplayBuffer, i );
        	dspLine = STR_BC_SCAN_CARD;
			DisplayStr( 0, __LINE0__ , 1, DispInformationTable[dspLine].str, DispInformationTable[dspLine].len );		
            
			//WaitForWork( SYS_WAIT_CARD_IN, NULL );
            WaitForWork( SystemParameter.BCReadTime*1000, NULL );

			TradeCounter.BCTradeSn ++;
            flag = BCMission_Trade( GoodsWaySetVal[CurrentStockCode].Price, TradeCounter .BCTradeSn );
            
			if( flag == BC_ERR_NULL )
			{
			    if( sysBCMission.recACK == BC_ACK_OK )
				{
                    Beep();
				    VPMission_Card_RPT();
					VPMission_Payin_RPT( VP_DEV_BILLCOIN, GoodsWaySetVal[CurrentStockCode].Price );
					CurrentPayed = GoodsWaySetVal[CurrentStockCode].Price;
					return 1;	
				}
				else if( sysBCMission.recACK == BC_ACK_MONEY_LOW )
				{
				    DisplayLine = STR_VP_CARD_MONEYLOW;	
                    DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
				    WaitForWork( 2000, NULL );	
				}
                else if( (sysBCMission.recACK == BC_ACK_READERR)||(sysBCMission.recACK == BC_ACK_BALCKCARD)||(sysBCMission.recACK == BC_ACK_UNKOWNCARD) )
                {
                    DisplayLine = STR_VP_CARD_READERR;	
                    DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
				    WaitForWork( 2000, NULL );	
                }
			}
			else
			{
			    DisplayLine = STR_VP_CARD_READERR;	
                DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
				WaitForWork( 2000, NULL );		
			}
		}
		//只有非现金交易模式，才进行PC端卡交易;by gzz 20110727
		else if( (SystemStatus != 1) && (sysVPMission.SystemState == 1) && ( GoodsWaySetVal[CurrentStockCode].Price > 0 ) )
        {
			DisplayStr( 0, __LINE1__, 1, DisplayBuffer, i );
        	//dspLine = STR_BC_SCAN_CARD;
			//DisplayStr( 0, __LINE0__ , 1, DispInformationTable[dspLine].str, DispInformationTable[dspLine].len );		
            
			//WaitForWork( SYS_WAIT_CARD_IN, NULL );
            WaitForWork( 5000, NULL );	
        } 
		else
		*/
		{
		    DisplayStr( 0, __LINE1__, 1, DisplayBuffer, i );
			WaitForWork( 2000, NULL );   //3000-2000
        } 
        /*
		ClearKey();
		QueryFlag = 0;
		QueryTimer = 0;
		CurrentStockCode = 0;
		UpdataDisp = 6;
		*/
		CurrentStockCode = 0;
		return 0;
        //============================================================================================				
	}
	ClearKey();
	QueryFlag = 0;
	QueryTimer = 0;
	CurrentStockCode = 0;
	UpdataDisp = 6;
	return 0;
}

u_char DispWelOrOutServ()
{
	u_char xdata j = 0;
	
	#ifdef   _SHUPING_
		//显示欢迎信息或暂停服务
		if( ( QueryFlag == 1 ) && ( QueryTimer != 0 ) )
		{
			return 1;
		}
		else
		{
			CurrentStockCode = 0;	
	    }
	#endif
   
	if( HardWareErr == 0 )
	{
		SystemStatus = 1;
	}
    //
	if( SystemStatus == 1)
	{				
        
		if( (sysVPMission.dspUpdateFlag==1) &&(sysVPMission.dspTimer1>=VP_DSP_TIME2) )
		{
		     sysVPMission.dspTimer1 = 0x00;
			 /*
			 if( ( SystemParameter.busCardOn == 1 ) && ( (DeviceStatus.BusCard & 0xfb) == 0x00 ) )
			 {
			 	 DisplayLine = STR_VP_CASHLESS_TRADE;	
	             DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
				 WaitForWork( 50, NULL );
				 DisplayLine = STR_VP_S_C_G;
	             DisplayStr( 0, __LINE1__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
				 WaitForWork( 50, NULL );
	             if( GetDisplayState() == 1 )
				 {
					DisplayStr( 0, __LINE1__ , 1 , DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
					WaitForWork( 50, NULL );
					if( GetDisplayState() == 1 )
					{
						Trace( "\n LCD OutTime" );	
						HardWareErr |= 0x0200;//LCD通讯不通
						return 1;
					}
				 }	
			 }
			 else
			 */
			 {
			     DisplayLine = STR_VP_CASH_TRADE;	
	             DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
				 WaitForWork( 50, NULL );
				 DisplayLine = STR_VP_M_S_G;
	             DisplayStr( 0, __LINE1__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
				 WaitForWork( 50, NULL );
	             if( GetDisplayState() == 1 )
				 {
					DisplayStr( 0, __LINE1__ , 1 , DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
					WaitForWork( 50, NULL );
					if( GetDisplayState() == 1 )
					{
						Trace( "\n LCD OutTime" );	
						HardWareErr |= 0x0200;//LCD通讯不通
						return 1;
					}
				 }	 		 
			 }
		}
		else if( (sysVPMission.dspUpdateFlag==1) && (sysVPMission.dspTimer1>=VP_DSP_TIME1) )
		{
		     DisplayLine = STR_VP_CASH_TRADE;	
             DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
			 WaitForWork( 50, NULL );
			 DisplayLine = STR_VP_M_S_G;
             DisplayStr( 0, __LINE1__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
			 WaitForWork( 50, NULL );
             if( GetDisplayState() == 1 )
			 {
				DisplayStr( 0, __LINE1__ , 1 , DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
				WaitForWork( 50, NULL );
				if( GetDisplayState() == 1 )
				{
					Trace( "\n LCD OutTime" );	
					HardWareErr |= 0x0200;//LCD通讯不通
					return 1;
				}
			 }	 	
		}

	}
	else if( (SystemStatus != 1) && (sysVPMission.SystemState != 1) )//只有暂停服务模式，才显示暂停服务;by gzz 20110727
	{		
		DisplayLine = STR_OUT_OF_SERVICE;	
		DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
        //ClearDisplayLine1();
		ClearDisplayLine( 2 );
		//WaitForWork( 500, NULL );		
	}
	UpdataDisp++;

	//
	if( IsInsertBill() == 1 )
	{
		return 2;
	}
	else
	{
		//WaitForWork( 500, NULL );
        WaitForWork( 50, NULL );
		return 1;
	}
}

u_char GetBill( )
{
	u_int  xdata CurrentDomain = 0;
	u_int  xdata CoinSum = 0;
	u_int  xdata coinMoney = 0;
	u_char xdata coinFlag = 0;
	u_char xdata mission = 0;
	//u_char xdata tempTradeTime = 15;
	/*
    //Trace( "\n CheckBill");
	if(SystemParameter.tradeTime==255)
		tempTradeTime = 255;
	else if(SystemParameter.tradeTime>=240)
		tempTradeTime = 254;
	else
		tempTradeTime += SystemParameter.tradeTime;
	*/
	CurrentDomain = 0;
    //CurrentTransType = 1;   //正在交易
   

	if( (sysITLMission.ITLSet != 1) )
	{
		if( CurrentPayed > 0 )    //确实有钱收入了才是进入了服务状态
			CasherGetMachineState( 1 );//交易状态
    }

	//ClearDisplayLine( 2 );	
	if( KeyLockFlag )
	{
        //Trace( "\n KeyUnLock");
		KeyUnLock(); //打开键盘，让客户按键
		KeyLockFlag = 0;		
	}	
	
	if( QueryFlag == 1 )
	{
		DisplayCursorState( 0, 1, 1, 0, 1 );
		QueryFlag = 0;
		QueryTimer = 0;
		CurrentStockCode = 0;
	}

    
	/*
	//
	if( (SystemParameter.BillNo == 1)&&(sysITLMission.ITLSet != 1) )
	{
		if( CasherIsCashIn() )
		{
			CurrentDomain = GetComingCash( );//读取已检测到的纸币面额	
			if( CurrentDomain == 0 )//不可识别的纸币
			{
				Trace( "\n Do not know bill" );		
				RejectBill();//退钞				
			}
			else
			{
				if( CurrentPayed + CurrentDomain*BILLMULTIPLE > SystemParameter.BanknoteMax )
				{
					//收到的钱超出了限制,退出纸币
					DisplayLine = STR_DOMAIN_ERROR;
					DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
					ClearDisplayLine( 2 );
					WaitForWork( 1500, NULL );				
					RejectBill();//退钞			
					if( CurrentPayed == 0 )
					{
						UpdataDisp = 0;
						return 2;
					}
					else
					{
						DisplayCurrentMoney( CurrentPayed );
						return 0;
					}
				}
				Trace( "\n Have cash in");		
				Trace( "\n CurrentPayed = %d",CurrentPayed );		
				CasherAccept(); //准备收下		
				LzjSecTimer = DEFAULT_GET_BILL_TIMEOUT;
				while( LzjSecTimer )
				{
					if( CasherIsAcceptOK() )							
						break;			
					SchedulerProcess();
				}
				if( LzjSecTimer == 0 )
				{
					Trace( "\n Out Cash" );
					RejectBill();//退钞
					SystemParameter.DoubtCash = CurrentDomain * BILLMULTIPLE;				
					SaveGlobalParam();
					if( CurrentPayed == 0 )
					{					
						UpdataDisp = 0;
						return 2;	
					}
				}
				else
				{
					Trace( "\n Get in Cash Ok" );
			        //CurrentDomain = GetCash();
					Trace( "\n CurrentDomain = %d",CurrentDomain );		
					CurrentPayed += CurrentDomain * BILLMULTIPLE;	
					TradeCounter.TrueCurrencySum += CurrentDomain * BILLMULTIPLE;
					TradeCounter.CashSum += CurrentDomain * BILLMULTIPLE;
					//------------------------------------------------------
					sysVPMission.payInBillFlag = 1;
					sysVPMission.payInBillMoney += CurrentDomain * BILLMULTIPLE;
					//reset the trade timer
	                sysVPMission.sTimer1 = tempTradeTime;
					//======================================================
				}			
			}	
			WaitForWork( 20, NULL );
		}
	}	
	*/
	
	//判断收入的金额
	coinFlag = 0;
	coinMoney = 0;
	if( CurrentPayed >= SystemParameter.BanknoteMax )
	{	
	    //DisplayStr( 0, 0, 1, "1", 1 );  
	    //WaitForWork(2000,NULL);
		DisableBillDev();              //禁能纸、硬币品
		DisplayCurrentMoney( CurrentPayed );
        //Added 2011.4.13. Update the the goods's led, when the balance exceed the max value.
        UpdateSelLed_Trade(); 
		/*
		if( (SystemParameter.BillNo == 1)&&(sysITLMission.ITLSet !=1 ) )
		{			
			LzjSecTimer = 1;
			while( LzjSecTimer )
			{
				if( CasherIsCashIn() ) //是否有纸币进入
				{	
					RejectBill();      //退钞																
					break;
				}
				WaitForWork( 10, NULL );
			}
		}		
		*/
		//WaitForWork( 500, NULL );
		return 1;		
	}	
	else	
	{
		DisplayCurrentMoney( CurrentPayed );
		UpdateSelLed_Trade();
		return 2;	
	}
}



u_char IsInputGoodsNo()
{
	u_char xdata key = 0;
	u_int  xdata moneyBuf = 0;

    //Trace( "\n IsInputGoodsNo" );
	//if( SystemStatus != 1 ) 
	if( (SystemStatus != 1) && (sysVPMission.SystemState != 1) )//只有暂停服务模式，才做这个;by gzz 20110727
		return 1; // 系统关闭，按键不理,当做没按
    
	if( sysITLMission.billHoldingFlag == 1 )
	{
        if( (CurrentPayed+sysITLMission.billHoldingValue) <= 0 )
		return 2;
	}
	else if( CurrentPayed <= 0 )
	{
		return 2;
	}    
	WaitForWork( 10, NULL );		
	key = GetKey();    
	//------------------------------------------------------
	//if( ( key >= 0 ) && ( key <= 9 ) )
#ifdef   _SHUPING_
	if( ( key > 0 ) && ( key <= 9 ) )
	{
//		Trace( "\n CheckKeyPress key2 = %bu", key );
		CurrentStockCode = key + CurrentStockCode*10;
		return 0;
	}
	else
	{
		if(key == KEY_CANCEL)
		{
			CurrentStockCode = 0;
			sysVPMission.dspCtr |= 0x01;
			DisplayCursorState( 0, 1, 1, 0, 1 );
			if( sysITLMission.billHoldingFlag == 1 )
			{
				moneyBuf = CurrentPayed + sysITLMission.billHoldingValue;
			}
			else
			{
			    moneyBuf = CurrentPayed;
			}
			if( moneyBuf < SystemParameter.BanknoteMax )
            {
            	EnableBillDev();		
            }
		}
		return 1;
	}
#else
	if( ( key >= SELECTION_VAL_MIN ) && ( key <= SELECTION_VAL_MAX ) )
	{
		//CurrentStockCode = key + 0x30;
        //CurrentStockCode = key;
 	    //sysVPMission.key = sysVPMission.selItem[key - 1];
		//sysVPMission.key = SystemParameter.selItem[key - 1];
		sysVPMission.key = key;
        sysVPMission.goodsType = key;
        //Trace( "\n IsInputGoodsNo key = %bu", key );		
		return 0;
	}
	else				
		return 1;	
#endif	
}

/////////////////////////////
//输入货道，并判断输入是否正确，是否存在此货道,输入正确返回0,输入失败返回1
u_char DisplayInputCode()
{
	u_char xdata MyKey = 0 ;
	u_char xdata len = 0;

	Trace("\n QueryDisplayInputCode" );
	//--------------------------------------------------
#ifdef   _SHUPING_
	memset( DisplayBuffer, 0, sizeof( DisplayBuffer ) );															
	DisplayLine = STR_INPUT_GOODSNO;	
	if( CurrentStockCode != 0 )
	{
		len = sprintf( DisplayBuffer,"%s%bu", DispInformationTable[DisplayLine].str, CurrentStockCode );		
		DisplayStr( 0, __LINE0__, 1, DisplayBuffer, len + 1 );
		if( CurrentStockCode < 10 )
		{
			ClearDisplayLine( 2 );
			DisplayCursorState( len, __LINE0__, 1, 1, 1 );					
			return 1;
		}
		else
			DisplayCursorState( 0, 1, 1, 0, 1 );
	}
	else
	{
		DisplayCursorState( 0, 1, 1, 0, 1 );
		QueryFlag = 0;
		QueryTimer = 0;
		CurrentStockCode = 0;
		UpdataDisp = 6;
		return 1;
	}

	ClearKey();
	if( ( CurrentStockCode > 99 ) || ( CurrentStockCode <= 0 ) )
	{
		DisplayCursorState( 0, 1, 1, 0, 1 );
		// 客户输入错误 		
		DisplayLine = STR_INVAILD_CHOICE;
		DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
		DisplayLine = STR_RE_INPUT_GOODS;
		DisplayStr( 0, __LINE1__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
		WaitForWork( 2000 , NULL );
		QueryFlag = 0;
		QueryTimer = 0;
		CurrentStockCode = 0;
		UpdataDisp = 6;
		return 1 ;
	}	
	//输入规则正确	
	MyKey = 0;
	for( len = 0; len < GOODSWAYNUM; len ++ )
	{
		if( CurrentStockCode == InputGoodsWayList[len].GoodsWayNo )
		{
			CurrentStockCode = len;//InputGoodsWayList[len].SetArrayNo;
			MyKey = 1;
			break;
		}
	}
	if( MyKey == 0 )
	{
		DisplayCursorState( 0, 1, 1, 0, 1 );
		// 客户输入错误 		
		DisplayLine = STR_INVAILD_CHOICE;
		DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
		DisplayLine = STR_RE_INPUT_GOODS;
		DisplayStr( 0, __LINE1__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
		WaitForWork( 2000 , NULL );
		QueryFlag = 0;
		QueryTimer = 0;
		CurrentStockCode = 0;
		UpdataDisp = 6;
		return 1;
	}
	Trace( "\n QueryDisplayInputCode CurrentStockCode = %bu", CurrentStockCode );	
#endif
	return 0;
	//==================================================
}


//判断货道是否正常
u_char IsGoodsWay()
{
	//u_char xdata str[20];
	//u_char xdata len = 0;
	uchar xdata i = 0;
	uchar xdata chuSet = 0;
	
	Trace( "\n CurrentStockCode = %bu, GoodsCurrentSum = %bu", CurrentStockCode, GoodsWaySetVal[CurrentStockCode].GoodsCurrentSum );	
	
	/*
	判断有这个商品id,该货道有存货数量,货道状态正确

	否则
	1.
	{"  商品已售完", 12 },//14	
	 {"  请选择其它商品", 16 },       //25


	//2.商品id无效
	
	//3.单价为0
	
	//4.缺货	
	*/
	#ifdef   _SHUPING_
		//按下购物按钮时，予以上报
		VPMission_Button_RPT( VP_BUT_GOODS, CurrentStockCode+1 );
	#else
	    //1.判断是否有货物出货
		for( i=0; i<COLUMN_NUM_SET; i++ )
	    {
	    	if(GoodsWaySetVal[i].GoodsType == SystemParameter.selItem[sysVPMission.key])
	    	{
	    		//len = sprintf( str, "%02bu,%02bu", sysVPMission.key,SystemParameter.selItem[sysVPMission.key] );
				//		DisplayStr( 0, 1, 1, str, len );
				//		WaitForWork( 2000, NULL );
				//CurrentStockCode = i;		
			    if( ( GoodsWaySetVal[i].GoodsCurrentSum > 0 )&&(GoodStateLed(i)==1) )
			    {		        
		        	CurrentStockCode = i;
					//len = sprintf( str, "%02bu", CurrentStockCode );
					//DisplayStr( 0, 1, 1, str, len );
					//WaitForWork( 2000, NULL );	        
					break;
			    }
	    	}
	    }
		//len = sprintf( str, "%02bu", i );
		//	DisplayStr( 0, 0, 1, str, len );		
		//len = sprintf( str, "%02bu,%02bu,%02bu", sysVPMission.key,SystemParameter.selItem[sysVPMission.key],CurrentStockCode );
		//	DisplayStr( 0, 1, 1, str, len );
		//	WaitForWork( 500, NULL );

	    //没有货物出货
	    if(i>=COLUMN_NUM_SET)
		{
			//len = sprintf( str, "%02bu", sysGoodsMatrix[i].NextColumn );
			//DisplayStr( 0, 0, 1, str, len );	
			//WaitForWork( 2000, NULL );
			
			DispGoodsState(4);
			CurrentStockCode = 0;
			sysVPMission.dspCtr |= 0x01;
	        //DisplayCurrentMoney( CurrentPayed );
			//ClearDisplayLine( 2 );
			return 2;
		}
		//len = sprintf( str, "1=%02bu", CurrentStockCode );
		//DisplayStr( 0, 0, 1, str, len );
		//WaitForWork( 2000, NULL );

		//2.判断是这次轮流到哪个货道出货
		while(chuSet==0)
		{		
			for( i=0; i<COLUMN_NUM_SET; i++ )
		    {
		    	if(GoodsWaySetVal[i].GoodsType == SystemParameter.selItem[sysVPMission.key])
		    	{
		    		//len = sprintf( str, "1=%02bu,%02bu,%02bu", i,GoodsWaySetVal[i].GoodsCurrentSum,GoodsWaySetVal[i].IsUsed );
					//		DisplayStr( 0, 1, 1, str, len );
					//		WaitForWork( 2000, NULL );
					if( ( GoodsWaySetVal[i].GoodsCurrentSum > 0 )&&(GoodStateLed(i)==1) &&( GoodsWaySetVal[i].IsUsed == 0 ) )
				    {		        
			        	CurrentStockCode = i;
						chuSet = 1;
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
			    	if(GoodsWaySetVal[i].GoodsType == SystemParameter.selItem[sysVPMission.key])
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
			    
		VPMission_Button_RPT( VP_BUT_GOODS, CurrentStockCode+1 );
	#endif
	/*
    else if( (GoodsWaySetVal[CurrentStockCode].GoodsCurrentSum == 0) )
	{			
		DisplayLine = STR_GOODS_SOLDOUT;
		DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
		DisplayLine = STR_INPUT_WAY_NO;
		DisplayStr( 0, __LINE1__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );		
		WaitForWork( 2000, NULL );
		CurrentStockCode = 0;
		sysVPMission.dspCtr |= 0x01;
		return 2;
	}
    //===============================================================================================================
	//货没有开启或货道故障或单价为0		
	else if( ( ( GoodsWaySetVal[CurrentStockCode].WayState & 0x01 ) == 0x00 ) || ( ( GoodsWaySetVal[CurrentStockCode].WayState & 0x0A ) != 0x00 )  )
	{
	    DisplayLine = STR_GOODS_SOLDOUT;
		DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
		DisplayLine = STR_INPUT_WAY_NO;
		DisplayStr( 0, __LINE1__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );		
		WaitForWork( 2000, NULL );
		CurrentStockCode = 0;
		sysVPMission.dspCtr |= 0x01;
		CurrentStockCode = 0;
		return 1;		
	}
	//The price is zero, none respond
	else if( ( GoodsWaySetVal[CurrentStockCode].Price == 0 ) )
	{
	    CurrentStockCode = 0;
		//sysVPMission.dspCtr |= 0x01;
		CurrentStockCode = 0;
		return 1;   
	}
	*/
	//货道故障:
	if( GoodsWaySetVal[CurrentStockCode].WayState & 0x0A )
	{
		DispGoodsState(1);
		CurrentStockCode = 0;
		sysVPMission.dspCtr |= 0x01;
		CurrentStockCode = 0;
		if( CurrentPayed < SystemParameter.BanknoteMax )
        {
        	EnableBillDev();		
        }
		sysVPMission.sTimer1 = SystemParameter.tradeTime;
		ClearKey();//20110622
		return 1;	
	}
	//货没有开启或商品id为ff或商品id为0	
	else if(( GoodsWaySetVal[CurrentStockCode].WayState == 0 )|| ( GoodsWaySetVal[CurrentStockCode].GoodsType == 0xff )|| ( GoodsWaySetVal[CurrentStockCode].GoodsType == 0x00 ))
	{
		DispGoodsState(2);
		CurrentStockCode = 0;
		sysVPMission.dspCtr |= 0x01;
		CurrentStockCode = 0;
		if( CurrentPayed < SystemParameter.BanknoteMax )
        {
        	EnableBillDev();		
        }
		sysVPMission.sTimer1 = SystemParameter.tradeTime;
		ClearKey();//20110622
		return 1;		
	}
	//单价为0
	else if( GoodsWaySetVal[CurrentStockCode].Price == 0 )
	{
		DispGoodsState(3);
		CurrentStockCode = 0;
		sysVPMission.dspCtr |= 0x01;
		CurrentStockCode = 0;
		if( CurrentPayed < SystemParameter.BanknoteMax )
        {
        	EnableBillDev();		
        }
		sysVPMission.sTimer1 = SystemParameter.tradeTime;
		ClearKey();//20110622
		return 1;		
	}
	//缺货
	else if( (GoodsWaySetVal[CurrentStockCode].WayState & 0x04) || (GoodsWaySetVal[CurrentStockCode].GoodsCurrentSum == 0) )
	{
		DispGoodsState(4);
		sysVPMission.dspCtr |= 0x01;
		CurrentStockCode = 0;
		if( CurrentPayed < SystemParameter.BanknoteMax )
        {
        	EnableBillDev();		
        }
		sysVPMission.sTimer1 = SystemParameter.tradeTime;
		ClearKey();//20110622
		return 2;	
	}
	GoodsWaySetVal[CurrentStockCode].IsUsed = 1;
	return 0;
}

u_char IsMoneyEnough()
{
    //Trace( "\n Price = %d", GoodsWaySetVal[CurrentStockCode].Price );		
    //Trace( "\n CurrentPayed = %d", CurrentPayed );
	u_char xdata i = 0;	
	u_int  xdata moneyBuf = 0;
	u_int  xdata CurrentDomain = 0;
	u_char xdata flag = 0;
	u_char xdata mission = 0;
	u_int  xdata tempPrice;
	u_int  xdata inBilltemp;
	u_char xdata resultdisp[2] = {0, 0};
	u_char xdata escrowFlag = 0;	

    if( sysITLMission.billHoldingFlag == 1 )
	{
		moneyBuf = CurrentPayed + sysITLMission.billHoldingValue;
	}
	else
	{
	    moneyBuf = CurrentPayed;
	}

  
	//if( GoodsWaySetVal[CurrentStockCode].Price > CurrentPayed )
	if( GoodsWaySetVal[CurrentStockCode].Price > moneyBuf )
	{
        DisplayLine = STR_VP_BALANCE_LOW;
		DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
        //
		DisplayLine = STR_PRICE;
		switch( SystemParameter.curUnit )
		{
			case 1:
				i = sprintf( DisplayBuffer, "%s %u", DispInformationTable[DisplayLine].str,	GoodsWaySetVal[CurrentStockCode].Price );
				break;				
			case 10:
				i = sprintf( DisplayBuffer, "%s %u.%u", DispInformationTable[DisplayLine].str,
					GoodsWaySetVal[CurrentStockCode].Price/SystemParameter.curUnit,GoodsWaySetVal[CurrentStockCode].Price%SystemParameter.curUnit );				
				break;
			case 100:
				i = sprintf( DisplayBuffer, "%s %u.%02u", DispInformationTable[DisplayLine].str,
					GoodsWaySetVal[CurrentStockCode].Price/SystemParameter.curUnit,GoodsWaySetVal[CurrentStockCode].Price%SystemParameter.curUnit );				
			break;		
		}
        DisplayStr( 0, __LINE1__, 1, DisplayBuffer, i );
		WaitForWork( 1500, NULL );
		CurrentStockCode = 0;
	    sysVPMission.dspCtr |= 0x01;
		return 1;	
	}
	//硬币器故障时只能扣款或购买与暂存纸币等额的商品
	if( ( sysMDBMission.coinDeviceStatus != 0 )||( sysMDBMission.tubeRemoved == 1 ) )
	{
		if(GoodsWaySetVal[CurrentStockCode].Price > 0)
		{
			if(GoodsWaySetVal[CurrentStockCode].Price != moneyBuf )
			{
		        DisplayLine = STR_COIN_NOT_ENOUGH;
				DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
		        //				
				WaitForWork( 1500, NULL );
				CurrentStockCode = 0;
			    sysVPMission.dspCtr |= 0x01;
				return 1;	
			}
		}
	}

    //是否可以找零
	if(GoodsWaySetVal[CurrentStockCode].Price > CurrentPayed)
	{
		tempPrice = GoodsWaySetVal[CurrentStockCode].Price - CurrentPayed;
		if( sysITLMission.billHoldingFlag == 1 )
		{
			tempPrice = sysITLMission.billHoldingValue - tempPrice;
			inBilltemp = tempPrice;
			tempPrice = tempPrice/10;
			mission =changeCNY(resultdisp,tempPrice);
			if(mission)
			{
				escrowFlag = 1;
				sysVPMission.inBillMoney = inBilltemp;
			}
			else
			{
				DisplayLine = STR_COIN_UNPAY;
				DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
				DisplayLine = STR_INPUT_WAY_NO;
				DisplayStr( 0, __LINE1__, 1, " ", 1 );		
				WaitForWork( 2000, NULL );
				CurrentStockCode = 0;
				sysVPMission.dspCtr |= 0x01;
				return 1;
			}
		}
	}
	else
	{		
		tempPrice = CurrentPayed - GoodsWaySetVal[CurrentStockCode].Price;
		tempPrice = tempPrice/10;
		mission =changeCNY(resultdisp,tempPrice);
		if(mission == 0)		
		{
			DisplayLine = STR_COIN_UNPAY;
			DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
			DisplayLine = STR_INPUT_WAY_NO;
			DisplayStr( 0, __LINE1__, 1, " ", 1 );		
			WaitForWork( 2000, NULL );
			CurrentStockCode = 0;
			sysVPMission.dspCtr |= 0x01;
			return 1;
		}
	}

    //Before vending, stack the holding bill
	if(( sysITLMission.billHoldingFlag == 1 )&&(escrowFlag == 1))
	{
	    
        flag = MDB_Bill_EscrowCtr( 1 );
		if(flag==0)
		{
		    sysVPMission.billSTimer = TIME_BILL_STACK;
		    while( sysVPMission.billSTimer )
			{
                mission = MDB_Bill_IfStacked( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
				if( mission == 1 )
				{
				    CurrentDomain = sysITLMission.billHoldingValue;					
					Trace( "\n Get in Cash Ok" );
					//CurrentDomain = GetCash();
					Trace( "\n CurrentDomain = %d",CurrentDomain );		
					CurrentPayed += CurrentDomain * BILLMULTIPLE;	
					//TradeCounter.TrueCurrencySum += CurrentDomain * BILLMULTIPLE;
					//TradeCounter.CashSum += CurrentDomain * BILLMULTIPLE;
					//TradeCounter.CashSumBack += CurrentDomain * BILLMULTIPLE;//区间
					SellAccumulateMoney(&TradeCounter.CashSum,CurrentDomain * BILLMULTIPLE);
					SellAccumulateMoney(&TradeCounter.CashSumBack,CurrentDomain * BILLMULTIPLE);
					//------------------------------------------------------
					sysVPMission.payInBillFlag = 1;
					sysVPMission.payInBillMoney += CurrentDomain * BILLMULTIPLE;
					//reset the trade timer
			        //sysVPMission.sTimer1 = SystemParameter.tradeTime;
					//======================================================
					//sysITLMission.restartTime = ITL_TIME_RESTART; //reset the timer!
					memset(sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));
					sysMDBMission.billBufLen=0;
					sysMDBMission.billType=0;
					sysMDBMission.billStock=0;
		            sysITLMission.billHoldingFlag = 0;
					sysITLMission.billHoldingValue = 0;
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
                    break;//压钞成功后，直接跳出时间循环等待;by gzz 20110721
				}
				else
				{
					VP_GameKeyPoll( 200 );
				}
            }
			if( sysVPMission.billSTimer == 0 )
			{
                //钞箱满，钞箱压钞后就能时，还可以认到纸币;by gzz 20111017 
			    //检测纸币器钞箱是否已满;by gzz 20111014 
				WaitForWork(2000,NULL);
                MDB_Bill_Stacker( &sysMDBMission.billBuf[0], &sysMDBMission.billBuf[1] ); 
            	if(sysMDBMission.billIsFull==1)
				{
				    CurrentDomain = sysITLMission.billHoldingValue;
					Trace( "\n Get in Cash Ok" );
					//CurrentDomain = GetCash();
					Trace( "\n CurrentDomain = %d",CurrentDomain );		
					CurrentPayed += CurrentDomain * BILLMULTIPLE;	
					//TradeCounter.TrueCurrencySum += CurrentDomain * BILLMULTIPLE;
					//TradeCounter.CashSum += CurrentDomain * BILLMULTIPLE;
					//TradeCounter.CashSumBack += CurrentDomain * BILLMULTIPLE;//区间
					SellAccumulateMoney(&TradeCounter.CashSum,CurrentDomain * BILLMULTIPLE);
					SellAccumulateMoney(&TradeCounter.CashSumBack,CurrentDomain * BILLMULTIPLE);
					//------------------------------------------------------
					sysVPMission.payInBillFlag = 1;
					sysVPMission.payInBillMoney += CurrentDomain * BILLMULTIPLE;
					//reset the trade timer
			        //sysVPMission.sTimer1 = SystemParameter.tradeTime;
					//======================================================
					//sysITLMission.restartTime = ITL_TIME_RESTART; //reset the timer!
					memset(sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));
					sysMDBMission.billBufLen=0;
					sysMDBMission.billType=0;
					sysMDBMission.billStock=0;
		            sysITLMission.billHoldingFlag = 0;
					sysITLMission.billHoldingValue = 0;
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
				}
				else
				{ 
					//...	
					//调整顺序，billHoldingFlag==0放在sysVPMission之上;by gzz 20110721
					sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
					sysITLMission.billHoldingFlag = 0;
					sysITLMission.billHoldingValue = 0;
					//Need better solutoin. 
					//Maybe cheated by man
		            //CurrentPayed += sysITLMission.billHoldingValue;
					//-----------------------------------------------------
				    sysVPMission.escrowOutFlag = 1;				    
					if( sysVPMission.escrowOutFlag == 1 )
				    {
					    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
						sysVPMission.escrowOutFlag = 0;
				        sysVPMission.escrowMoney = 0;     
					}
				    //=====================================================
					DisplayCurrentMoney( CurrentPayed );
					if( GoodsWaySetVal[CurrentStockCode].Price > CurrentPayed )
					{
						return 1;
					}
                }	
			}
        }
        else
		{
			//...
			//调整顺序，billHoldingFlag==0放在sysVPMission之上;by gzz 20110721
			sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
			sysITLMission.billHoldingFlag = 0;
			sysITLMission.billHoldingValue = 0;
			//Need better solutoin. 
			//Maybe cheated by man
            //CurrentPayed += sysITLMission.billHoldingValue;
			//-----------------------------------------------------
			sysVPMission.escrowOutFlag = 1;			
			if( sysVPMission.escrowOutFlag == 1 )
		    {
			    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
				sysVPMission.escrowOutFlag = 0;
		        sysVPMission.escrowMoney = 0;     
			}
			//=====================================================
			DisplayCurrentMoney( CurrentPayed );
			if( GoodsWaySetVal[CurrentStockCode].Price > CurrentPayed )
			{
				return 1;
			}
		}
    }
    else
	{
		//若此时还有插币，等压钞结束后再判断;by gzz 20110429 
		//WaitForWork( NULL, 1000 );
        /*
	    if((SystemParameter.BillNo==1)&&(sysITLMission.ITLSet==1))
	    {	   
	       if(sysITLMission.billStaCtr & ITL_BILL_READ)
	       {	
	                
	          return 1;          
	       }
	    }
        */ 
	}
	//
	{
        sysITLMission.billStaCtr = 0;
        sysITLMission.billSta = 0;
		/*
        WaitForWork( 300, NULL );
		MDB_Bill_EscrowCtr( 0 );
		*/
    	//sysITLMission.billStaCtr |= ITL_BILL_VEDNING;
	}
	return 0;
}

//显示货道故障并让客户确定
u_char GoodsWayErr()
{
	//u_char xdata MyKey ;
	//------------------------------------------------------------
	DisplayLine = STR_GOODS_WAY_ERR2;
	#ifdef _CHINA_
		DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len - 3 );
	#else
		DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len - 1 );
	#endif
	WaitForWork( 2000, NULL );
	ClearKey();	
	CurrentStockCode = 0;
	return 0;
	//============================================================

}

//判断是否支付过款，没有返回1,有返回0
u_char	CheckIsPayment()
{
	if( CurrentPayed ==  0 )
		return 1;
	else
		return 0;
}

//出货
//0 - trade mode
//1 - free vend
//2 - card
unsigned char OutGoods( unsigned char type )
{
	u_char   xdata flag = 0 ;
	u_char 	 xdata length = 0;
	u_char   xdata result = 0;
	u_int    xdata moneyBuf = 0;
	//u_char xdata str[20];
	//u_char xdata len = 0;
    
	//Trace( "\n OutGoods" );
	//1.Disable the device...
	//ITLMission_Reject();
	//WaitForWork(100,NULL);
	DisableBillDev();
	//WaitForWork(100,NULL);

	if( sysITLMission.billHoldingFlag == 1 )
	{
		moneyBuf = CurrentPayed + sysITLMission.billHoldingValue;
	}
	else
	{
	    moneyBuf = CurrentPayed;
	}
    //if( (type==0)||(type==2) )   //type==1
    {
    	sysVPMission.boughtFlag = 1;    	
    }

    if( (type==0) )
    {	
		DisplayLine = STR_TRANCE_ING;
		DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );		
		memset( DisplayBuffer, 0, sizeof( DisplayBuffer ) );	
		switch( SystemParameter.curUnit )
		{
			case 1:	
				length = sprintf( DisplayBuffer, "%s%u", DispInformationTable[STR_BALANCE].str,moneyBuf );	
			break;				
			case 10:
				length = sprintf( DisplayBuffer, "%s%u.%u", DispInformationTable[STR_BALANCE].str,
					moneyBuf / SystemParameter.curUnit, moneyBuf % SystemParameter.curUnit );	
			break;
			case 100:
				length = sprintf( DisplayBuffer, "%s%u.%02u", DispInformationTable[STR_BALANCE].str,
					moneyBuf / SystemParameter.curUnit, moneyBuf % SystemParameter.curUnit );	
			break;		
		}	
		DisplayStr( 0, __LINE1__, 1, DisplayBuffer, length );
		WaitForWork( 100 , NULL );	 //500,100
		Trace("\n InputGoodsWayList[CurrentStockCode].SetArrayNo = %bu", InputGoodsWayList[CurrentStockCode].SetArrayNo );
        sysVPMission.vendColumn = CurrentStockCode;
        sysVPMission.vendType = 0;
        sysVPMission.vendCost = GoodsWaySetVal[ CurrentStockCode].Price;
	}
	else if( type==2 )
	{
	    DisplayLine = STR_TRANCE_ING;
		DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );		
		memset( DisplayBuffer, 0, sizeof( DisplayBuffer ) );	
		switch( SystemParameter.curUnit )
		{
			case 1:	
				length = sprintf( DisplayBuffer, "%s%lu", DispInformationTable[STR_VP_CARD_BALANCE].str,sysBCMission.balance );	
			break;				
			case 10:
				length = sprintf( DisplayBuffer, "%s%lu.%lu", DispInformationTable[STR_VP_CARD_BALANCE].str,
					sysBCMission.balance/SystemParameter.curUnit, sysBCMission.balance%SystemParameter.curUnit );	
			break;
			case 100:
				length = sprintf( DisplayBuffer, "%s%lu.%02lu", DispInformationTable[STR_VP_CARD_BALANCE].str,
					sysBCMission.balance/SystemParameter.curUnit, sysBCMission.balance%SystemParameter.curUnit );	
			break;		
		}	
		DisplayStr( 0, __LINE1__, 1, DisplayBuffer, length );
		WaitForWork( 100 , NULL );	 //500,100
		Trace("\n InputGoodsWayList[CurrentStockCode].SetArrayNo = %bu", InputGoodsWayList[CurrentStockCode].SetArrayNo );
        sysVPMission.vendColumn = CurrentStockCode;
        sysVPMission.vendType = 0;
        sysVPMission.vendCost = GoodsWaySetVal[ CurrentStockCode].Price;
	}
    else if( type==1 )
    {
    	DisplayLine = STR_TRANCE_ING;
		DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
        /*		
		memset( DisplayBuffer, 0, sizeof( DisplayBuffer ) );	
		switch( SystemParameter.curUnit )
		{
			case 1:	
				length = sprintf( DisplayBuffer, "%s%u", DispInformationTable[STR_BALANCE].str,moneyBuf );	
			break;				
			case 10:
				length = sprintf( DisplayBuffer, "%s%u.%u", DispInformationTable[STR_BALANCE].str,
					moneyBuf / SystemParameter.curUnit, moneyBuf % SystemParameter.curUnit );	
			break;
			case 100:
				length = sprintf( DisplayBuffer, "%s%u.%02u", DispInformationTable[STR_BALANCE].str,
					moneyBuf / SystemParameter.curUnit, moneyBuf % SystemParameter.curUnit );	
			break;		
		}	
		DisplayStr( 0, __LINE1__, 1, DisplayBuffer, length );
        */
        ClearDisplayLine( 2 );
		WaitForWork( 100 , NULL );	 //500, 100
        CurrentStockCode = sysVPMission.vendColumn;
		Trace("\n InputGoodsWayList[CurrentStockCode].SetArrayNo = %bu", InputGoodsWayList[CurrentStockCode].SetArrayNo );
        //sysVPMission.vendCost = GoodsWaySetVal[ CurrentStockCode].Price;
     
    }
    //---------------------------------------------------------------------------------
    sysVPMission.vendSta = 0;
    
	//Added for testing
    //ITLMission_Reject();

	#ifdef   _SJJ_//升降机
		flag = ChannelLifterTask( InputGoodsWayList[CurrentStockCode].GoodsWayNo, CHANNEL_EXEC );		
	#else         //普通货道
    	flag = ChannelTask( InputGoodsWayList[CurrentStockCode].SetArrayNo, CHANNEL_EXEC );
		if( flag == 7 )
		{
			//接收到的命令包有错误，但货没有出
			flag = 0;
			flag = ChannelTask( InputGoodsWayList[CurrentStockCode].SetArrayNo, CHANNEL_EXEC );
		}		
	#endif
    
	Trace( "\n OutGoods flag = %bu", flag );
	VPMission_Act_RPT(VP_ACT_CHUHUO,0);
//MAIN_TAB_VEND_NEXT:
	switch( flag )
	{
		case 0:   //出货成功	
		case 5:   //在限定的时间内电机不能到位（但货已出成功，货道有硬件故障）
		{	
	
			Trace( "\n OutGoods 11" );
			if( (type == 0) || (type==2)  )
			{
				CurrentPayed = CurrentPayed - GoodsWaySetVal[CurrentStockCode].Price;
			}

            if( ( type == 1 ) )
            {
            	if( CurrentPayed >= sysVPMission.vendCost )
                {
                    CurrentPayed -= sysVPMission.vendCost;
                }
            }
			DisplayLine = STR_TAKE_GOODS;
			DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );
            
            //-----------------------------------------------------------------------------------------------------------
            if( type == 0 )
            {
				if( ( IsCanChange() == 0 ) && ( ( SystemParameter.IsMuliVerd != 1 ) || \
				( ( SystemParameter.IsMuliVerd == 1 ) && ( CurrentPayed < SystemParameter.Min ) ) ) )
	 		    	ClearDisplayLine( 2 );
				else
				{
					memset( DisplayBuffer, 0, sizeof(DisplayBuffer ));
					switch( SystemParameter.curUnit )
					{
						case 1:	
							length = sprintf( DisplayBuffer, "%s%u", DispInformationTable[STR_BALANCE].str,moneyBuf );	
						break;				
						case 10:
							length = sprintf( DisplayBuffer, "%s%u.%u", DispInformationTable[STR_BALANCE].str,
								moneyBuf / SystemParameter.curUnit, moneyBuf % SystemParameter.curUnit );	
						break;
						case 100:
							length = sprintf( DisplayBuffer, "%s%u.%02u", DispInformationTable[STR_BALANCE].str,
								moneyBuf / SystemParameter.curUnit, moneyBuf % SystemParameter.curUnit );	
						break;		
					}
					DisplayStr( 0, __LINE1__, 1, DisplayBuffer, length );
					WaitForWork( 200 , NULL );	    //1500-200	
				}
            }
			GoodsWaySetVal[CurrentStockCode].WayState = 0x01;
			//if( SystemParameter.GOCOn != 0x01 )
			if(GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum > 0)
			{
				GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum --;
				if( GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum == 0 )
					GoodsWaySetVal[CurrentStockCode].WayState = 0x05;
				//length = sprintf( DisplayBuffer, "%u", GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum);
				//DisplayStr( 0, __LINE1__, 1, DisplayBuffer, length );
				WaitForWork( 2000 , NULL );	    //1500-200	
				
            }

			if( flag == 5 )
			{
				//GoodsWaySetVal[CurrentStockCode].WayState |= 0x02;
				//VPMission_ColumnSta_RPT();
			}
			SaveGoodsSet();	
			//TradeCounter.TradeCurrencySum += GoodsWaySetVal[ CurrentStockCode].Price;   //正常交易的总金额
			//TradeCounter.GoodTraceSum ++ ;             //正常交易次数
			//TradeLog[ CurrentStockCode].TraceSum += 1; //此货道正常交易次数
			SaveTradeCounter();
			if( type == 1 )
			{
            	UpdateGoodsMatrixStatus( sysVPMission.goodsType2 );
			}
			else
			{
			    UpdateGoodsMatrixStatus( sysVPMission.goodsType );
			}
			//
            sysVPMission.vendSta = 0;
			if(sysVPMission.offLineFlag == 1)
			{
				TradeCounter.offLineNum++;
				TradeCounter.offLineMoney += sysVPMission.vendCost; 
				//len = sprintf( str, "1=%02bu,2=%02bu", TradeCounter.offLineNum,TradeCounter.offLineMoney);
				//DisplayStr( 0, 0, 1, str, strlen(str) ); 
				//WaitForWork(2000,NULL);				
			}
			//len = sprintf( str, "2=%02bu,%02bu", CurrentStockCode, sysVPMission.vendColumn );
	        //DisplayStr( 0, 0, 1, str, len );
		    //WaitForWork( 2000, NULL );
            VPMission_Vendout_RPT( sysVPMission.vendSta, sysVPMission.vendColumn, sysVPMission.vendType, sysVPMission.vendCost,GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum );
			SaveTradeParam(); 
			//--------------------------------------------------------
			//if( (type == 0) || (type==2)  )
			{
				sysVPMission.msGameTimer1 = 100; //200-100
				while( sysVPMission.msGameTimer1 )
				{
					//----------------------------------------------------
			        //Send the game key and poll pc in speciall time
		    		if( IfGameKeyOn() )
					{
						VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
		        		//Beep();
					}
					//if( ( sysVPMission.vendCmd == 0 )&&(sysVPMission.changeCmd == 0)&&(sysVPMission.costCmd == 0)&&(sysVPMission.returnCmd == 0) )
			        	result = VPMission_Poll();
			        //===============================
				}
			}
			//=========================================================
			return 0;
		}
		break;
		//命令失败和超时都是有疑问的交易
		case 2:   //超时,货道板和主机通讯有问题,可认为整个出货统有问题		
		case 1:   //命令失败，但失败原因不明，只能当疑问处理了
		{
			Trace( "\n OutGoods 12" );
			// 出货超时
			DisplayLine = STR_GOODS_WAY_ERR1;
			DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );			
			//GoodsWaySetVal[CurrentStockCode].WayState |= 0x02;
			if(GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum > 0)
			{
				GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum --;
				if( GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum == 0 )
					GoodsWaySetVal[CurrentStockCode].WayState = 0x05;
				//length = sprintf( DisplayBuffer, "%u", GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum);
				//DisplayStr( 0, __LINE1__, 1, DisplayBuffer, length );
				WaitForWork( 2000 , NULL );	    //1500-200	
				
            }
			VPMission_ColumnSta_RPT();
	
			if( (type == 0) || (type==2)  )
			{
				if( IsCanChange() == 1 )
				{
					memset( DisplayBuffer, 0, sizeof( DisplayBuffer ) );				
					switch( SystemParameter.curUnit )
					{
						case 1:	
							length = sprintf( DisplayBuffer, "%s%u", DispInformationTable[STR_BALANCE].str,CurrentPayed );	
						break;				
						case 10:
							length = sprintf( DisplayBuffer, "%s%u.%u", DispInformationTable[STR_BALANCE].str,
								CurrentPayed / SystemParameter.curUnit, CurrentPayed % SystemParameter.curUnit );	
						break;
						case 100:
							length = sprintf( DisplayBuffer, "%s%u.%02u", DispInformationTable[STR_BALANCE].str,
								CurrentPayed / SystemParameter.curUnit, CurrentPayed % SystemParameter.curUnit );	
						break;		
					}				
					DisplayStr( 0, __LINE1__, 1, DisplayBuffer, length );
					WaitForWork( 1000 , NULL );
				}
				else
				{
					ClearDisplayLine( 2 );
					WaitForWork( 1000 , NULL );	
				}
				CurrentDispenseCoin = CurrentPayed;	
				CurrentPayed = 0;
				//DisplayStr( 0, __LINE0__ , 1 , "2", 1 );
				//WaitForWork( 2000, NULL );  //500-50-100-200	
				DispenseCoin(0);
	        }
            //=====================================================================
			//GoodsWaySetVal[CurrentStockCode].WayState |= 0x02;
            SaveGoodsSet();                                 //Added by Andy on 2010.9.2
			//---------------------------------------------------------------------							
			//TradeCounter.GoodDoubtTraceSum ++ ;             //有疑问出货交易次数计数		
			//TradeLog[ CurrentStockCode].DoubtTraceSum ++;   //此货道有疑问交易次数
			SaveTradeCounter();
			SaveTradeParam();
			if( type == 1 )
			{
            	UpdateGoodsMatrixStatus( sysVPMission.goodsType2 );
			}
			else
			{
			    UpdateGoodsMatrixStatus( sysVPMission.goodsType );
			}
			//
			sysVPMission.vendSta = 2;
			VPMission_Vendout_RPT( sysVPMission.vendSta, sysVPMission.vendColumn, sysVPMission.vendType, sysVPMission.vendCost,GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum );
			return 2;						
		}
		break;
		//出货失败，如果不能找零，则看其它货道是否能服务，如能则让顾额选其它货
		case 3:   //在转动前电机没有到位，没有出货，此货道已不能工作了
		case 4:   //电机没有转动，没有出货，可以重试一次
		case 7:   //接收到的命令包有错误，但货没有出
		{	
			Trace( "\n OutGoods 13" );
			DisplayLine = STR_GOODS_WAY_ERR1;
			DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );			
			//=====================================================================
			GoodsWaySetVal[CurrentStockCode].WayState = 0x03;
			GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum = 0;
			//if(GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum > 0)
			//{
				//GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum = 0;
				//GoodsWaySetVal[CurrentStockCode].WayState = 0x05;
				//length = sprintf( DisplayBuffer, "%u", GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum);
				//DisplayStr( 0, __LINE1__, 1, DisplayBuffer, length );
				WaitForWork( 2000 , NULL );	    //1500-200	
				
            //}
            SaveGoodsSet();                  //Added by Andy on 2010.9.2
            VPMission_ColumnSta_RPT();
			if( type == 1 )
			{
            	UpdateGoodsMatrixStatus( sysVPMission.goodsType2 );
			}
			else
			{
			    UpdateGoodsMatrixStatus( sysVPMission.goodsType );
			}
			//---------------------------------------------------------------------
			//After wrong dispense, payout automaticly.
			{
			    {
					//ClearDisplayLine( 2 );
					//WaitForWork( 2500 , NULL );
				}
			    sysVPMission.vendSta = 2;
				VPMission_Vendout_RPT( sysVPMission.vendSta, sysVPMission.vendColumn, sysVPMission.vendType, sysVPMission.vendCost,GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum );
                WaitForWork( 2500 , NULL );
				return 0;
			}
			//=====================================================================
			//---------------------------------------------------------------------			
			/*if( sysGoodsMatrix[goodsCodeBack].NextColumn == 0xff ) //In the goods matrix, if none good column left, ask for another one!
			{
				{
					//ClearDisplayLine( 2 );
					//WaitForWork( 2500 , NULL );
				}
                sysVPMission.vendSta = 2;
				VPMission_Vendout_RPT( sysVPMission.vendSta, sysVPMission.vendColumn, sysVPMission.vendType, sysVPMission.vendCost,GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum );
				return 0;
			}
			else  //Automaticly vend the next good column's goods.
			{
			    //
				CurrentStockCode = sysGoodsMatrix[goodsCodeBack].NextColumn;
				sysVPMission.vendColumn = CurrentStockCode;
                sysVPMission.vendSta = 0;
                sysVPMission.vendCost = GoodsWaySetVal[ CurrentStockCode].Price;
				//
                flag = 0;
		        flag = ChannelTask( InputGoodsWayList[CurrentStockCode].SetArrayNo, CHANNEL_EXEC );
	            Trace( "\n OutGoods flag = %bu", flag );
				goto MAIN_TAB_VEND_NEXT;
			}*/
		}
		break;
		case 8:   //The goods cann't pass the check position
		{	
			Trace( "\n OutGoods 13" );
			DisplayLine = STR_GOODS_WAY_ERR1;
			DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );			
			//=====================================================================
            //#ifdef SYS_GOOS_SOLDOUT_CTR
			//	GoodsWaySetVal[CurrentStockCode].WayState |= 0x04;   //Need?
            //#endif
            if(GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum > 0)
			{
				GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum --;
				if( GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum == 0 )
					GoodsWaySetVal[CurrentStockCode].WayState = 0x05;
				//length = sprintf( DisplayBuffer, "%u", GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum);
				//DisplayStr( 0, __LINE1__, 1, DisplayBuffer, length );
				WaitForWork( 2000 , NULL );	    //1500-200	
				
            }
            SaveGoodsSet();                  
		    if( type == 1 )
			{
            	UpdateGoodsMatrixStatus( sysVPMission.goodsType2 );
			}
			else
			{
			    UpdateGoodsMatrixStatus( sysVPMission.goodsType );
			}
			//---------------------------------------------------------------------
			//After wrong dispense, payout automaticly.
			{
			    {
					//ClearDisplayLine( 2 );
					//WaitForWork( 2500 , NULL );
				}
			    sysVPMission.vendSta = 2;
				VPMission_Vendout_RPT( sysVPMission.vendSta, sysVPMission.vendColumn, sysVPMission.vendType, sysVPMission.vendCost,GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum );
				WaitForWork( 1000 , NULL );   //2500, 1000
				return 0;
			}
			//=====================================================================
			/*
			//---------------------------------------------------------------------
			if( sysGoodsMatrix[goodsCodeBack].NextColumn == 0xff ) //In the goods matrix, if none good column left, ask for another one!
			{
				{
					//ClearDisplayLine( 2 );
					//WaitForWork( 2500 , NULL );
				}
				sysVPMission.vendSta = 2;
				VPMission_Vendout_RPT( sysVPMission.vendSta, sysVPMission.vendColumn, sysVPMission.vendType, sysVPMission.vendCost,GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum );
				return 0;
			}
			else  //Automaticly vend the next good column's goods.
			{
			    //
				CurrentStockCode = sysGoodsMatrix[goodsCodeBack].NextColumn;
				sysVPMission.vendColumn = CurrentStockCode;
                sysVPMission.vendSta = 0;
                sysVPMission.vendCost = GoodsWaySetVal[ CurrentStockCode].Price;
				//
                flag = 0;
		        flag = ChannelTask( InputGoodsWayList[CurrentStockCode].SetArrayNo, CHANNEL_EXEC );
	            Trace( "\n OutGoods flag = %bu", flag );
				goto MAIN_TAB_VEND_NEXT;
			}
			*/
			//return 0;
		}
		break;
		default: break;
	}
	VPMission_Vendout_RPT( sysVPMission.vendSta, sysVPMission.vendColumn, sysVPMission.vendType, sysVPMission.vendCost,GoodsWaySetVal[ CurrentStockCode].GoodsCurrentSum );
	return 0;
}

//确定是否断续购买商品,返回0,不继续购买，进行找零；返回1则继续购买
u_char ReBuy()
{
	u_char xdata i = 0;	
	u_char xdata j = 0;
	u_char xdata result = 0;
	u_int  xdata moneyBuf = 0;

	CurrentStockCode = 0;
	//----------------------------------------------------
	//Send the game key and poll pc in speciall time
    if( IfGameKeyOn() )
	{
		VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
        //Beep();
	}
	//if( ( sysVPMission.vendCmd == 0 )&&(sysVPMission.changeCmd == 0)&&(sysVPMission.costCmd == 0)&&(sysVPMission.returnCmd == 0) )
		result = VPMission_Poll();
	//====================================================

	if( sysITLMission.billHoldingFlag == 1 )
	{
		moneyBuf = CurrentPayed + sysITLMission.billHoldingValue;
	}
	else
	{
	    moneyBuf = CurrentPayed;
	}
	sysVPMission.inBillMoney = CurrentPayed;
	if( moneyBuf == 0 ) 
		return 0;
	//判断是否允许再次购买
	if( SystemParameter.IsMuliVerd != 1 ) 
	{
		i = 0;
		if( IsCanChange() == 1 ) 
		{
            
			ClearDisplayLine( 1 );
            /*
			memset( DisplayBuffer,0,sizeof(DisplayBuffer) );
			switch( SystemParameter.curUnit )
			{
				case 1:
					i = sprintf( DisplayBuffer, "%s%u", DispInformationTable[STR_CHANGE].str, CurrentPayed );
				break;				
				case 10:
					i = sprintf( DisplayBuffer, "%s%u.%u", DispInformationTable[STR_CHANGE].str,
						CurrentPayed/SystemParameter.curUnit,CurrentPayed%SystemParameter.curUnit );   //"%s%u.%02u--"%s%u.%u"				
				break;
				case 100:
					i = sprintf( DisplayBuffer, "%s%u.%02u", DispInformationTable[STR_CHANGE].str,
						CurrentPayed/SystemParameter.curUnit,CurrentPayed%SystemParameter.curUnit );				
				break;		
			}					
			DisplayStr( 0, __LINE1__, 1, DisplayBuffer, i );
			WaitForWork( 200 , NULL );          //2000-200-
            */
		}
		return 0;
	}	
	//先判断余钱是否超过再次购物的标准,如超过则让用户确定是否断续购物
    //取消SystemParameter.Min != 0判断;by gzz 20110624
	//if( ( CurrentPayed >= SystemParameter.Min ) && ( SystemParameter.Min != 0  ))	
    if( moneyBuf > SystemParameter.Min ) 
	{		
		if( ColnumIsCanServer() == 1 )
		{
		    //--------------------------------------------------------- 
			//Added for EV736-UB by Andy 2011.6.3
		    DisplayCurrentMoney(CurrentPayed);
			//UpdateGoodsMatrixStatus( sysVPMission.goodsType );
			UpdateSelLed_Trade();
            if( moneyBuf < SystemParameter.BanknoteMax )
            {
            	EnableBillDev();		
            }
			sysVPMission.sTimer1 = SystemParameter.tradeTime;
			SaveTradeParam();  //2011.6.10 added: Update the sale data in flash!
			ClearKey();//20110622
			return 1;
            //=========================================================

			//还有好的货道可以服务,则询问顾客是否还买货			
			//显示字符
		#ifdef _CHINA_
			DisplayStr( 0, 0, 1, DispInformationTable[STR_RE_BUY].str, DispInformationTable[STR_RE_BUY ].len );
		#else
			DisplayStr( 0, 0, 1, DispInformationTable[STR_RE_BUY].str, DispInformationTable[STR_RE_BUY ].len );
		#endif
		#ifdef _CHINA_
			DisplayStr( 0, 1, 1, DispInformationTable[STR_OK_NOT].str, DispInformationTable[STR_OK_NOT ].len );
		#else
			DisplayStr( 0, 1, 1, DispInformationTable[STR_OK_NOT].str, DispInformationTable[STR_OK_NOT ].len );
		#endif			    
			//等待选择
			SgwSecTimer= 30;
			while( SgwSecTimer )
			{
				WaitForWork( 50, NULL ); 
				i = GetKey();		
				switch ( i )
			 	{
					case KEY_SUBMIT:	
						DisplayCurrentMoney(CurrentPayed);
						return 1 ;	//确定再继续		
					case KEY_CANCEL:	
						i = 0;
						if( IsCanChange() == 1 )
						{
							ClearDisplayLine( 1 );
							memset( DisplayBuffer,0,sizeof(DisplayBuffer) );
							switch( SystemParameter.curUnit )
							{
								case 1:
									i = sprintf( DisplayBuffer, "%s%u", DispInformationTable[STR_CHANGE].str, CurrentPayed );
								break;				
								case 10:
									i = sprintf( DisplayBuffer, "%s%u.%u", DispInformationTable[STR_CHANGE].str,
										CurrentPayed/SystemParameter.curUnit,CurrentPayed%SystemParameter.curUnit );	//			
								break;
								case 100:
									i = sprintf( DisplayBuffer, "%s%u.%02u", DispInformationTable[STR_CHANGE].str,
										CurrentPayed/SystemParameter.curUnit,CurrentPayed%SystemParameter.curUnit );				
								break;		
							}													
							DisplayStr( 0, __LINE1__, 1, DisplayBuffer, i );
							WaitForWork( 1500 , NULL );
						}
						return 0 ;//确定不不买了
					default:
						continue;
				}	
			}
			//等待超时，直接找零了
			if( IsCanChange() == 1 )
			{
				i = 0;
				ClearDisplayLine( 1 );				
				memset( DisplayBuffer,0,sizeof(DisplayBuffer) );
				switch( SystemParameter.curUnit )
				{
					case 1:
						i = sprintf( DisplayBuffer, "%s%u", DispInformationTable[STR_CHANGE].str, CurrentPayed );
					break;				
					case 10:
						i = sprintf( DisplayBuffer, "%s%u.%u", DispInformationTable[STR_CHANGE].str,
							CurrentPayed/SystemParameter.curUnit,CurrentPayed%SystemParameter.curUnit );				
					break;
					case 100:
						i = sprintf( DisplayBuffer, "%s%u.%02u", DispInformationTable[STR_CHANGE].str,
							CurrentPayed/SystemParameter.curUnit,CurrentPayed%SystemParameter.curUnit );				
					break;		
				}					
			
				DisplayStr( 0, __LINE1__, 1, DisplayBuffer, i );
				WaitForWork( 1500 , NULL );
			}
			return 0;
		}		
		else
		{
            UpdateSelLed_Trade();//更新指示灯;by gzz 20110617
			//已无货道可以服务了，直接找零
			if( IsCanChange() == 1 )
			{
				i = 0;
				ClearDisplayLine( 1 );
				///////////////
				memset( DisplayBuffer,0,sizeof(DisplayBuffer) );
				switch( SystemParameter.curUnit )
				{
					case 1:
						i = sprintf( DisplayBuffer, "%s%u", DispInformationTable[STR_CHANGE].str, CurrentPayed );
					break;				
					case 10:
						i = sprintf( DisplayBuffer, "%s%u.%u", DispInformationTable[STR_CHANGE].str,
							CurrentPayed/SystemParameter.curUnit,CurrentPayed%SystemParameter.curUnit );				
					break;
					case 100:
						i = sprintf( DisplayBuffer, "%s%u.%02u", DispInformationTable[STR_CHANGE].str,
							CurrentPayed/SystemParameter.curUnit,CurrentPayed%SystemParameter.curUnit );				
					break;		
				}			
				DisplayStr( 0, __LINE1__, 1, DisplayBuffer, i );
				WaitForWork( 1500 , NULL );
			}
			return 0;
		}		
	}
	else  //直接找零
	{
		if( IsCanChange() == 1 )
		{
			i = 0;
			ClearDisplayLine( 1 );			
			memset( DisplayBuffer,0,sizeof(DisplayBuffer) );
			switch( SystemParameter.curUnit )
			{
				case 1:
					i = sprintf( DisplayBuffer, "%s%u", DispInformationTable[STR_CHANGE].str, CurrentPayed );
				break;				
				case 10:
					i = sprintf( DisplayBuffer, "%s%u.%u", DispInformationTable[STR_CHANGE].str,
						CurrentPayed/SystemParameter.curUnit,CurrentPayed%SystemParameter.curUnit );				
				break;
				case 100:
					i = sprintf( DisplayBuffer, "%s%u.%02u", DispInformationTable[STR_CHANGE].str,
						CurrentPayed/SystemParameter.curUnit,CurrentPayed%SystemParameter.curUnit );				
				break;		
			}											
			DisplayStr( 0, __LINE1__, 1, DisplayBuffer, i );
			WaitForWork( 2000 , NULL );
		}
		return 0;
	}
}

u_char TestHardWare()
{
    //bit data z = 0;	
	if( HardWareErr )
 	{	
		CheckHardWare( 3 ); //查检硬件故障
	}
	return 0 ;
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
u_char MakeChange(int length, unsigned char* coins,
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

//找零美分
void changeUSD()
{
    //*可找零硬币面额（美分）（面额从大到小）
    unsigned char xdata coins[5] = {100, 50, 25, 10, 5};
    //*每种面额硬币可找零个数（硬币只支持255个，超过255也只填255*
    unsigned char xdata lefts[5] = {0, 3, 5, 1, 0};
    //*找零结果（确保传入MakeChange前，results被清零）*
    unsigned char xdata results[5] = {0, 0, 0, 0, 0};
    if(MakeChange(5, coins, lefts, results, 335))
    {
        //*找零335美分 *
        //printf("USA Success\n");
    }
    else
    {
        //printf("USA Fail\n");
    }
}


//找零人民币/*找零money元,即3.5元=35(money)*/
u_char changeCNY(unsigned char* results, u_int money)
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
    //len = sprintf( str, "2.1:%u.%02u,%u,%u", money/10,money%10,lefts[0],lefts[1] );
	//DisplayStr( 0, 0, 1, str, len );
	//WaitForWork( 5000, NULL );
	
    if(MakeChange(2, coins, lefts, results, money))
    {
        
        //printf("CNY Success\n");
        //DisplayStr( 0, 0, 1, "2CNY Success",12 );
		//WaitForWork( 2000, NULL );
        return 1;
    }
    else
    {
        //printf("CNY Fail\n");
        //DisplayStr( 0, 0, 1, "2CNY Fail",9 );
		//WaitForWork( 2000, NULL );
        return 0;
    }

}


u_char DispenseCoin(unsigned char flag )
{
	u_char xdata tHopper1 = 0;
	u_char xdata tHopper2 = 0;
	u_char xdata tHopper3 = 0;
	u_char xdata ret = 0 ;
	uint   xdata outMoney = 0;
	u_char xdata i = 0;
	u_char xdata coin1flag = 1;
	u_char xdata coin2flag = 1;
	u_char xdata coin3flag = 1;
	u_char xdata m_Errorflag = 0;
	u_char xdata result = 0;
	u_char xdata mission;
	//u_char xdata tempMoney;
    u_int xdata dispenseValue = 0;
	u_char xdata dispensedNum = 0;
    u_char xdata trymis = 0;
	u_char xdata resultdisp[2] = {0, 0};
	u_char xdata tempdisp = 0;
	u_int  xdata  temppayOutMoney1=0;
	u_int  xdata  temppayOutMoney2=0;
	//u_char xdata str[20];
	//u_char xdata len = 0;
	
	Trace( "\n DispenseCoin" );
	Trace( "\n CurrentDispenseCoin = %d", CurrentDispenseCoin );	
	WaitForWork( 500 , NULL );  
	VPMission_Act_RPT(VP_ACT_PAYOUT,0);
	WaitForWork( 100 , NULL );  
    
	/*
	if( ( IsCanChange() != 1 ) && ( ColnumIsCanServer() == 1 ) )
	{
		return 0;
	}
    */
    
    MDBMission_Coin_Disable();
	MDBMission_Bill_Disable();
    //1.Judge the payout value
	//if( CurrentDispenseCoin > (SystemParameter.BanknoteMax + 200) )
	//{
	//	CurrentDispenseCoin = SystemParameter.BanknoteMax + 200;	
	//}
    //2.
    DisplayLine = STR_TAKE_COIN;
	DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[ DisplayLine ].str, DispInformationTable[ DisplayLine ].len );
	WaitForWork( 50 , NULL ); 
	memset( DisplayBuffer,0,sizeof(DisplayBuffer) );
	switch( SystemParameter.curUnit )
	{
		case 1:
			i = sprintf( DisplayBuffer, "%s%u", DispInformationTable[STR_CHANGE].str, CurrentDispenseCoin );
		break;				
		case 10:
			i = sprintf( DisplayBuffer, "%s%u.%u", DispInformationTable[STR_CHANGE].str,
				CurrentDispenseCoin/SystemParameter.curUnit,CurrentDispenseCoin%SystemParameter.curUnit );   //"%s%u.%02u--"%s%u.%u"				
		break;
		case 100:
			i = sprintf( DisplayBuffer, "%s%u.%02u", DispInformationTable[STR_CHANGE].str,
				CurrentDispenseCoin/SystemParameter.curUnit,CurrentDispenseCoin%SystemParameter.curUnit );				
		break;		
	}					
	DisplayStr( 0, __LINE1__, 1, DisplayBuffer, i );
	WaitForWork( 50 , NULL );          


    sysVPMission.changeFlag = 1;
	//--------------------------------------------------------------
	sysVPMission.payOutMoney1 = 0;
    sysVPMission.payOutMoney2 = 0;
    //==============================================================


	/*
	//用于Hopper找零
	tHopper2= CurrentDispenseCoin/100;
	tHopper1= CurrentDispenseCoin%100/10/5;
    mission=MDB_Coin_Dispense(0,tHopper1);
    DelayMs( 100 );
	//if(mission==0)
    //{
    	VP_GameKeyPoll( 1000 );
		outMoney = 0;			    
        do
		{
		    mission=MDB_Coin_IfDispensed(sysMDBMission.coinBuf,&sysMDBMission.coinBufLen,&sysMDBMission.coinType,&tempMoney,&sysMDBMission.coinStock);
		    outMoney++;
			DelayMs(10);
		}
		while(mission!=0x00);
	//}

	mission=MDB_Coin_Dispense(1,tHopper2);
    DelayMs( 100 );
	//if(mission==0)
	//{
    	VP_GameKeyPoll( 1000 );
		outMoney=0;
		do
		{
		    mission=MDB_Coin_IfDispensed(sysMDBMission.coinBuf,&sysMDBMission.coinBufLen,&sysMDBMission.coinType,&tempMoney,&sysMDBMission.coinStock);
		    outMoney++;
			DelayMs(10);
		}
		while(mission!=0x00);
	//}
	memset(sysMDBMission.coinBuf,0,sizeof(sysMDBMission.coinBuf));
	sysMDBMission.coinBufLen=0;
	sysMDBMission.coinType=0;
	sysMDBMission.coinStock=0;
   	*/
	
	
TAB_DISPENSE_RETRY:
	//用于level3的找零
	//dispenseValue = CurrentDispenseCoin/50;
    //mission = MDB_Coin_EXP_Payout( dispenseValue );
    //用于level2的找零
	dispenseValue = CurrentDispenseCoin/10;
	mission =changeCNY(resultdisp,dispenseValue);
	if(mission == 1)
	{
		for(i=0;i<2;i++)
		{
			if(resultdisp[i]==0) 
				continue;

			//len = sprintf( str, "i=%02bu,%02bu", i, resultdisp[i] );
	        //DisplayStr( 0, 0, 1, str, len );
		    //WaitForWork( 5000, NULL );
			while(resultdisp[i])
			{
				if(resultdisp[i] > 15)
				{
					tempdisp = 15;
					resultdisp[i] -= tempdisp;
				}
				else
				{
					tempdisp = resultdisp[i];
					resultdisp[i] -= tempdisp;
				}
				if(i==0)
					//一元
					mission=MDB_Coin_Dispense(1,tempdisp);
				else if(i==1)
					//5jiao
					mission=MDB_Coin_Dispense(0,tempdisp);
				
				WaitForWork(1000,NULL);	
			    if( mission == 0x00 )
				{

				    sysVPMission.hopperOutTimer = tempdisp + 20;
				    while( sysVPMission.hopperOutTimer )
					{
				    	mission = MDB_Coin_EXP_PayoutValuePoll( &dispensedNum );
						if( mission == 0x00 )
						{
							mission = MDB_Coin_EXP_PayoutStatus( sysMDBMission.coinBuf, &sysMDBMission.coinBufLen );
							if( mission == 0x01 )
							{
								if(sysMDBMission.coinBuf[0] > 0)
								{
									sysMDBMission.payout5jNum = sysMDBMission.coinBuf[0];
									sysVPMission.payOutMoney1 = sysMDBMission.payout5jNum*50;
									temppayOutMoney1 += sysVPMission.payOutMoney1;
									//TradeCounter.Hopper1Num += sysMDBMission.payout5jNum;
						            //TradeCounter.Hopper1Sum += sysVPMission.payOutMoney1;
									SellAccumulateMoney(&TradeCounter.Hopper1Sum,sysVPMission.payOutMoney1);
						            //TradeCounter.ChangeSum += sysVPMission.payOutMoney1;
									CurrentDispenseCoin = CurrentDispenseCoin-sysVPMission.payOutMoney1;
                                    //区间
									//TradeCounter.Hopper1SumBack += sysVPMission.payOutMoney1;
									SellAccumulateMoney(&TradeCounter.Hopper1SumBack,sysVPMission.payOutMoney1);
								}
								else if(sysMDBMission.coinBuf[1] > 0)
								{
									sysMDBMission.payout1yNum = sysMDBMission.coinBuf[1];
									sysVPMission.payOutMoney2 = sysMDBMission.payout1yNum*100;
									temppayOutMoney2 += sysVPMission.payOutMoney2;
									//TradeCounter.Hopper2Num += sysMDBMission.payout1yNum;
						            //TradeCounter.Hopper2Sum += sysVPMission.payOutMoney2;
									SellAccumulateMoney(&TradeCounter.Hopper2Sum,sysVPMission.payOutMoney2);
						            //TradeCounter.ChangeSum += sysVPMission.payOutMoney2;
									CurrentDispenseCoin = CurrentDispenseCoin-sysVPMission.payOutMoney2;

									//区间
									//TradeCounter.Hopper2SumBack += sysVPMission.payOutMoney2;
									SellAccumulateMoney(&TradeCounter.Hopper2SumBack,sysVPMission.payOutMoney2);
								}
								sysVPMission.hopperOutTimer = 0;							
							}
							else
							{
								//VP_GameKeyPoll( 1000 );	         
							}
						}
						else
						{
							VP_GameKeyPoll( 1000 );		
							FeedWatchDog();	
						}
					}
				}
				else if( mission == 0x80 )
				{
			    	trymis++;
			        if(trymis<3)
			        {
					    WaitForWork( 300, NULL );
			        	goto TAB_DISPENSE_RETRY;
			        }
				}
			}
		}
	}
	
	   
    if( CurrentDispenseCoin > 0 )
	{
		m_Errorflag = 1;
		//如果找零器有多找零情况，保存多找零金额;
        //如果有欠款，显示欠款金额;by gzz 20110825
        if(CurrentDispenseCoin>30000)
        {
         	SystemParameter.MuchOutFlag = 1;
            SystemParameter.NotOutMax = 65535-CurrentDispenseCoin;
			//HardWareErr |= 0x8000;
        } 
        else
        {  
            SystemParameter.MuchOutFlag = 0;
			SystemParameter.NotOutMax = CurrentDispenseCoin;
			VPMission_Debt_RPT(0,0,CurrentDispenseCoin);//上报欠条;by gzz 20110825
        }
		IOUFlag = 1;
	}
	else
	{
	    m_Errorflag = 0;	
	}

    //-------------------------------------------------------------------------------------------
	//if( (temppayOutMoney1>0) || ( temppayOutMoney2>0 ) )
	if(flag)
	{
		VPMission_Payout_RPT( VP_DEV_COIN, temppayOutMoney1, temppayOutMoney2  );
        sysVPMission.payOutMoney1 = 0;
		sysVPMission.payOutMoney2 = 0;
	}
	else
	{
		VPMission_Payout_RPTNOACK( VP_DEV_COIN, temppayOutMoney1, temppayOutMoney2  );
        sysVPMission.payOutMoney1 = 0;
		sysVPMission.payOutMoney2 = 0;
	}	
	//===========================================================================================
	if( m_Errorflag == 0 )
	{	
	    /*
		DisplayLine = STR_TAKE_COIN;
		DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[ DisplayLine ].str, DispInformationTable[ DisplayLine ].len );
        //WaitForWork( 1000, NULL );	   //2000-1000
		WaitForWork( 100, NULL );
		sysVPMission.msGameTimer1 = 100;   //
		while( sysVPMission.msGameTimer1 )
		{
			//----------------------------------------------------
	        //Send the game key and poll pc in speciall time
    		if( IfGameKeyOn() )
			{
				VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
        		//Beep();
			}
	        result = VPMission_Poll();
	        //====================================================
		}
	    */

	}
	else
	{
		SaveGlobalParam();
		DisplayLine = STR_COIN_NOT_ENOUGH; // 硬币不够 
		DisplayStr( 0, __LINE0__ , 1, DispInformationTable[	DisplayLine ].str, DispInformationTable[ DisplayLine ].len );			
		DisplayLine = STR_DUO_MONEY;
		memset( DisplayBuffer, 0, sizeof( DisplayBuffer ) );
		memcpy( DisplayBuffer, DispInformationTable[DisplayLine ].str, DispInformationTable[ DisplayLine ].len );
		i = DispInformationTable[ DisplayLine ].len;		
		switch( SystemParameter.curUnit )
		{
			case 1:
				i += sprintf( DisplayBuffer + i, "%u", SystemParameter.NotOutMax );
			break;				
			case 10:
				i += sprintf( DisplayBuffer + i, "%u.%u", SystemParameter.NotOutMax/SystemParameter.curUnit, SystemParameter.NotOutMax%SystemParameter.curUnit );
			break;
			case 100:
			i += sprintf( DisplayBuffer + i, "%u.%02u", SystemParameter.NotOutMax/SystemParameter.curUnit, SystemParameter.NotOutMax%SystemParameter.curUnit );			
			break;		
		}		
		/*if( SystemParameter.NotOutMax >= 100 )					
			i += LenOfNum( SystemParameter.NotOutMax ) + 1;
		else 
			i += LenOfNum( SystemParameter.NotOutMax ) + 2;*/
		DisplayStr( 0, __LINE1__ , 1, DisplayBuffer, i );			
		//WaitForWork( 2500 , NULL );
		/*
		WaitForWork( 100, NULL );
		sysVPMission.msGameTimer1 = 250;
		while( sysVPMission.msGameTimer1 )
		{
			//----------------------------------------------------
	        //Send the game key and poll pc in speciall time
    		if( IfGameKeyOn() )
			{
				VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
        		//Beep();
			}
	        result = VPMission_Poll();
	        //====================================================
		}
		*/
		VP_GameKeyPoll( 2000 );
	    //HardWareErr = ERR_HOPPER;
	}
   

	//Update the changer tube status
	VP_GameKeyPoll( 500 );	
	MDB_Coin_TubeSta_API();
	sysVPMission.tubeCoinMoney = sysMDBMission.coinAllValue;//保存硬币斗可找零金额
	sysVPMission.inBillMoney = 0;
	if( SystemParameter.SVC_NoChange != 1 )
	{
	    #ifdef MACHINE_SET_MDB
		    //if( (sysMDBMission.coinDeviceStatus!=0) || (sysMDBMission.coin5jNum < 3 ) || (sysMDBMission.coinAllValue<500) )
            if( (sysMDBMission.coinDeviceStatus!=0) || (sysMDBMission.coinAllValue<500))
	        {
	        	//HardWareErr |= 	0x0008;                
	        }
			else
			{                
				HardWareErr &= 	0xFFF7;
			}
		#else
		    if( DeviceStatus.ChangeUnit1 != 0 )
			{
				//HardWareErr |= 	0x0008;
			}
			else
			{
				HardWareErr &= 	0xFFF7;
			}
		#endif
	}
	return 0;

}

//上面超时且无金额则返回1,有钱则返回0
u_char CheckTimerOrMoney()
{
	if ( CurrentPayed == 0 )
	{
		DisplayLine = STR_OPERATION_TIMEOUT;	
		DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
	//	ClearDisplayLine1();
		ClearDisplayLine( 2 );
		WaitForWork( 3000 , NULL );
		return 1 ;
	}
	return 0;
}

u_char DispInputMoney()
{
	u_char xdata length = 0;
	u_char xdata pt= 0;

	//先显示钞票不足
	DisplayLine = STR_NOT_ENOUGH_NOTE;
	DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );	
	pt = 0;
	pt = ChooseReturn( );
	if( pt == 0 )//确定
	{
		EnableBillDev();
		DisplayCurrentMoney( CurrentPayed );
		return 1;
	}
	else //取消或超时
	{
		memset( DisplayBuffer , 0 , sizeof( DisplayBuffer ));
		DisplayLine = STR_PLEASE_WAIT;		
		DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
//		ClearDisplayLine1();
		ClearDisplayLine( 2 );
		WaitForWork( 100 , NULL );
		return 0;
	}
}

//找更便宜的货物
u_char ExistAnyChoose()
{
	u_char xdata i;
	
	for( i= 0 ; i <= GOODSWAYNUM ; i++)
	{		
		if ( CurrentPayed >= GoodsWaySetVal[i].Price )		 	
			return 0 ; //找到同类更便宜卡	
	}
	//Trace( "\n Cheaper stock Not Found! Ready to Reject Coin=%d" , CurrentPayed );
	CurrentDispenseCoin = CurrentPayed;
	return( 1 ); //没的选择，必需要退币 
}

//提示客户是否选择买其它的货，确定返回1并到50节点让用户重新输入，取消或超时返回0
u_char CheaperOnePlz()
{
//	u_char xdata length;
	u_char xdata pt= 0;
//	u_int xdata Unicode_buffer[ MAX_CHAR_NUMBER *2 ];

//	Trace( "\n CheaperOnePlz");
	DisplayLine = STR_CHOICE_CHEAPERONE;
  //  length = DispInformationTable[	DisplayLine ].len;
  //  memcpy( DisplayBuffer , DispInformationTable[ DisplayLine ].str , length );	
	DisplayStr( 0, 0, 1, DispInformationTable[	DisplayLine ].str, DispInformationTable[ DisplayLine ].len );
//	DisplayInfo( __LINE0__ , 0 , DisplayBuffer );

	pt = ChooseReturn( );
	if( pt == 0 )//确定
	{
		ClearKey();	
		return 1;
	}
	if( pt == 1 ) //取消
		return 0;
	DisplayLine = STR_OPERATION_TIMEOUT;
	DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
	ClearDisplayLine( 2 );
	WaitForWork( 500 , NULL );
	return 0;
}

//看系统是否允许退币，允许退币且需要退币则退并返0，不允许则返回1反复提示,不需要退币返回2	
u_char CheckChangePermit()
{
//	Trace( "\n CheckChangePermit");
//	ClearDisplayLine1();	
	u_char xdata i;
	if( CurrentPayed > 0 ) 
	{
		if ( SystemParameter.RefundPermission )
		{
			if( IsCanChange() != 1 )
			{
				CurrentDispenseCoin = 0;
				return 2;
			}
			//在此设置出币总额
			DisplayLine = STR_REFUND_MONEY;
			DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
	//		DisplayLine = STR_PLEASE_WAIT;
	//		DisplayStr( 0, __LINE1__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
			memset( DisplayBuffer , 0 , sizeof( DisplayBuffer ));
			//////////////////
			memset( DisplayBuffer,0,sizeof(DisplayBuffer) );
			switch( SystemParameter.curUnit )
			{
				case 1:
					i = sprintf( DisplayBuffer, "%s%u", DispInformationTable[STR_CHANGE].str, CurrentPayed );
				break;				
				case 10:
					i = sprintf( DisplayBuffer, "%s%u.%02u", DispInformationTable[STR_CHANGE].str,
						CurrentPayed/SystemParameter.curUnit,CurrentPayed%SystemParameter.curUnit );				
				break;
				case 100:
					i = sprintf( DisplayBuffer, "%s%u.%02u", DispInformationTable[STR_CHANGE].str,
						CurrentPayed/SystemParameter.curUnit,CurrentPayed%SystemParameter.curUnit );				
				break;		
			}
			DisplayStr( 0, __LINE1__ , 1 , DisplayBuffer, i );
			CurrentDispenseCoin = CurrentPayed;
			return 0; //退币
		}
		else
			return 1;
	}
	else 
		return 2;
}

u_char DispGetCoin()
{
	return( 0 );
}

//找零
u_char ReturnChange()
{
    u_char xdata result = 0;
	u_int  xdata moneyBuf = 0;
	u_char xdata flag = 0;
//	Trace( "\n ReturnChange");
//	LightBlinkFlag = 0;    
//	LightBlinkFlag = 1; 			
//	if ( CurrentPayed > GoodsWaySetVal[CurrentStockCode].Price )
//	{

    sysVPMission.boughtFlag = 0; //Clear the bought flag after vending.
    //----------------------------------------------------
	//Send the game key and poll pc in speciall time
    if( IfGameKeyOn() )
	{
		VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
        //Beep();
	}
	//if( ( sysVPMission.vendCmd == 0 )&&(sysVPMission.changeCmd == 0)&&(sysVPMission.costCmd == 0)&&(sysVPMission.returnCmd == 0) )
		result = VPMission_Poll();
	//====================================================
	sysVPMission.inBillMoney = 0;
	if( sysITLMission.billHoldingFlag == 1 )
	{
		moneyBuf = CurrentPayed + sysITLMission.billHoldingValue;
	}
	else
	{
	    moneyBuf = CurrentPayed;
	}
	
    if( moneyBuf > 0 )
	{
		if( sysITLMission.billHoldingFlag == 1 ) 
		{
		    
			flag = MDB_Bill_EscrowCtr(0);
			//调整顺序，flag==0放在sysVPMission之上;by gzz 20110721
			if( flag == 0 )
			{
				sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
				sysITLMission.billHoldingFlag = 0;
			    sysITLMission.billHoldingValue = 0;
			}
			else
			{
				sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
				//... Need more discussion for better soluton!
	            sysITLMission.billHoldingFlag = 0;
			    sysITLMission.billHoldingValue = 0;
			}
	        //-----------------------------------------------------
			sysVPMission.escrowOutFlag = 1;			
			if( sysVPMission.escrowOutFlag == 1 )
		    {
			    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
				sysVPMission.escrowOutFlag = 0;
		        sysVPMission.escrowMoney = 0;     
			}
			//=====================================================
			//.Send the reject message to PC

			if( CurrentPayed > 0 )
	        {
	            //UpdateSelLed_Trade();
				//DisplayCurrentMoney( CurrentPayed/*+sysITLMission.billHoldingValue* / );
	            //by gzz 20110416
				//DisableBillDev();
	            //EnableBillDev();
	            //End_by gzz 20110416	
			
				
	        }
	        else
	        {
	            //by gzz 20110416
				//DisableBillDev();
	            //EnableBillDev();
	            //End_by gzz 20110416	
	            UpdateSelectionLed_GoodsSta(); 
	        }
		}
		CurrentDispenseCoin = CurrentPayed;     // - GoodsWaySetVal[CurrentStockCode].Price; 
		CurrentPayed = 0;
		//DisplayStr( 0, __LINE0__ , 1 , "3", 1 );
		//WaitForWork( 2000, NULL );  //500-50-100-200
		if( DispenseCoin(0) == 0 )					
			return( 0 );		
		else					
			return 1;
    }
	else
	{
		return 0;
	}

//	}	
//	return  0 ;
}

u_char DispThx()
{
    /*
	DisplayLine = STR_THX_YOU_WITHOUT_COIN;
	DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
	ClearDisplayLine( 2 );
	WaitForWork( 100 , NULL );	 //1500,500
    */

	return( 0 );
}

u_char SellCodeOver()
{	
	//KeyLock( );		
	SaveTradeParam();

	//----------------------------------------------------
	//2011.5.20 added
	//After vending, update the goods window led status.
    UpdateSelectionLed_GoodsSta();
    //====================================================

    if( sysITLMission.ITLSet != 1 )
    {
		CasherGetMachineState( 0 );//空闲状态
		CasherResetOuttime();
    }
	CheckHardWare( 1 );		
	//KeyUnLock();
	if(HardWareErr==0)//by gzz 20110419
	{
	    /*
		ITLMission_Reject();
		WaitForWork(100,NULL);
		ITLMission_Init_1();
		*/
	}
    //----------------------------------------
	ClearKey();
	//========================================
	sysVPMission.VPDevCtr  |= 0x80;
	sysVPMission.dspTimer1 = VP_DSP_TIME1;
    //ChangeOver(); 
    /////////////////////////
    //LoadGlobalParam();
    ///////////////////////////    
	return( 1 );
}

u_char AllGoodsWayError()
{
	u_char xdata i = 0;

	SaveTradeParam();
	
	/*
	Disabled by Andy on 2010.10.26
	for( i = 0; i < GOODSWAYNUM; i ++ )	
		GoodsWaySetVal[i].WayState = 0x08;
	*/

	HardWareErr |= 0x0040;
	DeviceStatus.Driver64 |= 0x01;
  //HardWareErr = ERR_GOODSBOARDERR;	
	DisplayLine = STR_HARDWARE_ERR;
	DisplayStr( 0, __LINE0__, 1, DispInformationTable[DisplayLine].str, DispInformationTable[DisplayLine].len );	
	return 1;
}


u_char MaintFunction()
{	
	u_char xdata i = 0;
	u_int  xdata coinBuf = 0;

	Beep();
	WaitForWork(200,NULL);
	Beep();	
    //SaveGlobalParam();
	AlarmSpacing = 0;
	AlarmTime = 0;
	if( SystemParameter.BillNo == 1 )
	{
		DisableBillDev();
	}
	//MDBMission_Bill_Disable();//by gzz 20120323
	SystemStatus = 2;
	if( KeyLockFlag )
	{
		KeyUnLock(); //锁定键盘，不让客户按键
		KeyLockFlag = 0;
	}	
	#ifdef   _SHUPING_
	#else
		sysVPMission.sel1ReadyLed = 0x00;
	    sysVPMission.sel1ErrLed   = 0x00;
	    sysVPMission.sel2ReadyLed = 0x00;
	    sysVPMission.sel2ErrLed   = 0x00;
	    sysVPMission.sel3ReadyLed = 0x00;
	    sysVPMission.sel3ErrLed   = 0x00;
		GetKey_M2();
	#endif	
	MaintFlag = 1;

	HardWareErr = 0;   //Changed by GZZ 2011.7.19
	//--------------------------------------------------------------
	sysVPMission.VPMode = 0;
    VPMission_Admin_RPT( VP_ADMIN_ENTER_MENU, 0, 0 );
	//WaitForWork(1000,NULL);
	VPMission_Act_RPT(VP_ACT_ADMIN,VP_ACT_ENTERADMIN);
	//WaitForWork( 800, NULL );
	sysVPMission.adminType = 1;
	VPMission_Status_RPT();	
	do
	{
		WatchDogDisable();
		mainmenu();
		//WatchDogInit();
	}
	while( IsModeOff() );    //是否模式开关处于维护状态

	//sysVPMission.VPMode = 1;
	sysVPMission.VPInit = 1;
	sysVPMission.VPMode = 1;
	VPMission_Admin_RPT( VP_ADMIN_QUIT_MENU, 0, 0 );
	//WaitForWork(1000,NULL);
	VPMission_Act_RPT(VP_ACT_ADMIN,VP_ACT_OUTADMIN);
	//WaitForWork( 800, NULL );
	sysVPMission.adminType = 0;
	//VP_CMD_GetStatus();	
	MDB_Coin_TubeSta_API();
	sysITLMission.billHoldingFlag = 0;
	sysITLMission.billHoldingValue = 0;
    //===============================================================

	MaintFlag = 0;
	DisplayStr( 0, 0, 1, DispInformationTable[ STR_PLEASE_WAIT ].str, DispInformationTable[ STR_PLEASE_WAIT ].len );	
	DisplayStr( 0, 1, 1, " ", 1 );
	ClearKey();
	SystemStatus = 1;
	HardWareErr = 0; //默认硬件无故障，然后自检去真正检查	
	IsCanInCashFlag = 1;
	if( KeyLockFlag == 0 )
	{
		KeyLock(); //锁定键盘，不让客户按键
		KeyLockFlag = 1;
	}	
	//isShowLcd = 1;

	//扫描单价是否有为0的
    /*
	for( i = 0; i < GOODSWAYNUM; i ++ )	
	{
		if( ( GoodsWaySetVal[i].WayNo == InputGoodsWayList[i].GoodsWayNo ) && 
			( InputGoodsWayList[ i ].UseState == 1 ) &&
			( GoodsWaySetVal[i].Price == 0 ) )
		{
			HardWareErr |= 0x0400;
			break;
		}
	}	
	if( i != GOODSWAYNUM )
	{
		UpdataDisp = 0;
		///清除报警标志位	
		MoneyDevAlarm = 0;
		SystemErrorAlarm = 0;
		SMSQuery = 0;
		//清除短信发送缓存区内的所有短信
		//ClearAllSMSInSMSBuffer();
		return 0;
	}
    */

	GetHopperList();
	CheckHardWare( 0 );
	GetHopperList();		
	
	/*
	for( i = 0; i < GOODSWAYNUM; i ++ )	
	{
		GoodsWaySetVal[i].WayState &= 0x1f;	
    }
    */

	if( ( ( DeviceStatus.BillAccepter != 0 ) || ( IsCanInCashFlag == 0 ) ) && ( DeviceStatus.CoinAccepter != 0 ) )
	{	
	    //HardWareErr |= 0x0002;
	}
    
    //清货道控制板的SN
	if( ChannelTask( 0, CHANNEL_CLEAR ) == 2 )
	{
		//执行命令超时,应是主板与货道驱动板通讯故障
		Trace("\n Channel are break");
        /*
        //Disabled by Andy on 2010.10.26
		for( i = 0; i < GOODSWAYNUM; i ++ )	
			GoodsWaySetVal[i].WayState = 0x09;
        */
		HardWareErr |= 0x0040;
		DeviceStatus.Driver64 |= 0x01;
	}
    else
    {
        //ChannelTask( 0, CHANNEL_QUERY );
    }
	UpdataDisp = 0;
	///清除报警标志位	
	MoneyDevAlarm = 0;
	SystemErrorAlarm = 0;
	SMSQuery = 0;
	//清除短信发送缓存区内的所有短信
	//ClearAllSMSInSMSBuffer();	
    coinBuf = GetCoin();
	return 0;
}

void Mission()
{	
    /*	
	if ( IsShortMsgIn())
	{	
 		IsShortMessageIn = 1;
		Trace("Have SMS come in ");
	}
	*/
}

//有金额的时候,不显示故障页面
u_char ShowOutofService()
{	
	u_int  xdata moneyBuf= 0;

	if( sysITLMission.billHoldingFlag == 1 )
	{
		moneyBuf = CurrentPayed + sysITLMission.billHoldingValue;
	}
	else
	{
		moneyBuf = CurrentPayed;
	}	
	//WaitForWork( 1000, NULL );
	if(moneyBuf)
	{		
		if(errInputMoney==0)
			errInputMoney = 1;		
	}
	else
	{
		if(errInputMoney==1)
			errInputMoney = 0;	
	}
	return 0;
}


//在故障页面判断禁能纸币器硬币器
u_char EnableBillOutofService()
{	
	u_int  xdata moneyBuf= 0;

	if( sysITLMission.billHoldingFlag == 1 )
	{
		moneyBuf = CurrentPayed + sysITLMission.billHoldingValue;
	}
	else
	{
		moneyBuf = CurrentPayed;
	}	
	//WaitForWork( 1000, NULL );
	if(moneyBuf < SystemParameter.BanknoteMax)
	{		
		EnableBillDev();
	}	
	return 0;
}



u_char OutofService()
{	

    //#define CHKTIMES   600    //1000-600
	u_char xdata len = 0;	
	u_char xdata errcode = 0,errcode2=0;
	u_char xdata errCodeStr[20];
	u_char xdata flag = 0;
	//static  u_int   xdata chkTime   = 0;
	static  u_char  xdata errUpdate = 0;
	
    //Trace( "\n HardWareErr3 = %bu", HardWareErr );
    //CasherGetMachineState( 1 );//按交易状态处理
	
    /*
	chkTime++;
	if( (chkTime >= CHKTIMES)&&(HardWareErr & 0x0800) )
	{
		chkTime = 0;
	    //-------------------------------------------
		//Added by Andy on 2010.10.21 for GOC dev.
	    if( ChannelTask( 1, CHANNEL_QUERY ) == 2 )
		{
			//执行命令超时,应是主板与货道驱动板通讯故障
			DeviceStatus.Driver64 |= 0x01;
			HardWareErr |= 0x0040;	
		}
        DisableBillDev();    //Added 2011.1.20
    }
    */

	//if ( isShowLcd == 1 )
	{		
		if(sysVPMission.ErrFlag2 == 0)
		{		
			/*
			if( SystemParameter.ACDCModule == 1 )
			{
				//关闭压缩机展示灯
				sysVPMission.ACDCLedCtr = 0; 
				sysVPMission.ACDCCompressorCtr = 0;			
				ACDCMission();
			}
			*/
			//DisplayStr( 0, __LINE0__ , 1 , "close", 5 );
			//WaitForWork( 2000, NULL );  //500-50-100-200	

			//强制退币
			//-----------------------------------------------------------------------------------
		    //Andy added 2011.4.14.Rject the holding bill.
		    if( sysITLMission.billHoldingFlag == 1 ) 
			{
				flag = MDB_Bill_EscrowCtr(0);
				//调整顺序，flag==0放在sysVPMission之上;by gzz 20110721
				//-----------------------------------------------------
				sysVPMission.escrowOutFlag = 1;
				sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
				sysITLMission.billHoldingFlag = 0;
			    sysITLMission.billHoldingValue = 0;
				if( sysVPMission.escrowOutFlag == 1 )
			    {
				    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
					sysVPMission.escrowOutFlag = 0;
			        sysVPMission.escrowMoney = 0;     
				}
				//=====================================================
				if( flag == 0 )
				{
					sysITLMission.billHoldingFlag = 0;
				    sysITLMission.billHoldingValue = 0;
				}
				else
				{
					//... Need more discussion for better soluton!
	                sysITLMission.billHoldingFlag = 0;
				    sysITLMission.billHoldingValue = 0;
				}
				//.Send the reject message to PC				
			}
			if(CurrentPayed>0) 
			{
			    CurrentDispenseCoin = CurrentPayed;	
				CurrentPayed = 0;
				//DisplayStr( 0, __LINE0__ , 1 , "4", 1 );
				//WaitForWork( 2000, NULL );  //500-50-100-200
				DispenseCoin(0);
				DisableBillDev();				
	            SaveTradeParam();  //2011.6.10 added: Update the sale data in flash!
	            UpdateSelectionLed_GoodsSta();
	            EnableBillDev(); 
				memset(&sysMDBMission.coinBuf,0,sizeof(sysMDBMission.coinBuf));
		        memset(&sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));            
			}
			if( sysVPMission.boughtFlag==1 )
			{
				sysVPMission.boughtFlag = 0;
			}
			if( sysITLMission.billHoldingFlag != 1 )
			{
				MDB_Bill_EscrowCtr(0);  //Added to reject the unknown holding bill
			}
			//关闭纸币器硬币器
			DisableBillDev(); 
			sysVPMission.billCoinEnable = 0;
			sysVPMission.ErrFlag2 = 1;
			//上报STATUS_RPT
			if(sysVPMission.ErrFlag==0)
			{
				sysVPMission.ErrFlag=1;
				VPMission_Status_RPT();
			}
		}
		memset(errCodeStr,0,sizeof(errCodeStr));        
		//if( HardWareErr & 0x0001 )
		//{
			//errcode = 0xF1;
			//sprintf( errCodeStr, "%s", "纸硬币器故障" );			
			//sprintf( errCodeStr, "%s", "  请用其它方式购物" );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "纸硬币器故障" );			
		//}
		//else 
		//if( HardWareErr & 0x0002 )
		//{
			//errcode = 0xF2;
			//sprintf( errCodeStr, "%s", "硬币器故障" );			
			//sprintf( errCodeStr, "%s", "  请用其它方式购物" );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "硬币器故障");
		//}
		//else if( HardWareErr & 0x0004 )
		//{
			//errcode = 0xF3;
			//sprintf( errCodeStr, "%s", "找币器故障" );			
			//sprintf( errCodeStr, "%s", "  请用其它方式购物" );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "找币器故障");
		//}
		//else if( HardWareErr & 0x0008 )
		//{
			//errcode = 0xF4;
            //sprintf( errCodeStr, "%s", "找币器缺币" );			
			//sprintf( errCodeStr, "%s", "  请用其它方式购物" );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "找币器缺币");
		//}
		//else if( HardWareErr & 0x0010 )
		//{
			//errcode = 0xF5;			
			//sprintf( errCodeStr, "%s", "  " );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "所有货道无货");
		//}
		//else if( HardWareErr & 0x0020 )
		//{
			//errcode = 0xF6;
			//sprintf( errCodeStr, "%s", "所有货道故障" );			
			//sprintf( errCodeStr, "%s", "  " );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "所有货道故障");
		//}
		//else 
		if( HardWareErr & 0x0040 )
		{
			errcode = 0xF7;	
			//sprintf( errCodeStr, "%s", "货道板故障" );			
			//sprintf( errCodeStr, "%s", "  " );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "货道板故障");
		}
		else if( HardWareErr & 0x0080 )
		{
			errcode = 0xF8;				
			//sprintf( errCodeStr, "%s", "  " );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "所有货道不能工作");
		}
		else if( HardWareErr & 0x0100 )
		{
			errcode = 0xF9;	
			//sprintf( errCodeStr, "%s", "系统参数故障" );			
			//sprintf( errCodeStr, "%s", "  " );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "系统参数故障");
		}
		else if( HardWareErr & 0x0200 )
		{
			errcode = 0xFA;
			//sprintf( errCodeStr, "%s", "LCD 故障" );			
			//sprintf( errCodeStr, "%s", "  " );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "LCD 故障" );
		}
		//else if( HardWareErr & 0x0400 )
		//{
			//errcode = 0xFB;
			//sprintf( errCodeStr, "%s", "货道单价异常" );			
			//sprintf( errCodeStr, "%s", "  " );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "货道单价异常" );
		//}
		//else if( HardWareErr & 0x0800 )
		//{
		    //errcode = 0xFC;
			//sprintf( errCodeStr, "%s", "出货确认故障" );			
			//sprintf( errCodeStr, "%s", "  " );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "出货确认故障" );
		//}
		else if( HardWareErr & 0x1000 )
		{
		    errcode = 0xFD;
			//sprintf( errCodeStr, "%s", "选货模块故障" );			
			//sprintf( errCodeStr, "%s", "  " );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "选货模块故障" );
		}
		else if( HardWareErr & 0x2000 )
		{
			errcode2 = 0x70;
		    errcode = 0x02;
			//sprintf( errCodeStr, "%s", "PC小数点故障" );			
			//sprintf( errCodeStr, "%s", "  " );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "PC小数点故障" );
		}
		else if( HardWareErr & 0x4000 )	
		{
		    errcode = 0xE1;
			//sprintf( errCodeStr, "%s", "pc离线" );			
			//sprintf( errCodeStr, "%s", "  " );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "pc离线" );
		}
        //添加币管没有锁紧故障;by gzz 20110827
		//else if( HardWareErr & 0x8000 )	
		//{
		    //errcode = 0xF0;//币管没有锁紧故障
			//sprintf( errCodeStr, "%s", "币管未关闭" );			
			//sprintf( errCodeStr, "%s", "  请用其它方式购物" );

			//sprintf( sysVPMission.hardWareErrStr, "%s", "币管未关闭" );
		//}
		else
		{
			errcode = 0xff;
		}
		sysVPMission.hardWareErrShow = errcode;


		if(errInputMoney==0)
		{
			/*
	        //if((errcode == 0xF1)||(errcode == 0xF2)||(errcode == 0xF3)||(errcode == 0xF4))
	        //添加币管f0故障;by gzz 20110825
			if((errcode == 0xF1)||(errcode == 0xF2)||(errcode == 0xF3)||(errcode == 0xF4)||(errcode == 0xF0))
	        {
	        	DisplayLine = STR_OUT_OF_CASHSERVICE;
				DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
				//WaitForWork( 200, NULL );  //500-50-100-200
				WaitForWork( 20, NULL );

	            len = sprintf( DisplayBuffer, "%s" , errCodeStr );
	        }
	        else
			*/
	        {
				DisplayLine = STR_OUT_OF_SERVICE;
				DisplayStr( 0, __LINE0__ , 1 , DispInformationTable[DisplayLine ].str, DispInformationTable[DisplayLine ].len );
				//WaitForWork( 200, NULL );  //500-50-100-200
				WaitForWork( 20, NULL );

	            //-------------------------------------------------------------------------------------
				//发现如果找币欠款，会显示故障信息，加上欠款金额。由于字符超过第二行显示信息最大值，因此
		        //出现溢出现象。这时，由于hardwareerr值被覆盖,当hopper补币恢复后，不能重新回到交易状态
				//by gzz 20110811
	            /*
				if( ( IOUFlag ) && ( SystemParameter.NotOutMax != 0 ) )
				{			
				#ifdef _CHINA_
					len = sprintf( DisplayBuffer, "[%02bx]  %s" ,errcode, errCodeStr );			
				#else
					len = sprintf( DisplayBuffer, "Err code=%02bx  " , errcode );			
			    #endif
					DisplayLine = STR_DUO_MONEY;			
					memcpy( DisplayBuffer + len , DispInformationTable[DisplayLine ].str, DispInformationTable[ DisplayLine ].len );
					len += DispInformationTable[ DisplayLine ].len;
					switch( SystemParameter.curUnit )
					{
						case 1:
							len += sprintf( DisplayBuffer + len, "%u", SystemParameter.NotOutMax);
							break;				
						case 10:
							len += sprintf( DisplayBuffer + len, "%u.%u", SystemParameter.NotOutMax/SystemParameter.curUnit, SystemParameter.NotOutMax%SystemParameter.curUnit );
							break;
						case 100:
							len += sprintf( DisplayBuffer + len, "%u.%02u", SystemParameter.NotOutMax/SystemParameter.curUnit, SystemParameter.NotOutMax%SystemParameter.curUnit );
						break;		
					}			
					IOUFlag = 0;
				}
				else		
				#ifdef _CHINA_
					len = sprintf( DisplayBuffer, "[%02bx]  %s" , errcode, errCodeStr );			
				#else
					len = sprintf( DisplayBuffer, "Err code = %02bx  " , errcode );			
			    #endif
				*/

				#ifdef _CHINA_
					len = sprintf( DisplayBuffer, "[%02bx%02bx]  %s" , errcode2,errcode, errCodeStr );			
				#else
					len = sprintf( DisplayBuffer, "Err code = %02bx  " , errcode );			
			    #endif
		        
			}

	        //Trace( "\n LEN = %bu", len );
			DisplayStr( 0, __LINE1__ , 1, DisplayBuffer, len );
			//WaitForWork( 300, NULL );    //500-50-100-200-300
			WaitForWork( 20, NULL );
		}


		errUpdate++;
		if( errUpdate >= 2 )
		{
			//isShowLcd = 0;
			errUpdate = 0;
		}
        //--------------------------------------------------------
		//SaveGlobalParam();
		//SaveGoodsSet();
		//ClearKey();
        //========================================================			
	}
	
	//111024 by cq 删除

	/*
	if( SystemParameter.MobileON == 1 )
    {
		if( ( AlarmTime == 0x00 ) && ( ScanIsSendAlarm() == 0 ) )
		{	
			Trace("\n The Frist Alarm");			
			AlarmTime = 1;		
		}
		else 
		{
			CycleSendSMS();		
		}
    }	
	*/
    return 1;
}

u_char StackTheBillAPI( void )
{
    u_char xdata flag = 0;
	u_char xdata mission = 0;
    u_int  xdata CurrentDomain = 0;
	
	//Before vending, stack the holding bill
	if( sysITLMission.billHoldingFlag == 1 )
	{ 
        flag = MDB_Bill_EscrowCtr(1);
		if(flag==0)
		{
		    sysVPMission.billSTimer = TIME_BILL_STACK;
		    while( sysVPMission.billSTimer )
			{
                mission = MDB_Bill_IfStacked( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
				if( mission == 1 )
				{
				    CurrentDomain = sysITLMission.billHoldingValue;
					Trace( "\n Get in Cash Ok" );
					Trace( "\n CurrentDomain = %d",CurrentDomain );		
					CurrentPayed += CurrentDomain * BILLMULTIPLE;	
					//TradeCounter.TrueCurrencySum += CurrentDomain * BILLMULTIPLE;
					//TradeCounter.CashSum += CurrentDomain * BILLMULTIPLE;
					//TradeCounter.CashSumBack += CurrentDomain * BILLMULTIPLE;//区间
					SellAccumulateMoney(&TradeCounter.CashSum,CurrentDomain * BILLMULTIPLE);
					SellAccumulateMoney(&TradeCounter.CashSumBack,CurrentDomain * BILLMULTIPLE);
					//------------------------------------------------------
					sysVPMission.payInBillFlag = 1;
					sysVPMission.payInBillMoney += CurrentDomain * BILLMULTIPLE;
					//reset the trade timer
			        //sysVPMission.sTimer1 = SystemParameter.tradeTime;
					//======================================================
					//sysITLMission.restartTime = ITL_TIME_RESTART; //reset the timer!
					memset(sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));
		            sysITLMission.billHoldingFlag = 0;
					sysITLMission.billHoldingValue = 0;
					/*
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
					*/
					return 1;
				}
				else
				{
					//VP_GameKeyPoll( 200 );
				}
            }
			if( sysVPMission.billSTimer == 0 )
			{
                //钞箱满，钞箱压钞后就能时，还可以认到纸币;by gzz 20111017
			    //检测纸币器钞箱是否已满;by gzz 20111014 
				WaitForWork(2000,NULL);
                MDB_Bill_Stacker( &sysMDBMission.billBuf[0], &sysMDBMission.billBuf[1] ); 
            	if(sysMDBMission.billIsFull==1)
 			    {
				    CurrentDomain = sysITLMission.billHoldingValue;
					Trace( "\n Get in Cash Ok" );
					Trace( "\n CurrentDomain = %d",CurrentDomain );		
					CurrentPayed += CurrentDomain * BILLMULTIPLE;	
					//TradeCounter.TrueCurrencySum += CurrentDomain * BILLMULTIPLE;
					//TradeCounter.CashSum += CurrentDomain * BILLMULTIPLE;
					//TradeCounter.CashSumBack += CurrentDomain * BILLMULTIPLE;//区间
					SellAccumulateMoney(&TradeCounter.CashSum,CurrentDomain * BILLMULTIPLE);
					SellAccumulateMoney(&TradeCounter.CashSumBack,CurrentDomain * BILLMULTIPLE);
					//------------------------------------------------------
					sysVPMission.payInBillFlag = 1;
					sysVPMission.payInBillMoney += CurrentDomain * BILLMULTIPLE;
					//reset the trade timer
			        //sysVPMission.sTimer1 = SystemParameter.tradeTime;
					//======================================================
					//sysITLMission.restartTime = ITL_TIME_RESTART; //reset the timer!
					memset(sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));
		            sysITLMission.billHoldingFlag = 0;
					sysITLMission.billHoldingValue = 0;
					/*
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
					*/
					return 1;
				}
				else
				{
					//...	
					//调整顺序，billHoldingFlag==0放在sysVPMission之上;by gzz 20110721
					sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
					sysITLMission.billHoldingFlag = 0;
					sysITLMission.billHoldingValue = 0;
					//Need better solutoin. 
					//Maybe cheated by man
		            //CurrentPayed += sysITLMission.billHoldingValue;
					//-----------------------------------------------------
				    sysVPMission.escrowOutFlag = 1;				    
					//取消注释,允许对pc发送压钞不成功信息;by gzz 20110721
					if( sysVPMission.escrowOutFlag == 1 )
				    {
					    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
						sysVPMission.escrowOutFlag = 0;
				        sysVPMission.escrowMoney = 0;     
					}
					
				    //=====================================================
					//DisplayCurrentMoney( CurrentPayed );
                }
			}
        }
        else
		{
			//...	
			//调整顺序，billHoldingFlag==0放在sysVPMission之上;by gzz 20110721
			sysVPMission.escrowMoney = sysITLMission.billHoldingValue;	
			sysITLMission.billHoldingFlag = 0;
			sysITLMission.billHoldingValue = 0;
            //Need better solutoin. 
			//Maybe cheated by man
            //CurrentPayed += sysITLMission.billHoldingValue;
			//-----------------------------------------------------
			sysVPMission.escrowOutFlag = 1;			
			//取消注释,允许对pc发送压钞不成功信息;by gzz 20110721
			if( sysVPMission.escrowOutFlag == 1 )
		    {
			    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
				sysVPMission.escrowOutFlag = 0;
		        sysVPMission.escrowMoney = 0;     
			}
			
			//=====================================================
			//DisplayCurrentMoney( CurrentPayed );
		}
    }
	return 0;

}

u_char VMC_PC_POLL( void )
{
    u_char xdata i = 0;
    u_char xdata j = 0;
    u_char xdata flag = 0;
	u_char xdata returnflag = 0;
	//static u_char xdata gocErrTimes = 0;
	u_int  xdata MoneyBuf = 0;
	u_int  xdata CurrentDomain = 0;
	u_char xdata mission = 0;
    
    #ifndef MACHINE_SET_VMC_PC
	   return VP_ERR_NULL;
	#endif

	//0.
    if( sysVPMission.VPDevCtr & 0x80 )
	{
	    sysVPMission.VPDevCtr &= 0x7F;
	    flag = VPMission_Poll();
		sysVPMission.msTimer1 = VP_TIME_POLL;
        if( sysVPMission.resetCmd == 1 )
        {
        	//VPMission_Init();
            //sysVPMission.resetCmd = 0;
 		    returnflag = 1;
        }
	}

    //1.
    if( sysVPMission.msTimer1 == 0 )
	{
		//if( ( sysVPMission.vendCmd == 0 )&&(sysVPMission.changeCmd == 0)&&(sysVPMission.costCmd == 0)&&(sysVPMission.returnCmd == 0) )
		{
    		flag = VPMission_Poll();
			//sysVPMission.msTimer1 = VP_TIME_POLL;
	        if( sysVPMission.resetCmd == 1 )
	        {
	        	//VPMission_Init();
	            //sysVPMission.resetCmd = 0;
	 		    returnflag = 1;
	        }
		}
		
    }
    
    //2.
	if( sysVPMission.requestFlag == 1 )
	{
	    VPMission_Request(1);
	    sysVPMission.requestFlag = 0;	
	}
    //3.
    if( sysVPMission.vendCmd == 1 )
	{
		OutGoods(1);		
		if( CurrentPayed > 0 )
        {
            UpdateSelLed_Trade();
			DisplayCurrentMoney( CurrentPayed );
			if( (HardWareErr==0x00)&&((CurrentPayed+sysITLMission.billHoldingValue)<SystemParameter.BanknoteMax) )
			{
				EnableBillDev();
			}
			sysVPMission.sTimer1 = SystemParameter.tradeTime;
		}
		sysVPMission.vendCmd = 0;
		
		//WaitForWork( 10000, NULL );
	}
	//4.    
    if( (sysVPMission.changeCmd == 1)  )
    {
        CurrentDispenseCoin = sysVPMission.changeMoney;	
        sysVPMission.changeMoney = 0;
        DisableBillDev();
		//DisplayStr( 0, __LINE0__ , 1 , "3", 1 );
		//WaitForWork( 2000, NULL );  //500-50-100-200
		DispenseCoin(1);
        SaveTradeParam();  //2011.6.10 added: Update the sale data in flash!
        UpdateSelectionLed_GoodsSta();        
		//
		if( sysITLMission.billHoldingFlag == 1 )
		{
			MoneyBuf = CurrentPayed + sysITLMission.billHoldingValue;
		}
		else
		{
			MoneyBuf = CurrentPayed;
		}
		if( MoneyBuf > 0 )
        {
            UpdateSelLed_Trade();
			DisplayCurrentMoney( CurrentPayed );
			if( (HardWareErr==0x00)&&((CurrentPayed+sysITLMission.billHoldingValue)<SystemParameter.BanknoteMax) )
			{
				EnableBillDev();
			}
			sysVPMission.sTimer1 = SystemParameter.tradeTime;
		}
		if( (HardWareErr==0x00)&&(MoneyBuf<SystemParameter.BanknoteMax) )		
        {
        	EnableBillDev();		
        }
		sysVPMission.changeCmd = 0;
    }
	
	//4.接收扣钱命令，进行扣钱操作 	
	if( (sysVPMission.costCmd == 1))
    {	        
		//在扣款之前, stack the holding bill
		if( ( sysITLMission.billHoldingFlag == 1 )&&(sysVPMission.costMoney > CurrentPayed) )
		{
		    
	        flag = MDB_Bill_EscrowCtr( 1 );
			if(flag==0)
			{
			    sysVPMission.billSTimer = TIME_BILL_STACK;
			    while( sysVPMission.billSTimer )
				{
	                mission = MDB_Bill_IfStacked( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
					if( mission == 1 )
					{
					    CurrentDomain = sysITLMission.billHoldingValue;
						Trace( "\n Get in Cash Ok" );
						//CurrentDomain = GetCash();
						Trace( "\n CurrentDomain = %d",CurrentDomain );		
						CurrentPayed += CurrentDomain * BILLMULTIPLE;	
						//TradeCounter.TrueCurrencySum += CurrentDomain * BILLMULTIPLE;
						//TradeCounter.CashSum += CurrentDomain * BILLMULTIPLE;
						//TradeCounter.CashSumBack += CurrentDomain * BILLMULTIPLE;//区间
						SellAccumulateMoney(&TradeCounter.CashSum,CurrentDomain * BILLMULTIPLE);
						SellAccumulateMoney(&TradeCounter.CashSumBack,CurrentDomain * BILLMULTIPLE);
						//------------------------------------------------------
						sysVPMission.payInBillFlag = 1;
						sysVPMission.payInBillMoney += CurrentDomain * BILLMULTIPLE;
						//reset the trade timer
				        //sysVPMission.sTimer1 = SystemParameter.tradeTime;
						//======================================================
						//sysITLMission.restartTime = ITL_TIME_RESTART; //reset the timer!
						memset(sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));
						sysMDBMission.billBufLen=0;
						sysMDBMission.billType=0;
						sysMDBMission.billStock=0;
			            sysITLMission.billHoldingFlag = 0;
						sysITLMission.billHoldingValue = 0;
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
                        DisplayCurrentMoney( CurrentPayed ); 
	                    break;//压钞成功后，直接跳出时间循环等待;by gzz 20110721
					}
					else
					{
						VP_GameKeyPoll( 200 );
					}
	            }
				/*
				if( sysVPMission.billSTimer == 0 )
				{
                    //钞箱满，钞箱压钞后就能时，还可以认到纸币;by gzz 20111017
				    //检测纸币器钞箱是否已满;by gzz 20111014 
					WaitForWork(2000,NULL);
	                MDB_Bill_Stacker( &sysMDBMission.billBuf[0], &sysMDBMission.billBuf[1] ); 
	            	if(sysMDBMission.billIsFull==1)
					{
					    CurrentDomain = sysITLMission.billHoldingValue;
						Trace( "\n Get in Cash Ok" );
						//CurrentDomain = GetCash();
						Trace( "\n CurrentDomain = %d",CurrentDomain );		
						CurrentPayed += CurrentDomain * BILLMULTIPLE;	
						TradeCounter.TrueCurrencySum += CurrentDomain * BILLMULTIPLE;
						TradeCounter.CashSum += CurrentDomain * BILLMULTIPLE;
						TradeCounter.CashSumBack += CurrentDomain * BILLMULTIPLE;//区间
						//------------------------------------------------------
						sysVPMission.payInBillFlag = 1;
						sysVPMission.payInBillMoney += CurrentDomain * BILLMULTIPLE;
						//reset the trade timer
				        //sysVPMission.sTimer1 = SystemParameter.tradeTime;
						//======================================================
						//sysITLMission.restartTime = ITL_TIME_RESTART; //reset the timer!
						memset(sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));
						sysMDBMission.billBufLen=0;
						sysMDBMission.billType=0;
						sysMDBMission.billStock=0;
			            sysITLMission.billHoldingFlag = 0;
						sysITLMission.billHoldingValue = 0;
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
                        DisplayCurrentMoney( CurrentPayed ); 	                    
					}
					else
					{
						//...	
						//调整顺序，billHoldingFlag==0放在sysVPMission之上;by gzz 20110721
						sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
						sysITLMission.billHoldingFlag = 0;
						sysITLMission.billHoldingValue = 0;
						//Need better solutoin. 
						//Maybe cheated by man
			            //CurrentPayed += sysITLMission.billHoldingValue;
						//-----------------------------------------------------
					    sysVPMission.escrowOutFlag = 1;					    
						if( sysVPMission.escrowOutFlag == 1 )
					    {
						    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
							sysVPMission.escrowOutFlag = 0;
					        sysVPMission.escrowMoney = 0;     
						}
					    //=====================================================
	                    DisplayCurrentMoney( CurrentPayed );					
						/*					
						if( GoodsWaySetVal[CurrentStockCode].Price > CurrentPayed )
						{
							return 1;
						}	
	                    * /
                    }
				}*/
	        }
	        else
			{
				/*
				//...
				//调整顺序，billHoldingFlag==0放在sysVPMission之上;by gzz 20110721
				sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
				sysITLMission.billHoldingFlag = 0;
				sysITLMission.billHoldingValue = 0;
				//Need better solutoin. 
				//Maybe cheated by man
	            //CurrentPayed += sysITLMission.billHoldingValue;
				//-----------------------------------------------------
				sysVPMission.escrowOutFlag = 1;				
				if( sysVPMission.escrowOutFlag == 1 )
			    {
				    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
					sysVPMission.escrowOutFlag = 0;
			        sysVPMission.escrowMoney = 0;     
				}
				//=====================================================
                DisplayCurrentMoney( CurrentPayed );				
				/ *				
				if( GoodsWaySetVal[CurrentStockCode].Price > CurrentPayed )
				{
					return 1;
				}
                */
			}
	    }
	    else
		{
			//若此时还有插币，等压钞结束后再判断;by gzz 20110429 
			//WaitForWork( NULL, 1000 );
	        /*
		    if((SystemParameter.BillNo==1)&&(sysITLMission.ITLSet==1))
		    {	   
		       if(sysITLMission.billStaCtr & ITL_BILL_READ)
		       {	
		                
		          return 1;          
		       }
		    }
	        */ 
		}
		//
		{
	        sysITLMission.billStaCtr = 0;
	        sysITLMission.billSta = 0;
			/*
	        WaitForWork( 300, NULL );
			MDB_Bill_EscrowCtr( 0 );
			*/
	    	//sysITLMission.billStaCtr |= ITL_BILL_VEDNING;
		}

		if(sysVPMission.costMoney > CurrentPayed)
 	    {
			 sysVPMission.costMoney = 0;
		}
		else
		{
			CurrentPayed = CurrentPayed - sysVPMission.costMoney;  
		}
		
		VPMission_Cost_RPT(sysVPMission.costDev, sysVPMission.costMoney, sysVPMission.costType);

        //---------------------------------------------------        
        if( CurrentPayed >= 1 )
		{
			sysVPMission.sTimer1 = SystemParameter.tradeTime;
		} 
		if( CurrentPayed < SystemParameter.BanknoteMax )
        {
        	EnableBillDev();		
        }
		sysVPMission.inBillMoney = CurrentPayed;
        //显示剩余的金额
        DisplayCurrentMoney( CurrentPayed );
		sysVPMission.costCmd = 0;	
    }
	    

	//5.定时重启纸币器,硬币器;20110510  
    #ifdef MACHINE_SET_MDB
    if(( sysITLMission.restartTime == 0)&&( HardWareErr== 0))
	{
	    sysITLMission.restartTime = ITL_TIME_RESTART; 
	    if( (SystemParameter.BillNo==1)&&(sysITLMission.ITLSet==1))  
	    {
			MDBMission_Bill_Enable();
        }
        WaitForWork( 500 , NULL );
        MDBMission_Coin_Enable();        	
    }
    #endif

	//6.
    if( IfReturnKeyOn()||(returnflag==1)||( sysVPMission.sTimer1 == 0 ) )
	{	
		//-----------------------------------------------------------------------------------
		sysVPMission.returnCmd=1;
	    //Andy added 2011.4.14.Rject the holding bill.
	    if( ((SystemParameter.RefundPermission==1)||(sysVPMission.boughtFlag==1))&&( sysITLMission.billHoldingFlag == 1 ) )
		{
			flag = MDB_Bill_EscrowCtr(0);
			//调整顺序，flag==0放在sysVPMission之上;by gzz 20110721
			//=====================================================
			if( flag == 0 )
			{
				sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
				sysITLMission.billHoldingFlag = 0;
			    sysITLMission.billHoldingValue = 0;
			}
			else
			{
				sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
				//... Need more discussion for better soluton!
                sysITLMission.billHoldingFlag = 0;
			    sysITLMission.billHoldingValue = 0;
			}			
			//-----------------------------------------------------
			sysVPMission.escrowOutFlag = 1;						
			if( sysVPMission.escrowOutFlag == 1 )
		    {
			    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
				sysVPMission.escrowOutFlag = 0;
		        sysVPMission.escrowMoney = 0;     
			}
			
			//.Send the reject message to PC
			if( CurrentPayed > 0 )
            {
                //UpdateSelLed_Trade();
				//DisplayCurrentMoney( CurrentPayed/*+sysITLMission.billHoldingValue* / );
                //by gzz 20110416
				//DisableBillDev();
                //EnableBillDev();
                //End_by gzz 20110416		
            }
            else
            {
                //by gzz 20110416
				//DisableBillDev();
                //EnableBillDev();
                //End_by gzz 20110416	
                UpdateSelectionLed_GoodsSta(); 
            }
		}
		if( ((SystemParameter.RefundPermission==1)||(sysVPMission.boughtFlag==1))&&(CurrentPayed>0) )
		{
		    CurrentDispenseCoin = CurrentPayed;	
			CurrentPayed = 0;
			//DisplayStr( 0, __LINE0__ , 1 , "5", 1 );
			//WaitForWork( 2000, NULL );  //500-50-100-200
			DispenseCoin(0);
			DisableBillDev();
			//DisplayStr( 0, __LINE0__ , 1 , "4", 1 );
			//WaitForWork( 2000, NULL );  //500-50-100-200
            SaveTradeParam();  //2011.6.10 added: Update the sale data in flash!
            UpdateSelectionLed_GoodsSta();
            EnableBillDev(); 
			memset(&sysMDBMission.coinBuf,0,sizeof(sysMDBMission.coinBuf));
	        memset(&sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));            
		}
		if( sysVPMission.boughtFlag==1 )
		{
			sysVPMission.boughtFlag = 0;
		}
		if( sysITLMission.billHoldingFlag != 1 )
		{
			MDB_Bill_EscrowCtr(0);  //Added to reject the unknown holding bill
		}
		sysVPMission.returnCmd=0;
	}
	if(returnflag==1)
	{
		return 1;
	}
	
	/*
	//7.
    if(  sysVPMission.comErrNum >= VP_TIME_OUNT_NUM )
    {    	
	    //HardWareErr |= 0x2000;
        MDB_Bill_EscrowCtr(0);  //Added 2011.7.19
	    if( ( CurrentPayed > 0 ) )
	    {
			CurrentDispenseCoin = CurrentPayed;	
			CurrentPayed = 0;
	        DisableBillDev();
			DispenseCoin();
            SaveTradeParam();  //2011.6.10 added: Update the sale data in flash!
	        UpdateSelectionLed_GoodsSta();
	    }
		return 2;
    }
	else
	{
		//HardWareErr &= 0xDFFF;
	}
	*/
	/*
	//8.
	if( (SystemParameter.busCardOn==0x01)&&( DeviceStatus.BusCard!=0x00 ) )
	{
		if( sysVPMission.VPDevCtr & 0x01 )
    	{
		    if( sysVPMission.sTimerBC >= VPM_DEVUPDATE_BUSCARD ) 
			{
				sysVPMission.sTimerBC = 0x00;
				BCMission_Inq();
			}
		}
		else
		{
			sysVPMission.VPDevCtr |= 0x01;
		}
	}
	*/
	//9.
	/*
	for( i=0; i<GOODSTYPEMAX; i++ )
	{
	  //if( sysGoodsMatrix[i].NextColumn != GOODS_MATRIX_NONE )
		if( (sysGoodsMatrix[i].NextColumn != GOODS_MATRIX_NONE)&&(sysGoodsMatrix[i].Price != 0) )
		{
			break;
		}
	}
	if( i >= GOODSTYPEMAX )
	*/
	if(isEmpty_Goods()==0)
	{
		HardWareErr |= 0x0080;
	}
	else
	{
	    HardWareErr &= 0xFF7F;	
	}
    //10.
	VP_UpdateChnagerStatus();
    //11.
    if(IfGameKeyOn())
	{
		VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
        //Beep();
	}
    //12.
	//if( (SystemParameter.GOCOn==0x01)&&( DeviceStatus.GOCStatus !=0x00 )&&(gocErrTimes<10) )
    if( (SystemParameter.GOCOn==0x01)&&( DeviceStatus.GOCStatus !=0x00 ) )
	{
		if( sysVPMission.VPDevCtr & 0x10 )
    	{
		    if( sysVPMission.sTimerGOC >= VPM_DEVUPDATE_GOC ) 
			{
                //--------------------------------------------
                //出货模块卡货时，每隔一段时间，予以检测，
                //如果货物已经掉下，予以恢复;by cq 20110815               
                /*
				sysVPMission.sTimerGOC = 0x00;
				if( ChannelTask( 1, CHANNEL_QUERY ) == 2 )
				{
					//执行命令超时,应是主板与货道驱动板通讯故障
					DeviceStatus.Driver64 |= 0x01;
					HardWareErr |= 0x0040;	
				}
                DisableBillDev();
                
				if( DeviceStatus.GOCStatus == 0x00 )
				{
				    gocErrTimes = 0x00;	
				}
				else if( gocErrTimes < 10 )
				{
					gocErrTimes++;	
				}
				*/
                sysVPMission.sTimerGOC = 0x00;
				
				sysVPMission.GOCTestFlag = 1;//cq

				if( ChannelTask( 1, CHANNEL_QUERY ) == 2 )
				{
					//执行命令超时,应是主板与货道驱动板通讯故障
					DeviceStatus.Driver64 |= 0x01;
					HardWareErr |= 0x0040;	
				}
				
				sysVPMission.GOCTestFlag = 0;//cq

				if( ChannelTask( 1, CHANNEL_QUERY ) == 2 )
				{
					//执行命令超时,应是主板与货道驱动板通讯故障
					DeviceStatus.Driver64 |= 0x01;
					HardWareErr |= 0x0040;	
				}
				
                //DisableBillDev();
                //============================================ 
				
			}
		}
		else
		{
			sysVPMission.VPDevCtr |= 0x10;
		}
	}
	/*
	else if( DeviceStatus.GOCStatus ==0x00 )
	{
		gocErrTimes = 0;
	}
	*/
    //13.检测到闭管没有关紧，进行重复检测;by gzz 20110827	
	if( sysVPMission.sTimertubeRemoved >= VPM_DEVUPDATE_TUBEREMOVE ) 
	{
	    sysVPMission.sTimertubeRemoved = 0x00;
		MDB_Coin_EXP_TubeRemoved();        
	}
	//14.
	//加快按键反应速度;by gzz 20110721  
	if(GetKeyOperStatus())
    {
		MaintFunction();
	}
	//16.
	if(sysVPMission.billCoinEnable == 0)
	{
		//DisplayStr( 0, __LINE0__ , 1 , "1", 1 );
		//WaitForWork( 2000, NULL );  //500-50-100-200	
		DisableBillDev();
		//DisplayStr( 0, __LINE0__ , 1 , "5", 1 );
		//WaitForWork( 2000, NULL );  //500-50-100-200
	}
	/*
	if(sysVPMission.ErrFlag2 == 1)
	{
		DisplayStr( 0, __LINE0__ , 1 , "2", 1 );
		WaitForWork( 2000, NULL );  //500-50-100-200
		if( (sysVPMission.billCoinEnable == 1)&&( sysMDBMission.billIsEnable == 0 ) )
		{
			DisplayStr( 0, __LINE0__ , 1 , "3", 1 );
			WaitForWork( 2000, NULL );  //500-50-100-200
			EnableBillDev();
		}
	}*/
	//17.
	if(sysVPMission.offLineFlag==1)
	{
    	HardWareErr |= 0x4000;
    }
	else
    {
		HardWareErr &= 0xBFFF;
    }
	//if(sysVPMission.ErrFlag2 == 1)
	//{
	//	flag = MDB_Coin_IfDeposited( sysMDBMission.coinBuf, &sysMDBMission.coinBufLen, &sysMDBMission.coinType, &sysMDBMission.coinStock );
	//}
	
	//离线售卖时，打开纸币器硬币器
	//if( (TradeCounter.offLineFlag == 1)&&(sysVPMission.billCoinEnable == 0) )
	//{
	//	sysVPMission.billCoinEnable = 1;
	//}
	//16.
	//-------------------------------------------------
    //Feed the watch dog
	FeedWatchDog();	
    //=================================================		
    return 0;
}


u_char VP_TradeIdle( void )
{
    u_char xdata i = 0;
    u_char xdata j = 0;
    u_char xdata flag = 0;
    u_int  xdata CurrentDomain = 0;
    u_char xdata mission = 0;
	u_char xdata returnflag = 0;
	u_int  xdata MoneyBuf = 0;
	
    
    #ifndef MACHINE_SET_VMC_PC
	   return VP_ERR_NULL;
	#endif

    //1.
    if( sysVPMission.msTimer1 == 0 )
	{
		//if( ( sysVPMission.vendCmd == 0 )&&(sysVPMission.changeCmd == 0)&&(sysVPMission.costCmd == 0)&&(sysVPMission.returnCmd == 0) )
		{
	    	flag = VPMission_Poll();
			//sysVPMission.msTimer1 = VP_TIME_POLL;
	        if( sysVPMission.resetCmd == 1 )
	        {
	        	//VPMission_Init();
	            //sysVPMission.resetCmd = 0;
	 		    returnflag = 1;
	        }
		}
		
    }   
	//1.1.
	if(sysVPMission.offLineFlag==1)
	{
    	HardWareErr |= 0x4000;
		returnflag = 1;
    }
	else
    {
		HardWareErr &= 0xBFFF;
    }
	//2.
    if( sysVPMission.vendCmd == 1 )
	{		
		OutGoods(1);		
		//sysVPMission.dspCtr |= 0x02;
		if( CurrentPayed >= 1 )
		{
			sysVPMission.sTimer1 = SystemParameter.tradeTime;
		}
		if( CurrentPayed < SystemParameter.BanknoteMax )
        {
        	EnableBillDev();		
        }
		//显示剩余的金额
        DisplayCurrentMoney( CurrentPayed );
		sysVPMission.inBillMoney = CurrentPayed;
		sysVPMission.vendCmd = 0;
		UpdateSelLed_Trade();
		//WaitForWork( 300, NULL );
	}
	//3.
    if( (sysVPMission.changeCmd == 1)  )
    {
        CurrentDispenseCoin = sysVPMission.changeMoney;	
        sysVPMission.changeMoney = 0;
        DisableBillDev();
		DispenseCoin(1);
        SaveTradeParam();  //2011.6.10 added: Update the sale data in flash!
        //UpdateSelectionLed_GoodsSta();        
		//
		
        //sysVPMission.dspCtr |= 0x02;
		if( sysITLMission.billHoldingFlag == 1 )
		{
			MoneyBuf = CurrentPayed + sysITLMission.billHoldingValue;
		}
		else
		{
			MoneyBuf = CurrentPayed;
		}
		if( MoneyBuf > 0 )
		{
			sysVPMission.sTimer1 = SystemParameter.tradeTime;
		}
		if( MoneyBuf < SystemParameter.BanknoteMax )
        {
        	EnableBillDev();		
        }
		//显示剩余的金额
        DisplayCurrentMoney( CurrentPayed );
		sysVPMission.changeCmd = 0;
		UpdateSelLed_Trade();
    }

    //4.接收扣钱命令，进行扣钱操作  
	if( (sysVPMission.costCmd == 1)  )
    {	        
		//在扣款之前, stack the holding bill
		if( ( sysITLMission.billHoldingFlag == 1 )&&(sysVPMission.costMoney > CurrentPayed) )
		{
		    
	        flag = MDB_Bill_EscrowCtr( 1 );
			if(flag==0)
			{
			    sysVPMission.billSTimer = TIME_BILL_STACK;
			    while( sysVPMission.billSTimer )
				{
	                mission = MDB_Bill_IfStacked( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
					if( mission == 1 )
					{
					    CurrentDomain = sysITLMission.billHoldingValue;
						Trace( "\n Get in Cash Ok" );
						//CurrentDomain = GetCash();
						Trace( "\n CurrentDomain = %d",CurrentDomain );		
						CurrentPayed += CurrentDomain * BILLMULTIPLE;	
						//TradeCounter.TrueCurrencySum += CurrentDomain * BILLMULTIPLE;
						//TradeCounter.CashSum += CurrentDomain * BILLMULTIPLE;
						//TradeCounter.CashSumBack += CurrentDomain * BILLMULTIPLE;//区间
						SellAccumulateMoney(&TradeCounter.CashSum,CurrentDomain * BILLMULTIPLE);
						SellAccumulateMoney(&TradeCounter.CashSumBack,CurrentDomain * BILLMULTIPLE);
						//------------------------------------------------------
						sysVPMission.payInBillFlag = 1;
						sysVPMission.payInBillMoney += CurrentDomain * BILLMULTIPLE;
						//reset the trade timer
				        //sysVPMission.sTimer1 = SystemParameter.tradeTime;
						//======================================================
						//sysITLMission.restartTime = ITL_TIME_RESTART; //reset the timer!
						memset(sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));
						sysMDBMission.billBufLen=0;
						sysMDBMission.billType=0;
						sysMDBMission.billStock=0;
			            sysITLMission.billHoldingFlag = 0;
						sysITLMission.billHoldingValue = 0;
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
                        DisplayCurrentMoney( CurrentPayed ); 
	                    break;//压钞成功后，直接跳出时间循环等待;by gzz 20110721
					}
					else
					{
						VP_GameKeyPoll( 200 );
					}
	            }
				if( sysVPMission.billSTimer == 0 )
				{
                    //钞箱满，钞箱压钞后就能时，还可以认到纸币;by gzz 20111017
				    //检测纸币器钞箱是否已满;by gzz 20111014 
					WaitForWork(2000,NULL);
	                MDB_Bill_Stacker( &sysMDBMission.billBuf[0], &sysMDBMission.billBuf[1] ); 
	            	if(sysMDBMission.billIsFull==1)
					{
					    CurrentDomain = sysITLMission.billHoldingValue;
						Trace( "\n Get in Cash Ok" );
						//CurrentDomain = GetCash();
						Trace( "\n CurrentDomain = %d",CurrentDomain );		
						CurrentPayed += CurrentDomain * BILLMULTIPLE;	
						//TradeCounter.TrueCurrencySum += CurrentDomain * BILLMULTIPLE;
						//TradeCounter.CashSum += CurrentDomain * BILLMULTIPLE;
						//TradeCounter.CashSumBack += CurrentDomain * BILLMULTIPLE;//区间
						SellAccumulateMoney(&TradeCounter.CashSum,CurrentDomain * BILLMULTIPLE);
						SellAccumulateMoney(&TradeCounter.CashSumBack,CurrentDomain * BILLMULTIPLE);
						//------------------------------------------------------
						sysVPMission.payInBillFlag = 1;
						sysVPMission.payInBillMoney += CurrentDomain * BILLMULTIPLE;
						//reset the trade timer
				        //sysVPMission.sTimer1 = SystemParameter.tradeTime;
						//======================================================
						//sysITLMission.restartTime = ITL_TIME_RESTART; //reset the timer!
						memset(sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));
						sysMDBMission.billBufLen=0;
						sysMDBMission.billType=0;
						sysMDBMission.billStock=0;
			            sysITLMission.billHoldingFlag = 0;
						sysITLMission.billHoldingValue = 0;
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
                        DisplayCurrentMoney( CurrentPayed ); 	                    
					}
					else
					{
						//...	
						//调整顺序，billHoldingFlag==0放在sysVPMission之上;by gzz 20110721
						sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
						sysITLMission.billHoldingFlag = 0;
						sysITLMission.billHoldingValue = 0;
						//Need better solutoin. 
						//Maybe cheated by man
			            //CurrentPayed += sysITLMission.billHoldingValue;
						//-----------------------------------------------------
					    sysVPMission.escrowOutFlag = 1;					    
						if( sysVPMission.escrowOutFlag == 1 )
					    {
						    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
							sysVPMission.escrowOutFlag = 0;
					        sysVPMission.escrowMoney = 0;     
						}
					    //=====================================================
	                    DisplayCurrentMoney( CurrentPayed );					
						/*					
						if( GoodsWaySetVal[CurrentStockCode].Price > CurrentPayed )
						{
							return 1;
						}	
	                    */
                    }
				}
	        }
	        else
			{
				//...
				//调整顺序，billHoldingFlag==0放在sysVPMission之上;by gzz 20110721
				sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
				sysITLMission.billHoldingFlag = 0;
				sysITLMission.billHoldingValue = 0;
				//Need better solutoin. 
				//Maybe cheated by man
	            //CurrentPayed += sysITLMission.billHoldingValue;
				//-----------------------------------------------------
				sysVPMission.escrowOutFlag = 1;				
				if( sysVPMission.escrowOutFlag == 1 )
			    {
				    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
					sysVPMission.escrowOutFlag = 0;
			        sysVPMission.escrowMoney = 0;     
				}
				//=====================================================
                DisplayCurrentMoney( CurrentPayed );				
				/*				
				if( GoodsWaySetVal[CurrentStockCode].Price > CurrentPayed )
				{
					return 1;
				}
                */
			}
	    }
	    else
		{
			//若此时还有插币，等压钞结束后再判断;by gzz 20110429 
			//WaitForWork( NULL, 1000 );
	        /*
		    if((SystemParameter.BillNo==1)&&(sysITLMission.ITLSet==1))
		    {	   
		       if(sysITLMission.billStaCtr & ITL_BILL_READ)
		       {	
		                
		          return 1;          
		       }
		    }
	        */ 
		}
		//
		{
	        sysITLMission.billStaCtr = 0;
	        sysITLMission.billSta = 0;
			/*
	        WaitForWork( 300, NULL );
			MDB_Bill_EscrowCtr( 0 );
			*/
	    	//sysITLMission.billStaCtr |= ITL_BILL_VEDNING;
		}

		if(sysVPMission.costMoney > CurrentPayed)
 	    {
			 sysVPMission.costMoney = 0;
		}
		else
		{
			CurrentPayed = CurrentPayed - sysVPMission.costMoney;  
		}
		
		VPMission_Cost_RPT(sysVPMission.costDev, sysVPMission.costMoney, sysVPMission.costType);

        //---------------------------------------------------        
        if( CurrentPayed >= 1 )
		{
			sysVPMission.sTimer1 = SystemParameter.tradeTime;
		} 
		if( CurrentPayed < SystemParameter.BanknoteMax )
        {
        	EnableBillDev();		
        }
		else
		{
			DisableBillDev();
		}
		sysVPMission.inBillMoney = CurrentPayed;
        //显示剩余的金额
        DisplayCurrentMoney( CurrentPayed );
		sysVPMission.costCmd = 0;
		UpdateSelLed_Trade();
    }

	/*
	//5.
	//if( sysVPMission.dspCtr & 0x02 )
	//{
	    //sysVPMission.dspCtr &= 0xfd;
		if( CurrentPayed > 0 )
        {
            UpdateSelLed_Trade();
			DisplayCurrentMoney( CurrentPayed );
			if( (HardWareErr==0x00)&&((CurrentPayed+sysITLMission.billHoldingValue)<SystemParameter.BanknoteMax) )
			{
				EnableBillDev();
			}
			//sysVPMission.sTimer1 = SystemParameter.tradeTime;
		}
	//}
	*/
	
	//6.
    if( sysVPMission.sTimer1 == 0 )
    {   
		//-----------------------------------------------------------------------------------
	    //Andy added 2011.4.14.Rject the holding bill.
	    //if( (SystemParameter.RefundPermission==1) && ( sysITLMission.billHoldingFlag == 1 ) )
		if( ((SystemParameter.RefundPermission==1)||(sysVPMission.boughtFlag==1))&&( sysITLMission.billHoldingFlag == 1 ) )
		{
		    
			flag = MDB_Bill_EscrowCtr(0);
			//调整顺序，flag==0放在sysVPMission之上;by gzz 20110721
			if( flag == 0 )
			{
				sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
				sysITLMission.billHoldingFlag = 0; 
			    sysITLMission.billHoldingValue = 0;
			}
			else
			{
				sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
				//... Need more discussion for better soluton!
                sysITLMission.billHoldingFlag = 0;
			    sysITLMission.billHoldingValue = 0;
			}
            //-----------------------------------------------------
			sysVPMission.escrowOutFlag = 1;			
            if( sysVPMission.escrowOutFlag == 1 )
		    {
			    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
				sysVPMission.escrowOutFlag = 0;
		        sysVPMission.escrowMoney = 0;     
			}
			//=====================================================
			//Send the reject mesage to pc
			if( CurrentPayed > 0 )
            {
                //UpdateSelLed_Trade();
				//DisplayCurrentMoney( CurrentPayed/*+sysITLMission.billHoldingValue* / );
                //by gzz 20110416
				//DisableBillDev();
                //EnableBillDev();
                //End_by gzz 20110416			
				
            }
            else
            {
                //by gzz 20110416
				//DisableBillDev();
                //EnableBillDev();
                //End_by gzz 20110416
                UpdateSelectionLed_GoodsSta(); 
            }
		}
		//if( (SystemParameter.RefundPermission==1) && (CurrentPayed>0) )
		if( ((SystemParameter.RefundPermission==1)||(sysVPMission.boughtFlag==1))&&(CurrentPayed>0) )
		{
		    CurrentDispenseCoin = CurrentPayed;	
			CurrentPayed = 0;
			//DisplayStr( 0, __LINE0__ , 1 , "6", 1 );
			//WaitForWork( 2000, NULL );  //500-50-100-200
			DispenseCoin(0);
			DisableBillDev();
            SaveTradeParam();  //2011.6.10 added: Update the sale data in flash!
            UpdateSelectionLed_GoodsSta();
            EnableBillDev(); 
			memset(&sysMDBMission.coinBuf,0,sizeof(sysMDBMission.coinBuf));
	        memset(&sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));            
		}
		//MDB_Bill_EscrowCtr(0);    //Added to reject the unknown holding bill
		if( sysVPMission.boughtFlag==1 )
		{
		    sysVPMission.boughtFlag = 0;
		}
		if( sysITLMission.billHoldingFlag != 1 )
		{
		    MDB_Bill_EscrowCtr(0);  //Added to reject the unknown holding bill
		}
    }
    //7.
	if( IfReturnKeyOn()||(returnflag == 1) )
	{		
	    //-----------------------------------------------------------------------------------
	    sysVPMission.returnCmd=1;
	    //Andy added 2011.4.14.Rject the holding bill.
	    //if( (SystemParameter.RefundPermission==1) && ( sysITLMission.billHoldingFlag == 1 ) )
		if( ((SystemParameter.RefundPermission==1)||(sysVPMission.boughtFlag==1))&&( sysITLMission.billHoldingFlag == 1 ) )
		{
		    
			flag = MDB_Bill_EscrowCtr(0);
			//调整顺序，flag==0放在sysVPMission之上;by gzz 20110721
			if( flag == 0 )
			{
				sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
				sysITLMission.billHoldingFlag = 0;
			    sysITLMission.billHoldingValue = 0;
			}
			else
			{
				//... Need more discussion for better soluton!
				sysVPMission.escrowMoney = sysITLMission.billHoldingValue;
                sysITLMission.billHoldingFlag = 0;
			    sysITLMission.billHoldingValue = 0;
			}
            //-----------------------------------------------------
			sysVPMission.escrowOutFlag = 1;			
			if( sysVPMission.escrowOutFlag == 1 )
		    {
			    VPMission_Payin_RPT( VP_DEV_ESCROWOUT, sysVPMission.escrowMoney );
				sysVPMission.escrowOutFlag = 0;
		        sysVPMission.escrowMoney = 0;     
			}
			//=====================================================
			//.Send the reject message to PC

			if( CurrentPayed > 0 )
            {
                //UpdateSelLed_Trade();
				//DisplayCurrentMoney( CurrentPayed/*+sysITLMission.billHoldingValue* / );
                //by gzz 20110416
				//DisableBillDev();
                //EnableBillDev();
                //End_by gzz 20110416	
			
				
            }
            else
            {
                //by gzz 20110416
				//DisableBillDev();
                //EnableBillDev();
                //End_by gzz 20110416	
                UpdateSelectionLed_GoodsSta(); 
            }
		}
		//if( (SystemParameter.RefundPermission==1) && (CurrentPayed>0))
		if( ((SystemParameter.RefundPermission==1)||(sysVPMission.boughtFlag==1))&&(CurrentPayed>0) )
		{
		    CurrentDispenseCoin = CurrentPayed;	
			CurrentPayed = 0;
			//DisplayStr( 0, __LINE0__ , 1 , "7", 1 );
			//WaitForWork( 2000, NULL );  //500-50-100-200
			DispenseCoin(0);
			DisableBillDev();
            SaveTradeParam();  //2011.6.10 added: Update the sale data in flash!
            UpdateSelectionLed_GoodsSta();
            EnableBillDev(); 
			memset(&sysMDBMission.coinBuf,0,sizeof(sysMDBMission.coinBuf));
	        memset(&sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));           
		}
 	    //MDB_Bill_EscrowCtr(0);  //Added to reject the unknown holding bill
		if( sysVPMission.boughtFlag==1 )
		{
		    sysVPMission.boughtFlag = 0;
		}
		if( sysITLMission.billHoldingFlag != 1 )
		{
		    MDB_Bill_EscrowCtr(0);  //Added to reject the unknown holding bill
		}
		sysVPMission.inBillMoney = 0;
		sysVPMission.returnCmd=0;

	}
	if(returnflag == 1)
	{
		return 1;
	}

	/*
	//8.
    if(  sysVPMission.comErrNum >= VP_TIME_OUNT_NUM )
    {
	    //HardWareErr |= 0x2000;
        MDB_Bill_EscrowCtr(0);  //Added 2011.7.19
	    if( ( CurrentPayed > 0 ) )
	    {
			CurrentDispenseCoin = CurrentPayed;	
		    CurrentPayed = 0;
	        DisableBillDev();
		    DispenseCoin();
            SaveTradeParam();  //2011.6.10 added: Update the sale data in flash!
	        UpdateSelectionLed_GoodsSta();
	    }
		return 2;
    }
	else
	{
		//HardWareErr &= 0xDFFF;
	}
	*/

	//9.
    if( IfGameKeyOn() )
	{
		VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
        //Beep();
	}

	

    //10.
	//-------------------------------------------------
    //Feed the watch dog
	FeedWatchDog();
    //=================================================
	return 0;
    
}
 
//单独作一个永远不会被调用到的节点
//u_char UNUSE_NODE();


BEGIN_FLOW( RunFlow )
	NODE(5)		SystemInit()
	//硬件全检,其它设备逻辑检测
	//初始化参数、设备工作状态(如允许查询状态等等)	
	NODE(15)	DeviceDefaultSetting()		
	//显示退出服务
	NODE(16)	DisplayOutServer()      R1(200)
	NODE(17)    CheckOperStatus()		R1(200)   //Added by Andy 2010.11.04 
	//初始化系统参数
	NODE(20)	SysInitParam()

	NODE(21)	IsTradeClear()
	
	//查询是否有货，全无货则为无货，无货返回1并退出服务
	//NODE(22)	CheckGoodsSumAndLoad()			R1(400)	
    //-----------------------------------------------------------------------
    NODE(23)    VPMission_Init()
    //=======================================================================

	//IDLE
	//return1=维护状态，　0=运行状态,运行相互变量清理工作也放在这里
	NODE(25)	CheckOperStatus()		R1(200)		
    //检查是否有查询命令
	//NODE(26) 	CheckQureyCmd()
	//判断是否有短信需要发送
	//NODE(27) 	CycleSendSMS()
	
    NODE(28)    VMC_PC_POLL()       R1(23)  R2(23)
	//查检是否有按键,有数字键输入，返回1,否则返回2
	NODE(29)	CheckKeyPress()		R1(42)  R2(30)
	
	//查询硬件状态值,硬件包括键盘、液晶屏，无故障返回0,有故障返回1并退出服务
	NODE(30)	CheckHardwareState()			R1(400)	
	//确定找零器中的硬币是否足
  //NODE(36) 	CheckCoinRemain()		R1(400)
	//查询是否有货，全无货则为无货，无货返回1并退出服务
  //NODE(40)	CheckGoodsSumAndLoad()			R1(400)	
    NODE(33)	GetKeyOperStatus()              R1(200)//加快按键反应速度;by gzz 20110721  

	//使能收币器
	NODE(35)	EnableBillDev()	
	//查检是否有币插入,有返回1,无返回0
	NODE(36)	IsInsertBill()			R1(47)			

	//查检是否有按键,有数字键输入，返回1,否则返回2
	NODE(41)	CheckKeyPress()		    R1(42)  R2(44)	

	//显示输入的货道，并判断输入是否正确，是否存在此货道,输入正确返回0,输入失败返回1
	NODE(42)	QueryDisplayInputCode()	R1(44)

	//判断货道是否正常并显示
	NODE(43) 	QueryIsGoodsWay()       R1(62)
			
	//显示服务情况，如是进入服务则判断是否有按键，有返回0，无返回1,
	NODE(44)	DispWelOrOutServ()		R1(25)	R2(47)
	

	//服务　
	NODE(45)    VP_TradeIdle()          R1(23)  R2(23)
	//查检是否有币插入,有返回1,无返回0
	NODE(46)	IsInsertBill()			R1(47)  //85
			    GOTONODE( 49 )
	//收取纸币并比较和显示，
	//当金额大于限额时返回1直接去选货，
	//否则返回0继续等待按键或插钱
	//第一次收钱失败时返回2
	NODE(47)	GetBill()		R1(45)	R2(45)   //R1_50_49
    NODE(48)    UpdateSelLed_Trade()
	//查检是否有数字按键,有返回0,没有但余额大于零返回1，否则返回2
	NODE(49)	IsInputGoodsNo()		R1(45)	R2(25)  //105

	//显示输入货道编号,包括输入正确与否判断和输入超时判断，输入正确返回0，输入错误返回1
	//输入CANNEL或输入超时返回2并退出服务
	NODE(50)	DisplayInputCode()	    R1( 45 )	R2(46 )

	//-------------------------------------------------------------------
	//Changed by Andy on 2010.12.23
	/*
	//判断货道是否正常，正常返回0显示货物信息,不正常返回1,货道已无货返回2
	NODE(55)	IsGoodsWay()	R1(60)		R2(50)
	//判断插入金额是否够买货，够返回0,不够返回1
	NODE(58)	IsMoneyEnough()		R1( 65 )//125
	//转到出货
			GOTONODE( 125 )
	*/
    //判断货道是否正常，正常返回0显示货物信息,不正常返回1,货道已无货返回2
	NODE(55)	IsGoodsWay()	 R1(46)		R2(46)
	//判断插入金额是否够买货，够返回0,不够返回1
	//NODE(58)	IsMoneyEnough()	 R1( 46 )       //125
	NODE(58)	IsMoneyEnough()	 R1(46) R2(47)  //125
	//转到出货
			    GOTONODE( 125 )
    //====================================================================
 	
		
	//显示货道故障，询问是否选其它货，选其它货返回0,不选其它货返回1，找零
	NODE(60)	GoodsWayErr()	R1(115)
	//户客选其它货
			GOTONODE(50)
	//---------------------------------------------------------------------
    NODE(62)    OutGoods(2)	 R1(46)	    R2(355)	
	        GOTONODE(127)     
	//=====================================================================
	
	//显示金额不足,要求再插币，返回1
	//提示是否继续插币，确定返回1,取消或超时则返回0
	NODE(65)	DispInputMoney()		R1( 46 )		//640	

	//查找同类型更便宜的卡,找到返回0,没有找到返回1
	NODE(100)	ExistAnyChoose()		R1(110)		R2(135)	//650
	//提示客户是否选择买其它的货，确定返回1并到50节点让用户重新输入，取消或超时返回0
	NODE(105)	CheaperOnePlz()			R1(50)		//660
	//看系统是否允许退币，允许退币且需要退币则退并返0，不允许则返回1反复提示,不需要退币返回2	
	NODE(110)	CheckChangePermit()		R1(65)		R2( 135 )		//700	
	//找零，配币失败返回1,出币错误返回2,出币正确返回0	
	NODE(115)	DispenseCoin(0)			R1(150)		R2(120)		//770
	NODE(120)	DispGetCoin()		//780
			GOTONODE(150)			
	//出货,正常出货返回0,出货失败但不能找零且还有其它货道可服务则设置货道故障并返回1,出货超时返回2
	NODE(125)	OutGoods(0)	R1(46)		R2(355)		//140
	//确定是否断续购买商品,返回0,不继续购买，进行找零；返回1则继续购买
	NODE(127)   ReBuy()		R1(46)
	//找零		
	NODE(130)	ReturnChange()			R1(150)		//810	
	//提示用户取货和找零
	NODE(135)	DispThx()			    R1(150)		//820
	//点亮出货灯,并返回1
  //NODE(140)	LightBlink()			R1(150)		//830		
	//结束售货流程并返回1	
	NODE(150)	SellCodeOver()					//990	
	//故障检查，都用标志位
	//查询硬件状态值,如有故障,再查硬件故障看是否有清除，无返回
	NODE(165)	TestHardWare()		    R1(25)	
	//判断货道库存和货道状态
	NODE(170)	CheckGoodsSumAndLoad()	
	NODE(172)	ScanIsSendAlarm()
	NODE(175)	EndMission()
			GOTONODE(25)	
	//维护流程
	NODE(200)	MaintFunction()
	//判断货道库存和货道状态
	NODE(205)	CheckGoodsSumAndLoad()  R1(400)	
			GOTONODE(23)                //25-23
	//货道故障，联系营运商，退出服务
	NODE(355)	AllGoodsWayError()		R1(172)		

	NODE(400)	OutofService()	
	NODE(403)	ShowOutofService()
	//使能收币器
	NODE(405)	EnableBillOutofService()
	//查检是否有币插入,有返回1,无返回0
	NODE(406)	IsInsertBill()			R2(25)	
	NODE(417)	ShowOutofService()
	NODE(418)	GetBill()
	GOTONODE( 25 )
	
END_FLOW()

void main()
{
	ZhkInit();
#ifdef _DEBUG_TRACE
	TraceInit();
#endif	
	RunFlow();

	//120419 by cq TotalSell
	/*
	InitAllCounter_1(); 
	while(1)
	{
		DisplayStr( 0, 0, 1, "  交易记录清零完毕", 18 );	
		DisplayStr( 0, 1, 1, "  可以重新下载软件", 18 );	
		WaitForWork( 1000, NULL );
	}
	*/
}

