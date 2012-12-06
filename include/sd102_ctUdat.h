/** @file sd102_ctUdat.h
 * 用户数据头Udat_head结构
 * @page 用户数据头Udat_head结构
 * 用户数据头 结构 和一些数据类型的常量表达式.
 * 用户数据头={控制域,链路地址}
 * @code
+---------+------------------+---------------+------+--------+
|         |                  |  Ctrl Field   |1 byte|        |
|Udat_head|                  | link_addr_t lo|1 byte|3 Bytes |
|         |                  | link_addr_t hi|1 byte|        |
+---------+------+-----------+---------------+------+--------+
@endcode
*/
#ifndef SD102_UDAT_H
#define SD102_UDAT_H
#include "typedefine.h"
#pragma pack(1)
///定义单个功能码及描述结构体
struct stFcn_info {
	const char *msg;
	const char *msg_CN;
};
#if 0 //
extern struct stFcn_info fcn_info_c[];
extern struct stFcn_info fcn_info_m[];
#endif
///C3.1
typedef u8 fcn_t;//u8:4 functions code number
///C3.1 控制域 主站->终端(下行) 控制
union Ctrl_c {
	u8 val;
	struct {
		u8 fcn:4;
		u8 fcv:1;
		u8 fcb:1;///<frame count bit
		u8 prm:1;///<1 主站向终端
		u8 dir:1;///<非平衡传输,应保留为0
	};
} ;
///C3.1 控制域 终端->主站(上行) 监视
union Ctrl_m {
	u8 val;
	struct {
		u8 fcn:4;
		u8 dfc:1;
		u8 acd:1;///<access demand //访问需求
		u8 prm:1;///<0 终端向主站
		u8 res:1; ///<本规约备用
	};
};

//6.1传输帧格式
typedef u16 link_addr_t;///<链路地址
//S2	发送/确认	由控制站向电能累计量数据终端发送命令等(写指令)
//S3	请求/响应	由控制站向电能累计量数据终端召唤数据或事件(读数据)

/// C3.2.5 功能码:控制站向电能累计量数据终端传输的帧中功能码的定义 Ctrl(下行)
enum e_fcn_c{
	// S2: 帧类型 0,3 发送 帧
	FCN_C_RCU=0,///<复位 Reset Communication Unit
	FCN_C_TRANS_DAT=3,///<传输数据 Transport date
	// S3: 帧类型 9,10,11 请求 帧
	FCN_C_RLK=9,///<召唤链路 call link 请求链路状态
	FCN_C_PL1=10,///<召唤1级链路 poll class 1 data
	FCN_C_PL2=11,///<召唤2级链路 poll class 2 date
	FCN_C_RES1=12,///<备用1
	FCN_C_RES2=13///<备用2
};

/// C3.3.4 功能码:电能累计量数据终端向控制站传输的帧中功能码的定义 Monitor(上行)
enum e_fcn_m{
	// S2: 帧类型: 0 1 确认
	FCN_M_CON=0,///<确认 Confirm
	FCN_M_LINK_BUSY=1,///<链路繁忙 link busy
	// S3: 帧类型: 8 9 11 响应
	FCN_M_SEND_DAT=8,///<以数据响应请求帧 send data
	FCN_M_NO_DAT=9,///<没有所召唤的数据 no data
	FCN_M_RSP=11,///<以链路状态或访问请求回答请求帧 Response
};
///控制位的其他定义 Ctrl Field other bit define
const u8 ACD_ACCESS=1;///<有数据需要访问
const u8 ACD_NO_ACCESS=0;///<无数据
const u8 PRM_DOWN=1;///<下行 C
const u8 PRM_UP=0;///<上行 M
const u8 CF_RES=0;///<保留
const u8 DFC_FULL=1;///<数据满
const u8 DFC_NOT_FULL=0;///<数据不满,可以采集

#pragma pack()
#endif // SD102_FUNCODE_H
