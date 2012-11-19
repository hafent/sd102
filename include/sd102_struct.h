/* File encode:	 GB2312
   filename:	sd102_struct.h
山东102规约 结构体定义头文件
未注明的章节号内容参考资料1,其他章节号见其具体协议编号.
其他说明:
	数据格式按照 GB/T18657.4 idt IEC60870-5-4
	*_t 为无符号整型数据(8位/16位等)
	首字母大写的单词为结构体类型/联合体 如  It Vsq
参考资料:
	1.山东电网 DL/T 719-2000 电力系统电能累计量传输配套标准实施规范(2011)
	 	 ref IEC60870-5-102
	2.http://en.wikipedia.org/wiki/IEC_60870-5
*/
#ifndef SD102_STRUCT_H
#define SD102_STRUCT_H
#include "typedefine.h"
//包含各种情形的分类编码
#include "sd102_funcode.h" //功能码
#include "sd102_typ.h" //类型码
#include "sd102_cot.h" //传输原因分类
#include "sd102_start_char.h" //起始码
#pragma pack(1) //本文件内所有结构体压缩!按照规约所规定紧凑分布
// 事件
#define SPQ_ERTU_RST_GX		1
#define SPQ_ERTU_CLOSE_GX	1//系统掉电
#define SPQ_SYN_TIME_GX 	2//对时
#define SPQ_PARA_MOD_GX		1//参数修改
#define SPQ_COMM_ERR_GX 	48//表计通讯故障
#define SPQ_COMM_SUCCESS 	49//表计通讯成功
#define SPQ_LOW_VA_GX  		53//
#define SPQ_LOW_VB_GX   	54//
#define SPQ_LOW_VC_GX   	55//
#define SPA_ERTU_RST_GX		1
#define SPA_ERTU_CLOSE_GX 	3 //系统掉电
#define SPA_SYN_TIME_GX 	7//对时
#define SPA_PARA_MOD_GX		8//参数修改
//6.1传输帧格式
typedef u16 link_addr_t;//链路地址
//7.2.2 可变结构限定词（VARIABLE OF STRUCTURE QUALIFIER）
union  Vsq {
	u8 val;
	struct {
		u8 n:7;
		u8 sq:1;//n的寻址方式:0-顺序 1-单独.
	};
};
//7.2.3 传送原因 (Cause Of Transmission)
union Cot {
	u8 val;
	struct {
		u8 cause:6;
		u8 pn:1;
		u8 t:1;
	};
};
//7.2.4 电能累计量数据终端设备地址(应用服务单元公共地址)ASDU_addr
typedef u16 asdu_addr_t; //协议一小尾端方式存储,同是小尾端不需要转换
//7.2.5 记录地址(RAD)	Record Address
typedef u8 rad_t;
//7.2.6 信息体地址(IOA) Information Object Address
typedef u8 ioa_t;
//7.2.7.1 序列号和数据状态 字节 //sd102中全改为状态位,
union Seq_number {
	u8 val;
	struct {
		u8 sn:5;//序列号
		u8 cy:1;//carry
		u8 ca:1;//计数器被调整(CA＝counter was adjusted)
		u8 iv:1;//无效(IV＝invalid)
	};
};
//7.2.7.1 序列号和数据状态 字节 //替代上面的
union Data_status {
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
struct	It {
	u32 dat;
	union Data_status d_status;
};
//7.2.7.2 时间信息 a(Time information a,分至年)
struct Ta {
	u8 min:6;
	u8 tis:1;//费率信息开关 0-off tariff information switch
	u8 iv:1;//时间陈述无效标志位 invalid
	//
	u8 hour:5;
	u8 res1:2;//备用 reserve 1 <0>
	u8 su:1;//夏令时 summer time
	//
	u8 day:5;//几号 1-31
	u8 week:3;//星期几 1-7
	//
	u8 month:4;
	u8 eti:2; //能量费率信息(ETI＝energy tariff information)
	u8 pti:2;//功率费率信息(PTI＝power tariff information)
	//
	u8 year:7;
	u8 res2:1;//备用2(RES2＝reserve 2) <0>
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
struct Dos {
	u8 month:4;//月 <1..12>
	u8 year:4;//年 <	0..9>
};
//7.2.7.5 制造厂编码 Manufacturer code
typedef u8 factcode_t; //<0..255>
//7.2.7.6 产品编码 product code ;BS32 bit strings 32bits
typedef u32 productcode_bs;
//7.2.7.7 带地址和限定词的单点信息 single-point information
struct Spinfo {
	u8 spa;//single-point address
	u8 spi:1;//
	u8 spq:7;//
};
//7.2.7.8 电能累计量数据保护的校核 TODO:再详细查阅规约,查询对那几部分CS
typedef u8 signature_t;
//7.2.7.9 初始化原因 Cause of initialization
union Coi {
	u8 val;
	struct {
		u8 coi:7;//初始化原因
		u8 bs:1;
	};
};
//7.2.7.10 复费率电能累计量 Multi-rate  Integrated total
struct Multi_it {
	u32 total;
	u32 rate1;//费率1
	u32 rate2;
	u32 rate3;
	u32 rate4;
	u32 rate5;//保留为0
	union Data_status d_status;
};
//7.2.7.11 月结算复费率电能累计量 Monthly balance sheet multi-rate  IT
struct Month_mit {
	u32 total;
	u32 rate1;//费率1
	u32 rate2;
	u32 rate3;
	u32 rate4;
	u32 rate5;//保留为0
	union Data_status d_status;
};
//7.2.7.12 月总最大需量及发生时间 Monthly total demand and time
struct Month_maxdemand {
	u32 total_maxdemand;//月总最大需量值
	struct Ta occur_time;
};
//7.2.7.13 遥测量 Remote measurement
struct	Remote_measure {
	u32 dat;
	union Data_status d_status;
};
//7.2.7.14 表计谐波数据 Meter harmonic data
struct Harmonic_data {
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
	struct Spinfo sp;
	struct Tb tb;
};
//7.3.1.2 电能累计量 information object
struct M_IT_TA_2_InfoObj {
	ioa_t ioa;//
	struct It it_power;
	signature_t cs;//电能累计量数据保护的校核
};
//7.3.1.3 复费率记帐(计费)电能累计量 information object
struct M_IT_TA_B_2_InfoObj {
	ioa_t ioa;//
	struct Multi_it mit;
	signature_t cs;//电能累计量数据保护的校核
};
//7.3.1.4遥测历史量 History of remote measurement -information object
struct M_YC_TA_2_InfoObj {
	ioa_t ioa;//
	struct Remote_measure rm;
};
//7.3.1.5 月最大需量及发生时间 顺序信息体(SQ=0) Information Object
struct M_MMD_TA_InfoObj {
	ioa_t ioa;//
	struct Month_maxdemand mmd;
};
//7.3.1.6 月结算复费率电能累计量 Information Object
struct M_MMIT_InfoObj {
	ioa_t ioa;//
	struct Month_mit mmit;
};
//7.3.1.7 表计谐波数据 顺序信息体(SQ=0) information object
struct M_THD_InfoObj {
	ioa_t ioa;//
	struct Harmonic_data mhd;
};
//7.3.2 在监视方向上的系统信息的应用服务数据单元
//7.3.2.1 初始化结束
struct M_EI_NA_2_InfoObj {
	ioa_t ioa;//
	union Coi coi;
};
//7.3.2.2 电能累计量数据终端设备的制造厂和产品的规范
struct P_MP_NA_2_iObj {
	struct Dos dos;
	factcode_t fcode;
	productcode_bs pcode;
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
struct C_CI_NA_D_2_iObj {
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

//C3.1 控制域 主站->终端(下行)
union Ctrl_down {
	u8 val;
	struct {
		u8 funcode:4;
		u8 fcv:1;
		u8 fcb:1;
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
		u8 acd:1;
		u8 prm:1;//0 终端向主站
		u8 res:1; //本规约备用
	};
};
/* 变长帧结构:
|                                    |     含义    |  长度 |  小节  |
+-------+------------------------------------------+------+--------+
|       |                                 0x68     |1 byte|        |
| Frame |                                 length    |1 byte|4 Bytes |
| head  |                             length(copy)  |1 byte|        |
|       |                                 0x68     |1 byte|        |
+-------+---------+--------------------------------+------+--------+
|       |         |                     Ctrl Field |1 byte|        |
|       |LPDU head|                    Link addr lo|1 byte|3 Bytes |
|       |  (LPCI) |                    Link addr hi|1 byte|        |
|       +---------+------+-----------+-------------+------+--------+
|       |         |      |           |  Type Id    |1 byte|        |
|       |         |      | ASDU head |    VSQ      |1 byte|        |
|       |         |      |  (APCI)   |    COT      |1 byte|6 Bytes |
|       |         |      |           | ASDU addr lo|1 byte|        |
| LPDU  |         |      |           | ASDU addr hi|1 byte|        |
|       |         |      |           |    RAD      |1 byte|        |
|       |LPDU body| ASDU +-----------+-------------+------+--------+
|       |         |(APDU)|           |             |      |        |
|       |         |      |           | Information |  ?   |        |
|       |         |      |           | Object(Obj1)|      |        |
|       |         |      | ASDU body | (necessary) |      |Unknown |
|       |         |      |           Z-------------Z      |        |
|       |         |      |           |     ...     |      |        |
|       |         |      |           Z-------------Z      |        |
|       |         |      |           |    Obj N    |      |        |
|       |         |      |           | (Optional)  |      |        |
+-------+---------+------+-----------+-------------+------+--------+
| Frame |                                  CS      |1 byte|2 Bytes |
| Tail  |                                 0x16     |1 byte|        |
+-------+------------------------------------------+------+--------+
*/
// C1.1 可变帧长帧 - 帧头 Frame head
struct Frame_head {
	u8 start_byte1;
	u8 len1;
	u8 len2;
	u8 start_byte2;
};
// 7.1 链路规约数据单元 Link Protocol Data Unit
struct Lpdu_head {
	union { //控制域
		union Ctrl_down c_down;
		union Ctrl_up c_up;
	};
	link_addr_t link_addr;
};
// 7.1 数据单元标识(应用服务数据单元头),Application Service Data Unit(ASDU)
struct Asdu_head { //ASDU头即 数据单元标识 Data unit identifier
	typ_t typ;
	union  Vsq vaq;
	union Cot cot;
	asdu_addr_t asdu_addr;//
	rad_t rad;
};
//Information Object 信息体,按照不通类型不通.
// C1.1  变长/定长帧尾 Frame Tail
struct Frame_tail {
	u8 cs;
	u8 end_byte;
};

/*C1.2 固定帧长帧 - 帧体
+------------+-----------------+-------+
| Frame head | 0x10(Start byte)|1 byte |
+------------+-----------------+-------+
|            | Ctrl Field      |1 byte |
| LPDU(head) | Link addr lo    |1 byte |
|            | Link addr hi    |1 byte |
+------------+-----------------+-------+
| Frame Tail |     CS          |1 byte |
|            | 0x16(End byte)  |1 byte |
+------------+-----------------+-------+
*/
struct Short_frame {
	u8 start_byte; //开始字节
	union { //控制域
		union Ctrl_down c_down;
		union Ctrl_up c_up;
	};
	link_addr_t link_addr;
	struct Frame_tail farme_tail;
};


/*  以下 定义一些固定的帧长的功能的帧 主要是下行帧结构,
 * 上行的很多长度都是由(读取数据)等参数确定的,比如读取地址a到地址b,
 */
//回复 读取时间帧 C_TI_NA_2 -> M_TI_TA_2 (上行)
struct stFrame_M_TI_TA_2{
	struct Frame_head farme_head;
	struct Lpdu_head lpdu_head;
	struct Asdu_head asdu_head;
	struct Tb tb; //信息一仅包含一个信息元素:Tb
	struct Frame_tail farme_tail;
};
// C_CI_NR_2 读电量
struct stFrame_C_CI_NR_2{
	struct Frame_head farme_head;
	struct Lpdu_head lpdu_head;
	struct Asdu_head asdu_head;
	ioa_t start_addr; //开始地址
	ioa_t end_addr;//结束地址
	struct Ta start_time;//开始时间
	struct Ta end_time;//结束时间
	struct Frame_tail farme_tail;
};
// C_CI_NS_2 读取周期复位的电能累积量
struct stFrame_C_CI_NS_2{
	struct Frame_head farme_head;
	struct Lpdu_head lpdu_head;
	struct Asdu_head asdu_head;
	ioa_t start_addr;
	ioa_t end_addr;
	struct Ta start_time;
	struct Ta end_time;
	struct Frame_tail farme_tail;
};
// C_RD_NA_2 读制造厂和产品的规范
struct stFrame_C_RD_NA_2{
	struct Frame_head farme_head;
	struct Lpdu_head lpdu_head;
	struct Asdu_head asdu_head;
	struct Frame_tail farme_tail;
};

// C_YC_TA_2 读遥测量
struct stFrame_C_YC_TA_2{
	struct Frame_head farme_head;
	struct Lpdu_head lpdu_head;
	struct Asdu_head asdu_head;
	ioa_t start_addr;
	ioa_t end_addr;
	struct Ta start_time;
	struct Ta end_time;
	struct Frame_tail farme_tail;
};

// C_XL_NB_2 读最大需量;
struct stFrame_C_XL_NB_2{
	struct Frame_head farme_head;
	struct Lpdu_head lpdu_head;
	struct Asdu_head asdu_head;
	ioa_t start_addr;
	ioa_t end_addr;
	struct Ta start_time;
	struct Ta end_time;
	struct Frame_tail farme_tail;
};
//读一个选定时间范围和选定地址范围的表计谐波数据
struct stFrame_C_CI_NA_C_2{
	struct Frame_head farme_head;
	struct Lpdu_head lpdu_head;
	struct Asdu_head asdu_head;
	ioa_t start_addr;
	ioa_t end_addr;
	struct Ta start_time;
	struct Ta end_time;
	struct Frame_tail farme_tail;
};


#endif // SD102_STRUCT_H
