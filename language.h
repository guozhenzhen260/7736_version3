#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

#include "global.h"

#define STR_WELCOME 0
#define STR_COIN_NOT_ENOUGH 1//硬币不足
#define STR_INPUT_CODE_TIMEOUT 2//选货超时
#define STR_DOMAIN_ERROR 3     /*面值错误*/
#define STR_INPUT_GOODS 4 /*输入货道编号*/
#define STR_OK_NOT 5 /* 选择确定*/
#define STR_GET_COIN 6 /*取回硬币*/
#define	STR_CANCEL 7 /*客户取消 */
#define STR_INVAILD_CHOICE 8 /* 无效选择 */
#define	STR_CONFIRM 9 /*客户取消 */
#define STR_INSERT_BILL_COIN 10 /*插入纸币和硬币 */
#define STR_CUSTOM_PAYED 11 /* 用户已经付出的金额统计 */
#define STR_CHOICE_CHEAPERONE  12 /*选择更便宜的同类卡 */
#define STR_HARDWARE_ERR 13 /* 硬件故障 */
#define STR_GOODS_SOLDOUT 14 /*货已售空 */
#define STR_THX_YOU_WITHOUT_COIN 15 /* 谢谢使用，请取回硬币找头 */
#define STR_OUT_OF_SERVICE 16 /*暂停服务 */
#define STR_HOPPER1_ERR 17 /*硬币模块１故障 */
#define STR_PLEASE_WAIT 18 /*请稍候*/
#define STR_NOT_ENOUGH_NOTE 19 /*钱步足*/
#define STR_TRANCE_ING	20//正在操作
#define STR_OUTING_COIN	21//正找零钱
#define STR_CHANGECOIN_ERRCFG 22//配币失败
#define	STR_PRESS_ANYKEY 23 //按任意键开始
#define STR_RE_INPUT_GOODS 24 /*重选货物编号*/
#define STR_INPUT_WAY_NO 25 /*重新输入*/
#define STR_TAKE_GOODS 26 /*取走货物*/ 
#define STR_TAKE_COIN 27 /*取走硬币*/
#define STR_GOODS_WAY_ERR1 28 /* 货道故障 */
#define STR_GOODS_WAY_ERR2 29 /* 货品售价 */
#define STR_OPERATION_TIMEOUT 30 /*操作超时*/
#define STR_REFUND_MONEY 31 /*退款*/
#define STR_DUO_MONEY 32 /*欠款*/
#define STR_INSERT_COIN 33 /*插入硬币 */
#define STR_INSERT_BILL 34/*插入纸币*/
#define STR_INSERT_MONEY 35/*插入适当的钱*/
#define STR_TYPE_VER        36   /*机型和版本号*/
#define STR_RE_BUY          37   /*再次购买商品*/
#define STR_SHORT_CHANGE    38   /*找零器内硬币不足*/
#define STR_PRICE           39   /*欢迎购买*/
#define STR_COLUMN          40   /*货道*/
#define STR_GOODS_OUT       41  /*无货*/
#define STR_COLUMN_ERROR    42  /*故障*/
#define STR_CHANGE          43  /*找零*/
#define STR_BALANCE         44  /*余额*/
//-------------------------------------------------------
#define STR_BC_SCAN_CARD     45  //SCAN the card
#define STR_VP_M_S_G         46  //Money-Selection-Goods
#define STR_VP_S_C_G         47  //Selection-Card-Goods
#define STR_VP_CARD_MONEYLOW 48
#define STR_VP_CARD_READERR  49
#define STR_VP_CARD_BALANCE  50
#define STR_VP_BALANCE_LOW   51
#define STR_VP_CASH_TRADE     52
#define STR_VP_CASHLESS_TRADE 53
#define STR_VP_SELECTGOODS    54
//=======================================================
#define STR_OUT_OF_CASHSERVICE 55 /*暂停现金交易 */
#define STR_COLUMNNO          56   /*货道*/
#define STR_GOODS_WAY_ERR3    57 /* 该货道暂停服务 */
#define STR_GOODS_WAY_NOSALE  58 /* 非现金售卖商品 */
#define STR_COIN_UNPAY        59
#define STR_INPUT_GOODSNO     60 /*输入货道编号*/
#ifdef _CHINA_
	const struct StrAndLen code DispInformationTable[] =
	{
		{"      欢迎光临", 14 },
        {"      零钱不足", 14 }, //16			
        //{"    不找零钱", 12 },
		{"    选择商品超时    ", 20 },	//2		
		{"  请插入更小面额纸币", 20 },
		//{"输入商品编号: ", 14 },      //4
        {"  请选货或继续投币!", 19  },  //4	
		{"确定ENTER 取消CANCEL", 20 },	//5
		{"    请取零钱", 12 },	        //6	
		{"取消CANCEL", 10 },            //7
		{"    输入错误", 12 },          //8			
		{"确定ENTER", 9 },              //9
		{"  请投入纸币或硬币", 18 },    //10	
		{"客户余额:", 9 },//11
		{"选择其它商品吗? ", 16  },//12	
		{"    机器故障", 12 },//13
		{"  商品已售完", 12 },//14	
		//{"      谢  谢", 12 },//15
		{"      谢谢!", 11    }, //15
		{"      暂停服务", 14 }, //16		
		{"    找零器故障", 14 }, //17	
		{"  请稍后...", 12    },  //18	
		{"金额不足, 加钱吗? ", 18 },
		{"  \xd5\xfd在出货...", 13    }, //20
		{"  \xd5\xfd在找零...", 13    }, //21
		{"    找零失败", 12 },//22
		{"    How are you!", 16 },     //23
		{"请输入其它商品编号", 18 },   //24
		//{"请输入其它商品编号", 18 },   //25
        {"  请选择其它商品", 16 },       //25
		{"  请取货...", 11 },            //26
		{"  请取走退币...", 15   },      //27
		{"  商品货道故障", 14    },      //28
		{"  选择其它商品吗?", 17 },    //29	
		{"      操作超时", 14 },       //30		
		{"    \xd5\xfd在退款", 12 },   //31		
		{"欠款:", 5 },                 //32
		{"    请投入硬币", 14 },       //33	
		{"    请插入纸币", 14 },       //34		
		{"(2014-09-22)", 12 },   //35
		{"Junpeng-zonghe-1.0", 18 },    //36_V1.0-		
		{"  还要购买商品吗?",17},      //37
		//Added by Andy on Oct 9,2008
	    //{"    不找零钱", 12 },       
		{"      无零币", 12 },         //38 
		{"  商品价格:",  11 },         //39
	  //--------------------------------------------------
      //{"  商品", 6 },
        {"  所选商品暂时", 14 },       //40
 	  //{"  所选商品", 10 },
	  //{"  所选商品暂时缺货", 18},    //41	
      //{"  商品缺货", 10 },           //41
	    {"缺货", 4   },                //41
		{"  故障", 8 },                //42	
		{"  退币金额:", 11 },          //43
		//{"    余额: ",10 },          //44
        {"  当前金额:", 11 },          //44
        //---------------------------------------------------
        {"  请刷卡...", 11 },          //45
        {"  投币->选货->取货  ", 20 }, //46
      //{"  选货->贴卡->取货  ", 20 }, //47
        {"  贴卡->选货->取货  ", 20 }, //47
        {"  卡余额不足...",    15   }, //48
		{"  读卡失败...",      13   }, //49
		{"  卡余额:",          9    }, //50
		{"  金额不足...",      13   }, //51
        {"      现金消费",     14   }, //52
		{"      刷卡消费",     14   }, //53
		{"  请选货!",          9    }, //54
        //===================================================
        {"  本机暂停现金交易", 18 }, //55
        {"货道: ", 6 },              //56
        {"  该货道暂停服务", 16    },      //57
        {"  非现金售卖", 12    },      //58
        {"零币不足，无法购买", 20   },      //59
        {"输入商品编号: ", 14 },     //56
	};
