/**     @file sd102_stElement.h
	定义7.2  IEC 60870-5-4（应用信息元素的定义和编码）的结构.
	application information elements(仅定义结构体)

说明:

	未注明的章节号内容参考资料1,其他章节号见其具体协议编号.
	数据格式按照 GB/T18657.4 idt IEC60870-5-4
	*_t 为无符号整型数据(8位/16位等)
	首字母大写的单词为结构体类型/联合体 如  It Vsq

参考资料:

	1.山东电网 DL/T 719-2000 电力系统电能累计量传输配套标准实施规范(2011)
	 	 ref IEC60870-5-102
	2.http://en.wikipedia.org/wiki/IEC_60870-5
*/
#ifndef SD102_ELEMENT_H
#define SD102_ELEMENT_H
#define KIND_OF_102 SD102 ///<定义特殊类别的102规约,指示为山东102
#include "typedefine.h"
#pragma pack(1)
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

///C3.1 控制域功能码 functions code number
typedef u8 fcn_t;//u8:4 functions code number
///C3.1 控制域 主站->终端(下行) 控制
union Ctrl_c {
	u8 val;
	struct {
		u8 fcn:4;
		u8 fcv:1;///<(dfc,data flow control)
		u8 fcb:1;///<frame count bit
		u8 prm:1;///<1 主站向终端 primary message
		u8 dir:1;///<非平衡传输,应保留为0
	};
} ;
///C3.1 控制域 终端->主站(上行) 监视
union Ctrl_m {
	u8 val;
	struct {
		u8 fcn:4;
		u8 dfc:1;///<(dfc,data flow control)
		u8 acd:1;///<access demand //访问需求
		u8 prm:1;///<0 终端向主站 primary message
		u8 res:1; ///<本规约备用
	};
};
///6.1传输帧格式
typedef u16 link_addr_t;///<链路地址
///7.2.1.1 类型标识域值的语义的定义
typedef u8 typ_t;
///7.2.2 可变结构限定词(VARIABLE OF STRUCTURE QUALIFIER）
union  Vsq {
	u8 val;
	struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
		u8 n:7;///<可变结构数量
		u8 sq:1;///<n的寻址方式:0-顺序 1-单独.
#elif __BYTE_ORDER == __BIG_ENDIAN
		u8 sq:1;
		u8 n:7;
#else
#warning "Unkown Byte Order __BYTE_ORDER "
#endif /*__BYTE_ORDER*/
	};
};
/**7.2.3 传送原因(Cause Of Transmission).
 7 6 5 4 3 2 1 0
+-+-+-+-+-+-+-+-+
|T|P|  cause    |
+-+-+-+-+-+-+-+-+
*/
union Cot {
	u8 val;
	struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
		u8 cause:6;//传送原因＝Cause of transmission
		u8 pn:1;// 应答
		u8 t:1;//test 试验
#elif __BYTE_ORDER == __BIG_ENDIAN
		u8 t:1
		u8 pn:1
		u8 cause:6
#else
#endif /*__BYTE_ORDER*/
	};
};
/** 7.2.4 电能累计量数据终端设备地址(应用服务单元公共地址)ASDU_addr rtu addr */
typedef u16 rtu_addr_t; ///<协议以小尾端方式存储,不像通常的使用网络字节序
/** 7.2.5 记录地址(RAD)	Record Address
*/
typedef u8 rad_t;
///7.2.6 信息体地址(IOA) Information Object Address
typedef u8 ioa_t;
/** 7.2.7 信息体元素
   7.2.7.1 序列号和数据状态 字节 //sd102中全改为状态位, */
union Seq_number {
	u8 val;
	struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
		u8 sn:5;///<序列号
		u8 cy:1;///<carry
		u8 ca:1;///<计数器被调整(CA＝counter was adjusted)
		u8 iv:1;///<无效(IV＝invalid)
#elif __BYTE_ORDER == __BIG_ENDIAN
		u8 iv:1;
		u8 ca:1;
		u8 cy:1;
		u8 sn:5;
#else
#endif /*__BYTE_ORDER*/
	};
};

///7.2.7.1 数据状态字节(替换序列号和数据状态),1表示这种状态,0表示没有这种状态
union Data_status {
	u8 val;
	struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
		u8 lv:1;///<失压 lost v
		u8 phb:1;///<断相
		u8 lc:1;///<lose current
		u8 wph:1;///< wrong phase
		u8 blv:1;///<Battery lose voltage
		u8 ct:1;///<timeout
		u8 com_terminal:1;///<Communication terminal
		u8 iv:1;///<invalid
#elif __BYTE_ORDER == __BIG_ENDIAN
		u8 iv:1;
		u8 com_terminal:1;
		u8 ct:1;
		u8 blv:1;
		u8 wph:1;
		u8 lc:1;
		u8 phb:1;
		u8 lv:1;
#endif /*__BYTE_ORDER*/

	};
};

