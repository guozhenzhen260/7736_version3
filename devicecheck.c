#include "device.h"
#include "CommonFunction.h"
#include "devicecheck.h"
#include "scheduler.h"
#include "common.h"
#include "key.h"
#include <string.h>
#include "procotol.h"
#include "IOInput.h"
#include "mainflow.h"  
#include "timer.h" 
#include "debug.h"
#include "global.h"
#include "ShortMsg.h"
#include "communication.h"
#include "ITL.h"
#include "BusCard.h"
#include "VMC_PC.h"

#ifdef _CHINA_
	const struct StrAndLen code WMenuStr[] = 
	{	
		/*
		{ "1.纸币器状态: ", 14 },
		{ "2.硬币器状态: ", 14 },
	    //{ "3.#1找零器状态: ", 16 },	
		//{ "4.#2找零器状态: ", 16 },
		{ "3.五角找零: ", 12 },	
		{ "4.一元找零: ", 12 },
		//{ "5.#3找零器状态: ", 16 },	//111021	
		//{ "6.手机模块状态测试", 18 },//111021
		//{ "7.短信测试", 10 },//111021
		//{ "8.货道状态", 10 },//111021
        { "5.货道状态快速查询", 18 },
		{ "6.货道检测", 12 },
        { "7.货道测试结果", 16 },
        { "8.选货按钮检测", 16 },	
		{ "9.游戏按钮检测", 16 },
		{ "10. 退币按钮检测", 16 },
        { "11. 读卡器测试: ", 16 },		
		{ "12. 压缩机测试", 14 },
		{ "13. 展示灯测试", 14 },
		{ "14. 出货确认测试", 16 },//111017	by cq 出货确认
        { "15. 运行状态查询", 16 },
        */
        { "1.整机故障查看", 14 },
        { "2.出货测试", 10 },
        { "3.纸币器状态: ", 14 },
		{ "4.硬币器状态: ", 14 },
	    { "5.五角余额: ", 12 },	
		{ "6.一元余额: ", 12 },				
        //{ "7.货道测试结果", 14 },
        { "7.选货按钮检测", 14 },	
		{ "8.游戏按钮检测", 14 },
		{ "9.退币按钮检测", 16 },
        { "10. 出货确认测试", 16 },//111017	by cq 出货确认	
        { "11. 照明测试", 12 },
		{ "12. 压缩机测试", 14 },
		
        
	};

	const struct StrAndLen code DeviceStatusStr[] = 
	{
		{ "\xd5\xfd常", 4 },  //0	
    	{ "故障", 4 }, //1
		{ "无", 2 }, //2
		{ "不足", 4 },//3
	};
	
	const struct StrAndLen code GoodsWaysStatusStr[] = 
	{   
	    
		{ "未", 2 },       //未测试
		{ "\xd5\xfd", 2 }, //正常	
    	{ "故", 2 },       //故障	
		{ "缺", 2 },       //无货
		/*
		{ "未测试", 6 },       //未测试
		{ "\xd5\xfd常", 4 },   //正常	
    	{ "故障", 4 },         //故障	
		{ "缺货", 4 },         //无货
		*/
	};
#else
	const struct StrAndLen code WMenuStr[] = 
	{	
		{ "1.Bill acceptor:", 16 },
		{ "2.Coin acceptor:", 16 },
		{ "3.Change unit#1:", 16 },	
		{ "4.Change unit#2:", 16 },
		//{ "5.Change unit#3:", 16 },//111021
		//{ "6.Mobile status test", 20 },//111021
		//{ "7.SMS test", 10 },//111021
		//{ "8.Column status", 15 },//111021
        { "5.goods status", 14}
		{ "6.Column test", 13 },
        { "7.Col test result", 18 },
        { "8.Selection button", 19 },
		{ "9.Game button", 14 },
		{ "10.Return button", 16 },
        { "11.IC card:", 11 },		
		{ "12.compressor test", 18 },
		{ "13.light test", 13 },
		{ "14.GOC test", 11 },//111017
        { "15.Deal Status", 14 },
	
	};

	const struct StrAndLen code DeviceStatusStr[] = 
	{
		{ "OK", 2 },  //0	
	    { "Err", 3 }, //1
		{ "No", 2 }, //2
		{ "Litt", 4 },//3	
	};
	
	const struct StrAndLen code GoodsWaysStatusStr[] = 
	{
		{ "NO", 2 },//未用
		{ "OK", 2 },//正常	
	    { "Er", 2 }, //故障	
		{ "Em", 2 },//无货	
	};
#endif




u_char xdata m_iCheckDevice = 0;
u_char xdata hardWareErrShow[2]={0,0};

//用于显示确定或取消并返回用记选择值,返回值，确定返回0,取消返回1,超时返回2
u_char ChooseReturn( u_char *str )
{	
	u_char xdata MyKey = 0;

	ClearKey();	
	//显示字符	
	DisplayInfo( 0, 0 , str );
#ifdef _CHINA_
	DisplayStr( 0, 1, 1, "确定ENTER 取消CANCEL", 20 );
#else
	DisplayStr( 0, 1, 1, "    ENTER  CANCEL", 17 );
#endif
    
	//等待选择
	SgwSecTimer= 6;
	while( SgwSecTimer )
	{
		WaitForWork( 50, NULL ); 
		MyKey = GetKey();		
		switch ( MyKey )
	 	{
			case KEY_SUBMIT:				
				return 0 ;			
			case KEY_CANCEL:				
				return 1 ;					
			default:
				continue;
		}	
	}
	return 2;
}


//显示当前测试的各货道故障数,by gzz 20110303
void DispGoodsCheckValue()
{		
	u_char xdata i = 0;
	u_char xdata j = 0;
	u_char xdata z = 0;
					
	u_char  xdata LineNum = 2;  //选中行在屏幕中的行号		
	u_char  xdata key;
	u_char xdata  DispFlag = 1;
	u_char xdata SumColunm = 0;
	u_char xdata m_Index = 0;
    u_char xdata str[20];
	u_char xdata len = 0;

	//------------------------------------
	memset( str, 0, sizeof( str ) );
	len = sprintf( str, "累计测试次\xca\xfd: %u", WayCheck.testNum );
	DisplayStr( 0, 0, 1, str, len );
	ClearDisplayLine( 2 );
	//====================================
	for( m_Index = 0; m_Index < GOODSWAYNUM; m_Index ++ )
    {
       WayCheck.WayNo[m_Index] = InputGoodsWayList[m_Index].GoodsWayNo;
       //WayCheck[m_Index].BreakCount = 0;
    } 
	key = KEY_NULL;		
	ClearKey();
	DisplayCursorState( 0, 1, 1, 2, 1 );//设置当在第二行上且为整行光标状态
	LineNum = 2;
	for( i = 0; i < GOODSWAYNUM; i ++ )
	{
		if( WayCheck.WayNo[i] != 0 )
			SumColunm++;
		else
			break;
	}
	WaitForWork( 3000,NULL );
    i = 0;
    ClearKey();

	if( DispFlag  )
	{
		#ifdef _CHINA_		    
		    memset( str, 0, sizeof( str ) );
		    len = sprintf( str, "货道%02bu故障次\xca\xfd:%u", WayCheck.WayNo[i], WayCheck.BreakCount[i] );
	        DisplayStr( 0, 0, 1, str, len );
		    
			len = sprintf( str, "货道%02bu故障次\xca\xfd:%u", WayCheck.WayNo[i+1], WayCheck.BreakCount[i+1] );
	        DisplayStr( 0, 1, 1, str, len );					
		#else	
	        memset( str, 0, sizeof( str ) );
		    len = sprintf( str, "Column%02buErrNum:%u", WayCheck.WayNo[i], WayCheck.BreakCount[i] );
	        DisplayStr( 0, 0, 1, str, len );
		    
			len = sprintf( str, "Column%02buErrNum:%u", WayCheck.WayNo[i+1], WayCheck.BreakCount[i+1] );
	        DisplayStr( 0, 1, 1, str, len );				
		#endif			
			DisplayCursorState( 0, LineNum-1, 1, 2, 1 );
			DispFlag = 0;
    }
    ClearKey();
	

	while( 1 )    //超时或客户退出
	{        
		key = GetKey();
		//if( key == KEY_NULL ) continue;
		switch( key )		
		{
            case KEY_UP://上移                   	
			     if( LineNum == 2 )//选中标记上移
			     {			     
			     	DisplayCursorState( 0, 0, 1, 2, 1 );//第一行整光标
			     	LineNum = 1;
					if (i>1) i--;
			     }
			     else   //菜单上翻
			     {		
		 		 	if( i == 0 )
					{
						break;
				 	}
					else
					{			
					    i--;
					#ifdef _CHINA_	   
					    len = sprintf( str, "货道%02bu故障次\xca\xfd:%u", WayCheck.WayNo[i], WayCheck.BreakCount[i] );
				        DisplayStr( 0, 0, 1, str, len );
					    
						//如果下一行超出货道直，就不能显示了
                        if( (i+1) >= SumColunm )
                        {
                           ClearDisplayLine( 2 );
                        }
                        else
                        {
							len = sprintf( str, "货道%02bu故障次\xca\xfd:%u", WayCheck.WayNo[i+1], WayCheck.BreakCount[i+1] );
					        DisplayStr( 0, 1, 1, str, len );	
						}
					#else
						len = sprintf( str, "Column%02buErrNum:%u", WayCheck.WayNo[i], WayCheck.BreakCount[i] );
				        DisplayStr( 0, 0, 1, str, len );
					    
						//如果下一行超出货道直，就不能显示了
                        if( (i+1) >= SumColunm )
                        {
                           ClearDisplayLine( 2 );
                        }
                        else
                        {
							len = sprintf( str, "Column%02buErrNum:%u", WayCheck.WayNo[i+1], WayCheck.BreakCount[i+1] );
					        DisplayStr( 0, 1, 1, str, len );						
						}
					#endif					
						//DispFlag = 1;
						DisplayCursorState( 0, 0, 1, 2, 1 );//第一行整光标
						LineNum = 1;						
					}
			     }
				 break;
			case KEY_DOWN://下移
                if( i >= SumColunm ) break;//如果i的直超出货道直，则不能再递增了
			    i++;				
				if( LineNum == 1 )//选中标记下移
				{				 
				     DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标					
				     LineNum = 2;				     
				 }
				 else//菜单下翻
				 {	
			     	if( i >= SumColunm )//如果i的直超出货道直，则不能再下翻了
						break;
					else
					{					
					#ifdef _CHINA_	   
					    len = sprintf( str, "货道%02bu故障次\xca\xfd:%u", WayCheck.WayNo[i], WayCheck.BreakCount[i] );
				        DisplayStr( 0, 0, 1, str, len );
					    
						//如果下一行超出货道直，就不能显示了
                        if( (i+1) >= SumColunm )
                        {
                           ClearDisplayLine( 2 );
                        }
                        else
                        {
						   len = sprintf( str, "货道%02bu故障次\xca\xfd:%u", WayCheck.WayNo[i+1], WayCheck.BreakCount[i+1] );
				           DisplayStr( 0, 1, 1, str, len );
                        }
                        				
					#else
						len = sprintf( str, "Column%02buErrNum:%u", WayCheck.WayNo[i], WayCheck.BreakCount[i] );
				        DisplayStr( 0, 0, 1, str, len );
					    
						if( (i+1) >= SumColunm )
                        {
                           ClearDisplayLine( 2 );
                        }
                        else
                        {
						   len = sprintf( str, "Column%02buErrNum:%u", WayCheck.WayNo[i+1], WayCheck.BreakCount[i+1] );
				           DisplayStr( 0, 1, 1, str, len );
						}
		
					#endif						
						//DispFlag = 1;	
						DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标	
						LineNum = 2;					
					}
				}
				break;
		     case KEY_CANCEL:
				return;
				break;					
		}
        ClearKey();	
		CoreProcessCycle(); //让出时间片
		DelayMs( 10 );
		//WaitForWork( 50, NULL );
	}	
}

