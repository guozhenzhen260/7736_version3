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
#include "IOInput.h"
#include "communication.h"

#include "Serial1.h"
#include "scheduler.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"



//VMCÍ¨ÖªGCC¸´Î»³õÊ¼»¯
#define VMC_RESET_REQ				(0x80)
//VMCÍ¨ÖªGCC±¨¸æGCC×´Ì¬
#define VMC_STATUS_REQ				(0x81)
//VMCÍ¨ÖªGCC³ö»õ
#define VMC_VENDING_REQ				(0x82)
//VMCÍ¨ÖªGCC±¨¸æ³ö»õÇé¿ö
#define VMC_VENDINGRESULT_REQ		(0x83)


//GCCÍ¨ÖªVMC¸´Î»³õÊ¼»¯
#define GCC_RESET_ACK				(0x00)
//GCCÍ¨ÖªVMC±¨¸æGCC×´Ì¬
#define GCC_STATUS_ACK				(0x01)
//GCCÍ¨ÖªVMC³ö»õ
#define GCC_VENDING_ACK				(0x02)
//GCCÍ¨ÖªVMC±¨¸æ³ö»õÇé¿ö
#define GCC_VENDINGRESULT_ACK		(0x03)
//GCCÍ¨ÖªVMC¿ªÃÅ
#define GCC_OPENDOOR_ACK			(0x04)
//GCCÍ¨ÖªVMC¹ØÃÅ
#define GCC_CLOSEDOOR_ACK			(0x05)
//GCCÍ¨ÖªVMC´ò¿ªÕÕÃ÷µÆ
#define GCC_OPENLIGHT_ACK			(0x06)
//GCCÍ¨ÖªVMC¹Ø±ÕÕÕÃ÷µÆ
#define GCC_CLOSELIGHT_ACK			(0x07)
//GCCÍ¨ÖªVMC±¨¸æÅäÖÃ²ÎÊý
#define GCC_GETINFO_ACK				(0x08)




//³ö»õ²Ù×÷
#define CHANNEL_OUTGOODS		(1)
//¼ì²â»õµÀµ±Ç°×´Ì¬			
#define CHANNEL_CHECKSTATE		(2)
//¼ì²â»õµÀÉÏ´ÎµÄ³ö»õ½á¹û			
#define CHANNEL_CHECKOUTRESULT	(3)	
//Çå³þsn
#define CHANNEL_CLEARSN			(4)



static struct COMMTASK xdata lifterTask,ackTask;
uchar lifterSendMsg(unsigned char type,uchar binNo,uchar rowNo,uchar columNo);
uchar LifterVendoutTask(struct COMMTASK xdata*liftPtr);
uint LifterCheckStateTask(uchar binNo,uchar *errNo);
uchar LifterVendout(uchar binNum,uchar logicNo);













	


void lifterDelay(unsigned long timeout)
{
	lifterTimer = timeout / 10;
	while(lifterTimer);
}



/*****************************************************************************
** Function name:	ZhkLifterRxMsg	
** Descriptions:	      ½ÓÊÕ´®¿Ú»ØÓ¦°ü													 			
** parameters:		ÎÞ
** Returned value:	0:ÎÞÊý¾Ý½ÓÊÕ 1:½ÓÊÕÍê±Ï 2:Êý¾Ý½ÓÊÕ´íÎó
*****************************************************************************/
u_char ZhkLifterRxMsg( void )
{
	
	uchar i,index = 0,rcx = 50;
	uint crc;
	memcpy( &ackTask, &CurrentTask, sizeof( struct COMMTASK ) );
	while(rcx)
	{
		if(ZhkSerial1IsRxBufNull())//ÎÞÊý¾Ý ÐèÒªÑÓÊ±µÈ´ýÒ»ÏÂ
		{
			for(i = 0;i < 10;i++)
			{
				i = i;
			}
			rcx--;
		}
		else
		{
			ackTask.Param[index] = ZhkSerial1GetCh();
			if(index == 0)
			{
				//ÅÐ¶Ï°üÍ·
				if(ackTask.Param[0] != 0xC8)//head err
					continue;
			}
			else if(index == 1)
				ackTask.ParamLen = ackTask.Param[index];
			index++;
			if(index >= 7)
			{
				crc = CrcCheck(ackTask.Param,ackTask.ParamLen);
				return 1;
#if 0
				if(ackTask.Param[index - 2] == crc / 256 && ackTask.Param[index-1] == crc % 256)
					return 1;
				else
					return 2;
#endif
			}
			
		}
	}
	
	if(index)
		return 2;
	else
		return 0;
	
	
}




/*********************************************************************************************************
** Function name:     	ZhkLifterTxMsg
** Descriptions:	       Éý½µ»ú·¢ËÍ´®¿Ú
** input parameters:    
** output parameters:   ÎÞ
** Returned value:      CRC¼ìÑé½á¹û
*********************************************************************************************************/
bit ZhkLifterTxMsg( struct COMMTASK xdata* NewTask )
{
	uchar i,len;
	uint  crc;
	len = NewTask->Param[1];
	crc	= CrcCheck(NewTask->Param,len);//¼ÓÐ£Ñé
	NewTask->Param[len] = (uchar)(crc >> 8);
	NewTask->Param[len + 1] = (uchar)(crc & 0xFF);
	for(i = 0;i < len + 2;i++)
	{	
		ZhkSerial1PutCh( NewTask->Param[i]);
	}
	
	return 1;	
}

