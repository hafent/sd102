/***********************************************************
	何升韩20070730
	定义该类使得参数设置和传递能让不同
	的102规约调用,使代码重用
************************************************************/
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include "define.h"
#include "sys_utl.h"
#include "HisDB.h"
#include "CBASE102s.h"
#include "loopbuf.h"
#include "unistd.h"
#include "metershm.h"

CBASE102::CBASE102():CProtocol()
{
	METER_CShareMemory metershm;
	mtrConfig = metershm.GetMtrConfig();
	sioplanConfig = metershm.GetSIOPlanpara();
	sysConfig = metershm.GetSysConfig();
	controlConfig = metershm.GetCtlConfig();
	monitorConfig = metershm.GetMonConfig();
	netConfig = metershm.GetNetConfig();
	Remote_Para_Changed = metershm.GetParaStatus();
	Reboot_Flag = Remote_Para_Changed+1;
	*Remote_Para_Changed = 0;
	SoftVersion = metershm.GetSoftVersion();
	syn_char_num=4;
	Syn_Head_Rece_Flag=0;
	m_ACD=0;
	info_overflow=0;
	Command=0;
	e_EDbase.Init_SaveCfg(sysConfig->meter_num);
	
}
CBASE102::~CBASE102()
{
	
}
void CBASE102::Clear_Continue_Flag()
{
	Command=C_END_ACT;
	Continue_Flag=0;
	Send_DataEnd=0;
	Continue_Step_Flag=0;
	c_TI_tmp=0xff;
	c_FCB_Tmp=0xff;
	m_TI=0;
	Info_Size=0;
	m_Now_Frame_Seq=0;
	info_overflow=0;
}
void CBASE102::Clear_FrameFlags()
{
	Command=0;
	c_TI=0;
	c_TI_tmp=0xff;	
	c_FCB_Tmp=0xff;
	m_ACD=0;
	m_Resend=0;
	m_Now_Frame_Seq=0;
	info_overflow=0;
}
void CBASE102:: E5H_Yes()
{
	m_transBuf.m_transceiveBuf[0]=0xe5;
	m_transBuf.m_transCount=1;	
	m_LastSendBytes=m_transBuf.m_transCount;
	return;
}

void CBASE102:: M_NV_NA_2(unsigned char flag)
{
	unsigned char i;

	m_transBuf.m_transceiveBuf[0]=0x10;
	if(flag==0){//复位链路
		m_transBuf.m_transceiveBuf[1]=0x20;
		
		}
	else if(1==flag)
		m_transBuf.m_transceiveBuf[1]=0x0b;
	else{
		m_transBuf.m_transceiveBuf[1]=0x29;//0x09;		
	}
	if(m_ACD){
			m_transBuf.m_transceiveBuf[1]|=0x20;
		}
	m_transBuf.m_transceiveBuf[2]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[3]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[4]=0;
	for(i=1;i<4;i++)
	m_transBuf.m_transceiveBuf[4]+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[5]=0x16;
	m_transBuf.m_transCount=6;
	m_LastSendBytes=m_transBuf.m_transCount;
	return;
}

unsigned char CBASE102::Pro102_check_sum(unsigned char* data,unsigned char len)
{
	unsigned char  CS,i;
	CS=0;
	if(data[0]==0x10)	{
		for(i=1;i<4;i++)
			CS+=data[i];
	}
	else if(data[0]==0x68)	{
		for(i=4;i<len-2;i++)
			CS+=data[i];
	}
	if ((data[len-1]==0x16) && (data[len-2]==CS))	{
		return 0;
	}
	else	{
		
		return 1;
	}	
	return 1;
}
/*复位链路*/
void CBASE102::C_RCU_NAF()
{
	m_ACD=0;
	Command=C_NULL;
	c_TI=0;
	c_TI_tmp=0xff;
	c_FCB_Tmp=0xff;
	m_Resend=0;
	m_Now_Frame_Seq=0;
	info_overflow=0;
}
unsigned short CBASE102::M_MainFile_CRC_Check(unsigned char *data,unsigned char len)
{
	unsigned short CRC,flag;
	unsigned char i,j;
	
	CRC=0;
	for(i=0;i<len;i++){
		CRC^=(data[i]<<8);
		for(j=0;j<8;j++){
			flag=CRC & 0x8000;
			CRC<<=1;
			if(flag){
				CRC^=0x1021;
			}
		}
	}
	return	CRC;
}

unsigned char CBASE102::M_Write_Main_File(unsigned char *data)
{	
	unsigned char i,j,ret;
	FILE* fp;
	unsigned char CRCH,CRCL;
	unsigned short CRC;
	unsigned char  data_len;
	
	//m_Now_Frame_Seq=*(data+13);
	if(m_Now_Frame_Seq==m_Now_Frame_Seq_tmp)m_Resend++;
	m_Now_Frame_Seq_tmp=m_Now_Frame_Seq;
	data_len=*(data+1)-17;
	CRCH=*(data+18+data_len+1);
	CRCL=*(data+18+data_len+2);
	CRC=M_MainFile_CRC_Check(data+19,data_len);
	//printf("In M_Wiet_Main_File,data_len=%d,CRC=%04x,CRCH=%02x,CRCL=%02x\n",data_len,CRC,CRCH,CRCL);
	if((CRCH<<8|CRCL)!=CRC){
			//m_Update_Flag=0;
			return 0;
		}
	else{
			if(m_Now_Frame_Seq==0)
					fp=fopen(MainFilebak,"wb");
			else
					fp=fopen(MainFilebak,"rb+");
			if(fp==NULL){
				//m_Update_Flag=0;
				return 0;
				}
			else{//start write
					//fseek(fp,0,SEEK_END);
					fseek(fp,m_Now_Frame_Seq*Per_MainFile_Len,0);
					ret=fwrite(data+19,data_len,1,fp);
					printf("~~~fwrite ret = %d~~~\n",ret);
					fflush(fp);
					fclose(fp);					
					return (ret?1:0);
				}
				
		}
	
}

