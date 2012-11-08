/*ɽ��102��Լ �ṹ�嶨��ͷ�ļ�
 δע�����½ں� �ο�:ɽ������ DL/T 719-2000
			����ϵͳ�����ۼ���
			�������ױ�׼ʵʩ�淶(2011)
 �����½ںż������Э����
*/
#ifndef SD102_STRUCT_H
#define SD102_STRUCT_H
#include "typedefine.h"
#pragma pack(1) //���ļ������нṹ��ѹ��!���չ�Լ���涨���շֲ�
// 7.2.1.1 ���ͱ�ʶ��ֵ������Ķ���
// �� 4 ���ͱ�ʶ������-�ڼ��ӷ����ϵĹ�����Ϣ
#define C_TE_TA_GX2	120    /*--- ���󸴷��ʵ���-----*/
#define C_YC_NA_GX2	171 /*����ǰң����*/
#define C_YC_TA_GX2	172  /*----������ʷң����-----*/
#define C_XL_NA_GX2	173  /*-----����ǰ�������----*/
#define C_XL_TA_GX2	174 /*-----������ʷ�������-----*/
#define C_SP_NB_GX2	102  /*--��һ��ʱ�䷶Χ�ڴ�ʱ��ĵ�����Ϣ��¼---*/
#define C_TI_NA_GX2	103 /*---���ն˵�ǰϵͳʱ��------*/
#define C_SYN_TA_GX2	128  /*---ͬ���ն�ʱ��-------*/
// �� 5 ���ͱ�ʶ������-�ڼ��ӷ����ϵ�ϵͳ��Ϣ
// �� 6 ���ͱ�ʶ������-�ڿ��Ʒ����ϵ�ϵͳ��Ϣ
#define M_IT_TA_B_GX2	2   /*-----��ʷ�����ʵ���--------*/
#define M_YC_NA_GX2	161   /*-----��ǰң����----------*/
#define M_YC_TA_GX2	162   /*-----��ʷң����----------*/
#define M_XL_TA_GX2	163   /*-----�������-----------*/
#define M_SP_TA_GX2	1     /*-----������Ϣ-------*/
#define M_TI_TA_GX2	72   /*ϵͳ��ǰʱ��*/
#define M_SYN_TA_GX2	128  /*---ͬ���ն�ʱ��-------*/
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
union  vsq_t{
	u8 val;
	struct{
		u8 n:7;
		u8 sq:1;
	};
};
//7.2.3 ����ԭ�� (CAUSE OF TRANSMISSION)
union cot_t{
	u8 val;
	struct{
		u8 cause:6;
		u8 pn:1;
		u8 t:1;
	};
};
//7.2.4 �����ۼ��������ն��豸��ַ
typedef u16 rtuaddr;
//7.2.5 ��¼��ַ(RAD)
typedef u8 rad_t;
//7.2.6 ��Ϣ���ַ(IOA) Information Object Address
typedef u8 ioa_t;

