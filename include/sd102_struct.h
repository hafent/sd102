/*山东102规约 结构体定义头文件
 未注明的章节号 参考:山东电网 DL/T 719-2000
			电力系统电能累计量
			传输配套标准实施规范(2011)
			引用标准IEC60870-5-102
 其他章节号见其具体协议编号
 数据格式按照 GBT18657.4 dit IEC60870-5-4
*/
#ifndef SD102_STRUCT_H
#define SD102_STRUCT_H
#include "typedefine.h"
#pragma pack(1) //本文件内所有结构体压缩!按照规约所规定紧凑分布
// 7.2.1.1 类型标识域值的语义的定义
typedef u8 typ_t;
// 表4 类型标识的语义-在监视方向上的过程信息
const typ_t M_UNUSED=0;// 未用
const typ_t M_SP_TA_2=1;//带时标的单点信息
const typ_t M_IT_TA_2=2;//记账(计费)电能累计量,每个量为四个八位位组
const typ_t M_IT_TD_2=5;//周期复位记账(计费)电能累计量,每个量为四个八位位组
const typ_t M_IT_TA_B_2=160;//复费率记帐(计费)电能累计量
const typ_t M_YC_TA_2=162;//遥测历史值
const typ_t M_XL_TA_2=163;//最大需量
const typ_t M_IT_TA_C_2=164;//月结算复费率电能累计量
const typ_t M_IT_TA_D_2=165;//表计谐波数据
// 表5 类型标识的语义-在监视方向上的系统信息
const typ_t M_EI_NA_2=70;//初始化结束
const typ_t P_MP_NA_2=71;//电能累计量数据终端设备的制造厂和产品规范
const typ_t M_TI_TA_2=72;//电能累计量数据终端设备的当前系统时间
// 表6 类型标识的语义-在控制方向上的系统信息
const typ_t C_RD_NA_2=100;//读制造厂和产品规范
const typ_t C_SP_NA_2=101;//读带时标的单点信息的记录
const typ_t C_SP_NB_2=102;//读一个所选定时间范围的带时标的单点信息的记录
const typ_t C_TI_NA_2=103;//读电能累计量数据终端设备的当前系统时间
const typ_t C_CI_NR_2=120;//读一个选定的时间范围和一个选定的地址范围的记账(计费)电能累计量
const typ_t C_CI_NS_2=121;//读周期地复位的一个选定的时间范围和一个选定的地址范围的记账(计费)电能累计量
const typ_t C_CI_NA_B_2=170;//读一个选定的时间范围和一个选定的地址范围的复费率记帐(计费)电能累计量
const typ_t C_YC_TA_2=172;//读一个选定时间范围和选定地址范围的遥测量
const typ_t C_CI_NA_C_2=173;//读一个选定的时间范围和一个选定的地址范围的月结算复费率电能累计量
const typ_t C_XL_NB_2=174;//读一个选定的时间范围和一个选定的地址范围的最大需量
const typ_t C_CI_NA_D_2=175;//读一个选定的时间范围和一个选定的地址范围的表计谐波数据
// 事件
#define SPQ_ERTU_RST_GX	1
#define SPQ_ERTU_CLOSE_GX	1 //系统掉电
#define SPQ_SYN_TIME_GX 	2//对时
#define SPQ_PARA_MOD_GX	1//参数修改
#define SPQ_COMM_ERR_GX 48//表计通讯故障
#define SPQ_COMM_SUCCESS 49//表计通讯成功
#define SPQ_LOW_VA_GX  53	//
#define SPQ_LOW_VB_GX   54	//
#define SPQ_LOW_VC_GX   55 	//
#define SPA_ERTU_RST_GX 	1
#define SPA_ERTU_CLOSE_GX 3 //系统掉电
#define SPA_SYN_TIME_GX 	7//对时
#define SPA_PARA_MOD_GX	8//参数修改
//
const u8 START_SHORT_FARME=0x10; //固定长帧
const u8 START_LONG_FARME=0x68; //变长帧
const u8 START_SINGLE_FARME=0xE5; //单字符帧
const u8 C_FC_Reset_communication_unit=0;
const u8 C_Transfer_data=3;
//7.2.2 可变结构限定词（VARIABLE OF STRUCTURE QUALIFIER）
union  vsq_t {
	u8 val;
	struct {
		u8 n:7;
		u8 sq:1;
	};
};
//7.2.3 传送原因 (Cause Of Transmission)
union cot_t {
	u8 val;
	struct {
		u8 cause:6;
		u8 pn:1;
		u8 t:1;
	};
};
//7.2.4 电能累计量数据终端设备地址
typedef u16 rtuaddr;
//7.2.5 记录地址(RAD)	Record address
typedef u8 rad_t;
//7.2.6 信息体地址(IOA) Information Object Address
typedef u8 ioa_t;
//7.2.7.1 序列号和数据状态 字节
union data_status {
	u8 val;
	struct {
		u8 lv:1;//lost v
		u8 phb:1;//断相
		u8 lc:1;//lose current
		u8 wph:1;// wrong phase
		u8 blv:1;//Battery lose voltage
		u8 ct:1;//color timeout
		u8 com_terminal:1;//Communication terminal
		u8 iv:1;//invalid
	};
};
//7.2.7.1 电能累计量 IT (Integrated total)
struct	it {
	u32 dat;
	union data_status d_status;
};
//7.2.7.2 时间信息 a(Time information a,分至年)
struct Ta {
	u8 min:6;
	u8 tis:1;//费率信息开关 0-off tariff information switch
	u8 iv:1;//时间陈述无效标志位 invalid
	//
	u8 hour:5;
	u8 res1:2;//备用 reserve 1
	u8 su:1;//夏令时 summer time
	//
	u8 day:5;//几号 1-31
	u8 week:3;//星期几 1-7
	//
	u8 month:4;
	u8 eti:2; //energy tariff information
	u8 pti:2;//power tariff information
	//
	u8 year:7;
	u8 res2:1;//reserve 2
};
//7.2.7.3 时间信息 b(Time information b,毫秒至年)
struct Tb {
	u16 ms:10;//毫秒 <0..999>
	u16 second:6;
	//
	u8 min:6;
	u8 tis:1;//费率信息开关 0-off tariff information switch
	u8 iv:1;//时间陈述无效标志位 invalid
	//
	u8 hour:5;
	u8 res1:2;//备用 reserve 1
	u8 su:1;//夏令时 summer time
	//
	u8 day:5;//几号 1-31
	u8 week:3;//星期几 1-7
	//
	u8 month:4;// <1..12>
	u8 eti:2; //energy tariff information
	u8 pti:2;//power tariff information
	//
	u8 year:7;
	u8 res2:1;//reserve 2
};
//7.2.7.4 标准的日期(DOS)Date of standard
struct dos_t {
	u8 month:4;//月 <1..12>
	u8 year:4;//年 <	0..9>
};
//7.2.7.5 制造厂编码 Manufactuer code
typedef u8 factcode; //<0..255>
//7.2.7.6 产品编码 product code ;BS32 bit strings 32bits
typedef u32 productcode;
//7.2.7.7 带地址和限定词的单点信息 single-piont information
struct spinfo {
	u8 spa;//single-point addr
	u8 spi:1;//
	u8 spq:7;//
};
//7.2.7.8 电能累计量数据保护的校核 TODO:再详细查阅规约
typedef u8 Signature;
//7.2.7.9 初始化原因 Cause of initialization
union coi_t {
	u8 val;
	struct {
		u8 coi:7;
		u8 bs:1;
	};
};
//7.2.7.10 复费率电能累计量 Multi-rate  Integrated total
struct multi_it {
	u32 total;
	u32 rate1;//费率1
	u32 rate2;
	u32 rate3;
	u32 rate4;
	u32 rate5;//保留为0
	union data_status d_status;
};
//7.2.7.11 月结算复费率电能累计量 Monthly balance sheet multi-rate  IT
struct month_mit {
	u32 total;
	u32 rate1;//费率1
	u32 rate2;
	u32 rate3;
	u32 rate4;
	u32 rate5;//保留为0
	union data_status d_status;
};
//7.2.7.12 月总最大需量及发生时间 Monthly total demand and time
struct month_maxdemand {
	u32 total_maxdemand;//月总最大需量值
	struct Ta occur_time;
};
//7.2.7.13 遥测量 Remote measurement
struct	remote_measure {
	u32 dat;
	union data_status d_status;
};
//7.2.7.14 表计谐波数据 Meter harmonic data
struct mtr_harmonic_data {
	u32  fa_thd ;//forward active Total harmonic power
	u32  ra_thd;//Reverse active total harmonic power
	u32  ta_distortion;//Total active the harmonic power consumption distortion rate
	u32  v_distortion;//Total voltage harmonic distortion
	u32  c_distortion;//Total current harmonic distortion
	u32  res1;//备用1 reserve 1
	u32  res2;//备用2
	u32  res3;//备用3
};
/*特定应用服务数据单元的定义和表示
  APDU(Application Service Data Unit)
  */
