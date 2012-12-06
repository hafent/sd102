/** @file typedefine.h File encode UTF-8
	定义一些较为通用的自定义类型.
	如 8位无符号整形 u8 等
*/
#ifndef TYPEDEFINE_H
#define TYPEDEFINE_H
#define SD102_Special_structure //标识山东102特定的结构
//打印运行信息.最原始的调试方式
#define PRINT_HERE {					\
	printf("[File:%s Line:%d] Fun:%s\n",	\
	__FILE__, __LINE__, __FUNCTION__);		\
	}
//打印编译构建的日期和时间，类似：Dec  3 2012 09:59:57
#define BUILD_INFO {					\
	printf("Build:%s %s\n",	\
	__DATE__, __TIME__);		\
	}
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
//根据 gbt18657-5-4应用题元素的定义和编码中定义的数据格式 idt IEC60870-5-4数据表示格式
typedef unsigned char UI8;
typedef unsigned short int UI16;
typedef unsigned int UI32;
typedef  char I8;
typedef  short int I16;
typedef  int I32;
typedef float R16;// 2字节
typedef double R32 ;//4字节 浮点实数类型
typedef unsigned char BS8;
#endif // TYPEDEFINE_H
