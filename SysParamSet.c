#define SYSPARAM_C
#include "device.h"
#include "global.h"
#include "scheduler.h"
#include "CommonFunction.h"
#include "STRING.h"
#include "SysParamSet.h"
#include "key.h"
#include "common.h"
#include "mainflow.h"
#include "debug.h"
#include "VMC_PC.h"


//#undef _DEBUG_TRACE
/***********************************/
#ifdef _CHINA_
	const struct StrAndLen code SysMenustr[] =
	{	
		/*
		{ "1.#1找零器面值:", 15 },	
		{ "2.#2找零器面值:", 15 },		
		//{ "3.#3找零器面值:", 15 }, //111024 by cq 删除
		{ "3.允许多次购买:", 15 },
		{ "4.不买货退币:", 13 },
		{ "5.开启纸币器:", 13 },        
		//{ "7.开启手机模块:", 15 },	 //111024 by cq 删除
        { "6.余额下限:", 11 },
        { "7.收币上限:", 11 },	
        //最后一次纸币有疑问的收币金额
		//{ "10. 疑问金额:", 13 },//111024 by cq 删除
		//{ "11. 机器编号: ", 13 },//111024 by cq 删除
		//{ "12.GSM号码:", 11 },//111024 by cq 删除
		//{ "1X. 欠条:", 9 },//111024 by cq 删除
		{ "8.欠条:", 7 },//111024 by cq 删除
		//-----------------------------------------------------
        //{ "1X. 小\xca\xfd点:", 11},//111024 by cq 删除
		{ "9.小\xca\xfd点:", 9 },//111024 by cq 删除
        { "10. #1纸币:", 11 },
        { "11. #2纸币:", 11 },
        { "12. #3纸币:", 11 },
        { "13. #4纸币:", 11 },
        { "14. #5纸币:", 11 },
        { "15. #6纸币:", 11 },
        { "16. #7纸币:", 11 },
        { "17. #8纸币:", 11 },
        { "18. #1硬币:", 11 },
        { "19. #2硬币:", 11 },
        { "20. #3硬币:", 11 },
        { "21. #4硬币:", 11 },
        { "22. #5硬币:", 11 },
        { "23. #6硬币:", 11 },
        { "24. 开启出货确认:", 17 },
      //{ "30. 服务无出货确认:", 19 },
	    { "25. 交易时间:", 13 },
        { "26. 不找零模式:", 15 },
        { "27. 开启硬币器:", 15 },
        { "28. 开启公交卡:", 15 }, 
        { "29. 读卡时间:",   13 },
        { "30. 暂存纸币模式:",   17 },
        { "31. 五元纸币暂存:",   17 },
        { "32. 十元纸币暂存:",   17 },
        { "33. 开启ACDC模块:",   17 },
        { "34. 开启制冷:",   13 },
        */
        /*
        { "1.开启制冷:",   11 },
        { "2.自动退币:", 11 }, 
        { "3.开启出货确认:", 15 },
        { "4.欠条:", 7 },//111024 by cq 删除 
		{ "5.允许多次购买:", 15 },
		{ "6.不买货退币:", 13 },
        { "7.余额下限:", 11 },
        { "8.收币上限:", 11 },
		{ "9.开启纸币器:", 13 },
        { "10. 开启硬币器:", 15 },        
		{ "11. 开启ACDC模块:",   17 },	
		//-----------------------------------------------------
        { "12. 小\xca\xfd点:", 11 },//111024 by cq 删除
        { "13. #1找零面值:", 15 },	
		{ "14. #2找零面值:", 15 }, 
        { "15. #1纸币:", 11 },
        { "16. #2纸币:", 11 },
        { "17. #3纸币:", 11 },
        { "18. #4纸币:", 11 },
        { "19. #5纸币:", 11 },
        { "20. #6纸币:", 11 },
        { "21. #7纸币:", 11 },
        { "22. #8纸币:", 11 },
        { "23. #1硬币:", 11 },
        { "24. #2硬币:", 11 },
        { "25. #3硬币:", 11 },
        { "26. #4硬币:", 11 },
        { "27. #5硬币:", 11 },
        { "28. #6硬币:", 11 },
        //{ "29. 不找零模式:", 15 },
        //{ "30. 开启公交卡:", 15 }, 
        //{ "31. 读卡时间:",   13 }, 
        */
        { "1.开启制冷:",   11 },
        { "2.自动退币:", 11 }, 
        { "3.开启出货确认:", 15 },
        { "4.欠条:", 7 },//111024 by cq 删除 
		{ "5.允许多次购买:", 15 },
		{ "6.不买货退币:", 13 },
        { "7.余额下限:", 11 },
        { "8.开启纸币器:", 13 },
        { "9.开启硬币器:", 13 },        
		{ "10. 开启ACDC模块:",   17 },	
		//-----------------------------------------------------
        //{ "11. #1找零面值:", 15 },	
		//{ "12. #2找零面值:", 15 }, 
        { "11. #1纸币:", 11 },
        { "12. #2纸币:", 11 },
        { "13. #3纸币:", 11 },
        { "14. #4纸币:", 11 },
        { "15. #5纸币:", 11 },
        { "16. #6纸币:", 11 },
        { "17. #1硬币:", 11 },
        { "18. #2硬币:", 11 },
        { "19. #3硬币:", 11 },
        { "20. #4硬币:", 11 },
        //=====================================================
	};
