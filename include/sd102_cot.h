#ifndef SD102_COT_H
#define SD102_COT_H
#include "typedefine.h"
typedef u8 cot_t;
const cot_t C_FC_Reset_communication_unit=0;
const cot_t C_Transfer_data=3;


const cot_t COT_TEST		=1;
const cot_t COT_cycle		=2;
const cot_t COT_Spontaneous	=3;
const cot_t COT_INIT		=4;
const cot_t COT_REQUEST		=5;
const cot_t COT_ACT		=6;
const cot_t COT_ACT_CON		=7;
const cot_t COT_DE_ACT		=8;
const cot_t COT_DE_ATC_CON	=9;
const cot_t COT_ACT_TREMINATE	=10;


const cot_t COT_NO_DAT		=13;
const cot_t COT_NO_ASDU		=14;


const cot_t COT_SYN_TIME	=48;
#endif // SD102_COT_H
