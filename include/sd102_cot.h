#ifndef SD102_COT_H
#define SD102_COT_H
#include "typedefine.h"
#pragma pack(1)
typedef u8 cot_t;
//const cot_t C_FC_Reset_communication_unit=0;
//const cot_t C_Transfer_data=3;

//下行:
const cot_t COT_TEST		=1;//主站测试链路,并不执行操作
const cot_t COT_CYCLE		=2;//主站周期性轮询
const cot_t COT_REQUEST	=5;//请求或者被请求
const cot_t COT_ACT		=6;//激活
const cot_t COT_DE_ACT	=8;//取消激活

//上行:
const cot_t COT_Spontaneous	=3;//自发 突发
const cot_t COT_INIT		=4;//初始化
//const cot_t COT_REQUEST	=5;//被请求[回复请求,同是5]
const cot_t COT_ACT_CON	=7;//激活-确认[回复激活]
const cot_t COT_DE_ATC_CON	=9;//取消激活-确认[回复取消激活]
const cot_t COT_ACT_TREMINATE=10;//激活终止[自发]
//
const cot_t COT_NO_DAT	=13;
const cot_t COT_NO_ASDU	=14;
const cot_t COT_SYN_TIME	=48;
//PN 和 test位
const int COT_PN_ACK		=0;//肯定确认
const int COT_PN_NACK		=1;//否定确认
const int COT_T_NOT_TEST	=0;//非测试,执行
const int COT_T_TEST		=1;//测试链路,不执行

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
const int RAD_DEFAULT=0;
const int RAD_ALL_SP_INFO=51;
#pragma pack()
#endif // SD102_COT_H
