#include "device.h"
#include "CommonFunction.h"
#include "scheduler.h"
#include "string.h"	
#include "global.h"
#include "stdlib.h"
#include "stdio.h"
#include "key.h"
#include "common.h"
#include "mainflow.h"
#include "timer.h" 
#include "debug.h"
#include "VMC_PC.h"


#ifdef _CHINA_	
	const struct StrAndLen code InfoStr[] =
	{		
		{ "  你确定吗?", 11 },      //全部计数值清零
		{ "确定ENTER 取消CANCEL", 20 },		
		{ "  没有交易记录", 14 },	
	};
#else
	const struct StrAndLen code InfoStr[] =
	{		
		{ "   Are you sure ?", 17 }, //全部计数值清零
		{ "   ENTER   CANCEL", 17 },		
		{ " No note Of trade", 17 },	
	};
#endif

#ifdef _CHINA_
	const struct StrAndLen code CurrencyMenuStr[] =
	{	
		{ "纸收:",   6 },	     
		{ "现纸收:", 8 },	         
		{ "1Y收:",   6 },           
		{ "现1Y收:", 8 },         
		{ "5J收:",   6 },         
		{ "现5J收:", 8 },       
		{ "1Y找:",   6 },       
		{ "现1Y找:", 8 },      
		{ "5J找:",   6 },   		
		{ "现5J找:", 8 },		
		//{ "累计销售商品:", 15 },//120419 by cq TotalSell	
		//{ "累计销售金额:", 15 },//120419 by cq TotalSell	
	};
#else
	const struct StrAndLen code CurrencyMenuStr[] =
	{	
		{ "Bill:", 5 },			//总收入
		{ "Now Bill:", 9 },			//纸币总收入
		{ "Coin1.0 In:", 11 },   		//硬币总收入
		{ "Now Coin1.0:", 12 },   		//交易总金额
		{ "Coin0.5 In:", 11 }, 		//找零总金额
		{ "Now Coin0.5:", 12 },	//1#出币总金额
		{ "Coin1.0 Out:", 12 },   		//硬币总收入
		{ "Now Coin1.0:", 12 },   		//交易总金额
		{ "Coin0.5 Out:", 12 }, 		//找零总金额
		{ "Now Coin0.5:", 12 },	//1#出币总金额	
		//{ "TotalSellNum:", 13 },//120419 by cq TotalSell	
		//{ "TotalSellMoeny: ", 15 },//120419 by cq TotalSell	
				
	};
#endif



//120521 by cq SellAccumulate
#ifdef _CHINA_
	const struct StrAndLen code CurrencyMenuStr_1[] =
	{	
		{ "全部量:",   7 },	     
		{ "全部额:", 7 },	         
		{ "现金量:",   7 },           
		{ "现金额:", 7 },         
		{ "游戏量:",   7 },         
		{ "游戏额:", 7 },       
		{ "刷卡量:",   7 },       
		{ "刷卡额:", 7 },      
		{ "在线量:",   7 },   		
		{ "在线额:", 7 }, 
		{ "硬币投入:", 9 },      
		{ "纸币投入:",   9 },   		
		{ "硬币出币:", 9 },
		{ "区间硬币投入:", 13 },      
		{ "区间纸币投入:",   13 },   		
		{ "区间硬币出币:", 13 }
	};
#else
	const struct StrAndLen code CurrencyMenuStr_1[] =
	{	
		{ "SuccessNum:", 11 },			//总收入
		{ "SuccessMoney:", 13 },			//纸币总收入
		{ "CashNum:", 8 },   		//硬币总收入
		{ "CashMoney:", 10 },   		//交易总金额
		{ "GameNum:", 8 }, 		//找零总金额
		{ "GameMoney:", 10 },	//1#出币总金额
		{ "CardNum:", 8 },   		//硬币总收入
		{ "CardMoney:", 10 },   		//交易总金额
		{ "OnlineNum:", 10 }, 		//找零总金额
		{ "OnlineMoney:", 11 }	//1#出币总金额					
	};
#endif


//void GoodsWayTraceList( u_char xdata  iIndex );