//////////////////////////////////////////////////////////
///»õµÀ´¦Àí1±íÊ¾
//////////////////////////////////////////////////////////
bit  ZhkLifterTask( struct COMMTASK xdata* TaskTemp )
{
	switch( ZhkSchedulerState )
	{
	case TASK_NULL:
		break;
	case TASK_REDAY:	//·¢ËÍ´®¿ÚÇëÇó
		ZhkChannelSwitch( EQUIP_CHANNEL );	
		if(TaskTemp->Cmd == VMC_VENDING_REQ)//³ö»õ×ßÁíÒ»¸öÃüÁî
		{
			Channel.State = LifterVendoutTask(TaskTemp);
			ZhkSchedulerState = TASK_FINISH;
		}
		else
		{
			ZhkLifterTxMsg( TaskTemp );
			ZhkSchedulerState = TASK_WAIT;
			ZhkDownMsgAckTimer  = 500;//15Ãë µÈ´ý
			Channel.CommState = COMM_BUZY;
			Channel.ExtState[2] = 0;
		}
		
		break;	
	case TASK_WAIT:
		if(!ZhkSerial1IsRxBufNull())//ÓÐÊý¾Ý
		{
			ZhkSchedulerState = TASK_FINISH;
			Channel.State = ZhkLifterRxMsg();
			break;
		}
		
		if ( ZhkDownMsgAckTimer == 0 )//³¬Ê±
		{
			ZhkSchedulerState = TASK_ERROR;					
		}		
		break;			
	case TASK_FINISH:
		ZhkSchedulerState = TASK_NULL; 		
		Channel.CommState   = COMM_COMMOK;
		memset( &CurrentTask, 0, sizeof( struct COMMTASK ) );
		ZhkDownMsgAckTimer = 0;
		break;
	case TASK_ERROR:
		Channel.CommState = COMM_TIMEOUT;
		ZhkSchedulerState   = TASK_NULL; 
		memset( &CurrentTask, 0, sizeof( struct COMMTASK ) );
		break;
	default:
		ZhkSchedulerState = TASK_NULL;
		
	}
	return 1;
}

/*****************************************************************************
** Function name:	LifterSendVendout	
** Descriptions:	      	·¢ËÍ³ö»õ²éÑ¯½á¹ûÃüÁî													 			
** parameters:		binNo:¹ñºÅ
** Returned value:	µÍ°ËÎ»  0:²éÑ¯Ê§°Ü 1:³ö»õÍê³É 2:ÕýÔÚ³ö»õ
								0xFF:Í¨ÐÅ¹ÊÕÏ 
					

					¸ß°ËÎ» ½ö"³ö»õÊ§°Ü ÒÔÏÂ×Ö¶ÎÓÐÐ§"
					0:³ö»õ³É¹¦
					1:Êý¾Ý´íÎó
					2:ÎÞ»õ				3:¿¨»õ
					4:È¡»õÃÅÃ»¿ªÆô
					5:»õÎïÎ´È¡×ß 		6:»õ¹ñ´óÃÅ±»´ò¿ªÉý
					7:½µ»ú¹¹¹ÊÕÏ  	other:ÆäËû¹ÊÕÏ
*****************************************************************************/
uint LifterSendCheckRst(uchar binNo)
{
	uchar rst = 0;
	rst = lifterSendMsg(VMC_VENDINGRESULT_REQ,binNo,0,0);
	if(rst == 0)
		return 0xFF;
	if(ackTask.Param[4] == GCC_VENDINGRESULT_ACK)//³É¹¦ÊÕµ½ACK
	{	
		if(ackTask.Param[5] == 0x01)//ÕýÔÚ³ö»õ
			return 2;
		else
		{
			if(ackTask.Param[6] == 0x00)//³ö»õ³É¹¦
				return 1;
			else
				return (((uint)ackTask.Param[7] << 8)| (0x01));
		}
	}
	return 0;
	
}



