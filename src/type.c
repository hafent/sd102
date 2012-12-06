/** @file type.c
 * 定义各种描述,在sd102.h中通过 extern 引用.
 * 只有在c文件中 g++ 才支持 指定初始化(Designated Initializers):
 * http://gcc.gnu.org/onlinedocs/gcc/Designated-Inits.html\n
 * 在cpp中即使extern和无法使用, 指定初始化是c90标准(C语言标准)不是C++标准.\n
 * g++尚未是现实对指定初始化的支持(包括数组的和结构体的),结构体的指定初始化可以通过\n
 * 	GNU的扩展语法实现,但是数组的指定初始化在g++中尚未支持.\n
 * 若必须使用,则可以在1.c中实现,编译成1.o,然后和main.cpp编译成的main.o通过g++链接\n
 * 	而成. 例如,有 main.cpp(主程序) 1.c(使用指定初始化的实现)\n
 * 	在mian.cpp中使用 extern 引用 1.c中的变量\n
 * 	gcc -c 1.c 	-o 1.o\n
 * 	g++ -c main.cpp -o main.o\n
 * 	g++ 1.o main.o 	-o main.exe\n
 * 未被使用,使用需要修改Makefile,取消gcc的注销,\n
 * 	修改 "sd102_ctDuid.h" "sd102_ctUdat.h",开启对extern的宏编译\n
 * @author 李培钢
 * */
#include "typedefine.h"
#include "sd102_ctDuid.h"
#include "sd102_ctUdat.h"
///下行(c)功能码及描述结构体数组
struct stFcn_info fcn_info_c[]={
		[FCN_C_RCU]={"Reset Communication Unit","复位通讯单元"},
		[FCN_C_RLK]={"Call Link Status","链路状态请求"},
		[FCN_C_PL1]={"Call class 1 date","召唤1级数据"},
		[FCN_C_PL2]={"Call class 2 date","召唤2级数据"},
		[FCN_C_RES1]={"Reseave","保留1"},
		[FCN_C_RES2]={"Reseave","保留2"},
};
///上行(m|p)功能码及描述结构体数组
struct stFcn_info fcn_info_m[]={
		[FCN_M_CON]={"Confirm","确认"},
		[FCN_M_LINK_BUSY]={"Link busy","链路繁忙"},
		[FCN_M_SEND_DAT]={"Send data","以数据回复"},
		[FCN_M_NO_DAT]={"No data","没有所召唤的数据"},
		[FCN_M_RSP]={"Response","返回链路状态"},
};

struct stTyp typ_info_c[]={
		[C_RD_NA_2]={"Read Manufact","读制造厂和产品规范"}
};