//120419 by cq TotalSell
void InitAllCounter_1()
{
	//区间交易记录
	TradeCounter.CoinSum1yBack = 0;
	TradeCounter.CoinSum5jBack = 0;
	TradeCounter.CashSumBack = 0;
	TradeCounter.Hopper1SumBack = 0;
	TradeCounter.Hopper2SumBack = 0;
	SaveTradeParam();//by gzz 20110610			
	//SaveTradeCounter();
	SchedulerProcess();//让出时间片
	WaitForWork( 100, NULL );	
	return;
}

//清零
void InitAllCounter()
{	
	u_char xdata key;
    //struct time xdata Temptime;

	DisplayStr( 0, 0, 1, InfoStr[0].str, InfoStr[0].len );
	DisplayStr( 0, 1, 1, InfoStr[1].str, InfoStr[1].len);
	SgwSecTimer= 6;		
	while( SgwSecTimer )
	{
		key = GetKey();
		if( key == KEY_CANCEL )
			break;
		if (key == KEY_SUBMIT)
		{
			memset( &TradeCounter, 0, sizeof( struct TRADECOUNTER ) );			    
		    //memset( DoubtTradeLog, 0, sizeof( DoubtTradeLog ) );
		    /*
			memset( TradeLog, 0, sizeof( TradeLog ) );	
			for( key = 0; key < GOODSWAYNUM; key++ )					
				TradeLog[key].WayNo = InputGoodsWayList[key].GoodsWayNo;
			*/
		    /*	
		    memset( &Temptime, 0, sizeof( struct time ) );
			GetCurrentTime( &Temptime );
			TradeCounter.Date[0] = ( Temptime.year & 0xff00 ) >> 8;
			TradeCounter.Date[1] = ( Temptime.year & 0x00ff );
			TradeCounter.Date[2] = Temptime.mon;
			TradeCounter.Date[3] = Temptime.date;			
			SgwSecTimer = 0;*/
			SaveTradeParam();
			SaveTradeCounter();
			SchedulerProcess();//让出时间片
			WaitForWork( 100, NULL );	
			return;
		}
		SchedulerProcess();//让出时间片
		WaitForWork( 100, &Lcd );	
	}
}