///7.2.7.1 电能累计量 IT (Integrated total)
struct	It {
	u32 dat;
#if KIND_OF_102 ==  SD102
	union Data_status d_status;
#else
	union Seq_number sq;
#endif

};
///7.2.7.2 5字节时间信息 a(Time information a,分至年)
struct Ta {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	//Byte 1
	u8 min:6;///分钟 0..59
	u8 tis:1;///<费率信息开关 0-off tariff information switch
	u8 iv:1;///<时间陈述无效标志位 invalid
	//Byte 2
	u8 hour:5;///小时 0..23
	u8 res1:2;///<备用 reserve 1 <0>
	u8 su:1;///<夏令时 summer time
	//Byte 3
	u8 day:5;///<几号:1..31
	u8 week:3;///<星期几:1..7
	//Byte 4
	u8 month:4;///<月:1..12
	u8 eti:2; ///<能量费率信息(ETI＝energy tariff information)
	u8 pti:2;///<功率费率信息(PTI＝power tariff information)
	//Byte 5
	u8 year:7;///< 2000到目前为止的年数:0..99
	u8 res2:1;///<备用2(RES2＝reserve 2) <0>
#elif __BYTE_ORDER == __BIG_ENDIAN
	u8 iv:1;
	u8 tis:1;
	u8 min:6;
	//
	u8 su:1;
	u8 res1:2;
	u8 hour:5;
	//
	u8 week:3;
	u8 day:5;
	//
	u8 pti:2;
	u8 eti:2;
	u8 month:4;
	//
	u8 res2:1;
	u8 year:7;
#else
#endif
};
///7.2.7.3 7字节时间信息 b(Time information b,毫秒至年)
struct Tb {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u16 ms:10;///<毫秒 <0..999>
	u16 second:6;///< 秒 0..59 闰秒是不被允许的
	//
	u8 min:6;///<分钟 0..59
	u8 tis:1;///<费率信息开关 0-off tariff information switch
	u8 iv:1;///<时间陈述无效标志位 invalid
	//
	u8 hour:5;///<小时 0..23
	u8 res1:2;///<备用 reserve 1
	u8 su:1;///<夏令时 summer time
	//
	u8 day:5;///<几号 1..31
	u8 week:3;///<星期几 1..7
	//
	u8 month:4;///< 月:1..12
	u8 eti:2; ///<energy tariff information
	u8 pti:2;///<power tariff information
	//
	u8 year:7;
	u8 res2:1;///<reserve 2
#elif __BYTE_ORDER == __BIG_ENDIAN
	u16 second:6;
	u16 ms:10;
	//
	u8 iv:1;
	u8 tis:1;
	u8 min:6;
	//
	u8 su:1;
	u8 res1:2;
	u8 hour:5;
	//
	u8 week:3;//星期几 1-7
	u8 day:5;//几号 1-31
	//
	u8 pti:2;//power tariff information
	u8 eti:2; //energy tariff information
	u8 month:4;// <1..12>
	//
	u8 res2:1;//reserve 2
	u8 year:7;
#else
#endif /*__BYTE_ORDER*/
};

///7.2.7.4 标准的日期(DOS)Date of standard
struct Dos {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u8 month:4;///<月 <1..12>
	u8 year:4;///<年 <0..9>
#elif __BYTE_ORDER == __BIG_ENDIAN
	u8 year:4;
	u8 month:4;
#endif/*__BYTE_ORDER*/
};
///7.2.7.5 制造厂编码 Manufacturer code
typedef u8 factcode_t; ///< 0..255
///7.2.7.6 产品编码 product code ;BS32 bit strings 32bits
typedef u32 productcode_bs;
///7.2.7.7 带地址和限定词的单点信息 single-point information
struct Sp {
	u8 spa;//single-point address
	u8 spi:1;//
	u8 spq:7;//
};
///7.2.7.8 电能累计量数据保护的校核 @todo 再详细查阅规约,查询对哪几部分CS
typedef u8 signature_t;
///7.2.7.9 初始化原因 Cause of initialization
union Coi {
	u8 val;
	struct {
		u8 coi:7;///<初始化原因
		u8 pc:1;///<local parameter change. 0-unchanged 1-changed;
	};
};

///7.2.7.10 复费率电能累计量 Multi-rate  Integrated total
struct Multi_it {
	u32 total;
	u32 rate1;///<费率1
	u32 rate2;
	u32 rate3;
	u32 rate4;
	u32 rate5;///<保留为0
	union Data_status d_status;
};
///7.2.7.11 月结算复费率电能累计量 Monthly balance sheet multi-rate  IT
struct Month_mit {
	u32 total;
	u32 rate1;///<费率1
	u32 rate2;
	u32 rate3;
	u32 rate4;
	u32 rate5;///<保留为0
	union Data_status d_status;
};
///7.2.7.12 月总最大需量及发生时间 Monthly total demand and time
struct Month_maxdemand {
	u32 total_maxdemand;///<月总最大需量值
	struct Ta occur_time;
};
///7.2.7.13 遥测量 Remote measurement
struct	Remote_measure {
	u32 dat;
	union Data_status dst;
};
///7.2.7.14 表计谐波数据 Meter harmonic data
struct Harmonic_data {
	u32  fa_thd ;///<forward active Total harmonic power
	u32  ra_thd;///<Reverse active total harmonic power
	u32  ta_distortion;///<Total active the harmonic power consumption distortion rate
	u32  v_distortion;///<Total voltage harmonic distortion
	u32  c_distortion;///<Total current harmonic distortion
	u32  res1;///<备用1 reserve 1
	u32  res2;///<备用2
	u32  res3;///<备用3
};



#pragma pack()
#endif // SD102_STRUCT_H