//测试货道
void  GoodsWayTest()
{
	u_char xdata m_ChannelNum = 0;
	u_char xdata m_Index = 0;
	u_char xdata m_str[20];
	u_char xdata i = 0;
	u_char xdata len = 0;
	u_char xdata j = 0;
	
	m_ChannelNum = InputGoodsWayNo( 0 );
	if( m_ChannelNum == 100 )
		return;
    GoodsSetSave = 1;         //Added by Andy 2011.6.8
	if( m_ChannelNum == 99 )  //货道全检
	{	
        //初始化货道检测数组的初始故障值,by gzz 20110302
        for( m_Index = 0; m_Index < GOODSWAYNUM; m_Index ++ )
        {
           WayCheck.WayNo[m_Index] = InputGoodsWayList[m_Index].GoodsWayNo;
           WayCheck.BreakCount[m_Index] = 0;
        } 
        while( 1 )
        {
		    WayCheck.testNum++;
        	GoodsSetSave = 1;
		    //Trace("\n GoodsSetSave = %bu", GoodsSetSave );
			for( m_Index = 0; m_Index < GOODSWAYNUM; m_Index ++ )					
			{			
				i = 0;	
				i = GetKey();
				if ( i == KEY_CANCEL )
			    {
					ClearKey();
					return;
				}
				WaitForWork( 100, NULL );	
				if( InputGoodsWayList[m_Index].UseState == 0 )
					continue;
				m_ChannelNum = InputGoodsWayList[m_Index].SetArrayNo;
				//清除其它状态位
				GoodsWaySetVal[ m_Index ].WayState = 0x01;		
			#ifdef _CHINA_
				len = sprintf( m_str, "\xd5\xfd在检测 %02bu 货道", InputGoodsWayList[m_Index].GoodsWayNo );	
			#else
				len = sprintf( m_str, "Checking column %02bu", InputGoodsWayList[m_Index].GoodsWayNo );
			#endif
				DisplayStr( 0, 0, 1, m_str, len );
			#ifdef _CHINA_
				len = sprintf( m_str,  "    请稍等..." );
			#else
				len = sprintf( m_str,  "Please wait..." );
			#endif
				DisplayStr( 0, 1, 1, m_str, len );
				WaitForWork( 500, NULL );	
				i = 0;
				j = 0;
				#ifdef   _SJJ_//升降机
					i = ChannelLifterTask( InputGoodsWayList[m_Index].GoodsWayNo, CHANNEL_TEST );		
				#else         //普通货道
                	i = ChannelTask( m_ChannelNum, CHANNEL_TEST );		
				#endif					
			#ifdef _CHINA_			
				if( i )
				{
					if( i == 2 )//主板与货道板通讯超时
					{				
						len = sprintf( m_str, "  货道板: 故障   " );
						/*
						for( i = 0; i < GOODSWAYNUM; i ++ )
						GoodsWaySetVal[ i ].WayState |= 0x09;
		                */		
						j = 1;			
					}				
					else
					{
					    if( (i!=7)&&(i!=8) )
						{
							GoodsWaySetVal[ m_Index ].WayState = 0x03;//修改货道状态查询;by gzz 20120323
							len = sprintf( m_str, "  货道%02bu: 故障   ", InputGoodsWayList[m_Index].GoodsWayNo );
							WayCheck.BreakCount[m_Index]+=1;
						}
						else if( i==8 )
						{
						    #ifdef  SYS_GOOS_SOLDOUT_CTR
								GoodsWaySetVal[ m_Index ].WayState = 0x05;
							#endif
							len = sprintf( m_str, "  货道%02bu: 无货   ", InputGoodsWayList[m_Index].GoodsWayNo );
						}
		
					}
				}
				else
				{
					GoodsWaySetVal[ m_Index ].WayState = 0x01;
					len = sprintf( m_str, "  货道 %02bu:  \xd5\xfd常 ", InputGoodsWayList[m_Index].GoodsWayNo );
				}
			#else
				if( i )
				{
					if( i == 2 )
					{				
						len = sprintf( m_str, "Broad:    Error " );
						/*
						for( i = 0; i < GOODSWAYNUM; i ++ )
							GoodsWaySetVal[ i ].WayState |= 0x09;
			            */		
						j = 1;
					}				
					else
					{
						if( (i!=7)&&(i!=8) )
						{
							GoodsWaySetVal[ m_Index ].WayState = 0x03;//修改货道状态查询;by gzz 20120323
							len = sprintf( m_str, "Column %02bu: Error", InputGoodsWayList[m_Index].GoodsWayNo );
						}
						else if( i==8 )
						{
						    #ifdef  SYS_GOOS_SOLDOUT_CTR
								GoodsWaySetVal[ m_Index ].WayState = 0x05;
							#endif
							len = sprintf( m_str, "Column %02bu: Soldout", InputGoodsWayList[m_Index].GoodsWayNo );
						}
					}
				}
				else
				{
					GoodsWaySetVal[ m_Index ].WayState = 0x01;
					len = sprintf( m_str, "Column %02bu:    OK", InputGoodsWayList[m_Index].GoodsWayNo );
				}		
			#endif
		        //Trace( " \n SystemParameter.BanknoteMax = %d", SystemParameter.BanknoteMax );
				ClearDisplayLine( 1 );
				DisplayStr( 0, 1, 1, m_str, len );
				WaitForWork( 2000, NULL );//by gzz 20120215
				if( j == 1 )
				{
					j = 0;
					break;
				}
				i = 0;	
				i = GetKey();
				if ( i == KEY_CANCEL )
			    {
					ClearKey();
					DispGoodsCheckValue();
					return;
				}	
				if(sysVPMission.msActTimer == 0)
				{
					sysVPMission.msActTimer = 100;
					VPMission_Act_RPT(VP_ACT_ADMIN,VP_ACT_ENTERADMIN);
				}
			}	
			//if(sysVPMission.msActTimer == 0)
			//{
			//	sysVPMission.msActTimer = 50;
			//	VPMission_Act_RPT(VP_ACT_ADMIN,VP_ACT_ENTERADMIN);
			//}
			//return;	
        }		
	}


	//单检指定货道
//  if( InputGoodsWayList[m_Index].UseState != 1 )
//			continue;
	memset( m_str,0, sizeof( m_str ) );
	m_Index = InputGoodsWayList[m_ChannelNum].SetArrayNo;				
//	m_ChannelNum = InputGoodsWayList[m_Index].SetArrayNo;
#ifdef _CHINA_
	sprintf( m_str, "\xd5\xfd在检测 %02bu 货道", InputGoodsWayList[m_ChannelNum].GoodsWayNo );
	DisplayStr( 0, 0, 1, m_str, 16 );
	sprintf( m_str,  "    请稍等..." );			
	DisplayStr( 0, 1, 1, m_str, 13 );
#else				
	sprintf( m_str, "Checking Column %02bu", InputGoodsWayList[m_ChannelNum].GoodsWayNo );	
//		Trace( "\n Test goodsCode = %bu", m_ChannelNum );
	DisplayStr( 0, 0, 1, m_str, 18 );
	sprintf( m_str,  "Please wait..." );
	DisplayStr( 0, 1, 1, m_str, 14 );
#endif	
	WaitForWork( 100, NULL );		
	i = 0;
//	i = ChannelExec( m_Index );
	Trace("\n GoodsWayTest m_Index = %bu", m_Index);

	#ifdef   _SJJ_//升降机
		i = ChannelLifterTask( InputGoodsWayList[m_ChannelNum].GoodsWayNo, CHANNEL_TEST );		
	#else         //普通货道
    	i = ChannelTask( m_Index, CHANNEL_TEST );		
	#endif
	
	//清除其它状态位
	GoodsWaySetVal[ m_ChannelNum ].WayState = 0x01;
#ifdef _CHINA_
	if( i )
	{
		if( i == 2 )
		{
			sprintf( m_str, "  货道板: 故障   " );
            /*
			for( i = 0; i < GOODSWAYNUM; i ++ )
				GoodsWaySetVal[ i ].WayState |= 0x09;
            */			
		}				
		else
		{
			if( (i!=7)&&(i!=8) )
			{
				GoodsWaySetVal[ m_ChannelNum ].WayState = 0x03;//修改货道状态查询;by gzz 20120323
				len = sprintf( m_str, "  货道%02bu: 故障   ", InputGoodsWayList[m_ChannelNum].GoodsWayNo );
			}
			else if( i==8 )
			{
			    #ifdef  SYS_GOOS_SOLDOUT_CTR
					GoodsWaySetVal[ m_ChannelNum ].WayState = 0x05;
				#endif
				len = sprintf( m_str, "  货道%02bu: 无货   ", InputGoodsWayList[m_ChannelNum].GoodsWayNo );
			}
		}
	}
	else
	{
		GoodsWaySetVal[ m_ChannelNum ].WayState = 0x01;
		sprintf( m_str, "  货道 %02bu:  \xd5\xfd常 ", InputGoodsWayList[m_ChannelNum].GoodsWayNo );
	}
#else
	if( i )
	{
		if( i == 2 )
		{
			sprintf( m_str, "Broad:   Error   " );
			/*
			for( i = 0; i < GOODSWAYNUM; i ++ )
				GoodsWaySetVal[ i ].WayState |= 0x09;
            */					
		}			
		else
		{
			if( (i!=7)&&(i!=8) )
			{
				GoodsWaySetVal[ m_ChannelNum ].WayState = 0x03;//修改货道状态查询;by gzz 20120323
				len = sprintf( m_str, "Column %02bu: Error", InputGoodsWayList[m_ChannelNum].GoodsWayNo );
			}
			else if( i==8 )
			{
			    #ifdef  SYS_GOOS_SOLDOUT_CTR
					GoodsWaySetVal[ m_ChannelNum ].WayState = 0x05;
				#endif
				len = sprintf( m_str, "Column %02bu: Soldout", InputGoodsWayList[m_ChannelNum].GoodsWayNo );
			}
		}
	}
	else
	{
		GoodsWaySetVal[ m_ChannelNum ].WayState = 0x01;
		sprintf( m_str, "Column %02bu:    OK ", InputGoodsWayList[m_ChannelNum].GoodsWayNo );
	}				
#endif
	GoodsSetSave = 1;
	Trace("\n GoodsWayTest GoodsWaySetVal[%bu].WayState = %02bx", m_ChannelNum,GoodsWaySetVal[ m_ChannelNum ].WayState );
	ClearDisplayLine( 1 );
	DisplayStr( 0, 1, 1, m_str, 17 );
	WaitForWork( 2000, NULL );
	if( i == 3 )
	{
		for( i = 0; i < GOODSWAYNUM; i ++ )
		{
			//清除其它状态位
            //GoodsWaySetVal[ i ].WayState = 0x00;
			//GoodsWaySetVal[ i ].WayState = 0x09;
		}
	}
}


