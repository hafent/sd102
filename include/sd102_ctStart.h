/**@file sd102_ctStart.h
 * FT1.2帧的两种启动字符 (0x68 0x10)一种单一字符 (0xE5) 一种结束符号(0x16)
 * */
#ifndef SD102_START_CHAR_H
#define SD102_START_CHAR_H
//#include "typedefine.h"
///启动字符
enum e_schar{
	 START_SC_FRAME=0xE5, ///<单字符帧 single char
	 START_SHORT_FRAME=0x10, ///<固定长帧
	 START_LONG_FRAME=0x68 ///<变长帧
};
///结束字符
enum e_echar{
	END_BYTE=0x16///<帧结束字符
};

#endif // SD102_START_CHAR_H
