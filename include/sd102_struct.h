/*ɽ��102��Լ �ṹ�嶨��ͷ�ļ�
 δע�����½ں� �ο�:ɽ������ DL/T 719-2000
			����ϵͳ�����ۼ���
			�������ױ�׼ʵʩ�淶(2011)
			���ñ�׼IEC60870-5-102
 �����½ںż������Э����
 ���ݸ�ʽ���� GBT18657.4 dit IEC60870-5-4
*/
#ifndef SD102_STRUCT_H
#define SD102_STRUCT_H
#include "typedefine.h"
#pragma pack(1) //���ļ������нṹ��ѹ��!���չ�Լ���涨���շֲ�
// 7.2.1.1 ���ͱ�ʶ��ֵ������Ķ���
typedef u8 typ_t;
// ��4 ���ͱ�ʶ������-�ڼ��ӷ����ϵĹ�����Ϣ
const typ_t M_UNUSED=0;// δ��
const typ_t M_SP_TA_2=1;//��ʱ��ĵ�����Ϣ
const typ_t M_IT_TA_2=2;//����(�Ʒ�)�����ۼ���,ÿ����Ϊ�ĸ���λλ��
const typ_t M_IT_TD_2=5;//���ڸ�λ����(�Ʒ�)�����ۼ���,ÿ����Ϊ�ĸ���λλ��
const typ_t M_IT_TA_B_2=160;//�����ʼ���(�Ʒ�)�����ۼ���
const typ_t M_YC_TA_2=162;//ң����ʷֵ
const typ_t M_XL_TA_2=163;//�������
const typ_t M_IT_TA_C_2=164;//�½��㸴���ʵ����ۼ���
const typ_t M_IT_TA_D_2=165;//���г������
// ��5 ���ͱ�ʶ������-�ڼ��ӷ����ϵ�ϵͳ��Ϣ
const typ_t M_EI_NA_2=70;//��ʼ������
const typ_t P_MP_NA_2=71;//�����ۼ��������ն��豸�����쳧�Ͳ�Ʒ�淶
const typ_t M_TI_TA_2=72;//�����ۼ��������ն��豸�ĵ�ǰϵͳʱ��
// ��6 ���ͱ�ʶ������-�ڿ��Ʒ����ϵ�ϵͳ��Ϣ
const typ_t C_RD_NA_2=100;//�����쳧�Ͳ�Ʒ�淶
const typ_t C_SP_NA_2=101;//����ʱ��ĵ�����Ϣ�ļ�¼
const typ_t C_SP_NB_2=102;//��һ����ѡ��ʱ�䷶Χ�Ĵ�ʱ��ĵ�����Ϣ�ļ�¼
const typ_t C_TI_NA_2=103;//�������ۼ��������ն��豸�ĵ�ǰϵͳʱ��
const typ_t C_CI_NR_2=120;//��һ��ѡ����ʱ�䷶Χ��һ��ѡ���ĵ�ַ��Χ�ļ���(�Ʒ�)�����ۼ���
const typ_t C_CI_NS_2=121;//�����ڵظ�λ��һ��ѡ����ʱ�䷶Χ��һ��ѡ���ĵ�ַ��Χ�ļ���(�Ʒ�)�����ۼ���
const typ_t C_CI_NA_B_2=170;//��һ��ѡ����ʱ�䷶Χ��һ��ѡ���ĵ�ַ��Χ�ĸ����ʼ���(�Ʒ�)�����ۼ���
const typ_t C_YC_TA_2=172;//��һ��ѡ��ʱ�䷶Χ��ѡ����ַ��Χ��ң����
const typ_t C_CI_NA_C_2=173;//��һ��ѡ����ʱ�䷶Χ��һ��ѡ���ĵ�ַ��Χ���½��㸴���ʵ����ۼ���
const typ_t C_XL_NB_2=174;//��һ��ѡ����ʱ�䷶Χ��һ��ѡ���ĵ�ַ��Χ���������
const typ_t C_CI_NA_D_2=175;//��һ��ѡ����ʱ�䷶Χ��һ��ѡ���ĵ�ַ��Χ�ı��г������
// �¼�
#define SPQ_ERTU_RST_GX	1
#define SPQ_ERTU_CLOSE_GX	1 //ϵͳ����
#define SPQ_SYN_TIME_GX 	2//��ʱ
#define SPQ_PARA_MOD_GX	1//�����޸�
#define SPQ_COMM_ERR_GX 48//���ͨѶ����
#define SPQ_COMM_SUCCESS 49//���ͨѶ�ɹ�
#define SPQ_LOW_VA_GX  53	//
#define SPQ_LOW_VB_GX   54	//
#define SPQ_LOW_VC_GX   55 	//
#define SPA_ERTU_RST_GX 	1
#define SPA_ERTU_CLOSE_GX 3 //ϵͳ����
#define SPA_SYN_TIME_GX 	7//��ʱ
#define SPA_PARA_MOD_GX	8//�����޸�
//
const u8 START_SHORT_FARME=0x10; //�̶���֡
const u8 START_LONG_FARME=0x68; //�䳤֡
const u8 START_SINGLE_FARME=0xE5; //���ַ�֡
const u8 C_FC_Reset_communication_unit=0;
const u8 C_Transfer_data=3;
//7.2.2 �ɱ�ṹ�޶��ʣ�VARIABLE OF STRUCTURE QUALIFIER��
union  vsq_t {
	u8 val;
	struct {
		u8 n:7;
		u8 sq:1;
	};
};
//7.2.3 ����ԭ�� (Cause Of Transmission)
union cot_t {
	u8 val;
	struct {
		u8 cause:6;
		u8 pn:1;
		u8 t:1;
	};
};
//7.2.4 �����ۼ��������ն��豸��ַ
typedef u16 rtuaddr;
//7.2.5 ��¼��ַ(RAD)	Record address
typedef u8 rad_t;
//7.2.6 ��Ϣ���ַ(IOA) Information Object Address
typedef u8 ioa_t;
//7.2.7.1 ���кź�����״̬ �ֽ�
union data_status {
	u8 val;
	struct {
		u8 lv:1;//lost v
		u8 phb:1;//����
		u8 lc:1;//lose current
		u8 wph:1;// wrong phase
		u8 blv:1;//Battery lose voltage
		u8 ct:1;//color timeout
		u8 com_terminal:1;//Communication terminal
		u8 iv:1;//invalid
	};
};
//7.2.7.1 �����ۼ��� IT (Integrated total)
struct	it {
	u32 dat;
	union data_status d_status;
};
//7.2.7.2 ʱ����Ϣ a(Time information a,������)
struct Ta {
	u8 min:6;
	u8 tis:1;//������Ϣ���� 0-off tariff information switch
	u8 iv:1;//ʱ�������Ч��־λ invalid
	//
	u8 hour:5;
	u8 res1:2;//���� reserve 1
	u8 su:1;//����ʱ summer time
	//
	u8 day:5;//���� 1-31
	u8 week:3;//���ڼ� 1-7
	//
	u8 month:4;
	u8 eti:2; //energy tariff information
	u8 pti:2;//power tariff information
	//
	u8 year:7;
	u8 res2:1;//reserve 2
};
//7.2.7.3 ʱ����Ϣ b(Time information b,��������)
struct Tb {
	u16 ms:10;//���� <0..999>
	u16 second:6;
	//
	u8 min:6;
	u8 tis:1;//������Ϣ���� 0-off tariff information switch
	u8 iv:1;//ʱ�������Ч��־λ invalid
	//
	u8 hour:5;
	u8 res1:2;//���� reserve 1
	u8 su:1;//����ʱ summer time
	//
	u8 day:5;//���� 1-31
	u8 week:3;//���ڼ� 1-7
	//
	u8 month:4;// <1..12>
	u8 eti:2; //energy tariff information
	u8 pti:2;//power tariff information
	//
	u8 year:7;
	u8 res2:1;//reserve 2
};
//7.2.7.4 ��׼������(DOS)Date of standard
struct dos_t {
	u8 month:4;//�� <1..12>
	u8 year:4;//�� <	0..9>
};
//7.2.7.5 ���쳧���� Manufactuer code
typedef u8 factcode; //<0..255>
//7.2.7.6 ��Ʒ���� product code ;BS32 bit strings 32bits
typedef u32 productcode;
//7.2.7.7 ����ַ���޶��ʵĵ�����Ϣ single-piont information
struct spinfo {
	u8 spa;//single-point addr
	u8 spi:1;//
	u8 spq:7;//
};
//7.2.7.8 �����ۼ������ݱ�����У�� TODO:����ϸ���Ĺ�Լ
typedef u8 Signature;
//7.2.7.9 ��ʼ��ԭ�� Cause of initialization
union coi_t {
	u8 val;
	struct {
		u8 coi:7;
		u8 bs:1;
	};
};
//7.2.7.10 �����ʵ����ۼ��� Multi-rate  Integrated total
struct multi_it {
	u32 total;
	u32 rate1;//����1
	u32 rate2;
	u32 rate3;
	u32 rate4;
	u32 rate5;//����Ϊ0
	union data_status d_status;
};
//7.2.7.11 �½��㸴���ʵ����ۼ��� Monthly balance sheet multi-rate  IT
struct month_mit {
	u32 total;
	u32 rate1;//����1
	u32 rate2;
	u32 rate3;
	u32 rate4;
	u32 rate5;//����Ϊ0
	union data_status d_status;
};
//7.2.7.12 �����������������ʱ�� Monthly total demand and time
struct month_maxdemand {
	u32 total_maxdemand;//�����������ֵ
	struct Ta occur_time;
};
//7.2.7.13 ң���� Remote measurement
struct	remote_measure {
	u32 dat;
	union data_status d_status;
};
//7.2.7.14 ���г������ Meter harmonic data
struct mtr_harmonic_data {
	u32  fa_thd ;//forward active Total harmonic power
	u32  ra_thd;//Reverse active total harmonic power
	u32  ta_distortion;//Total active the harmonic power consumption distortion rate
	u32  v_distortion;//Total voltage harmonic distortion
	u32  c_distortion;//Total current harmonic distortion
	u32  res1;//����1 reserve 1
	u32  res2;//����2
	u32  res3;//����3
};
/*�ض�Ӧ�÷������ݵ�Ԫ�Ķ���ͱ�ʾ
  APDU(Application Service Data Unit)
  */