/*****************************************************************************
** Function name:	LifterSendCheckState	
** Descriptions:	      	·¢ËÍ²éÑ¯×´Ì¬ÃüÁî													 			
** parameters:		binNo:¹ñºÅ
** Returned value:	µÍ°ËÎ»0Õû»ú¿ÕÏÐ 1Õû»úÃ¦2²éÑ¯Ê§°Ü0xFFÍ¨ÐÅÊ§°Ü
					¸ß°ËÎ»(°´Î»ËãÖÃ1±íÊ¾ÓÐ¹ÊÕÏ)
					bit0 Õû»ú×´Ì¬¹ÊÕÏ
					bit1Éý½µ»ú¹ÊÕÏ
					bit2È¡»õµç»ú¹ÊÕÏ
					bit3ÓÃ»§µç¶¯ÃÅ¹ÊÕÏ
					bit4µ¯±Ò»ú¹¹¹ÊÕÏ
					bit5·ÀµÁÄ£¿é¹ÊÕÏ
					bit6»õµ½¿ØÖÆÆ÷¹ÊÕÏ
					bit7»õµÀ´óÃÅ¹ÊÕÏ
*****************************************************************************/
uint LifterSendCheckState(uchar binNo)
{
	uchar rst = 0,state,errNo = 0,rcx = 2;
	while(rcx--)
	{
		rst = lifterSendMsg(VMC_STATUS_REQ,binNo,0,0);
		if(rst == 0)
			continue;		
		if(ackTask.Param[4] == GCC_STATUS_ACK)//³É¹¦ÊÕµ½ACK
		{	
			state = (ackTask.Param[5] == 0x01) ? 1 : 0;//Õû»ú±êÖ¾ 0¿ÕÏÐ 1Ã¦
			errNo |= (ackTask.Param[6] & 0x01) << 0;//Õû»ú×´Ì¬
			errNo |= (ackTask.Param[7] & 0x01) << 1;//Éý½µ»ú
			errNo |= (ackTask.Param[8] & 0x01) << 2;//È¡»õµç»ú
			errNo |= (ackTask.Param[9] & 0x01) << 3;//ÓÃ»§µç¶¯ÃÅ
			errNo |= (ackTask.Param[10] & 0x01) << 4;//µ¯±Ò»ú¹¹
			errNo |= (ackTask.Param[11] & 0x01) << 5;//·ÀµÁÄ£¿é
			errNo |= (ackTask.Param[12] & 0x01) << 6;//»õµ½¿ØÖÆÆ÷
			errNo |= (ackTask.Param[13] & 0x01) << 6;//»õµÀ´óÃÅ	
			
			return (((uint)errNo << 8) | state);
			
		}
		return 2;
	}

	return 0xFF;
	
}



/*****************************************************************************
** Function name:	LifterSendVendout	
** Descriptions:	      	·¢ËÍ³ö»õÃüÁî													 			
** parameters:		binNo:¹ñºÅ
					rowNo:²ãºÅ
					columnNo:µÀºÅ
** Returned value:	0:·¢ËÍÊ§°Ü; 1:·¢ËÍ³É¹¦;0xFF Í¨ÐÅÊ§°Ü
*****************************************************************************/
uchar LifterSendVendout(uchar binNo,uchar rowNo,uchar columnNo)
{
	uchar rst = 0,rcx = 2;
	while(rcx--)
	{
		rst = lifterSendMsg(VMC_VENDING_REQ,binNo,rowNo,columnNo);
		if(rst == 0)
			return 0xFF;
		if(ackTask.Param[4] == GCC_VENDING_ACK)//³É¹¦ÊÕµ½ACK
		{
			return 1;
		}
		//Èç¹ûÃ»ÓÐÊÕµ½ACKÔòÐèÒªÔÚ·¢ËÍÒ»´Î²éÑ¯ÃüÁî
		//È·±£³ö»õÃüÁî·¢ËÍ³É¹¦±ÜÃâÖØ¸´·¢ËÍ³ö»õÃüÁî
		if(LifterSendCheckRst(binNo) == 2)//±íÊ¾»úÆ÷·±Ã¦ Í¬ÑùÒâÎ¶×Å³ö»õÃüÁî·¢ËÍ³É¹¦
			return 1;
	}
	return 0;
}


/*****************************************************************************
** Function name:	lifterSendMsg	
** Descriptions:													 			
** parameters:		type:²Ù×÷ÀàÐÍ
					binNo:Ïä¹ñ±àºÅ
					rowNo:²ã±àºÅ
					columNo:»õµÀºÅ					
** Returned value:	0:Í¨ÐÅÊ§°Ü; 1:Í¨ÐÅ³É¹¦;
*****************************************************************************/
uchar lifterSendMsg(unsigned char type,uchar binNo,uchar rowNo,uchar columNo)
{
	uchar timeout = 0,comOK = 0,index = 0;
	uchar devType = 0x40;//ÆÕÍ¨µ¯»ÉÐÍ 
	binNo = binNo;
	memset( &lifterTask, 0, sizeof( struct COMMTASK ) );
	Channel.CommState   = COMM_BUZY;
	lifterTask.Id 		= ID_CHANNEL;
	lifterTask.Cmd 		= type;
	lifterTask.Priority = PRIORITY_NORMAL; 

	lifterTask.Param[index++] = 0xC7;
	lifterTask.Param[index++] = 0x05;//³¤¶È
	lifterTask.Param[index++] = devType;
	lifterTask.Param[index++] = 0x00;//°æ±¾¹Ì¶¨0
	lifterTask.Param[index++] = type;
	if(rowNo != 0x00)
		lifterTask.Param[index++] = rowNo;
	if(columNo != 0x00)
		lifterTask.Param[index++] = columNo;

	lifterTask.ParamLen	= index;
	lifterTask.Param[1] = index;//³¤¶ÈÖØ¶¨Î»

	ZhkSchedulerAddTask( &lifterTask );
	while(! ( timeout || comOK ))
	{
		WaitForWork( 50, NULL );
		timeout = TestDeviceTimeOut( &Channel );
		comOK = TestDeviceCommOK( &Channel );	
	}

	if(timeout)
		return 0;
	if(comOK)
		return (Channel.State == 1);	
	return 0;


}



