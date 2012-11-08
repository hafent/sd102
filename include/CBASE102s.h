#ifndef CBASE102_H
#define CBASE102_H

#include "protocol.h"
#include "HisDB.h"
//控制方向
//#define C_FILE_TRAN 175//文件传输
//#define C_PARA_TRAN 190//参数上装
//#define C_PARA_SET    191//参数下装
//#define C_CTL_MON_ON	192	/*控制端口报文监视开*/
//#define C_CTL_MON_OFF	193 /*控制端口报文监视关*/
#define C_FILE_TRAN 250//175//文件传输
#define C_PARA_TRAN 251//190//参数上装
#define C_PARA_SET    252//191//参数下装
#define C_CTL_MON_ON	253	/*控制端口报文监视开*/
#define C_CTL_MON_OFF	254 /*控制端口报文监视关*/

#define C_END_ACT 4
#define C_NULL 0
#define C_CON_E5 1
#define C_CON_ACT 2
#define C_DATA_TRAN 3
#define C_RCU_NA_2    0x5 // 复位通信单元帧记数位
#define C_RLK_NA_3    0x6 // 请求链路状态
#define C_PL1_NA_2    0x7 // 召唤1级用户数据
#define C_PL2_NA_2    0x8 // 召唤2级用户数据
#define C_Soft_Reset	       194	/*系统软件复位*/


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
	维护接口上传参数
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
	维护接口下装参数
----------------------------------------------*/
	int M_Cfg_Para_Update_NA2(unsigned char flag);
	int Update_Cfg_File(unsigned char *databuf);
	 unsigned char WriteCfgFlie(std::string filename,unsigned char *databuf,unsigned char cfg_len,unsigned char pro_len );
	 unsigned char m_Calc_Now_Frame_Seq(unsigned char *data_buf);
	 unsigned char M_CTL_PORT_Open();
	 unsigned char M_CTL_PORT_Close();
	/*102 帧处理*/
	void Clear_Continue_Flag();
	void Clear_FrameFlags();
	void E5H_Yes();
	void M_NV_NA_2(unsigned char flag);
	unsigned char Pro102_check_sum(unsigned char* data,unsigned char len);
	void C_RCU_NAF();

	int Realmtr2Retranmtr(unsigned char realmtr);//由真实表号转换得到在监视转发表中的表号
	//监视转发所需
	unsigned char m_Max_Mtrnum;
     	std::vector<unsigned char> m_retransmit_table;
	unsigned char retran_table_valid;
	/*-----------------------------------*/
	//控制帧相关信息
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
//报文接收控制帧	
	unsigned char  syn_char_num;
	unsigned char  expect_charnum;
	unsigned char  Syn_Head_Rece_Flag;	
//监视帧相关信息
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
	规约控制流程
--------------------------------------------------------*/
/*------------------------------------------------------
	帧转换控制命令:ACT/DATA/ACTEND   
--------------------------------------------------------*/
	unsigned char  Command;
/*------------------------------------------------------
	需要发送的总信息体数
--------------------------------------------------------*/
	unsigned int  Send_Total;
/*------------------------------------------------------
	每帧可发送的最大信息体数
--------------------------------------------------------*/
	unsigned int  Send_num;
/*------------------------------------------------------
	续传标志，在每个主站命令起始时清零 ，续传开始时置1
--------------------------------------------------------*/
	unsigned char  Continue_Flag;
/*------------------------------------------------------
	数据发送完毕标志
--------------------------------------------------------*/
	unsigned char  Send_DataEnd;
/*------------------------------------------------------
	某个时间点的所有信息点发送完毕需要的总帧数
--------------------------------------------------------*/
	unsigned int  Send_Times;
	unsigned int  Send_RealTimes;
/*------------------------------------------------------
	所有时间点发送完毕需要的总步数
--------------------------------------------------------*/	
	unsigned int Steps;
	unsigned int  SendT_RealTimes;
	unsigned int  Continue_Step_Flag;
/*------------------------------------------------------
	信息体大小
-------------------------------------------------------*/
	unsigned char  Info_Size;
/*----------------------------------------------------
	重发控制机制
------------------------------------------------------*/
	int m_LastSendBytes;
/*------------------------------------------------------
	维护控制过程中当前帧序号061130  he add	
-----------------------------------------------------------*/
unsigned short m_Now_Frame_Seq;
unsigned short m_Now_Frame_Seq_tmp;
unsigned short m_Frame_Total;

/*------------------------------------------------------
	维护成功或失败标记061130  he add	
-----------------------------------------------------------*/
unsigned char m_Update_Flag;
/*----------------------------------------------------
	 监视通道密码
-------------------------------------------*/
unsigned short m_suppwd;
unsigned short m_pwd1;
unsigned short m_pwd2;
unsigned char  mirror_buf[256];

};
#endif
