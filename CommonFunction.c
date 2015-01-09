//#undef _DEBUG_TRACE
#define COMMONFUNCTION_C
#include "debug.h"
#include "device.h"
#include "global.h"
#include "common.h"
#include "CommonFunction.h"
#include "key.h"
#include "STRING.H"
#include "DataExChange.h"
#include "mainflow.h"
#include "timer.h"
#include "SstFlash.h"
#include "procotol.h"
#include "casher.h"
#include "scheduler.h"
#include "IOInput.h"
#include "communication.h"
#include "ITL.h"
#include "VMC_PC.h"
#include "lifterApi.h"


/*把时间片让给硬件和通讯模块*/

struct WayList xdata InputGoodsWayList[ GOODSWAYNUM ] ;

void CoreProcessCycle()
{
	SchedulerProcess();
#ifndef _DEBUG_NO_MOBILE
	if( SystemParameter.MobileON == 1 )
		DoCommunication();
#endif
}

//Flash初始化
void FlashInit()
{
	//Flash片选
	//选中第一片
	ZhkHardFlashSelect();
	//整片擦除
	SstFlashChipErase();
	ZhkHardSramSelect();
	//选中第二片
	ZhkHardFlash1Select();
	//整片擦除
	SstFlashChipErase();
	ZhkHardSramSelect();
}

/*让出时间片实现等待,msec参数为指定等待时间, DeviceID为设备ID编号指针*/
u_char WaitForWork( unsigned long msec, struct DEVICE xdata* DeviceID )
{
	unsigned long loopSec = msec / 1000;
	unsigned short loopMSec = msec % 1000;
		
	TestDeviceCommOK( DeviceID );
	// 毫秒级定时
	LzjMSecTimer = loopMSec & 0x7F;
	loopMSec /= 128;
	do
	{
		while(LzjMSecTimer)
		{			
			if( DeviceID != NULL )
			{
				SchedulerProcess();
				if( TestDeviceCommOK(  DeviceID ) == 0 )
					return 0;
			}
			else
			{		
				if( SystemParameter.MobileON == 1 )
				{
					Mission();
					CoreProcessCycle();			
				}
				else
					SchedulerProcess();	
			}
		}
		LzjMSecTimer = 128;
	}while(loopMSec--);

	// 秒级定时
	SgpSecTimer = loopSec & 0x7F;
	loopSec /= 128;
	do
	{
		while(SgpSecTimer)
		{
			if( DeviceID != NULL )
			{
				SchedulerProcess();
				if( TestDeviceCommOK(  DeviceID ) == 0 )
					return 0;
			}
			else
			{
				if( SystemParameter.MobileON == 1 )
				{
					Mission();
					CoreProcessCycle();			
				}
				else
					SchedulerProcess();	
			}
		}
		SgpSecTimer = 128;
	}while(loopSec--);

	if( DeviceID != NULL )	
		return 1;
	else
		return 0;
			
}

/*********************************************************************************************************
** Function name:     	CrcCheck
** Descriptions:	    CRC校验和
** input parameters:    msg需要检验的数据;len数据长度
** output parameters:   无
** Returned value:      CRC检验结果
*********************************************************************************************************/
u_int CrcCheck(u_char *msg, u_int len) 
{
    u_int xdata i, j;
    u_int xdata crc = 0;
    u_int xdata current = 0;
    for(i=0;i<len;i++) 
    {
        current = msg[i] << 8;
        for(j=0;j<8;j++) 
        {
            if((int)(crc^current)<0)
                crc = (crc<<1)^0x1021;
            else 
                crc <<= 1; 
            current <<= 1; 
        }
    }
    return crc;
}


//清除键值	
void ClearKey()
{
    u_char xdata ch = 0;

    if( sysVPMission.VPMode == 1 )
    {
	    //1.
        if( DeviceStatus.Selection[0] == 0 )
		{
			ch = GetSelectionState( 1, &Selection1 );
		    if( ch == 0 )
			{
			    DeviceStatus.Selection[0] &= 0xfe;
			}
			else
			{
				DeviceStatus.Selection[0] |= 0x01;
			}
		}
		//2.
        if( DeviceStatus.Selection[1] == 0 )
		{
			ch = GetSelectionState( 2, &Selection2 );
		    if( ch == 0 )
			{
			    DeviceStatus.Selection[1] &= 0xfe;
			}
			else
			{
				DeviceStatus.Selection[1] |= 0x01;
			}
		}
		//3.
		if( DeviceStatus.Selection[2] == 0 )
		{
			ch = GetSelectionState( 3, &Selection3 );
		    if( ch == 0 )
			{
			    DeviceStatus.Selection[2] &= 0xfe;
			}
			else
			{
				DeviceStatus.Selection[2] |= 0x01;
			}
		}
    
    }
    else
    {
		KeyLock();
		KeyUnLock();
    }
}

/*显示字符,flag标志指定是以光标当前位置还是以指定位置显示字符，
	当flag为0则以当前位置*/	
void  DisplayStr( u_char x, u_char y, bit flag, u_char *str, const u_char len )
{	
	u_char xdata timeflag = 1;
	u_char xdata clearstr[] = "                    ";
	u_char xdata tempstr[20];
	u_char xdata i = 0;
	u_char xdata j = 0;

	memset( tempstr, 0x20, sizeof( tempstr ) );
	if( flag == 0 )//在当前位置显示				
		i = DisplayString( str, len );		
	else
	{				
		if( ( x + len ) < 20 )			
			memcpy( tempstr + x, str, len );		
		else
			memcpy( tempstr + x, str, 20 );
        //len = 20;		
		i = DisplayPosString( 0, y, tempstr, 20 );//设置屏幕字串
	}	
	if( i == 0 )
		return ;
	i = 0;
	j = 0;
	while( ! ( i || j ) )
	{
		WaitForWork( 50, NULL );
		i = TestDeviceTimeOut( &Lcd );
		j = TestDeviceCommOK( &Lcd );		
	}
    //WaitForWork( 3000, &Lcd );
}

