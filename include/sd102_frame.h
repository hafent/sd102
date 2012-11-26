/*
 * sd_102_frame.h
 * 	定义sd102各种帧的信息体和帧.长帧(变长帧,0x68开始的帧)
 * 	实现sd102 7.3 特定的应用服务数据单元的定义和表示
 * 固定帧长的帧 一般以:
 * 	struct <标识>_iObj 		(帧格式包含信息体,则定义)
 * 	struct stFrame_帧<标识>		(信息体个数固定,则定义)
 * 其中:
 * 	1. <标识>时形如 M_SP_TA_2 的字符,在"sd_102_typ.h"中定义
 * 	2. _iObj 为 information object (信息体)之缩写
 * 	3. stFrame: st代表结构体,Frame表示这是一个帧的结构.
 * 	4. 若帧长不固定,如包含 信息体1~信息体n 则不定义.
 * 	   在需要时参照给定的结构,结合信息体个数,定义成局部变量
 * 按照规约分类,分成3类:
 * 	1. 监视方向上的**过程信息**的应用服务数据单元 (数据返回帧)
 * 	2. 监视方向上的**系统信息**的应用服务数据单元 (信息返回帧)
 * 	3. 控制方向上的**系统信息**的应用服务数据单元 (读信息/数据帧)
 *  1类帧长度大多不定长,需根据请求计算长度
 *  2类和3类均按结构,长度固定.
 *  Created on: 2012-11-20
 *      Author: lee
 */

#ifndef SD102_FRAME_H_
#define SD102_FRAME_H_
#pragma pack(1)
#include "sd102_struct.h" //定义了信息体元素,如Tb中的秒,分等
#include "typedefine.h"
#include  "sd102_typ.h" //标识 TYP (type)
#define VARIABLE_LENGTH_FRAME 1 //不固定长度的帧,通常时应为包含n个信息体
#define INFO_OBJ_NUM 1 // 信息体数量,按照实际情况在定义帧时重新设置!