////货币记录值
void DispCurrencyList( u_char yPos, u_char iIndex )
{
	u_char xdata len = 0; 
	u_char xdata line[26];	
	
	memset( line, 0, sizeof( line ) );		
	//memcpy( line, CurrencyMenuStr[iIndex].str, CurrencyMenuStr[iIndex].len );
	//len = CurrencyMenuStr[iIndex].len;
	switch( iIndex )
	{
		case 0://上次纸币收币总金额			   
		    //len = sprintf( line, "%05u %s", TradeCounter.CashSumID,CurrencyMenuStr[iIndex].str );
			//len += sprintf( line + len, "%lu.%02lu", TradeCounter.CashSumBack/SystemParameter.curUnit, TradeCounter.CashSumBack%SystemParameter.curUnit );
			len = sprintf( line, "%05u %s%lu.%02lu", 0,CurrencyMenuStr[iIndex].str, TradeCounter.CashSumBack/SystemParameter.curUnit, TradeCounter.CashSumBack%SystemParameter.curUnit );
			/*
			switch( SystemParameter.curUnit )		
			{
				case 1:
					len += sprintf( line + len, "%lu", TradeCounter.CashSumBack );
				break;
				case 10:
					len += sprintf( line + len, "%lu.%lu", TradeCounter.CashSumBack/SystemParameter.curUnit, TradeCounter.CashSumBack%SystemParameter.curUnit );
				break;
				case 100:
					len += sprintf( line + len, "%lu.%02lu", TradeCounter.CashSumBack/SystemParameter.curUnit, TradeCounter.CashSumBack%SystemParameter.curUnit );
				break;
			}	
			*/
		break;
		case 1://本次纸币收币总金额	
		    memcpy( line, CurrencyMenuStr[iIndex].str, CurrencyMenuStr[iIndex].len );
	        len = CurrencyMenuStr[iIndex].len;
			len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%02lu", TradeCounter.CashSum/SystemParameter.curUnit, TradeCounter.CashSum%SystemParameter.curUnit );
			/*
			switch( SystemParameter.curUnit )		
			{
				case 1:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu", TradeCounter.CashSum );
				break;
				case 10:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%lu", TradeCounter.CashSum/SystemParameter.curUnit, TradeCounter.CashSum%SystemParameter.curUnit );
				break;
				case 100:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%02lu", TradeCounter.CashSum/SystemParameter.curUnit, TradeCounter.CashSum%SystemParameter.curUnit );
				break;
			}
			*/
		break;
        case 2://收入1元硬总金额 
			//len = sprintf( line, "%05u %s", TradeCounter.CoinSum1yID,CurrencyMenuStr[iIndex].str );
			//len += sprintf( line + len, "%lu.%02lu", TradeCounter.CoinSum1yBack/SystemParameter.curUnit, TradeCounter.CoinSum1yBack%SystemParameter.curUnit );
			len = sprintf( line, "%05u %s%lu.%02lu", 0,CurrencyMenuStr[iIndex].str, TradeCounter.CoinSum1yBack/SystemParameter.curUnit, TradeCounter.CoinSum1yBack%SystemParameter.curUnit );
			/*
			switch( SystemParameter.curUnit )		
			{
				case 1:
					len += sprintf( line + len, "%lu", TradeCounter.CoinSum1yBack );
				break;
				case 10:
					len += sprintf( line + len, "%lu.%lu", TradeCounter.CoinSum1yBack/SystemParameter.curUnit, TradeCounter.CoinSum1yBack%SystemParameter.curUnit );
				break;
				case 100:
					len += sprintf( line + len, "%lu.%02lu", TradeCounter.CoinSum1yBack/SystemParameter.curUnit, TradeCounter.CoinSum1yBack%SystemParameter.curUnit );
				break;
			}
			*/
		break;
		case 3:		
			memcpy( line, CurrencyMenuStr[iIndex].str, CurrencyMenuStr[iIndex].len );
	        len = CurrencyMenuStr[iIndex].len;
			len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%02lu", TradeCounter.CoinSum1y/SystemParameter.curUnit, TradeCounter.CoinSum1y%SystemParameter.curUnit );
			/*
			switch( SystemParameter.curUnit )		
			{
				case 1:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu", TradeCounter.CoinSum1y );
				break;
				case 10:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%lu", TradeCounter.CoinSum1y/SystemParameter.curUnit, TradeCounter.CoinSum1y%SystemParameter.curUnit );
				break;
				case 100:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%02lu", TradeCounter.CoinSum1y/SystemParameter.curUnit, TradeCounter.CoinSum1y%SystemParameter.curUnit );
				break;
			}	
			*/
		break;
		case 4://收入5角硬总金额	
			//len = sprintf( line, "%05u %s", TradeCounter.CoinSum5jID,CurrencyMenuStr[iIndex].str );
			//len += sprintf( line + len, "%lu.%02lu", TradeCounter.CoinSum5jBack/SystemParameter.curUnit, TradeCounter.CoinSum5jBack%SystemParameter.curUnit );
			len = sprintf( line, "%05u %s%lu.%02lu", 0,CurrencyMenuStr[iIndex].str, TradeCounter.CoinSum5jBack/SystemParameter.curUnit, TradeCounter.CoinSum5jBack%SystemParameter.curUnit );
			/*
			switch( SystemParameter.curUnit )		
			{
				case 1:
					len += sprintf( line + len, "%lu", TradeCounter.CoinSum5jBack );
				break;
				case 10:
					len += sprintf( line + len, "%lu.%lu", TradeCounter.CoinSum5jBack/SystemParameter.curUnit, TradeCounter.CoinSum5jBack%SystemParameter.curUnit );
				break;
				case 100:
					len += sprintf( line + len, "%lu.%02lu", TradeCounter.CoinSum5jBack/SystemParameter.curUnit, TradeCounter.CoinSum5jBack%SystemParameter.curUnit );
				break;
			}
			*/
		break;
		case 5:		
			memcpy( line, CurrencyMenuStr[iIndex].str, CurrencyMenuStr[iIndex].len );
	        len = CurrencyMenuStr[iIndex].len;
			len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%02lu", TradeCounter.CoinSum5j/SystemParameter.curUnit, TradeCounter.CoinSum5j%SystemParameter.curUnit );
			/*
			switch( SystemParameter.curUnit )		
			{
				case 1:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu", TradeCounter.CoinSum5j );
				break;
				case 10:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%lu", TradeCounter.CoinSum5j/SystemParameter.curUnit, TradeCounter.CoinSum5j%SystemParameter.curUnit );
				break;
				case 100:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%02lu", TradeCounter.CoinSum5j/SystemParameter.curUnit, TradeCounter.CoinSum5j%SystemParameter.curUnit );
				break;
			}	
			*/
		break;	
        case 6://#2HOPPER出币总金额	
			//len = sprintf( line, "%05u %s", TradeCounter.Hopper2SumID,CurrencyMenuStr[iIndex].str );
			//len += sprintf( line + len, "%lu.%02lu", TradeCounter.Hopper2SumBack/SystemParameter.curUnit, TradeCounter.Hopper2SumBack%SystemParameter.curUnit );
			len = sprintf( line, "%05u %s%lu.%02lu", 0,CurrencyMenuStr[iIndex].str, TradeCounter.Hopper2SumBack/SystemParameter.curUnit, TradeCounter.Hopper2SumBack%SystemParameter.curUnit );
			/*
			switch( SystemParameter.curUnit )		
			{
				case 1:
					len += sprintf( line + len, "%lu", TradeCounter.Hopper2SumBack );
				break;
				case 10:
					len += sprintf( line + len, "%lu.%lu", TradeCounter.Hopper2SumBack/SystemParameter.curUnit, TradeCounter.Hopper2SumBack%SystemParameter.curUnit );
				break;
				case 100:
					len += sprintf( line + len, "%lu.%02lu", TradeCounter.Hopper2SumBack/SystemParameter.curUnit, TradeCounter.Hopper2SumBack%SystemParameter.curUnit );
				break;
			}
			*/
		break;
		case 7:		
			memcpy( line, CurrencyMenuStr[iIndex].str, CurrencyMenuStr[iIndex].len );
	        len = CurrencyMenuStr[iIndex].len;
			len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%02lu", TradeCounter.Hopper2Sum/SystemParameter.curUnit, TradeCounter.Hopper2Sum%SystemParameter.curUnit );
			/*
			switch( SystemParameter.curUnit )		
			{
				case 1:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu", TradeCounter.Hopper2Sum );
				break;
				case 10:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%lu", TradeCounter.Hopper2Sum/SystemParameter.curUnit, TradeCounter.Hopper2Sum%SystemParameter.curUnit );
				break;
				case 100:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%02lu", TradeCounter.Hopper2Sum/SystemParameter.curUnit, TradeCounter.Hopper2Sum%SystemParameter.curUnit );
				break;
			}	
			*/
		break;	
		case 8://#1HOPPER出币总金额	
			//len = sprintf( line, "%05u %s", TradeCounter.Hopper1SumID,CurrencyMenuStr[iIndex].str );
			//len += sprintf( line + len, "%lu.%02lu", TradeCounter.Hopper1SumBack/SystemParameter.curUnit, TradeCounter.Hopper1SumBack%SystemParameter.curUnit );
			len = sprintf( line, "%05u %s%lu.%02lu", 0,CurrencyMenuStr[iIndex].str, TradeCounter.Hopper1SumBack/SystemParameter.curUnit, TradeCounter.Hopper1SumBack%SystemParameter.curUnit );
			/*
			switch( SystemParameter.curUnit )		
			{
				case 1:
					len += sprintf( line + len, "%lu", TradeCounter.Hopper1SumBack );
				break;
				case 10:
					len += sprintf( line + len, "%lu.%lu", TradeCounter.Hopper1SumBack/SystemParameter.curUnit, TradeCounter.Hopper1SumBack%SystemParameter.curUnit );
				break;
				case 100:
					len += sprintf( line + len, "%lu.%02lu", TradeCounter.Hopper1SumBack/SystemParameter.curUnit, TradeCounter.Hopper1SumBack%SystemParameter.curUnit );
				break;
			}
			*/
		break;
		case 9:		
			memcpy( line, CurrencyMenuStr[iIndex].str, CurrencyMenuStr[iIndex].len );
	        len = CurrencyMenuStr[iIndex].len;
			len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%02lu", TradeCounter.Hopper1Sum/SystemParameter.curUnit, TradeCounter.Hopper1Sum%SystemParameter.curUnit );
			/*
			switch( SystemParameter.curUnit )		
			{
				case 1:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu", TradeCounter.Hopper1Sum );
				break;
				case 10:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%lu", TradeCounter.Hopper1Sum/SystemParameter.curUnit, TradeCounter.Hopper1Sum%SystemParameter.curUnit );
				break;
				case 100:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%02lu", TradeCounter.Hopper1Sum/SystemParameter.curUnit, TradeCounter.Hopper1Sum%SystemParameter.curUnit );
				break;
			}
			*/
		break;	
		/*
		case 10://#3HOPPER出币总金额
			len = sprintf( line, "%05u %s", TradeCounter.Hopper3SumID,CurrencyMenuStr[iIndex].str );
			switch( SystemParameter.curUnit )		
			{
				case 1:
					len += sprintf( line + len, "%lu", TradeCounter.Hopper3SumBack );
				break;
				case 10:
					len += sprintf( line + len, "%lu.%lu", TradeCounter.Hopper3SumBack/SystemParameter.curUnit, TradeCounter.Hopper3SumBack%SystemParameter.curUnit );
				break;
				case 100:
					len += sprintf( line + len, "%lu.%02lu", TradeCounter.Hopper3SumBack/SystemParameter.curUnit, TradeCounter.Hopper3SumBack%SystemParameter.curUnit );
				break;
			}
		break;
		case 11:		
			memcpy( line, CurrencyMenuStr[iIndex].str, CurrencyMenuStr[iIndex].len );
	        len = CurrencyMenuStr[iIndex].len;

			switch( SystemParameter.curUnit )		
			{
				case 1:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu", TradeCounter.Hopper3Sum );
				break;
				case 10:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%lu", TradeCounter.Hopper3Sum/SystemParameter.curUnit, TradeCounter.Hopper3Sum%SystemParameter.curUnit );
				break;
				case 100:
					len += sprintf( line + CurrencyMenuStr[iIndex].len, "%lu.%02lu", TradeCounter.Hopper3Sum/SystemParameter.curUnit, TradeCounter.Hopper3Sum%SystemParameter.curUnit );
				break;
			}	
		break;	
		
		
		//120419 by cq TotalSell
		case 10://累计销售数量
			len = sprintf( line, "%s%lu", CurrencyMenuStr[iIndex].str, TradeCounter.TotalSellNum);
			break;
			
		//120419 by cq TotalSell
		case 11://累计销售金额
			len = sprintf( line, "%s%lu.%02lu", CurrencyMenuStr[iIndex].str, TradeCounter.TotalSellMoeny/SystemParameter.curUnit, TradeCounter.TotalSellMoeny%SystemParameter.curUnit );
			break;
		*/
        default:
        break;

	}
	Trace("\n %s ",line );
	Trace("\n len = %bu",len );
	DisplayStr( 0, yPos, 1, line, len );	

}



