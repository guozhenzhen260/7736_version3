#ifndef _KEY_
#define _KEY_
#ifndef KEY_C
	#define KeySourse extern 
#else
	#define KeySourse
#endif

	// 键值定义
	#define KEY_0   	0x00
	#define KEY_1   	0x01
	#define KEY_2   	0x02
	#define KEY_3   	0x03
	#define KEY_4   	0x04
	#define KEY_5   	0x05
	#define KEY_6   	0x06
	#define KEY_7   	0x07
	#define KEY_8   	0x08
	#define KEY_9   	0x09
	#define KEY_DOT   	0x2E
	#define KEY_UP   	0x48
	#define KEY_DOWN   	0x50
	#define KEY_BACK   	0x18
	#define KEY_SUBMIT   	0x0D
	#define KEY_CANCEL   	0x1B
	#define KEY_NULL   	0xFF

	
	KeySourse bit KeyLock( void );
	// 键盘锁定
	KeySourse bit KeyUnLock( void );
	// 键盘解锁
	KeySourse unsigned char GetKey( void );
	// 读取键值
	KeySourse void KeyboardInit( void );
    //
    KeySourse unsigned char GetKey_M2( void );
	
#endif
