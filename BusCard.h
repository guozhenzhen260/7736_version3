#ifndef __BUSCARD_H__
#define __BUSCARD_H__

#define BC_ERR_NULL     0
#define BC_ERR_PAR      1   //mission parameter error
#define BC_ERR_COM      2   //communication error

#define BC_DEV_ERR_NULL      0
#define BC_DEV_ERR_COM       1
#define BC_DEV_ERR_HARDWARE  2
#define BC_DEV_ERR_BUSY      4


#define BC_MSG_STX       0x02
#define BC_MSG_ETX       0x03

#define BC_TIME_POLL       10     //30, 15,  10
#define BC_TIME_OUT        100    //50, 100, 150, 1000, 100   
#define BC_TIME_OUT_NUM    1      //1,2,1 
#define BC_COM_DELAY       0
#define BC_COM_RETRY       0      //0,1 

#define BC_CMD_INQ         0xFF
#define BC_CMD_TRADE       0xF1

#define BC_ACK_OK          0xA0
#define BC_ACK_MONEY_LOW   0xA1    //indicate: money low
#define BC_ACK_COMERR      0xA2
#define BC_ACK_NOCARD      0xA3
#define BC_ACK_PARERR      0xA4
#define BC_ACK_READERR     0xA5
#define BC_ACK_BALCKCARD   0xA6
#define BC_ACK_UNKOWNCARD  0xA7
#define BC_ACK_BUSY        0xA8    //indicate: busy

#define BC_ACK_NOSIGNED    0xAA    //indicate: error
#define BC_ACK_COMMERR     0xAB    //indicate: error
#define BC_ACK_DEVERR      0xAC    //indicate: error



extern void BCSerialInit( void );
extern unsigned int  BC_CalCRC( unsigned char *ptr, unsigned char len );
extern unsigned char BC_Hex_to_HAscii( unsigned char hex );
extern unsigned char BC_Hex_to_LAscii( unsigned char hex );
extern unsigned char BC_Ascii_to_Hex( unsigned char highBit, unsigned char lowBit );
extern unsigned char BCBusUnpack( void );
extern unsigned char BCBusTxMsg( void );
extern unsigned char BCInqMsgPackSend( void );
extern unsigned char BCTradeMsgPackSend( unsigned int price, unsigned int sn );
extern unsigned char BCMission_Inq( void );
extern unsigned char BCMission_Trade( unsigned int price, unsigned int sn );


#endif  //__BUS_CARD_H__