#else
	const struct StrAndLen code SysMenustr[] =
	{	        
		{ "1.#1 coin type:", 15 },	
		{ "2.#2 coin type:", 15 },		
		//{ "3.#3 coin type:", 15 },   //111024 by cq 删除     
		{ "3.Multi-verd:", 13 },//是否允许多次购物		
		{ "4.Refund:", 9 },//是否允许不买货退钱	
		{ "5.Bill on:", 10 },
		//{ "7.Mobile on:", 12 },
		{ "7.Change value:", 15 },	
		{ "8.Banknote-max:", 15 },	
		//有疑问的金额
		//{ "10.Doubtful Amt:", 16 },//111024 by cq 删除
		//{ "11.VM ID:", 9 },//111024 by cq 删除
		//{ "12.GSM No.:", 11 },//111024 by cq 删除				
		{ "8.IOU:", 7 },
		//-----------------------------------------------------
		{ "9.Decimal num: ", 15 },
		
        { "10.#1 Note:", 11 },
        { "11.#2 Note:", 11 },
        { "12.#3 Note:", 11 },
        { "13.#4 Note:", 11 },
        { "14.#5 Note:", 11 },
        { "15.#6 Note:", 11 },
        { "16.#7 Note:", 11 },
        { "17.#8 Note:", 11 },
        { "18.#1 Coin:", 11 },
        { "19.#2 Coin:", 11 },
        { "20.#3 Coin:", 11 },
        { "21.#4 Coin:", 11 },
        { "22.#5 Coin:", 11 },
        { "23.#6 Coin:", 11 },
		{ "24.GOC_dev on:",  14 },
	  //{ "30.SVC/GOC ERR:", 15 },
        { "25.Trade time:", 14 },
        { "26.No change mode:", 18 },        
        //=====================================================
        
	};