//输入密码
u_char GetOneIDKey()
{
	u_char xdata k = KEY_NULL ;
	u_char xdata m_str[2];
//	u_char xdata line[2];	
	unsigned int xdata interval = 500;   //超时时间5秒

	memset( m_str, 0, 2 );
	m_str[0] = '*';
	while( ( k == KEY_NULL ) && ( interval != 0 ) )
	{
		SchedulerProcess();
		WaitForWork( 50, NULL );
		k = GetKey();			
		interval --;
	}
	switch( k )
	{
		case KEY_0:
		case KEY_1:
		case KEY_2:
		case KEY_3:
		case KEY_4:
		case KEY_5:
		case KEY_6:
		case KEY_7:
		case KEY_8:
		case KEY_9:			
	//	case KEY_DOT:		
		//	NumberToString( k ,  m_str , 1  );				
		//	sprintf( m_str,"%bu", k );
			DisplayStr( 0, 0, 0, m_str, 1 );
		    break;			
	}	
	if(interval == 0)
		return KEY_NULL;	
	return k;	
}


/*输入一行字符串，strTip为提示符，len为strTip的长度，y 为显示行号，line为返回的字符串,len1为最大输入长度及实际输出长度
1:成功，0：失败*/	  
u_char GetLine( u_char  *strTip, unsigned char len, u_char y, u_char * line, unsigned char *len1)
{
    u_char 	xdata key;
	u_char  xdata iCounter = 0;
	u_char  xdata len_loca;
	u_char xdata space_char = 0x20;
	u_char xdata Tempstr[20];

	memset( Tempstr, 0, sizeof( Tempstr ) );
	iCounter = 0;
	ClearKey();
	len_loca = *len1;		
	if( len + len_loca > 20 )
	{	
		DisplayStr( 0, 0, 1, strTip, len );		
		DisplayStr( 0, 1, 1, " ", 1 );	
		DisplayCursorState( 0, 1, 1, 1, 1 );	
	}
	else
	{
		DisplayStr( 0, y, 1, strTip, len );
		DisplayCursorState( len, y, 1, 1, 1 );		
	}
	SgwSecTimer= 30;
	while( SgwSecTimer )
	{
		WaitForWork( 20, NULL );			
		if(iCounter >= len_loca)	
		{
			*len1 = len_loca;
		//	WaitForWork( 800, NULL );		
			memcpy( line, Tempstr, len_loca );
			return 1;
		}
		key = GetOneKey();	
		if ( (key == KEY_UP) || (key == KEY_DOWN) || (key == KEY_DOT) || (key == KEY_NULL) )
		    continue;
		else if( key == KEY_SUBMIT) 
		{				
			*len1 = iCounter;			
			memcpy( line, Tempstr, iCounter );
			return 1;
		}			
		else if(key == KEY_CANCEL)
			return 0;
		else if( key == KEY_BACK)//clear键,光标后退一格
		{		
			if( iCounter >= 1 )
			{
				if( len + len_loca >= 20 )
				{										
					DisplayPosString( iCounter - 1, 1, &space_char, 1); /* 屏幕清掉一个数字字符 */	
					WaitForWork( 300, &Lcd );
					DisplaySetCursorPos( iCounter - 1 , 1 );//设置光标位置				
					WaitForWork( 300, &Lcd );
				}
				else
				{										
					DisplayPosString( iCounter + len - 1, y, &space_char, 1); /* 屏幕清掉一个数字字符 */	
					WaitForWork( 300, &Lcd );
					DisplaySetCursorPos( iCounter + len - 1 , y );//设置光标位置				
					WaitForWork( 300, &Lcd );
				}						
				*(line + iCounter) = 0;
				iCounter--;		
				SgwSecTimer = 30;
			}
			
		}
		else
		{						
			Tempstr[ iCounter ] = key;			
			iCounter++;
			SgwSecTimer = 30;			
		}			
	}
}

/*输入一行字符串，strTip为提示符，len为strTip的长度，line为返回的字符串,
len为line的最大长度,Passwdlog为输入密码(1)或输入其它(0) 1:成功，0：失败*/	  
u_char GetLine_1( u_char  *strTip, unsigned char len, u_char y, u_char * line, unsigned char *len1, bit Passwdlog)
{
    u_char 	xdata key;
	u_char  xdata iCounter = 0;
	u_char  xdata len_loca;
	u_char xdata space_char = 0x20;
	u_char xdata Tempstr[20];

	memset( Tempstr, 0, sizeof( Tempstr ) );
	iCounter = 0;
	ClearKey();
	len_loca = *len1;		
	if( len + len_loca >= 20 )
	{	
		DisplayStr( 0, 0, 1, strTip, len );		
		DisplayStr( 0, 1, 1, " ", 1 );	
		DisplayCursorState( 0, 1, 1, 1, 1 );	
	}
	else
	{
		DisplayStr( 0, y, 1, strTip, len );
		DisplayCursorState( len, y, 1, 1, 1 );		
	}

    //SgwSecTimer = 30;
	while( 1 )
	{
		WaitForWork( 50, NULL );			
		key = KEY_NULL;
		ClearKey();
		if(iCounter >= len_loca)	
		{
			*len1 = len_loca;		
			memcpy( line, Tempstr, len_loca );
			return 1;
		}
		if( Passwdlog == 1 )
			key = GetOneIDKey();	
		else
			key = GetOneKey();			
		if ( (key == KEY_UP) || (key == KEY_DOWN) /*|| (key == KEY_BACK)*/ || (key == KEY_NULL) )
		    continue;
		else if( key == KEY_SUBMIT) 
		{				
			*len1 = iCounter;			
			memcpy( line, Tempstr, iCounter );
			return 1;
		}			
		else if(key == KEY_CANCEL)
			return 0;
		else if( key == KEY_BACK)//clear键,光标后退一格
		{		
			if( iCounter >= 1 )
			{
				if( len + len_loca >= 20 )
				{										
					DisplayPosString( iCounter - 1, 1, &space_char, 1); /* 屏幕清掉一个数字字符 */	
					WaitForWork( 300, &Lcd );
					DisplaySetCursorPos( iCounter - 1 , 1 );//设置光标位置				
					WaitForWork( 300, &Lcd );
				}
				else
				{										
					DisplayPosString( iCounter + len - 1, y, &space_char, 1); /* 屏幕清掉一个数字字符 */	
					WaitForWork( 300, &Lcd );
					DisplaySetCursorPos( iCounter + len - 1 , y );//设置光标位置				
					WaitForWork( 300, &Lcd );
				}						
				*(line + iCounter) = 0;
				iCounter--;				
			}			
		}
		else
		{						
			Tempstr[ iCounter ] = key;			
			iCounter++;					
		}			
	}
	return 0;
}


