
#ifndef _SYSPARAM_
#define _SYSPARAM_
#ifndef SYSPARAM_C
	#define SysParamSourse extern
#else
	#define SysParamSourse
#endif			
	//输入系统参数主控函数
	SysParamSourse bit SysParamManage();		
#endif