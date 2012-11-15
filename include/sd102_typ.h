#ifndef SD102_TYP_H
#define SD102_TYP_H
#include "typedefine.h"
// 7.2.1.1 类型标识域值的语义的定义
typedef u8 typ_t;
// 表4 类型标识的语义-在监视方向上的过程信息 (上行)
const typ_t M_UNUSED=0;// 未用
const typ_t M_SP_TA_2=1;//带时标的单点信息
const typ_t M_IT_TA_2=2;//记账(计费)电能累计量,每个量为四个八位位组
const typ_t M_IT_TD_2=5;//周期复位记账(计费)电能累计量,每个量为四个八位位组
const typ_t M_SYN_TA_2=128;//电能累计量数据终端系统时间同步确认帧
const typ_t M_IT_TA_B_2=160;//复费率记帐(计费)电能累计量
const typ_t M_YC_TA_2=162;//遥测历史值
const typ_t M_XL_TA_2=163;//最大需量
const typ_t M_IT_TA_C_2=164;//月结算复费率电能累计量
const typ_t M_IT_TA_D_2=165;//表计谐波数据
// 表5 类型标识的语义-在监视方向上的系统信息 (上行)
const typ_t M_EI_NA_2=70;//初始化结束
const typ_t P_MP_NA_2=71;//电能累计量数据终端设备的制造厂和产品规范
const typ_t M_TI_TA_2=72;//电能累计量数据终端设备的当前系统时间
// 表6 类型标识的语义-在控制方向上的系统信息 (下行)
const typ_t C_RD_NA_2=100;//读制造厂和产品规范
const typ_t C_SP_NA_2=101;//读带时标的单点信息的记录
const typ_t C_SP_NB_2=102;//读一个所选定时间范围的带时标的单点信息的记录
const typ_t C_TI_NA_2=103;//读电能累计量数据终端设备的当前系统时间
const typ_t C_CI_NR_2=120;//读一个选定的时间范围和一个选定的地址范围的记账(计费)电能累计量
const typ_t C_CI_NS_2=121;//读周期地复位的一个选定的时间范围和一个选定的地址范围的记账(计费)电能累计量
const typ_t C_SYN_TA_2=128;//电能累计量数据终端系统时间同步命令
const typ_t C_CI_NA_B_2=170;//读一个选定的时间范围和一个选定的地址范围的复费率记帐(计费)电能累计量
const typ_t C_YC_TA_2=172;//读一个选定时间范围和选定地址范围的遥测量
const typ_t C_CI_NA_C_2=173;//读一个选定的时间范围和一个选定的地址范围的月结算复费率电能累计量
const typ_t C_XL_NB_2=174;//读一个选定的时间范围和一个选定的地址范围的最大需量
const typ_t C_CI_NA_D_2=175;//读一个选定的时间范围和一个选定的地址范围的表计谐波数据

#endif // SD102_TYP_H
