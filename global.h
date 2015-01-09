#ifndef __GLOBAL__
#define __GLOBAL__

#define MACHINE_SET_MDB
#define TIME_BILL_STACK     5//3-5
#define WATCHDOGTIMEOUT     120//240
#define _DEBUG_NO_MOBILE    //Added by andy on 2010.12.22
#define SELECTION_NUM       3
#define RACKNUM		        8
#define RACKCOLUMNNUM		8
#define GOODSWAYNUM		    64  

#define HOPPERNUM 		    3
#define VERFYMESSAGELEN 	50
#define ADDONPHONEMAX 		5
#define ONEWAYGOODSNUM 		10  //每货道满货时的商品数量
#define _CHINA_                 //打开为中文交互界面，关闭为英文交互界面
#define _SHUPING_              //定义为竖屏 
#define _SJJ_                  //定义为升降机


typedef unsigned int  u_int;
typedef unsigned char u_char;
typedef unsigned int  uint_PARALUE;	// 金额类型定义，双字节一位定点
typedef unsigned long u_long;

//#define _BAOLIAN_ //(COINTYPE)用硬币器名称指定硬币器类型，此款为宝联
#define _GUANYU_ //(COINTYPE)用硬币器名称指定硬币器类型，此款为冠宇
//#define _COIN_INPUT_SERIAL  //以串口方式使用硬币器
#define _COIN_INPUT_PARALLEL  //以并口方式使用冠宇硬币器

//#define _MOBILE_SMS_1;		//用短信进行远程管理，版本是1.0的

//#define _MOBILE_GPRS;		//用GPRS进行远程管理

#define HUINT16(n)				(((n) >> 8) & 0xff)//16位数值高8位
#define LUINT16(n)				((n) & 0xff)//16位数值低8位	
#define H0UINT32(n)				(((n) >> 24) & 0xff)//32位数值第一个高8位
#define H1UINT32(n)				(((n) >> 16) & 0xff)//32位数值第二个高8位
#define L0UINT32(n)				(((n) >> 8) & 0xff)//32位数值第一个低8位
#define L1UINT32(n)				((n) & 0xff)//32位数值第二个低8位

#ifdef _CHINA_
	#define BILLMULTIPLE	1 //100  //纸币陪率，即纸币器每通道的实际面值陪数	
#else
	#define BILLMULTIPLE	1 //100  //纸币陪率，即纸币器每通道的实际面值陪数	
#endif

#define COINMULTIPLE 		1 //100   //硬币陪率，即硬币器每个脉冲的实际面值陪数,用于串口工作方式
#define MONEYMULTIPLE 		1 //100   //显示金额时的精确度,1表示无小数点(即以元为单位),10表示小数点后1位(即以10分为单位)，100表示小数点后2位(即以分为单位)

#define MENU_COUNT(menu) (sizeof(menu) / sizeof( struct StrAndLen ))

#define MAX_SMS_LIST	10	//短信队列的长度

#define MAX_SMS_LEN		160	//因为用7位编码发送，每短信的最大长度,byte
//定义此变量则为7位编码，关闭则为16位编码
#define SMSENCODE_7_BIT

#define SYS_GOOS_SOLDOUT_CTR    //If need to manage the goods soldout status  
#define GOODSTYPEMAX2        18


