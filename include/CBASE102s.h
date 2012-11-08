#ifndef CBASE102_H
#define CBASE102_H

#include "protocol.h"
#include "HisDB.h"
//���Ʒ���
//#define C_FILE_TRAN 175//�ļ�����
//#define C_PARA_TRAN 190//������װ
//#define C_PARA_SET    191//������װ
//#define C_CTL_MON_ON	192	/*���ƶ˿ڱ��ļ��ӿ�*/
//#define C_CTL_MON_OFF	193 /*���ƶ˿ڱ��ļ��ӹ�*/
#define C_FILE_TRAN 250//175//�ļ�����
#define C_PARA_TRAN 251//190//������װ
#define C_PARA_SET    252//191//������װ
#define C_CTL_MON_ON	253	/*���ƶ˿ڱ��ļ��ӿ�*/
#define C_CTL_MON_OFF	254 /*���ƶ˿ڱ��ļ��ӹ�*/

#define C_END_ACT 4
#define C_NULL 0
#define C_CON_E5 1
#define C_CON_ACT 2
#define C_DATA_TRAN 3
#define C_RCU_NA_2    0x5 // ��λͨ�ŵ�Ԫ֡����λ
#define C_RLK_NA_3    0x6 // ������·״̬
#define C_PL1_NA_2    0x7 // �ٻ�1���û�����
#define C_PL2_NA_2    0x8 // �ٻ�2���û�����
#define C_Soft_Reset	       194	/*ϵͳ�����λ*/


class CBASE102:public CProtocol
{
public:
		CBASE102();
		~CBASE102();
		 void SendProc();
		 int ReciProc();
protected:
	stMtrConfig *mtrConfig;
	stSIOPlanpara *sioplanConfig;
	struct stSyspara *sysConfig;
	class CErtudbase e_EDbase;
	struct stControlpara *controlConfig;
	struct stMonitorpara *monitorConfig;
	struct stNetpara *netConfig;
	unsigned char *Remote_Para_Changed;
	unsigned char *SoftVersion;
	unsigned char *Reboot_Flag;
protected:
	
	unsigned short M_MainFile_CRC_Check(unsigned char * data, unsigned char len);
	unsigned char M_Write_Main_File(unsigned char * data);
	int M_Soft_Update_Can();
	void M_Return_Pin_Error(unsigned char type);
	/*-----------------------------------
	ά���ӿ��ϴ�����
-------------------------------------*/
	 int M_Save_LP_Para_NA2();
	 int M_Save_MNT_Para_NA2();
	 int M_Save_Para_NA2();
	 int M_Save_QR_Para_NA2();
	 int M_Save_TA_Para_NA2();
	 int M_Save_TE_Para_NA2();
	 int M_Save_TOU_Para_NA2();
	 int M_Save_PB_Para_NA2();
	int M_Clt_Para_NA2();
	 int M_Ctr_Para_NA2();
	int M_Moin_Para_NA2();
	 int M_Net_Para_NA2();
	unsigned char Is_IP_3(char * data_buf);
	 int M_Sioplan_Para_NA2();
	 int M_Meter_Para_NA2();
	 int M_Sys_Para_NA2();
	 void Ertu_Para_NA2();//061130 he add
	 int M_Soft_SerNum_NA_2();
	int M_HardWare_Status_NA2();
	
/*----------------------------------------
	ά���ӿ���װ����
----------------------------------------------*/
	int M_Cfg_Para_Update_NA2(unsigned char flag);
	int Update_Cfg_File(unsigned char *databuf);
	 unsigned char WriteCfgFlie(std::string filename,unsigned char *databuf,unsigned char cfg_len,unsigned char pro_len );
	 unsigned char m_Calc_Now_Frame_Seq(unsigned char *data_buf);
	 unsigned char M_CTL_PORT_Open();
	 unsigned char M_CTL_PORT_Close();
	/*102 ֡����*/
	void Clear_Continue_Flag();
	void Clear_FrameFlags();
	void E5H_Yes();
	void M_NV_NA_2(unsigned char flag);
	unsigned char Pro102_check_sum(unsigned char* data,unsigned char len);
	void C_RCU_NAF();