/*脉冲硬币测试*/
void CoinerTest()
{	
	u_int xdata CoinSum = 0;
	u_int xdata CoinMnu = 0;
	u_char xdata str[20];
	u_char xdata flag = 0;
	u_char xdata len = 0;
	u_char xdata MyKey = 0;
    u_char xdata coinFull = 0;
    u_char xdata disFullFlag = 0;
	u_char xdata disEmpFlag = 0;

	//照明灯测试
	Trace( "Coiner Test begin\n" );
	CoinerEnable();//使能硬币器
#ifdef _CHINA_
	DisplayStr( 0, 0, 1,"    硬币器测试", 14 );
	DisplayStr( 0, 1, 1, "请投币:", 7 );
#else
	DisplayStr( 0, 0, 1,"Coin Acceptor test!", 19 );
  //DisplayStr( 0, 1, 1, "Insert coin:\x24", 13 );
	DisplayStr( 0, 1, 1, "Insert coin:", 12 );
#endif
	
    //SgwSecTimer= 30;		
	memset( str, 0, sizeof( str ) );
	while( 1 )
	{
		WaitForWork( 100, NULL );
		CoinMnu = GetCoin();	
		if( CoinMnu != 0 )
		{		
			while ( CoinMnu ) 
			{		
		#ifdef _COIN_INPUT_PARALLEL			
				CoinSum += CoinMnu;
		#else
				CoinSum += CoinMnu*COINMULTIPLE;			
		#endif
				Trace("\n CoinSum = %u", CoinSum);
                SchedulerProcess();
                WaitForWork( SYS_WAIT_NEXT_PULSE, NULL );    //Wait for the next pulse, 100-300-120                 
				CoinMnu = GetCoin();
			}
			flag = 1;
            WaitForWork( 100, NULL );
		}

        //----------------------------------------------------
        if( IfCoinHopperFull() )
        {
            if( coinFull == 0 )
            {
        		coinFull = 1;
                CoinerDisable();
                disFullFlag = 1;
            }
        }
        else
        {
            if( coinFull == 1 )
            {
                CoinerEnable();
            	coinFull = 0;
				flag = 1;
				disEmpFlag = 1;
            }
        }
        //====================================================
        if( coinFull == 1 )
        {
            if (disFullFlag == 1)
			{
	            #ifdef _CHINA_
						len = sprintf( str, "  币斗已满, 请处理!"  );	
				#else
						len = sprintf( str, "  Coin full, check!" );		
				#endif	
	            ClearDisplayLine( 2 );
	        	DisplayStr( 0, 0, 1, str, len );  
                disFullFlag = 0;
  		    }
             
        }
		else if( flag == 1 )
		{	
		    if(disEmpFlag == 1 )
			{
	            #ifdef _CHINA_
					DisplayStr( 0, 0, 1,"    硬币器测试", 14 );
				#else
					DisplayStr( 0, 0, 1,"Coin Acceptor test!", 19 );
				#endif
				disEmpFlag = 0;
			}

			switch( SystemParameter.curUnit )
			{
				case 1:
			#ifdef _CHINA_
					len = sprintf( str, "已投入金额: %u", CoinSum );	
			#else
					len = sprintf( str, "Insert coin: %u", CoinSum );		
			#endif	
				break;				
				case 10:
			#ifdef _CHINA_
					len = sprintf( str, "已投入金额: %u.%u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );	
			#else
					len = sprintf( str, "Insert coin: %u.%u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );		
			#endif	
				break;
				case 100:
			#ifdef _CHINA_
					len = sprintf( str, "已投入金额: %u.%02u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );	
			#else
					len = sprintf( str, "Insert coin: %u.%02u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );		
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
				flag = 0;
			}
		}
		MyKey = GetKey();		
		if( ( MyKey  == KEY_SUBMIT ) || ( MyKey  == KEY_CANCEL ) )
			break;
	}		
	CoinerDisable();//禁能硬币器
#ifdef _CHINA_
	len = ChooseReturn( "  硬币器是否\xd5\xfd常?" );
#else
	len = ChooseReturn( "Coin Acceptor OK ?" );	
#endif	
	if( len == 0 )//正常		
		DeviceStatus.CoinAccepter = 0;		
	else 		
		DeviceStatus.CoinAccepter = 1;	
}

//展示灯和压缩机测试，1代表展示灯，2代表压缩机
void ACDCModuleTest(uchar xdata testDev)
{	
	u_char xdata len = 0;
	u_char xdata key = 0;
	u_char xdata DevStr[10];
	u_char xdata str[20];
    
	//111017 by cq 
	//提示ACDC模式未开启
	if( SystemParameter.ACDCModule == 0 )
	{
		//...
#ifdef _CHINA_
			len = sprintf( str, "%s", "ACDC模式未开启" );
			DisplayStr( 0, 0, 1, str, len ); 
#else		
			len = sprintf( str, "%s", "ACDCmode closed" );	
			DisplayStr( 0, 0, 1, str, len ); 
#endif
        ClearDisplayLine( 2 );
		WaitForWork(2000, NULL);
		return;
	}

	if(testDev == 1)
	{
		#ifdef _CHINA_
			sprintf( DevStr, "%s", "展示灯" );
		#else
			sprintf( DevStr, "%s", "Light" );
		#endif		
	}
	else if(testDev == 2)
	{
		#ifdef _CHINA_
			sprintf( DevStr, "%s", "压缩机" );
		#else
			sprintf( DevStr, "%s", "Compressor" );
		#endif		
	}

	#ifdef _CHINA_
		len = sprintf( str, "  %s测试", DevStr );
		DisplayStr( 0, 0, 1, str, len ); 
		DisplayStr( 0, 1, 1, "1-打开  2-关闭", 14 );
	#else		
		len = sprintf( str, "  %s Test!", DevStr );		
	    DisplayStr( 0, 0, 1, str, len ); 
		DisplayStr( 0, 1, 1, "1-Open 2-Close", 14 );
	#endif
	
	ClearKey();	
    while( 1 )
	{
		key = GetKey();
		if(key == KEY_1 ) 
		{
		    if( testDev == 1 )
			{
			    sysVPMission.ACDCLedCtr = 1; 
			}
			else
			{
				sysVPMission.ACDCCompressorCtr = 1;
			}
			ACDCMission();
			ClearDisplayLine( 1 );
		    ClearDisplayLine( 2 );
			#ifdef _CHINA_
				len = sprintf( str, "%s已打开，请确认", DevStr );				
			#else
				len = sprintf( str, "Open %s", DevStr );				
			#endif	
			DisplayStr( 0, 0, 1, str, len ); 
		}
		else if(key == KEY_2 ) 
		{
		    if( testDev == 1 )
			{
			    sysVPMission.ACDCLedCtr = 0; 
			}
			else
			{
				sysVPMission.ACDCCompressorCtr = 0;
			}
			ACDCMission();

			ClearDisplayLine( 1 );
		    ClearDisplayLine( 2 );
			#ifdef _CHINA_
				len = sprintf( str, "%s已关闭，请确认", DevStr );				
			#else
				len = sprintf( str, "Close %s", DevStr );				
			#endif	
			DisplayStr( 0, 0, 1, str, len ); 
		}
		else if((key==KEY_CANCEL)||(key==KEY_SUBMIT))
 	    {
		    #ifdef _CHINA_
				len = sprintf( str, "请不要随意关闭设备！");				
			#else
				len = sprintf( str, "Keep the dev work..." );				
			#endif	
			ClearDisplayLine( 2 );
			DisplayStr( 0, 0, 1, str, len ); 
		    WaitForWork( 2000, NULL );
			break;
		}
		WaitForWork( 100, NULL );	
	}    	
}


void ITLBillATest()
{	
	u_int xdata  CoinSum = 0;
	u_int xdata  CoinMnu = 0;
	u_char xdata str[20];
	u_char xdata flag = 0;
	u_char xdata len = 0;
	u_char xdata MyKey = 0;
	u_char xdata mission = 0;

	ITLMission_Init();
#ifdef _CHINA_
	DisplayStr( 0, 0, 1,"    纸币器测试", 14 );
	DisplayStr( 0, 1, 1, "请投币:", 7 );
#else
	DisplayStr( 0, 0, 1, "Bill Acceptor test!", 19 );
	DisplayStr( 0, 1, 1, "Insert bill:", 12 );
#endif		
	memset( str, 0, sizeof( str ) );
    //ITLMission_Enable();
	ITLMission_Init_1();

	while( 1 )
	{
	
        flag = 0;  

        if( (SystemParameter.BillNo==1)&&(sysITLMission.ITLSet==1))
        {
			if( /*(sysITLMission.billSta&ITL_BILL_SJAM)||*/( sysITLMission.billSta& ITL_BILL_UJAM) )
	 	    {
				ITLMission_Init_1();
			}
	        //
			if((sysITLMission.status&ITL_DEV_FAIL)||(sysITLMission.status&ITL_DEV_FATTN)/*||(sysITLMission.status&ITL_DEV_NCFFAR)||( sysITLMission.status&ITL_DEV_NCICBAR)*/)
			{
				ITLMission_Init_1();
			}
	    }
        
		if( (SystemParameter.BillNo==1)&&(sysITLMission.ITLSet==1)&&(sysITLMission.enableFlag==1)&&(sysITLMission.status&ITL_DEV_DISABLED) )
	    {
	        //flag = ITLMission_Enable();
			sysITLMission.reEnableFlag = 1;
			if( sysITLMission.reEnableTime >= ITL_TIME_DISABLED_INIT )
			{
				flag = ITLMission_Init_1();
				sysITLMission.reEnableTime = 0;
			}
	    }
		else
		{
		    sysITLMission.reEnableFlag = 0;	
		}

	    if( (sysITLMission.ITLSet==1)&&(sysITLMission.msPoll == 0)/*&&( DeviceStatus.BillAccepter == 0 )*/ )
        {
    		mission = ITLMission_Poll(); 
            sysITLMission.msPoll = ITL_TIME_POLL;    //
        }
      
		if( sysITLMission.billSta & ITL_BILL_READ )
		{
            sysITLMission.billSta    &= ~ ITL_BILL_READ;
            sysITLMission.billStaCtr |= ITL_BILL_READ;
			if( (CoinSum+sysITLMission.billValue) > 10000 )
			{
				mission = ITLMission_Reject();
			}
		}

		//if( sysITLMission.billSta & ITL_BILL_CREDIT )
        if( ( sysITLMission.billSta & ITL_BILL_CREDIT )&&( sysITLMission.billStaCtr & ITL_BILL_READ )/*&&( CurrentPayed < SystemParameter.BanknoteMax )*/ )
		{
            sysITLMission.billSta    &= ~ITL_BILL_CREDIT;
            sysITLMission.billStaCtr &= ~ITL_BILL_READ;
            CoinSum += sysITLMission.billValue;
			if( CoinSum >= 10000 )
			{
				mission = ITLMission_Disable();
			}
			flag = 1;
		}

        if( sysITLMission.billSta & ITL_BILL_STACKED )
		{   /*
			CoinSum += sysITLMission.billValue;
			if( CoinSum >= 10000 )
			{
				mission = ITLMission_Disable();
			}
			flag = 1;
            */
		}
       
		if( flag == 1 )
		{	
			switch( SystemParameter.curUnit )
			{
				case 1:
			#ifdef _CHINA_
					len = sprintf( str, "已投入金额: %u", CoinSum );	
			#else
					len = sprintf( str, "Insert bill: %u", CoinSum );		
			#endif	
				break;				
				case 10:
			#ifdef _CHINA_
					len = sprintf( str, "已投入金额: %u.%u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );	
			#else
					len = sprintf( str, "Insert bill: %u.%u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );		
			#endif	
				break;
				case 100:
			#ifdef _CHINA_
					len = sprintf( str, "已投入金额: %u.%02u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );	
			#else
					len = sprintf( str, "Insert bill: %u.%02u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );		
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
				flag = 0;
			}
		}
		MyKey = GetKey();		
		if( ( MyKey  == KEY_SUBMIT ) || ( MyKey  == KEY_CANCEL ) )
        {
			break;
        }
	}		

	ITLMission_Disable();
#ifdef _CHINA_
	len = ChooseReturn( "  纸币器是否\xd5\xfd常?" );
#else
	len = ChooseReturn( "Bill acceptor OK ?" );	
#endif	

	if( len == 0 )//正常		
		DeviceStatus.BillAccepter = 0;		
	else 		
		DeviceStatus.BillAccepter = 1;	
    
}

/*****************************************************************************
* 1*k豪秒延时
* k的取值范围0~65535
*****************************************************************************/
void DELAY_MS(uint k)
{
uint j;

while(k!=0)
  {
  k--;
  for(j=0;j<4800;j++);
  }
}

/*
void MDBBillATest( void )
{	
	u_int xdata  CoinSum = 0;
	u_int xdata  CoinMnu = 0;
	u_char xdata str[20];
	u_char xdata flag = 0;
	u_char xdata len = 0;
	u_char xdata MyKey = 0;
	u_char xdata mission = 0;
	
	uchar const JUSTCOMMAND[1]={0};
	uchar const COIN_TYPE[4]={0xff, 0xff, 0xff, 0xff};
	uchar const DISPENSE1[1]={0x10};
	uchar const DISPENSE2[1]={0x11};
	uchar const EXPANSION[2]={0x02,0x03};  
	uchar const BILL_TYPE[4]={0xff, 0xff, 0xff, 0xff};
	uchar const ESCROW1[1]={0xff};  //压钞
	uchar const ESCROW2[1]={0x00};  //退钞
    
#ifdef _CHINA_
	DisplayStr( 0, 0, 1," MDB纸币器测试", 14 );
	DisplayStr( 0, 1, 1, "请投币:", 7 );
#else
	DisplayStr( 0, 0, 1, "Bill Acceptor test!", 19 );
	DisplayStr( 0, 1, 1, "Insert bill:", 12 );
#endif		
    //DELAY_MS(100);
	DelayMs( 100 );
	MDBMission_Init(); 
    DelayMs( 200 );
	memset( str, 0, sizeof( str ) );

    Session(0x08,JUSTCOMMAND,0);        //复位硬币机
	Session(0x30,JUSTCOMMAND,0);        //复位纸币机
	//DELAY_MS(500);
    DelayMs( 500 );
	Session(0x09,JUSTCOMMAND,0);        //SETUP硬币机;
	Session(0x0c,COIN_TYPE,4);          //COIN TYPE
	Session(0x34,BILL_TYPE,4);          //BILL TYPE
    
	while( 1 )
	{	
        //MDBMission_Init();
		Session(0x0b,JUSTCOMMAND,0);  //轮询硬币机,poll
    	Session(0x33,JUSTCOMMAND,0);  //轮询纸币机,poll
		MyKey = GetKey();		
		if( ( MyKey  == KEY_SUBMIT ) || ( MyKey  == KEY_CANCEL ) )
        {
			break;
        }
	}		
    ITLSerialInit();
#ifdef _CHINA_
	len = ChooseReturn( "  纸币器是否\xd5\xfd常?" );
#else
	len = ChooseReturn( "Bill acceptor OK ?" );	
#endif	
	if( len == 0 )//正常		
		DeviceStatus.BillAccepter = 0;		
	else 		
		DeviceStatus.BillAccepter = 1;	    
}
*/


void MDBBillATest( void )
{	
	u_int xdata  CoinSum = 0;
	u_int xdata  CoinMnu = 0;
	u_char xdata str[20];
	u_char xdata flag = 0;
	u_char xdata len = 0;
	u_char xdata MyKey = 0;
	u_char xdata mission = 0;
    u_char xdata i = 0;

    //111019 by gzz 
	//提示纸币器未开启
	if( SystemParameter.BillNo == 0 )
	{
		//...
#ifdef _CHINA_
			len = sprintf( str, "%s", "纸币器未开启" );
			DisplayStr( 0, 0, 1, str, len ); 
#else		
			len = sprintf( str, "%s", "Coin closed" );	
			DisplayStr( 0, 0, 1, str, len ); 
#endif
        ClearDisplayLine( 2 );
		WaitForWork(2000, NULL);
		return;
	}

    //启动不成功，重试三次;by gzz 20111025
    DelayMs( 50 );
    do
    {
		MDBMission_Bill_Init();
        i++; 
        DelayMs( 100 );
    }
    while((sysMDBMission.billDeviceStatus!=0)&&(i<3));
    

#ifdef _CHINA_
	DisplayStr( 0, 0, 1,"    纸币器测试", 14 );
	DisplayStr( 0, 1, 1, "请投币:", 7 );
#else
	DisplayStr( 0, 0, 1, "Bill Acceptor test!", 19 );
	DisplayStr( 0, 1, 1, "Insert bill:", 12 );
#endif		
	memset( str, 0, sizeof( str ) );
    DelayMs( 100 );
    
	
    
	while( 1 )
	{
        flag = 0;            
		mission =MDB_Bill_IfSecrow( sysMDBMission.billBuf, &sysMDBMission.billBufLen, &sysMDBMission.billType );
        if( mission == 1 )
		{
		    if( CoinSum+SystemParameter.billValue[sysMDBMission.billType] >=65000 )
            {
				mission = MDB_Bill_EscrowCtr( 0 );
			}
		    else
			{
				mission = MDB_Bill_EscrowCtr( 1 );
                if(mission==0)
                {   
				    sysVPMission.billSTimer = TIME_BILL_STACK;  
				    while( sysVPMission.billSTimer )
					{
                        mission = MDB_Bill_IfStacked( sysMDBMission.billBuf, &sysMDBMission.billBufLen );
						if( mission == 1 )
					    {
		                	CoinSum += SystemParameter.billValue[sysMDBMission.billType];
		                 	memset(sysMDBMission.billBuf,0,sizeof(sysMDBMission.billBuf));
							sysMDBMission.billBufLen=0;
							sysMDBMission.billType=0;
							sysMDBMission.billStock=0;
							flag = 1;
                            break;//更新币值慢，而且容易投入纸币后，将其暂存而不显示金额;by gzz 20111021
						}
					}
                }
			}					
		}
       
		if( flag == 1 )
		{	
		    
			switch( SystemParameter.curUnit )
			{
				case 1:
			#ifdef _CHINA_
					len = sprintf( str, "已投入金额: %u", CoinSum );	
			#else
					len = sprintf( str, "Inserted bill: %u", CoinSum );		
			#endif	
				break;				
				case 10:
			#ifdef _CHINA_
					len = sprintf( str, "已投入金额: %u.%u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );	
			#else
					len = sprintf( str, "Inserted bill: %u.%u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );		
			#endif	
				break;
				case 100:
			#ifdef _CHINA_
					len = sprintf( str, "已投入金额: %u.%02u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );	
			#else
					len = sprintf( str, "Inserted bill: %u.%02u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );		
			#endif	
				break;
				default:
					len = 0;
					memset( str, 0, sizeof( str ) );
				break;
			}
			if( len != 0 )
			{
			    // 
                DelayMs( 50 );
				//DelayMs( 50 );
				DisplayStr( 0, 1, 1, str, len );
				flag = 0; 
				//
				DelayMs( 100 );
	            MDBSerialInit();
                DelayMs( 100 );
			}
		}
		MyKey = GetKey();		
		if( ( MyKey  == KEY_SUBMIT ) || ( MyKey  == KEY_CANCEL ) )
        {
			break;
        }
	}
	MDB_Bill_EscrowCtr(0);
	MDBMission_Bill_Disable();
#ifdef _CHINA_
	len = ChooseReturn( "  纸币器是否\xd5\xfd常?" );
#else
	len = ChooseReturn( "Bill acceptor OK ?" );	
#endif	

	if( len == 0 )
	{	
		DeviceStatus.BillAccepter = 0;
		sysMDBMission.billDeviceStatus = 0;
	}
	else 
	{	
		DeviceStatus.BillAccepter = 1;
		sysMDBMission.billDeviceStatus = 1;
	}
    
}

void MDBCoinATest( void )
{	
	u_int xdata  CoinSum = 0;
	u_int xdata  CoinMnu = 0;
	u_char xdata str[20];
	u_char xdata flag = 0;
	u_char xdata len = 0;
	u_char xdata MyKey = 0;
	u_char xdata mission = 0;
    u_char xdata i = 0;

       
	//111019 by gzz 
	//提示硬币器未开启	
	if( SystemParameter.coinOn == 0 )
	{   
		//...
#ifdef _CHINA_
			len = sprintf( str, "%s", "硬币器未开启" );
			DisplayStr( 0, 0, 1, str, len ); 
#else		
			len = sprintf( str, "%s", "Coin closed" );	
			DisplayStr( 0, 0, 1, str, len ); 
#endif
        ClearDisplayLine( 2 );
		WaitForWork(2000, NULL);
		return;
	}

    //调整启动的位置;by gzz 20120323
    //启动不成功，重试三次;by gzz 20111025
    DelayMs( 50 );
    do
    { 
		MDBMission_Coin_Init(); 
        i++; 
        DelayMs( 100 );
    }
    while((sysMDBMission.coinDeviceStatus!=0)&&(i<3));

#ifdef _CHINA_
	DisplayStr( 0, 0, 1,"    硬币器测试", 14 );
	DisplayStr( 0, 1, 1, "请投币:", 7 );
#else
	DisplayStr( 0, 0, 1,"Coin Acceptor test!", 19 );
	DisplayStr( 0, 1, 1, "Insert coin:", 12 );
#endif	
	memset( str, 0, sizeof( str ) );    
    DelayMs( 100 );
    
	while( 1 )
	{
	
        flag = 0;          
        mission = MDB_Coin_IfDeposited( sysMDBMission.coinBuf, &sysMDBMission.coinBufLen, &sysMDBMission.coinType, &sysMDBMission.coinStock );
        if( mission == 1 )
		{
			CoinSum += SystemParameter.coinValue[sysMDBMission.coinType];
			flag = 1;
            memset(sysMDBMission.coinBuf,0,sizeof(sysMDBMission.coinBuf));
			sysMDBMission.coinBufLen=0;
			sysMDBMission.coinType=0;
			sysMDBMission.coinStock=0;
		}
		if( flag == 1 )
		{	
		    
			switch( SystemParameter.curUnit )
			{
				case 1:
			#ifdef _CHINA_
					len = sprintf( str, "已投入金额: %u", CoinSum );	
			#else
					len = sprintf( str, "Inserted coin: %u", CoinSum );		
			#endif	
				break;				
				case 10:
			#ifdef _CHINA_
					len = sprintf( str, "已投入金额: %u.%u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );	
			#else
					len = sprintf( str, "Inserted coin: %u.%u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );		
			#endif	
				break;
				case 100:
			#ifdef _CHINA_
					len = sprintf( str, "已投入金额: %u.%02u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );	
			#else
					len = sprintf( str, "Inserted coin: %u.%02u", CoinSum/SystemParameter.curUnit, CoinSum%SystemParameter.curUnit );		
			#endif	
				break;
				default:
					len = 0;
					memset( str, 0, sizeof( str ) );
				break;
			}
			if( len != 0 )
			{
                DelayMs( 50 );
				DisplayStr( 0, 1, 1, str, len );
				flag = 0; 
				DelayMs( 100 );
	            MDBSerialInit(); 
                DelayMs( 200 );
			}
		}
		MyKey = GetKey();		
		if( ( MyKey  == KEY_SUBMIT ) || ( MyKey  == KEY_CANCEL ) )
        {
			break;
        }
	}
	
	//退出时，更新硬币剩余金额;by gzz 20120309
	MDB_Coin_TubeSta_API(); 
    WaitForWork(1000,NULL);
	MDBMission_Coin_Disable();
	WaitForWork(1000,NULL);
#ifdef _CHINA_
	len = ChooseReturn( "  硬币器是否\xd5\xfd常?" );
#else
	len = ChooseReturn( "Bill acceptor OK ?" );	
#endif	

	if( len == 0 )
	{	
		DeviceStatus.CoinAccepter = 0;
		sysMDBMission.coinDeviceStatus = 0;
	}
	else 	
	{
		DeviceStatus.CoinAccepter = 1;	
		sysMDBMission.coinDeviceStatus |= 0x01;
	}
}


void MDBHopperATest( uchar xdata testDev )
{	
	u_char xdata mission = 0;
	u_char xdata len = 0;
	u_char xdata dispensedNum = 0;
    u_char xdata coin[MDB_MAX_BLOCK_SIZF];
    u_char xdata coinLen;
    
	//修改退币函数;by gzz 20110721
	if(testDev == 1)
	{
		#ifdef _CHINA_
			len = ChooseReturn( "  五角退币，一次两枚" );
		#else
			len = ChooseReturn( "5jiao" );	
		#endif	
		if( len == 0 )//确认退币
		{
            sysMDBMission.coinDeviceStatus = 1;
			mission=MDB_Coin_Dispense(0,2);
            DelayMs(500);
			if(mission==0)
			{
             	sysVPMission.hopperOutTimer = 20;
			    while( sysVPMission.hopperOutTimer )
				{
			    	mission = MDB_Coin_EXP_PayoutValuePoll( &dispensedNum );
					if( mission == 0x00 )
					{
						mission = MDB_Coin_EXP_PayoutStatus( &coin, &coinLen );
						if( mission == 0x01 )
						{
						    sysMDBMission.coinDeviceStatus = 0;  
                            break;
						}						
					}					
				}
				//sysMDBMission.coinDeviceStatus = 0;
			}
			else
			{
				sysMDBMission.coinDeviceStatus = 1;
			}
		}
		
	}
	else if(testDev == 2)
	{
		#ifdef _CHINA_
			len = ChooseReturn( "  一元退币，一次两枚" );
		#else
			len = ChooseReturn( "1yuan" );	
		#endif	
		if( len == 0 )//确认退币
		{
            sysMDBMission.coinDeviceStatus = 1;
			mission=MDB_Coin_Dispense(1,2);
			DelayMs(500);
			if(mission==0)
			{
             	sysVPMission.hopperOutTimer = 20;
			    while( sysVPMission.hopperOutTimer )
				{
			    	mission = MDB_Coin_EXP_PayoutValuePoll( &dispensedNum );
					if( mission == 0x00 )
					{
						mission = MDB_Coin_EXP_PayoutStatus( &coin, &coinLen );
						if( mission == 0x01 )
						{
						    sysMDBMission.coinDeviceStatus = 0;  
                            break;
						}						
					}					
				}
				//sysMDBMission.coinDeviceStatus = 0;
			}
			else
			{
				sysMDBMission.coinDeviceStatus = 1;
			}
		}
	}    
}


/*选货按钮测试*/
void ButtonTest()
{		
	unsigned char xdata key = 0;
	unsigned char xdata Oldkey = 0 ;
	u_char xdata str[20];	
	u_char xdata len = 0;
	u_char xdata MyKey = 0;

	Trace( "\n Button test begin" );	
#ifdef _CHINA_
	DisplayStr( 0 , 0 , 1 , "    按钮测试!" , 13 ); 		
	DisplayStr( 0, 1, 1, "输入按钮:", 9 );	
#else
	DisplayStr( 0 , 0 , 1 , "Button test!" , 12 ); 		
	DisplayStr( 0, 1, 1, "Input:", 6 );	
#endif
    WaitForWork( 200, NULL );	
	//SgwSecTimer= 30;
	//Open the led
    sysVPMission.sel1ReadyLed = 0xff;
    sysVPMission.sel1ErrLed   = 0xff;
    sysVPMission.sel2ReadyLed = 0xff;
    sysVPMission.sel2ErrLed   = 0xff;
    sysVPMission.sel3ReadyLed = 0xff;
    sysVPMission.sel3ErrLed   = 0xff;		
	while( 1 )
	{
		WaitForWork( 100, NULL );
		/*
		key = PowerCotrlTask( POWER_QUERY );
		Trace( "\n key1 = %bu", key );
		*/
		key = GetKey_M2();
        WaitForWork( 100, NULL );
		if( ( key >= 0 ) && ( key < GOODSTYPEMAX ) )
		{
            key += 1;            
			Trace( "\n key2 = %bu", key );	
			Trace( "\n Oldkey = %bu", Oldkey );	
			if( Oldkey != key )
			{
			#ifdef _CHINA_
				len = sprintf( str, "输入按钮: %bu", key );	
			#else
				len = sprintf( str, "Input: %bu",key );
			#endif				
				DisplayStr( 0, 1, 1, str, len );	
				Oldkey = key;
                WaitForWork( 200, NULL );
			}
		}
		key = GetKey();		
		if( ( key  == KEY_SUBMIT ) || ( key  == KEY_CANCEL ) )
			break;
	}
	//Close the led
	sysVPMission.sel1ReadyLed = 0x00;
    sysVPMission.sel1ErrLed   = 0x00;
    sysVPMission.sel2ReadyLed = 0x00;
    sysVPMission.sel2ErrLed   = 0x00;
    sysVPMission.sel3ReadyLed = 0x00;
    sysVPMission.sel3ErrLed   = 0x00;
    key = GetKey_M2();
}

void GameButtonTest()
{		
	unsigned char xdata key = 0;
	unsigned char xdata Oldkey = 0 ;
	//u_char xdata str[20];	
	u_char xdata len = 0;
	u_char xdata MyKey = 0;

	Trace( "\n Game button test begin" );	
#ifdef _CHINA_
	DisplayStr( 0 , 0 , 1 , "    游戏按钮测试!" , 17 ); 		
	DisplayStr( 0, 1, 1, "有键按下, 则鸣叫一声", 20  );	
#else
	DisplayStr( 0 , 0 , 1 , "Game button test!" , 17 ); 		
	DisplayStr( 0, 1, 1, "Beep for one press!", 19 );	
#endif
    WaitForWork( 200, NULL );	
	//SgwSecTimer= 30;
	//Open the led
	GameLedOn();
	while( 1 )
	{
		//WaitForWork( 100, NULL );
		if( IfGameKeyOn() )
		{
			Beep();
			/*
            #ifdef _CHINA_
				len = sprintf( str, "按钮: 1" );	
			#else
				len = sprintf( str, "Button: 1");
			#endif	
			DisplayStr( 0, 1, 1, str, len );	
            WaitForWork( 100, NULL );
			*/
		}
		key = GetKey();		
		if( ( key  == KEY_SUBMIT ) || ( key  == KEY_CANCEL ) )
			break;
	}
	//Close the led
    GameLedOff();
}


//111017 by cq 出货确认	 
void GOCTest()
{	
	u_char xdata len = 0;
	u_char xdata key = 0;
	u_char xdata DevStr[10];
	u_char xdata str[20];

	ClearDisplayLine( 1 );
	ClearDisplayLine( 2 );
    
	if( SystemParameter.GOCOn == 0 )
	{
	    //...	
#ifdef _CHINA_
		len = sprintf( str, "%s", "出货确认未开启" );
		DisplayStr( 0, 0, 1, str, len ); 
#else		
		len = sprintf( str, "%s", "GOC closed" );	
		DisplayStr( 0, 0, 1, str, len ); 
#endif
		WaitForWork(2000,NULL);
		return;
	}
	
#ifdef _CHINA_
			sprintf( DevStr, "%s", "出货确认" );
#else
			sprintf( DevStr, "%s", "GOC" );
#endif		

	#ifdef _CHINA_
		len = sprintf( str, "  %s测试", DevStr );
		DisplayStr( 0, 0, 1, str, len ); 
		//DisplayStr( 0, 1, 1, "1-打开  2-关闭", 14 );
	#else		
		len = sprintf( str, "  %s Test!", DevStr );		
	    DisplayStr( 0, 0, 1, str, len ); 
		//DisplayStr( 0, 1, 1, "1-Open 2-Close", 14 );
	#endif
	WaitForWork( 500, NULL );	
	ClearKey();	
	
    while( 1 )
	{
		key = GetKey();		
		ChannelTask( 0, CHANNEL_QUERY );
		
		if( SystemParameter.GOCOn == 1 )
		{
			   if( Channel.State & 0x80 )
			   {
				   //DeviceStatus.GOCStatus |= 0x01; 

				#ifdef _CHINA_
					len = sprintf( str, "%s", "    检测到有遮挡" );		
					DisplayStr( 0, 1, 1, str, len );
				#else		
					len = sprintf( str, "%s", "is covering GOC" );		
					DisplayStr( 0, 1, 1, str, len );
				#endif
			   }
			   else
			   {
				   //DeviceStatus.GOCStatus &= 0xFE;

				#ifdef _CHINA_
				   len = sprintf( str, "%s", "    请遮挡两秒" );	   
				   DisplayStr( 0, 1, 1, str, len );
				#else		
				   len = sprintf( str, "%s", "cover GOC 2sec" );	   
				   DisplayStr( 0, 1, 1, str, len );
				#endif
			   }
		}
		
				
		if((key==KEY_CANCEL)||(key==KEY_SUBMIT))
 	    {
			SystemSave = 1;
			break;
		}
		WaitForWork( 500, NULL );//可以适当延长
	}    	
}


void ReturnButtonTest()
{		
	unsigned char xdata key = 0;
	unsigned char xdata Oldkey = 0 ;
	//u_char xdata str[20];	
	u_char xdata len = 0;
	u_char xdata MyKey = 0;

	Trace( "\n Game button test begin" );	
#ifdef _CHINA_
	DisplayStr( 0 , 0 , 1 , "    退币按钮测试!" , 17 ); 		
	DisplayStr( 0, 1, 1,    "      没有按下", 14  );	
#else
	DisplayStr( 0 , 0 , 1 , "Return button test!" , 19 ); 		
	DisplayStr( 0, 1, 1, "Beep for one press!", 19 );	
#endif
    WaitForWork( 200, NULL );	
	//SgwSecTimer= 30;
	//Open the led
	//GameLedOn();
	while( 1 )
	{
        //Added  for changer return key
        MDB_Coin_IfDeposited( sysMDBMission.coinBuf, &sysMDBMission.coinBufLen, &sysMDBMission.coinType, &sysMDBMission.coinStock );
		WaitForWork( 100, NULL );
		if( IfReturnKeyOn() )
		{
			Beep();
			DisplayStr( 0, 1, 1,    "    按下退币按钮", 16  );
			WaitForWork( 2000, NULL );
			DisplayStr( 0, 1, 1,    "      没有按下", 14  );
			IfReturnKeyOn();
		}
		key = GetKey();		
		if( ( key  == KEY_SUBMIT ) || ( key  == KEY_CANCEL ) )
			break;
	}
	//Close the led
    //GameLedOff();
}

void ICCardTest()
{		
	
}




/*取得设备的状态,可取状态的设备有纸硬币器，退币器，*/
void GetDeviceStatus(unsigned char iDeviceID )
{
	u_char   xdata flag ;
//	unsigned char xdata i;	

	flag = 0;
	switch(iDeviceID)
	{	
		case 0:       //1纸币器状态	
            #ifdef MACHINE_SET_MDB
            	break;
            #else
                if( SystemParameter.BillNo == 1 )				
				{
	                if( sysITLMission.ITLSet == 1 )
	                {
	                    //flag = ITLMission_Syn();
	                    flag = ITLMission_Poll();
	                    if( flag == ITL_ERR_NULL )
	                    {
	                        DeviceStatus.BillAccepter = 0;
	                    }
	                    else
	                    {
	                        DeviceStatus.BillAccepter |= 0x01;
	                    }
	                }
	                else
	                {
						flag = GetHardWareDeviceState( 4, &Casher );
						if( flag == 0 )
						{
							flag = 0;
							flag = TestDeviceException( &Casher );			
							if ( flag )
							{					
								DeviceStatus.BillAccepter = 1;
								return ;
							}
							else				
								DeviceStatus.BillAccepter = 0;				
						}	
						else
		                    DeviceStatus.BillAccepter = 1;
	                }		
				}
				else
					DeviceStatus.BillAccepter = 128;
				Trace( "\n DeviceStatus.BillAccepter = %bu", DeviceStatus.BillAccepter );
				break;		    
            #endif		
		case 1:       //2硬币器状态
            #ifdef MACHINE_SET_MDB
            	break;
            #else
				CoinerTest();
				WaitForWork( 100, NULL );		
				break;
            #endif			
		case 2:       //3 #1退币器测试	
            #ifdef MACHINE_SET_MDB
			    DeviceStatus.ChangeUnit1 = 0;
            	break;            
            #endif
			if( SystemParameter.HopperCoinPrice1 == 0 )
			{ 
				DeviceStatus.ChangeUnit1 = 1;
				break;
			}		
			if( m_iCheckDevice )
			{			
				flag = HopperOutCoin( 0, 1, &Hopper1 );  //出币5枚,5-0(Chnaged by Andy on 2011.3.3 )
				WaitForWork( 1500 , NULL );
				if( flag == 0 )                          //出币命令发送成功
				{				
					flag = GetHardWareDeviceState( 1, &Hopper1 );
					if( flag != 1 )
					{
						
                      //if( Hopper1.State & 0x50 )         //系统故障
                        if( Hopper1.State & 0x50 )         //系统故障																									
							DeviceStatus.ChangeUnit1 = 1;
                        else if( Hopper1.State & 0x08 ) 
                            DeviceStatus.ChangeUnit1 = 4;
						else if( Hopper1.State & 0x20 )    //光测无币或实际无币							
							DeviceStatus.ChangeUnit1 = 2;						
						else						
							DeviceStatus.ChangeUnit1 = 0;						
					}	
					else
						DeviceStatus.ChangeUnit1 = 1;
				}
				else//出币命令发送不成功				
					DeviceStatus.ChangeUnit1 = 1;				
			}
			else
			{
				flag = GetHardWareDeviceState( 1, &Hopper1 );			
				if( flag != 1 )
				{					
					if( Hopper1.State & 0x50 )         //系统故障																									
						DeviceStatus.ChangeUnit1 = 1;
                    else if( Hopper1.State & 0x08 ) 
                        DeviceStatus.ChangeUnit1 = 4;
					else if( Hopper1.State & 0x20 )    //光测无币或实际无币							
						DeviceStatus.ChangeUnit1 = 2;  						
					else						
						DeviceStatus.ChangeUnit1 = 0;					
				}	
				else
					DeviceStatus.ChangeUnit1 = 1;
			}
			WaitForWork( 100, NULL );
			break;								
		case 3:       //3 #2退币器测试	

		    #ifdef MACHINE_SET_MDB  
                DeviceStatus.ChangeUnit2 = 0;	  
            	break;            
            #endif

			if( SystemParameter.HopperCoinPrice2 == 0 )
			{
				DeviceStatus.ChangeUnit2 = 1;
				break;
			}		
			if( m_iCheckDevice )
			{				
				flag = HopperOutCoin( 0, 2, &Hopper2 );   //出币5枚, 5-0(Chnaged by Andy on 2011.3.3 )
				WaitForWork( 1500 , NULL );
				if( flag == 0 )//出币命令发送成功
				{					
					flag = GetHardWareDeviceState( 2, &Hopper2 );			
					if( flag != 1 )
					{
						if( Hopper2.State & 0x50 )        //系统故障																									
							DeviceStatus.ChangeUnit2 = 1;
                        else if( Hopper2.State & 0x08 )
                            DeviceStatus.ChangeUnit2 = 4;
						else if( Hopper2.State & 0x20 )   //光测无币或实际无币							
							DeviceStatus.ChangeUnit2 = 2; 						
						else						
							DeviceStatus.ChangeUnit2 = 0;							
					}	
					else
						DeviceStatus.ChangeUnit2 = 1;
				}
				else//出币命令发送不成功				
					DeviceStatus.ChangeUnit2 = 1;				
			}
			else
			{
				flag = GetHardWareDeviceState( 2, &Hopper2 );
				Trace("\n Hopper2 flag = %bu", flag );			
				if( flag != 1 )
				{
					if( Hopper2.State & 0x50 )          //系统故障																									
						DeviceStatus.ChangeUnit2 = 1;
                    else if( Hopper2.State & 0x08 )
                        DeviceStatus.ChangeUnit2 = 4;
					else if( Hopper2.State & 0x20 )     //光测无币或实际无币							
						DeviceStatus.ChangeUnit2 = 2; 						
					else						
						DeviceStatus.ChangeUnit2 = 0;						
				}	
				else
					DeviceStatus.ChangeUnit2 = 1;
			}
			WaitForWork( 100, NULL );
			break;
	   case 4:       //3 #1退币器测试

	        #ifdef MACHINE_SET_MDB
                DeviceStatus.ChangeUnit3 = 0;
            	break;            
            #endif

			if( SystemParameter.HopperCoinPrice3 == 0 )
			{
				DeviceStatus.ChangeUnit3 = 1;
				break;
			}		
			if( m_iCheckDevice )
			{				
				flag = HopperOutCoin( 0, 3, &Hopper3 );   //出币5枚, 5-0(Chnaged by Andy on 2011.3.3 )
				WaitForWork( 1500 , NULL );
				if( flag == 0 )                           //出币命令发送成功
				{					
					flag = GetHardWareDeviceState( 3, &Hopper3 );			
					if( flag != 1 )
					{
						if( Hopper3.State & 0x50 )          //系统故障																									
							DeviceStatus.ChangeUnit3 = 1;
                        else if( Hopper3.State & 0x08 )     //Empty							
							DeviceStatus.ChangeUnit3 = 4; 
						else if( Hopper3.State & 0x20 )     //光测无币或实际无币							
							DeviceStatus.ChangeUnit3 = 2; 						
						else						
							DeviceStatus.ChangeUnit3 = 0;							
					}	
					else
						DeviceStatus.ChangeUnit3 = 1;
				}
				else//出币命令发送不成功				
					DeviceStatus.ChangeUnit3 = 1;				
			}
			else
			{
				flag = GetHardWareDeviceState( 3, &Hopper3 );			
				if( flag != 1 )
				{
					if( Hopper3.State & 0x50 )           //系统故障																									
						DeviceStatus.ChangeUnit3 = 1;
                    else if( Hopper3.State & 0x08 )     //Empty							
						DeviceStatus.ChangeUnit3 = 4; 
					else if( Hopper3.State & 0x20 )      //光测无币或实际无币							
						DeviceStatus.ChangeUnit3 = 2; 						
					else						
						DeviceStatus.ChangeUnit3 = 0;					
				}	
				else
					DeviceStatus.ChangeUnit3 = 1;
			}
			WaitForWork( 100, NULL );			
			break;
       case 10:
	   	    /*
	        {
				if( SystemParameter.busCardOn==0x01 )
				{
				    flag = BCMission_Inq();
				}
				else
				{
					DeviceStatus.BusCard |= 0x80;
				}
			}*/
	        break;
	   default:
	        break;
	}
}

/*Index,设备编号，y，显示行号，PrintFlag打印标志，此标志为1是返回值，为0时显示值*/
u_char DispDeviceStatus( u_char Index, u_char y, bit PrintFlag )
{
	u_char xdata yStateus = 0;
	u_char xdata uline[20];
	u_char xdata line[10];
	u_int  xdata yValue;
	u_char xdata len = 0;
	
	switch( Index )
	{
		case 2://纸币器状态，Bit 7 -- 无此模块；Bit 6~1 保留；Bit 0 -- 故障	
		    /*
            if( DeviceStatus.BillAccepter & 128 )//Bit 7
				yStateus = 2;//无此模块
			else 
			*/
			if( sysMDBMission.billDeviceStatus & 1 )//Bit 7
 		    	yStateus = 1;//故障
			else
				yStateus = 0;//正常
		break;
		case 3://收硬币器状态，Bit 7 -- 无此模块；Bit 6~1 保留；Bit 0 -- 故障；
		    /*
            if( DeviceStatus.CoinAccepter & 128 )//Bit 7
				yStateus = 2;//无此模块
			else
			*/
			if( sysMDBMission.coinDeviceStatus & 1 )//Bit 7
 		    	yStateus = 1;//故障
			else
				yStateus = 0;//正常
		break;
		case 4: // //#1退币器，Bit 7 -- 无此模块；Bit 6~2 保留；Bit 1 -- 缺币；Bit 0 -- 故障；
			/*
            if( DeviceStatus.CoinAccepter & 128 )//Bit 7
				yStateus = 2;//无此模块
			else
			*/
			if( sysMDBMission.coinDeviceStatus & 1 )//Bit 7
 		    	yStateus = 1;//故障
			else
			{ 
				yStateus = 0;//正常
				yValue = sysMDBMission.coin5jValue;//正常
			}
		break;		
		case 5: // //#2退币器，Bit 7 -- 无此模块；Bit 6~2 保留；Bit 1 -- 缺币；Bit 0 -- 故障；
			/*
            if( DeviceStatus.CoinAccepter & 128 )//Bit 7
				yStateus = 2;//无此模块
			else
			*/
			if( sysMDBMission.coinDeviceStatus & 1 )//Bit 7
 		    	yStateus = 1;//故障
			else
			{ 
				yStateus = 0;//正常
				yValue = sysMDBMission.coin1yValue;//正常
			}				
		break;
		case 6://#3退币器，Bit 7 -- 无此模块；Bit 6~2 保留；Bit 1 -- 缺币；Bit 0 -- 故障；
			/*
			if( DeviceStatus.ChangeUnit3 & 128 )//Bit 7
				yStateus = 2;//无此模块
			else
			*/
			if( DeviceStatus.ChangeUnit3 & 6 )//Bit 7
 		    	yStateus = 3;//缺币
			else if( DeviceStatus.ChangeUnit3 & 1 )//Bit 7
 		    	yStateus = 1;//故障
			else
				yStateus = 0;//正常
		break;
		//case 5://手机模块，Bit 7 -- 无此模块；Bit 6~2 保留；Bit 0 -- 故障；
		/*	
		    if( DeviceStatus.MobileStates & 128 )//Bit 7
				yStateus = 2;//无此模块			
			else if( DeviceStatus.MobileStates & 1 )//Bit 7
 		    	yStateus = 1;//故障
			else
				yStateus = 0;//正常
		*/
		//break;
        case 10:
		     if( DeviceStatus.BusCard & 0x80 )
			 {
			 	yStateus = 2;
			 }
			 else if( DeviceStatus.BusCard & 0x03 )
			 {
			    yStateus = 1;  
			 }
			 else
			 {
			    yStateus = 0;
			 }
		break;

		default:
		break;
 	}
	if( PrintFlag == 0 )
	{
		memcpy( uline, WMenuStr[Index].str, WMenuStr[Index].len );	
        if(((Index==4)||(Index==5))&&(yStateus == 0))
        {
		    switch( SystemParameter.curUnit )		
			{
				case 1:
					len += sprintf( line, "%u", yValue );
				break;
				case 10:
					len += sprintf( line, "%u.%u", yValue/SystemParameter.curUnit, yValue%SystemParameter.curUnit );
				break;
				case 100:
					len += sprintf( line, "%u.%02u", yValue/SystemParameter.curUnit, yValue%SystemParameter.curUnit );
				break;
			}
			memcpy( uline + WMenuStr[Index].len, line, len );
			DisplayStr( 0, y, 1, uline, WMenuStr[Index].len + len );
		}
        else
        {
			memcpy( uline + WMenuStr[Index].len, DeviceStatusStr[yStateus].str, DeviceStatusStr[yStateus].len );
		    DisplayStr( 0, y, 1, uline, WMenuStr[Index].len + DeviceStatusStr[yStateus].len );	
		}
        		
	}
	else
		return yStateus;
}



void DispDeviceMenu( u_char Index, u_char y)
{	
//#ifdef _HAVE_CASHER
    /*	
	if( ( Index != 5 ) && ( Index != 7 ) && ( Index != 8 ) && ( Index != 6 ) && ( Index != 9 ) && ( Index != 10 ) && ( Index != 11 ) && ( Index != 13 ) )											
		DispDeviceStatus( Index, y, 0 );						
	else
		DisplayStr( 0, y, 1, WMenuStr[Index].str, WMenuStr[Index].len );
    */    
    //111024 by cq 删除
    //if( (Index==0)||(Index==1)||(Index==2)||(Index==3)||(Index==4)||(Index==0)||(Index==14))
	if( (Index==2)||(Index==3)||(Index==4)||(Index==5))
    {   
		DispDeviceStatus( Index, y, 0 );
    }
    else
    {
    	DisplayStr( 0, y, 1, WMenuStr[Index].str, WMenuStr[Index].len );
    }
						
/*#else
	if( ( Index != 4 ) && ( Index != 5 ) )											
		DispDeviceStatus( Index, y, 0 );						
	else
		DisplayStr( 0, y, 1, WMenuStr[Index].str, WMenuStr[Index].len );
#endif*/
}

//显示当前货道状态
void DispGoodsWaysStates()
{		
	u_char xdata i = 0;
	u_char xdata j = 0;
	u_char xdata z = 0;
					
	u_char  xdata LineNum = 2;  //选中行在屏幕中的行号		
	u_char  xdata key;
	u_char xdata  DispFlag = 1;
	u_char xdata SumColunm = 0;
	u_char xdata m_Index = 0;
    u_char xdata str[20];
	u_char xdata len = 0;
	u_char xdata WayState[GOODSWAYNUM];

	memset( WayState, 0, sizeof(WayState) );

	for( m_Index = 0; m_Index < GOODSWAYNUM; m_Index ++ )
    {
       //WayCheck.WayNo[m_Index] = InputGoodsWayList[m_Index].GoodsWayNo;
       //WayCheck[m_Index].BreakCount = 0;
	    if( GoodsWaySetVal[m_Index].WayState == 0 )
		{
		    //未
		    WayState[m_Index] = 0;
		}
		else if( GoodsWaySetVal[m_Index].WayState & 0x0A )
		{
		    //故
		    WayState[m_Index] = 2;
		}
		else if( (GoodsWaySetVal[m_Index].WayState & 0x04) || (GoodsWaySetVal[m_Index].GoodsCurrentSum == 0) )
		{
		    //缺
		    WayState[m_Index] = 3;
		}
		else
		{
		    //正
		    WayState[m_Index] = 1;
		}
    } 
	LineNum = 2;
	for( i = 0; i < GOODSWAYNUM; i ++ )
	{
		if( InputGoodsWayList[i].GoodsWayNo != 0 )
			SumColunm++;
		else
			break;
	}
	i = 0;
    ClearKey();

	if( DispFlag  )
	{
		#ifdef _CHINA_		    
		    memset( str, 0, sizeof( str ) );
		    len = sprintf( str, "货道%02bu: %s", InputGoodsWayList[i].GoodsWayNo, GoodsWaysStatusStr[ WayState[i] ].str );
	        DisplayStr( 0, 0, 1, str, len );
		    WaitForWork( 100, NULL );

			len = sprintf( str, "货道%02bu: %s", InputGoodsWayList[i+1].GoodsWayNo, GoodsWaysStatusStr[ WayState[i+1] ].str );
	        DisplayStr( 0, 1, 1, str, len );
			WaitForWork( 100, NULL );		
		#else	
	        memset( str, 0, sizeof( str ) );
		    len = sprintf( str, "Column%02bu: %s", InputGoodsWayList[i].GoodsWayNo, GoodsWaysStatusStr[ WayState[i] ].str );
	        DisplayStr( 0, 0, 1, str, len );
		    WaitForWork( 100, NULL );

			len = sprintf( str, "Column%02bu: %s", InputGoodsWayList[i+1].GoodsWayNo, GoodsWaysStatusStr[ WayState[i+1] ].str );
	        DisplayStr( 0, 1, 1, str, len );
			WaitForWork( 100, NULL );
		#endif			
			DisplayCursorState( 0, LineNum-1, 1, 2, 1 );
			DispFlag = 0;
    }
    ClearKey();
	

	while( 1 )    //超时或客户退出
	{        
		key = GetKey();
		switch( key )		
		{
            case KEY_UP://上移                   	
			     if( LineNum == 2 )//选中标记上移
			     {			     
			     	DisplayCursorState( 0, 0, 1, 2, 1 );//第一行整光标
			     	LineNum = 1;
					if (i>1) i--;
			     }
			     else   //菜单上翻
			     {		
		 		 	if( i == 0 )
					{
						break;
				 	}
					else
					{			
					    i--;
					#ifdef _CHINA_	   
					    len = sprintf( str, "货道%02bu: %s", InputGoodsWayList[i].GoodsWayNo, GoodsWaysStatusStr[ WayState[i] ].str );
				        DisplayStr( 0, 0, 1, str, len );
					    
						//如果下一行超出货道直，就不能显示了
                        if( (i+1) >= SumColunm )
                        {
                           ClearDisplayLine( 2 );
                        }
                        else
                        {
							len = sprintf( str, "货道%02bu: %s", InputGoodsWayList[i+1].GoodsWayNo, GoodsWaysStatusStr[ WayState[i+1] ].str );
					        DisplayStr( 0, 1, 1, str, len );	
						}
					#else
						len = sprintf( str, "Column%02bu: %s", InputGoodsWayList[i].GoodsWayNo, GoodsWaysStatusStr[ WayState[i] ].str );
				        DisplayStr( 0, 0, 1, str, len );
					    
						//如果下一行超出货道直，就不能显示了
                        if( (i+1) >= SumColunm )
                        {
                           ClearDisplayLine( 2 );
                        }
                        else
                        {
							len = sprintf( str, "Column%02bu: %s", InputGoodsWayList[i+1].GoodsWayNo, GoodsWaysStatusStr[ WayState[i+1] ].str );
					        DisplayStr( 0, 1, 1, str, len );						
						}
					#endif					
						//DispFlag = 1;
						DisplayCursorState( 0, 0, 1, 2, 1 );//第一行整光标
						LineNum = 1;						
					}
			     }
				 break;
			case KEY_DOWN://下移
                if( i >= SumColunm ) break;//如果i的直超出货道直，则不能再递增了
			    i++;				
				if( LineNum == 1 )//选中标记下移
				{				 
				     DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标					
				     LineNum = 2;				     
				 }
				 else//菜单下翻
				 {	
			     	if( i >= SumColunm )//如果i的直超出货道直，则不能再下翻了
						break;
					else
					{					
					#ifdef _CHINA_	   
					    len = sprintf( str, "货道%02bu: %s", InputGoodsWayList[i].GoodsWayNo, GoodsWaysStatusStr[ WayState[i] ].str );
				        DisplayStr( 0, 0, 1, str, len );
					    
						//如果下一行超出货道直，就不能显示了
                        if( (i+1) >= SumColunm )
                        {
                           ClearDisplayLine( 2 );
                        }
                        else
                        {
						   len = sprintf( str, "货道%02bu: %s", InputGoodsWayList[i+1].GoodsWayNo, GoodsWaysStatusStr[ WayState[i+1] ].str );
				           DisplayStr( 0, 1, 1, str, len );
                        }
                        				
					#else
						len = sprintf( str, "Column%02bu: %s", InputGoodsWayList[i].GoodsWayNo, GoodsWaysStatusStr[ WayState[i] ].str );
				        DisplayStr( 0, 0, 1, str, len );
					    
						if( (i+1) >= SumColunm )
                        {
                           ClearDisplayLine( 2 );
                        }
                        else
                        {
						   len = sprintf( str, "Column%02bu: %s", InputGoodsWayList[i+1].GoodsWayNo, GoodsWaysStatusStr[ WayState[i+1] ].str );
				           DisplayStr( 0, 1, 1, str, len );
						}
		
					#endif						
						//DispFlag = 1;	
						DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标	
						LineNum = 2;					
					}
				}
				break;
		     case KEY_CANCEL:
				return;
				break;					
		}
        ClearKey();	
		CoreProcessCycle(); //让出时间片
		DelayMs( 10 );
		//WaitForWork( 50, NULL );
	}	
}
/*
{
	u_char xdata line1[21];
	u_char xdata line2[21];
	u_char xdata i = 0;
	u_char xdata j = 0;
	u_char xdata z = 0;
#ifdef _CHINA_
	u_char xdata Start = 0;
	u_char xdata End = 10;
#else
	u_char xdata Start = 0;
	u_char xdata End = 8;
#endif					
	u_char  xdata LineNum = 2;  //选中行在屏幕中的行号		
	u_char  xdata key;
	u_char xdata  DispFlag = 1;
	u_char xdata SumColunm = 0;

	key = KEY_NULL;		
	ClearKey();
	DisplayCursorState( 0, 1, 1, 2, 1 );//设置当在第二行上且为整行光标状态
	LineNum = 2;
	for( i = 0; i < GOODSWAYNUM; i ++ )
	{
		if( GoodsWaySetVal[i].WayNo != 0 )
			SumColunm++;
		else
			break;
	}

	while( 1 )    //超时或客户退出
	{
		if( DispFlag )
		{
			memset( line1, 0, sizeof( line1 ) );	
			memset( line2, 0, sizeof( line2 ) );	
			j = 0;
			i = 0;			
			for( i = Start; i < End; i++ )
			{	
			#ifdef _CHINA_
				if( i < ( Start + 5 ) )
			#else
				if( i < ( Start + 4 ) )
			#endif				
				{
					if( GoodsWaySetVal[i].WayNo == 0 )
						continue;
				#ifdef _CHINA_
					sprintf( line1 + 4*j, "%02bu ", GoodsWaySetVal[i].WayNo );
				#else
					sprintf( line1 + 5*j, "%02bu ", GoodsWaySetVal[i].WayNo );
			    #endif
					z = 0;
                    /*
					if( GoodsWaySetVal[i].GoodsCurrentSum == 0 )
						z = 3;
					z = ( GoodsWaySetVal[i].WayState & 0x0f );
					if( ( z & 0x01 ) != 0x01 )						
						z = 0;					
					else if( ( z & 0x01 ) == 0x01 )
						z = 1;
					else if( ( z & 0x02 ) == 0x02 )
						z = 2;
					else if( ( z & 0x04 ) == 0x04 )
						z = 3;	
					else if( ( z & 0x08 ) == 0x08 )
					{
						if( ( z & 0x08) == 0x08 )
							GoodsWaySetVal[i].WayState &= 0xf0;
						z = 4;
					}
                    * /
                    if( GoodsWaySetVal[i].WayState == 0 )
					{
					    //未
					    z = 0;
					}
					else if( GoodsWaySetVal[i].WayState & 0x0A )
					{
					    //故
					    z = 2;
					}
					else if( (GoodsWaySetVal[i].WayState & 0x04) || (GoodsWaySetVal[i].GoodsCurrentSum == 0) )
					{
					    //缺
					    z = 3;
					}
					else
					{
					    //正
					    z = 1;
					}
				#ifdef _CHINA_
					memcpy( line1 + 4*j + 2, GoodsWaysStatusStr[ z ].str, GoodsWaysStatusStr[ z ].len );
				#else
					memcpy( line1 + 5*j + 3, GoodsWaysStatusStr[ z ].str, GoodsWaysStatusStr[ z ].len );
				#endif					
				    //Trace("\n GoodsWaySetVal[i].WayNo = %02bu", GoodsWaySetVal[i].WayNo );		
				    //Trace("\n GoodsWaySetVal[i].WayState = %bu", GoodsWaySetVal[i].WayState );		
				    //Trace("\n line1 = %s", line1 );		
					j++;
				}
				else
				{					
					if( i < SumColunm )
					{					
						if( GoodsWaySetVal[i].WayNo == 0 )
							continue;
					#ifdef _CHINA_
						sprintf( line2 + 4*(j - 5 ), "%02bu ", GoodsWaySetVal[i].WayNo );
					#else
						sprintf( line2 + 5*(j - 4 ), "%02bu ", GoodsWaySetVal[i].WayNo );
			    	#endif						
						z = 0;
                        /*
						if( GoodsWaySetVal[i].GoodsCurrentSum == 0 )
							z = 3;
						z = ( GoodsWaySetVal[i].WayState & 0x0f );
						if( ( z & 0x01 ) != 0x01 )
							z = 0;						
						else if( ( z & 0x01 ) == 0x01 )
							z = 1;
						else if( ( z & 0x02 ) == 0x02 )
							z = 2;
						else if( ( z & 0x04 ) == 0x04 )
							z = 3;
						else if( ( z & 0x08) == 0x08 )
						{
							z = 4;
							if( ( z & 0x08) == 0x08 )
								GoodsWaySetVal[i].WayState &= 0xf0;
						}
                        * /
                        if( GoodsWaySetVal[i].WayState == 0 )
						{
						    //未
						    z = 0;
						}
						else if( GoodsWaySetVal[i].WayState & 0x0A )
						{
						    //故
						    z = 2;
						}
						else if( (GoodsWaySetVal[i].WayState & 0x04) || (GoodsWaySetVal[i].GoodsCurrentSum == 0) )
						{
						    //缺
						    z = 3;
						}
						else
						{
						    //正
						    z = 1;
						}
					#ifdef _CHINA_
						memcpy( line2 + 4*(j - 5 ) + 2, GoodsWaysStatusStr[ z ].str, GoodsWaysStatusStr[ z ].len );
					#else
						memcpy( line2 + 5*(j - 4 ) + 3, GoodsWaysStatusStr[ z ].str, GoodsWaysStatusStr[ z ].len );
					#endif
						j += 1;
					}
					else
						break;
				}				
			}	
			if( ( strlen( line1 ) == 0 ) && ( strlen(line2) == 0 ) )
				return;		
		#ifdef _CHINA_
			DisplayStr( 0, 0, 1, line1, 20 );
			DisplayStr( 0, 1, 1, line2,( j - 5 )*4 );		
		#else
			DisplayStr( 0, 0, 1, line1, 20 );
			DisplayStr( 0, 1, 1, line2,( j - 4 )*5 );		
		#endif			
			DisplayCursorState( 0, LineNum-1, 1, 2, 1 );
			DispFlag = 0;
		}
		key = GetKey();
		switch( key )		
		{
			case KEY_UP://上移  	
			     if( LineNum == 2 )//选中标记上移
			     {			     
			     	DisplayCursorState( 0, 0, 1, 2, 1 );//第一行整光标
			     	LineNum = 1;					
			     }
			     else//菜单上翻
			     {		
		 		 	if( Start <= 0 )
						break;				 	
					else
					{
					#ifdef _CHINA_
						Start = Start -5;					
						End = End - 5;	
					#else
						Start = Start -4;					
						End = End - 4;	
					#endif					
						DispFlag = 1;						
						LineNum = 1;						
					}
			     }
				 break;
			case KEY_DOWN://下移				
				if( LineNum == 1 )//选中标记下移
				{				 
				     DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标					
				     LineNum = 2;				     
				 }
				 else//菜单下翻
				 {	
			     	if( End >= SumColunm )
						break;
					else
					{
					#ifdef _CHINA_
						Start = Start + 5;					
						End = End + 5;	
					#else
						Start = Start + 4;					
						End = End + 4;	
					#endif					
						DispFlag = 1;				
						LineNum = 2;					
					}
				}
				break;
			case KEY_CANCEL:
				return;
				break;					
		}
		CoreProcessCycle();//让出时间片
		DelayMs( 10 );
	}		
	/////////////////////
}
*/
//显示故障货道
void DispFaultGoodsWays()
	
{
	u_char xdata line1[21];
	u_char xdata line2[21];
	u_char xdata i = 0;
	u_char xdata j = 0;
	u_char xdata z = 0;
#ifdef _CHINA_
	u_char xdata Start = 0;
	u_char xdata End = 10;
#else
	u_char xdata Start = 0;
	u_char xdata End = 8;
#endif					
	u_char xdata LineNum = 2;  //选中行在屏幕中的行号		
	u_char xdata key;
	u_char xdata  DispFlag = 1;
	u_char xdata SumColunm = 0;
	u_char xdata ucBadWayNum = 0;//故障货道数量
	u_char xdata str[20];
	u_char xdata len = 0;

	//memset(&WayCheck, 0x00, sizeof(struct COLUMNTEST));//避免清空货道99检测的信息by gzz 20111028
	for( i = 0; i < GOODSWAYNUM; i ++ )
	{
	   //货道状态查询中，显示故障货道次数出错;by gzz 20120323
	   if(( GoodsWaySetVal[i].WayNo != 0 )&&( GoodsWaySetVal[i].WayState & 0x0A ))
	   {
			//只放故障货道
		   //WayCheck.WayNo[ucBadWayNum] = InputGoodsWayList[i].GoodsWayNo;//避免清空货道99检测的信息by gzz 20111028
			ucBadWayNum++;			
	   }
	}	
	memset( str, 0, sizeof( str ) );	
	if(ucBadWayNum == 0)
	{
		len = sprintf( str, "无故障货道");
	}
	else
	{
		len = sprintf( str, "故障货道%02bu个", ucBadWayNum );
	}
	DisplayStr( 0, 0, 1, str, len );
	ClearDisplayLine( 2 );	
	WaitForWork( 3000,NULL );

	key = KEY_NULL; 	
	ClearKey();
	DisplayCursorState( 0, 1, 1, 2, 1 );//设置当在第二行上且为整行光标状态
	LineNum = 2;
	for( i = 0; i < GOODSWAYNUM; i ++ )
	{
		if( InputGoodsWayList[i].GoodsWayNo != 0 )
			SumColunm++;
		else
			break;
	}

	while( 1 )	  //超时或客户退出
	{
		if( DispFlag )
		{
			memset( line1, 0, sizeof( line1 ) );	
			memset( line2, 0, sizeof( line2 ) );	
			j = 0;
			i = 0;			
			for( i = Start; i < End; i++ )
			{	
		#ifdef _CHINA_
				if( i < ( Start + 5 ) )
		#else
				if( i < ( Start + 4 ) )
		#endif				
				{
					if(InputGoodsWayList[i].GoodsWayNo == 0)//修改货道状态查询;by gzz 20120323
						continue;
			#ifdef _CHINA_
					sprintf( line1 + 4*j, "%02bu ", InputGoodsWayList[i].GoodsWayNo );
			#else
					sprintf( line1 + 5*j, "%02bu ", InputGoodsWayList[i].GoodsWayNo );
		    #endif
					z = 0;
					//注释掉原来货道状态判断函数;by gzz 20110309
					/*
					if( GoodsWaySetVal[i].GoodsCurrentSum == 0 )
						z = 3;
					z = ( GoodsWaySetVal[i].WayState & 0x0f );
					if( ( z & 0x01 ) != 0x01 )						
						z = 0;					
					else if( ( z & 0x01 ) == 0x01 )
						z = 1;
					else if( ( z & 0x02 ) == 0x02 )
						z = 2;
					else if( ( z & 0x04 ) == 0x04 )
						z = 3;	
					else if( ( z & 0x08 ) == 0x08 )
					{
						if( ( z & 0x08) == 0x08 )
							GoodsWaySetVal[i].WayState &= 0xf0;
						z = 4;
					}
					*/
					//添加新货道状态判断函数;by gzz 20110309
					if(( GoodsWaySetVal[i].WayState & 0x01 )!= 0x01)
					{
						//未
						z = 0;
					}
					else if( GoodsWaySetVal[i].WayState & 0x0A )
					{
						//故
						z = 2;
					}
					else if( (GoodsWaySetVal[i].WayState & 0x04) || (GoodsWaySetVal[i].GoodsCurrentSum == 0) )
					{
						//缺
						z = 3;
					}
					else
					{
						//正
						z = 1;
					}
			#ifdef _CHINA_
					memcpy( line1 + 4*j + 2, GoodsWaysStatusStr[ z ].str, GoodsWaysStatusStr[ z ].len );
			#else
					memcpy( line1 + 5*j + 3, GoodsWaysStatusStr[ z ].str, GoodsWaysStatusStr[ z ].len );
			#endif					
				//	Trace("\n GoodsWaySetVal[i].WayNo = %02bu", GoodsWaySetVal[i].WayNo );		
				//	Trace("\n GoodsWaySetVal[i].WayState = %bu", GoodsWaySetVal[i].WayState );		
				//	Trace("\n line1 = %s", line1 ); 	
					j++;
				}
				else
				{					
					if( i < SumColunm )
					{					
						if( InputGoodsWayList[i].GoodsWayNo == 0)//修改货道状态查询;by gzz 20120323
							continue;
				#ifdef _CHINA_
						sprintf( line2 + 4*(j - 5 ), "%02bu ", InputGoodsWayList[i].GoodsWayNo );
				#else
						sprintf( line2 + 5*(j - 4 ), "%02bu ", InputGoodsWayList[i].GoodsWayNo );
		    	#endif						
						z = 0;
						//注释掉原来货道状态判断函数;by gzz 20110309
						/*
						if( GoodsWaySetVal[i].GoodsCurrentSum == 0 )
							z = 3;
						z = ( GoodsWaySetVal[i].WayState & 0x0f );
						if( ( z & 0x01 ) != 0x01 )
							z = 0;						
						else if( ( z & 0x01 ) == 0x01 )
							z = 1;
						else if( ( z & 0x02 ) == 0x02 )
							z = 2;
						else if( ( z & 0x04 ) == 0x04 )
							z = 3;
						else if( ( z & 0x08) == 0x08 )
						{
							z = 4;
							if( ( z & 0x08) == 0x08 )
								GoodsWaySetVal[i].WayState &= 0xf0;
						}
						*/
						//添加新货道状态判断函数;by gzz 20110309
						if(( GoodsWaySetVal[i].WayState & 0x01 )!= 0x01)
						{
							//未
							z = 0;
						}
						else if( GoodsWaySetVal[i].WayState & 0x0A )
						{
							//故
							z = 2;
						}
						else if( (GoodsWaySetVal[i].WayState & 0x04) || (GoodsWaySetVal[i].GoodsCurrentSum == 0) )
						{
							//缺
							z = 3;
						}
						else
						{
							//正
							z = 1;
						}
				#ifdef _CHINA_
						memcpy( line2 + 4*(j - 5 ) + 2, GoodsWaysStatusStr[ z ].str, GoodsWaysStatusStr[ z ].len );
				#else
						memcpy( line2 + 5*(j - 4 ) + 3, GoodsWaysStatusStr[ z ].str, GoodsWaysStatusStr[ z ].len );
				#endif
						j += 1;
					}
					else
						break;
				}				
			}	
			if( ( strlen( line1 ) == 0 ) && ( strlen(line2) == 0 ) )
				return; 	
	#ifdef _CHINA_
			DisplayStr( 0, 0, 1, line1, 20 );
			DisplayStr( 0, 1, 1, line2,( j - 5 )*4 );		
	#else
			DisplayStr( 0, 0, 1, line1, 20 );
			DisplayStr( 0, 1, 1, line2,( j - 4 )*5 );		
	#endif			
			DisplayCursorState( 0, LineNum-1, 1, 2, 1 );
			DispFlag = 0;
		}
		key = GetKey();
		switch( key )		
		{
			case KEY_UP://上移		
				 if( LineNum == 2 )//选中标记上移
				 {				 
					DisplayCursorState( 0, 0, 1, 2, 1 );//第一行整光标
					LineNum = 1;					
				 }
				 else//菜单上翻
				 {		
					if( Start <= 0 )
						break;					
					else
					{
				#ifdef _CHINA_
						Start = Start -5;					
						End = End - 5;	
				#else
						Start = Start -4;					
						End = End - 4;	
				#endif					
						DispFlag = 1;						
						LineNum = 1;						
					}
				 }
				 break;
			case KEY_DOWN://下移				
				if( LineNum == 1 )//选中标记下移
				{				 
					 DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标 				
					 LineNum = 2;					 
				 }
				 else//菜单下翻
				 {	
					if( End >= SumColunm )
						break;
					else
					{
				#ifdef _CHINA_
						Start = Start + 5;					
						End = End + 5;	
				#else
						Start = Start + 4;					
						End = End + 4;	
				#endif					
						DispFlag = 1;				
						LineNum = 2;					
					}
				}
				break;
			case KEY_CANCEL:
				return;
				break;					
		}
		CoreProcessCycle();//让出时间片
		DelayMs( 10 );
		if(sysVPMission.msActTimer == 0)
		{
			sysVPMission.msActTimer = 100;
			VPMission_Act_RPT(VP_ACT_ADMIN,VP_ACT_ENTERADMIN);
		}
	}		
	/////////////////////
}


/*
{		
	u_char xdata i = 0;
	u_char xdata j = 0;
	u_char xdata z = 0;
					
	u_char  xdata LineNum = 2;  //选中行在屏幕中的行号		
	u_char  xdata key;
	u_char xdata  DispFlag = 1;
	u_char xdata SumColunm = 0;
	u_char xdata m_Index = 0;
    u_char xdata str[20];
	u_char xdata len = 0;

	u_char xdata ucIsBadWay = 0;//货道是否故障
	u_char xdata ucBadWayNum = 0;//故障货道数量

	memset(&WayCheck, 0x00, sizeof(struct COLUMNTEST));
	for( m_Index = 0; m_Index < GOODSWAYNUM; m_Index ++ )
    {
       if( GoodsWaySetVal[m_Index].WayState & 0x0A )
	   {
		   	//只放故障货道
	       WayCheck.WayNo[ucBadWayNum] = InputGoodsWayList[m_Index].GoodsWayNo;
			ucBadWayNum++;
			if(ucBadWayNum%7 == 0) SumColunm++;
	   }
    } 	
	SumColunm++;
	memset( str, 0, sizeof( str ) );
	if(ucBadWayNum == 0)
	{
		len = sprintf( str, "无故障货道");
		DisplayStr( 0, 0, 1, str, len );
		ClearDisplayLine( 2 );		
		WaitForWork( 3000,NULL );		
		ClearKey();
		return;

	}
	
	len = sprintf( str, "故障货道%02bu个", ucBadWayNum );
	DisplayStr( 0, 0, 1, str, len );
	ClearDisplayLine( 2 );	

	key = KEY_NULL;		
	ClearKey();
	DisplayCursorState( 0, 1, 1, 2, 1 );//设置当在第二行上且为整行光标状态
	LineNum = 2;
	WaitForWork( 3000,NULL );
    i = 0;
    ClearKey();

	if( DispFlag  )
	{
		#ifdef _CHINA_		    
		    memset( str, 0, sizeof( str ) );
			//len = sprintf( str, "货道%02bu故障", WayCheck.WayNo[i]);
			len = sprintf( str, "%02bu %02bu %02bu %02bu %02bu %02bu %02bu", WayCheck.WayNo[i*7+0], WayCheck.WayNo[i*7+1], WayCheck.WayNo[i*7+2], WayCheck.WayNo[i*7+3], WayCheck.WayNo[i*7+4], WayCheck.WayNo[i*7+5], WayCheck.WayNo[i*7+6]);
	        DisplayStr( 0, 0, 1, str, len );		    
			len = sprintf( str, "%02bu %02bu %02bu %02bu %02bu %02bu %02bu", WayCheck.WayNo[i*7+0+7], WayCheck.WayNo[i*7+1+7], WayCheck.WayNo[i*7+2+7], WayCheck.WayNo[i*7+3+7], WayCheck.WayNo[i*7+4+7], WayCheck.WayNo[i*7+5+7], WayCheck.WayNo[i*7+6+7]);
	        DisplayStr( 0, 1, 1, str, len );					
		#else	
	        memset( str, 0, sizeof( str ) );
			len = sprintf( str, "%02bu %02bu %02bu %02bu %02bu %02bu %02bu", WayCheck.WayNo[i*7+0], WayCheck.WayNo[i*7+1], WayCheck.WayNo[i*7+2], WayCheck.WayNo[i*7+3], WayCheck.WayNo[i*7+4], WayCheck.WayNo[i*7+5], WayCheck.WayNo[i*7+6]);
			DisplayStr( 0, 0, 1, str, len );			
			len = sprintf( str, "%02bu %02bu %02bu %02bu %02bu %02bu %02bu", WayCheck.WayNo[i*7+0+7], WayCheck.WayNo[i*7+1+7], WayCheck.WayNo[i*7+2+7], WayCheck.WayNo[i*7+3+7], WayCheck.WayNo[i*7+4+7], WayCheck.WayNo[i*7+5+7], WayCheck.WayNo[i*7+6+7]);
			DisplayStr( 0, 1, 1, str, len );					
		#endif			
			DisplayCursorState( 0, LineNum-1, 1, 2, 1 );
			DispFlag = 0;
    }
    ClearKey();
	

	while( 1 )    //超时或客户退出
	{        
		key = GetKey();
		//if( key == KEY_NULL ) continue;
		switch( key )		
		{
            case KEY_UP://上移                   	
			     if( LineNum == 2 )//选中标记上移
			     {			     
			     	DisplayCursorState( 0, 0, 1, 2, 1 );//第一行整光标
			     	LineNum = 1;
					if (i>1) i--;
			     }
			     else   //菜单上翻
			     {		
		 		 	if( i == 0 )
					{
						break;
				 	}
					else
					{			
					    i--;
					#ifdef _CHINA_	   
					len = sprintf( str, "%02bu %02bu %02bu %02bu %02bu %02bu %02bu", WayCheck.WayNo[i*7+0], WayCheck.WayNo[i*7+1], WayCheck.WayNo[i*7+2], WayCheck.WayNo[i*7+3], WayCheck.WayNo[i*7+4], WayCheck.WayNo[i*7+5], WayCheck.WayNo[i*7+6]);
				        DisplayStr( 0, 0, 1, str, len );
					    
						//如果下一行超出货道直，就不能显示了
						if( (i+1) >= SumColunm )							
                        {
                           ClearDisplayLine( 2 );
                        }
                        else
                        {
							len = sprintf( str, "%02bu %02bu %02bu %02bu %02bu %02bu %02bu", WayCheck.WayNo[i*7+0+7], WayCheck.WayNo[i*7+1+7], WayCheck.WayNo[i*7+2+7], WayCheck.WayNo[i*7+3+7], WayCheck.WayNo[i*7+4+7], WayCheck.WayNo[i*7+5+7], WayCheck.WayNo[i*7+6+7]);
							DisplayStr( 0, 1, 1, str, len );	
						}
					#else
					len = sprintf( str, "%02bu %02bu %02bu %02bu %02bu %02bu %02bu", WayCheck.WayNo[i*7+0], WayCheck.WayNo[i*7+1], WayCheck.WayNo[i*7+2], WayCheck.WayNo[i*7+3], WayCheck.WayNo[i*7+4], WayCheck.WayNo[i*7+5], WayCheck.WayNo[i*7+6]);
						DisplayStr( 0, 0, 1, str, len );
					    
						//如果下一行超出货道直，就不能显示了
						if( (i+1) >= SumColunm )
                        {
                           ClearDisplayLine( 2 );
                        }
                        else
                        {
							len = sprintf( str, "%02bu %02bu %02bu %02bu %02bu %02bu %02bu", WayCheck.WayNo[i*7+0+7], WayCheck.WayNo[i*7+1+7], WayCheck.WayNo[i*7+2+7], WayCheck.WayNo[i*7+3+7], WayCheck.WayNo[i*7+4+7], WayCheck.WayNo[i*7+5+7], WayCheck.WayNo[i*7+6+7]);
							DisplayStr( 0, 1, 1, str, len );						
						}
					#endif					
						//DispFlag = 1;
						DisplayCursorState( 0, 0, 1, 2, 1 );//第一行整光标
						LineNum = 1;						
					}
			     }
				 break;
			case KEY_DOWN://下移
                if( i >= SumColunm ) break;//如果i的直超出货道直，则不能再递增了
			    i++;				
				if( LineNum == 1 )//选中标记下移
				{				 
				     DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标					
				     LineNum = 2;				     
				 }
				 else//菜单下翻
				 {	
					if( i >= SumColunm ) break;//如果i的直超出货道直，则不能再递增了
					else
					{					
					#ifdef _CHINA_	   
					len = sprintf( str, "%02bu %02bu %02bu %02bu %02bu %02bu %02bu", WayCheck.WayNo[i*7+0], WayCheck.WayNo[i*7+1], WayCheck.WayNo[i*7+2], WayCheck.WayNo[i*7+3], WayCheck.WayNo[i*7+4], WayCheck.WayNo[i*7+5], WayCheck.WayNo[i*7+6]);
						DisplayStr( 0, 0, 1, str, len );
					    
						//如果下一行超出货道直，就不能显示了
						if( (i+1) >= SumColunm )
                        {
                           ClearDisplayLine( 2 );
                        }
                        else
                        {
							len = sprintf( str, "%02bu %02bu %02bu %02bu %02bu %02bu %02bu", WayCheck.WayNo[i*7+0+7], WayCheck.WayNo[i*7+1+7], WayCheck.WayNo[i*7+2+7], WayCheck.WayNo[i*7+3+7], WayCheck.WayNo[i*7+4+7], WayCheck.WayNo[i*7+5+7], WayCheck.WayNo[i*7+6+7]);
						   DisplayStr( 0, 1, 1, str, len );
                        }
                        				
					#else
					len = sprintf( str, "%02bu %02bu %02bu %02bu %02bu %02bu %02bu", WayCheck.WayNo[i*7+0], WayCheck.WayNo[i*7+1], WayCheck.WayNo[i*7+2], WayCheck.WayNo[i*7+3], WayCheck.WayNo[i*7+4], WayCheck.WayNo[i*7+5], WayCheck.WayNo[i*7+6]);
						DisplayStr( 0, 0, 1, str, len );
					    
						if( (i+1) >= SumColunm )
                        {
                           ClearDisplayLine( 2 );
                        }
                        else
                        {
							len = sprintf( str, "%02bu %02bu %02bu %02bu %02bu %02bu %02bu", WayCheck.WayNo[i*7+0+7], WayCheck.WayNo[i*7+1+7], WayCheck.WayNo[i*7+2+7], WayCheck.WayNo[i*7+3+7], WayCheck.WayNo[i*7+4+7], WayCheck.WayNo[i*7+5+7], WayCheck.WayNo[i*7+6+7]);
						   DisplayStr( 0, 1, 1, str, len );
						}
		
					#endif						
						//DispFlag = 1;	
						DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标	
						LineNum = 2;					
					}
				}
				break;
		     case KEY_CANCEL:
				return;
				break;					
		}
        ClearKey();	
		CoreProcessCycle(); //让出时间片
		DelayMs( 10 );
		//WaitForWork( 50, NULL );
	}	
}
*/


//测试和显示手机模块状态
void TestAndDispMobileStates( u_char type, u_char DispLine )
{	
	type = 0;
    DispLine = 0;
}

void ErrShow()
{
	//u_int xdata  CurrentMoney = 0;

	hardWareErrShow[0]=0;
	hardWareErrShow[1]=0;
	
	hardWareErrShow[1]=sysVPMission.hardWareErrShow;
	if(sysVPMission.hardWareErrShow==0x02)
		hardWareErrShow[0]=0x70;
	//本次出货确认板的工作状态;by gzz 20111028 
    if( SystemParameter.GOCOn == 1 )
    {
        if( DeviceStatus.GOCStatus == 0 )
        {
        	hardWareErrShow[0]=0x00;
        	hardWareErrShow[1]=0xfc;
        }     	
    }
	//纸币器故障
	//具体故障情况
	//钞箱已满							
	if(sysMDBMission.billIsFull == 1)
	{
		hardWareErrShow[0] = 0x60;
		hardWareErrShow[1] = 0x08;
	}
	//钞箱移除报故障
	if(sysMDBMission.billCashBuf == 1)//钞箱移除报故障;by gzz 20121224
    {		
		{
			hardWareErrShow[0] = 0x60;
			hardWareErrShow[1] = 0x07;
		}
	}
	//电机异常
	if(sysMDBMission.billOthErr & 0x01)
	{
		hardWareErrShow[0] = 0x60;
		hardWareErrShow[1] = 0x03;
	}
	//传感器异常
	if(sysMDBMission.billOthErr & 0x02)
	{
		hardWareErrShow[0] = 0x60;
		hardWareErrShow[1] = 0x04;
	}
	//卡币
	if(sysMDBMission.billOthErr & 0x08)
	{
		hardWareErrShow[0] = 0x60;
		hardWareErrShow[1] = 0x06;
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
		hardWareErrShow[0] = 0x50;
		hardWareErrShow[1] = 0x04;
	}
	//传感器异常
	if(sysMDBMission.coinOthErr & 0x01)
	{
		hardWareErrShow[0] = 0x50;
		hardWareErrShow[1] = 0x03;
	}
	//出币卡币
	if(sysMDBMission.coinOthErr & 0x02)
	{
		hardWareErrShow[0] = 0x50;
		hardWareErrShow[1] = 0x05;
	}
	//进币通道出错
	if(sysMDBMission.coinOthErr & 0x08)
	{
		hardWareErrShow[0] = 0x50;
		hardWareErrShow[1] = 0x07;
	}
	//投币卡币
	if(sysMDBMission.coinOthErr & 0x10)
	{
		hardWareErrShow[0] = 0x50;
		hardWareErrShow[1] = 0x08;
	}	 
}

void DealStatusQuery()
{
	u_char xdata len = 0;	
    //u_char xdata StatusBuf[10];
	u_char xdata Buffer[20];
	u_char xdata key = 0;
	u_char xdata MyKey = 0;
    /*
	//无故障时，直接提示运行正常;by gzz 20111028
	if(sysVPMission.hardWareErrShow==0)
        sprintf( StatusBuf, "%s", "运行\xd5\xfd常" );
		//DisplayStr( 0, 0 , 1 , "      运行\xd5\xfd常", 14 );
	else
        sprintf( StatusBuf, "%s", "故障代码" );
		//DisplayStr( 0, 0 , 1 , "      故障代码", 14 );
    */
    ErrShow();
	len = sprintf( Buffer, "[%02bx%02bx]" , hardWareErrShow[0],hardWareErrShow[1] );	
	        
	//Trace( "\n LEN = %bu", len );
	DisplayStr( 0, 0 , 1, Buffer, len );
    WaitForWork( 100, NULL );
	
	//本次出货确认板的工作状态;by gzz 20111028 
    if( SystemParameter.GOCOn == 1 )
    {
        if( DeviceStatus.GOCStatus == 0 )
        {
        	len = sprintf( Buffer, "出货确认板开启");
        }
     	else
        {
        	len = sprintf( Buffer, "出货确认板故障关闭");
        }
    }
    else
	{
		len = sprintf( Buffer, "出货确认板未开启");
	}
	DisplayStr( 0, 1 , 1, Buffer, len );
	WaitForWork( 100, NULL );
	
    while( 1 )
	{
    	MyKey = GetKey();		
		if( ( MyKey  == KEY_SUBMIT ) || ( MyKey  == KEY_CANCEL ) )
        {
			break;
        }
		WaitForWork( 100, NULL );
    } 	
}

void DeviceCheck()
{
    //菜单显示数组   
	unsigned char  xdata firstNum; //一级菜单编号	
	unsigned char  xdata LineNum;  //选中行在屏幕中的行号		
	unsigned char  xdata key;
	u_char   xdata status;
	bit	data Notout;
	unsigned char xdata line[20];

	memset(line , 0 , sizeof( line ));
	key = KEY_NULL;
	status = 0;
	firstNum = 1;
	LineNum = 2;	
	Notout = 1;

	ClearKey();
	/*菜单初始化显示（第1，2行）*/	  	
//	DisplayStr( 0, 0, 1, WMenuStr[0].str, WMenuStr[0].len );		
//#ifndef _HAVE_CASHER
	m_iCheckDevice = 0;
	/*
	if( SystemParameter.BillNo != 1 )
//		GetDeviceStatus( 0 );	
//	else
		DeviceStatus.BillAccepter = 128;
//	GetDeviceStatus( 1 );	
//#endif
    //测试一下各找零器的状态值;by gzz 20110430
    //GetDeviceStatus(2);
    //GetDeviceStatus(3);
    //GetDeviceStatus(4);
    */
   
	//DispDeviceStatus( 0, 0, 0 );	
	//DispDeviceStatus( 1, 1, 0 );
	DispDeviceMenu( 0, 0);
	DispDeviceMenu( 1, 1);
	DisplayCursorState( 0, 1, 1, 2, 1 );//设置当在第二行上且为整行光标状态
	//Trace( "in Menu select begin\n" );		
	m_iCheckDevice = 1;
	while( Notout )    //超时或客户退出
	{				
		key = GetKey();
		switch( key )		
		{
			case KEY_UP://上移  	
			     if( LineNum == 2 )//选中标记上移
			     {			     
			     	DisplayCursorState( 0, 0, 1, 2, 1 );//第一行整光标
			     	LineNum = 1;			   
					if( firstNum == 0 )
						firstNum = MENU_COUNT(WMenuStr)-1;					
					else
						firstNum--;
			     }
			     else//菜单上翻
			     {
			      	/*当firstNum<=1不能上翻了*/			      	
					DispDeviceMenu( firstNum, 1);									    
					if( firstNum == 0 )
						firstNum = MENU_COUNT(WMenuStr)-1;					
					else
						firstNum--;
					DispDeviceMenu( firstNum, 0);						
					LineNum = 1;					
			     }
				 break;
			case KEY_DOWN://下移				
				if( LineNum == 1 )//选中标记下移
				{				 
				     DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标				   
					if( firstNum == MENU_COUNT(WMenuStr)-1 )
						firstNum = 0;					
					else
						firstNum++;
				     LineNum = 2;				     
				 }
				 else//菜单下翻
				 {
				    /*当firstNum<=2不能上翻了*/				    				
					DispDeviceMenu( firstNum, 0);						
			      	if( firstNum == MENU_COUNT(WMenuStr)-1 )
						firstNum = 0;					
					else
						firstNum++;
					DispDeviceMenu( firstNum, 1);		  
					LineNum = 2;					
				}
				break;
			case KEY_CANCEL:
				Notout = 0;
				break;
			case KEY_SUBMIT://Enter,选中了某项菜单	
				DisplayCursorState( 0, 0, 0, 0, 1 );//第二行整光标
				ClearKey();				
				switch(firstNum)
				{			
					case 0:       
							//在设备管理菜单上，添加一个交易状态查询，可以查询上次交易模式时的故障状态，
	                        //同时显示故障状态信息
	                        DealStatusQuery();
							break;
					case 1:       
					       //货道测试
						   GoodsWayTest();
                       	   VPMission_ColumnSta_RPT();//by gzz 20120223	                       	   
						   break;
					case 2:    						   
						   //1纸币器状态
						   if( sysITLMission.ITLSet==1 )
						   {
	                        	//ITLBillATest();	
								MDBBillATest();
	                        	break;
						   }
						   break;
					case 3:    						   
						   //2硬币器状态//
					       MDBCoinATest();
						   break;
					/*
					case 4:     //#3退币器测试
						GetDeviceStatus( firstNum );						
						break;	
					case 5: //用AT指令测试主板与手机模块的连接情况										
					case 6: //发送短信测试
						//TestAndDispMobileStates( firstNum - 4, LineNum );
					break;
					case 7://货道状态
						//DispGoodsWaysStates();	
						break;	
					*/	
                    case 4:  
	                    //Trace("\n GoodsSetSave1 = %bu", GoodsSetSave );
	                    //#1退币器测试
					    MDBHopperATest(1);
						MDB_Coin_TubeSta_API();
						break;				
					case 5:       
						//#2退币器测试
					    MDBHopperATest(2);
                        MDB_Coin_TubeSta_API();
	                    //Trace("\n GoodsSetSave1 = %bu", GoodsSetSave );
						break;					
					//case 6:
					    //DispGoodsCheckValue();
                        //break; 
					case 6:
					    //sysVPMission.VPMode = 1;
					    ButtonTest();
                        //sysVPMission.VPMode = 0;
					    break;
					case 7:
					    GameButtonTest();
					    break;					
					case 8:
					    ReturnButtonTest();
					    break;				
                    //case 13:
                    //    ICCardTest();
                    //    break;		
					case 9:
						//GetDeviceStatus( firstNum );
						//出货确认测试
						GOCTest();
						break;						
					case 10:       //led test
					    ACDCModuleTest(1);
                        break; 
					case 11:       //goods show compressor test
					    ACDCModuleTest(2);
                        break; 					
                    default:
                        break;
					
				}
				if(LineNum ==2)
				{	
					if( firstNum == 0 )						
						DispDeviceMenu( MENU_COUNT(WMenuStr)-1, 0);
					else						
						DispDeviceMenu( firstNum - 1, 0);
					DispDeviceMenu( firstNum, 1);					
				}
				else
				{												
					if( firstNum == ( MENU_COUNT(WMenuStr)-1 ) )
						DispDeviceMenu( 0, 1);					
					else						
						DispDeviceMenu( firstNum + 1, 1);					
					DispDeviceMenu( firstNum, 0);
				}				
				DisplayCursorState( 0, LineNum - 1, 1, 2, 1 );//第二行整光标
				ClearKey();	
				break;
		}
		CoreProcessCycle();//让出时间片
		DelayMs( 10 );
		if(sysVPMission.msActTimer == 0)
		{
			sysVPMission.msActTimer = 100;
			VPMission_Act_RPT(VP_ACT_ADMIN,VP_ACT_ENTERADMIN);
		}
	}		
	m_iCheckDevice = 0;
}

