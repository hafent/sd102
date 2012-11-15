#ifndef SD102_START_CHAR_H
#define SD102_START_CHAR_H
#include "typedefine.h"

const u8 START_SINGLE_FARME=0xE5; //单字符帧
const u8 START_SHORT_FARME=0x10; //固定长帧
const u8 START_LONG_FARME=0x68; //变长帧
const u8 END_BYTE=0x16;//帧结束字符

#endif // SD102_START_CHAR_H