//7.3.1 �ڼ��ӷ����ϵĹ�����Ϣ��Ӧ�÷������ݵ�Ԫ
//7.3.1.1 ����(Monitor)���� ��ʱ��ĵ�����Ϣ(Single-Point)
struct M_SP_TA_2_InfoObj {
	struct spinfo sp;
	struct Tb tb;
};
//7.3.1.2 �����ۼ��� information object
struct M_IT_TA_2_InfoObj {
	ioa_t ioa;//
	struct it it_power;
	Signature cs;//�����ۼ������ݱ�����У��
};
//7.3.1.3 �����ʼ���(�Ʒ�)�����ۼ��� information object
struct M_IT_TA_B_2_InfoObj {
	ioa_t ioa;//
	struct multi_it mit;
	Signature cs;//�����ۼ������ݱ�����У��
};
//7.3.1.4ң����ʷ�� History of remote measurement -information object
struct M_YC_TA_2_InfoObj {
	ioa_t ioa;//
	struct remote_measure rm;
};
//7.3.1.5 ���������������ʱ�� ˳����Ϣ��(SQ=0) Information Object
struct M_MMD_TA_InfoObj {
	ioa_t ioa;//
	struct month_maxdemand mmd;
};
//7.3.1.6 �½��㸴���ʵ����ۼ��� Information Object
struct M_MMIT_InfoObj {
	ioa_t ioa;//
	struct month_mit mmit;
};
//7.3.1.7 ���г������ ˳����Ϣ��(SQ=0) information object
struct M_THD_InfoObj {
	ioa_t ioa;//
	struct mtr_harmonic_data mhd;
};
//7.3.2 �ڼ��ӷ����ϵ�ϵͳ��Ϣ��Ӧ�÷������ݵ�Ԫ
//7.3.2.1 ��ʼ������
struct M_EI_NA_2_InfoObj {
	ioa_t ioa;//
	union coi_t coi;
};
//7.3.2.2 �����ۼ��������ն��豸�����쳧�Ͳ�Ʒ�Ĺ淶
struct P_MP_NA_2_iObj {
	struct dos_t dos;
	factcode fcode;
	productcode pcode;
};
//7.3.2.3  �����ۼ��������ն��豸Ŀǰ��ϵͳʱ��
struct M_TI_TA_2_iObj {
	struct Tb Time;
};
//7.3.3 �ڿ��Ʒ����ϵ�ϵͳ��Ϣ��Ӧ�÷������ݵ�Ԫ
//7.3.3.1 �����쳧�Ͳ�Ʒ�Ĺ淶
//struct C_RD_NA_2_iObj{
//	//û����Ϣ�� = =
//};
//7.3.3.2 ����ʱ��ĵ�����Ϣ��¼
/*struct C_SP NA_2{
	 //û����Ϣ��
 };*/