/*****************************************************************************
** Function name:	LifTerProcess	
** Descriptions:														 			
** parameters:		ÎÞ				
** Returned value:	01:³É¹¦
					11:Êý¾Ý´íÎó 
					12:ÎÞ»õ
					13:¿¨»õ		  
					14:È¡»õÃÅÃ»¿ªÆô
					15:»õÎïÎ´È¡×ß
					16:»õ¹ñ´óÃÅ±»´ò¿ª
					17:Éý½µ»ú¹¹¹ÊÕÏ
					21:Õû»ú¹ÊÕÏ
					22:Éý½µ»ú¹ÊÕÏ
					23:È¡»õµç»ú¹ÊÕÏ
					24:ÓÃ»§µç¶¯ÃÅ¹ÊÕÏ
					25:µ¯±Ò»ú¹¹¹ÊÕÏ
					26:·ÀµÁÄ£¿é¹ÊÕÏ
					27:»õµ½¿ØÖÆÆ÷¹ÊÕÏ
					28:»õµÀ´óÃÅ¹ÊÕ
					51:×´Ì¬²éÑ¯³¬Ê±
					52:³ö»õÃüÁî·¢ËÍÊ§°Ü
					53:³ö»õ½á¹û²éÑ¯³¬Ê±
					99:ÆäËû¹ÊÕÏ
					255(0xFF):Í¨ÐÅÊ§°Ü
*******************************************************************************/
uchar LifTerProcess(uchar binNum,uchar logicNo)
{
	uchar Result[36]={0},res = 0,rcx = 5;
	uchar rowNo = 0,columnNo = 0,stateH,stateL;
	unsigned short takeTimeOut = 10,vendoutTimeout;
	uint liftState;
	if(logicNo == 0x00)
		return 0xff;
	rowNo = logicNo / 10 ;
	columnNo = logicNo % 10;
	
	//¿ªÊ¼³ö»õ //²éÑ¯×´Ì¬ÊÇ·ñÕý³££¬Õý³£³ö»õ
	rcx = 10;
	while(rcx--)
	{
		liftState = LifterSendCheckState(binNum); 
		stateL = liftState & 0xFF;
		stateH = (liftState >> 8) & 0xFF;
		if(stateL == 0)//Õý³£¾ÍÌø¿ª
			break;
		else if(stateL == 0xFF)//Í¨ÐÅÊ§°Ü
		{
			return 0xFF;
		}
		else 
		{
			if(stateH & 0xFF)//ÓÐ¹ÊÕÏ
			{
				if(stateH & 0x01) return 21;
				if(stateH & 0x02) return 22;
				if(stateH & 0x04) return 23;
				if(stateH & 0x08) return 24;
				if(stateH & 0x10) return 25;
				if(stateH & 0x20) return 26;
				if(stateH & 0x40) return 27;
				if(stateH & 0x80) return 28;
				return 99;
					
			}
		}
		WaitForWork( 1000, NULL );
	}

	if(liftState != 0)//³¬Ê±²éÑ¯ÈÎÈ»²»Õý³£
		return 51;
	
	while(1)
	{
		//½øÐÐ³ö»õ²Ù×÷
		res = LifterSendVendout(binNum,rowNo,columnNo);
		if(res == 0 || res == 0xFF)
			return 52;
		
		WaitForWork(50, NULL );
		//5000 * 10  = 50000 
		vendoutTimeout = 100 +(8 - rowNo) * 10;//¼ì²â³ö»õ½á¹û³¬Ê±
		while(vendoutTimeout--)
		{	
			liftState = LifterSendCheckRst(binNum);//¼ì²â³ö»õ½á¹û
			if((liftState & 0xFF) == 0x01)//³ö»õÍê³É
			{
				break;
			}
			WaitForWork( 1000, NULL );
		}
		
		if(vendoutTimeout)
		{
			if((liftState & 0xFF00))//³ö»õÊ§°Ü
			{
				if((((liftState >> 8) & 0xFF) == 5) && takeTimeOut)//»õÎ´È¡×ß
				{
					WaitForWork( 2000, NULL );
					takeTimeOut--;
					continue;
				}
				stateH = (liftState >> 8) & 0xFF ;
				return ((stateH > 7) ? (stateH + 10) : 99);
			}	
			else//³ö»õ³É¹¦
				return 1;
		}
		else										   	
			return 53;		
	}
//	return 0xFF;
}