#else
	const struct StrAndLen code DispInformationTable[] =
	{
		{"       Welcome", 14 },
		{"Not enough coins", 16 },
		{"Choose goods timeout", 20 },	//2		
		{"Insert a smaller one", 20 },
	//	{"Input goods code:", 17 },//4	
		//{"Make Selection:", 15 },//4
	    {"Make Selection!", 15 },
		{"  ENTER   CANCEL", 16 },	//5
		{"Collect coins", 13 },	//6	
		{"CANCEL", 6 }, //7
		{"Invalid choice", 14 },//8			
		{"ENTER", 5 },//9
		{"  Insert bill/coin", 18 },//10	
	//	{"Price paid:\x24", 12 },//11
		{"Price paid:", 11 },//11
		{"Choose other goods?", 19  },//12	
		{"Hardware error", 14 },//13
		{"      Sellout", 13 },//14	
		{"     Thank you", 14 },//15
		{"Out of service", 14 },//16		
		{"Hopper is wrong", 15 },//17	
		{"Please wait...", 14 },//18	
	//	{"Fund shortage,add?", 18 },
		{"Insufficient fund?", 18 },
		{"Taking the goods", 16 },//20
		{"  Changing", 14 },//21
		{"   Change failed", 16 },//22
		{"    How are you!", 16 },//23
		{"Input other code", 16 },//24
		{"  Input other code", 18 },//25
		{"Take away goods", 15 },//26
		{"  Take away changes", 19 },//27
		{"   Column error", 15 },//28
		{"Select another one?", 19 },//29	
		{"Operation timeout", 17 },//30		
		{"    Refund of fare", 18 },//31		
		{"IOU:", 4 },//32
		{"    Insert coin", 15 },//33	
		{"    Insert bill", 15 },//34		
//		{"NoChange-ExactAmount", 20 }, //35		
		{"Exact change require", 20 }, //35		
		{"EV7736-UB MDB V1.72", 20 },    //36_V1.0-
		{" Buy another one?",17},      //37
		{"Insufficient change",19},    //38//不能找零
		{"  Price:", 8 },              //39
        {"Column", 6 },                //40
		{"Out", 9 },                   //41
		{"Error", 5 },                 //42
		{"  Change :", 10 },           //43 
		{"  Balance:", 10 },         //44
        //-------------------------------------------
        {"Pls scan the card!", 18 },   //45 
        {"Money->Sel->Goods",  17 },   //46
        {"Sel->Card->Goods",   16 },   //47
		{"Card's balance low", 18 },   //48
		{"Read card fail!",    14 },   //49
		{"  Balance:",         10 },   //50
		{"More money require", 18 },   //51
		{"  Cash trade",       12 },   //52
		{"  Cashless trade",   16 },   //53
		{"  Select the goods", 18 },   //54
		{"Stop trade", 17 },//4	   //55
		{"Input goods code:", 17 },//4	   //56  
		{"   Column error", 15 },//57
        {"   Column unSale", 16 },//58
        {"   Column unSale", 16 },//59
        {"Input goods code:", 17 },//4	   //56  
        //===========================================
	};
#endif

#endif