#ifndef SD102_COT_H
#define SD102_COT_H
#include "typedefine.h"
typedef u8 cot_t;
//const cot_t C_FC_Reset_communication_unit=0;
//const cot_t C_Transfer_data=3;

//下行:
const cot_t COT_TEST		=1;//主站测试链路,并不执行操作
const cot_t COT_cycle		=2;//主站周期性轮询
const cot_t COT_REQUEST	=5;//请求或者被请求
const cot_t COT_ACT		=6;//激活
const cot_t COT_DE_ACT	=8;//取消激活

//上行:
const cot_t COT_Spontaneous	=3;//自发 突发
const cot_t COT_INIT		=4;//从站初始化完毕
//const cot_t COT_REQUEST	=5;//被请求[回复请求,同是5]
const cot_t COT_ACT_CON	=7;//激活-确认[回复激活]
const cot_t COT_DE_ATC_CON	=9;//取消激活-确认[回复取消激活]
const cot_t COT_ACT_TREMINATE=10;//激活终止[自发]
//
const cot_t COT_NO_DAT	=13;
const cot_t COT_NO_ASDU	=14;
const cot_t COT_SYN_TIME	=48;
#endif // SD102_COT_H