#endif
//const code SysInputLen[] = { 5, 5, 5, 1, 1, 1, 1, 5, 5, 5, 5, 16, 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 5, 1, 1, 1, 2, 1, 1, 1, 1 /*, 1*/ };
//							  1,,,2,,,,,,,,,,,,,3,,,4,,,5,,,,,,,,,,,,,,6,,,7,,,,,,,,,,,,,,,,,,,,,,,,8,,,9,,10,11,12,13,14,15,16,17,18,19,20,21,,22,,23,24,25,26,27,28,29,,30,31,32,33
const code SysInputLen[] = { 5, 5,/*5,*/ 1, 1, 1, /*1,*/ 5, 5,/* 5, 5, 16,*/ 5, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 5, 1, 1, 1, 2, 1, 1, 1, 1 /*, 1*/ };//111024 by cq 删除
bit  data  SystemSave = 0;//系统是否保存
u_char  xdata y_displaystr[44];	//主显示串
//u_char  xdata y_displaystr_1[22];	//备用显示串
//读\写取内存中的系统参数,Num,选中的菜单项编号,len字符串长度,OutInFlag为0时是读,为1时是写
void ReadWriteSysParam( u_char Num,u_char xdata *OutStr,u_char xdata *InStr ,u_char xdata *len, bit OutInFlag )
{	
	u_char  xdata j = 0;	
	u_char	xdata  i;
	uint	xdata temp = 0;		
	uint	xdata tempMoney = 0;	//111129 by cq 缩减代码	

	i = 0;
	switch( Num )
	{

		case 0:  //If open compressor module 
			if( OutInFlag == 0 )
			{				
				*len = sprintf( OutStr, "%bu", SystemParameter.CompModule );			
			}
			else
			{
				j = *len;
				if( j == 0 )
					break;
				if( j > 1 )
					j = 1;
                //SystemParameter.ACDCModule = 0;
				for( i = 0; i < j; i ++ )
				{
					//111024 by cq 删除
					//if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 1 ) )
					{
						SystemParameter.CompModule = *( InStr + i );
						break;
					}
				}
				//111024 by cq 删除
				//if( SystemParameter.ACDCModule != 1 )
					//SystemParameter.ACDCModule = 0;	
				SystemSave = 1;
			}
		break;
		case 1:   //Trade time
			if( OutInFlag == 0 )
			{	
			    *len = sprintf( OutStr, "%u", SystemParameter.tradeTime );
			}
			else
			{
				j = *len;
				if( j==0 ) break;
				if( j>5 )  j = 5;
				SystemParameter.tradeTime = 0;
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.tradeTime = SystemParameter.tradeTime*10 + *( InStr + i );					
				}
				if(SystemParameter.tradeTime >= 255)
				{
					SystemParameter.tradeTime = 255;					
				}				
				SystemSave = 1;
			}			
		break;
		case 2:  //GOC device on
			if( OutInFlag == 0 )
			{				
				sprintf( OutStr, "%bu", SystemParameter.GOCOn );			
				*len = 1;
			}
			else
			{
				j = *len;
				if( j == 0 )
					break;
				if( j > 1 )
					j = 1;
				for( i = 0; i < j; i ++ )
				{
					//111024 by cq 删除
					//if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 1 ) )
					{
						SystemParameter.GOCOn = *( InStr + i );
						break;
					}
				}
				
				//if( SystemParameter.GOCOn != 1 )
					//SystemParameter.GOCOn = 0;	
				SystemSave = 1;
			}
		break;
		case 3://欠客户的钱
			if( OutInFlag == 0 )
			{				
				//如果找零器有多找零情况，显示负数加找零金额;
                //如果有欠款，显示欠款金额;by gzz 20110810	
                if(SystemParameter.MuchOutFlag==1)
                {
                 	switch( SystemParameter.curUnit )
					{
						case 1:
							*len = sprintf( OutStr, "-%u", SystemParameter.NotOutMax );
						break;				
						case 10:
							*len = sprintf( OutStr, "-%u.%u", SystemParameter.NotOutMax/SystemParameter.curUnit,SystemParameter.NotOutMax%SystemParameter.curUnit);
						break;
						case 100:
							*len = sprintf( OutStr, "-%u.%02u", SystemParameter.NotOutMax/SystemParameter.curUnit,SystemParameter.NotOutMax%SystemParameter.curUnit);
						break;					
					}
                }	
                else if(SystemParameter.MuchOutFlag==0)
                {
					switch( SystemParameter.curUnit )
					{
						case 1:
							*len = sprintf( OutStr, "%u", SystemParameter.NotOutMax );
						break;				
						case 10:
							*len = sprintf( OutStr, "%u.%u", SystemParameter.NotOutMax/SystemParameter.curUnit,SystemParameter.NotOutMax%SystemParameter.curUnit);
						break;
						case 100:
							*len = sprintf( OutStr, "%u.%02u", SystemParameter.NotOutMax/SystemParameter.curUnit,SystemParameter.NotOutMax%SystemParameter.curUnit);
						break;					
					}
                }
			}
			else
			{
				j = *len;
				if( j == 0 )
					break;
				if( j > 1 )
					j = 1;			
				if( *( InStr ) == 0 ) 					
					SystemParameter.NotOutMax = 0;				
				SystemSave = 1;
			}			
		break;
		case 4://允许多次购买	
			if( OutInFlag == 0 )
			{				
				sprintf( OutStr, "%bu", SystemParameter.IsMuliVerd );			
				*len = 1;
			}
			else
			{
				j = *len;
				if( j == 0 )
					break;
				if( j > 1 )
					j = 1;
				for( i = 0; i < j; i ++ )
				{
					//111024 by cq 删除
					//if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 1 ) )
					{
						SystemParameter.IsMuliVerd = *( InStr + i );
						break;
					}
				}
				//111024 by cq 删除
				//if( SystemParameter.IsMuliVerd != 1 )
					//SystemParameter.IsMuliVerd = 0;	
				SystemSave = 1;
			}
		break;
		case 5: //允许退币	
			if( OutInFlag == 0 )
			{				
				sprintf( OutStr, "%bu", SystemParameter.RefundPermission );			
				*len = 1;
			}
			else
			{
				j = *len;
				if( j == 0 )
					break;
				if( j > 1 )
					j = 1;
				for( i = 0; i < j; i ++ )
				{
					//111024 by cq 删除
					//if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 1 ) )
					{
						SystemParameter.RefundPermission = *( InStr + i );
						break;
					}
				}		
				//111024 by cq 删除
				//if( SystemParameter.RefundPermission != 1 )
					//SystemParameter.RefundPermission = 0;	
				SystemSave = 1;
			}
		break;
		case 6://再交易低限额
			if( OutInFlag == 0 )
			{				
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.Min );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.Min/SystemParameter.curUnit,SystemParameter.Min%SystemParameter.curUnit);
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.Min/SystemParameter.curUnit,SystemParameter.Min%SystemParameter.curUnit);
					break;					
				}
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				SystemParameter.Min = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.Min = SystemParameter.Min*10 + *( InStr + i );						
				}			
				SystemSave = 1;
			}	
		break;			
		case 7://是否允许开启纸币器	
			if( OutInFlag == 0 )
			{				
				sprintf( OutStr, "%bu", SystemParameter.BillNo );			
				*len = 1;
			}			
			else
			{
				j = *len;
				if( j == 0 )
					break;
				if( j > 1 )
					j = 1;
				for( i = 0; i < j; i ++ )
				{
					//111024 by cq 删除
					//if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 1 ) )
					{
						SystemParameter.BillNo = *( InStr + i );
						break;
					}
				}

				if( SystemParameter.BillNo == 0 )
					DeviceStatus.BillAccepter = 128;	
				else
					DeviceStatus.BillAccepter &= 0x7f;	
				
				//111024 by cq 删除
				//if(SystemParameter.BillNo != 1)					
					//SystemParameter.BillNo = 0;
				SystemSave = 1;
			}			
		break;
		case 8:  //Coin acceptor on
			if( OutInFlag == 0 )
			{				
				sprintf( OutStr, "%bu", SystemParameter.coinOn );			
				*len = 1;
			}
			/*
			else
			{  
				j = *len;
				if( j == 0 )
					break;
				if( j > 1 )
					j = 1;
				for( i = 0; i < j; i ++ )
				{
					//111024 by cq 删除
					//if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 1 ) )
					{
						SystemParameter.coinOn = *( InStr + i );
						break;
					}
				}

				if( SystemParameter.coinOn == 0 )
					DeviceStatus.CoinAccepter = 128;  
				else
					DeviceStatus.CoinAccepter &= 0x7f;
					
				//if( SystemParameter.coinOn != 1 )
					//SystemParameter.coinOn = 0;	
				SystemSave = 1;                
			}
			*/
		break;
		case 9:  //If open ACDC module 
			if( OutInFlag == 0 )
			{				
				*len = sprintf( OutStr, "%bu", SystemParameter.ACDCModule );			
			}
			/*
			else
			{
				j = *len;
				if( j == 0 )
					break;
				if( j > 1 )
					j = 1;
                //SystemParameter.ACDCModule = 0;
				for( i = 0; i < j; i ++ )
				{
					//111024 by cq 删除
					//if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 1 ) )
					{
						SystemParameter.ACDCModule = *( InStr + i );
						break;
					}
				}
				//111024 by cq 删除
				//if( SystemParameter.ACDCModule != 1 )
					//SystemParameter.ACDCModule = 0;	
				SystemSave = 1;
			}
			*/
		break;		
		/*
		//111024 by cq 删除  			
		case 10://#1找零面值			
			if( OutInFlag == 0 )
			{				
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.HopperCoinPrice1 );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.HopperCoinPrice1/SystemParameter.curUnit,SystemParameter.HopperCoinPrice1%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.HopperCoinPrice1/SystemParameter.curUnit,SystemParameter.HopperCoinPrice1%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{	
                break;
                /*			
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				SystemParameter.HopperCoinPrice1 = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.HopperCoinPrice1 = SystemParameter.HopperCoinPrice1*10 + *( InStr + i );						
				}			
				SystemSave = 1;
                * /
			}			
		break;
        		
		case 11://#2找零面值			
			if( OutInFlag == 0 )
			{					
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.HopperCoinPrice2 );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.HopperCoinPrice2/SystemParameter.curUnit,SystemParameter.HopperCoinPrice2%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.HopperCoinPrice2/SystemParameter.curUnit,SystemParameter.HopperCoinPrice2%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{
                break;
                /* 				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				SystemParameter.HopperCoinPrice2 = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.HopperCoinPrice2 = SystemParameter.HopperCoinPrice2*10 + *( InStr + i );						
				}			
				SystemSave = 1;
                * /
			}			
		break;
        */
        
		case 10://# 1 note
		case 11://# 2 note
		case 12://# 3 note
		case 13: //# 4 note
		case 14: //# 5 note
		case 15: //# 6 note
		case 16: //# 1 coin
		case 17: //# 2 coin
		case 18: //# 3 coin
		case 19: //# 4 coin
			if( OutInFlag == 0 )
			{
				
				switch(Num)
					{	
					case 10: //# 1 note
						tempMoney = SystemParameter.billValue[0];	
						break;
					case 11: //# 2 note
						tempMoney = SystemParameter.billValue[1];	
						break;
					case 12: //# 3 note
						tempMoney = SystemParameter.billValue[2];	
						break;
					case 13: //# 4 note
						tempMoney = SystemParameter.billValue[3];	
						break;
					case 14: //# 5 note
						tempMoney = SystemParameter.billValue[4];	
						break;
					case 15: //# 6 note
						tempMoney = SystemParameter.billValue[5];	
						break;
						
					case 16: //# 1 coin
						tempMoney = SystemParameter.coinValue[0];	
						break;
					case 17: //# 2 coin
						tempMoney = SystemParameter.coinValue[1];	
						break;
					case 18: //# 3 coin
						tempMoney = SystemParameter.coinValue[2];	
						break;
					case 19: //# 4 coin
						tempMoney = SystemParameter.coinValue[3];	
						break;													
					default:
						break;
					}

				
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", tempMoney/100 );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", tempMoney/10/SystemParameter.curUnit,tempMoney/10%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", tempMoney/SystemParameter.curUnit,tempMoney%SystemParameter.curUnit );
					break;					
				}		
				
			}
			/*
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;				
				if( j > 5 )
					j = 5;

				tempMoney = 0;
				//SystemParameter.HopperCoinPrice4 = 0;	
				
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )							
						tempMoney = tempMoney*10 + *( InStr + i );
						//SystemParameter.HopperCoinPrice4 = SystemParameter.HopperCoinPrice4*10 + *( InStr + i );						
				}	

				switch(Num)
					{					
					case 10: //# 1 note
						SystemParameter.billValue[0] = tempMoney;	
						break;
					case 11: //# 2 note
						SystemParameter.billValue[1] = tempMoney;	
						break;
					case 12: //# 3 note
						SystemParameter.billValue[2] = tempMoney;	
						break;
					case 13: //# 4 note
						SystemParameter.billValue[3] = tempMoney;	
						break;
					case 14: //# 5 note
						SystemParameter.billValue[4] = tempMoney;	
						break;
					case 15: //# 6 note
						SystemParameter.billValue[5] = tempMoney;	
						break;							
						
					case 16: //# 1 coin
						SystemParameter.coinValue[0] = tempMoney;	
						break;
					case 17: //# 2 coin
						SystemParameter.coinValue[1] = tempMoney;	
						break;
					case 18: //# 3 coin
						SystemParameter.coinValue[2] = tempMoney;	
						break;
					case 19: //# 4 coin
						SystemParameter.coinValue[3] = tempMoney;	
						break;															
					default:
						break;
					}				
				SystemSave = 1;
			}			
			break;	
			*/
			/*
		case 28:  //SVC no change
			
			if( OutInFlag == 0 )
			{				
				sprintf( OutStr, "%bu", SystemParameter.SVC_NoChange );			
				*len = 1;
			}
			else
			{
				j = *len;
				if( j == 0 )
					break;
				if( j > 1 )
					j = 1;
				for( i = 0; i < j; i ++ )
				{
					//111024 by cq 删除
					//if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 1 ) )
					{
						SystemParameter.SVC_NoChange = *( InStr + i );
						break;
					}
				}
				//if( SystemParameter.SVC_NoChange != 1 )
					//SystemParameter.SVC_NoChange = 0;	
				SystemSave = 1;
			}
		break;
		*/
        /*
        case 29:  //Bus card on
        	
			if( OutInFlag == 0 )
			{				
				sprintf( OutStr, "%bu", SystemParameter.busCardOn );			
				*len = 1;
			}
			else
			{
				j = *len;
				if( j == 0 )
					break;
				if( j > 1 )
					j = 1;
				for( i = 0; i < j; i ++ )
				{
					//111024 by cq 删除
					//if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 1 ) )
					{
						SystemParameter.busCardOn = *( InStr + i );
						break;
					}
				}
				//if( SystemParameter.busCardOn != 1 )
					//SystemParameter.busCardOn = 0;	
				SystemSave = 1;
			}
		break;
		*/
			/*
        case 30:  //bus card read time 
        	
			if( OutInFlag == 0 )
			{				
				*len = sprintf( OutStr, "%bu", SystemParameter.BCReadTime );			
			}
			else
			{
				j = *len;
				if( j == 0 )
					break;
				if( j > 2 )
					j = 2;
                SystemParameter.BCReadTime = 0;
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )
					{
						SystemParameter.BCReadTime = SystemParameter.BCReadTime*10 + *( InStr + i );
						break;
					}
				}
				SystemSave = 1;
			}
		break;
		*/

		 /*		
		case 2://#3硬币面值			
			if( OutInFlag == 0 )
			{													
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.HopperCoinPrice3 );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.HopperCoinPrice3/SystemParameter.curUnit,SystemParameter.HopperCoinPrice3%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.HopperCoinPrice3/SystemParameter.curUnit,SystemParameter.HopperCoinPrice3%SystemParameter.curUnit );
					break;					
				}
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				SystemParameter.HopperCoinPrice3 = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.HopperCoinPrice3 = SystemParameter.HopperCoinPrice3*10 + *( InStr + i );						
				}										
				SystemSave = 1;
			}			
		break;	
		*/
		
		
		
