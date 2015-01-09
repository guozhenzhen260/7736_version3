#ifndef __VMC_PC_H__
#define __VMC_PC_H__


#define VP_TIME_OUT        150
#define VP_TIME_POLL       50
#define VP_TIME_OUNT_NUM   3
#define VP_TIME_WAIT_GAME  10
#define VP_COM_RETRY       0  

#define VP_ERR_NULL     0
#define VP_ERR_PAR      1   //mission parameter error
#define VP_ERR_COM      2   //communication error

#define VP_SF             0xE7
#define VP_PROTOCOL_VER   0x03//0x01---0x02版本号设为2;by gzz 20110823
#define VP_PROTOCOL_ACK   0x80
#define VP_PROTOCOL_NAK   0x00

#define VP_REQUEST_G_C    0x01
#define VP_DEV_COIN       0
#define VP_DEV_BILL       1
#define VP_DEV_BILLCOIN   2
#define VP_DEV_BANKCARD   3
#define VP_DEV_MOBILE     4
#define VP_DEV_SCHOOL     5
#define VP_DEV_PAYOUT     6
#define VP_DEV_CTR_COMPRESSOR1 7
#define VP_DEV_GAME_LED        16
#define VP_DEV_SCALFACTOR      18
#define VP_DEV_ESCROWIN        100
#define VP_DEV_ESCROWOUT       101

#define VP_ADMIN_GOODSADDALL  1
#define VP_ADMIN_GOODSADDCOL  2
#define VP_ADMIN_CHANGEADD    3
#define VP_ADMIN_GOODSCOLUMN  4
#define VP_ADMIN_ENTER_MENU   5
#define VP_ADMIN_QUIT_MENU    6
#define VP_ADMIN_COL_PRICE    7
#define VP_ADMIN_GOODSADDTRAY 8
#define VP_ADMIN_GETBILL        9
#define VP_ADMIN_GOODSBUHUO  10

#define VP_BUT_GAME           0
#define VP_BUT_NUMBER         0
#define VP_BUT_GOODS          1
#define VP_BUT_RETURN         4
#define VP_BUT_RETURNNO       0
#define VP_STA_CHANGE_ENOUGH  20

#define VP_ACT_HEART          0
#define VP_ACT_CHUHUO         1
#define VP_ACT_PAYOUT         2
#define VP_ACT_ADMIN          5
#define VP_ACT_ONLINE        6

#define VP_ACT_ENTERADMIN    1
#define VP_ACT_OUTADMIN      0



#define VP_INFO_TOTALVALUE    3
#define VP_INFO_CHUHUO        6
#define VP_INFO_HUODAO        10
#define VP_INFO_POSITION      11
#define VP_INFO_SALEPRICE     12
#define VP_INFO_VMC           13
#define VP_INFO_VER           14
#define VP_INFO_HARD          15
#define VP_INFO_BILL          16
#define VP_INFO_COIN          17
#define VP_INFO_COMP          18
#define VP_INFO_ERR           23


//VMC-->PC
#define VP_TEXT_MSG      0
#define VP_ACK_RPT       1
#define VP_NAK_RPT       2
#define VP_POLL          3
#define VP_STARTUP_RPT   4
#define VP_VMC_SETUP     5
#define VP_PAYIN_RPT     6
#define VP_PAYOUT_RPT    7
#define VP_VENDOUT_RPT   8
#define VP_REQUEST       9
#define VP_ADMIN_RPT     10
#define VP_ACTION_RPT    11   //V2
#define VP_BUTTON_RPT    12
#define VP_STATUS_RPT    13
#define VP_HUODAO_RPT    14
#define VP_CARD_RPT      15
#define VP_COST_RPT      16//扣款后，返回扣款执行结果;by gzz 20110823
//#define VP_DEBT_RPT      17//返回欠条结果;by gzz 20110825
#define VP_INFO_RPT      17//120419 by cq TotalSell 
#define VP_VENDOUT_REQ   18
#define VP_OFFLINEDATA_RPT   19

#define VP_STARTUP_RPT_1   0x22 //120419 by cq TotalSell


#define VP_MT_MIN_SEND  VP_TEXT_MSG
//#define VP_MT_MAX_SEND  VP_DEBT_RPT
#define VP_MT_MAX_SEND  VP_OFFLINEDATA_RPT//120419 by cq TotalSell 
//#define VP_MT_MAX_SEND  VP_STARTUP_RPT_1//120419 by cq TotalSell 


