#ifndef _LIFTER_API_H_
#define _LIFTER_API_H_



extern unsigned char LifTerProcess(unsigned char Binnum,unsigned char ChannelNum);
extern bit  ZhkLifterTask( struct COMMTASK xdata* TaskTemp );
extern u_char ChannelLifterTask( u_char ChannelNum, u_char TaskId );


#endif