//111024 by cq 删除	
/*
		case 6://是否开启手机模块	
			if( OutInFlag == 0 )
			{				
				sprintf( OutStr, "%bu", SystemParameter.MobileON );			
				*len = 1;
			}
			else
			{
				j = *len;
				if( j == 0 )
					break;
				if( j > 1 )
					j = 1;				
				SystemParameter.MobileON = *( InStr + i );				
				SystemSave = 1;
			}
		break;
*/		
		
		//111024 by cq 删除
		/*
		case 9://疑问金额
			if( OutInFlag == 0 )
			{				
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.DoubtCash );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.DoubtCash/SystemParameter.curUnit,SystemParameter.DoubtCash%SystemParameter.curUnit);
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.DoubtCash/SystemParameter.curUnit,SystemParameter.DoubtCash%SystemParameter.curUnit);
					break;					
				}
			}
			else
			{
				j = *len;
				if( j == 0 )
					break;
				if( j > 1 )
					j = 1;			
				if( *( InStr ) == 0 ) 					
					SystemParameter.DoubtCash = 0;				
				SystemSave = 1;
			}			
		break;
		case 10://取本机ID号码				
			if(  OutInFlag == 0 )
			{
				for( i = 0; i < 5; i++ )
					*len += sprintf( OutStr + i, "%bu", SystemParameter.VmID[i] );
			}			
			else//存入
			{		
				j = *len;
				memset( SystemParameter.VmID, 0, sizeof( SystemParameter.VmID ) );				
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.VmID[i] = *( InStr + i );					
				}
				SystemSave = 1;
			}						
		break;
		case 11://GSM模块号码
			if( OutInFlag == 0 )
			{
				for( i = 0; i < 15; i++ )
				{
					if( SystemParameter.GSMNO[i] != 0xff )
					{
						if( SystemParameter.GSMNO[i] >= 0x10 )
							*len += sprintf( OutStr + 2*i, "%2bx", SystemParameter.GSMNO[i] );	
						else
							*len += sprintf( OutStr + 2*i, "%02bx", SystemParameter.GSMNO[i] );	
						if( ( SystemParameter.GSMNO[i] & 0x0f ) == 0x0f )
						{
							(*len)--;
							break;
						}
					}
					else
						break;				
				}
				OutStr[(*len)] = 0x00;
			}
			else
			{
				j = *len;
				memset( SystemParameter.GSMNO, 0, sizeof( SystemParameter.GSMNO ) );				
				for( i = 0; i < j/2; i ++ )									
					SystemParameter.GSMNO[i] = *( InStr + 2*i ) * 16 + *( InStr + 2*i + 1 );				
				if( ( j%2 ) == 1 )//奇数个数字		
					SystemParameter.GSMNO[i] = *( InStr + 2*i )*16 + 0x0f;
				i++;
				SystemParameter.GSMNO[i] = 0xff;
				SystemSave = 1;
			}
		break;
		*/
		
		//----------------------------------------------------------------------------------------------------
		/*	
        case 9: //# 1 note
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.billValue[0] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.billValue[0]/SystemParameter.curUnit,SystemParameter.billValue[0]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.billValue[0]/SystemParameter.curUnit,SystemParameter.billValue[0]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.billValue[0] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.billValue[0] = SystemParameter.billValue[0]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
		case 10: //# 2 note
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.billValue[1] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.billValue[1]/SystemParameter.curUnit,SystemParameter.billValue[1]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.billValue[1]/SystemParameter.curUnit,SystemParameter.billValue[1]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.billValue[1] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.billValue[1] = SystemParameter.billValue[1]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
        case 11: //# 3 note
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.billValue[2] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.billValue[2]/SystemParameter.curUnit,SystemParameter.billValue[2]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.billValue[2]/SystemParameter.curUnit,SystemParameter.billValue[2]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.billValue[2] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.billValue[2] = SystemParameter.billValue[2]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
		case 12: //# 4 note
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.billValue[3] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.billValue[3]/SystemParameter.curUnit,SystemParameter.billValue[3]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.billValue[3]/SystemParameter.curUnit,SystemParameter.billValue[3]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.billValue[3] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.billValue[3] = SystemParameter.billValue[3]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
        case 13: //# 5 note
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.billValue[4] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.billValue[4]/SystemParameter.curUnit,SystemParameter.billValue[4]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.billValue[4]/SystemParameter.curUnit,SystemParameter.billValue[4]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.billValue[4] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.billValue[4] = SystemParameter.billValue[4]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
        case 14: //# 6 note
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.billValue[5] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.billValue[5]/SystemParameter.curUnit,SystemParameter.billValue[5]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.billValue[5]/SystemParameter.curUnit,SystemParameter.billValue[5]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.billValue[5] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.billValue[5] = SystemParameter.billValue[5]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
        case 15: //# 7 note
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.billValue[6] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.billValue[6]/SystemParameter.curUnit,SystemParameter.billValue[6]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.billValue[6]/SystemParameter.curUnit,SystemParameter.billValue[6]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.billValue[6] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.billValue[6] = SystemParameter.billValue[6]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
        case 16: //# 8 note
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.billValue[7] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.billValue[7]/SystemParameter.curUnit,SystemParameter.billValue[7]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.billValue[7]/SystemParameter.curUnit,SystemParameter.billValue[7]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.billValue[7] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.billValue[7] = SystemParameter.billValue[7]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
        // Coin value begin with 22
        case 17: //# 1 coin
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.coinValue[0] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.coinValue[0]/SystemParameter.curUnit,SystemParameter.coinValue[0]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.coinValue[0]/SystemParameter.curUnit,SystemParameter.coinValue[0]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.coinValue[0] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.coinValue[0] = SystemParameter.coinValue[0]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
		case 18: //# 2 coin
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.coinValue[1] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.coinValue[1]/SystemParameter.curUnit,SystemParameter.coinValue[1]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.coinValue[1]/SystemParameter.curUnit,SystemParameter.coinValue[1]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.coinValue[1] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.coinValue[1] = SystemParameter.coinValue[1]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
        case 19: //# 3 coin
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.coinValue[2] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.coinValue[2]/SystemParameter.curUnit,SystemParameter.coinValue[2]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.coinValue[2]/SystemParameter.curUnit,SystemParameter.coinValue[2]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.coinValue[2] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.coinValue[2] = SystemParameter.coinValue[2]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
		case 20: //# 4 coin
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.coinValue[3] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.coinValue[3]/SystemParameter.curUnit,SystemParameter.coinValue[3]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.coinValue[3]/SystemParameter.curUnit,SystemParameter.coinValue[3]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.coinValue[3] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.coinValue[3] = SystemParameter.coinValue[3]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
        case 21: //# 5 coin
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.coinValue[4] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.coinValue[4]/SystemParameter.curUnit,SystemParameter.coinValue[4]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.coinValue[4]/SystemParameter.curUnit,SystemParameter.coinValue[4]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.coinValue[4] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.coinValue[4] = SystemParameter.coinValue[4]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
        case 22: //# 6 coin
        if( OutInFlag == 0 )
			{			
				switch( SystemParameter.curUnit )
				{
					case 1:
						*len = sprintf( OutStr, "%u", SystemParameter.coinValue[5] );
					break;				
					case 10:
						*len = sprintf( OutStr, "%u.%u", SystemParameter.coinValue[5]/SystemParameter.curUnit,SystemParameter.coinValue[5]%SystemParameter.curUnit );
					break;
					case 100:
						*len = sprintf( OutStr, "%u.%02u", SystemParameter.coinValue[5]/SystemParameter.curUnit,SystemParameter.coinValue[5]%SystemParameter.curUnit );
					break;					
				}				
			}
			else
			{				
				//Trace( "\n set IP val \n");
				j = *len;											
				if( j == 0 )
					break;
				if( j > 5 )
					j = 5;
				SystemParameter.coinValue[5] = 0;	
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )					
						SystemParameter.coinValue[5] = SystemParameter.coinValue[5]*10 + *( InStr + i );						
				}	
				SystemSave = 1;
			}			
		break;
		*/
		
		/*
		case 29:  //If service, when GOC error
			if( OutInFlag == 0 )
			{				
				sprintf( OutStr, "%bu", SystemParameter.SVC_GOCErr );			
				*len = 1;
			}
			else
			{
				j = *len;
				if( j == 0 )
					break;
				if( j > 1 )
					j = 1;
				for( i = 0; i < j; i ++ )
				{
					if( ( *( InStr + i ) >= 0 ) && ( *( InStr + i ) <= 9 ) )
					{
						SystemParameter.SVC_GOCErr = *( InStr + i );
						break;
					}
				}
				if( SystemParameter.SVC_GOCErr != 1 )
					SystemParameter.SVC_GOCErr = 0;	
				SystemSave = 1;
			}
		break;
		*/		
        
               
		
		//====================================================================================================
		default:
			break;
	}
}

