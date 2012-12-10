/** @file sd102_ctDuid.h
 * 数据单元标识符Duid定义
 *  @page duid 数据单元标识符Duid定义
 * 数据单元标识符 duid的常量定义.\n
 * 仅定义常量,不定义结构,结构在element中定义.\n
 * Duid={标识类型(e_typc|e_typm),可变结构体(Vsq),传输原因(Cot),
 * 	终端地址(rtu_addr_t),记录地址(rad_t)}.\n
 * 在 @ref long-frame 中被引用.
 * @code
 +-----------+-------------+------+--------+
 |           |    typ_t    |1 byte|        |
 | ASDU head |    Vsq      |1 byte|        |
 |  (Duid)   |    Cot      |1 byte|6 Bytes |
 |           |rtu_addr_t lo|1 byte|        |
 |           |rtu_addr_t hi|1 byte|        |
 |           |    rad_t    |1 byte|        |
 +-----------+-------------+------+--------+
 |           其他信息体元素的常量定义        |
 +-----------------------------------------+
 @endcode
 * */
#ifndef _SD102_DUID_H_
#define _SD102_DUID_H_
#include "typedefine.h"
#pragma pack(1)
struct stTyp {
	const char *msg;
	const char *msg_CN;
};
#if 0 //在type.c中定义并初始化,未使用,
extern struct stTyp typ_info_c[];
extern struct stTyp typ_info_m[];  //
#endif

// **************** Type ID 常量定义 ***************
// 7.2.1.1 类型标识域值的语义的定义,
/// 表6 类型标识的语义-在控制方向上的系统信息		(下行,主站发送)
enum e_typc {
	C_RD_NA_2 = 100,  ///<读制造厂和产品规范
	C_SP_NA_2 = 101,  ///<读带时标的单点信息的记录
	C_SP_NB_2 = 102,  ///<读一个所选定时间范围的带时标的单点信息的记录
	C_TI_NA_2 = 103,  ///<读电能累计量数据终端设备的当前系统时间
	C_CI_NR_2 = 120,  ///<读一个选定的时间范围和一个选定的地址范围的记账(计费)电能累计量
	C_CI_NS_2 = 121,  ///<读周期地复位的一个选定的时间范围和一个选定的地址范围的记账(计费)电能累计量
	C_SYN_TA_2 = 128,  ///<电能累计量数据终端系统时间同步命令
	C_CI_NA_B_2 = 170,  ///<读一个选定的时间范围和一个选定的地址范围的复费率记帐(计费)电能累计量
	C_YC_TA_2 = 172,  ///<读一个选定时间范围和选定地址范围的遥测量
	C_CI_NA_C_2 = 173,  ///<读一个选定的时间范围和一个选定的地址范围的月结算复费率电能累计量
	C_XL_NB_2 = 174,  ///<读一个选定的时间范围和一个选定的地址范围的最大需量
	C_CI_NA_D_2 = 175  ///<读一个选定的时间范围和一个选定的地址范围的表计谐波数据
};
/// 表4 类型标识的语义-在监视方向上的过程信息		(上行,终端回复)
enum e_typm {
	M_UNUSED = 0,  ///< 未用
	M_SP_TA_2 = 1,  ///<带时标的单点信息
	M_IT_TA_2 = 2,  ///<记账(计费)电能累计量,每个量为四个八位位组
	M_IT_TD_2 = 5,  ///<周期复位记账(计费)电能累计量,每个量为四个八位位组
	M_SYN_TA_2 = 128,  ///<电能累计量数据终端系统时间同步确认帧
	M_IT_TA_B_2 = 160,  ///<复费率记帐(计费)电能累计量
	M_YC_TA_2 = 162,  ///<遥测历史值
	M_XL_TA_2 = 163,  ///<最大需量
	M_IT_TA_C_2 = 164,  ///<月结算复费率电能累计量
	M_IT_TA_D_2 = 165,  ///<表计谐波数据
// 表5 类型标识的语义-在监视方向上的系统信息(也被归为e_typm中)(上行,终端回复)
	M_EI_NA_2 = 70,  ///<初始化结束
	P_MP_NA_2 = 71,  ///<电能累计量数据终端设备的制造厂和产品规范
	M_TI_TA_2 = 72  ///<电能累计量数据终端设备的当前系统时间
};

//**************** VSQ 常量定义 ***************
const int SQ_Similar = 0;  //在同一种类型的一些信息体中寻址一个个别的元素或综合的元素
const int SQ_sequence = 1;  //在一个体中寻址一个顺序的元素

