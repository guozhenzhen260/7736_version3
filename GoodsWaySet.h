
#ifndef _GOODS_WAY_
#define _GOODS_WAY_
#ifndef GOODS_WAY_C
	#define GoodsWaySetSourse extern
#else
	#define GoodsWaySetSourse
#endif			
	//输入货道参数主控函数
	GoodsWaySetSourse bit GoodsParamManage();
    GoodsWaySetSourse bit AddSalveGoods();//从机添货;by gzz 20110509		
#endif