struct SYSTEMPARAMETER			// 本机系统参数定义
{		
	u_char 		RefundPermission;		// 是否允许未买退币 1 表示允许 , 0 表示不允许		
	u_int 		HopperCoinPrice1;		// 找零机构1中硬币面值	
	u_int 		HopperCoinPrice2;		// 找零机构1中硬币面值	
	u_int 		HopperCoinPrice3;		// 找零机构1中硬币面值
	u_char 		BillNo;		            // 是否开启纸币器
	u_char 		IsMuliVerd;		        // 是否开启多次购物
	u_int 		Min;		            //再次购买的最低额
	u_int 		BanknoteMax;	        //允许接收的最多的钱
	u_int  		DoubtCash;	            //最后一次有疑问的收纸币金额
	u_int 		NotOutMax;	            //最后一次欠的钱
    u_char 		MuchOutFlag;            //最后一次是否有多找钱
	u_char		VmID[5];			    //本机ID号码
	u_char		GSMNO[16];		       	//服务器端手机号码	
	u_char		MobileON;		        //是否开启手机模块
    //----------------------------------------------------
    //Added by Andy on 2010.8.17
    u_char      printerOn;            //open printer
	u_char      printerErrIfService;  //
	u_int       billValue[8];         //The bill value for each channel. If 0, disable the channel.
	u_int       coinValue[6]; 
    u_char      currencyRate;         //Change rate...
    u_char      printMode;            //0: reserved mode; 1:without overprint; 2:overprint;
    u_char      decimal;  
	u_int       curUnit;              //1: 1 dollar; 10: 0.1 dollar--10 cents; 100: 0.01 dollar-- 1 cent;
	u_char      SVC_NoChange;         //
    u_char      GOCOn;                //1: open the GOC device
    u_char      SVC_GOCErr;           //1: service when GOC error
	u_int       tradeTime;	
	u_char      coinOn;
	u_char      busCardOn;
	u_char      BCReadTime;
	//u_char      holdingMode;
    //u_char      holdNoteFive;   //by gzz 20110419
    //u_char      holdNoteTen;    //by gzz 20110419
	u_char      ACDCModule; 
	u_char      CompModule;
	u_char      selItem[GOODSTYPEMAX2];       //Selection item
    //====================================================
	u_char 		Checkbyte;	//校验位
};


/*各设备状态0：正常，各位置1的定义见说明*/
struct DEVICESTATUS
{
	u_char BillAccepter;      //收纸币器状态，Bit 7 -- 无此模块；Bit 6~1 保留；Bit 0 -- 故障；
	u_char CoinAccepter;      //收硬币器状态，Bit 7 -- 无此模块；Bit 6~1 保留；Bit 0 -- 故障；
	u_char ChangeUnit1;       //#1退币器，Bit 7 -- 无此模块；Bit 6~2 保留；Bit 1 -- 缺币；Bit 0 -- 故障；				
	u_char ChangeUnit2;       //#1退币器，Bit 7 -- 无此模块；Bit 6~2 保留；Bit 1 -- 缺币；Bit 0 -- 故障；				
	u_char ChangeUnit3;       //#1退币器，Bit 7 -- 无此模块；Bit 6~2 保留；Bit 1 -- 缺币；Bit 0 -- 故障；				
	u_char PickupLED;         //电源状态，Bit 7 -- 无此模块；Bit 6~1 保留；Bit 0 -- 电源无输出；	
	u_char KeyBoard;          //键盘，Bit 7 -- 无此模块；Bit 6~1 保留；Bit 0 -- 故障；
	u_char LCD;               //LCD，Bit 7 -- 无此模块；Bit 6~1 保留；Bit 0 -- 故障；				
	u_char MobileStates;              //手机模块，Bit 7 -- 无此模块；Bit 6~1 保留；Bit 0 -- 故障;
	u_char GOCStatus;                 //
    u_char Selection[SELECTION_NUM];  //
	u_char BusCard;
    u_char ACDCModule;
    u_char Driver64;
};

struct StrAndLen
{
	unsigned char *str;
	unsigned char len;
};

struct RackSet//货架配置
{
	u_char  RackNo; //层架编号
	u_char  UseState;//启用标志,0为未启用,1为启用
	u_char	GoodsMax;//本层架内所有货道存货上限
};

struct RackColumnSet //每层货道启用配置
{
	u_char	ColumnNo;//货道序号,1--8
	u_char  UseState;//开启状态,0为未开启,1为开启,	
};

////在这个结构中的所有货道都是启用的货道
struct WaySet //货道设置 6byte
{
	u_char  	WayNo;				//货道编号
	//货道状态字节的定义
	//bit0为0表示开启但状态不确定,状态不确定是指开配置好,没有经过测试,为1表示已测试过,只要是测试则就要置此位为1;
	//bit1为0表示正常,bit1为1表示故障(2),bit2为1表示缺货(4),bit3为1表示货道板板坏(8)
	//bit7、bit6、bit5故障报警验正位,当对应位为1时表示相应故障已发送过,bit5为1表示缺货,bit6为1表示故障；
	u_char 		WayState;	
	u_int		Price;				//单价	
	u_char		GoodsCurrentSum;	//货品现存量
	u_char      GoodsType;          //2010.12.16 changed
	u_char      IsUsed;             //1已经使用过一次了
	
};