uchar LifterVendoutReturn(uchar res,uchar flag)
{
	uchar val = 0;
	switch(res)
	{
		case 1:
#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "³ö»õ³É¹¦", sizeof("³ö»õ³É¹¦") );
#endif
			val = 0;
			break;
		case 11:
#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "Êý¾Ý´íÎó", sizeof("Êý¾Ý´íÎó") );
#endif
			//DisplayStr( 0, 1, 1, "Err data", sizeof("Err data") );
			
			val = 11;
			break;
		case 12:
#ifdef _CHINA_	
			if(flag == 1)
			DisplayStr( 0, 1, 1, "»õµÀÎÞ»õ", sizeof("»õµÀÎÞ»õ") );
			//DisplayStr( 0, 1, 1, "No goods", sizeof("No goods") );
#endif
			val = 3;
			break;
		case 13:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "»õµÀ¿¨»õ", sizeof("»õµÀ¿¨»õ") );
			//DisplayStr( 0, 1, 1, "Ka Huo", sizeof("Ka Huo") );
			#endif
			val = 13;
			break;
		case 14:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "È¡»õÃÅÃ»¿ªÆô", sizeof("È¡»õÃÅÃ»¿ªÆô") );
			//DisplayStr( 0, 1, 1, "Ka Huo", sizeof("Ka Huo") );
			#endif
			val = 14;
			break;
		case 15:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "»õÎïÎ´È¡×ß", sizeof("»õÎïÎ´È¡×ß") );
			//DisplayStr( 0, 1, 1, "Mei Qu zou", sizeof("Mei Qu zou") );
			#endif
			val = 15;
			break;
		case 16:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "»õ¹ñ´óÃÅ±»´ò¿ª", sizeof("»õ¹ñ´óÃÅ±»´ò¿ª") );
			#endif
			val = 16;
			break;
		case 17:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "Éý½µ»ú¹¹¹ÊÕÏ", sizeof("Éý½µ»ú¹¹¹ÊÕÏ") );
			//DisplayStr( 0, 1, 1, "Err Lift", sizeof("Err Lift") );
			#endif
			val = 17;
			break;
		case 21:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "Õû»ú¹ÊÕÏ", sizeof("Õû»ú¹ÊÕÏ") );
			#endif
			val = 21;
			break;
		case 22:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "Éý½µ»ú¹ÊÕÏ", sizeof("Éý½µ»ú¹ÊÕÏ") );
			#endif
			val = 22;
			break;
		case 23:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "È¡»õµç»ú¹ÊÕÏ", sizeof("È¡»õµç»ú¹ÊÕÏ") );
			#endif
			val = 23;
			break;
		case 24:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "ÓÃ»§µç¶¯ÃÅ¹ÊÕÏ", sizeof("ÓÃ»§µç¶¯ÃÅ¹ÊÕÏ") );
			#endif
			val = 24;
			break;
		case 25:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "µ¯±Ò»ú¹¹¹ÊÕÏ", sizeof("µ¯±Ò»ú¹¹¹ÊÕÏ") );
			#endif
			val = 25;
			break;
		case 26:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "·ÀµÁÄ£¿é¹ÊÕÏ", sizeof("·ÀµÁÄ£¿é¹ÊÕÏ") );
			#endif
			val = 26;
			break;
		case 27:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "»õµ½¿ØÖÆÆ÷¹ÊÕÏ", sizeof("»õµ½¿ØÖÆÆ÷¹ÊÕÏ") );
			#endif
			val = 27;
			break;
		case 28:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "»õµÀ´óÃÅ¹ÊÕÏ", sizeof("»õµÀ´óÃÅ¹ÊÕÏ") );
			#endif
			val = 28;
			break;
		case 51:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "×´Ì¬²éÑ¯³¬Ê±", sizeof("×´Ì¬²éÑ¯³¬Ê±") );
			#endif
			//DisplayStr( 0, 1, 1, "Err State Check", sizeof("Err State Check") );
			val = 2;
			break;
		case 52:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "³ö»õÃüÁî·¢ËÍÊ§°Ü", sizeof("³ö»õÃüÁî·¢ËÍÊ§°Ü") );
			#endif
			val = 2;
			//DisplayStr( 0, 1, 1, "Err Send Vendout", sizeof("Err Send Vendout") );
			break;
		case 53:
			#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "³ö»õ½á¹û²éÑ¯³¬Ê±", sizeof("³ö»õ½á¹û²éÑ¯³¬Ê±") );
			#endif
			//DisplayStr( 0, 1, 1, "Err Vendout Check", sizeof("Err Vendout Check") );
			val = 1;
			break;
		case 99:
#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "ÆäËû¹ÊÕÏ", sizeof("ÆäËû¹ÊÕÏ") );
#endif
			///DisplayStr( 0, 1, 1, "Err Other", sizeof("Err Other") );
			val = 99;
			break;
		case 0xFF:
#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "Í¨ÐÅ¹ÊÕÏ", sizeof("Í¨ÐÅ¹ÊÕÏ") );
#endif
			//DisplayStr( 0, 1, 1, "Err Com", sizeof("Err Com") );
			val = 0xFF;
			break;
		default:
#ifdef _CHINA_
			if(flag == 1)
			DisplayStr( 0, 1, 1, "ÆäËû¹ÊÕÏ", sizeof("ÆäËû¹ÊÕÏ") );
#endif
			val = 99;
			break;
	}

	WaitForWork( 2000, NULL );
	return val;
}