//PC-->VMC
#define VP_ACK             128
#define VP_NAK             129
#define VP_INITIAL_OK      130
#define VP_VENDOUT_IND     131
#define VP_RESET_IND       132
#define VP_CONTROL_IND     133
#define VP_GET_STATUS      134
#define VP_HOUDAO_IND      135
#define VP_POSITION_IND    136
#define VP_PAYOUT_IND      137    //V2
#define VP_GET_HUODAO	   138
#define VP_COST_IND        139//执行扣款命令;by gzz 20110823
#define VP_GETINFO_IND     140//120419 by cq TotalSell 
#define VP_SALETIME_IND    141
#define VP_SALEPRICE_IND   142
#define VP_HUODAO_SET_IND  143
#define VP_GET_SETUP_IND   144
#define VP_GET_OFFLINEDATA_IND   145



#define VP_MT_MIN_RECEIVE  VP_ACK
#define VP_MT_MAX_RECEIVE  VP_GET_OFFLINEDATA_IND


extern void VPSerialInit( void );
extern unsigned char VPBusTxMsg( void );
extern unsigned char VPBusFrameUnPack( void );
extern unsigned char VPMsgPackSend( unsigned char msgType, unsigned char flag, unsigned char snCtr );

extern unsigned char VP_CMD_ColumnPar( void );
extern unsigned char VP_CMD_GoodsPar( void );
extern unsigned char VP_CMD_Init_OK( void );
extern unsigned char VP_CMD_Vendout( void );
extern unsigned char VP_CMD_Reset( void );
extern unsigned char VP_CMD_Control( void );
extern unsigned char VP_CMD_GetStatus( void );
extern unsigned char VP_CMD_Payout( void );
extern unsigned char VP_CMD_GetColumnSta( void );

extern unsigned char changeCNY2(unsigned char* results, u_int money);
//extern unsigned char IsCanChange(u_int    Price);
extern unsigned char GoodStateLed(u_char i);
extern unsigned char UpdateSelLed_Trade( void );
extern unsigned char UpdateSelectionLed_GoodsSta( void );
extern unsigned char UpdateGoodsMatrixStatus( unsigned char goodsNum );
extern unsigned char VP_Update_ColumnGoodsPar( void );

extern unsigned char VPMission_Startup_RPT( void );
extern unsigned char VPMission_Setup_RPT( void );
extern unsigned char VPMission_Payin_RPT( unsigned char dev, unsigned int money );
extern unsigned char VPMission_Payout_RPT( unsigned char dev, unsigned int money1, unsigned int money2 );
extern unsigned char VPMission_Payout_RPTNOACK( unsigned char dev, unsigned int money1, unsigned int money2 );
extern unsigned char VPMission_Cost_RPT( unsigned char dev, unsigned int money,unsigned char type  );//发送pc扣款后金额;by gzz 20110823
extern unsigned char VPMission_Debt_RPT( unsigned char type, unsigned char dev, unsigned int money );//当上报欠款时，上报pc端;by gzz 20110825
extern unsigned char VPMission_Vendout_RPT( unsigned char status, unsigned char column, unsigned char type, unsigned int cost, unsigned char columnLeft );
extern unsigned char VPMission_Request( unsigned char type );
//extern unsigned char VPMission_Admin_RPT( unsigned char type, unsigned char column, unsigned char num );
extern unsigned char VPMission_Admin_RPT( unsigned char type, unsigned char column, unsigned int num );
extern unsigned char VPMission_Button_RPT( unsigned char type, unsigned char value );
extern unsigned char VPMission_Card_RPT( void );
extern unsigned char VPMission_Status_RPT( void );
extern unsigned char VPMission_ColumnSta_RPT( void );

extern unsigned char VPMission_Poll( void );
extern unsigned char VPMission_Init( void );
extern unsigned char VPMission_Act_RPT( unsigned char type, unsigned char value );


extern unsigned char VPAddSingleColGoods( unsigned char col, unsigned char num, unsigned char oldNum, unsigned char Flag );
extern unsigned char VPChangeColPrice( unsigned char col, unsigned int price, unsigned int oldPrice );
extern unsigned char VPAddAllColGoods( void );
extern unsigned char VPAddLayerColGoods( void );//by gzz 20110429
extern unsigned char VPAddChanges( uchar xdata testDev );//by gzz 20110506
extern unsigned char VPBuhuoCol( void );
extern unsigned char VPSynGoodsCol( void );
extern unsigned char VPCountCoin( unsigned char hopperNum );
extern void SellAccumulateNum(unsigned long * ul_num);
extern void SellAccumulateMoney(unsigned long * ul_money, unsigned int ui_cost);

#endif  //__VMC_PC_H__