////在这个结构中的所有货道都是启用的货道
struct WayList //货道编号对应列表结构
{
	u_char    GoodsWayNo;//逻辑编号
	u_char    SetArrayNo;//物理编号
	u_char	  UseState;	 //货道使用状态,0为未使用,1为开启
	u_char	  GoodsMax;	 //货道存货上限
};


struct HopperSet //Hopper排序结构
{
	u_int    price;
	u_char   HopperIndex;
	u_char   HopperState;//1为正常,2为故障
};

struct TRADECOUNTER					// 本机运行参数定义 22byte
{		
	//u_long  TrueCurrencySum;   //正常接受的总金额计数, 单位: 分	
	//u_long  CoinSum;           //收入的硬币总额
	u_long  CashSum;	      //收入的纸币总额
    u_long  CashSumBack;
    u_long  CoinSum5j;
    u_long  CoinSum5jBack;
    u_long  CoinSum1y;
    u_long  CoinSum1yBack;
    u_long  Hopper1Sum;	
    u_long  Hopper1SumBack;
    u_long  Hopper2Sum;	
    u_long  Hopper2SumBack;
    //u_long  TotalSellNum;//120419 by cq TotalSell
    //u_long  TotalSellMoeny;//120419 by cq TotalSell

	//120521 by cq SellAccumulate
	u_long   vpSuccessNum;                    //全部出货数量
	u_long   vpSuccessMoney;                  //全部出货金额
	u_long   vpCashNum;                       //现金出货数量
	u_long   vpCashMoney;                     //现金出货金额
	u_long   vpGameNum;                       //游戏出货数量 
	u_long   vpCardNum;                       //刷卡出货数量
	u_long   vpOnlineNum;                     //在线出货数量
	
	//------------------------------------------------
	//离线销售数据
	//u_char	 offLineFlag;                   //是否离线处理 
	u_long   offLineNum;                    //离线出货数量
	u_long   offLineMoney;                  //离线出货金额

	
    //============================================   

};

struct TRADELOG//单货道交易统计记录 7byte
{
	u_char 	WayNo;		    //货道编号
	u_int   TraceSum;	    //货品无疑问出货交易次数计数, u_char-u_int(changed by Andy on 2010.8.19)		
	u_int   DoubtTraceSum;	//货品有疑问出货交易次数计数，u_char-u_int(changed by Andy on 2010.8.19)		
	u_int  	MoneySum;	    //有疑问时疑问收入的总额
	u_int 	TradeSum;       //有疑问时疑问交易的总金额计数, 单位: 分	
};

struct SMSMessageList//待发的短信队列
{
	u_char 	SendTime;	//发送时间
	u_char  Flag;	//用于记录否是需要等计时器轮回，1是需要轮回，0不需要
	u_char  SMSLen;		//短信长度
	u_char  SMSMessage[MAX_SMS_LEN];	//短信内容
};

struct PassWordLog//所有货道交易统计记录 7byte
{
	u_char passwdlen;		//密码长度
	u_char checkbyte;	//货品无疑问出货交易次数计数，		
	u_char PassWordByte[10];	//货品有疑问出货交易次数计数，		
};

extern struct DEVICESTATUS xdata DeviceStatus;
extern struct SYSTEMPARAMETER xdata SystemParameter;

extern struct RackSet	xdata iRackSet[RACKNUM];//货架配置
extern struct RackColumnSet xdata iRackColumnSet[RACKNUM][RACKCOLUMNNUM]; //每层货道启用配置
extern struct WayList xdata InputGoodsWayList[ GOODSWAYNUM ];
extern struct WaySet  xdata GoodsWaySetVal[GOODSWAYNUM];//货道参数设置

extern struct TRADECOUNTER xdata TradeCounter;
//extern struct TRADELOG xdata TradeLog[GOODSWAYNUM];//各货道的交易统计
extern bit data  SystemSave ;//系统是否保存
extern unsigned char xdata  GoodsSetSave;//货道参数保存