/*****************************************************
			上传终端板件运行状态
********************************************************/
int CBASE102::M_HardWare_Status_NA2()
{
	unsigned char i,buffptr,ret_val,frm_sum,info_sum;
	char *channel_status;
	METER_CShareMemory meter_shm;
	channel_status = meter_shm.GetChannelStatus();	
	if(Send_DataEnd)
	{
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	m_TI=C_PARA_TRAN;
	m_COT=5;
	m_VSQ=1;
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=27;
	m_transBuf.m_transceiveBuf[buffptr++]=27;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	m_transBuf.m_transceiveBuf[buffptr++]=1;
	memcpy(m_transBuf.m_transceiveBuf+buffptr,&channel_status[0],16);
	buffptr+=16;
	m_transBuf.m_transceiveBuf[buffptr++]=1<<4+(1);
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	Send_DataEnd=1;
	return 0;
}

/*****************************************************
			上传软件版本号(年月日)
********************************************************/
int CBASE102::M_Soft_SerNum_NA_2()
{
	unsigned char i,buffptr,ret_val,frm_sum,info_sum;
	//unsigned char tmp_buf[7];
	if(Send_DataEnd)
	{
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	m_TI=C_PARA_TRAN;
	m_COT=5;
	m_VSQ=1;
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=0x0e;
	m_transBuf.m_transceiveBuf[buffptr++]=0x0e;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	m_transBuf.m_transceiveBuf[buffptr++]=SoftVersion[0];
	m_transBuf.m_transceiveBuf[buffptr++]=SoftVersion[1];///1.0	
	m_transBuf.m_transceiveBuf[buffptr++]=SoftVersion[2];///1.0	
	m_transBuf.m_transceiveBuf[buffptr++]=SoftVersion[3];///1.0	
	m_transBuf.m_transceiveBuf[buffptr++]=SoftVersion[4];///1.0
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	Send_DataEnd=1;
	
	return 0;
	
}
/*************************************上传参数******************************************/
/* M_TI=190
*RAD=0   (由此来区别需要传送的参数 )
----------------------------------------------------------------------------------*/
int CBASE102::M_Sys_Para_NA2()
{
	std::string filename;
	unsigned char i,buffptr,ret_val,frm_sum,info_sum;
	unsigned char tmp_buf[7];

	if(Send_DataEnd)
	{
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	
	m_TI=C_PARA_TRAN;
	m_COT=5;
	m_VSQ=1;
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=0x12;
	m_transBuf.m_transceiveBuf[buffptr++]=0x12;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	m_transBuf.m_transceiveBuf[buffptr++]=0;//ioa
	m_transBuf.m_transceiveBuf[buffptr++]=sysConfig->meter_num;
	m_transBuf.m_transceiveBuf[buffptr++]=sysConfig->sioports_num;
	m_transBuf.m_transceiveBuf[buffptr++]=sysConfig->netports_num;
	m_transBuf.m_transceiveBuf[buffptr++]=sysConfig->pulse_num;
	m_transBuf.m_transceiveBuf[buffptr++]=sysConfig->monitor_ports;
	m_transBuf.m_transceiveBuf[buffptr++]=sysConfig->control_ports;
	m_transBuf.m_transceiveBuf[buffptr++]=sysConfig->sioplan_num;
	info_sum=0;
	for(i=13;i<buffptr;i++)
		info_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//信息数据校验和
	
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	Send_DataEnd=1;
	return 0;	
}
/*******************参数传递---表计参数****************************
* M_TI=190
*RAD=1   (由此来区别需要传送的参数 )
----------------------------------------------------------------------------------*/
int CBASE102::M_Meter_Para_NA2()
{
	
	unsigned char i,j,buffptr,ret_val,frm_sum,info_sum;
	unsigned char inf_size;
	unsigned short tempV;

	
	
	if(Send_DataEnd){
			Clear_Continue_Flag();
			E5H_Yes();
			return -1;
		}
	if(Continue_Flag)goto Meter_Para_Send;
	m_TI=C_PARA_TRAN;
	m_COT=5;
	if(c_Stop_Info>=sysConfig->meter_num-1){
		c_Stop_Info=sysConfig->meter_num-1;
	}
	if(c_Stop_Info<c_Start_Info){
		Clear_Continue_Flag();
		E5H_Yes();
		return -2;
	}
	Send_Total=c_Stop_Info-c_Start_Info+1;
	if(Send_Total>=sysConfig->meter_num){
		Send_Total=sysConfig->meter_num;
	}
	m_IOA=c_Start_Info;
	Send_num=(MAXSENDBUF-DL719FIXLEN)/31;
	Send_Times=Send_Total/Send_num;
	if(Send_Total%Send_num)Send_Times++;
	Continue_Flag=0;
	Info_Size=31;//
	
Meter_Para_Send:	
	if(Continue_Flag==0){
		Continue_Flag=1;
		Send_RealTimes=0;
	}
	else{
		Send_RealTimes++;
	}

	if(Send_RealTimes<Send_Times-1){
		m_VSQ=Send_num;
	}
	else{
		m_VSQ=Send_Total-Send_num*Send_RealTimes;
		Continue_Flag=0;
		Send_RealTimes=0;
		Send_DataEnd=1;
	}
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	//printf("======In Meter Trans M_IOA=%d=============\n",m_IOA);
	for(i=0;i<m_VSQ;i++){
		m_transBuf.m_transceiveBuf[buffptr++]=m_IOA;		
		TransBinArray2BcdArray(&mtrConfig[m_IOA].m_Linename[0],&m_transBuf.m_transceiveBuf[buffptr],3);//		
		buffptr+=3;
		TransBinArray2BcdArray(&mtrConfig[m_IOA].m_Meteraddr[0],&m_transBuf.m_transceiveBuf[buffptr],6);
		buffptr+=6;
		TransBinArray2BcdArray(&mtrConfig[m_IOA].m_Meterpwd[0],&m_transBuf.m_transceiveBuf[buffptr],4);
		buffptr+=4;
		m_transBuf.m_transceiveBuf[buffptr++]=mtrConfig[m_IOA].m_commPort;//使用端口
		m_transBuf.m_transceiveBuf[buffptr++]=mtrConfig[m_IOA].m_commPlan; //使用端口方案
		m_transBuf.m_transceiveBuf[buffptr++]=mtrConfig[m_IOA].m_MeterType;//表计类型澹(产家编号)
		m_transBuf.m_transceiveBuf[buffptr++]=mtrConfig[m_IOA].m_MeterProt;//表计规约
		//memcpy(&m_transBuf.m_transceiveBuf[buffptr],&mtrConfig[m_IOA].m_DotNum[0],4);
		//buffptr+=4;
		m_transBuf.m_transceiveBuf[buffptr++]=mtrConfig[m_IOA].m_DLDot;
		m_transBuf.m_transceiveBuf[buffptr++]=mtrConfig[m_IOA].m_XLDot;
		m_transBuf.m_transceiveBuf[buffptr++]=mtrConfig[m_IOA].m_UDot;
		m_transBuf.m_transceiveBuf[buffptr++]=mtrConfig[m_IOA].m_IDot;
		m_transBuf.m_transceiveBuf[buffptr++]=mtrConfig[m_IOA].m_PDot;
		m_transBuf.m_transceiveBuf[buffptr++]=mtrConfig[m_IOA].m_QDot;

		m_transBuf.m_transceiveBuf[buffptr++]=mtrConfig[m_IOA].m_P3L4;
		m_transBuf.m_transceiveBuf[buffptr++]=mtrConfig[m_IOA].m_Ue&0xff;
		m_transBuf.m_transceiveBuf[buffptr++]=(mtrConfig[m_IOA].m_Ue>>8)&0xff;
		tempV=(mtrConfig[m_IOA].m_Ie/100);
		m_transBuf.m_transceiveBuf[buffptr++]=tempV&0xff;
		m_transBuf.m_transceiveBuf[buffptr++]=(tempV>>8)&0xff;
		
		m_transBuf.m_transceiveBuf[buffptr++]=mtrConfig[m_IOA].m_Validflag;
		info_sum=0;
		for(j=0;j<Info_Size-1;j++)
			info_sum+=m_transBuf.m_transceiveBuf[buffptr-Info_Size+1+j];
		m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//data check sum
		m_IOA++;
	}
	
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	return 0;
}

/*******************参数传递---串口方案****************************
* M_TI=190
*RAD=2  
*/
int CBASE102::M_Sioplan_Para_NA2()
{
	
	unsigned char i,j,buffptr,ret_val,frm_sum,info_sum;
	unsigned char inf_size;
	if(m_Resend){
			m_transBuf.m_transCount=m_LastSendBytes;
			m_Resend=0;
			return 0;
		}
	
	if(Send_DataEnd){
			Clear_Continue_Flag();
			E5H_Yes();
			return -1;
		}
	if(Continue_Flag)goto SioPlan_Para_Send;
	m_TI=C_PARA_TRAN;
	m_COT=5;
	if(c_Stop_Info>=sysConfig->sioplan_num-1){
		c_Stop_Info=sysConfig->sioplan_num-1;
	}
	if(c_Stop_Info<c_Start_Info){
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	Send_Total=c_Stop_Info-c_Start_Info+1;
	if(Send_Total>=sysConfig->sioplan_num){
		Send_Total=sysConfig->sioplan_num;
	}
	m_IOA=c_Start_Info;
	Send_num=(MAXSENDBUF-DL719FIXLEN)/7;//
	Send_Times=Send_Total/Send_num;
	if(Send_Total%Send_num)Send_Times++;
	Continue_Flag=0;
	Info_Size=7;//
	
SioPlan_Para_Send:	
	if(Continue_Flag==0){
		Continue_Flag=1;
		Send_RealTimes=0;
	}
	else{
		Send_RealTimes++;
	}

	if(Send_RealTimes<Send_Times-1){
		m_VSQ=Send_num;
	}
	else{
		m_VSQ=Send_Total-Send_num*Send_RealTimes;
		Continue_Flag=0;
		Send_RealTimes=0;
		Send_DataEnd=1;
	}
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	for(i=0;i<m_VSQ;i++){
		m_transBuf.m_transceiveBuf[buffptr++]=m_IOA;		
		m_transBuf.m_transceiveBuf[buffptr++]=sioplanConfig[i].m_Parity;//校验位
		m_transBuf.m_transceiveBuf[buffptr++]=sioplanConfig[i].m_Databits; //数据位
		m_transBuf.m_transceiveBuf[buffptr++]=sioplanConfig[i].m_Stopbits;//停止位
		m_transBuf.m_transceiveBuf[buffptr++]=sioplanConfig[i].m_Baudrate;//波特率
		m_transBuf.m_transceiveBuf[buffptr++]=sioplanConfig[i].m_Commtype;//通信方式	
		info_sum=0;
		for(j=0;j<Info_Size-1;j++)
			info_sum+=m_transBuf.m_transceiveBuf[buffptr-Info_Size+1+j];
		m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//data check sum
		m_IOA++;
	}
	
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	return 0;
}


unsigned char CBASE102::Is_IP_3(char *data_buf)
{
	unsigned char i,count=0;
	for(i=0;i<3;i++){
			if(*(data_buf+i)==0x2e)
				break;
			else 
				{count++;
				//printf(" %02x",*(data_buf+i));
				}
			}
	return count;			
		
}
/*******************参数传递---网络参数****************************
* M_TI=190
*RAD=3 
*/
int CBASE102::M_Net_Para_NA2()
{
	unsigned char i,j,buffptr,ret_val,info_sum,frm_sum;
	unsigned char inf_size,IP_Count,IP_Count1,IP_Count2;

	if(Send_DataEnd){
			Clear_Continue_Flag();
			E5H_Yes();
			return -1;
		}
	if(Continue_Flag)goto Net_Para_Send;
	m_TI=C_PARA_TRAN;
	m_COT=5;
	if(c_Stop_Info>=sysConfig->netports_num){
		c_Stop_Info=sysConfig->netports_num-1;
	}
	if(c_Stop_Info<c_Start_Info){
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	Send_Total=c_Stop_Info-c_Start_Info+1;
	printf("******In Read netpara,Send_Total=%d************\n",Send_Total);
	if(Send_Total>=sysConfig->netports_num){
		Send_Total=sysConfig->netports_num;
	}
	m_IOA=c_Start_Info;
	Send_num=(MAXSENDBUF-DL719FIXLEN)/17;//15=1+1+4+4+4+1
	Send_Times=Send_Total/Send_num;
	if(Send_Total%Send_num)Send_Times++;
	Continue_Flag=0;
	Info_Size=17;//
	
Net_Para_Send:	
	if(Continue_Flag==0){
		Continue_Flag=1;
		Send_RealTimes=0;
	}
	else{
		Send_RealTimes++;
	}

	if(Send_RealTimes<Send_Times-1){
		m_VSQ=Send_num;
	}
	else{
		m_VSQ=Send_Total-Send_num*Send_RealTimes;
		Continue_Flag=0;
		Send_RealTimes=0;
		Send_DataEnd=1;
	}
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	printf("In read netpara ,m_vsq=%d\n",m_VSQ);
	for(i=0;i<m_VSQ;i++)
	{
		m_transBuf.m_transceiveBuf[buffptr++]=m_IOA;	
		m_transBuf.m_transceiveBuf[buffptr++]=netConfig[i].m_Netport;
	//IP
		m_transBuf.m_transceiveBuf[buffptr++]=atoi(&netConfig[i].m_IPstr[0]);
			IP_Count=Is_IP_3(&netConfig[i].m_IPstr[0]);
			
		m_transBuf.m_transceiveBuf[buffptr++]=atoi(&netConfig[i].m_IPstr[1+IP_Count]);
			IP_Count1=Is_IP_3(&netConfig[i].m_IPstr[1+IP_Count]);
			
		m_transBuf.m_transceiveBuf[buffptr++]=atoi(&netConfig[i].m_IPstr[2+IP_Count+IP_Count1]);
			IP_Count2=Is_IP_3(&netConfig[i].m_IPstr[2+IP_Count+IP_Count1]);
			printf("IP IP_count=%d,IP_count1=%d,IP_COunt2=%d\n",IP_Count,IP_Count1,IP_Count2);
		m_transBuf.m_transceiveBuf[buffptr++]=atoi(&netConfig[i].m_IPstr[3+IP_Count+IP_Count1+IP_Count2]);
     //NetMask	
		m_transBuf.m_transceiveBuf[buffptr++]=atoi(&netConfig[i].m_NetMask[0]);
			IP_Count=Is_IP_3(&netConfig[i].m_NetMask[0]);
			
		m_transBuf.m_transceiveBuf[buffptr++]=atoi(&netConfig[i].m_NetMask[1+IP_Count]);
			IP_Count1=Is_IP_3(&netConfig[i].m_NetMask[1+IP_Count]);
			
		m_transBuf.m_transceiveBuf[buffptr++]=atoi(&netConfig[i].m_NetMask[2+IP_Count+IP_Count1]);
			IP_Count2=Is_IP_3(&netConfig[i].m_NetMask[2+IP_Count+IP_Count1]);
		printf("NetMask IP_count=%d,IP_count1=%d,IP_COunt2=%d\n",IP_Count,IP_Count1,IP_Count2);	
		m_transBuf.m_transceiveBuf[buffptr++]=atoi(&netConfig[i].m_NetMask[3+IP_Count+IP_Count1+IP_Count2]);
  //GateWay		
		m_transBuf.m_transceiveBuf[buffptr++]=atoi(&netConfig[i].m_Gateway[0]);
			IP_Count=Is_IP_3(&netConfig[i].m_Gateway[0]);
			
		m_transBuf.m_transceiveBuf[buffptr++]=atoi(&netConfig[i].m_Gateway[1+IP_Count]);
			IP_Count1=Is_IP_3(&netConfig[i].m_Gateway[1+IP_Count]);
			
		m_transBuf.m_transceiveBuf[buffptr++]=atoi(&netConfig[i].m_Gateway[2+IP_Count+IP_Count1]);
			IP_Count2=Is_IP_3(&netConfig[i].m_Gateway[2+IP_Count+IP_Count1]);
		printf("GateWay IP_count=%d,IP_count1=%d,IP_COunt2=%d\n",IP_Count,IP_Count1,IP_Count2);	
		m_transBuf.m_transceiveBuf[buffptr++]=atoi(&netConfig[i].m_Gateway[3+IP_Count+IP_Count1+IP_Count2]);

		m_transBuf.m_transceiveBuf[buffptr++]=0;
		m_transBuf.m_transceiveBuf[buffptr++]=0;
		
		info_sum=0;
		for(j=0;j<Info_Size-1;j++)
			info_sum+=m_transBuf.m_transceiveBuf[buffptr-Info_Size+1+j];
		m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//data check sum
		m_IOA++;
	}
	
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	return 0;
}

/*******************参数传递---监视参数****************************
* M_TI=190
*RAD=4
*/
int CBASE102::M_Moin_Para_NA2()
{
	unsigned char i,j,buffptr,ret_val,info_sum,frm_sum;
	unsigned char inf_size;
	if(m_Resend){
			m_transBuf.m_transCount=m_LastSendBytes;
			m_Resend=0;
			return 0;
		}
	if(Send_DataEnd){
			Clear_Continue_Flag();
			E5H_Yes();
			return -1;
		}
	if(Continue_Flag)goto Moin_Para_Send;
	m_TI=C_PARA_TRAN;
	m_COT=5;
	if(c_Stop_Info>=sysConfig->monitor_ports){
		c_Stop_Info=sysConfig->monitor_ports-1;
	}
	if(c_Stop_Info<c_Start_Info){
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	Send_Total=c_Stop_Info-c_Start_Info+1;
	if(Send_Total>=sysConfig->monitor_ports){
		Send_Total=sysConfig->monitor_ports;
	}
	m_IOA=c_Start_Info;
	Send_num=(MAXSENDBUF-DL719FIXLEN)/140;//
	Send_Times=Send_Total/Send_num;
	if(Send_Total%Send_num)Send_Times++;
	Continue_Flag=0;
	Info_Size=140;//
	
Moin_Para_Send:	
	if(Continue_Flag==0){
		Continue_Flag=1;
		Send_RealTimes=0;
	}
	else{
		Send_RealTimes++;
	}

	if(Send_RealTimes<Send_Times-1){
		m_VSQ=Send_num;
	}
	else{
		m_VSQ=Send_Total-Send_num*Send_RealTimes;
		Continue_Flag=0;
		Send_RealTimes=0;
		Send_DataEnd=1;
	}
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	for(i=0;i<m_VSQ;i++){
		m_transBuf.m_transceiveBuf[buffptr++]=m_IOA;		
		m_transBuf.m_transceiveBuf[buffptr++]=monitorConfig[m_IOA].m_commPort;
		m_transBuf.m_transceiveBuf[buffptr++]=(unsigned char)(monitorConfig[m_IOA].m_listenPort)&0xff;
		m_transBuf.m_transceiveBuf[buffptr++]=(unsigned char)(monitorConfig[m_IOA].m_listenPort>>8)&0xff;
		m_transBuf.m_transceiveBuf[buffptr++]=monitorConfig[m_IOA].m_commPlan;
		m_transBuf.m_transceiveBuf[buffptr++]=monitorConfig[m_IOA].m_protType;
		m_transBuf.m_transceiveBuf[buffptr++]=(unsigned char)monitorConfig[m_IOA].m_portAddr&0xff;
		m_transBuf.m_transceiveBuf[buffptr++]=(unsigned char)(monitorConfig[m_IOA].m_portAddr>>8)&0xff;//终端地址
		//for(j=0;j<3;j++){
		//m_transBuf.m_transceiveBuf[buffptr++]=(unsigned char)(monitorConfig[m_IOA].m_portPwd[0])&0xff;
		//m_transBuf.m_transceiveBuf[buffptr++]=(unsigned char)(monitorConfig[m_IOA].m_portPwd[0]>>8)&0xff;
		//}//口令
		m_transBuf.m_transceiveBuf[buffptr++]=monitorConfig[m_IOA].m_check_time_Valiflag;
		m_transBuf.m_transceiveBuf[buffptr++]=monitorConfig[m_IOA].m_retransmit_flag;
		m_transBuf.m_transceiveBuf[buffptr++]=monitorConfig[m_IOA].m_retransmit_mtrnum;
		memset(m_transBuf.m_transceiveBuf+buffptr,0,128);

		if(monitorConfig[m_IOA].m_retransmit_flag==1)
		{
			for(j=0;j<monitorConfig[i].m_retransmit_mtrnum;j++)
			{
				m_transBuf.m_transceiveBuf[buffptr+j]=monitorConfig[m_IOA].m_retransmit_mtr[j];
			}
			
		}

		buffptr+=128;
		
		info_sum=0;
		for(j=0;j<Info_Size-1;j++)
			info_sum+=m_transBuf.m_transceiveBuf[buffptr-Info_Size+1+j];
		m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//data check sum
		m_IOA++;
	}
	
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	return 0;
}

/*******************参数传递---控制参数****************************
* M_TI=190
*RAD=5
*/
int CBASE102::M_Ctr_Para_NA2()
{
	unsigned char i,j,buffptr,ret_val,info_sum,frm_sum;
	unsigned char inf_size;
	if(m_Resend){
			m_transBuf.m_transCount=m_LastSendBytes;
			m_Resend=0;
			return 0;
		}
	if(Send_DataEnd){
			Clear_Continue_Flag();
			E5H_Yes();
			return -1;
		}
	if(Continue_Flag)goto Ctr_Para_Send;
	m_TI=C_PARA_TRAN;
	m_COT=5;
	if(c_Stop_Info>=sysConfig->control_ports){
		c_Stop_Info=sysConfig->control_ports-1;
	}
	if(c_Stop_Info<c_Start_Info){
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	Send_Total=c_Stop_Info-c_Start_Info+1;
	if(Send_Total>=sysConfig->control_ports){
		Send_Total=sysConfig->control_ports;
	}
	m_IOA=c_Start_Info;
	Send_num=(MAXSENDBUF-DL719FIXLEN)/3;//
	Send_Times=Send_Total/Send_num;
	if(Send_Total%Send_num)Send_Times++;
	Continue_Flag=0;
	Info_Size=3;//
	
Ctr_Para_Send:	
	if(Continue_Flag==0){
		Continue_Flag=1;
		Send_RealTimes=0;
	}
	else{
		Send_RealTimes++;
	}

	if(Send_RealTimes<Send_Times-1){
		m_VSQ=Send_num;
	}
	else{
		m_VSQ=Send_Total-Send_num*Send_RealTimes;
		Continue_Flag=0;
		Send_RealTimes=0;
		Send_DataEnd=1;
	}
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	for(i=0;i<m_VSQ;i++){
		m_transBuf.m_transceiveBuf[buffptr++]=m_IOA;		
		m_transBuf.m_transceiveBuf[buffptr++]=controlConfig[i].m_commPort;	
		info_sum=0;
		for(j=0;j<Info_Size-1;j++)
			info_sum+=m_transBuf.m_transceiveBuf[buffptr-Info_Size+1+j];
		m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//data check sum
		m_IOA++;
	}	
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	return 0;
}

/*******************参数传递--采集任务****************************
* M_TI=190
*RAD=6
*直接读取配置文件
*/
int CBASE102::M_Clt_Para_NA2()
{
	
	std::string filename;
	unsigned char i,j,buffptr,ret_val,info_sum,frm_sum;
	unsigned char tmp_buf[20];
	filename="ctspara.cfg";
	ret_val=GetAbsolutePath(&filename, CFGPATH);
	ret_val=GetCfgFromDB(filename.c_str(), tmp_buf, sizeof(stCollectSave_para));
	if(m_Resend){
			m_transBuf.m_transCount=m_LastSendBytes;
			m_Resend=0;
			return 0;
		}
	m_TI=C_PARA_TRAN;
	m_COT=5;
	m_VSQ=1;
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=9+20;
	m_transBuf.m_transceiveBuf[buffptr++]=9+20;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	m_transBuf.m_transceiveBuf[buffptr++]=0;//ioa

	memcpy(&m_transBuf.m_transceiveBuf[buffptr],&tmp_buf[0],18);
	buffptr+=18;
	info_sum=0;
	for(j=13;j<buffptr;j++)
		info_sum+=m_transBuf.m_transceiveBuf[j];
	m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//信息数据校验和
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
		
	Clear_Continue_Flag();
	return 0;
}

/*******************参数传递--存储任务****************************
* M_TI=190
*RAD=7
*直接读取配置文件
*/
int CBASE102::M_Save_Para_NA2()
{
	
	std::string filename;
	unsigned char i,j,buffptr,ret_val,info_sum,frm_sum;
	unsigned char tmp_buf[Sav_Cfg_Len];
	filename="stspara.cfg";
	ret_val=GetAbsolutePath(&filename, CFGPATH);
	ret_val=GetCfgFromDB(filename.c_str(), tmp_buf, Sav_Cfg_Len);

	if(Send_DataEnd)
	{
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	
	m_TI=C_PARA_TRAN;
	m_COT=5;
	m_VSQ=1;
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=9+14;
	m_transBuf.m_transceiveBuf[buffptr++]=9+14;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	m_transBuf.m_transceiveBuf[buffptr++]=0;//ioa

	memcpy(&m_transBuf.m_transceiveBuf[buffptr],&tmp_buf[0],Sav_Cfg_Len);
	buffptr+=Sav_Cfg_Len;
	info_sum=0;
	for(j=13;j<buffptr;j++)
		info_sum+=m_transBuf.m_transceiveBuf[j];
	m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//信息数据校验和
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	Send_DataEnd=1;
	return 0;
}
/*******************参数传递-存储项目(TE)****************************
* M_TI=190
*RAD=81
*/
int CBASE102::M_Save_TE_Para_NA2()
{
	
	unsigned char i,j,buffptr,ret_val,frm_sum,info_sum;
	unsigned char inf_size;
	unsigned char savepara[5];
	if(m_Resend){
			m_transBuf.m_transCount=m_LastSendBytes;
			m_Resend=0;
			return 0;
		}
	if(Send_DataEnd){
			Clear_Continue_Flag();
			E5H_Yes();
			return -1;
		}
	if(Continue_Flag)goto Save_TE_Para_Send;
	m_TI=C_PARA_TRAN;
	m_COT=5;
	if(c_Stop_Info>=sysConfig->meter_num){
		c_Stop_Info=sysConfig->meter_num-1;
	}
	if(c_Stop_Info<c_Start_Info){
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	Send_Total=c_Stop_Info-c_Start_Info+1;
	if(Send_Total>=sysConfig->meter_num){
		Send_Total=sysConfig->meter_num;
	}
	m_IOA=c_Start_Info;
	Send_num=(MAXSENDBUF-DL719FIXLEN)/7;//
	Send_Times=Send_Total/Send_num;
	if(Send_Total%Send_num)Send_Times++;
	Continue_Flag=0;
	Info_Size=7;//
	
Save_TE_Para_Send:	
	if(Continue_Flag==0){
		Continue_Flag=1;
		Send_RealTimes=0;
	}
	else{
		Send_RealTimes++;
	}

	if(Send_RealTimes<Send_Times-1){
		m_VSQ=Send_num;
	}
	else{
		m_VSQ=Send_Total-Send_num*Send_RealTimes;
		Continue_Flag=0;
		Send_RealTimes=0;
		Send_DataEnd=1;
	}
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	for(i=0;i<m_VSQ;i++){
		m_transBuf.m_transceiveBuf[buffptr++]=m_IOA;
		e_EDbase.get_save_para(savepara, m_IOA, TASK_TE);
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[0];
		for(j=0;j<4;j++){
			m_transBuf.m_transceiveBuf[buffptr++]=(savepara[1]>>j)&0x01;
		}//TE
		info_sum=0;
		for(j=0;j<Info_Size-1;j++)
			info_sum+=m_transBuf.m_transceiveBuf[buffptr-Info_Size+1+j];
		m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//data check sum
		m_IOA++;
	}	
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	return 0;
}

/*******************参数传递-存储项目(TOU)****************************
* M_TI=190
*RAD=82

*/
int CBASE102::M_Save_TOU_Para_NA2()
{
	
	unsigned char i,j,buffptr,ret_val,frm_sum,info_sum;
	unsigned char  savepara[5];
	if(m_Resend){
			m_transBuf.m_transCount=m_LastSendBytes;
			m_Resend=0;
			return 0;
		}
	if(Send_DataEnd){
			Clear_Continue_Flag();
			E5H_Yes();
			return -1;
		}
	if(Continue_Flag)goto Save_TOU_Para_Send;
	m_TI=C_PARA_TRAN;
	m_COT=5;
	if(c_Stop_Info>=sysConfig->meter_num){
		c_Stop_Info=sysConfig->meter_num-1;
	}
	if(c_Stop_Info<c_Start_Info){
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	Send_Total=c_Stop_Info-c_Start_Info+1;
	if(Send_Total>=sysConfig->meter_num){
		Send_Total=sysConfig->meter_num;
	}
	m_IOA=c_Start_Info;
	Send_num=(MAXSENDBUF-DL719FIXLEN)/7;//
	Send_Times=Send_Total/Send_num;
	if(Send_Total%Send_num)Send_Times++;
	Continue_Flag=0;
	Info_Size=7;//
	
Save_TOU_Para_Send:	
	if(Continue_Flag==0){
		Continue_Flag=1;
		Send_RealTimes=0;
	}
	else{
		Send_RealTimes++;
	}

	if(Send_RealTimes<Send_Times-1){
		m_VSQ=Send_num;
	}
	else{
		m_VSQ=Send_Total-Send_num*Send_RealTimes;
		Continue_Flag=0;
		Send_RealTimes=0;
		Send_DataEnd=1;
	}
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	for(i=0;i<m_VSQ;i++){
		m_transBuf.m_transceiveBuf[buffptr++]=m_IOA;
		e_EDbase.get_save_para(savepara,m_IOA, TASK_TOU);
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[0];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[1];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[2];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[3];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[4];//tou	
		info_sum=0;
		for(j=0;j<Info_Size-1;j++)
			info_sum+=m_transBuf.m_transceiveBuf[buffptr-Info_Size+1+j];
		m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//data check sum
		m_IOA++;
	}	
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	return 0;
}

/*******************参数传递-存储项目(QR)****************************
* M_TI=190
*RAD=83

*/
int CBASE102::M_Save_QR_Para_NA2()
{
	
	unsigned char i,j,buffptr,ret_val,frm_sum,info_sum;
	unsigned char inf_size,savepara[5];
	if(m_Resend){
			m_transBuf.m_transCount=m_LastSendBytes;
			m_Resend=0;
			return 0;
		}
	if(Send_DataEnd){
			Clear_Continue_Flag();
			E5H_Yes();
			return -1;
		}
	if(Continue_Flag)goto Save_QR_Para_Send;
	m_TI=C_PARA_TRAN;
	m_COT=5;
	if(c_Stop_Info>=sysConfig->meter_num){
		c_Stop_Info=sysConfig->meter_num-1;
	}
	if(c_Stop_Info<c_Start_Info){
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	Send_Total=c_Stop_Info-c_Start_Info+1;
	if(Send_Total>=sysConfig->meter_num){
		Send_Total=sysConfig->meter_num;
	}
	m_IOA=c_Start_Info;
	Send_num=(MAXSENDBUF-DL719FIXLEN)/7;//
	Send_Times=Send_Total/Send_num;
	if(Send_Total%Send_num)Send_Times++;
	Continue_Flag=0;
	Info_Size=7;//
	
Save_QR_Para_Send:	
	if(Continue_Flag==0){
		Continue_Flag=1;
		Send_RealTimes=0;
	}
	else{
		Send_RealTimes++;
	}

	if(Send_RealTimes<Send_Times-1){
		m_VSQ=Send_num;
	}
	else{
		m_VSQ=Send_Total-Send_num*Send_RealTimes;
		Continue_Flag=0;
		Send_RealTimes=0;
		Send_DataEnd=1;
	}
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	for(i=0;i<m_VSQ;i++){
		m_transBuf.m_transceiveBuf[buffptr++]=m_IOA;
		e_EDbase.get_save_para(savepara, m_IOA, TASK_QR);
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[0];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[1];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[2];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[3];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[4];//qr	
		info_sum=0;
		for(j=0;j<Info_Size-1;j++)
			info_sum+=m_transBuf.m_transceiveBuf[buffptr-Info_Size+1+j];
		m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//data check sum
		m_IOA++;
	}	
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	return 0;
}
/*******************参数传递-存储项目(MNT)****************************
* M_TI=190
*RAD=84

*/
int CBASE102::M_Save_MNT_Para_NA2()
{
	
	unsigned char i,j,buffptr,ret_val,frm_sum,info_sum;
	unsigned char inf_size,savepara[5];
	if(m_Resend){
			m_transBuf.m_transCount=m_LastSendBytes;
			m_Resend=0;
			return 0;
		}
	if(Send_DataEnd){
			Clear_Continue_Flag();
			E5H_Yes();
			return -1;
		}
	if(Continue_Flag)goto Save_MNT_Para_Send;
	m_TI=C_PARA_TRAN;
	m_COT=5;
	if(c_Stop_Info>=sysConfig->meter_num){
		c_Stop_Info=sysConfig->meter_num-1;
	}
	if(c_Stop_Info<c_Start_Info){
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	Send_Total=c_Stop_Info-c_Start_Info+1;
	if(Send_Total>=sysConfig->meter_num){
		Send_Total=sysConfig->meter_num;
	}
	m_IOA=c_Start_Info;
	Send_num=(MAXSENDBUF-DL719FIXLEN)/7;//
	Send_Times=Send_Total/Send_num;
	if(Send_Total%Send_num)Send_Times++;
	Continue_Flag=0;
	Info_Size=7;//
	
Save_MNT_Para_Send:	
	if(Continue_Flag==0){
		Continue_Flag=1;
		Send_RealTimes=0;
	}
	else{
		Send_RealTimes++;
	}

	if(Send_RealTimes<Send_Times-1){
		m_VSQ=Send_num;
	}
	else{
		m_VSQ=Send_Total-Send_num*Send_RealTimes;
		Continue_Flag=0;
		Send_RealTimes=0;
		Send_DataEnd=1;
	}
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	for(i=0;i<m_VSQ;i++){
		m_transBuf.m_transceiveBuf[buffptr++]=m_IOA;
		e_EDbase.get_save_para(savepara, m_IOA, TASK_MNT);
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[0];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[1];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[2];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[3];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[4];//mnt	
		info_sum=0;
		for(j=0;j<Info_Size-1;j++)
			info_sum+=m_transBuf.m_transceiveBuf[buffptr+1-Info_Size+j];
		m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//data check sum
		m_IOA++;
	}	
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	return 0;
}
/*******************参数传递-存储项目(TA)****************************
* M_TI=190
*RAD=85

*/
int CBASE102::M_Save_TA_Para_NA2()
{
	
	unsigned char i,j,buffptr,ret_val,info_sum,frm_sum;
	unsigned char inf_size,savepara[5];
	if(m_Resend){
			m_transBuf.m_transCount=m_LastSendBytes;
			m_Resend=0;
			return 0;
		}
	if(Send_DataEnd){
			Clear_Continue_Flag();
			E5H_Yes();
			return -1;
		}
	if(Continue_Flag)goto Save_TA_Para_Send;
	m_TI=C_PARA_TRAN;
	m_COT=5;
	if(c_Stop_Info>=sysConfig->meter_num){
		c_Stop_Info=sysConfig->meter_num-1;
	}
	if(c_Stop_Info<c_Start_Info){
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	Send_Total=c_Stop_Info-c_Start_Info+1;
	if(Send_Total>=sysConfig->meter_num){
		Send_Total=sysConfig->meter_num;
	}
	m_IOA=c_Start_Info;	
	Send_num=(MAXSENDBUF-DL719FIXLEN)/7;//	
	Send_Times=Send_Total/Send_num;
	if(Send_Total%Send_num)Send_Times++;
	Continue_Flag=0;
	Info_Size=7;//
	
Save_TA_Para_Send:	
	if(Continue_Flag==0){
		Continue_Flag=1;
		Send_RealTimes=0;
	}
	else{
		Send_RealTimes++;
	}

	if(Send_RealTimes<Send_Times-1){
		m_VSQ=Send_num;
	}
	else{
		m_VSQ=Send_Total-Send_num*Send_RealTimes;
		Continue_Flag=0;
		Send_RealTimes=0;
		Send_DataEnd=1;
	}
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x08:0x28;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	for(i=0;i<m_VSQ;i++){
		m_transBuf.m_transceiveBuf[buffptr++]=m_IOA;
		e_EDbase.get_save_para(savepara, m_IOA, TASK_TA);
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[0];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[1];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[2];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[3];
		m_transBuf.m_transceiveBuf[buffptr++]=0;//ta	
		info_sum=0;
		for(j=0;j<Info_Size-1;j++)
			info_sum+=m_transBuf.m_transceiveBuf[buffptr-Info_Size+1+j];
		m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//data check sum
		m_IOA++;
	}	
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	return 0;
}
/*******************参数传递-存储项目(LP)****************************
* M_TI=190
*RAD=86

*/
int CBASE102::M_Save_LP_Para_NA2()
{
	unsigned char i,j,buffptr,ret_val,info_sum,frm_sum;
	unsigned char inf_size,savepara[5];	
	
	if(Send_DataEnd){
			Clear_Continue_Flag();
			E5H_Yes();
			return -1;
		}
	if(Continue_Flag)goto Save_LP_Para_Send;
	m_TI=C_PARA_TRAN;
	m_COT=5;
	if(c_Stop_Info>=sysConfig->meter_num){
		c_Stop_Info=sysConfig->meter_num-1;
	}
	if(c_Stop_Info<c_Start_Info){
		Clear_Continue_Flag();
		E5H_Yes();
		return -1;
	}
	Send_Total=c_Stop_Info-c_Start_Info+1;
	if(Send_Total>=sysConfig->meter_num){
		Send_Total=sysConfig->meter_num;
	}
	m_IOA=c_Start_Info;
	Send_num=(MAXSENDBUF-DL719FIXLEN)/7;//
	Send_Times=Send_Total/Send_num;
	if(Send_Total%Send_num)Send_Times++;
	Continue_Flag=0;
	Info_Size=7;//
	
Save_LP_Para_Send:	
	if(Continue_Flag==0){
		Continue_Flag=1;
		Send_RealTimes=0;
	}
	else{
		Send_RealTimes++;
	}

	if(Send_RealTimes<Send_Times-1){
		m_VSQ=Send_num;
	}
	else{
		m_VSQ=Send_Total-Send_num*Send_RealTimes;
		Continue_Flag=0;
		Send_RealTimes=0;
		Send_DataEnd=1;
	}
	buffptr=0;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
	m_transBuf.m_transceiveBuf[buffptr++]=0x68;
	m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x08:0x28;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
	m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
	for(i=0;i<m_VSQ;i++){
		m_transBuf.m_transceiveBuf[buffptr++]=m_IOA;
		e_EDbase.get_save_para(savepara, m_IOA, TASK_LP);
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[0];
		m_transBuf.m_transceiveBuf[buffptr++]=savepara[1];
		m_transBuf.m_transceiveBuf[buffptr++]=0;
		m_transBuf.m_transceiveBuf[buffptr++]=0;
		m_transBuf.m_transceiveBuf[buffptr++]=0;//lp	
		info_sum=0;
		for(j=0;j<Info_Size-1;j++)
			info_sum+=m_transBuf.m_transceiveBuf[buffptr-Info_Size+1+j];
		m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//data check sum
		m_IOA++;
	}	
	frm_sum=0;
	for(i=4;i<buffptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[buffptr++]=0x16;
	m_transBuf.m_transCount=buffptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	return 0;
}

int CBASE102::M_Save_PB_Para_NA2()
{
		unsigned char i,j,buffptr,ret_val,frm_sum,info_sum;
		unsigned char inf_size;
		unsigned char savepara[5];
		if(m_Resend){
				m_transBuf.m_transCount=m_LastSendBytes;
				m_Resend=0;
				return 0;
			}
		if(Send_DataEnd){
				Clear_Continue_Flag();
				E5H_Yes();
				return -1;
			}
		if(Continue_Flag)goto Save_PB_Para_Send;
		m_TI=C_PARA_TRAN;
		m_COT=5;
		if(c_Stop_Info>=sysConfig->meter_num){
			c_Stop_Info=sysConfig->meter_num-1;
		}
		if(c_Stop_Info<c_Start_Info){
			Clear_Continue_Flag();
			E5H_Yes();
			return -1;
		}
		Send_Total=c_Stop_Info-c_Start_Info+1;
		if(Send_Total>=sysConfig->meter_num){
			Send_Total=sysConfig->meter_num;
		}
		m_IOA=c_Start_Info;
		Send_num=(MAXSENDBUF-DL719FIXLEN)/11;//
		Send_Times=Send_Total/Send_num;
		if(Send_Total%Send_num)Send_Times++;
		Continue_Flag=0;
		Info_Size=11;//
		
	Save_PB_Para_Send:	
		if(Continue_Flag==0){
			Continue_Flag=1;
			Send_RealTimes=0;
		}
		else{
			Send_RealTimes++;
		}
	
		if(Send_RealTimes<Send_Times-1){
			m_VSQ=Send_num;
		}
		else{
			m_VSQ=Send_Total-Send_num*Send_RealTimes;
			Continue_Flag=0;
			Send_RealTimes=0;
			Send_DataEnd=1;
		}
		buffptr=0;
		m_transBuf.m_transceiveBuf[buffptr++]=0x68;
		m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
		m_transBuf.m_transceiveBuf[buffptr++]=9+m_VSQ*Info_Size;
		m_transBuf.m_transceiveBuf[buffptr++]=0x68;
		m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x28:0x08;
		m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
		m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
		m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
		m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
		m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
		m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
		m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
		m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
		for(i=0;i<m_VSQ;i++){
			m_transBuf.m_transceiveBuf[buffptr++]=m_IOA;
			e_EDbase.get_save_para(savepara, m_IOA, TASK_PB);
			m_transBuf.m_transceiveBuf[buffptr++]=savepara[0];

			for(j=0;j<4;j++){
				m_transBuf.m_transceiveBuf[buffptr++]=(savepara[1]>>j)&0x01;
			}//pbtotal


			for(j=0;j<4;j++){
				m_transBuf.m_transceiveBuf[buffptr++]=(savepara[2]>>j)&0x01;
			}//pbtimes
			info_sum=0;
			for(j=0;j<Info_Size-1;j++)
				info_sum+=m_transBuf.m_transceiveBuf[buffptr-Info_Size+1+j];
			m_transBuf.m_transceiveBuf[buffptr++]=info_sum;//data check sum
			m_IOA++;
		}	
		frm_sum=0;
		for(i=4;i<buffptr;i++)
			frm_sum+=m_transBuf.m_transceiveBuf[i];
		m_transBuf.m_transceiveBuf[buffptr++]=frm_sum;
		m_transBuf.m_transceiveBuf[buffptr++]=0x16;
		m_transBuf.m_transCount=buffptr;
		m_LastSendBytes=m_transBuf.m_transCount;
		return 0;

}

/*********************************下装参数*********************************************/
/*******************参数下装回答帧*********************
* M_TI=191
*m_Cot=50  表示参数下装
*由记录体地址来选择相应的回答帧
由flag=1:表示参数下装
     flag=0:表示程序升级回答帧
********************************************************8*/

int  CBASE102::M_Cfg_Para_Update_NA2(unsigned char flag)
{
	
	unsigned char i,ptr,frm_sum,info_sum;		
	Info_Size=7;
	m_VSQ=1;
	m_TI=flag?C_PARA_SET:C_FILE_TRAN;
	m_COT=0x32;
	ptr=0;
	m_transBuf.m_transceiveBuf[ptr++]=0x68;
	m_transBuf.m_transceiveBuf[ptr++]=9+m_VSQ*(Info_Size);
	m_transBuf.m_transceiveBuf[ptr++]=9+m_VSQ*(Info_Size);
	m_transBuf.m_transceiveBuf[ptr++]=0x68;
	m_transBuf.m_transceiveBuf[ptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[ptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[ptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[ptr++]=m_TI;
	m_transBuf.m_transceiveBuf[ptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[ptr++]=m_COT;
	m_transBuf.m_transceiveBuf[ptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[ptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[ptr++]=c_Record_Addr;	
	m_transBuf.m_transceiveBuf[ptr++]=0;//ioa  规定
	if(flag){
			
			m_transBuf.m_transceiveBuf[ptr++]=(unsigned char)(m_Now_Frame_Seq&0xff);
			m_transBuf.m_transceiveBuf[ptr++]=(unsigned char)(m_Now_Frame_Seq>>8);
		}
	else{
			
			m_transBuf.m_transceiveBuf[ptr++]=(unsigned char)(m_Frame_Total&0xff);
			m_transBuf.m_transceiveBuf[ptr++]=(unsigned char)(m_Frame_Total>>8);
		}		

	
	m_transBuf.m_transceiveBuf[ptr++]=(unsigned char)(m_Now_Frame_Seq&0xff);
	m_transBuf.m_transceiveBuf[ptr++]=(unsigned char)(m_Now_Frame_Seq>>8);
	m_transBuf.m_transceiveBuf[ptr++]=m_Update_Flag ?0x55:0xAA;
	info_sum=0;
	for(i=13;i<ptr;i++)
		info_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[ptr++]=info_sum;
	
	
	frm_sum=0;
	for(i=4;i<ptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[ptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[ptr++]=0x16;
	m_transBuf.m_transCount=ptr;	
	//Clear_Continue_Flag();
	if(flag&&m_Update_Flag)
		*Remote_Para_Changed=1;
	return 0;
	
	
}
int CBASE102::M_Soft_Update_Can()
{
	unsigned char i,ptr,frm_sum,info_sum;	
	Info_Size=6;
	m_VSQ=1;
	m_TI=C_FILE_TRAN;
	m_COT=0x32;
	ptr=0;
	m_transBuf.m_transceiveBuf[ptr++]=0x68;
	m_transBuf.m_transceiveBuf[ptr++]=9+m_VSQ*(Info_Size);
	m_transBuf.m_transceiveBuf[ptr++]=9+m_VSQ*(Info_Size);
	m_transBuf.m_transceiveBuf[ptr++]=0x68;
	m_transBuf.m_transceiveBuf[ptr++]=m_ACD? 0x28:0x08;
	m_transBuf.m_transceiveBuf[ptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[ptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[ptr++]=m_TI;
	m_transBuf.m_transceiveBuf[ptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[ptr++]=m_COT;
	m_transBuf.m_transceiveBuf[ptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[ptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[ptr++]=c_Record_Addr;		
	m_transBuf.m_transceiveBuf[ptr++]=m_Now_Frame_Seq&0xff;
	m_transBuf.m_transceiveBuf[ptr++]=(m_Now_Frame_Seq>>8)&0xff;
	m_transBuf.m_transceiveBuf[ptr++]='C';
	m_transBuf.m_transceiveBuf[ptr++]='A';
	m_transBuf.m_transceiveBuf[ptr++]='N';
	info_sum=0;
	for(i=13;i<ptr;i++)
		info_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[ptr++]=info_sum;
	
	
	frm_sum=0;
	for(i=4;i<ptr;i++)
		frm_sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[ptr++]=frm_sum;
	m_transBuf.m_transceiveBuf[ptr++]=0x16;
	m_transBuf.m_transCount=ptr;	
	Clear_Continue_Flag();
	return 0;
}
void CBASE102::M_Return_Pin_Error(unsigned char type)
{
	unsigned char i,ptr,sum;
        ptr=0;
        m_transBuf.m_transceiveBuf[ptr++]=0x68;
	m_transBuf.m_transceiveBuf[ptr++]=0x09;
	m_transBuf.m_transceiveBuf[ptr++]=0x09;
       m_transBuf.m_transceiveBuf[ptr++]=0x68;
	m_transBuf.m_transceiveBuf[ptr++]=0x08;
	m_transBuf.m_transceiveBuf[ptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[ptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[ptr++]=type;
	m_transBuf.m_transceiveBuf[ptr++]=1;
	m_transBuf.m_transceiveBuf[ptr++]=0x0d;
	m_transBuf.m_transceiveBuf[ptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[ptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[ptr++]=c_Record_Addr;
	for(i=4;i<ptr;i++)
		sum+=m_transBuf.m_transceiveBuf[i];
	m_transBuf.m_transceiveBuf[ptr++]=sum;
	m_transBuf.m_transceiveBuf[ptr++]=0x16;
	m_transBuf.m_transCount=ptr;
	m_LastSendBytes=m_transBuf.m_transCount;
}

/*写下装的参数
*入口参数cfg_type:参数类别inf_num:信息体个数，定位用

*databuf=info_addr(1 byte)+data(x byte)+info_checksum(1 byte)
*/

int CBASE102::Update_Cfg_File(unsigned char *databuf)
{
	std::string filename;
	unsigned char cfg_len,pro_len;
	unsigned char ret;
	filename = "/mnt/nor/para/";	
	switch(c_Record_Addr)
	{
		case  Sys_Para_Adr ://sys_para
			filename+="sysspara.cfg";	
			//filename="sysspara.cfg.bak";	//for debug 
			cfg_len=Sys_Cfg_Len;
			pro_len=Sys_Cfg_Len;			
			break;
		case Mtr_Para_Adr://mtr
			filename+="mtrspara.cfg";
			//filename="mtrspara.cfg.bak";
			cfg_len=Mtr_Cfg_Len;
			pro_len=29;
			break;
		case Sio_Para_Adr ://061213 
			filename+="sioplan.cfg";
			//filename="sioplan.cfg.bak";
			cfg_len=Sio_Cfg_Len;
			pro_len=Sio_Cfg_Len;
			break;
		case Net_Para_Adr:
			filename+="netpara.cfg";
			//filename="netpara.cfg.bak";
			cfg_len=Net_Cfg_Len;
			pro_len=15;
			break;
		case Mon_Para_Adr :
			filename+="monpara.cfg";
			//filename="monpara.cfg.bak";
			cfg_len=Mon_Cfg_Len;
			pro_len=138;
			break;
		case Ctl_Para_Adr:
			filename+="ctlpara.cfg";
			//filename="ctlpara.cfg.bak";
			cfg_len=Ctl_Cfg_Len;
			pro_len=1;
			break;
		/*case Clt_Para_Adr:
			filename="ctspara.cfg";
			//filename="ctspara.cfg.bak";
			cfg_len=Clt_Cfg_Len;
			pro_len=Clt_Cfg_Len;
			break;*/
		case Sav_Para_Adr:
			filename+="stspara.cfg";
			//filename="stspara.cfg.bak";
			cfg_len=Sav_Cfg_Len;
			pro_len=Sav_Cfg_Len;
			break;
		case TE_Para_Adr:
			filename+="teitems.cfg";
			//filename="teitems.cfg.bak";
			cfg_len=Te_Cfg_Len;
			pro_len=Te_Cfg_Len;
			break;
		case TOU_Para_Adr:
			filename+="touitems.cfg";
			//filename="touitems.cfg.bak";
			cfg_len=Tou_Cfg_Len;
			pro_len=5;
			break;
		case QR_Para_Adr:
			filename+="qritems.cfg";
			//filename="qritems.cfg.bak";
			cfg_len=Qr_Cfg_Len;
			pro_len=5;
			break;
		case MNT_Para_Adr:
			filename+="mntitems.cfg";
			//filename="mntitems.cfg.bak";
			cfg_len=Mnt_Cfg_Len;
			pro_len=5;
			break;
		case TA_Para_Adr:
			filename+="taitems.cfg";
			//filename="taitems.cfg.bak";
			cfg_len=Ta_Cfg_Len;
			pro_len=5;
			break;
		case LP_Para_Adr://lp
			
			break;
		case 0x87://last_tou
			break;

		case PB_Para_Adr:
			filename+="pbitems.cfg";
			//filename="taitems.cfg.bak";
			cfg_len=PB_Cfg_Len;
			pro_len=PB_Cfg_Len;
			break;	
		case 9://plus			
			break;		
		}
	printf("In Update_Cfg_File filename=%s\n",filename.c_str());
	ret=WriteCfgFlie(filename,databuf, cfg_len,pro_len);
	printf("In Update_Cfg_File ret=%d\n",ret);
	return ret;
}

/*------------------------------------------------------------
		写配置文件，由Update_Cfg_File  函数调用
------------------------------------------------------------------*/

unsigned char CBASE102::WriteCfgFlie(std::string filename,unsigned char *databuf,unsigned char cfg_len,unsigned char pro_len )
												
{
	unsigned char i,j,ret;
	FILE *fp,*retranFp;
	unsigned char tmpbuf[MAXCFGLEN];
	unsigned char retranFlag,retranum,retranTable[MAXMETER];
	unsigned short pos,tempV;
	
	if(filename=="")  return 0;	
	if(pro_len<=0)  return 0;
	if(m_Now_Frame_Seq==0)
		 fp=fopen(filename.c_str(),"wb");
	else
		fp=fopen(filename.c_str(),"ab+"); 
	if(fp==NULL)return 0;
	if(fp!=NULL){
		fseek(fp,0,SEEK_END);		
		switch(c_Record_Addr){
				case  Sys_Para_Adr :
				case  Sio_Para_Adr :
				case Ctl_Para_Adr:
				//case Clt_Para_Adr:
				case Sav_Para_Adr:
				case TE_Para_Adr:
				case PB_Para_Adr://这几个配置文件数据格式无需转换
					for(i=0;i<m_VSQ_tmp;i++)
						m_Update_Flag=fwrite(databuf+1+i*(pro_len+2),cfg_len,1,fp);					
					break;
				case Mtr_Para_Adr://mtr
					for(i=0;i<m_VSQ_tmp;i++)
					{
						TransBcdArray2BinArray(databuf+1+i*(pro_len+2),tmpbuf,6);//linename
						TransBcdArray2BinArray(databuf+1+i*(pro_len+2)+3,tmpbuf+6,12);//mtradr
						TransBcdArray2BinArray(databuf+1+i*(pro_len+2)+9,tmpbuf+18,8);
						memcpy(tmpbuf+26,databuf+1+i*(pro_len+2)+13,11);
						//TransBcdArray2BinArray(databuf+1+i*(pro_len+2)+24,tmpbuf+37,3);
						tempV=(*(databuf+1+i*(pro_len+2)+25)<<8)+*(databuf+1+i*(pro_len+2)+24);
						tempV=HexToBcd(tempV);
						tmpbuf[37]=(tempV>>8)&0xf;
						tmpbuf[38]=(tempV>>4)&0xf;
						tmpbuf[39]=tempV&0xf;

						tempV=HexToBcd(*(databuf+1+i*(pro_len+2)+26));
						tmpbuf[40]=(tempV>>4)&0xf;
						tmpbuf[41]=tempV&0xf;

						tmpbuf[42]=*(databuf+1+i*(pro_len+2)+28);
						
						//printf("\n");
						//for(int jj=0;jj<43;++jj)
						//	printf("%x  ",tmpbuf[jj]);
						//printf("\n");
						m_Update_Flag=fwrite(tmpbuf,cfg_len,1,fp);						
					}
					break;
				case Net_Para_Adr:
					for(i=0;i<m_VSQ_tmp;i++){
						memcpy(tmpbuf,databuf+1+i*(pro_len+2),1);//使用的网络口
						for(j=0;j<12;j++)
							TransHex2Bcd(databuf+1+i*(pro_len+2)+1+j, tmpbuf+1+3*j,3);//转换IP,netmask,gateway
						//TransHex2Bcd(databuf+1+i*(pro_len+2)+1+12, tmpbuf+1+36, 5);
						memset(tmpbuf+37,0,5);
						m_Update_Flag=fwrite(tmpbuf,cfg_len,1,fp);
					}
					break;
				case Mon_Para_Adr :
					
					if(m_Now_Frame_Seq==0)
		 				retranFp=fopen("retranTable.cfg","wb");
					else
						retranFp=fopen("retranTable.cfg","ab+"); 
						
					for(i=0;i<m_VSQ_tmp;i++)
					{
						tmpbuf[0]=databuf[1+i*(pro_len+2)]; //port
						TransHex2Bcd(databuf+1+i*(pro_len+2)+1, tmpbuf+1,5);//listen port
						memcpy(tmpbuf+6,databuf+1+i*(pro_len+2)+3,2);//plan,prot	
						TransHex2Bcd(databuf+1+i*(pro_len+2)+5, tmpbuf+8,4);//addr
						memcpy(tmpbuf+12,databuf+1+i*(pro_len+2)+7,3);//syntime,retran,retrannum
						
						fseek(fp,i*14,SEEK_SET);
						m_Update_Flag=fwrite(tmpbuf,cfg_len,1,fp);

						retranFlag=tmpbuf[12];	
						if(retranFlag==1)
						{
							retranum=tmpbuf[13];
							memset(retranTable,0,sizeof(retranTable));
							for(j=0;j<retranum;j++)
							{
								retranTable[j]=databuf[1+i*(pro_len+2)+10+j];
							}
							
    							//if(NULL==retranFp)
    							//{
        							//retranFp=fopen("retranTable.cfg","wb");
    							//}
   							 if(NULL!=retranFp)
   	 						{
								//fseek(retranFp,i*128,SEEK_SET);
                						m_Update_Flag=fwrite(retranTable,MAXMETER,1,retranFp);
                						//fclose(retranFp);
    							}
						}
					}
					fclose(retranFp);
					break;	
	
	
				case TOU_Para_Adr:
				case QR_Para_Adr:
				case MNT_Para_Adr:
					for(i=0;i<m_VSQ_tmp;i++){
						memcpy(tmpbuf,databuf+1+i*(pro_len+2),1);//save num;
						for(j=0;j<5;j++)
							tmpbuf[1+j]=(*(databuf+1+i*(pro_len+2)+1)>>j)&0x01;//+A
						for(j=0;j<5;j++)
							tmpbuf[1+5+j]=(*(databuf+1+i*(pro_len+2)+2)>>j)&0x01;//-A
						for(j=0;j<5;j++)
							tmpbuf[1+10+j]=(*(databuf+1+i*(pro_len+2)+3)>>j)&0x01;//+R
						for(j=0;j<5;j++)
							tmpbuf[1+15+j]=(*(databuf+1+i*(pro_len+2)+4)>>j)&0x01;//-R
						m_Update_Flag=fwrite(tmpbuf,cfg_len,1,fp);
					}
					break;
				case TA_Para_Adr://ta
					for(i=0;i<m_VSQ_tmp;i++){
						memcpy(tmpbuf,databuf+1+i*(pro_len+2),1);//save num;
						for(j=0;j<6;j++)
							tmpbuf[1+j]=(*(databuf+1+i*(pro_len+2)+1)>>j)&0x01;
						for(j=0;j<8;j++)
							tmpbuf[1+6+j]=(*(databuf+1+i*(pro_len+2)+2)>>j)&0x01;
						for(j=0;j<5;j++)
							tmpbuf[1+14+j]=(*(databuf+1+i*(pro_len+2)+3)>>j)&0x01;
						m_Update_Flag=fwrite(tmpbuf,cfg_len,1,fp);
					}
					break;
				case LP_Para_Adr:
					break;
				default:
					break;
					
			
		}
	}
	fclose(fp);
	return m_Update_Flag;
	
}
/*计算当前帧序号*/
unsigned char CBASE102::m_Calc_Now_Frame_Seq(unsigned char *data_buf)
{
	unsigned char Now_Frame_Seq;
	unsigned char cfg_file_len;
	switch(c_Record_Addr){
		case  Sys_Para_Adr ://sys_para			
			cfg_file_len=Sys_Cfg_Len;
			break;
		case Mtr_Para_Adr://mtr			
			cfg_file_len=29;
			break;
		case Sio_Para_Adr :			
			cfg_file_len=Sio_Cfg_Len;
			break;
		case Net_Para_Adr:		
			cfg_file_len=15;
			break;
		case Mon_Para_Adr :			
			cfg_file_len=138;
			break;
		case Ctl_Para_Adr:			
			cfg_file_len=Ctl_Cfg_Len;
			break;
		/*case Clt_Para_Adr:			
			cfg_file_len=Clt_Cfg_Len;
			break;*/
		case Sav_Para_Adr:			
			cfg_file_len=Sav_Cfg_Len;
			break;
		case TE_Para_Adr:			
			cfg_file_len=Te_Cfg_Len;
			break;
		case TOU_Para_Adr:			
			cfg_file_len=5;
			break;
		case QR_Para_Adr:			
			cfg_file_len=5;
			break;
		case MNT_Para_Adr:			
			cfg_file_len=5;
			break;
		case TA_Para_Adr:			
			cfg_file_len=5;
			break;
		case PB_Para_Adr:
			cfg_file_len=PB_Cfg_Len;
			break;
		case LP_Para_Adr://lp			
			break;
		case 0x87://last_tou
			break;
		case 9://plus			
			break;		
		}
	if(m_VSQ_tmp)
		Now_Frame_Seq=(*(data_buf+15+(cfg_file_len+2)*(m_VSQ_tmp-1)))/m_VSQ_tmp;
	printf("++++++++Now_Frame_Seq=%d,Febzi=%d,m_vsq_tmp=%d\n",Now_Frame_Seq,*(data_buf+15+(cfg_file_len+2)*(m_VSQ_tmp-1)),m_VSQ_tmp);
	return Now_Frame_Seq;
	
}
void CBASE102::Ertu_Para_NA2()
{
	int ret;
	switch(c_Record_Addr){
					case  Sys_Para_Adr :
						ret=M_Sys_Para_NA2();
						printf("M_Sys_Para_NA2 ret=%d\n",ret);
						break;
					case Mtr_Para_Adr:
						ret=M_Meter_Para_NA2();
						printf("M_Meter_Para_NA2 ret=%d\n",ret);
						break;
					case Sio_Para_Adr ://061213  tiao shi shi hou xiugai,070307 gai hui lai 2
						ret=M_Sioplan_Para_NA2();
						printf("M_sioplan_Para_NA2 ret=%d\n",ret);
						break;
					case Net_Para_Adr:
						ret=M_Net_Para_NA2();
						printf("M_net_Para_NA2 ret=%d\n",ret);
						break;
					case Mon_Para_Adr :
						ret=M_Moin_Para_NA2();
						printf("M_Moin_Para_NA2 ret=%d\n",ret);
						break;
					case Ctl_Para_Adr:
						ret=M_Ctr_Para_NA2();
						printf("M_ctr_Para_NA2 ret=%d\n",ret);
						break;
					/*case Clt_Para_Adr:
						ret=M_Clt_Para_NA2();
						printf("M_clt_Para_NA2 ret=%d\n",ret);
						break;*/
					case Sav_Para_Adr:
						ret=M_Save_Para_NA2();
						printf("M_save_Para_NA2 ret=%d\n",ret);
						break;
					case TE_Para_Adr:
						ret=M_Save_TE_Para_NA2();
						printf("M_save_TE_Para_NA2 ret=%d\n",ret);
						break;
					case TOU_Para_Adr:
						ret=M_Save_TOU_Para_NA2();
						printf("M_Save_TOU_Para_NA2 ret=%d\n",ret);
						break;
					case QR_Para_Adr:
						ret=M_Save_QR_Para_NA2();
						printf("M_Save_QR_Para_NA2 ret=%d\n",ret);
						break;
					case MNT_Para_Adr:
						ret=M_Save_MNT_Para_NA2();
						printf("M_Save_MNT_Para_NA2 ret=%d\n",ret);
						break;
					case TA_Para_Adr:
						ret=M_Save_TA_Para_NA2();
						printf("M_Save_TA_Para_NA2 ret=%d\n",ret);
						break;
					case LP_Para_Adr:
						ret=M_Save_LP_Para_NA2();
						printf("M_Save_LP_Para_NA2 ret=%d\n",ret);
						break;
					case PB_Para_Adr:
						ret=M_Save_PB_Para_NA2();
						printf("M_Save_TA_Para_NA2 ret=%d\n",ret);
						break;	

					case Hardware_Status:
						ret=M_HardWare_Status_NA2();
						break;
					/*case Soft_Ser_Num:
						ret=M_Soft_SerNum_NA_2();
						break;*/
					default:
						break;
				}
}
/*控制端口报文察看*/
unsigned char CBASE102::M_CTL_PORT_Open()
{
	return 0;
#if 0
	unsigned char buffptr;
	unsigned char databuf[256];
	unsigned char mode[2];
	unsigned char sendlen,recilen;
	unsigned char sum,i;
	if(m_Resend){
			m_transBuf.m_transCount=m_LastSendBytes;
			m_Resend=0;
			return 0;
		}
	mode[1]=1;
	m_TI=C_CTL_MON_ON;
	m_VSQ=1;
	m_COT=5;
	if(!Send_485_Len[c_Record_Addr]||!Reci_485_Len[c_Record_Addr])
			return 0;
	
			buffptr=0;			
			m_transBuf.m_transceiveBuf[buffptr++]=0x68;
			//buffptr+=2;
			m_transBuf.m_transceiveBuf[buffptr++]=9+40+80;
			m_transBuf.m_transceiveBuf[buffptr++]=9+40+80;
			m_transBuf.m_transceiveBuf[buffptr++]=0x68;
			m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x08:0x28;
			m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
			m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
			m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
			m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
			m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
			m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
			m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
			m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
			//m_transBuf.m_transceiveBuf[buffptr++]=0;//send
			memset(mode,1,1);
			sendlen=0;
			//sendlen=GetDataFromDispbuff(databuf, 40, c_Record_Addr, mode);
			printf("485_send:");
			for(i=0;i<sendlen;i++)
				printf(" %02x",databuf[i]);
			printf("\n");
			memcpy(&m_transBuf.m_transceiveBuf[buffptr],databuf,40);
			buffptr+=40;
			//m_transBuf.m_transceiveBuf[buffptr++]=1;//reci
			memset(mode,0,1);
			//recilen=GetDataFromDispbuff(databuf, 80, c_Record_Addr, mode);
			recilen=0;
			printf("485_reci:");
			for(i=0;i<recilen;i++)
				printf(" %02x",databuf[i]);
			printf("\n");
			memcpy(&m_transBuf.m_transceiveBuf[buffptr],databuf,80);
			buffptr+=80;
			sum=0;
			for(i=4;i<buffptr;i++)
				sum+=m_transBuf.m_transceiveBuf[i];
			m_transBuf.m_transceiveBuf[buffptr++]=sum;
			m_transBuf.m_transceiveBuf[buffptr++]=0x16;
			//m_transBuf.m_transceiveBuf[buffptr-9-sendlen-recilen-2/*-2*/-2]=9+sendlen+recilen;
			//m_transBuf.m_transceiveBuf[buffptr-9-sendlen-recilen-2/*-2*/-3]=9+sendlen+recilen;
			m_transBuf.m_transCount=buffptr;
			m_LastSendBytes=m_transBuf.m_transCount;
			//sleep(5);
			return 0;
#endif			
}
unsigned char CBASE102::M_CTL_PORT_Close()
{
	unsigned char buffptr;
	unsigned char sum;
	unsigned char i;
			if(m_Resend){
			m_transBuf.m_transCount=m_LastSendBytes;
			m_Resend=0;
			return 0;
		}
			m_TI=C_CTL_MON_OFF;
			m_VSQ=1;
			m_COT=5;
			buffptr=0;			
			m_transBuf.m_transceiveBuf[buffptr++]=0x68;
			m_transBuf.m_transceiveBuf[buffptr++]=9;
			m_transBuf.m_transceiveBuf[buffptr++]=9;
			m_transBuf.m_transceiveBuf[buffptr++]=0x68;
			m_transBuf.m_transceiveBuf[buffptr++]=m_ACD? 0x08:0x28;
			m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_L;
			m_transBuf.m_transceiveBuf[buffptr++]=c_Link_Address_H;
			m_transBuf.m_transceiveBuf[buffptr++]=m_TI;
			m_transBuf.m_transceiveBuf[buffptr++]=m_VSQ;
			m_transBuf.m_transceiveBuf[buffptr++]=m_COT;
			m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_L;
			m_transBuf.m_transceiveBuf[buffptr++]=c_Dev_Address_H;
			m_transBuf.m_transceiveBuf[buffptr++]=c_Record_Addr;
			//m_transBuf.m_transceiveBuf[buffptr++]=0;
			//m_transBuf.m_transceiveBuf[buffptr++]=1;
			sum=0;
			for(i=4;i<buffptr;i++)
				sum+=m_transBuf.m_transceiveBuf[i];
			m_transBuf.m_transceiveBuf[buffptr++]=sum;
			m_transBuf.m_transceiveBuf[buffptr++]=0x16;
			m_transBuf.m_transCount=buffptr;	
			m_LastSendBytes=m_transBuf.m_transCount;
			Clear_Continue_Flag();
			return 0;
}

int CBASE102::Realmtr2Retranmtr(unsigned char realmtr)
{
	std::vector<unsigned char>::iterator iter;

	for(iter=m_retransmit_table.begin();iter!=m_retransmit_table.end();iter++)
	{
		if(*iter==realmtr)
			return *iter;
	}

	return -1;
}

void CBASE102::SendProc()
{
	
}
int CBASE102::ReciProc()
{
	return 0;
}
