/* File encode:	 GB2312
   filename:	typedefine.h
����һЩ��Ϊͨ�õ��Զ�������*/
#ifndef TYPEDEFINE_H
#define TYPEDEFINE_H
//��ӡ������Ϣ.��ԭʼ�ĵ��Է�ʽ
#define PRINT_HERE {				\
	printf("[File:%s Line:%d] Fun:%s\n",	\
	__FILE__, __LINE__, __FUNCTION__);	\
	}
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
//���� gbt18657-5-4Ӧ����Ԫ�صĶ���ͱ����ж�������ݸ�ʽ idt IEC60870-5-4���ݱ�ʾ��ʽ
typedef unsigned char UI8;
typedef unsigned short int UI16;
typedef unsigned int UI32;
typedef  char I8;
typedef  short int I16;
typedef  int I32;
typedef float R16;// 2�ֽ�
typedef double R32 ;//4�ֽ� ����ʵ������
typedef unsigned char BS8;
#endif // TYPEDEFINE_H