//货币设备报警验证字节，各bit为1表示对应的故障已报警,具体定义为：
//bit7验证纸币器，bit6验证硬币器
//bit0-bit1验证hopper1,bit0验证缺币、bit1验证设备故障,下面类推
//bit2-bit3验证hopper2
//bit4-bit5验证hopper3
extern unsigned char xdata  MoneyDevAlarm;

//其它故障报警验证字节，各bit为1表示对应的故障已报警,具体定义为：
//bit0验证暂停服务,bit1验证系统参数校验,bit2验证主板与显示板通讯
//bit3验证所有货道无货,bit4验证所有货道故障,
//bit5验证所有货道均不能正常工作,bit6验证主板与货道驱动板通讯故障
//bit7保留
extern unsigned char xdata  SystemErrorAlarm;//货道参数保存

extern struct SMSMessageList xdata SMSMessage[MAX_SMS_LIST];

//extern unsigned char xdata ListIndex;


//--------------------------------------------------------------
//20_140&40_140&60_140
#define SYS_WAIT_NEXT_PULSE 200   //
#define SYS_WAIT_CARD_IN    1500
#define MACHINE_SET_VMC_PC  
//#define VP_PRO_VER          2//1--2版本号设为2;by gzz 20110823
#define VP_MAC_SET_H        0
#define VP_MAC_SET_L        1

#define KEYEXTRAVAL         0x06
#define GOODSTYPEMAX        18
#define EACHGOODSCOLUMNMAX  48
/*
#ifdef   _SJJ_//升降机
	#define COLUMN_NUM_SET      56		
#else         //普通货道
	#define COLUMN_NUM_SET      48
#endif
*/
extern u_char xdata COLUMN_NUM_SET;

#define GOODSMATRIXMODE
#define GOODS_MATRIX_NONE   0xff
#define SELECTION_VAL_MIN   0
#define SELECTION_VAL_MAX   17
#define DSP_WELCOME_UPDATE  8

#define VP_DSP_INTERVERL    250                 //200-250
#define VP_DSP_TIME1        VP_DSP_INTERVERL
#define VP_DSP_TIME2        VP_DSP_INTERVERL*2


//#define SELECTION_NUM     3
/*
#define INPUTGOODSSUM_2     2
#define INPUTGOODSSUM_3     3
#define INPUTGOODSSUM       INPUTGOODSSUM_3
*/
//The system goods matrix_33 bytes
/*
struct GoodsMatrix
{
	u_char   GoodsType;    //商品编号
	u_int    Price;        //商品单价
	u_char   Status;       //00-ok, !=0 error,
	//u_char   Led;          //
	u_char   ColumnNum;
	//u_char   ColumnMatrix[EACHGOODSCOLUMNMAX];   //Just the GoodsWaySetVal's address.                
	//u_char   NextColumn;                         //FF is none good column left.
};
extern struct GoodsMatrix xdata sysGoodsMatrix[124];
*/

#define VPM_LEN_MAX   250
struct VMC_PC_MSG
{
    u_char   sf;                //fixed with 0xe7
	u_char   len;               //5~250, msg+5
    u_char   datLen;            //
	u_char   verFlag;           //0x01( none response ); 0x81( need response )
    u_char   sn;                //0~255
	u_char   msgType;           //
	u_char   msg[VPM_LEN_MAX];  //0~250
	u_int    chk;
};

#define VPM_COM_TIMEOUT   1500   //1.5s
#define VPM_STA_PWRON     0x00
#define VPM_STA_FIRSTCOM  0x01
#define VPM_STA_GOODSPAR  0x02
#define VPM_STA_COLUMNPAR 0x04
#define VPM_STA_INITOK    0x08
#define VPM_STA_PCPAROK   0x80

#define VPM_DEVUPDATE_BUSCARD   300
#define VPM_DEVUPDATE_CHANGER1  240  //120-240
#define VPM_DEVUPDATE_CHANGER2  240
#define VPM_DEVUPDATE_CHANGER3  240
#define VPM_DEVUPDATE_GOC       180//20-180   
#define VPM_DEVUPDATE_HOPPEREMP 180
#define VPM_DEVUPDATE_TUBEREMOVE 3//30-3;by gzz 20111017 