/*****************************************************************************
** Function name:	ChannelLifterTask	
** Descriptions:		Éý½µ»ú+´«ËÍ´ø³ö»õº¯Êý												 			
** parameters:		ÎÞ				
** Returned value:	0:ÕýÈ·Ö´ÐÐµÄÃüÁî1:³ö»õ·µ»ØÊ§°Ü
**					2:Ö´ÐÐÃüÁî³¬Ê±     3:»õµÀÎÞ»õ
**					4:»õµÀ¹ÊÕÏ   		  5:ÔÚÏÞ¶¨µÄÊ±¼äÄÚµç»ú²»ÄÜµ½Î»

**					 6:						7: CMD ERR  8: GOC check error 
*******************************************************************************/
u_char ChannelLifterTask( u_char ChannelNum, u_char TaskId )
{
	u_char xdata res = 0;
	uint temp = 0;
	switch(TaskId)
	{
		case CHANNEL_EXEC:
		case CHANNEL_TEST:
			//res = LifTerProcess(1,ChannelNum);
			res = LifterVendout(1,ChannelNum);
			res = LifterVendoutReturn(res,TaskId == CHANNEL_TEST);
			break;
		case CHANNEL_CLEAR: 
		case CHANNEL_QUERY:
			temp = LifterSendCheckState(1);
			if((temp & 0x00FF) == 2)
				res = 2;
			else
				res = 0;
			break;
		default:
			res = 7;
			break;
	}		
	return res;

	
}



/*****************************************************************************
**ÈÎÎñ´¦Àí³ö»õ¶¯×÷
*******************************************************************************/

/*****************************************************************************
** Function name:	LifterRecv	
** Descriptions:	      ½ÓÊÕ´®¿Ú»ØÓ¦°ü													 			
** parameters:		ÎÞ
** Returned value:	0:ÎÞÊý¾Ý½ÓÊÕ 1:½ÓÊÕÍê±Ï 2:Êý¾Ý½ÓÊÕ´íÎó
*****************************************************************************/
uchar LifterRecv(uint timeout)
{
	uchar index = 0;
	uint crc;
	timeout = timeout;
	memset( &ackTask, 0, sizeof( struct COMMTASK ) );
	ZhkDownMsgAckTimer = 1500;
	while(ZhkDownMsgAckTimer)
	{
		if(!ZhkSerial1IsRxBufNull())
		{
			ackTask.Param[index++] = ZhkSerial1GetCh();
			if(index == 1)//first byte 
			{
				if(ackTask.Param[0] != 0xC8)//head err
					continue;
			}
			else if(index == 2) //2th byte
				ackTask.ParamLen = ackTask.Param[1];
			else if(index >= (ackTask.ParamLen + 2)) //recv over
			{
				crc = CrcCheck(ackTask.Param,ackTask.ParamLen);
				return 1;
#if 0
				if(ackTask.Param[index - 2] == crc / 256 && ackTask.Param[index-1] == crc % 256)
					return 1;
				else
					return 2;
#endif
			}
			
		}
	}
	
	if(index)
		return 2;
	else
		return 0;
}

/*****************************************************************************
** Function name:	LifterSend	
** Descriptions:	      ½ÓÊÕ·¢ËÍ²¢½ÓÊÜ													 			
** parameters:		ÎÞ
** Returned value:	0:ÎÞÊý¾Ý½ÓÊÕ 1:½ÓÊÕÍê±Ï 2:Êý¾Ý½ÓÊÕ´íÎó
*****************************************************************************/

uchar LifterSend(unsigned char type,uchar binNo,uchar rowNo,uchar columNo)
{
	uchar timeout = 0,comOK = 0,index = 0,i,buf[12] = {0};
	uint crc;
	binNo = binNo;
	Channel.CommState	= COMM_BUZY;
	buf[index++] = 0xC7;
	buf[index++] = 0x05;//³¤¶È
	buf[index++] = 0x40;
	buf[index++] = 0x00;//°æ±¾¹Ì¶¨0
	buf[index++] = type;
	if(rowNo != 0x00)
		buf[index++] = rowNo;
	if(columNo != 0x00)
		buf[index++] = columNo;
	
	buf[1] = index;//³¤¶ÈÖØ¶¨Î»
	crc	= CrcCheck(buf,index);//¼ÓÐ£Ñé
	buf[index++] = (uchar)(crc >> 8);
	buf[index++] = (uchar)(crc & 0xFF);
	for(i = 0;i < index;i++)
	{	
		ZhkSerial1PutCh( buf[i]);
	}
	
	return LifterRecv(500);


}
	

