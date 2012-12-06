/** @file sd102_stFrame.h
 * @page 特定意义信息体和帧
 * 定义通用和特定类型帧结构.
 * 	1.定义了FT1.23种帧格式中两种通用格式:固定长度帧,变帧长帧.
 * 	2.定义了 大量特定含义的 固定帧和变长帧
 * 	实现sd102 7.3 特定的应用服务数据单元的定义和表示
 *
 * 固定帧长的帧 一般以:
 *
 * 	struct iObj_标识 		(帧格式包含信息体,则定义)
 * 	struct stFrame_标识		(信息体个数固定,则定义)
 *
 * 其中:
 *
 * 	1. "标识"时形如 M_SP_TA_2 的字符,在"sd102_ctDuid.h"中定义
 * 	2. _iObj 为 information object (信息体)之缩写
 * 	3. stFrame: st代表结构体,Frame表示这是一个帧的结构.
 * 	4. 若帧长不固定,如包含 信息体1~信息体n 则不定义.
 * 	   在需要时参照给定的结构,结合信息体个数,定义成局部变量
 *
 * 按照规约分类,分成3类:
 *
 * 	1. 监视方向上的**过程信息**的应用服务数据单元 (数据返回帧)
 * 	2. 监视方向上的**系统信息**的应用服务数据单元 (信息返回帧)
 * 	3. 控制方向上的**系统信息**的应用服务数据单元 (读信息/数据帧)
 *
 *	1类帧长度大多不定长,需根据请求计算长度
 *	2类和3类均按结构,长度固定.
 *	Created on: 2012-11-20
 */
#ifndef SD102_FRAME_H_
#define SD102_FRAME_H_
#include "typedefine.h"
#include "sd102_ctStart.h"
#include "sd102_ctDuid.h"
#include "sd102_ctUdat.h"
#include "sd102_stElement.h" //定义了应用信息元素,如Tb,Typ等的结构

#pragma pack(1)
//********************** 第一部分 通用帧类型说明和定义 **************************
#define VARIABLE_LENGTH_FRAME 1 //不固定长度的帧,通常时应为包含n个信息体,调试时置1,使用时置0
#define INFO_OBJ_NUM 1 // 信息体数量,按照实际情况在定义帧时重新设置!
#define __LINK_ADD_LEN 2 //链路地址域长度
/**变长帧格式概括
 * @page  变长帧格式
 * @code
 * 帧结构中部分已知的结构, 帧头 帧尾 各种单元头等
 * 本规约中 LSDU=ASDU=APDU
 * 报文=LPDU=LPCI+LSDU
 * ADSU=DUID+N*Info_Obj+ADSU的公共地址(可选)
 *变长帧结构使用 FT1.2 格式:
|                                    |     含义    |  长度 |  小节  |          |
+-------+------------------------------------------+------+--------+----------+
|       |          START_LONG_FRAME->|    0x68     |1 byte|        |          |
| Frame_head                         |    length   |1 byte|4 Bytes |          |
|       |                            |length(copy) |1 byte|        |          |
|       |                            |    0x68     |1 byte|        | LPCI     |
+-------+----------------------------+-------------+------+--------+ 60870-5-2|
|       | Udat_head                  |Ctrl_c/Ctrl_m|1 byte|        |          |
|       |                            |link_addr_t L|1 byte|3 Bytes |          |
|       |                            |link_addr_t H|1 byte|        |          |
|       +---------+------+-----------+-------------+------+--------+-----\    |
|       |         |      |           |e_typ_t_c(m) |1 byte|        |     |    |
|       |         |      | ASDU head |    Vsq      |1 byte|        |     |    |
|       |         |      |  (Duid)   |    Cot      |1 byte|6 Bytes |     |    |
| User  |         |      |           |rtu_addr_t lo|1 byte|        |     |    |
| Data  |         |      |           |rtu_addr_t hi|1 byte|        |     |    |
|       | u_dat   |      |           |    rad_t    |1 byte|        |     |LPCI|
|       |     body| ASDU +-----------+-------------+------+--------+     |    |
|       |         |(APDU)|           |             |      |        | LSDU|    |
|       |         |      |           | Information |  ?   |        |     |    |
|       |         |      |           | Object(Obj1)|      |        |60870|    |
|       |         |      | ASDU body | (necessary) |      |Unknown |-5-3 |    |
|       |         |      |           Z-------------Z      |        |     |    |
|       |         |      |           |     ...     |      |        |     |    |
|       |         |      |           Z-------------Z      |        |     |    |
|       |         |      |           |    Obj N    |      |        |     |    |
|       |         |      |           | (Optional)  |      |        |     |    |
+-------+---------+------+-----------+-------------+------+--------+-----/    |
| Frame_tail                         |     CS      |1 byte|2 Bytes |   LPCI   |
|       |                            |    0x16     |1 byte|        |          |
+-------+----------------------------+-------------+------+--------+----------+
@endcode
*/
/// C1.1 可变帧长帧 - 帧头 Frame_head
struct Frame_head {
	u8 start_byte1;///< 开始字符
	u8 len1;///< 长度
	u8 len2;///< 长度2
	u8 start_byte2;///< 开始字节(副本)
};
/// IEC60870-5-2 3.2 中的用户数据头部分
struct Udat_head {
	union { //控制域 Crtl field
		union Ctrl_c cf_c;
		union Ctrl_m cf_m;
	};
#if __LINK_ADD_LEN == 2
	link_addr_t link_addr;
#elif __LINK_ADD_LEN == 1
	u8 link_addr;
#elif  __LINK_ADD_LEN == 0
	//没有链路地址
#endif

};
/// 7.1 数据单元标识(应用服务数据单元头),Application Service Data Unit(ASDU)
struct Duid { //ASDU头即 数据单元标识 Data Unit IDentifier
	u8 typ;
	union  Vsq vsq;
	union Cot cot;
	/* 7.2.4 电能累计量数据终端设备的地址从1开始，
	对于信息体每超过一次255个信息点的情况，
	将终端设备地址依次加1。
	终端设备的地址可以和链路地址不一致。*/
	rtu_addr_t rtu_addr;
	/* 7.2.5 记录地址(RAD) */
	rad_t rad;
};
// 填补信息体(Information Object),按照不通类型不通.
/// C1.1  变长/定长帧尾 Frame_tail
struct Frame_tail {
	u8 cs;
	u8 end_byte;
};

