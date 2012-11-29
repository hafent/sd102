/*数据单元标识符 duid的常量定义(仅定义常量,不定义结构,结构在element中定义)
 * duid={Type,VSQ,COT,ASDU addr,RAD}
 +-----------+-------------+------+--------+
 |           |  Type Id    |1 byte|        |
 | ASDU head |    VSQ      |1 byte|        |
 |  (DUID)   |    COT      |1 byte|6 Bytes |
 |           | ASDU addr lo|1 byte|        |
 |           | ASDU addr hi|1 byte|        |
 |           |    RAD      |1 byte|        |
 +-----------+-------------+------+--------+
 |	    其他信息体元素的常量定义	   |
 +-----------------------------------------+
 * */
#ifndef _SD102_DUID_H_
#define _SD102_DUID_H_
#include "typedefine.h"
//#include "sd102_stElement.h"
#pragma pack(1)
struct stTyp {
	const char *msg;
	const char *msg_CN;
};
#if 0 //在type.c中定义并初始化,未使用,
extern struct stTyp typ_info_c[];
extern struct stTyp typ_info_m[];  //
#endif
//7.2.1.1 类型标识域值的语义的定义
//typedef u8 typ_t;
// **************** Type ID 常量定义 ***************
// 7.2.1.1 类型标识域值的语义的定义,
// 表6 类型标识的语义-在控制方向上的系统信息		(下行,主站发送)
enum e_typ_t_c {
	TYP_C_RD_NA_2 = 100,  //读制造厂和产品规范
	TYP_C_SP_NA_2 = 101,  //读带时标的单点信息的记录
	TYP_C_SP_NB_2 = 102,  //读一个所选定时间范围的带时标的单点信息的记录
	TYP_C_TI_NA_2 = 103,  //读电能累计量数据终端设备的当前系统时间
	TYP_C_CI_NR_2 = 120,  //读一个选定的时间范围和一个选定的地址范围的记账(计费)电能累计量
	TYP_C_CI_NS_2 = 121,  //读周期地复位的一个选定的时间范围和一个选定的地址范围的记账(计费)电能累计量
	TYP_C_SYN_TA_2 = 128,  //电能累计量数据终端系统时间同步命令
	TYP_C_CI_NA_B_2 = 170,  //读一个选定的时间范围和一个选定的地址范围的复费率记帐(计费)电能累计量
	TYP_C_YC_TA_2 = 172,  //读一个选定时间范围和选定地址范围的遥测量
	TYP_C_CI_NA_C_2 = 173,  //读一个选定的时间范围和一个选定的地址范围的月结算复费率电能累计量
	TYP_C_XL_NB_2 = 174,  //读一个选定的时间范围和一个选定的地址范围的最大需量
	TYP_C_CI_NA_D_2 = 175  //读一个选定的时间范围和一个选定的地址范围的表计谐波数据
};
// 表4 类型标识的语义-在监视方向上的过程信息		(上行,终端回复)
enum e_typ_t_m {
	TYP_M_UNUSED = 0,  // 未用
	TYP_M_SP_TA_2 = 1,  //带时标的单点信息
	TYP_M_IT_TA_2 = 2,  //记账(计费)电能累计量,每个量为四个八位位组
	TYP_M_IT_TD_2 = 5,  //周期复位记账(计费)电能累计量,每个量为四个八位位组
	TYP_M_SYN_TA_2 = 128,  //电能累计量数据终端系统时间同步确认帧
	TYP_M_IT_TA_B_2 = 160,  //复费率记帐(计费)电能累计量
	TYP_M_YC_TA_2 = 162,  //遥测历史值
	TYP_M_XL_TA_2 = 163,  //最大需量
	TYP_M_IT_TA_C_2 = 164,  //月结算复费率电能累计量
	TYP_M_IT_TA_D_2 = 165,  //表计谐波数据
// 表5 类型标识的语义-在监视方向上的系统信息 		(上行,终端回复)
	TYP_M_EI_NA_2 = 70,  //初始化结束
	TYP_P_MP_NA_2 = 71,  //电能累计量数据终端设备的制造厂和产品规范
	TYP_M_TI_TA_2 = 72  //电能累计量数据终端设备的当前系统时间
};