/*获取单键金额输入*/
u_char GetOneMoneyKey()
{
	u_char xdata k = KEY_NULL ;
	u_char xdata m_str[2];
//	u_char xdata line[2];	
	unsigned int xdata interval = 500;   //超时时间5秒
	
	memset( m_str, 0 , sizeof (m_str ) );
	while( ( k == KEY_NULL ) && ( interval != 0 ) )
	{
		SchedulerProcess();
		WaitForWork( 50, NULL );
		k = GetKey();			
		interval --;
	}
	switch( k )
	{
		case KEY_0:
		case KEY_1:
		case KEY_2:
		case KEY_3:
		case KEY_4:
		case KEY_5:
		case KEY_6:
		case KEY_7:
		case KEY_8:
		case KEY_9:			
			NumberToString( k ,  m_str , 1  );				
		//	sprintf( m_str,"%bu", k );
			DisplayStr( 0, 0, 0, m_str, 1 );
		    break;	
	}	
	if(interval == 0)
		return KEY_NULL;	
	return k;	
}

/*输入一行字符串，strTip为提示符，len为strTip的长度，line为返回的字符串,
len为line的最大长度,Passwdlog为输入密码(1)或输入其它(0) 1:成功，0：失败*/	  
u_char GetLineMoney( u_char  *strTip, unsigned char len, u_char y, u_char * line, unsigned char *len1 )
{
    u_char xdata key;
	u_char xdata iCounter = 0;
	u_char xdata len_loca;
	u_char xdata space_char = 0x20;
	u_char xdata Tempstr[20];

	memset( Tempstr, 0, sizeof( Tempstr ) );
	iCounter = 0;
	ClearKey();
	len_loca = *len1;		
	if( len + len_loca >= 20 )
	{	
		DisplayStr( 0, 0, 1, strTip, len );		
		DisplayStr( 0, 1, 1, " ", 1 );	
		DisplayCursorState( 0, 1, 1, 1, 1 );	
	}
	else
	{
		DisplayStr( 0, y, 1, strTip, len );
		DisplayCursorState( len, y, 1, 1, 1 );		
	}
	while( 1 )
	{
		WaitForWork( 50, NULL );			
		key = KEY_NULL;
		ClearKey();
		if(iCounter >= len_loca)	
		{
			*len1 = len_loca;		
			memcpy( line, Tempstr, len_loca );
			return 1;
		}
		key = GetOneMoneyKey();			
		if ( (key == KEY_UP) || (key == KEY_DOWN) || (key == KEY_DOT) || (key == KEY_NULL) )
		    continue;
		else if( key == KEY_SUBMIT) 
		{				
			*len1 = iCounter;			
			memcpy( line, Tempstr, iCounter );
			return 1;
		}			
		else if(key == KEY_CANCEL)
			return 0;
		else if( key == KEY_BACK)//clear键,光标后退一格
		{		
			if( iCounter >= 1 )
			{
				if( len + len_loca >= 20 )
				{										
					DisplayPosString( iCounter - 1, 1, &space_char, 1); /* 屏幕清掉一个数字字符 */	
					WaitForWork( 300, &Lcd );
					DisplaySetCursorPos( iCounter - 1 , 1 );//设置光标位置				
					WaitForWork( 300, &Lcd );
				}
				else
				{										
					DisplayPosString( iCounter + len - 1, y, &space_char, 1); /* 屏幕清掉一个数字字符 */	
					WaitForWork( 300, &Lcd );
					DisplaySetCursorPos( iCounter + len - 1 , y );//设置光标位置				
					WaitForWork( 300, &Lcd );
				}						
				*(line + iCounter) = 0;
				iCounter--;						
			}			
		}
		else
		{						
			Tempstr[ iCounter ] = key;			
			iCounter++;			
		}			
	}
	return 0;
}

void ClearDisplayLine( u_char Line )
{
	if( ( Line > 0 ) && ( Line < 3 ) )
	{
		DisplayStr( 0, Line - 1 , 1 , "                    " , 20 );
		WaitForWork( 20, NULL );	
	}
}


/*显示光标位置和类型,posflag确定是否设置位置,当它是0时x、y无效；stateflag意义相同*/	
void  DisplayCursorState( u_char x, u_char y, bit posflag, u_char state, bit stateflag   )
{		
	u_char xdata i = 0;
	u_char xdata j = 0;

	if( posflag == 1 )	
		i = DisplaySetCursorPos( x, y );	
	if( stateflag == 1 )			
		i = DisplaySetCursorStyle( state );

	if( i == 0 )
		return ;
	i = 0;
	j = 0;
	while( ! ( i || j ) )
	{
		WaitForWork( 50, NULL );
		i = TestDeviceTimeOut( &Lcd );
		j = TestDeviceCommOK( &Lcd );		
	}
//	WaitForWork( 3000, &Lcd );
}

/*获取单键输入*/
u_char GetOneKey()
{
	u_char xdata k = KEY_NULL ;
	u_char xdata m_str[2];
//	u_char xdata line[2];	
	unsigned int xdata interval = 500;   //超时时间5秒
	
	memset( m_str, 0 , sizeof (m_str ) );
	while( ( k == KEY_NULL ) && ( interval != 0 ) )
	{
		CoreProcessCycle();
		WaitForWork( 50, NULL );
		k = GetKey();			
		interval --;
	}
	switch( k )
	{
		case KEY_0:
		case KEY_1:
		case KEY_2:
		case KEY_3:
		case KEY_4:
		case KEY_5:
		case KEY_6:
		case KEY_7:
		case KEY_8:
		case KEY_9:			
			NumberToString( k ,  m_str , 1  );				
		//	sprintf( m_str,"%bu", k );
			DisplayStr( 0, 0, 0, m_str, 1 );
		    break;
	//	case KEY_DOT:		
	//		m_str[0] = 0x2e;
    //      DisplayStr( 0, 0, 0, m_str, 1 );  
	//		break;
	}	
	if(interval == 0)
		return KEY_NULL;	
	return k;	
}