struct VP_Mission
{
    struct  VMC_PC_MSG  send;
	struct  VMC_PC_MSG  receive;
    u_char  comLock;
	u_int   msTimer1;
	u_int   msTimer2;
    u_int   msTimer3;          //for VP_GameKeyPoll
    u_int   msActTimer;
	u_int   msGameTimer1;
    u_int   sTimer1;
	u_int   sTimer2;
    u_int   billSTimer;
	u_char  sTimerHeart;
	u_char  sTimerOffLine;//离线poll
	//u_char  sWeihuTimer;//退出维护页面
    //bit0 - bus card reader
    //bit1 - changer 1
    //bit2 - changer 2
    //bit3 - changer 3
	//bit4 - GOC
	//bit5 -
	//bit6 -
	//bit7 - poll pc first
    u_char  VPDevCtr;     //
	u_int   sTimerBC;
	u_int   sTimerChanger1;
    u_int   sTimerChanger2;
    u_int   sTimerChanger3;
	u_int   sTimerGOC;
	//-----------------------------
	u_char  GoodsRev; //商品本次下传的商品编号数量
	//------------------------------------------------------
	u_int   dspTimer1;
	u_char  dspUpdateFlag;
	u_char  coinFull;
    u_char  changeFlag;
    //bit0 - update balance display, after wrong selection
    //bit1 - update balance display and enable the cash devices
    u_char  dspCtr;
	//=======================================================
    u_int   systemErr;
	u_char  sysStatus;                  //
	u_char  PCStatus;
	u_char  VPMode;
	u_char  VPInit;
    u_char  VPMode_Key;
    u_int   comTimer;
	u_char  comErrNum;
    u_char  returnKey;                   //Yes or no
	u_char  gameKey;                     //Yes or no
	u_char  returnKeyLock;               //
	u_char  gameKeyLock;                 //
	u_char  gameLed;                     //On or off
	u_char  key;                         //Selection key
    //u_char  selItem[GOODSTYPEMAX];       //Selection item
    u_int   selPrice[124];      //Selection price
    u_char  columnNum[64];   //Column's storage 
	u_char  columnItem[64];  //Column's item
    u_int   readyLed;                    //
	u_int   redLed;                      //
	u_char  sel1ReadyLed;               //
	u_char  sel1ErrLed;                 //
	u_char  sel2ReadyLed;               //
	u_char  sel2ErrLed;                 //
	u_char  sel3ReadyLed;               //
	u_char  sel3ErrLed;                 //
    //------------------------------------
    u_char  ACDCLedCtr;
    u_char  ACDCCompressorCtr;
    //------------------------------------
	//u_char  proVer;
	u_char  columnNumSet;
	u_char  selectionNumSet;
	u_char  scalFactor;
	u_char  decimalPlaces;//基本单位1角,2分,0元
	//------------------------------------
	u_char  payInDev;
	u_int   payInMoney;
	u_char  payOutDev;
	u_int   payOutMoney1;
	u_int   payOutMoney2;
	u_int   payOutMoney3;
	u_char  payInCoinFlag;
	u_char  payInBillFlag;
	u_int   payInCoinMoney;
	u_int   payInBillMoney;
	u_char  escrowInFlag;
	u_char  escrowOutFlag;
	u_int   escrowMoney;
	u_int   inBillMoney;//投入纸币金额;
	u_int   tubeCoinMoney;//可找零金额;
    //------------------------------------
    u_char  changeCmd;
    u_char  changeDev;
    u_int   changeMoney;
	u_char  changeType;
    //------------------------------------
    //接收扣钱命令;by gzz 20110822
    u_char  costCmd;
    u_char  costDev;
    u_int   costMoney;
	u_char  costType;
    //------------------------------------
    //上传欠条数据;by gzz 20110825
    u_char  debtType;
    u_char  debtDev;
    u_int   debtMoney;
	//------------------------------------
	u_char  vendGoods;
	u_char  vendSta;
	u_char  vendColumn;
	u_char  vendType;
	u_int   vendCost;
	u_char  vendcolumnLeft;
    u_char  vendCmd;
	u_char  goodsType;
	u_char  goodsType2;
    //------------------------------------
	u_char  request;
	u_char  requestFlag;
	//------------------------------------
	u_char  ADM_Type;
	u_char  ADM_Dat[3];
    //------------------------------------
	u_char  ACT_Type;	
	u_char  ACT_Value;
	//-------------------------------------
	u_char  BUT_Type;
	u_char  BUT_Value;
	//-------------------------------------
	u_char  Action_Type;
	u_char  Action_Value;
	//-------------------------------------
	u_char  STA_CoinA;
	u_int   STA_ChangeNum1;
    u_int   STA_ChangeNum2;
	u_char  STA_BillA;
    u_char  STA_Changer1;
	u_char  STA_Changer2;
	u_char  STA_VMC;
	u_char  STA_Tep1;
	u_char  STA_Tep2;
	u_char  STA_Tep3;
	u_char  STA_Bus;		
	u_char  STA_ICCard;
    u_char  E_Tem1;      //For ACDC module 0xfd无意义,0xff故障,0xfe不存在
    u_char  E_Tem2;      //For c
    u_char  ESTA_Tem1;
	u_char  ESTA_Tem2;
    //u_int   STA_BillCount;//只有连续故障次数大于5次时，才上报;by gzz 20110905
    u_int   STA_ICCardCount;//只有连续故障次数大于5次时，才上报;by gzz 20111012
    u_char  Payout_Time;
    //-------------------------------------
    u_char  resetDev;
    u_char  resetCmd;
	//-------------------------------------
	u_char  ctrType;
	u_char  ctrValue;
	u_char  billCoinEnable; 
    //u_char  sTimerHopperEmp;//by gzz 20110502
    //-------------------------------------
	u_char  menuTimer;
	u_char  menuFlag;
	u_char  menuKeyNum;
    u_char  menuKey[5];
    //-------------------------------------
    u_char  hopperComErr[3];
	u_int   hopperTimer;
    u_int   hopperCoinNum;
	u_int   hopperOutTimer;
    //-------------------------------------
    u_int   watchDogTimer;
	u_char  watchDogCtr;
	u_char  watchDogIng;
    //-------------------------------------
    u_char  boughtFlag; 
    //-------------------------------------
    u_char  SystemState;//1是非现金交易状态
    u_char  hardWareErrShow;//保存硬件故障值，用于显示
    u_char GOCTestFlag;//出货检测标记
	u_int   sTimertubeRemoved;//检测币管是否锁紧;by gzz 20110827
	u_char sTimerTubeUpdate;//出货检测标记
	
