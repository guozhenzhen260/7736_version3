// 合创公司内部协议规定
#ifndef _PROCOTOL_
#define _PROCOTOL_
#include "device.h"
#ifndef PROCOTOL_C
	#define ProcotolSourse extern
#else
	#define ProcotolSourse
#endif

// 帧结构中常量定义
	#define CMDHEAD 		0xED
	#define ACKHEAD 		0xFD
	#define FRAMETAIL 		0xFF

// 设备ID定义: 
	#define ID_HOPPER1 		0x00
	#define ID_HOPPER2 		0x01
	#define ID_HOPPER3 		0x02
	#define ID_HOPPER4 		0x03
	#define ID_HOPPER5 		0x04
	#define ID_HOPPER6 		0x05
	#define ID_HOPPER7 		0x06
	#define ID_HOPPER8 		0x07

	#define ID_HUB 			0x08
	#define ID_LCD  		0x09
	#define ID_POWER 		0x0A
	#define ID_MODE 		0x0B
	#define ID_PRN  		0x0C
	#define ID_CASHER 		0x0D
	#define ID_COINER1 		0x10
	#define ID_COINER2 		0x11
	#define ID_COINER3 		0x12
	#define ID_COINER4 		0x13
  //#define ID_LEDSCR  		0x14
	#define ID_CHANNEL  	0x15
    //--------------------------
    #define ID_SELECTION1   0x16
    #define ID_SELECTION2   0x17
    #define ID_SELECTION3   0x18
    #define ID_ACDCMODULE   0x1E
    //========================== 
	#define ID_MESSAGER 	0x7E
	#define ID_SERVER 		0x7F
	#define ID_NULL   		0xFF

	
// 协议相关设备编号定义
	#define EQUIP_NULL 		 0x00
	#define EQUIP_BUS 		 0x01
	#define EQUIP_CASHER 	 0x02
	#define EQUIP_COINER 	 0x03
	#define EQUIP_POWER 	 0x04
	#define EQUIP_MODE 		 0x05
	#define EQUIP_PRN 		 0x06
	#define EQUIP_LCD 		 0x07
	#define EQUIP_HOPPER 	 0x08
	#define EQUIP_HUB 		 0x09
	#define EQUIP_SEVER 	 0x0A
	#define EQUIP_MESSAGER   0x0B
//	#define EQUIP_LEDSCR  	 0x0C
	#define EQUIP_CHANNEL 	 0x0D
    #define EQUIP_SELECTION  0x0E
    #define EQUIP_ACDCMODULE 0x0F


// 出币机构指令定义：
	#define HOPPER_OUTPUT 		0x50	
	#define HOPPER_QUERY 		0x51
	
// 辅助单元指令定义
	#define HUB_LAMP 		0x50
	#define HUB_QUERY 		0x51
	#define HUB_DENOTE 		0x52
	#define HUB_LAMPMODE 		0x53
	#define HUB_BUZZER  		0x54
	#define HUB_COINPOWER           0x56
	
// 人机交互单元指令定义: 
	#define DISPKEY_DISP		0x50
	#define DISPKEY_QUERY 		0x51
	#define DISPKEY_DISPS 		0x52
	#define DISPKEY_SETCURSOR  	0x54
	#define DISPKEY_GETCURSOR 	0x55
	#define DISPKEY_KEYLOCK 	0x56
	#define DISPKEY_CURSORSTYLE 	0x57
	#define DISPKEY_DENOTE 		0x58
	#define DISPKEY_GETKEY 		0x59
	#define DISPKEY_GETKEYECHO 	0x5B
	#define DISPKEY_SCROLL 		0x5C
	#define DISPKEY_CLEAR   	0x5D
	#define DISPKEY_LANGUAGE 	0x5E

// 电源管理单元指令定义
	#define POWER_QUERY 		0x51
// 模式开关管理单元指令定义	
	#define MODE_QUERY 		0x51
// 打印机指令定义
	#define PRN_STR			0x50
	#define PRN_SKIP 		0x52
	#define PRN_QUERY 		0x51
	#define PRN_PICTURE 		0x54
	#define PRN_DATA      		0x56
	#define PRN_FONT 		0x5A
	#define PRN_FORWARD 		0x5B
	#define PRN_BACK 		0x5C
	#define PRN_CUT 		0x5D
	#define PRN_BARCODE 		0x5E
// 纸币器指令定义
	#define CASHER_PERMISSION 	0x50
	#define CASHER_QUERY  	 	0x51
	#define CASHER_ACCEPT 	 	0x52
	#define CASHER_REJECT 	 	0x53	
	#define CASHER_ENABLE  	 	0x54
	#define CASHER_DISABLE 	 	0x55
	#define CASHER_RESET 	 	0x56
// 硬币器指令定义
	#define COINER1_RESET 	 	0x50
	#define COINER1_ENABLE  	 	0x51
	#define COINER1_QUERY 		0x52
	#define COINER1_DISABLE 	 	0x53	
// 广告屏指令定义
//	#define LEDSCRSET  		0x50
// 货道驱动模块指令定义
	#define CHANNEL_EXEC   		0x50
	#define CHANNEL_QUERY   	0x51
	#define CHANNEL_TEST   		0x52
	#define CHANNEL_CLEAR	   	0x53
	
	ProcotolSourse void ZhkBusSerialInit( void );
	ProcotolSourse uchar ZhkBusTxMsg( struct COMMTASK xdata* Task );
	ProcotolSourse uchar ZhkBusFrameUnPack( struct COMMTASK xdata* Result );
//	ProcotolSourse uchar ZhkPrnFrameUnPack( struct COMMTASK xdata* Result );
	ProcotolSourse uchar ZhkCasherFrameUnPack( struct COMMTASK xdata* Result );
	ProcotolSourse uchar ZhkChannelFrameUnPack( struct COMMTASK xdata* Result  );
	ProcotolSourse unsigned char ZyzBusTxMsg( struct COMMTASK xdata* Task );
//	ProcotolSourse unsigned char CoinerFrameUnPack( struct COMMTASK xdata* Result );
	//控制宝连硬币器的电源
	ProcotolSourse unsigned char CtrlCoinPowerFrameUnPack( struct COMMTASK xdata* Result  );

	ProcotolSourse uchar ZhkChkAddr( uchar Addr );
	
#endif