/***********************************/
/*y为行号,Num为菜单项序号,flag为是否显示值*/
u_char  DisplayMenuAndVal( u_char y, u_char Num, bit flag )
{	
	u_char   xdata  len ;

	len = 0;	
	memset( y_displaystr,0,sizeof( y_displaystr ) );				
	if( flag == 0 )//编辑此项菜单
	{	
		DisplayStr( 0, 0, 1, SysMenustr[Num].str, SysMenustr[Num].len );//显示菜单			
		return 0;
	}
	else//显示此项菜单
	{
		memcpy( y_displaystr, SysMenustr[Num].str, SysMenustr[Num].len );//菜单串转化为unicode串				
		ReadWriteSysParam( Num, y_displaystr + SysMenustr[Num].len, 0, &len, 0 );//得到菜单对应的值				
		DisplayStr( 0, y, 1,  y_displaystr, SysMenustr[Num].len + len );//显示菜单
		y_displaystr[SysMenustr[Num].len + len ] = '\0';
		if( SysMenustr[Num].len + len > 20 )
			return ( SysMenustr[Num].len + len - 20) ;		
		else
			return 0;
	}
}	

/***********************************/
/*编辑选中项,CardStyle,为选中的卡种类编号,Num为选中的菜单编号,flag标志位,确定是系统参数(0,此时CardStyle无效),
还是卡参数(1)*/
bit  EditParam( u_char Num, u_char y )
{
	u_char  xdata Inputstr[30];
	u_char  xdata  key;
	u_char  xdata  Inputlen;	
	bit	data  ret;
	
	key = 0;
	Inputlen = 0;
	ret = 0;
	//Trace( "edit one SysParanMenu begin\n" );
	
	Inputlen = SysInputLen[Num];
	if( ( Num == 0 ) ||( Num == 1 ) ||( Num == 2 ) || ( Num == 7 )|| ( Num == 8 )|| ( Num == 9 ) )
		key = GetLineMoney( SysMenustr[Num].str, SysMenustr[Num].len, y, Inputstr, &Inputlen );		
	else
		key = GetLine( SysMenustr[Num].str, SysMenustr[Num].len, y, Inputstr, &Inputlen );	
	ClearKey();	
	//Trace( "input END RET (0x) = %bx\n", key );		
	if( key == 1 )//Enter	
	{
		//Trace( "display select \n" );
		DisplayCursorState( 0, 0, 0, 0, 1 );			
		ReadWriteSysParam( Num, 0, Inputstr ,&Inputlen, 1 );
		ClearKey();		
	}		
	//Trace( "edit one SysParanMenu end\n" );
	return ret;
}