	//-----------------------------
	u_char infoType;
	u_char  infoErr;//上报售货机故障;by gzz 20121226
	//------------------------------
	u_char  ErrFlag;//第一次故障时,需要上报记录
	u_char  ErrFlag2;//第一次故障时,进行的一些工作
	u_char  DevErrFlag;//设备故障时，不需要进入维护模式，但是需要上报一次
	u_char  adminType;//是否进入维护模式
	u_char	offLineFlag; //是否离线处理 
	u_char  returnCmd;  //找零反应
	u_char  GoodsWayNum[64];       //Selection item
};
extern struct VP_Mission xdata sysVPMission;
//==============================================================

//--------------------------------------------------------------
#define ITL_TIME_DISABLED_INIT   35    //
#define ITL_TIME_JAMED_DISABLE   30    //

struct ITL_MSG
{
	u_char stx;
	u_char seq;
	u_char len;
	u_char msg[255];
	u_char crcL;
	u_char crcH;
};

struct ITL_Mission
{
    struct ITL_MSG send;
	struct ITL_MSG receive;
	u_char ITLSet;
	u_char crcL;
	u_char crcH;
    u_int  msPoll;
    u_int  msTimer2;
	u_int  sTimer1;
    u_int  sTimer2;     //holding -- credit
	u_char comErrNum;
	u_char status;
	u_char chnCtr1;
	u_char chnCtr2;
    u_char billSta;
    u_char billESta;
    u_char billChn;
	u_int  billValue;
    u_char enableFlag;
    u_char billStaCtr;        //Added 2011.4.13 Andy.
	u_int  billHoldingValue;  //Added 2011.4.13 Andy.
	u_char billHoldingFlag;   //Added 2011.4.13 Andy.
	//
	u_char reEnableFlag;
	u_int  reEnableTime;
    //
    u_char billJamedFlag;
    u_int  billJamedTime;
    //
    u_int  restartTime;
};