// ���кź�����״̬ �ֽ�
union data_status{
	u8 val;
	struct{
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
//7.2.7.1 �����ۼ���(IT)
struct	it{
	u32 dat;
	union data_status d_status;
};
//7.2.7.2 ʱ����Ϣ a(Time information a,������)
struct Ta{
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
struct Tb{
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
struct dos{
	//TODO ���� ,������ϸ�Ķ��±�׼
};

//3�ֽڵ�ַ
struct addr_t{
	u16 Slave_addr;
};

//���ݵ�Ԫ��ʶ 6�ֽ�
struct data_unit_t{
	u8 typ;
	union  vsq_t vaq;
	union cot_t cot;
	struct addr_t addr;
};

//��׼��������
union proctol_date{
	u8 val;
	struct {
		u8 month:4;
		u8 year:4;
	};
};

//��ʼ��ԭ��=Cause of initialization
union coi_t{
	u8 val;
	struct{
		u8 coi:7;
		u8 bs:1;
	};
};


//����������Ч�ֽ�
union power_date_iv{
	u8 val;
	struct{
		u8 res:7;//���� ȡ0
		u8 iv:1;
	};
};
//�������ʲɼ���Ŀ
struct power_caiji{
	union  {
		u8 val;
		struct{
			u8 t:1;
			u8 j:1;
			u8 f:1;
			u8 p:1;
			u8 g:1;
			u8 res:3;
		};
	}zy;
	union  {
		u8 val;
		struct{
			u8 t:1;
			u8 j:1;
			u8 f:1;
			u8 p:1;
			u8 g:1;
			u8 res:3;
		};
	}fy;
	union  {
		u8 val;
		struct{
			u8 t:1;
			u8 j:1;
			u8 f:1;
			u8 p:1;
			u8 g:1;
			u8 res:3;
		};
	}zw;
	union  {
		u8 val;
		struct{
			u8 t:1;
			u8 j:1;
			u8 f:1;
			u8 p:1;
			u8 g:1;
			u8 res:3;
		};
	}fw;
};



//�����ʵ����ۼ��� Multi-rate IT
struct multi_it{
	u32 total;
	u32 rate1;//����1
	u32 rate2;
	u32 rate3;
	u32 rate4;
	u32 rate5;//����Ϊ0
	union data_status d_status;
};
// �½��㸴���ʵ����ۼ���
struct month_mit{
	u32 total;
	u32 rate1;//����1
	u32 rate2;
	u32 rate3;
	u32 rate4;
	u32 rate5;//����Ϊ0
	union data_status d_status;
};
//�����������������ʱ��
struct month_maxdemand{
	u32 total_maxdemand;//�����������ֵ
	struct Ta occur_time;
};

//���޶��ʵĵ�����Ϣ single-piont information
struct spinfo{
	u8 spa;//single-point addr
	u8 spi:1;//
	u8 spq:7;//
};

//ң���� Remote measurement
struct	remote_measure{
	u32 dat;
	union data_status d_status;
};
//���г������ Meter harmonic data
struct mtr_harmonic_data{
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
//����(Monitor)���� ��ʱ��ĵ�����Ϣ(Single-Point)
struct M_SP_TA_2_InfoObj{
	struct spinfo sp;
	struct Tb tb;
};
//�����ۼ��� information object
struct M_IT_TA_2_InfoObj{
	ioa_t ioa;//
	struct it it_power;
	//�����ۼ������ݱ�����У��
};
//�����ʼ���(�Ʒ�)�����ۼ��� information object
struct M_IT_TA_B_2_InfoObj{
	ioa_t ioa;//
	struct multi_it mit;
	//�����ۼ������ݱ�����У��
};
///ң����ʷ��-information object
struct M_YC_TA_2_InfoObj{
	ioa_t ioa;//
	struct	remote_measure rm;
};

//���������������ʱ�� ˳����Ϣ��(SQ=0) Information Object
struct M_MMD_TA_InfoObj{
	ioa_t ioa;//
	struct month_maxdemand mmd;
};
//�½��㸴���ʵ����ۼ��� Information Object
struct M_MMIT_InfoObj{
	ioa_t ioa;//
	struct month_mit mmit;
};
//���г������ ˳����Ϣ��(SQ=0) information object
struct M_THD_InfoObj{
	ioa_t ioa;//
	struct mtr_harmonic_data mhd;
};
//��ʼ������
struct M_EI_NA_2_InfoObj{
	ioa_t ioa;//
	union coi_t coi;
};
//�����ۼ��������ն��豸�����쳧�Ͳ�Ʒ�Ĺ淶
struct P_MP_NA_2_iObj{

};
//�����ۼ��������ն��豸Ŀǰ��ϵͳʱ��(
struct M_TI_TA_2_iObj{
	struct Tb Time;
};
//7.3.3 �ڿ��Ʒ����ϵ�ϵͳ��Ϣ��Ӧ�÷������ݵ�Ԫ
//�����쳧�Ͳ�Ʒ�Ĺ淶
//struct C_RD_NA_2_iObj{
//	//û����Ϣ�� = =
//};
//��ѡ��ʱ�䷶Χ�Ĵ�ʱ��ĵ�����Ϣ��¼(
struct C_SP_NB_2_iObj{
	struct Ta starttime;
	struct Ta endtime;
};
//�������ۼ��������ն��豸��Ŀǰ��ϵͳʱ��
//struct C_TI_NA_2_iObj{
//	//û����Ϣ��
//};
//7.3.3.5 ��һ��ѡ����ʱ�䷶Χ��һ��ѡ���ĵ�ַ��Χ���½��㸴���ʵ����ۼ���
struct C_CI_NA_C_2_iObj{
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
//7.3.3.6 ��һ��ѡ��ʱ�䷶Χ��ѡ����ַ��Χ��ң��(YC)��;
struct C_YC_TA_2_iObj{
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};
//7.3.3.7 ��һ��ѡ��ʱ�䷶Χ��ѡ����ַ��Χ�����������(XL)
struct C_XL_NB_2_iObj{
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
};

//7.3.3.8 ��һ��ѡ��ʱ�䷶Χ��ѡ����ַ��Χ�ı��г������
struct C_CI_NA_C_2_iObj{
	ioa_t start_ioa;
	ioa_t end_ioa;
	struct Ta Tstart;
	struct Ta Tend;
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
struct short_farme{
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
