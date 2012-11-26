/*     filename: sd102_struct.h
	山东102规约 结构体定义头文件
	7.2  IEC 60870-5-4（应用信息元素的定义和编码）的选集
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
#include "sd102_ctrl_field.h" //功能码
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
const int VSQ_SQ_Similar =0;//在同一种类型的一些信息体中寻址一个个别的元素或综合的元素
const int VSQ_SQ_sequence =1;//在一个体中寻址一个顺序的元素
//7.2.3 传送原因 (Cause Of Transmission)
union Cot {
	u8 val;
	struct {
		u8 cause:6;//传送原因＝Cause of transmission
		u8 pn:1;// 应答
		u8 t:1;//test 试验
	};
};

/* 7.2.4 电能累计量数据终端设备地址(应用服务单元公共地址)ASDU_addr rtu addr */
typedef u16 rtu_addr_t; //协议一小尾端方式存储,同是小尾端不需要转换
/* 7.2.5 记录地址(RAD)	Record Address
*/
typedef u8 rad_t;
//7.2.6 信息体地址(IOA) Information Object Address
typedef u8 ioa_t;
/* 7.2.7 信息体元素
   7.2.7.1 序列号和数据状态 字节 //sd102中全改为状态位, */
union Seq_number {
	u8 val;
	struct {
		u8 sn:5;//序列号
		u8 cy:1;//carry
		u8 ca:1;//计数器被调整(CA＝counter was adjusted)
		u8 iv:1;//无效(IV＝invalid)
	};
};
//7.2.7.1 序列号和数据状态 字节 //替代上面的 1表示这种状态,0表示没有这种状态
union Data_status {
	u8 val;
	struct {
		u8 lv:1;//失压 lost v
		u8 phb:1;//断相
		u8 lc:1;//lose current
		u8 wph:1;// wrong phase
		u8 blv:1;//Battery lose voltage
		u8 ct:1;//timeout
		u8 com_terminal:1;//Communication terminal
		u8 iv:1;//invalid
	};
};
//7.2.7.1 电能累计量 IT (Integrated total)
struct	It {
	u32 dat;
	union Data_status d_status;
};
//7.2.7.2 5字节时间信息 a(Time information a,分至年)
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
//7.2.7.3 7字节时间信息 b(Time information b,毫秒至年)
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
const int TB_VALID=0;//时间有效
const int TB_INVALID=1;//时间无效
const int TB_STANDARD_TIME=0;//标准时
const int TB_SUMMER_TIME=1;//夏令时
const int TB_RESERVE1=0;//保留1
const int TB_RESERVE2=0;//保留2
//7.2.7.4 标准的日期(DOS)Date of standard
struct Dos {
	u8 month:4;//月 <1..12>
	u8 year:4;//年 <0..9>
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
//7.2.7.8 电能累计量数据保护的校核 TODO:再详细查阅规约,查询对哪几部分CS
typedef u8 signature_t;
//7.2.7.9 初始化原因 Cause of initialization
union Coi {
	u8 val;
	struct {
		u8 coi:7;//初始化原因
		u8 pc:1;//local parameter change. 0-unchanged 1-changed;
	};
};
const int COI_LOC_POWON=0;
const int COI_LOC_MANUAL_RESET=1;
const int COI_RETOME_RESET=2;
/*	<3..31> 为此配套标准的标准定义保留（兼容范围）
 * 	<32..127>：＝为特殊应用保留（专用范围）
 * */
const int COI_PARAMETER_UNCHANGED=0;
const int COI_PARAMETER_CHANGED=1;
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




/* **************** 帧结构中部分已知的结构, 帧头 帧尾 各种单元头等 **************
 * 本规约中 LSDU=ASDU=APDU
 * 报文=LPDU=LPCI+LSDU
 * ADSU=DUID+N*Info_Obj+ADSU的公共地址(可选)
 *变长帧结构使用 FT1.2 格式:
|                                    |     含义    |  长度 |  小节  |
+-------+------------------------------------------+------+--------+----------+
|       |                            |    0x68     |1 byte|        |          |
| Frame |                            |    length   |1 byte|4 Bytes |          |
| head  |                            |length(copy) |1 byte|        |          |
|       |                            |    0x68     |1 byte|        | LPCI     |
+-------+---------+------------------+-------------+------+--------+ 60870-5-2|
|       | u_dat   |                  |  Ctrl Field |1 byte|        |          |
|       |   head  |                  | Link addr lo|1 byte|3 Bytes |          |
|       |         |                  | Link addr hi|1 byte|        |          |
|       +---------+------+-----------+-------------+------+--------+-----+    |
|       |         |      |           |  Type Id    |1 byte|        |     |    |
|       |         |      | ASDU head |    VSQ      |1 byte|        |     |    |
|       |         |      |  (DUID)   |    COT      |1 byte|6 Bytes |     |    |
| User  |         |      |           | ASDU addr lo|1 byte|        |     |    |
| Data  |         |      |           | ASDU addr hi|1 byte|        |     |    |
|       | u_dat   |      |           |    RAD      |1 byte|        |     |LPCI|
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
+-------+---------+------+-----------+-------------+------+--------+-----+    |
| Frame |                            |     CS      |1 byte|2 Bytes |   LPCI   |
| Tail  |                            |    0x16     |1 byte|        |          |
+-------+----------------------------+-------------+------+--------+----------+
*/
// C1.1 可变帧长帧 - 帧头 Frame head
struct Frame_head {
	u8 start_byte1;
	u8 len1;
	u8 len2;
	u8 start_byte2;
};
// IEC60870-5-2 3.2中的用户数据 头部分
struct Udat_head {
	union { //控制域
		union Ctrl_down c_down;
		union Ctrl_up c_up;
	};
	link_addr_t link_addr;
};
// 7.1 数据单元标识(应用服务数据单元头),Application Service Data Unit(ASDU)
struct Duid { //ASDU头即 数据单元标识 Data Unit IDentifier
	typ_t typ;
	union  Vsq vaq;
	union Cot cot;
	/* 7.2.4 电能累计量数据终端设备的地址从1开始，
	对于信息体每超过一次255个信息点的情况，
	将终端设备地址依次加1。
	终端设备的地址可以和链路地址不一致。*/
	rtu_addr_t rtu_addr;
	/* 7.2.5 记录地址(RAD) */
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
| LPDU       | Link addr lo    |1 byte |
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






#endif // SD102_STRUCT_H