/*求数字的长度*/
/*求数字的长度*/
u_char LenOfNum( unsigned long num )
{
	if(num >= 1000000000 )
		return 10;
	if(num >= 100000000 )
		return 9;
	if(num >= 10000000 )
		return 8;
	if(num >= 1000000 )
		return 7;
	if(num >= 100000 )
		return 6;
	if(num >= 10000 )
		return 5;
	else if(num >= 1000 )
		return 4;
	else if(num >= 100 )
		return 3;
	else if(num >= 10 ) 
		return 2;
	else
		return 1;
} 

//出币模块收到后会返回ACK应答包
u_char HopperOutCoin( u_int iCoinNum, u_char iHopperNo, struct DEVICE xdata* iDevice )
{	
	u_char xdata i = 0;
	u_char xdata j = 0;

	HopperOutput( iHopperNo , iCoinNum );
	WaitForWork( 100, NULL );
	i = 0;
	j = 0;
	while( ! ( i || j ) )
	{
		WaitForWork( 100, NULL );
		i = TestDeviceTimeOut( iDevice );
		j = TestDeviceCommOK( iDevice );		
	}		
	if ( i )	
	{
		return 1;//超时
	}
//	if ( TestDeviceException( iDevice ) )
//	{
//		return 2;//故障
//	}
	Trace("\n HopperOutCoin OK" );
	sysVPMission.hopperCoinNum = iCoinNum;
	return 0; // 正常返回
}
   

//输入货道,返回物理货道编号与逻辑货道数组的数组序列编号，输入失败返回100
u_char InputGoodsWayNo( u_char x )
{
	u_char xdata key = 0;
	u_char xdata m_InputCode;
	u_char xdata m_GoodsWayNo;
	u_char xdata Tempstr[8];
	u_char xdata len = 0;
	u_char xdata i = 0;
	
	ClearKey();
	while( 1 )
	{		
		ClearDisplayLine( 1 );
		ClearDisplayLine( 2 );		
		len = 2;
		memset( Tempstr,0, sizeof( Tempstr ) );	
	#ifdef _CHINA_
		key = GetLine( "请输入编号: ", 12 , 0, Tempstr, &len );
	#else	
		key = GetLine( "Input column code:", 18 , 0, Tempstr, &len );
	#endif
		WaitForWork( 500, NULL );	
		m_GoodsWayNo = 100;
		m_InputCode = 0;
		ClearKey();				
		if( key == 1 )//Enter	
		{			
			DisplayCursorState( 0, 0, 0, 0, 1 );			
			for( i = 0; i < len; i ++ )
			{
	//			Trace( "\n GoodsNo1 = %bu", Tempstr[i] );		
				if( ( Tempstr[i] >= 0 ) && ( Tempstr[i] <= 9 ) )
					m_InputCode = m_InputCode * 10 + Tempstr[i];				
			}
			if( ( m_InputCode == 99 ) && ( x == 0 ) )
				return 99;					
			ClearKey();	
			if( m_InputCode != 0 )
			{
				for( i = 0; i < GOODSWAYNUM; i ++ )
				{
					if( InputGoodsWayList[i].GoodsWayNo == m_InputCode )
					{
						m_GoodsWayNo = i;					
						return i;
					}
				}		
			}
			else
				i = GOODSWAYNUM;
			if( i == GOODSWAYNUM )
	 	    {
			#ifdef _CHINA_
				DisplayStr( 0, 0, 1, "编号输入错误", 12 );
				DisplayStr( 0, 1, 1, "请输入其它编号", 14 );
			#else
				DisplayStr( 0, 0, 1, "Input code error", 16 );
				DisplayStr( 0, 1, 1, "Input other code", 16 );
			#endif
				m_GoodsWayNo = 100;			
				WaitForWork( 2000, NULL );
				continue;
			}			
	//		Trace( "\n m_SetArrayNo = %bu", m_GoodsWayNo );
	//		Trace( "\n return 0" );
			return m_GoodsWayNo;
		}
		else
			return m_GoodsWayNo;
	}
}