/** C1.2 固定帧长帧-帧体
 * @page 定长帧格式
 * @code
 *	+------------+-----------------+-------+
 *	| Frame_head |START_SHORT_FRAME|1 byte |
 *	+------------+-----------------+-------+
 *	|            | Ctrl Field      |1 byte |
 *	| LPDU       | link_addr_t Lo  |1 byte |
 *	|            | link_addr_t Hi  |1 byte |
 *	+------------+-----------------+-------+
 *	| Frame_tail |     cs          |1 byte |
 *	|            | 0x16(End byte)  |1 byte |
 *	+------------+-----------------+-------+
 * @endcode
 */
/// C1.2 固定帧长帧-帧体
struct Short_frame {
	u8 start_byte; //开始字节
	union { //控制域
		union Ctrl_c c_down;
		union Ctrl_m c_up;
	};
	link_addr_t link_addr;
	struct Frame_tail farme_tail;
};


//********************** 第二部分 特定类型的帧定义 **************************
// ****服务数据单元的定义和表示 APDU(Application Service Data Unit)*******
// 7.3.1 ************* 在监视方向上的**过程信息**的应用服务数据单元 *************
/// 7.3.1.1 M_SP_TA_2 带时标的单点(Single-Point)信息体
struct Obj_M_SP_TA_2 {
	struct Spinfo sp;
	struct Tb tb;
};
#if 1 //VARIABLE_LENGTH_FRAME
/// 7.3.1.1 M_SP_TA_2 带时标的单点(Single-Point)信息帧
struct stFrame_M_SP_TA_2{
	struct Frame_head head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct Obj_M_SP_TA_2  obj[INFO_OBJ_NUM];
	struct Frame_tail tail;
};
#endif
/// 7.3.1.2 M_IT_TA_2 ~ M_IT_TM_2 电能累计量信息体
struct Obj_M_IT_TX_2 {
	ioa_t ioa;//
	struct It it_power;
	signature_t cs;//电能累计量数据保护的校核
};
#if VARIABLE_LENGTH_FRAME
/// 7.3.1.2 M_IT_TA_2 ~ M_IT_TM_2 电能累计量帧
struct stFrame_M_IT_TX_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct Obj_M_IT_TX_2  obj[INFO_OBJ_NUM];
	struct Ta ta;
	struct Frame_tail farme_tail;
};
#endif
/// 7.3.1.3 复费率记帐(计费)电能累计量 information object
struct Obj_M_IT_TA_B_2 {
	ioa_t ioa;//
	struct Multi_it mit;
	signature_t cs;//电能累计量数据保护的校核
};
#if VARIABLE_LENGTH_FRAME
/// 7.3.1.3 复费率记帐(计费)电能累计量
struct stFrame_M_IT_TA_B_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct Obj_M_IT_TA_B_2 obj[INFO_OBJ_NUM];
	struct Ta ta;//应用服务数据单元公共时标
	struct Frame_tail farme_tail;
};
#endif
/// 7.3.1.4 遥测历史量 History of remote measurement -information object
struct Obj_M_YC_TA_2 {
	ioa_t ioa;//
	struct Remote_measure rm;
};
#if VARIABLE_LENGTH_FRAME
/// 7.3.1.4 遥测历史量 History of remote measurement
struct stFrame_M_YC_TA_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct Obj_M_YC_TA_2  obj[INFO_OBJ_NUM];
	struct Ta ta;//应用服务数据单元公共时标
	struct Frame_tail farme_tail;
};
#endif
///7.3.1.5 月最大需量及发生时间 顺序信息体(SQ=0) Information Object
struct Obj_M_XL_TA_2 {
	ioa_t ioa;//
	struct Month_maxdemand mmd;
};
#if VARIABLE_LENGTH_FRAME
///7.3.1.5 月最大需量及发生时间 顺序信息体(SQ=0)
struct stFrame_M_XL_TA_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct Obj_M_XL_TA_2 obj[INFO_OBJ_NUM];
	struct Ta ta;//应用服务数据单元公共时标
	struct Frame_tail farme_tail;
};
#endif
///7.3.1.6 月结算复费率电能累计量 Information Object
struct Obj_M_IT_TA_C_2 {
	ioa_t ioa;//
	struct Month_mit mmit;
	signature_t cs;//电能累计量数据保护的校核
};
#if VARIABLE_LENGTH_FRAME
///7.3.1.6 月结算复费率电能累计量 I帧
struct stFrame_M_IT_TA_C_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct Obj_M_IT_TA_C_2 obj[INFO_OBJ_NUM];
	struct Ta ta;//应用服务数据单元公共时标
	struct Frame_tail farme_tail;
};
#endif
///7.3.1.7 表计谐波数据 顺序信息体(SQ=0) information object
struct Obj_M_THD {
	ioa_t ioa;//
	struct Harmonic_data mhd;
};
// 7.3.2 ************* 在监视方向上的**系统信息**的应用服务数据单元 *************
/// 7.3.2.1 M_EI_NA_2 初始化结束
struct Obj_M_EI_NA_2 {
	ioa_t ioa;//
	union Coi coi;
};
/// 7.3.2.1 M_EI_NA_2 初始化结束
struct stFrame_M_EI_NA_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct Obj_M_EI_NA_2 obj;
	struct Frame_tail farme_tail;
};
/// 7.3.2.2 P_MP NA_2 电能累计量数据终端设备的制造厂和产品的规范
struct Obj_P_MP_NA_2 {
	struct Dos dos;
	factcode_t fcode;
	productcode_bs pcode;
};
/// 7.3.2.2 P_MP_NA_2 电能累计量数据终端设备的制造厂和产品的规范
struct stFrame_P_MP_NA_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct Obj_P_MP_NA_2 obj;
	struct Frame_tail farme_tail;
};
/// 7.3.2.3 M_TI_TA_2 电能累计量数据终端设备目前的系统时间
struct Obj_M_TI_TA_2 {
	struct Tb t;
};
/// 7.3.2.3 M_TI_TA_2 电能累计量数据终端设备目前的系统时间
struct stFrame_M_TI_TA_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct Obj_M_TI_TA_2 obj;
	struct Frame_tail farme_tail;
};
/// C10.2 在监视方向的确认帧 电能累计量数据终端系统时间同步确认帧
struct stFrame_M_SYN_TA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Tb tb;
	struct Frame_tail farme_tail;
};
// 7.3.3 ************ 在控制方向上的**系统信息**的应用服务数据单元 ***************
/// 7.3.3.1 C_RD NA_2 读制造厂和产品的规范
struct stFrame_C_RD_NA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Frame_tail farme_tail;
};
/// 7.3.3.2 读带时标的单点信息记录
struct stFrame_C_SP_NA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Frame_tail farme_tail;
};
/// 7.3.3.3 C_SP_NB_2 读选定时间范围的带时标的单点信息记录
struct Obj_C_SP_NB_2 {
	struct Ta starttime;
	struct Ta endtime;
};
/// 7.3.3.3 C_SP_NB_2 读选定时间范围的带时标的单点信息记录
struct stFrame_C_SP_NB_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Obj_C_SP_NB_2 obj;
	struct Frame_tail farme_tail;
};
/// 7.3.3.4 C_TI_NA_2 读电能累计量数据终端设备的目前的系统时间
struct stFrame_C_TI_NA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	//不需要信息体
	struct Frame_tail farme_tail;
};
/** 7.3.3.5 (四种 CI 读取,结构一样) C_CI_NR_2  C_CI_NS_2 C_CI_NA_B_2 C_CI_NA_C_2 .
 * 信息体结构(四种都一样) 共用 */