////货币记录值
void DispCurrencyList_1( u_char yPos, u_char iIndex )//120521 by cq SellAccumulate
{
	u_char xdata len = 0; 
	u_char xdata line[26];	
	
	memset( line, 0, sizeof( line ) );		
	//memcpy( line, CurrencyMenuStr[iIndex].str, CurrencyMenuStr[iIndex].len );
	//len = CurrencyMenuStr[iIndex].len;
	switch( iIndex )
	{
		case 0://全部量		   
			//len = sprintf( line, "%05u %s%lu.%02lu", TradeCounter.CashSumID,CurrencyMenuStr[iIndex].str, TradeCounter.CashSumBack/SystemParameter.curUnit, TradeCounter.CashSumBack%SystemParameter.curUnit );
		    memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%lu", TradeCounter.vpSuccessNum );
			break;
		case 1://全部额
		    memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%lu", TradeCounter.vpSuccessMoney/SystemParameter.curUnit );
			break;
        case 2://现金量
			//len = sprintf( line, "%05u %s%lu.%02lu", TradeCounter.CoinSum1yID,CurrencyMenuStr[iIndex].str, TradeCounter.CoinSum1yBack/SystemParameter.curUnit, TradeCounter.CoinSum1yBack%SystemParameter.curUnit );
		    memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%lu", TradeCounter.vpCashNum );
			break;
		case 3://现金额		
			memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%lu", TradeCounter.vpCashMoney/SystemParameter.curUnit );
			break;
		case 4://游戏量	
			//len = sprintf( line, "%05u %s%lu.%02lu", TradeCounter.CoinSum5jID,CurrencyMenuStr[iIndex].str, TradeCounter.CoinSum5jBack/SystemParameter.curUnit, TradeCounter.CoinSum5jBack%SystemParameter.curUnit );
		    memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%lu", TradeCounter.vpGameNum );
			break;
		case 5:		
			memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%bu", 0 );
			break;	
        case 6://刷卡量
			//len = sprintf( line, "%05u %s%lu.%02lu", TradeCounter.Hopper2SumID,CurrencyMenuStr[iIndex].str, TradeCounter.Hopper2SumBack/SystemParameter.curUnit, TradeCounter.Hopper2SumBack%SystemParameter.curUnit );
		    memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%lu", TradeCounter.vpCardNum );
			break;
		case 7:		
			memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%bu", 0 );
		    break;	
		case 8://在线量
			//len = sprintf( line, "%05u %s%lu.%02lu", TradeCounter.Hopper1SumID,CurrencyMenuStr[iIndex].str, TradeCounter.Hopper1SumBack/SystemParameter.curUnit, TradeCounter.Hopper1SumBack%SystemParameter.curUnit );
		    memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%lu", TradeCounter.vpOnlineNum );
			break;
		case 9:			
			memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%bu", 0 );
			break;			
		case 10://硬币投入		
			memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%lu", (TradeCounter.CoinSum1y+TradeCounter.CoinSum5j)/SystemParameter.curUnit );
			break;
		case 11://纸币投入		
			memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%lu", (TradeCounter.CashSum)/SystemParameter.curUnit );
			break;
		case 12://硬币出币		
			memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%lu", (TradeCounter.Hopper2Sum+TradeCounter.Hopper1Sum)/SystemParameter.curUnit );
			break;	
		case 13://区间硬币投入		
			memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%lu", (TradeCounter.CoinSum1yBack+TradeCounter.CoinSum5jBack)/SystemParameter.curUnit );
			break;
		case 14://区间纸币投入		
			memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%lu", (TradeCounter.CashSumBack)/SystemParameter.curUnit );
			break;
		case 15://区间硬币出币		
			memcpy( line, CurrencyMenuStr_1[iIndex].str, CurrencyMenuStr_1[iIndex].len );
	        len = CurrencyMenuStr_1[iIndex].len;
			len += sprintf( line + CurrencyMenuStr_1[iIndex].len, "%lu", (TradeCounter.Hopper2SumBack+TradeCounter.Hopper1SumBack)/SystemParameter.curUnit );
			break;		
        default:
        break;

	}
	Trace("\n %s ",line );
	Trace("\n len = %bu",len );
	DisplayStr( 0, yPos, 1, line, len );	

}