	int Realmtr2Retranmtr(unsigned char realmtr);//����ʵ���ת���õ��ڼ���ת�����еı��
	//����ת������
	unsigned char m_Max_Mtrnum;
     	std::vector<unsigned char> m_retransmit_table;
	unsigned char retran_table_valid;
	/*-----------------------------------*/
	//����֡�����Ϣ
	unsigned char  c_FCV;
	unsigned char  c_FCV_Temp;
	unsigned char  c_FCB;
	unsigned char  c_FCB_Tmp;
	unsigned char  c_func;
	unsigned char  c_func_tmp;
	unsigned char c_prm;
	
	unsigned short c_Start_Info;
	unsigned short c_Stop_Info;
	unsigned char  c_Start_MIN;
	unsigned char  c_Start_H;
	unsigned char  c_Start_D;
	unsigned char  c_Start_MON;
	unsigned char  c_Start_YL;
	
	unsigned char  c_End_MIN;
	unsigned char  c_End_H;
	unsigned char  c_End_D;
	unsigned char  c_End_MON;
	unsigned char  c_End_YL;
	/*heshnghan add */
	unsigned int  c_HisSendT;
	unsigned int  c_HisStartT;
	unsigned int  c_CircleTime;
	std::string       filename;

	unsigned char  c_Dev_Address_L;
	unsigned char  c_Dev_Address_H;
	unsigned char  c_Link_Address_L;
	unsigned char  c_Link_Address_H;
	unsigned char  c_COT;
	unsigned char  c_TI;
	unsigned char  c_TI_tmp;
	unsigned char  c_Record_Addr;
//���Ľ��տ���֡	
	unsigned char  syn_char_num;
	unsigned char  expect_charnum;
	unsigned char  Syn_Head_Rece_Flag;	
//����֡�����Ϣ
	unsigned char  m_ACD;
	unsigned char m_Resend;
	unsigned char m_checktime_valifalg;
	unsigned char m_VSQ;
	unsigned char m_VSQ_tmp;//061130 he add
	unsigned char m_TI;
	unsigned char m_COT;
	unsigned char m_COT_tmp;
	unsigned short m_IOA;
	unsigned char info_overflow;
/*------------------------------------------------------
	��Լ��������
--------------------------------------------------------*/
/*------------------------------------------------------
	֡ת����������:ACT/DATA/ACTEND   
--------------------------------------------------------*/
	unsigned char  Command;
/*------------------------------------------------------
	��Ҫ���͵�����Ϣ����
--------------------------------------------------------*/
	unsigned int  Send_Total;
/*------------------------------------------------------
	ÿ֡�ɷ��͵������Ϣ����
--------------------------------------------------------*/
	unsigned int  Send_num;
/*------------------------------------------------------
	������־����ÿ����վ������ʼʱ���� ��������ʼʱ��1
--------------------------------------------------------*/
	unsigned char  Continue_Flag;
/*------------------------------------------------------
	���ݷ�����ϱ�־
--------------------------------------------------------*/
	unsigned char  Send_DataEnd;
/*------------------------------------------------------
	ĳ��ʱ����������Ϣ�㷢�������Ҫ����֡��
--------------------------------------------------------*/
	unsigned int  Send_Times;
	unsigned int  Send_RealTimes;
/*------------------------------------------------------
	����ʱ��㷢�������Ҫ���ܲ���
--------------------------------------------------------*/	
	unsigned int Steps;
	unsigned int  SendT_RealTimes;
	unsigned int  Continue_Step_Flag;
/*------------------------------------------------------
	��Ϣ���С
-------------------------------------------------------*/
	unsigned char  Info_Size;
/*----------------------------------------------------
	�ط����ƻ���
------------------------------------------------------*/
	int m_LastSendBytes;
/*------------------------------------------------------
	ά�����ƹ����е�ǰ֡���061130  he add	
-----------------------------------------------------------*/
unsigned short m_Now_Frame_Seq;
unsigned short m_Now_Frame_Seq_tmp;
unsigned short m_Frame_Total;

/*------------------------------------------------------
	ά���ɹ���ʧ�ܱ��061130  he add	
-----------------------------------------------------------*/
unsigned char m_Update_Flag;
/*----------------------------------------------------
	 ����ͨ������
-------------------------------------------*/
unsigned short m_suppwd;
unsigned short m_pwd1;
unsigned short m_pwd2;
unsigned char  mirror_buf[256];

};
#endif