// ****服务数据单元的定义和表示 APDU(Application Service Data Unit)*******
// 7.3.1 ************* 在监视方向上的**过程信息**的应用服务数据单元 *************
// 7.3.1.1 M_SP_TA_2  带时标的单点信息(Single-Point)
struct M_SP_TA_2_iObj {
	struct Spinfo sp;
	struct Tb tb;
};
#if VARIABLE_LENGTH_FRAME
struct stFrame_M_SP_TA_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct M_SP_TA_2_iObj  obj[INFO_OBJ_NUM];
	struct Frame_tail farme_tail;
};
#endif
// 7.3.1.2 M_IT_TA_2 ~ M_IT_TM_2 电能累计量 information object (X取 A~M)
struct M_IT_TX_2_iObj {
	ioa_t ioa;//
	struct It it_power;
	signature_t cs;//电能累计量数据保护的校核
};
#if VARIABLE_LENGTH_FRAME
struct stFrame_M_IT_TX_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct M_IT_TX_2_iObj  obj[INFO_OBJ_NUM];
	struct Ta ta;
	struct Frame_tail farme_tail;
};
#endif
// 7.3.1.3 复费率记帐(计费)电能累计量 information object
struct M_IT_TA_B_2_iObj {
	ioa_t ioa;//
	struct Multi_it mit;
	signature_t cs;//电能累计量数据保护的校核
};
#if VARIABLE_LENGTH_FRAME
struct stFrame_M_IT_TA_B_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct M_IT_TA_B_2_iObj obj[INFO_OBJ_NUM];
	struct Ta ta;//应用服务数据单元公共时标
	struct Frame_tail farme_tail;
};
#endif
// 7.3.1.4 遥测历史量 History of remote measurement -information object
struct M_YC_TA_2_InfoObj {
	ioa_t ioa;//
	struct Remote_measure rm;
};
#if VARIABLE_LENGTH_FRAME
struct stFrame_M_YC_TA_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct M_YC_TA_2_InfoObj  obj[INFO_OBJ_NUM];
	struct Ta ta;//应用服务数据单元公共时标
	struct Frame_tail farme_tail;
};
#endif
//7.3.1.5 月最大需量及发生时间 顺序信息体(SQ=0) Information Object
struct M_XL_TA_2_iObj {
	ioa_t ioa;//
	struct Month_maxdemand mmd;
};
#if VARIABLE_LENGTH_FRAME
struct stFrame_M_XL_TA_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct M_XL_TA_2_iObj obj[INFO_OBJ_NUM];
	struct Ta ta;//应用服务数据单元公共时标
	struct Frame_tail farme_tail;
};
#endif
//7.3.1.6 月结算复费率电能累计量 Information Object
struct M_IT_TA_C_2_iObj {
	ioa_t ioa;//
	struct Month_mit mmit;
	signature_t cs;//电能累计量数据保护的校核
};
#if VARIABLE_LENGTH_FRAME
struct stFrame_M_IT_TA_C_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct M_IT_TA_C_2_iObj obj[INFO_OBJ_NUM];
	struct Ta ta;//应用服务数据单元公共时标
	struct Frame_tail farme_tail;
};
#endif
//7.3.1.7 表计谐波数据 顺序信息体(SQ=0) information object
struct M_THD_iObj {
	ioa_t ioa;//
	struct Harmonic_data mhd;
};
// 7.3.2 ************* 在监视方向上的**系统信息**的应用服务数据单元 *************
// 7.3.2.1 M_EI_NA_2 初始化结束
struct M_EI_NA_2_iObj {
	ioa_t ioa;//
	union Coi coi;
};
struct stFrame_M_EI_NA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct M_EI_NA_2_iObj obj;
	struct Frame_tail farme_tail;
};
// 7.3.2.2 P_MP NA_2 电能累计量数据终端设备的制造厂和产品的规范
struct P_MP_NA_2_iObj {
	struct Dos dos;
	factcode_t fcode;
	productcode_bs pcode;
};
struct stFrame_P_MP_NA_2{
	struct Frame_head farme_head;
	struct Udat_head udat_head;
	struct Duid duid;
	struct P_MP_NA_2_iObj obj;
	struct Frame_tail farme_tail;
};
// 7.3.2.3 M_TI_TA_2 电能累计量数据终端设备目前的系统时间
struct M_TI_TA_2_iObj {
	struct Tb t;
};
struct stFrame_M_TI_TA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct M_TI_TA_2_iObj obj;
	struct Frame_tail farme_tail;
};
// C10.2 在监视方向的确认帧
struct stFrame_M_SYN_TA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Tb tb;
	struct Frame_tail farme_tail;
};
// 7.3.3 ************ 在控制方向上的**系统信息**的应用服务数据单元 ***************
// 7.3.3.1 C_RD NA_2 读制造厂和产品的规范
struct stFrame_C_RD_NA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Frame_tail farme_tail;
};
// 7.3.3.2 读带时标的单点信息记录
struct stFrame_C_SP_NA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Frame_tail farme_tail;
};
// 7.3.3.3 C_SP_NB_2 读选定时间范围的带时标的单点信息记录
struct C_SP_NB_2_iObj {
	struct Ta starttime;
	struct Ta endtime;
};
struct stFrame_C_SP_NB_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct C_SP_NB_2_iObj obj;
	struct Frame_tail farme_tail;
};
// 7.3.3.4 C_TI_NA_2 读电能累计量数据终端设备的目前的系统时间
struct stFrame_C_TI_NA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	//不需要信息体
	struct Frame_tail farme_tail;
};
// 7.3.3.5 (四种 CI 读取,结构一样) C_CI_NR_2  C_CI_NS_2 C_CI_NA_B_2 C_CI_NA_C_2
// 信息体结构(四种都一样) 共用
struct C_CI_XX_2_iObj {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
// 	C_CI_NR_2 读一个选定的时间范围和一个选定的地址范围的记账(计费)电能累计量
struct stFrame_C_CI_NR_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct C_CI_XX_2_iObj obj;
	struct Frame_tail farme_tail;
};
// 	C_CI_NS_2 读一个选定的时间范围和一个选定的地址范围的周期地复位的记账(计费)电能累计量
struct stFrame_C_CI_NS_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct C_CI_XX_2_iObj obj;
	struct Frame_tail farme_tail;
};
// 	C_CI_NA_B_2 读一个选定的时间范围和一个选定的地址范围的复费率记帐(计费)电能累计量
struct stFrame_C_CI_NA_B_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct C_CI_XX_2_iObj obj;
	struct Frame_tail farme_tail;
};
// 	C_CI_NA_C_2 读一个选定的时间范围和一个选定的地址范围的月结算复费率电能累计量
struct stFrame_C_CI_NA_C_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct C_CI_XX_2_iObj obj;
	struct Frame_tail farme_tail;
};
// 7.3.3.6 C_YC_TA_2 读一个选定时间范围和选定地址范围的遥测(YC=遥测)量;
struct C_YC_TA_2_iObj {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
struct stFrame_C_YC_TA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid asdu_head;
	struct C_YC_TA_2_iObj obj;
	struct Frame_tail farme_tail;
};
// 7.3.3.7 C_XL_NB_2 读一个选定时间范围和选定地址范围的月最大需量(XL)
struct C_XL_NB_2_iObj {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
struct stFrame_C_XL_NB_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid asdu_head;
	struct C_XL_NB_2_iObj obj;
	struct Frame_tail farme_tail;
};
// 7.3.3.8 C_CI_NA_D_2 读一个选定时间范围和选定地址范围的表计谐波数据
//	[规约错误写成C_CI_NA_C_2,注意!]
struct C_CI_NA_D_2_iObj {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
struct stFrame_C_CI_NA_D_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid asdu_head;
	struct C_CI_NA_D_2_iObj obj;
	struct Frame_tail farme_tail;
};
// C10.1 在监视方向的确认帧
struct stFrame_C_SYN_TA_2{
	struct Frame_head farme_head;
	struct Udat_head lpdu_head;
	struct Duid duid;
	struct Tb tb;
	struct Frame_tail farme_tail;
};


#endif /* SD_102_FRAME_H_ */