/*交易记录维护主函数*/
void CounterMainMenu()
{
	u_char xdata key = 0;
	bit	data Notout = 1;
	u_char  xdata firstNum = 1;//一级菜单编号
	u_char  xdata LineNum = 2;//选中屏幕的行号	
	u_char xdata  MenuNum = 0;
	

	ClearDisplayLine( 1 );
	ClearDisplayLine( 2 );		
	DispCurrencyList( 0, 0 );
	DispCurrencyList( 1, 1 );
	DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标
	MenuNum = MENU_COUNT( CurrencyMenuStr ) - 1;	
	
	while( Notout )
	{		
		/*判断按下何键*/	
		key = GetKey();
		switch( key )		
		{
			case KEY_UP://上移  	
			    if( LineNum == 2 )//选中标记上移
			    {			     
			    	DisplayCursorState( 0, 0, 1, 2, 1 );//第一行整光标
			    	LineNum = 1;
					if( firstNum == 0 )
						firstNum = MenuNum;					
					else
						firstNum--;			    
			     }
			     else//菜单上翻
			     {
				 	DispCurrencyList( 1, firstNum );			     
				  	if( firstNum == 0 )
				  		firstNum = MenuNum;					
				  	else
						firstNum--;						
					DispCurrencyList( 0, firstNum );
				  	LineNum = 1;										
			     }
				 break;
			case KEY_DOWN://下移				
				if( LineNum == 1 )//选中标记下移
				{			
				     DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标
					 if( firstNum == MenuNum )
						firstNum = 0;					
					 else						
				    	firstNum++;				     
				     LineNum = 2;				     
				 }
				 else//菜单下翻
				 {
				 	DispCurrencyList( 0, firstNum );
					if( firstNum == MenuNum )
						firstNum = 0;					
					else				    	 
				    	firstNum++;				
					DispCurrencyList( 1, firstNum );
					LineNum = 2;					
				}
				break;
			case KEY_CANCEL:
				Notout = 0;
				break;					
		}
		SchedulerProcess();//让出时间片
		WaitForWork( 100, &Lcd );
	}
}