/*返回值的意义
	0:正确执行的命令
	1:出货返回失败
	2:执行命令超时
	3:货道无货
	4:货道故障
	5:在限定的时间内电机不能到位
    6:
    7: CMD ERR
    8: GOC check error 
*/
u_char ChannelTask( u_char ChannelNum, u_char TaskId )
{	
	u_char xdata i = 0;
	u_char xdata j = 0;
	u_char xdata z = 0;

	#ifdef   _SJJ_//升降机
		return 0;
	#endif
	
	while( z < 2 )
	{
		switch( TaskId )
		{
			case CHANNEL_EXEC://出货			
				ZhkChannelExec( ChannelNum, z );			
				break;
			case CHANNEL_QUERY://查询命令执行情况
				ZhkChannelQuery();
				break;
			case CHANNEL_TEST://货道测试
				ZhkChannelTest( ChannelNum );			
				break;
			case CHANNEL_CLEAR://同步SN
				ZhkChannelClear();
				break;		
			default:
				return 0;
		}
		i = 0;
		j = 0;
		while( ! ( i || j ) )
		{
			WaitForWork( 50, NULL );
			i = TestDeviceTimeOut( &Channel );
			j = TestDeviceCommOK( &Channel );		
		}
	//	Trace("\n i = %bu, j = %bu", i, j );
		if( i ) //超时，货道故障	
		{
			if( ( TaskId == CHANNEL_EXEC ) || ( TaskId == CHANNEL_TEST ) )
			{
				if( Channel.ExtState[2] == 0 )	//没有收到命令ACK，重试一次
				{
					z++;
					continue;
				}
				else if( Channel.ExtState[2] == 1 )//收到命令ACK，但没有收到消息ACK，则查询命令执行情况
				{
					TaskId = CHANNEL_QUERY;
					z++;
			//		Trace("\n CHANNEL_QUERY" );
					continue;
				}
			}
			else
			{
		//		Trace("\n TimeOut" );
				return 2;
			}
		}
		else
			break;
	}
	if( i )
		return 2;
	/*状态字节中各位的意义
	  Bit 6: 接收到的命令包有错误
	  Bit 5: 货物没有通过出货检测口
	  Bit 4: 在限定的时间内电机不能到位
	  Bit 3: 电机没有转动
	  Bit 2: 在转动前电机没有到位
	  Bit 1: 商品卖空，保留
	  Bit 0: 命令失败 */
	switch( TaskId )
	{
	    
		case CHANNEL_EXEC:
		case CHANNEL_QUERY:	
		case CHANNEL_TEST:
			//----------------------------------------------------------------------
			//Added by Andy on 2010.10.21 for GOC device
             if( SystemParameter.GOCOn == 1 )
    		 {
					if( Channel.State & 0x80 )
					{
						DeviceStatus.GOCStatus |= 0x01;	
					}
                    else
                    {
                        DeviceStatus.GOCStatus &= 0xFE;
                    }
			 }
			//======================================================================
			if( Channel.State & 0x01 ) 
			{	
                /*
                2011.3.4 disabled!
				if( Channel.State & 0x04 )
					return 3;//在转动前电机没有到位，没有出货，此货道已不能工作了
                */
				if( Channel.State & 0x08 )
				{				    
					return 4;//电机没有转动，没有出货，可以重试一次
				}
				//有出现货道未转，或未转到位时，不扣钱;by cq 20110909
				//if( Channel.State & 0x10 )
				//	return 5;//在限定的时间内电机不能到位（但货已出成功，货道有硬件故障）
                if( Channel.State & 0x10 )					
				{

				    /*
	                if( SystemParameter.GOCOn == 1 )
	    			{
						if(!( Channel.State & 0x20 ))
						{						    
							return 5;
						}						
						return 4;
	                }
					*/					
					return 4;
				}

				if( Channel.State & 0x40 )
					return 7;//接收到的命令包有错误，但货没有出，可以重试一次
				//----------------------------------------------------------------------
				//Added by Andy on 2010.10.21 for GOC device
                if( SystemParameter.GOCOn == 1 )
    			{
					if( Channel.State & 0x20 )
					{
					    //if( ( DeviceStatus.GOCStatus == 0x00 ) )
                        /*
						if( SystemParameter.SVC_GOCErr == 0x00 )
						{
							return 8;
						}
						else
						{
						    return 0;	
						}
                        */						
                        return 8;
					}
				}
				//======================================================================
				return 1;//命令失败
			}
			break;		
		case CHANNEL_CLEAR:
            {
            	//----------------------------------------------------------------------
				//Added by Andy on 2010.10.21 for GOC device
	             if( SystemParameter.GOCOn == 1 )
	    		 {
						if( Channel.State & 0x80 )
						{
							DeviceStatus.GOCStatus |= 0x01;	
						}
	                    else
	                    {
	                        DeviceStatus.GOCStatus &= 0xFE;
	                    }
				 }
				//======================================================================
            }
			break;
	}		
	return 0;//出货成功
}

/*
//货道查询
u_char ChannelQuery( u_char ChannelNum )
{
	u_char xdata i = 0;
	u_char xdata j = 0;

//	if ( ChannelNum > GOODSWAYNUM || ChannelNum == 0 ) 	
//		return 4;//货道不正确
//	Trace( "\n ChannelQuery" );
	ZhkChannelQuery( ChannelNum );
	i = 0;
	j = 0;
	while( ! ( i || j ) )
	{
		WaitForWork( 100, NULL );
		i = TestDeviceTimeOut( &Channel );
		j = TestDeviceCommOK( &Channel );		
	}
	if( i ) 
		return 3;//超时，货道故障
	else if( Channel.State & 0x01 )
		return 2;//出货失败
	return 0;//货道正常
}

//货道出货
u_char ChannelExec( u_char ChannelNum )
{
	u_char xdata i = 0;
	u_char xdata j = 0;

//	if ( ChannelNum > 64 || ChannelNum == 0 ) 	
//		return 1;//货道不正确
	ZhkChannelExec( ChannelNum );
	i = 0;
	j = 0;
	while( ! ( i || j ) )
	{
		WaitForWork( 100, NULL );
		i = TestDeviceTimeOut( &Channel );
		j = TestDeviceCommOK( &Channel );		
	}	
	if ( i )	
		return 3;//超时,货道板和主机通讯有问题,可认为整个出货统有问题
	if ( Channel.State & 0x01 ) 	
		return 2;//出货失败
	return 0;//出货成功
	
}
*/

//id=4, ACDCModule
u_char GetSelectionState( u_char id, struct DEVICE xdata* iDevice )
{
	u_char xdata i = 0;
	u_char xdata j = 0;
	u_char xdata timeBuf = 0;
 
	if( ( id == 1 ) || ( id == 2 ) || ( id == 3 ) || ( id == 4 ) )
	{
		timeBuf = 3;  //10-3
    }
	for( YgpSecTimer=timeBuf; YgpSecTimer; )
	{  		
	    SelectionQuery( id );
		while( 1 )
		{
			i = TestDeviceCommOK( iDevice );
			j = TestDeviceTimeOut( iDevice );
			if( !( j || i ) )					
			{
				WaitForWork( 100, NULL );
				i = 0;
			}
			else
            {
				break;
            }	
		}
		if( i == 4 )
        {
			break;
		}
		WaitForWork( 100, NULL );
	}
	if( ( YgpSecTimer == 0 ) || ( j ) )
	{		
		Trace("\n Time Out" );
		return 1;      //超时
	}
    /*	
	if ( TestDeviceException( iDevice ) )
	{
		return 2;   //故障
	}
    */
	return 0;   //正常返回
	
}


u_char ACDCMission( void )
{
    u_char xdata flag = 0;

	if( SystemParameter.ACDCModule == 0 )
	{
		return 1;
	}

	if( DeviceStatus.ACDCModule == 0 )
	{
		flag = GetSelectionState( 4, &ACDCModule );
	    if( flag == 0 )
		{
		    DeviceStatus.ACDCModule &= 0xfe;
            //DeviceStatus.ACDCModule = 0;
			return 0;
		}
		else
		{
			DeviceStatus.ACDCModule |= 0x01;
			return 1;
		}
	}
	//return 0;
}