//**************** VSQ 常量定义 ***************
const int SQ_Similar = 0;  //在同一种类型的一些信息体中寻址一个个别的元素或综合的元素
const int SQ_sequence = 1;  //在一个体中寻址一个顺序的元素
//**************** COT 传输原因 常量定义 ***************
//typedef u8 cot_t;
enum cot_e {
	COT_TEST = 1,  //主站测试链路,并不执行操作
	COT_CYCLE = 2,  //主站周期性轮询
	COT_SPONT = 3,  //自发 突发
	COT_INIT = 4,  //初始化
	COT_REQUEST = 5,  //请求或者被请求
	COT_ACT = 6,  //激活
	COT_ACTCON = 7,  //激活-确认[回复激活]
	COT_DEACT = 8,  //取消激活
	COT_DEATCCON = 9,  //取消激活-确认[回复取消激活]
	COT_ACTTREM = 10,  //激活终止[自发]
	COT_NO_DAT = 13,
	COT_NO_ASDU = 14,
	COT_SYN_TIME = 48  //时间同步
};
//PN 和 test位
const int COT_PN_ACK = 0;  //肯定确认
const int COT_PN_NACK = 1;  //否定确认
const int COT_T_NOT_TEST = 0;  //非测试,执行
const int COT_T_TEST = 1;  //测试链路,不执行
//**************** RAD 记录地址 常量定义 ***************
/* RAD
 <0>：＝缺省
 <1>：＝从记账（计费）时段开始的电能累计量的记录地址
 <2.. 10>：＝为将来兼容定义保留
 <11>：＝电能累计量累计时段1的记录地址
 <12>：＝电能累计量累计时段2的记录地址
 <13>：＝电能累计量累计时段3的记录地址
 <14.. 20>：＝为将来兼容定义保留
 <21>：＝电能累计量（日值）累计时段1的记录地址
 <22>：＝电能累计量（日值）累计时段2的记录地址
 <23>：＝电能累计量（日值）累计时段3的记录地址
 <24.. 30>：＝为将来兼容定义保留
 <31>：＝电能累计量（周/旬值）累计时段1的记录地址
 <32>：＝电能累计量（周/旬值）累计时段2的记录地址
 <33>：＝电能累计量（周/旬值）累计时段3的记录地址
 <34.. 40>：＝为将来兼容定义保留
 <41>：＝电能累计量（月值）累计时段1的记录地址
 <42>：＝电能累计量（月值）累计时段2的记录地址
 <43>：＝电能累计量（月值）累计时段3的记录地址
 <44.. 49>：＝为将来兼容定义保留
 <50>：＝最早的单点信息
 <51>：＝单点信息的全部记录
 <52>：＝单点信息记录区段1
 <53>：＝单点信息记录区段2
 <54>：＝单点信息记录区段3
 <55>：＝单点信息记录区段4
 <56..127>：＝为将来兼容定义保留
 <128.. 255>：＝为特殊应用（专用范围） */
const int RAD_DEFAULT = 0;
const int RAD_ALL_SP_INFO = 51;
// ************  其他信息体元素的常量定义  ************ //
/*SPQ Single-Point*/
enum Tb_e {
	TB_VALID = 0,  //时间有效
	TB_INVALID = 1,  //时间无效
	TB_STANDARD_TIME = 0,  //标准时
	TB_SUMMER_TIME = 1,  //夏令时
	TB_RESERVE1 = 0,  //保留1
	TB_RESERVE2 = 0,  //保留2
};
const int COI_LOC_POWON = 0;
const int COI_LOC_MANUAL_RESET = 1;
const int COI_RETOME_RESET = 2;
/*	<3..31> 为此配套标准的标准定义保留（兼容范围）
 * 	<32..127>：＝为特殊应用保留（专用范围）
 * */
const int COI_PARAMETER_UNCHANGED = 0;
const int COI_PARAMETER_CHANGED = 1;

const int SPQ_RST = 1;  // 重新启动
const int SPQ_PARAM_CHANGE = 15;  //参数改变
const int SPQ_MANUAL_INPUT = 17;  // 人工输入
const int SPQ_POWER_FAULT = 3;  //电源故障
const int SPQ_WARN_MSG = 18;  //警告报文
const int SPQ_ERR_SINGAL = 19;  // 差错信号
const int SPQ_TIME_DRIFT = 7;  //时间偏移
const int SPQ_NO_DIFF = 13;  //电能累计量的不允许差额

#endif // SD102_TYP_H