/************************************/
/*光标在菜单中移动或选择,SelectedNum当前光标所在的菜单编号,
	CursorLine，当前光标所覆盖的行,值为1或2,MoveRange,菜单翻动的范围
	SysNum,指显示哪类菜单,1为系统菜单,2为品牌菜单,3为种类菜单,
	ChangeLineflag指定移动光标是否移动菜单项*/
void  SP_MenuSelected( u_char xdata *SelectedNum, u_char xdata *CursorLine, u_char MoveRange, u_char ChangeLineflag )
{
	u_char xdata  MenuNum = *SelectedNum;
	//此项为1表示光标在第一行,为二表示光标在第二行
	u_char xdata  Cursor = *CursorLine;
	bit	   data   flag = 1;	
	u_char xdata  ReDisplen = 0;
	u_char xdata  key = 0xff;
	u_char  xdata  len = 0;	

	while( flag )
	{
		key = GetKey();
		switch( key )		
		{
			case KEY_CANCEL:
				flag = 0;
				*SelectedNum = 100;				
				break;
			case KEY_UP://上移  
	//			Trace("\n Cursor1 = %bu ", Cursor );				
	//			Trace("\n MenuNum1 = %bu ", MenuNum );
				if( Cursor == 2 )//选中标记上移
				{			
					DisplayCursorState( 0, 0, 1, 2, 1 );//第一行整光标					
					Cursor = 1;
					if( ( ChangeLineflag & 0x01 ) == 0x01 )					
						ChangeLineflag |= 0x02;
					else
					{
						if( MenuNum == 0 )
							MenuNum = MoveRange;					
						else									    
							MenuNum--;					
					}						
	//				Trace("\n Cursor2 = %bu ", Cursor );				
	//				Trace("\n MenuNum2 = %bu ", MenuNum );
				}
				else//菜单上翻
				{	
	//				Trace("\n Cursor3 = %bu ", Cursor );				
	//				Trace("\n MenuNum3 = %bu ", MenuNum );					
				//	DisplayMenuAndVal( 1, MenuNum, 1 );					
					if( MenuNum == 0 )
						MenuNum = MoveRange;					
					else									    
						MenuNum--;					
					ReDisplen = DisplayMenuAndVal( 0, MenuNum , 1 );
					if( ReDisplen != 0 )
					{
						DisplayStr( 0, 1, 1,  y_displaystr + 20, ReDisplen );//显示菜单						
						ReDisplen = 0;
						DisplayCursorState( 0, Cursor - 1, 1, 2, 1 );//第一行整光标
						if( Cursor == 1 )
						{
							//光标目前在第一行，则下移光标不改变菜单选项
							ChangeLineflag |= 0x02;
						}
						else
						{
							//光标目前在第二行，则上移光标不改变菜单选项
							ChangeLineflag |= 0x01;
						}
					}
					else
					{
						if(	MenuNum == MoveRange )
							ReDisplen = DisplayMenuAndVal( 1, 0 , 1 );
						else
							ReDisplen = DisplayMenuAndVal( 1, MenuNum + 1 , 1 );
						DisplayCursorState( 0, Cursor - 1, 1, 2, 1 );//第一行整光标
						ChangeLineflag = 0;
					 //  Cursor = 1;
					}	
	//				Trace("\n Cursor4 = %bu ", Cursor );				
	//				Trace("\n MenuNum4 = %bu ", MenuNum );										
				}
				break;
			case KEY_DOWN://下移	
	//			Trace("\n Cursor5 = %bu ", Cursor );				
	//			Trace("\n MenuNum5 = %bu ", MenuNum );					
				if( Cursor == 1 )//选中标记下移
				{
					DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标
					if( ( ChangeLineflag & 0x02 ) == 0x02 )					
						ChangeLineflag |= 0x01;
					else
					{
						if( MenuNum == MoveRange )
							MenuNum = 0;					
						else									    						
							MenuNum++;				     
					}
					Cursor = 2;	
	//				Trace("\n Cursor7 = %bu ", Cursor );				
	//				Trace("\n MenuNum7 = %bu ", MenuNum );					
				}
				else//菜单下翻
				{	
					if( ChangeLineflag != 0x00 )
					{
						ChangeLineflag = 0;
						if( MenuNum == MoveRange )
							MenuNum = 0;					
						else			
							MenuNum++;	
			//		   DisplayCursorState( 0, 0, 1, 2, 1 );//第二行整光标
			//		   Cursor = 1;
					}
	//				Trace("\n Cursor8 = %bu ", Cursor );				
	//				Trace("\n MenuNum8 = %bu ", MenuNum );	
					ReDisplen = DisplayMenuAndVal( 0, MenuNum, 1 );															
					if( ReDisplen != 0 )
					{
						DisplayStr( 0, 1, 1,  y_displaystr + 20, ReDisplen );//显示菜单						
						ReDisplen = 0;
						if( Cursor == 1 )
						{
							//光标目前在第一行，则下移光标不改变菜单选项
							ChangeLineflag |= 0x02;
						}
						else
						{
							//光标目前在第二行，则上移光标不改变菜单选项
							ChangeLineflag |= 0x01;
						}
					}
					else
					{						
						if( MenuNum == MoveRange )
							MenuNum = 0;					
						else									    
							MenuNum++;	
						ReDisplen = DisplayMenuAndVal( 1, MenuNum , 1 );																					
					//	Cursor = 2;
					}
	//				Trace("\n Cursor9 = %bu ", Cursor );				
	//				Trace("\n MenuNum9 = %bu ", MenuNum );
				}
				break;
			case KEY_SUBMIT://Enter,选中了某项菜单
				{			
					*SelectedNum = MenuNum;
				//	if( ChangeLineflag != 0 )
				//	{
				//		*CursorLine = 1;
				//		DisplayStr( 0, 1, 1,  " ", 20 );//显示菜单
				//	}
				//	else
					*CursorLine = Cursor;					
					flag = 0;	
				}
				break;			
		}
		CoreProcessCycle();//让出时间片
		DelayMs( 5 );
		if(sysVPMission.msActTimer == 0)
		{
			sysVPMission.msActTimer = 100;
			VPMission_Act_RPT(VP_ACT_ADMIN,VP_ACT_ENTERADMIN);
		}
	}
	ClearKey();		
}