/*交易记录维护主函数*/
void CounterMainMenu_1()//120521 by cq SellAccumulate
{
	u_char xdata key = 0;
	bit	data Notout = 1;
	u_char  xdata firstNum = 1;//一级菜单编号
	u_char  xdata LineNum = 2;//选中屏幕的行号	
	u_char xdata  MenuNum = 0;
	

	ClearDisplayLine( 1 );
	ClearDisplayLine( 2 );		
	DispCurrencyList_1( 0, 0 );
	DispCurrencyList_1( 1, 1 );
	DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标
	MenuNum = MENU_COUNT( CurrencyMenuStr_1 ) - 1;	
	
	while( Notout )
	{		
		/*判断按下何键*/	
		key = GetKey();
		switch( key )		
		{
			case KEY_UP://上移  	
			    if( LineNum == 2 )//选中标记上移
			    {			     
			    	DisplayCursorState( 0, 0, 1, 2, 1 );//第一行整光标
			    	LineNum = 1;
					if( firstNum == 0 )
						firstNum = MenuNum;					
					else
						firstNum--;			    
			     }
			     else//菜单上翻
			     {
				 	DispCurrencyList_1( 1, firstNum );			     
				  	if( firstNum == 0 )
				  		firstNum = MenuNum;					
				  	else
						firstNum--;						
					DispCurrencyList_1( 0, firstNum );
				  	LineNum = 1;										
			     }
				 break;
			case KEY_DOWN://下移				
				if( LineNum == 1 )//选中标记下移
				{			
				     DisplayCursorState( 0, 1, 1, 2, 1 );//第二行整光标
					 if( firstNum == MenuNum )
						firstNum = 0;					
					 else						
				    	firstNum++;				     
				     LineNum = 2;				     
				 }
				 else//菜单下翻
				 {
				 	DispCurrencyList_1( 0, firstNum );
					if( firstNum == MenuNum )
						firstNum = 0;					
					else				    	 
				    	firstNum++;				
					DispCurrencyList_1( 1, firstNum );
					LineNum = 2;					
				}
				break;
			case KEY_CANCEL:
				Notout = 0;
				break;					
		}
		SchedulerProcess();//让出时间片
		WaitForWork( 100, &Lcd );
		if(sysVPMission.msActTimer == 0)
		{
			sysVPMission.msActTimer = 100;
			VPMission_Act_RPT(VP_ACT_ADMIN,VP_ACT_ENTERADMIN);
		}
	}
}