/*****************************************************************************
** Function name:	LifterCheckRstTask	
** Descriptions:	      	·¢ËÍ³ö»õ²éÑ¯½á¹ûÃüÁî													 			
** parameters:		binNo:¹ñºÅ
** Outputparameters: errNo   ½ö³ö»õÍê³ÉºóÒ»ÏÂ×Ö¶ÎÓÐÐ§
					0:³ö»õ³É¹¦
					1:Êý¾Ý´íÎó
					2:ÎÞ»õ				3:¿¨»õ
					4:È¡»õÃÅÃ»¿ªÆô
					5:»õÎïÎ´È¡×ß 		6:»õ¹ñ´óÃÅ±»´ò¿ªÉý
					7:½µ»ú¹¹¹ÊÕÏ  	other:ÆäËû¹ÊÕÏ
** Returned value:	 0:²éÑ¯Ê§°Ü 1:³ö»õÍê³É 
					 2:ÕýÔÚ³ö»õ0xFF:Í¨ÐÅ¹ÊÕÏ 
*****************************************************************************/
uint LifterCheckRstTask(uchar binNo,uchar *errNo)
{
	uchar rst = 0;
	rst = LifterSend(VMC_VENDINGRESULT_REQ,binNo,0,0);
	if(rst == 0)
		return 0xFF;
	if(rst == 2)
		return 0;
	if(ackTask.Param[4] == GCC_VENDINGRESULT_ACK)//³É¹¦ÊÕµ½ACK
	{	
		if(ackTask.Param[5] == 0x01)//ÕýÔÚ³ö»õ
			return 2;
		else
		{
			if(ackTask.Param[6] == 0x00)//³ö»õ³É¹¦
			{
				*errNo = 0;
				return 1;
			}
			else
			{
				*errNo = ackTask.Param[7];
				return 1;
			}
				
		}
	}
	return 0;
	
}



/*****************************************************************************
** Function name:	LifterSendCheckState	
** Descriptions:	      	·¢ËÍ²éÑ¯×´Ì¬ÃüÁî													 			
** parameters:		binNo:¹ñºÅ
** outparameters:      0:ÎÞ¹ÊÕÏ
					1:Õû»ú×´Ì¬¹ÊÕÏ
					2:Éý½µ»ú¹ÊÕÏ
					3:È¡»õµç»ú¹ÊÕÏ
					4:ÓÃ»§µç¶¯ÃÅ¹ÊÕÏ
					5:µ¯±Ò»ú¹¹¹ÊÕÏ
					6:·ÀµÁÄ£¿é¹ÊÕÏ
					7:»õµ½¿ØÖÆÆ÷¹ÊÕÏ
					8:»õµÀ´óÃÅ¹ÊÕÏ
** Returned value:	0Õû»ú¿ÕÏÐ 1Õû»úÃ¦2²éÑ¯Ê§°Ü0xFFÍ¨ÐÅÊ§°Ü
*****************************************************************************/
uint LifterCheckStateTask(uchar binNo,uchar *errNo)
{
	uchar rst = 0;
	rst = LifterSend(VMC_STATUS_REQ,binNo,0,0);
	if(rst == 0)
		return 0xFF;
	if(rst == 2)
		return 2;
	
	if(ackTask.Param[4] == GCC_STATUS_ACK)//³É¹¦ÊÕµ½ACK
	{	
		if(ackTask.Param[5] == 0)//Õû»ú±êÖ¾ 0¿ÕÏÐ 1Ã¦
		{
			*errNo = 0;
			return 0;
		}
		else
		{
			*errNo = 1;
			if(ackTask.Param[6] & 0x01) *errNo = 1;//Õû»ú×´Ì¬
			if(ackTask.Param[7] & 0x01) *errNo = 2;//Éý½µ»ú
			if(ackTask.Param[8] & 0x01) *errNo = 3;//È¡»õµç»ú
			if(ackTask.Param[9] & 0x01) *errNo = 4;//ÓÃ»§µç¶¯ÃÅ
			if(ackTask.Param[10] & 0x01) *errNo = 5;//µ¯±Ò»ú¹¹
			if(ackTask.Param[11] & 0x01) *errNo = 6;//·ÀµÁÄ£¿é
			if(ackTask.Param[12] & 0x01) *errNo = 7;//»õµ½¿ØÖÆÆ÷
			if(ackTask.Param[13] & 0x01) *errNo = 8;//»õµÀ´óÃÅ	
			return 1;
		}
		
	}
	return 2;
	
}

/*****************************************************************************
** Function name:	LifterSendVendout	
** Descriptions:	      	·¢ËÍ³ö»õÃüÁî													 			
** parameters:		binNo:¹ñºÅ
					rowNo:²ãºÅ
					columnNo:µÀºÅ
** Returned value:	0:·¢ËÍÊ§°Ü; 1:·¢ËÍ³É¹¦;0xFF Í¨ÐÅÊ§°Ü
*****************************************************************************/
uchar LifterSendVendoutTask(uchar binNo,uchar rowNo,uchar columnNo)
{
	uchar rst = 0,rcx = 2,errNo;
	while(rcx--)
	{
		rst = LifterSend(VMC_VENDING_REQ,binNo,rowNo,columnNo);
		if(rst == 0 || rst == 2)
			return 0xFF;
		if(ackTask.Param[4] == GCC_VENDING_ACK)//³É¹¦ÊÕµ½ACK
		{
			return 1;
		}
		
		//Èç¹ûÃ»ÓÐÊÕµ½ACKÔòÐèÒªÔÚ·¢ËÍÒ»´Î²éÑ¯ÃüÁî
		//È·±£³ö»õÃüÁî·¢ËÍ³É¹¦±ÜÃâÖØ¸´·¢ËÍ³ö»õÃüÁî
		rst = LifterCheckRstTask(binNo,&errNo);
		if(rst == 2 || rst == 1)//±íÊ¾»úÆ÷·±Ã¦ Í¬ÑùÒâÎ¶×Å³ö»õÃüÁî·¢ËÍ³É¹¦
			return 1;
	}
	return 0;
}



