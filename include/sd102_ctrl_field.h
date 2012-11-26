/*定义 控制域 结构
 * 包括链路功能码 Link function code
 * 每一位的状态定义
*/
#ifndef SD102_FUNCODE_H
#define SD102_FUNCODE_H
#include "typedefine.h"
#pragma pack(1)
//C3.1 控制域 主站->终端(下行)
union Ctrl_down {
	u8 val;
	struct {
		u8 funcode:4;
		u8 fcv:1;
		u8 fcb:1;//frame count bit
		u8 prm:1;//1 主站向终端
		u8 dir:1;//非平衡传输,应保留为0
	};
} ;
//C3.1 控制域 终端->主站(上行)
union Ctrl_up {
	u8 val;
	struct {
		u8 funcode:4;
		u8 dfc:1;
		u8 acd:1;//access demand //访问需求
		u8 prm:1;//0 终端向主站
		u8 res:1; //本规约备用
	};
};

typedef u8 link_fun_code_t;//u8:4

//S2	发送/确认	由控制站向电能累计量数据终端发送命令等(写指令)
//S3	请求/响应	由控制站向电能累计量数据终端召唤数据或事件(读数据)

// C3.2.5 功能码:控制站向电能累计量数据终端传输的帧中功能码的定义 Ctrl(下行)
// S2: 帧类型 0,3 发送 帧
const link_fun_code_t FN_C_RCU  =0;//复位 Reset Communication Unit
const link_fun_code_t FN_C_TRANS_DAT  =3;//传输数据 Transport date
// 	S3: 帧类型 9,10,11 请求 帧
const link_fun_code_t FN_C_RLK  =9;//召唤链路 call link 请求链路状态
const link_fun_code_t FN_C_PL1 =10;//召唤1级链路 poll class 1 data
const link_fun_code_t FN_C_PL2 =11;//召唤2级链路 poll class 2 date
const link_fun_code_t FN_C_RES1=12;//备用1
const link_fun_code_t FN_C_RES2=13;//备用2

// C3.3.4 功能码:电能累计量数据终端向控制站传输的帧中功能码的定义 Monitor(上行)
// S2: 帧类型: 0 1 确认
const link_fun_code_t FN_M_CON =0;//确认 Confirm
const link_fun_code_t FN_M_LINK_BUSY =1;//链路繁忙 link busy
// 	S3: 帧类型: 8 9 11 响应
const link_fun_code_t FN_M_SEND_DAT  =8;//以数据响应请求帧 send data
const link_fun_code_t FN_M_NO_DAT  =9;//没有所召唤的数据 no data
const link_fun_code_t FN_M_RSP =11;//以链路状态或访问请求回答请求帧 Response

//控制位的其他定义 Ctrl Field other bit define
const u8 CF_ACD_IS_DAT=1;//有数据需要访问
const u8 CF_ACD_NO_DAT=0;//无数据
const u8 CF_PRM_DOWN=1;//下行
const u8 CF_PRM_UP=0;//上行
const u8 CF_RES=0;//保留
const u8 CF_DFC_FULL=1;//数据满
const u8 CF_DFC_NOT_FULL=0;//数据不满,可以采集
#endif // SD102_FUNCODE_H
