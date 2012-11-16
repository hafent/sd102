/*������·������ Link function code
*/
#ifndef SD102_FUNCODE_H
#define SD102_FUNCODE_H
#include "typedefine.h"

typedef u8 link_fun_code_t;//u8:4

//S2	����/ȷ��	�ɿ���վ������ۼ��������ն˷��������(дָ��)
//S3	����/��Ӧ	�ɿ���վ������ۼ��������ն��ٻ����ݻ��¼�(������)

// C3.2.5 ������:����վ������ۼ��������ն˴����֡�й�����Ķ��� Ctrl(����)
//֡���� 0,3 ����/ȷ�� ֡ S2
const link_fun_code_t FN_C_RCU  =0;//��λ Reset Communication Unit
const link_fun_code_t FN_C_TRANS_DAT  =3;//�������� trans date
//֡���� 9,10,11 ����/��Ӧ ֡ S3
const link_fun_code_t FN_C_RLK  =9;//�ٻ���· call link ������·״̬
const link_fun_code_t FN_C_PL1 =10;//�ٻ�1����· call class 1 data
const link_fun_code_t FN_C_PL2 =11;//�ٻ�2����· call class 2 date
const link_fun_code_t FN_C_RES1=12;//����1
const link_fun_code_t FN_C_RES2=13;//����2


// C3.3.4 ������:�����ۼ��������ն������վ�����֡�й�����Ķ��� Monitor(����)
//֡����: 0 1 ȷ�� S2
const link_fun_code_t FN_M_CON =0;//ȷ�� Confirm
const link_fun_code_t FN_M_LINK_BUSY	 =1;//��·��æ link busy
//֡����: 8 9 11 ��Ӧ S3
const link_fun_code_t FN_M_SEND_DAT  =8;//��������Ӧ����֡ send data
const link_fun_code_t FN_M_NO_DAT  =9;//û�����ٻ������� no data
const link_fun_code_t FN_M_RSP =11;//����·״̬���������ش�����֡ Response

#endif // SD102_FUNCODE_H