/*****************************************************************************
** Function name:	LifterVendoutTask	
** Descriptions:														 			
** parameters:		ÎÞ				
** Returned value:	01:³É¹¦
					11:Êý¾Ý´íÎó 
					12:ÎÞ»õ
					13:¿¨»õ		  
					14:È¡»õÃÅÃ»¿ªÆô
					15:»õÎïÎ´È¡×ß
					16:»õ¹ñ´óÃÅ±»´ò¿ª
					17:Éý½µ»ú¹¹¹ÊÕÏ
					21:Õû»ú¹ÊÕÏ
					22:Éý½µ»ú¹ÊÕÏ
					23:È¡»õµç»ú¹ÊÕÏ
					24:ÓÃ»§µç¶¯ÃÅ¹ÊÕÏ
					25:µ¯±Ò»ú¹¹¹ÊÕÏ
					26:·ÀµÁÄ£¿é¹ÊÕÏ
					27:»õµ½¿ØÖÆÆ÷¹ÊÕÏ
					28:»õµÀ´óÃÅ¹ÊÕ
					51:×´Ì¬²éÑ¯³¬Ê±
					52:³ö»õÃüÁî·¢ËÍÊ§°Ü
					53:³ö»õ½á¹û²éÑ¯³¬Ê±
					99:ÆäËû¹ÊÕÏ
					255(0xFF):Í¨ÐÅÊ§°Ü
*******************************************************************************/

uchar LifterVendoutTask(struct COMMTASK xdata*liftPtr)
{
	uchar res = 0,rcx = 5;
	uchar rowNo = 0,columnNo = 0,binNum,errNo,state;
	unsigned short takeTimeOut = 10,vendoutTimeout;
	binNum = liftPtr->Param[0];
	rowNo = liftPtr->Param[1] / 10 ;
	columnNo = liftPtr->Param[1] % 10;
	
	//¿ªÊ¼³ö»õ //²éÑ¯×´Ì¬ÊÇ·ñÕý³££¬Õý³£³ö»õ
	rcx = 10;
	while(rcx--)
	{
		state = LifterCheckStateTask(binNum,&errNo); 	
		if(state == 0)//Õý³£¾ÍÌø¿ª
			break;
		else if(state == 1)//»úÆ÷Ã¦
		{
			if(errNo & 0xFF)//ÓÐ¹ÊÕÏ
			{
				if(errNo == 1) return 21;
				if(errNo == 2) return 22;
				if(errNo == 3) return 23;
				if(errNo == 4) return 24;
				if(errNo == 5) return 25;
				if(errNo == 6) return 26;
				if(errNo == 7) return 27;
				if(errNo == 8) return 28;
				return 99;
					
			}
		}
		else if(state == 0xFF)
		{
			rcx = 1;
		}
		lifterDelay(2000);
	}

	if(state != 0)//³¬Ê±²éÑ¯Éý½µ»úÈÔÈ»²»Õý³£
	{
		if(state == 0xFF)
			return 0xFF;
		else
			return 51;
	}
		
	while(1)
	{
		//½øÐÐ³ö»õ²Ù×÷
		res = LifterSendVendoutTask(binNum,rowNo,columnNo);
		if(res == 0 || res == 0xFF)
			return 52;
		
		lifterDelay(500);
		//5000 * 10  = 50000 
		vendoutTimeout = 100 +(8 - rowNo) * 10;//¼ì²â³ö»õ½á¹û³¬Ê±
		while(vendoutTimeout--)
		{	
			state = LifterCheckRstTask(binNum,&errNo);//¼ì²â³ö»õ½á¹û
			if(state == 0x01)//³ö»õÍê³É
			{
				break;
			}
			lifterDelay(1000);
		}
		
		if(vendoutTimeout)
		{
			if(errNo != 0)//³ö»õÊ§°Ü
			{
				if((errNo == 5) && takeTimeOut)//»õÎ´È¡×ß
				{
					lifterDelay(2000);
					takeTimeOut--;
					continue;
				}
				//return ((stateH > 7) ? (stateH + 10) : 99);
				return (errNo + 10);
			}	
			else//³ö»õ³É¹¦
				return 1;
		}
		else										   	
			return 53;		
	}

}

uchar LifterVendout(uchar binNum,uchar logicNo)
{
	uchar timeout = 0,comOK = 0,index = 0;

	memset( &lifterTask, 0, sizeof( struct COMMTASK ) );
	Channel.CommState	= COMM_BUZY;
	lifterTask.Id		= ID_CHANNEL;
	lifterTask.Cmd		= VMC_VENDING_REQ;
	lifterTask.Priority = PRIORITY_NORMAL;
	lifterTask.Param[0] = binNum;
	lifterTask.Param[1] = logicNo;
	ZhkSchedulerAddTask( &lifterTask );
	
	while(! ( timeout || comOK ))
	{
		WaitForWork( 100, NULL );
		timeout = TestDeviceTimeOut( &Channel );
		comOK = TestDeviceCommOK( &Channel );	
	}

	if(timeout)
		return 0xFF;
	if(comOK)
		return Channel.State;	
	return 0xFF;
}