//7.3.1 在监视方向上的过程信息的应用服务数据单元
//7.3.1.1 监视(Monitor)方向 带时标的单点信息(Single-Point)
struct M_SP_TA_2_InfoObj {
	struct spinfo sp;
	struct Tb tb;
};
//7.3.1.2 电能累计量 information object
struct M_IT_TA_2_InfoObj {
	ioa_t ioa;//
	struct it it_power;
	Signature cs;//电能累计量数据保护的校核
};
//7.3.1.3 复费率记帐(计费)电能累计量 information object
struct M_IT_TA_B_2_InfoObj {
	ioa_t ioa;//
	struct multi_it mit;
	Signature cs;//电能累计量数据保护的校核
};
//7.3.1.4遥测历史量 History of remote measurement -information object
struct M_YC_TA_2_InfoObj {
	ioa_t ioa;//
	struct remote_measure rm;
};
//7.3.1.5 月最大需量及发生时间 顺序信息体(SQ=0) Information Object
struct M_MMD_TA_InfoObj {
	ioa_t ioa;//
	struct month_maxdemand mmd;
};
//7.3.1.6 月结算复费率电能累计量 Information Object
struct M_MMIT_InfoObj {
	ioa_t ioa;//
	struct month_mit mmit;
};
//7.3.1.7 表计谐波数据 顺序信息体(SQ=0) information object
struct M_THD_InfoObj {
	ioa_t ioa;//
	struct mtr_harmonic_data mhd;
};
//7.3.2 在监视方向上的系统信息的应用服务数据单元
//7.3.2.1 初始化结束
struct M_EI_NA_2_InfoObj {
	ioa_t ioa;//
	union coi_t coi;
};
//7.3.2.2 电能累计量数据终端设备的制造厂和产品的规范
struct P_MP_NA_2_iObj {
	struct dos_t dos;
	factcode fcode;
	productcode pcode;
};
//7.3.2.3  电能累计量数据终端设备目前的系统时间
struct M_TI_TA_2_iObj {
	struct Tb Time;
};
//7.3.3 在控制方向上的系统信息的应用服务数据单元
//7.3.3.1 读制造厂和产品的规范
//struct C_RD_NA_2_iObj{
//	//没有信息体 = =
//};
//7.3.3.2 读带时标的单点信息记录
/*struct C_SP NA_2{
	 //没有信息体
 };*/