u_char GetHardWareDeviceState( u_char iHopperNo, struct DEVICE xdata* iDevice )
{
	u_char xdata i = 0;
	u_char xdata j = 0;
	u_int xdata timeBuf = 0;
    u_char xdata result = 0;

    //----------------------------------------------------------------------------------
    //Changed by Andy on 2010.11.2
    if( ( iHopperNo == 1 ) || ( iHopperNo == 2 ) || ( iHopperNo == 3 ) )
    {
    	//timeBuf = 5000;  //255-120-255(2011.6.16),5000(20110616)
		//时间由需要出币的数量来确定，20S是延时时间;by gzz 20110616
		timeBuf = sysVPMission.hopperCoinNum/4 + 20;		
    }
    else
    {
    	timeBuf = 10;
    }        
    //==================================================================================
    
	//for( YgpSecTimer = timeBuf; YgpSecTimer ; )
    //for( YgpSecTimer=timeBuf; YgpSecTimer>0; )//by gzz 20110616
	for( sysVPMission.hopperOutTimer=timeBuf; sysVPMission.hopperOutTimer>0; )
	{			
		if( ( iHopperNo == 1 ) || ( iHopperNo == 2 ) || ( iHopperNo == 3 ) )  //出币器状态
        {
			HopperQuery( iHopperNo );
        }
		else if( iHopperNo == 4 )   //纸币器状态
		{
            if( sysITLMission.ITLSet == 1 )
            {
                i = ITLMission_Poll();
            }
            else
            {
				i = CasherQuery( );	
			    WaitForWork( 100, NULL );
            }
		}
        sysVPMission.hopperTimer = timeBuf;
		while( sysVPMission.hopperTimer )
		{
            
            //--------------------------------------------------------------------------
            if( (iHopperNo>=1)&&(iHopperNo<=3) )
            {
            	WaitForWork( 100, NULL );
                //sysVPMission.msGameTimer1 = 200;
				//while( sysVPMission.msGameTimer1 )
				{
					//----------------------------------------------------
			        //Send the game key and poll pc in speciall time
		    		if( IfGameKeyOn() )
					{
						VPMission_Button_RPT( VP_BUT_GAME, VP_BUT_NUMBER );
		        		//Beep();
					}
			        result = VPMission_Poll();
			        //===============================
				} 
            }
            //============================================================================

			i = TestDeviceCommOK( iDevice );
			j = TestDeviceTimeOut( iDevice );
			if( !( j || i ) )					
			{
				WaitForWork( 100, NULL );
				i = 0;
			}
			else
				break;	
		}
		if( iHopperNo == 4 )
			break;
		if( i == 4 )
			break;		
		WaitForWork( 100, NULL );
	}

	//if( ( YgpSecTimer == 0 ) || ( j ) )//by gzz 20110616
	if( ( sysVPMission.hopperOutTimer == 0 ) || ( j ) )
	{		
		Trace("\n Time Out" );
		return 1;//超时
	}	
	if ( TestDeviceException( iDevice ) )
	{
        //Trace("\n Exception" );
		return 2;//故障
	}
	return 0; //正常返回	
}


/*
void   PhoneTBCD( u_char *OutBuffer )
{
	u_char xdata i = 0;
	for( i = 0; i < 16; i +=2 )		
	{
		if( SystemParameter.GSMNO[i] != 0xff )
		{
			if( SystemParameter.GSMNO[i + 1] != 0xff )
				OutBuffer[i/2]  = SystemParameter.GSMNO[i] * 16 + SystemParameter.GSMNO[i+1];
			else
			{
				if( ( i % 2 ) == 0 )
				{
					OutBuffer[i/2] = SystemParameter.GSMNO[i] * 16 + 15;
					OutBuffer[i/2+1 ] = 0xff;
					break;
				}
				else
				{						
					OutBuffer[i/2 + 1 ] = 0xff;
					break;
				}
			}
		}
		else
			OutBuffer[i/2 ] = 0xff;
	}	
	return;	
}
*/
u_char DisplayInfo( u_char line , u_char offset , u_char * buffer )
{
	u_char xdata length;

	length = strlen( buffer );
	DisplayStr( offset , line , 1 , buffer , length );
	return( 0 );
}

void  LoadTradeParam()
{
	//uint8_t *tp2;
	//uint16_t crc;
	u_char xdata RdBuf[128]={0};
	u_int xdata num=0;
	u_int xdata len = 0;
	u_int xdata crc=0;
	//u_char xdata str[20];
	/*
	tp2 = (uint8_t *) &SystemPara;//交易数据记录		
	AT45DBReadPage(4090,tp2);
	

	crc = CrcCheck(tp2,124);
	Trace("\r\n defaultsys=%d,%x,%x",124,crc/256,crc%256);
	if( (crc/256 == SystemPara.CrcCheck[0])&&(crc%256 == SystemPara.CrcCheck[1]) )
	{
		return 1;
	}
	else
	{
		return 0;
	}*/
	//交易记录
	//MovFlashToSram( ADDRF_TRADECOUNTER , ( unsigned char xdata * )&TradeCounter , sizeof(struct TRADECOUNTER) );
	//1.读数据到数组中
	MovFlashToSram( ADDRF_TRADECOUNTER , RdBuf , 128 );	
	len = sizeof(struct TRADECOUNTER);
	//2.数据导入到结构体中
	memcpy(( unsigned char xdata * )&TradeCounter ,RdBuf, len);
    //3.数组作CRC校验
	crc = CrcCheck(RdBuf,len);
	//Trace("\r\n defaultsys=%d,%x,%x",num,crc/256,crc%256);
	//4.结构体中存放原先读入的校验值
	//TradeCounter.CrcCheck[0] = RdBuf[126];
	//TradeCounter.CrcCheck[1] = RdBuf[127]; 
	//作比较
	if( (HUINT16(crc) == RdBuf[len])&&(LUINT16(crc) == RdBuf[len+1]) )
	{
		//len = sprintf( str, "deng=%x,%x-%x,%x", HUINT16(crc),LUINT16(crc),RdBuf[len],RdBuf[len+1] );
		//DisplayStr( 0, 0, 1, str, len );
		//WaitForWork( 2000, NULL );		
	}	
	else
	{
		//len = sprintf( str, "budeng=%x,%x-%x,%x", HUINT16(crc),LUINT16(crc),RdBuf[len],RdBuf[len+1] );
		//DisplayStr( 0, 0, 1, str, len );
		//WaitForWork( 2000, NULL );
		//拷贝备份数据页
		//1.读数据到数组中
		MovFlashToSram( ADDRF_TRADECOUNTERCOPY , RdBuf , 128 );	
		len = sizeof(struct TRADECOUNTER);
		//2.数据导入到结构体中
		memcpy(( unsigned char xdata * )&TradeCounter ,RdBuf, len);
	    //3.数组作CRC校验
		crc = CrcCheck(RdBuf,len);
		//Trace("\r\n defaultsys=%d,%x,%x",num,crc/256,crc%256);
		//4.结构体中存放原先读入的校验值
		RdBuf[len] = HUINT16(crc);
		RdBuf[len+1] = LUINT16(crc);

		//将备份数据页覆盖主数据页
		MovSramToFlash( ADDRF_TRADECOUNTER , RdBuf , 128 ); 
	}
	
	/*	
	TradeCounter.TrueCurrencySum 	= (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.CoinSum = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.CashSum = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.CashSumBack = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.CoinSum5j = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.CoinSum5jBack = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.CoinSum1y = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.CoinSum1yBack = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.Hopper1Sum = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.Hopper1SumBack = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.Hopper2Sum = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.Hopper2SumBack = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.TotalSellNum = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.TotalSellMoeny = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.vpSuccessNum = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.vpSuccessMoney = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.vpCashNum = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.vpCashMoney = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.vpGameNum = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.vpCardNum = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	TradeCounter.vpOnlineNum = (RdBuf[num++]<<24)|(RdBuf[num++]<<16)|(RdBuf[num++]<<8)|(RdBuf[num++]);
	*/
}


