#ifndef __MAINFLOW_H__
#define __MAINFLOW_H__

#include "global.h"
#define ERR_BILL_COIN 		0xF1      
#define ERR_COIN 			0xF2
#define ERR_HOPPER 			0xF3
#define ERR_COINLESS		0xF4//硬币不足
#define ERR_NOGOODS			0xF5//无货242
#define ERR_GOODSWAYERR		0xF6//货道故障243
#define ERR_GOODSBOARDERR	0xF7//货道板故障243
#define ERR_COLNUMERR		0xF8//无可用货道
#define ERR_SYSPARAMETER	0xF9//系统参数故障
#define ERR_LCD				0xFA//主板与显示板通讯故障
#define ERR_PRICE			0xFB//某货道单价为0

extern void Mission();

#if defined(_DEBUG_FLOW)
#define TRACE_NODE(RET, NO) SearchNode(RET, NO)
#define TRACE_CHECKFLOW()	{ if (0!= ret) goto NODE_FAIL; }
#else
#define TRACE_NODE(RET, NO)
#define TRACE_CHECKFLOW()
#endif

#define BEGIN_FLOW(PARAM_FUNC) \
unsigned char PARAM_FUNC()	\
{	\
	unsigned char data ret = 0;
	
#define NODE(NO)	\
	;	TRACE_CHECKFLOW();\
	NODE_##NO:	while(0){goto NODE_##NO;}\
		TRACE_NODE(ret, NO);	\
		CoreProcessCycle();	\
		ret = 	
			
#define R1(NO)	\
		; {if (1 == ret) goto NODE_##NO;}
		
#define R2(NO)	\
		; {if (2 == ret) goto NODE_##NO;}
		
#define R(NUM, NO)	\
		; {if (NUM == ret) goto NODE_##NO;}
		
#define GOTONODE(NO)	\
		; {goto NODE_##NO;}
			
#define GOTOEND()	\
		; {goto NODE_END;}
			
#define RETURN_FLOW(VAL)	\
		; {ret = VAL; goto NODE_END;}

#define END_FLOW()	\
	;	\
	NODE_END:	\
		return ret;	\
	NODE_FAIL:	\
		return -1;\	
	while(0){goto NODE_END; goto NODE_FAIL;}\
}

#endif