///COT 传输原因
enum e_cot {
	COT_TEST = 1,  ///<主站测试链路,并不执行操作
	COT_CYCLE = 2,  ///<主站周期性轮询
	COT_SPONT = 3,  ///<自发 突发
	COT_INIT = 4,  ///<初始化
	COT_REQUEST = 5,  ///<请求或者被请求
	COT_ACT = 6,  ///<激活
	COT_ACTCON = 7,  ///<激活-确认[回复激活]
	COT_DEACT = 8,  ///<取消激活
	COT_DEATCCON = 9,  ///<取消激活-确认[回复取消激活]
	COT_ACTTREM = 10,  ///<激活终止[自发]
	COT_NO_DAT = 13, ///<没有数据
	COT_NO_ASDU = 14, ///<没有应用程序服务单元(ASDU)
	COT_SYN_TIME = 48  ///<时间同步
};
///传输原因(COT)的确认否认(P/N)位
enum e_cot_pn{
	 COT_PN_ACK = 0,  ///<肯定确认
	 COT_PN_NACK = 1  ///<否定确认
};
///传输原因(COT)的测试(test)位
enum e_cot_t{
	 COT_T_NOT_TEST = 0, ///<非测试,执行
	 COT_T_TEST = 1  ///<测试链路,不执行
};

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
///@todo 这些应该被移动到另外分类 ****  其他信息体元素的常量定义  ************ //
/**电量Tb一些类型*/
enum e_Tb {
	TB_VALID = 0,  ///<时间有效
	TB_INVALID = 1,  ///<时间无效
	TB_STANDARD_TIME = 0,  ///<标准时
	TB_SUMMER_TIME = 1,  ///<夏令时
	TB_RESERVE1 = 0,  ///<保留1
	TB_RESERVE2 = 0,  ///<保留2
};
/**7.2.7.9 初始化原因
 * */
enum e_Coi{
	 COI_LOC_POWON = 0,///,当地电源合上
	 COI_LOC_MANUAL_RESET = 1,///<当地手动复位
	 COI_RETOME_RESET = 2,///<远方复位
	/**	<3..31> 为此配套标准的标准定义保留（兼容范围）
	 * 	<32..127>：＝为特殊应用保留（专用范围）
	 * */
	 COI_PARAMETER_UNCHANGED = 0,///<当地参数未被改变的初始化
	 COI_PARAMETER_CHANGED = 1///<当地参数改变后的初始化
};




const int SPA_INIT = 1;  // 重新启动
const int SPA_SELF_TEST = 2;  //系统自测试
const int SPA_POWER = 3;  //电源故障
const int SPA_BOTTY =4;//电池失效
const int SPA_DAT_OVER =5;//数据溢出
const int SPA_DAT_LOSE =6;//数据丢失
const int SPA_TIME_MSG =7;//时间报文
const int SPA_MODUL =8;//模块故障
const int SPA_CONTER_ERR =9;//在计数器脉冲输入的脉冲错误
const int SPA_UART_ERR =10;//在串行输入的请求错误
const int SPA_EXT_STATUS =11;//  状态报文，外部
const int SPA_PLUSE_OVER = 12;  //  在计数器脉冲输出溢出
const int SPA_CMP_CONTEROR = 13; //计数器比较
const int SPA_REG_OVER = 14;// 寄存器溢出
const int SPA_PARAM_CHANGE = 15;  //参数改变
const int SPA_EXT_INPUT=16;//外部状态量输入
const int SPA_MANUAL_INPUT = 17;  // 人工输入
const int SPA_WARN_MSG = 18;  //警告报文
const int SPA_ERR_SINGAL = 19;  // 差错信号

const  int SPI_ON =1;//事件发生
const int SPI_OFF =0 ;//事件恢复
//SPQ
const int SPQ_SYS_RESET =0;//系统重启
const int SPQ_CPU_RESET =1;//热启动-重启
const int SPQ_CPU_BOOT =2;//启动-冷启动
const int SPQ_MEM_RESET =17;//启动-冷启动
const int SPQ_MEM_BOOT =18;
const int SPQ_PTR_RESET =33;//printer打印机
const int SPQ_PTR_BOOT =34;
const int SPQ_CU_RESET =49;//通讯单元
const int SPQ_CU_BOOT =50;

#endif // SD102_TYP_H