struct Obj_C_CI_XX_2 {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
/// 	C_CI_NR_2 读一个选定的时间范围和一个选定的地址范围的记账(计费)电能累计量
struct stFrame_C_CI_NR_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Obj_C_CI_XX_2 obj;
	struct Frame_tail farme_tail;
};
/// 	C_CI_NS_2 读一个选定的时间范围和一个选定的地址范围的周期地复位的记账(计费)电能累计量
struct stFrame_C_CI_NS_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Obj_C_CI_XX_2 obj;
	struct Frame_tail farme_tail;
};
/// 	C_CI_NA_B_2 读一个选定的时间范围和一个选定的地址范围的复费率记帐(计费)电能累计量
struct stFrame_C_CI_NA_B_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Obj_C_CI_XX_2 obj;
	struct Frame_tail farme_tail;
};
/// 	C_CI_NA_C_2 读一个选定的时间范围和一个选定的地址范围的月结算复费率电能累计量
struct stFrame_C_CI_NA_C_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Obj_C_CI_XX_2 obj;
	struct Frame_tail farme_tail;
};
/// 7.3.3.6 C_YC_TA_2 读一个选定时间范围和选定地址范围的遥测(YC=遥测)量
struct Obj_C_YC_TA_2 {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
/// 7.3.3.6 C_YC_TA_2 读一个选定时间范围和选定地址范围的遥测(YC=遥测)量
struct stFrame_C_YC_TA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Obj_C_YC_TA_2 obj;
	struct Frame_tail farme_tail;
};
/// 7.3.3.7 C_XL_NB_2 读一个选定时间范围和选定地址范围的月最大需量(XL)
struct Obj_C_XL_NB_2 {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
/// 7.3.3.7 C_XL_NB_2 读一个选定时间范围和选定地址范围的月最大需量(XL)
struct stFrame_C_XL_NB_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Obj_C_XL_NB_2 obj;
	struct Frame_tail farme_tail;
};
/** 7.3.3.8 C_CI_NA_D_2 读一个选定时间范围和选定地址范围的表计谐波数据.
 *	[规约错误写成C_CI_NA_C_2,注意!]
 */
struct Obj_C_CI_NA_D_2 {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
///7.3.3.8 C_CI_NA_D_2 读一个选定时间范围和选定地址范围的表计谐波数据.
struct stFrame_C_CI_NA_D_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Obj_C_CI_NA_D_2 obj;
	struct Frame_tail farme_tail;
};
/// C10.1 在控制方向的系统时间同步命令
struct stFrame_C_SYN_TA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Tb tb;
	struct Frame_tail farme_tail;
};

#pragma pack()
#endif /* SD_102_FRAME_H_ */