//7.3.3.3 读选定时间范围的带时标的单点(SP)信息记录
struct C_SP_NB_2_iObj {
	struct Ta starttime;
	struct Ta endtime;
};
//7.3.3.4 读电能累计量数据终端设备的目前的系统时间
/*struct C_TI_NA_2_iObj{
	//没有信息体
};*/
//7.3.3.5 读一个选定的时间范围和一个选定的地址范围的月结算复费率电能累计量
struct C_CI_NA_C_2_iObj {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
//7.3.3.6 读一个选定时间范围和选定地址范围的遥测(YC)量;
struct C_YC_TA_2_iObj {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
//7.3.3.7 读一个选定时间范围和选定地址范围的月最大需量(XL)
struct C_XL_NB_2_iObj {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
//7.3.3.8 读一个选定时间范围和选定地址范围的表计谐波数据
struct C_CI_NA_C_2_iObj {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
//数据单元标识 6字节
struct data_unit_t {
	typ_t typ;
	union  vsq_t vaq;
	union cot_t cot;
	struct addr_t addr;
};
//标准发行日期
union proctol_date {
	u8 val;
	struct {
		u8 month:4;
		u8 year:4;
	};
};


//电量数据有效字节
union power_date_iv {
	u8 val;
	struct {
		u8 res:7;//保留 取0
		u8 iv:1;
	};
};


//下行 控制域 主站->终端
union ctrl_down_t {
	u8 val;
	struct {
		u8 funcode:4;
		u8 fcv:1;
		u8 fcb:1;
		u8 prm:1;
		u8 dir:1;//非平衡传输,应保留为0
	};
} ;
//上行 控制域 终端->主站
union ctrl_up_t {
	u8 val;
	struct {
		u8 funcode:4;
		u8 dfc:1;
		u8 acd:1;
		u8 prm:1;
		u8 dir:1; //本规约备用
	};
};
//可变帧长帧 - 帧头
struct long_farme_head {
	u8 start_byte1;
	u8 len_hi;
	u8 len_lo;
	u8 start_byte2;
};
//可变帧长帧 - 帧尾
struct long_farme_tail {
	u8 cs;
	u8 end_byte;
};
//固定帧长帧 - 帧体
struct short_farme {
	u8 start_byte; //开始字节
	union { //控制域
		union ctrl_down_t c_down;
		union ctrl_up_t c_up;
	};
	u8 addr1;//地址
	u8 addr2;//地址
	u8 cs;//校验和
	u8 end_byte;//终止字符
};


#endif // SD102_STRUCT_H