/*主控函数*/
bit SysParamManage()		
{
	u_char  xdata fristNum ;//一级菜单编号
	u_char  xdata LineNum ;//选中屏幕的行号	
	u_char xdata  ReDisplen = 0;
	//bit0为上移光不改变菜单项,bit1为下移光标不改变菜单项
	u_char xdata  ChangeLineflag = 0;	

	ClearKey();
	
	/*显示两行菜单及其值*/				
	DisplayMenuAndVal( 0, 0, 1 );	
	DisplayMenuAndVal( 1, 1, 1 );		
	DisplayCursorState( 0, 1, 1, 2, 1 );//设置光标在第二行上且为整行光标状态
	fristNum = 1;
	LineNum = 2;	
	while( 1 )
	{		
		/*判断按下何键*/			
		SP_MenuSelected( &fristNum, &LineNum, MENU_COUNT( SysMenustr ) - 1, ChangeLineflag );			
		//选中了某项菜单				
		if( fristNum == 100 )//选中退出									
			break;		
		else//没有选中退出
		{				
			ClearKey();									
			EditParam( fristNum, LineNum - 1 );	
			//重新显示此项参数的值			
			if( LineNum == 2 )
			{			
				if( fristNum == 0 )
					ReDisplen = DisplayMenuAndVal( 0, MENU_COUNT( SysMenustr ) - 1, 1 );
				else						
					ReDisplen = DisplayMenuAndVal( 0, fristNum-1, 1 );						
				if( ReDisplen > 0 )
				{
					DisplayStr( 0, 1, 1,  y_displaystr + 20, ReDisplen );//显示菜单					
					ReDisplen = 0;
					ChangeLineflag |=0x01;
				}
				else
				{
					ReDisplen = DisplayMenuAndVal( 1, fristNum, 1 );
					if( ReDisplen > 0 )
						ChangeLineflag = 0;
				}
			}
			else
			{	
				ReDisplen = DisplayMenuAndVal( 0, fristNum, 1 );				
				if( ReDisplen > 0 )
				{
					DisplayStr( 0, 1, 1,  y_displaystr + 20, ReDisplen );//显示菜单					
					ReDisplen = 0;
					ChangeLineflag |=0x02;
				}
				else
				{
					if( fristNum == ( MENU_COUNT( SysMenustr ) - 1 ) )
						DisplayMenuAndVal( 1, 0, 1 );	
					else						
						DisplayMenuAndVal( 1, fristNum + 1, 1 );	
				}
			//	DisplayMenuAndVal( 0, fristNum, 1 );
			}			
			DisplayCursorState( 0, LineNum - 1, 1, 2, 1 );//设置光标为整行状态			
		}		
	}		
	return 0;
}

