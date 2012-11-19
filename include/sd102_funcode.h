/*定义链路功能码 Link function code
*/
#ifndef SD102_FUNCODE_H
#define SD102_FUNCODE_H
#include "typedefine.h"

typedef u8 link_fun_code_t;//u8:4

//S2	发送/确认	由控制站向电能累计量数据终端发送命令等(写指令)
//S3	请求/响应	由控制站向电能累计量数据终端召唤数据或事件(读数据)

// C3.2.5 功能码:控制站向电能累计量数据终端传输的帧中功能码的定义 Ctrl(下行)
//帧类型 0,3 发送/确认 帧 S2
const link_fun_code_t FN_C_RCU  =0;//复位 Reset Communication Unit
const link_fun_code_t FN_C_TRANS_DAT  =3;//传输数据 Transport date
//帧类型 9,10,11 请求/响应 帧 S3
const link_fun_code_t FN_C_RLK  =9;//召唤链路 call link 请求链路状态
const link_fun_code_t FN_C_PL1 =10;//召唤1级链路 call class 1 data
const link_fun_code_t FN_C_PL2 =11;//召唤2级链路 call class 2 date
const link_fun_code_t FN_C_RES1=12;//备用1
const link_fun_code_t FN_C_RES2=13;//备用2


// C3.3.4 功能码:电能累计量数据终端向控制站传输的帧中功能码的定义 Monitor(上行)
//帧类型: 0 1 确认 S2
const link_fun_code_t FN_M_CON =0;//确认 Confirm
const link_fun_code_t FN_M_LINK_BUSY =1;//链路繁忙 link busy
//帧类型: 8 9 11 响应 S3
const link_fun_code_t FN_M_SEND_DAT  =8;//以数据响应请求帧 send data
const link_fun_code_t FN_M_NO_DAT  =9;//没有所召唤的数据 no data
const link_fun_code_t FN_M_RSP =11;//以链路状态或访问请求回答请求帧 Response

#endif // SD102_FUNCODE_H