/*从Flash中装载数据*/
void   LoadGlobalParam()
{
	u_char xdata i = 0;		
	
	memset( &SystemParameter, 0, sizeof( SystemParameter ) );
	memset( GoodsWaySetVal, 0, sizeof( GoodsWaySetVal ) );
	memset( &TradeCounter, 0, sizeof(struct TRADECOUNTER) ); 
	//memset( TradeLog, 0, sizeof(struct TRADELOG) * GOODSWAYNUM ); 	
	memset( iRackSet, 0, sizeof(struct RackSet) * RACKNUM ); 	
	memset( iRackColumnSet, 0, sizeof(struct RackColumnSet) * RACKNUM * RACKCOLUMNNUM );
    //memset( &sysGoodsMatrix, 0, sizeof( sysGoodsMatrix ) );

	//本机系统参数
	MovFlashToSram( ADDRF_SYSTEMPARAM , ( unsigned char xdata * )&SystemParameter , sizeof(struct SYSTEMPARAMETER) );	
	MovFlashToSram( ADDRF_GOODSWAYS_SET , ( unsigned char xdata * )GoodsWaySetVal , sizeof( GoodsWaySetVal ) );	
	//交易记录
	//MovFlashToSram( ADDRF_TRADECOUNTER , ( unsigned char xdata * )&TradeCounter , sizeof(struct TRADECOUNTER) );		
    LoadTradeParam();
	//货道交易LOG设置
	//MovFlashToSram( ADDRF_WAYSTARDE_LOG , ( unsigned char xdata * )TradeLog , sizeof(struct TRADELOG) * GOODSWAYNUM );	
	//货架配置
	MovFlashToSram( ADDRF_RACK_SET , ( unsigned char xdata * )iRackSet , sizeof(struct RackSet) * RACKNUM );		
	//货道配置
	MovFlashToSram( ADDRF_RACK_COLUMN_SET , ( unsigned char xdata * )iRackColumnSet , sizeof(struct RackColumnSet) * RACKNUM * RACKCOLUMNNUM );	
    //Goods parameter
    ReadGoodsParSet();	

}

//写入货道参数
void SaveGoodsSet()
{	
	MovSramToFlash( ADDRF_GOODSWAYS_SET , ( unsigned char xdata * )GoodsWaySetVal , sizeof(GoodsWaySetVal) );	
}

/*把数据写到Flash中*/
void   SaveGlobalParam()
{
	//本机系统参数	
	u_char xdata i = 0;
	u_char xdata j = 0;
	u_char xdata z = 0;
	//本机系统参数	
	//生成系统参数的校验位	
	i = sizeof( struct SYSTEMPARAMETER ) - 1;
	j = 0;
	for( z = 0; z < i; z++ )			
		j ^= (( u_char* )&SystemParameter)[z];			
	SystemParameter.Checkbyte = j;

	MovSramToFlash( ADDRF_SYSTEMPARAM , ( unsigned char xdata * )&SystemParameter , sizeof(struct SYSTEMPARAMETER) );	
}