extern struct ITL_Mission xdata sysITLMission;
//==============================================================

//--------------------------------------------------------------
#define MDB_MAX_BLOCK_SIZF    36         //数据块最大长度
//#define MDB_ERR_TIME_OUT    0x81       //超时
//#define MDB_ERR_NO_MODE_BIT 0x82       //未收到最后字节
//#define MDB_ERR_CHECKSUM    0x83       //校验和错
#define MDB_ERR_TIME_OUT      0x80       //超时
#define MDB_ERR_NO_MODE_BIT   0x80       //未收到最后字节
#define MDB_ERR_CHECKSUM      0x80       //校验和错
#define MDB_ACK 0x00
#define MDB_RET 0xaa
#define MDB_NAK 0xff

struct MDB_Mission
{
    u_char port0_counter;                //串口0收到字节计数
    u_int  MDBMSTimer;
	u_char modeBit;
    u_char buff[MDB_MAX_BLOCK_SIZF];     //串口0收到字节缓存
	u_char billBuf[MDB_MAX_BLOCK_SIZF];
    u_char coinBuf[MDB_MAX_BLOCK_SIZF];
	u_char Pc_billBuf[MDB_MAX_BLOCK_SIZF];
    u_char Pc_coinBuf[MDB_MAX_BLOCK_SIZF];
	u_char billIDENTITYBuf[MDB_MAX_BLOCK_SIZF];
    u_char coinIDENTITYBuf[MDB_MAX_BLOCK_SIZF];
    u_char billBufLen;
    u_char coinBufLen;
	u_char billType;
	u_char coinType;
	u_char coinStock;   //
	u_char billStock;
    u_char billDeviceStatus;
    u_char coinDeviceStatus;    //bit0(com error)
    u_char billIsEnable;
    u_char coinIsEnable;    //bit0=1代表目前是使能的;by gzz 20110920
    u_int  coin5jNum;
    u_int  coin1yNum;
    u_int  coin5jValue;
    u_int  coin1yValue;
    u_int  coinAllValue;
	u_int  payout5jNum;
	u_int  payout1yNum;
    u_char returnKeyFlag;    //return key pressed
    u_char returnCmdFlag;    //return key pressed
    u_char returnBilFlag;    //return bill
    u_char tubeRemoved;      //币管是否没有锁紧;by gzz 20110827
    u_char billIsFull;      //判断纸币器是否钞箱已满;by gzz 20111017 
    u_char billCashBuf;     //钞箱移除报故障;by gzz 20121224
    u_char billOthErr;     //纸币器其他故障bit0:motor,bit1:sensor,bit2:chksum,bit3:jammed,bit4:disabled,bit5:cashErr
    u_char coinOthErr;     //硬币器其他故障bit0:sensor,bit1:tubejam,bit2:romchk,bit3:routing,bit4:jam,bit5:removeTube
   
};
extern struct MDB_Mission xdata sysMDBMission;

//==============================================================

//--------------------------------------------------------------
struct  BC_MSG
{
	u_char stx;
	u_char len;
	u_char msg[255];
	u_int  crc;
	u_char etx;
};

struct BC_Mission
{
    struct BC_MSG  send;
	struct BC_MSG  receive;
	//-----------------------
    u_int  msPoll;
    u_int  msTimer2;
	u_char comErrNum;
	//------------------------
	u_char recACK;
	u_char recDatLen;
    u_long balance;
    //------------------------
    u_char msgLen;
    u_char msgBuf[255];

};
extern struct BC_Mission xdata sysBCMission;

////在这个结构中保存所有货道本次循环检测所得故障次数
struct COLUMNTEST 
{
    u_int   testNum;  
	u_char  WayNo[GOODSWAYNUM];				//货道编号		
	u_int	BreakCount[GOODSWAYNUM]; 		//故障次数			
};
extern struct COLUMNTEST xdata WayCheck;
//==============================================================

#endif





