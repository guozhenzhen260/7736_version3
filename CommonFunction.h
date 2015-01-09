#ifndef _FUNCTION_
#define _FUNCTION_
#ifndef COMMONFUNCTION_C
	#define FunctionSourse extern
#else
	#define FunctionSourse
#endif	

//#include "timer.h"
#include "Clock.h"
#include "scheduler.h"
#include "global.h"

//FunctionSourse struct WayList xdata InputGoodsWayList[ GOODSWAYNUM ];

//FunctionSourse unsigned char xdata InputGoodsNum[ GOODSWAYNUM ];
/*把时间片让给硬件和通讯模块*/
FunctionSourse void CoreProcessCycle();

/*让出时间片实现等待,msec参数为指定毫秒数,DeviceID为设备的ID编号指针
	返回值，1为设备忙，0为设备不忙*/
FunctionSourse unsigned char WaitForWork( unsigned long msec, struct DEVICE xdata* DeviceID  );


FunctionSourse u_int CrcCheck(u_char *msg, u_int len); 
//清除键值	
FunctionSourse void ClearKey();

/*输入一行字符串，strTip为提示符，len为strTip的长度，line为返回的字符串,len为line的最大长度
1:成功，0：失败*/
FunctionSourse unsigned char GetLine( unsigned char * strTip,unsigned char len, unsigned char y, unsigned char * line, unsigned char *len1);
FunctionSourse unsigned char GetLine_1( u_char  *strTip, unsigned char len, u_char y, u_char * line, unsigned char *len1, bit Passwdlog);

u_char GetLineMoney( u_char  *strTip, unsigned char len, u_char y, u_char * line, unsigned char *len1 );

/*显示字符,flag标志指定是以光标当前位置还是以指定位置显示字符，
	当flag为0则以当前位置*/	
FunctionSourse void  DisplayStr( unsigned char x, unsigned char y, bit flag, unsigned char *str, const unsigned char len );

/*显示光标位置和类型,posflag确定是否设置位置,当它是0时x、y无效；stateflag意义相同*/	
FunctionSourse void  DisplayCursorState( unsigned char x, unsigned char y, bit posflag, unsigned char state, bit stateflag );

/*获取单键输入*/
FunctionSourse unsigned char GetOneKey();
FunctionSourse unsigned char GetOneIDKey();
FunctionSourse unsigned char GetOneMoneyKey();

FunctionSourse unsigned char LenOfNum( unsigned long num);

/*出币器出币，iCoinNum出币数量,iHopperNo出币器编号*/
//FunctionSourse unsigned char HopperOutCoin( unsigned char iCoinNum, unsigned char iHopperNo, struct DEVICE xdata* iDevice );
  FunctionSourse unsigned char HopperOutCoin( unsigned int iCoinNum, unsigned char iHopperNo, struct DEVICE xdata* iDevice );

//FunctionSourse unsigned char ChannelQuery(unsigned char ChannelNum );
//货道出货
//FunctionSourse unsigned char ChannelExec( unsigned char ChannelNum );
FunctionSourse unsigned char ChannelTask( u_char ChannelNum, u_char TaskId );
FunctionSourse u_char ChannelLifterTask( u_char ChannelNum, u_char TaskId );

//输入货道编号
FunctionSourse unsigned char InputGoodsWayNo( u_char i );

//纸币器处理
//FunctionSourse unsigned char HykCasherTask( unsigned char TaskId );

FunctionSourse unsigned char BaoLianCoinerTask( u_char TaskId );

/*得到设备状态,iHopperNo传入编号,指定设备,iDevice设备结构指针*/
FunctionSourse unsigned char GetHardWareDeviceState( unsigned char iHopperNo, struct DEVICE xdata* iDevice );
FunctionSourse unsigned char GetSelectionState( unsigned char id, struct DEVICE xdata* iDevice );
FunctionSourse unsigned char ACDCMission( void );

FunctionSourse unsigned char DisplayInfo( unsigned char line , unsigned char offset , unsigned char * buffer );

FunctionSourse void ClearDisplayLine( unsigned char Line );

//FunctionSourse void  PhoneTBCD( u_char *OutBuffer );

/*从Flash中装载数据*/
FunctionSourse void  LoadGlobalParam();

/*把数据写到Flash中*/
FunctionSourse void  SaveGlobalParam();

/*保存货道参数*/
FunctionSourse void SaveGoodsSet();

FunctionSourse void  SaveTradeParam();

//写交易LOG
FunctionSourse void SaveTradeCounter();

/*把Flash全部重新刷一次*/
FunctionSourse void  FlashInit();

//货架配置
FunctionSourse void SaveRackSet();

//货道配置
FunctionSourse void SaveRackColumnSet();

//
FunctionSourse void SaveGoodsParSet( void );
FunctionSourse void ReadGoodsParSet( void );

#endif