void  SaveTradeParam()
{
	//uint8_t *tp2;
	u_char xdata Wrbuf[128]={0},RdBuf[128]={0};
	u_int xdata num=0,i=0;
	//u_char xdata str[20];
	u_int xdata len = 0;
	u_int xdata crc=0;
	/*
	tp2 = (uint8_t *) &SystemPara;//交易数据记录	
	//crc = CrcCheck(tp2,sizeof(SystemPara)-2); 
	crc = CrcCheck(tp2,124);	
	SystemPara.CrcCheck[0] = crc/256;
	SystemPara.CrcCheck[1] = crc%256;
	Trace("\r\n defaultsys=%d,%x,%x",sizeof(SystemPara)-2,SystemPara.CrcCheck[0],SystemPara.CrcCheck[1]);	
	AT45DBWritePage(4090,tp2);
	*/
	//本机运行参数
	//MovSramToFlash( ADDRF_TRADECOUNTER , ( unsigned char xdata * )&TradeCounter , sizeof(struct TRADECOUNTER) );
	/*
	Wrbuf[num++] = (TradeCounter.TrueCurrencySum>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.TrueCurrencySum>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.TrueCurrencySum>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.TrueCurrencySum)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum)&0xff;
	Wrbuf[num++] = (TradeCounter.CashSum>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.CashSum>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.CashSum>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.CashSum)&0xff;
	Wrbuf[num++] = (TradeCounter.CashSumBack>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.CashSumBack>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.CashSumBack>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.CashSumBack)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum5j>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum5j>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum5j>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum5j)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum5jBack>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum5jBack>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum5jBack>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum5jBack)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum1y>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum1y>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum1y>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum1y)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum1yBack>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum1yBack>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum1yBack>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.CoinSum1yBack)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper1Sum>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper1Sum>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper1Sum>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper1Sum)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper1SumBack>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper1SumBack>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper1SumBack>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper1SumBack)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper2Sum>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper2Sum>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper2Sum>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper2Sum)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper2SumBack>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper2SumBack>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper2SumBack>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.Hopper2SumBack)&0xff;
	Wrbuf[num++] = (TradeCounter.TotalSellNum>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.TotalSellNum>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.TotalSellNum>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.TotalSellNum)&0xff;
	Wrbuf[num++] = (TradeCounter.TotalSellMoeny>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.TotalSellMoeny>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.TotalSellMoeny>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.TotalSellMoeny)&0xff;
	Wrbuf[num++] = (TradeCounter.vpSuccessNum>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.vpSuccessNum>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.vpSuccessNum>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.vpSuccessNum)&0xff;
	Wrbuf[num++] = (TradeCounter.vpSuccessMoney>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.vpSuccessMoney>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.vpSuccessMoney>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.vpSuccessMoney)&0xff;
	Wrbuf[num++] = (TradeCounter.vpCashNum>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.vpCashNum>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.vpCashNum>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.vpCashNum)&0xff;	
	Wrbuf[num++] = (TradeCounter.vpCashMoney>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.vpCashMoney>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.vpCashMoney>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.vpCashMoney)&0xff;
	Wrbuf[num++] = (TradeCounter.vpGameNum>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.vpGameNum>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.vpGameNum>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.vpGameNum)&0xff;
	Wrbuf[num++] = (TradeCounter.vpCardNum>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.vpCardNum>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.vpCardNum>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.vpCardNum)&0xff;	
	Wrbuf[num++] = (TradeCounter.vpOnlineNum>>24)&0xff;
	Wrbuf[num++] = (TradeCounter.vpOnlineNum>>16)&0xff;
	Wrbuf[num++] = (TradeCounter.vpOnlineNum>>8)&0xff;
	Wrbuf[num++] = (TradeCounter.vpOnlineNum)&0xff;
	*/
	for(i=0;i<5;i++)
	{
		len = sizeof(struct TRADECOUNTER);
	    //1.存放到数组中
		memcpy(Wrbuf,( unsigned char xdata * )&TradeCounter , len);
		//2.作CRC校验
		crc = CrcCheck(Wrbuf,len);	
		Wrbuf[len] = HUINT16(crc);
		Wrbuf[len+1] = LUINT16(crc);
		//len = sprintf( str, "%x,%x", Wrbuf[len],Wrbuf[len+1]);
		//DisplayStr( 0, 0, 1, str, len );
		//len = sprintf( str, "%u,%u,%u,%u,%u,%u,%u,%u-%u,%u", Wrbuf[8],Wrbuf[9],Wrbuf[10],Wrbuf[11],Wrbuf[12],Wrbuf[13],Wrbuf[14],Wrbuf[15],Wrbuf[126],Wrbuf[127]);
		//DisplayStr( 0, 1, 1, str, len );
		//WaitForWork( 5000, NULL );
		//3.保存到主页中
		MovSramToFlash( ADDRF_TRADECOUNTER , Wrbuf , 128 ); 
		//4.保存到备份页中
		MovSramToFlash( ADDRF_TRADECOUNTERCOPY , Wrbuf , 128 ); 
		DelayMs(5);
		//5.反读回来,然后重新判断,如果写入成功，就退出；否则重复写5次
		MovFlashToSram( ADDRF_TRADECOUNTER , RdBuf , 128 );	
		
		//len = sprintf( str, "%x,%x", RdBuf[len],RdBuf[len+1]);
		//DisplayStr( 0, 1, 1, str, len );
		//len = sprintf( str, "%u,%u,%u,%u,%u,%u,%u,%u-%u,%u", RdBuf[8],RdBuf[9],RdBuf[10],RdBuf[11],RdBuf[12],RdBuf[13],RdBuf[14],RdBuf[15],RdBuf[126],RdBuf[127]);
		//DisplayStr( 0, 1, 1, str, len );
		//WaitForWork( 5000, NULL );
		if(memcmp(Wrbuf,RdBuf,128)==0)//相等
		{
			//len = sprintf( str, "deng=%u,%x,%x-%x,%x", i,Wrbuf[len],Wrbuf[len+1],RdBuf[len],RdBuf[len+1] );
			//DisplayStr( 0, 0, 1, str, len );
			//WaitForWork( 2000, NULL );
			break;
		}
		else
		{
			//len = sprintf( str, "budeng=%u,%x,%x-%x,%x", i,Wrbuf[len],Wrbuf[len+1],RdBuf[len],RdBuf[len+1] );
			//DisplayStr( 0, 0, 1, str, len );
			//WaitForWork( 2000, NULL );
		}
		
	}	
}


//写交易LOG
void SaveTradeCounter()
{
	//货道交易LOG设置
	//MovSramToFlash( ADDRF_WAYSTARDE_LOG , ( unsigned char xdata * )TradeLog , sizeof(struct TRADELOG) * GOODSWAYNUM );		
}

//货架配置
void SaveRackSet()
{
	//货架配置
	MovSramToFlash( ADDRF_RACK_SET , ( unsigned char xdata * )iRackSet , sizeof(struct RackSet) * RACKNUM );		
}

//货道配置
void SaveRackColumnSet()
{
	//货道配置
	MovSramToFlash( ADDRF_RACK_COLUMN_SET , ( unsigned char xdata * )iRackColumnSet , sizeof(struct RackColumnSet) * RACKNUM * RACKCOLUMNNUM );		
}

//Save the goods parameter
void SaveGoodsParSet( void )
{
	//MovSramToFlash( ADDRF_GOODSPAR_SET, ( unsigned char xdata * )&sysGoodsMatrix, sizeof( sysGoodsMatrix ) );	
}

void ReadGoodsParSet( void )
{
    // MovFlashToSram( ADDRF_GOODSPAR_SET, ( unsigned char xdata * )&sysGoodsMatrix, sizeof( sysGoodsMatrix ) );
}