//7.3.3.3 ��ѡ��ʱ�䷶Χ�Ĵ�ʱ��ĵ���(SP)��Ϣ��¼
struct C_SP_NB_2_iObj {
	struct Ta starttime;
	struct Ta endtime;
};
//7.3.3.4 �������ۼ��������ն��豸��Ŀǰ��ϵͳʱ��
/*struct C_TI_NA_2_iObj{
	//û����Ϣ��
};*/
//7.3.3.5 ��һ��ѡ����ʱ�䷶Χ��һ��ѡ���ĵ�ַ��Χ���½��㸴���ʵ����ۼ���
struct C_CI_NA_C_2_iObj {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
//7.3.3.6 ��һ��ѡ��ʱ�䷶Χ��ѡ����ַ��Χ��ң��(YC)��;
struct C_YC_TA_2_iObj {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
//7.3.3.7 ��һ��ѡ��ʱ�䷶Χ��ѡ����ַ��Χ�����������(XL)
struct C_XL_NB_2_iObj {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
//7.3.3.8 ��һ��ѡ��ʱ�䷶Χ��ѡ����ַ��Χ�ı��г������
struct C_CI_NA_C_2_iObj {
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
//���ݵ�Ԫ��ʶ 6�ֽ�
struct data_unit_t {
	typ_t typ;
	union  vsq_t vaq;
	union cot_t cot;
	struct addr_t addr;
};
//��׼��������
union proctol_date {
	u8 val;
	struct {
		u8 month:4;
		u8 year:4;
	};
};


//����������Ч�ֽ�
union power_date_iv {
	u8 val;
	struct {
		u8 res:7;//���� ȡ0
		u8 iv:1;
	};
};


//���� ������ ��վ->�ն�
union ctrl_down_t {
	u8 val;
	struct {
		u8 funcode:4;
		u8 fcv:1;
		u8 fcb:1;
		u8 prm:1;
		u8 dir:1;//��ƽ�⴫��,Ӧ����Ϊ0
	};
} ;
//���� ������ �ն�->��վ
union ctrl_up_t {
	u8 val;
	struct {
		u8 funcode:4;
		u8 dfc:1;
		u8 acd:1;
		u8 prm:1;
		u8 dir:1; //����Լ����
	};
};
//�ɱ�֡��֡ - ֡ͷ
struct long_farme_head {
	u8 start_byte1;
	u8 len_hi;
	u8 len_lo;
	u8 start_byte2;
};
//�ɱ�֡��֡ - ֡β
struct long_farme_tail {
	u8 cs;
	u8 end_byte;
};
//�̶�֡��֡ - ֡��
struct short_farme {
	u8 start_byte; //��ʼ�ֽ�
	union { //������
		union ctrl_down_t c_down;
		union ctrl_up_t c_up;
	};
	u8 addr1;//��ַ
	u8 addr2;//��ַ
	u8 cs;//У���
	u8 end_byte;//��ֹ�ַ�
};


#endif // SD102_STRUCT_H
