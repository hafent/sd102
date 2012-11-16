/* File encode:	 GB2312
   filename:	sd102.h
ɽ��102��Լ ʵ���ļ� ���� DL/T719-2000��IEC60870-5-102��1996��
���� GB/T 18657.2-2002 ��Ч�� IEC60870-5-2:1990 ��·�������*/
#include <sys/msg.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h> //�ֽ������
#include "define.h"
#include "sys_utl.h"
#include "loopbuf.h"
#include "rtclocklib.h"
#include "HisDB.h"
#include "log.h"
#include "Hisfile.h"
#include "sd102.h"
#include <stdarg.h>
#pragma pack(1)
//
extern "C" CProtocol *CreateCProto_sd102()
{
	PRINT_HERE
			//return NULL;
			printf("**** create sd1021111\n");
	return  new Csd102;
}

Csd102::Csd102()
{
	syn_char_num=6;
	//Syn_Head_Rece_Flag=0;
	m_ACD=0;
	Command=0;
	m_Resend=0;
	Continue_Flag=0;
	Send_DataEnd=0;
	Send_num=0;
	Send_Times=0;
	Send_RealTimes=0;
	SendT_RealTimes=0;
	Send_Total=0;
	Info_Size=0;
	c_FCB=0;
	c_FCB_Tmp=0xff;
	c_TI_tmp=0xff;
	memset(mirror_buf,0,sizeof(mirror_buf));
	PRINT_HERE
			//��ʼʱ����֡Ӧ�ñ����
			//memset(this->reci_farme_bak,0x00,sizeof(reci_farme_bak)*sizeof(u8));
			//this->reci_farme_bak_len=0;
}
Csd102::~Csd102()
{
	PRINT_HERE
			//memset(this->reci_farme_bak,0x00,sizeof(reci_farme_bak)*sizeof(u8));
			//this->reci_farme_bak_len=0;
}
int Csd102::Init(struct stPortConfig *tmp_portcfg)
{
	has_class1_dat=false;
	has_class2_dat=false;
	c_Link_Address_H=tmp_portcfg->m_ertuaddr>>8;
	c_Link_Address_L=(unsigned char)tmp_portcfg->m_ertuaddr;
	link_addr=tmp_portcfg->m_ertuaddr;
	m_checktime_valifalg=tmp_portcfg->m_checktime_valiflag;
	m_suppwd=tmp_portcfg->m_usrsuppwd;
	m_pwd1=tmp_portcfg->m_usrpwd1;
	m_pwd2=tmp_portcfg->m_usrpwd2;
	m_retransmit_table=tmp_portcfg->m_retransmit_mtr;//ת����
	retran_table_valid=(m_retransmit_table.empty())?0:1;
	m_Max_Mtrnum=(!retran_table_valid)?(sysConfig->meter_num):(m_retransmit_table.size());
	this->exist_backup_frame=false;
	return 0;
}

//�ն�(��վ)��������.��ɽ��102,��ƽ�⴫����(Э��涨)����Ҫ
void Csd102::SendProc(void)
{
	return;
}
//��ʾ�ȴ�״̬
void Csd102::show_wait(u32 &stat)
{
	printf("\rsd102:Wait for farme.");
	switch(stat%4){
	case 0:
		printf("|");
		break;
	case 1:
		printf("/");
		break;
	case 2:
		printf("-");
		break;
	case 3:
		printf("\\");
		break;
	default:
		printf("?");
	}
	stat++;
	fflush(stdout);
	return;
}

//����(����վ������)����
int Csd102::ReciProc(void)
{
#if 0 //just for reci base debug
	m_transBuf.m_transceiveBuf[0]=0;
	m_transBuf.m_transceiveBuf[1]=1;
	m_transBuf.m_transceiveBuf[2]=2;
	m_transBuf.m_transceiveBuf[3]=3;
	m_transBuf.m_transCount=4;
#endif
	//u8 len,reallen;
	int ret=-1;
	u8 reci_farme[1024];//reci farme
	int reci_len=0;//֡,֡��
	//1. get farme [���޸����Ա]
	ret=separate_msg(reci_farme,reci_len);
	if(ret!=0) {
		show_wait(status);
		return 0;
	}
	printf("Receive Farme[%d]:",reci_len);
	print_array(reci_farme,reci_len);
	//2. verify farme
	ret=verify_farme(reci_farme,reci_len);
	if(ret!=0) {
		printf("verify err,ignord\n");
		return 0;
	}
	//
	union Ctrl_down c2=get_ctrl_field(reci_farme_bak,reci_farme_bak_len);
	union Ctrl_down c=get_ctrl_field(reci_farme,reci_len);
	//���������ʼ��Ҫ��Ӧ
#if 0
	//ǰ��fcbӦ�ò�һ��
	printf("c.fcb=%d c_bak.fcb=%d || c.fcv=%d c_bak.fcv=%d\n"
	       ,c.fcb,c2.fcb,c.fcv,c2.fcv);
#endif
	//	if(c.fcv==0 && c.fcb==0 && c.funcode==FN_C_RESET){//��·��λ
	//		printf("Reset Link\n");
	//		Transfer(farme,farme_len);
	//		save_reci_farme(farme,farme_len);
	//		save_tran_farme(farme,farme_len);
	//		return 0;
	//	}
	if(c.fcv==1 && c.fcb==c2.fcb && exist_backup_frame) { //��·�ط�
		//�ط����ĵķ���֡,�ط�
		printf("Resend Farme\n");
		Transfer(reci_farme_bak,reci_farme_bak_len);
		save_reci_farme(reci_farme,reci_len);
		save_tran_farme(reci_farme,reci_len,
				this->tran_farme_bak,
				this->tran_farme_bak_len,
				this->exist_backup_frame);
		return 0;
	}
	int send_len=0;
	u8 tran_farme[4+255+2];//֡

	bool need_to_trans=false;
/*

if(�ظ�֡){
	�ط�;
	�˳�;
}

if("S2:����/ȷ��֡"){ //����: 1.��λ��·(FC0) 2.��������(FC3)

1.��λ��·(C_RCU_NA_2)(FC0):
	�ش�:	��λ֡(FC0)

2.0��������(FC3)
	�ش�:	ȷ��	ACK	E5

2.1ʱ��ͬ��(FC3):CF=0 1 FCB FCV 0 0 1 1(C SYN TA 2)
	�ش�:	ϵͳʱ��ͬ��ȷ��(M SYN TA 2)(-FC0)

}

if("S3:����/��Ӧ֡"){ //����: 1.�ٻ�1��(FC10) 2.�ٻ�2��(FC11) 3.��·����(FC9)

1.�ٻ�1������(FC10):
	if(��1������){
		�ش�:	�����ݻش�(-FC8)
	}

2.�ٻ�2������(FC11):
	if(��2������){
		�ش�:	�����ݻش�(-FC8)
	}
	if(û��2������){
		if(�� 1������){
			�ش�:	(M_NV_NA_2)(-FC9)
		}
		if(û��1������){
			�ش�:	NACK:	E5 (�񶨻ش�)NACK (Negative ACKnowledgment)
		}
	}

3.��·����(C_LKR_NA_2)(FC9):
	�ش�:	��·����M_LKR_NA_2(FC11);
}

���淢��֡�ͽ���֡;
�˳�;
*/
	switch(c.funcode){
	//S2 дָ��
	case FN_C_RCU:
		//��λ
		fun_M_CON_NA_2(tran_farme,send_len);
		//����֡����λ TODO
		PRINT_HERE;
		goto OK;
		break;
	case FN_C_TRANS_DAT:
		//�������� Ӧ��
		confirm(tran_farme,send_len);
		goto OK;
		PRINT_HERE;
		break;
	//S3 ������
	case FN_C_RLK:
		fun_M_LKR_NA_2(tran_farme,send_len);//�ظ���·״̬
		PRINT_HERE;
		break;
	case FN_C_PL1:
		if(has_class1_dat){
			printf("has_class1_dat\n");
		}else{
			nack(tran_farme,send_len);
			goto OK;
		}
		PRINT_HERE;
		break;
	case FN_C_PL2:
		if(has_class2_dat){
			printf("has_class2_dat\n");
		}else{
			if(has_class1_dat){
				printf("has_class1_dat\n");
				fun_M_NV_NA_2(tran_farme,send_len);
				goto OK;
			}else{
				nack(tran_farme,send_len);
				goto OK;
			}
		}
		PRINT_HERE;
		break;
	case FN_C_RES1:
		PRINT_HERE;
		break;
	case FN_C_RES2:
		PRINT_HERE;
		break;
	default:
		PRINT_HERE;
	}
	printf("c.FC=%d\n",c.funcode);

	switch(reci_farme[0]) {
	case START_SHORT_FARME:
		process_short_frame(reci_farme,reci_len,
				    tran_farme,send_len);
		break;
	case START_LONG_FARME:
		process_long_frame(reci_farme,reci_len,
				   tran_farme,send_len);
		//printf("reci: tran_farme[0]=%x send_len=%d \n",tran_farme[0],send_len);
		break;
	default:
		PRINT_HERE;
	}
	//����....
	//��ⷢ��
	//	printf("link respond \n");
	//	printf(" len =%d  %02X %02X %02X %02X  \n",
	//	       len,tran_farme[0],tran_farme[1],tran_farme[2],tran_farme[3]);
	OK:
	Transfer(tran_farme,send_len);

	//	switch(farme[0]){
	//	case START_SHORT_FARME:
	//		process_short_frame(farme,farme_len);
	//		//PRINT_HERE;
	//		break;
	//	case START_LONG_FARME:
	//		process_long_frame(farme,farme_len);
	//		//PRINT_HERE;
	//		break;
	//	case START_SINGLE_FARME:
	//		PRINT_HERE;
	//		break;
	//	default:
	//		PRINT_HERE;
	//		printf("err start byte of farme");
	//	}
#if 0
	//�ظ�һЩ���� ��ʾ�յ���
	m_transBuf.m_transceiveBuf[0]=0xab;
	m_transBuf.m_transCount=1;
#endif
	//���� ���յ���֡
	save_reci_farme(reci_farme,reci_len);
	save_tran_farme(tran_farme,send_len,
			this->tran_farme_bak,this->tran_farme_bak_len,
			this->exist_backup_frame);
	return 0;
	/*#�ӻ�������ȡ��ȷ�ı���
	  #�жϱ�������
	  #���鱨��
	  #���ݷ���, �ɼ�����,���߷��;���֡,���߲�Ӧ���.
	*/
}
/*S2:����/ȷ��֡ ���ն�Ӧ��(ȷ��)
*/
int Csd102::confirm(u8 *farme_out,int &len_out)const
{
	farme_out[0]=SINGLE_CHARACTER;
	len_out=sizeof(SINGLE_CHARACTER);
	return 0;
}
/*��Ӧ��
*/
int Csd102::nack(u8 *farme_out,int &len_out)const
{
	farme_out[0]=SINGLE_CHARACTER;
	len_out=sizeof(SINGLE_CHARACTER);
	return 0;
}

/*����֡ �� farme ͨ�����Ƶ� m_transBuf �ṹ�巢��.
in:	farme	�ֽ���
	farme_len	����
out:	m_transBuf	�޸ĵķ��ͻ������ṹ��,��������[���Ա,����ʹ��!!]
*/
int Csd102::Transfer(const u8* farme,const int farme_len)
{
	printf("Send Farme[%d]:",farme_len);
	print_array(farme,farme_len);
	//printf("send: farme[0]=%x farme_len=%d \n",farme[0],farme_len);
	memcpy(this->m_transBuf.m_transceiveBuf,farme,farme_len);
	this->m_transBuf.m_transCount=farme_len;
	return 0;
}

/* ��֡�л�ȡ������,һ���ֽ�
in:	farme	֡/�ֽ���
	farme_len	�ֽ�������
out:	none
return:	�����ֽ�(union Ctrl_down)
*/
union Ctrl_down Csd102::get_ctrl_field(const u8* farme,const int farme_len)
{

	union Ctrl_down c;
	if(farme_len<(int)sizeof(struct Short_farme)) {
		//PRINT_HERE;
		//printf("farme len(%d) is too small\n",farme_len);
		return c;
	}
	switch(farme[0]) {
	case START_SHORT_FARME:
		c.val=farme[0+sizeof(START_SHORT_FARME)];//��ʼ�ֽں�����ǿ�����
		break;
	case START_LONG_FARME:
		c.val=farme[0+sizeof(struct Farme_head)];//֡ͷ������ǿ�����
		break;
	default:
		if(farme[0]==0 && farme[1]==0 && farme[2]==0) {
			printf("back-up farme is empty \n");
		} else {
			PRINT_HERE;
		}
	}
	return c;
}

/*	�����������ȷ�ı���.֡ǰ���������,֡β�����ݱ����ڻ������.
	��Ч�Ĺ��˴󲿷ֲ��ϸ�ı���
	��������֮��׼ȷ����ı��ı������� readbuf[len] ������,���ݳ���
m_recvBuf:	In/Out	���뻺����/�޸� [ע��:��������ʹ��]
readbuf:	Out	����
len:	Out		���鳤��
return:	0-�ɹ�
	��0-ʧ��
*/
int Csd102::separate_msg(u8 *readbuf,int &len)
{
	int farme_len=0;
	bool syn_head_ok=false;
	//Syn_Head_Rece_Flag = 0;
	len=get_num(&m_recvBuf);//����������
#if 0
	printf("*get_num=%d\n",len);
#endif
	if(!len) {
		return 1;//���Ȳ���/������û������,�ָ����
	}
	//1.����
	while(len>=syn_char_num) {//���г���̫�̾Ͳ����ٲ�����
		copyfrom_buf(readbuf, &m_recvBuf, len);
#if 0
		printf("farme:");
		print_array(readbuf,len);
#endif
		if(!sync_head(readbuf,farme_len)) {//���ҵ�һ֡
			syn_head_ok=true;
			break;
		} else {//���� 1Ԫ�س����� //��ֹ�������ݳ���������֡ǰ��
			pop_char(&m_recvBuf);
			len=get_num(&m_recvBuf);
#if 0
			printf("else get_num=%d\n",len);
#endif
		}
	}
	//2.�ж�
	if(!syn_head_ok) {
		return 2;
	}
	//2.2�ҵ�֡,�������
	len=get_num(&m_recvBuf);
	if(len>=farme_len) {//�ضϵ� ֡β,ʣ�µ����ڻ�����
		len=get_buff(readbuf, &m_recvBuf,farme_len);
		if(len!=farme_len) {
			PRINT_HERE;
			printf("error:farme!=len\n");
			return 3;
		}
		//printf("ʣ�� %d\n",get_num(&m_recvBuf));
		/*
			printf("gx102 recieve:");
			for(int i=0;i<reallen;i++)
			printf(" %02x",readbuf[i]);
			printf("\n");
			*/
		//return len;
	}
	return 0;
}
/*���뱨���Ӳ���,���ո�ʽ����,
  ͨ���򵥵ıȽ�֡ͷ,��ʼ�ֽ�,�����ֽ�,֡�� �⼸��
in:	buf ��������
out:	farme_len ����ɹ������֡��,ʧ�������0
return:	0-�ɹ�;	��0-ʧ��;
*/
int Csd102::sync_head(const u8 *buf,int &farme_len)const
{
	farme_len=0;
	//����֡
	if(buf[0]==START_SHORT_FARME) {
		int end_index=sizeof(struct Short_farme)-1;
		if(buf[end_index]==END_BYTE) {
			farme_len=sizeof(struct Short_farme);
			return 0;
		} else {
			return -1;
		}
	}
	//�䳤֡
	if((buf[0]==START_LONG_FARME) && (buf[3]==START_LONG_FARME)
			&& buf[1]==buf[2]) {
		int len=buf[1];
		//��ĩβ��Ԫ�ص�index
		int end_index=0+sizeof(struct Farme_head) //֡ͷ
				+len				//��·���ݵ�Ԫ(LPDU)����
				+sizeof(struct Farme_tail) //֡β
				-1;
		//printf("end_index=%d\n",end_index);
		if(buf[end_index]==END_BYTE) {
			//printf("#end_index=%d\n",end_index);
			farme_len=sizeof(struct Farme_head)+len
					+sizeof(struct Farme_tail);
			return 0;
		} else {
			return -1;
		}
	}
	return -1;
}
/* ��һ������֡. ��У��,��ַ���.����д��ĳ�Ա����(const[this])
  ��� �̶�֡ �� �䳤֡
  in:	dat	��������
	len	���鳤��
  return:	0-ͨ�����;��0-δͨ�����
*/
int Csd102::verify_farme(const u8  *dat, const int len)const
{
	struct Short_farme* farme;//���ڹ̶�֡
	struct Lpdu_head* lpdu_head;//���ڱ䳤֡
	struct Farme_tail* farme_tail;//֡β,���ڱ䳤֡
	union Ctrl_down c;
	u16 farme_link_addr=0;//��·��ַ
	u8 farme_cs=0;//��֡��cs
	u8 cs=0;//����õ���cs
	const u8  C_PRM_DOWN=1;//���з����־(1-����;0-����)
	switch(dat[0]) { //��������
	case START_SHORT_FARME:
		farme=(struct Short_farme *)(dat);
		cs=check_sum(dat+sizeof(START_SHORT_FARME),
			     sizeof(union Ctrl_down)+sizeof(link_addr_t));
		//ͳһ
		farme_link_addr=farme->link_addr;
		c.prm=farme->c_down.prm;
		farme_cs=farme->farme_tail.cs;
		break;
	case START_LONG_FARME:
		lpdu_head=(struct Lpdu_head *)(dat+sizeof(struct Farme_head));
		//�ֽ�֡β
		farme_tail=(struct Farme_tail*)
				(dat+len-sizeof(struct Farme_tail));
		//У��:ȥ��ͷ,(���Դ�),ȥ֡��β.У���м䲿��
		cs=check_sum(dat+sizeof(struct Farme_head),
			     len-sizeof(struct Farme_head)
			     -sizeof(struct Farme_tail));
		//ͳһ
		farme_link_addr=lpdu_head->link_addr;
		c.prm=lpdu_head->c_down.prm;
		farme_cs=farme_tail->cs;
		break;
	default:
		PRINT_HERE;
		return 0x10;
		break;
	}
	//1.��У��
	//printf("farme_cs=%02X  cs=%02X \n",farme_cs,cs);
	if(farme_cs!=cs) {
		PRINT_HERE;
		printf("CS err farme_cs=0x%02X but Calculate cs=0x%02X ,Ignore.\n",
		       farme_cs,cs);
		return 0x11;
	}
	//2.�ж��Ƿ�����֡
	if(c.prm!=C_PRM_DOWN){
		PRINT_HERE;
		printf("c.prm[%d] !=C_PRM_DOWN[%d] ,Ignore.\n",
		       c.prm,C_PRM_DOWN);
		return 0x12;
	}
	//3.�ж��Ƿ񴫵ݸ����ն�
	if(farme_link_addr!=this->link_addr) {
		PRINT_HERE;
		printf("link_addr=%d farme.link_addr=%d \n",
		       link_addr,farme->link_addr);
		return 0x13;
	}
	//4. ֡�������: ͨ������ ����0;
	return 0;
}

/*���� FT1.2 �̶�֡��֡
in:	farme_in	����֡
	len_in	����֡����
out:	farme_out	���֡
	len_out	���֡����
return:	0	��ȷ����
	��0	����ʧ��
*/
int Csd102::process_short_frame(const u8  *farme_in ,const int len_in,
				u8 *farme_out,int &len_out)const
{
	struct Short_farme *farme=(struct Short_farme *)farme_in;
	struct Short_farme *farme_up=(struct Short_farme *)farme_out;
	u8 cs=0;
	if(len_in<6) {
		PRINT_HERE;
		printf("len(%d) too small\n",len_in);
	}
	//�������
	switch(farme->c_down.funcode) {
	case FN_C_RCU://��λ
		//PRINT_HERE;
		farme_up->c_up.funcode=FN_M_CON;
		farme_up->c_up.acd=0;
		farme_up->c_up.dfc=0;
		farme_up->link_addr=link_addr;
		cs=check_sum((u8*)farme_up+sizeof(START_SHORT_FARME),
			     sizeof(union Ctrl_down)+sizeof(link_addr_t));
		farme_up->farme_tail.cs=cs;
		break;
	case FN_C_TRANS_DAT:
		PRINT_HERE;
		break;
	case FN_C_RLK://������·״̬
		//if()
		farme_up->c_up.funcode=FN_M_RSP;
		farme_up->c_up.acd=0;
		farme_up->c_up.dfc=0;
		farme_up->link_addr=link_addr;
		cs=check_sum((u8*)farme_up+sizeof(START_SHORT_FARME),
			     sizeof(union Ctrl_down)+sizeof(link_addr_t));
		farme_up->farme_tail.cs=cs;
		//PRINT_HERE;
		break;
	case FN_C_PL1:
		PRINT_HERE;
		break;
	case FN_C_PL2:
		PRINT_HERE;
		break;
	case FN_C_RES1:
		PRINT_HERE;
		break;
	case FN_C_RES2:
		PRINT_HERE;
		break;
	default:
		PRINT_HERE;
		break;
	}
	//memcpy(farme_out,&farme_up,sizeof(struct Short_farme));
	//������������Ϣ ����/��ʼ/����
	farme_up->start_byte=START_SHORT_FARME;
	farme_up->farme_tail.end_byte=END_BYTE;
	farme_up->c_up.prm=0;
	farme_up->c_up.res=0;
	len_out=sizeof(struct Short_farme);
	//	printf("send_farme.start_byte=%x Tfarme[0]=%x t_len=%d\n"
	//	       ,send_farme.start_byte,Tfarme[0],t_len);
	return 0;
}
/*���� FT1.2 ��֡��֡
  ���ձ䳤֡,��typ���ദ�����б���(��վ������)
  */
int Csd102::process_long_frame(u8 const *farme_in,int const len_in, u8 *farme_out, int &len_out)
{
	int ret;
	/*������֡�зֽ��һЩ���ݵ�Ԫ
		:Lpdu_head + Asdu_head + (��Ϣ��δ����) + Farme_tail*/
	struct Lpdu_head *lpdu_head=(struct Lpdu_head *)
			(farme_in+
			 sizeof(struct Farme_head));
	struct Asdu_head  *asdu_head=(struct Asdu_head  *)
			(farme_in+
			 sizeof(struct Farme_head)+
			 sizeof(struct Lpdu_head));
	struct Farme_tail *farme_tail=(struct Farme_tail *)
			(farme_in+
			 len_in-sizeof(struct Farme_tail));
#if 0
	printf("addr=%d , cf=%02X \n",
	       lpdu_head->link_addr ,lpdu_head->c_down.val);
	printf("asdu_head->typ=%d , asdu_head->rad=%02X \n",
	       asdu_head->typ ,asdu_head->rad);
	printf("farme_tail->cs=%02X , farme_tail->end_byte=%02X \n",
	       farme_tail->cs ,farme_tail->end_byte);
#endif
	switch(asdu_head->typ) {
	case C_RD_NA_2:
		PRINT_HERE;
		break;
	case C_SP_NA_2:
		PRINT_HERE;
		break;
	case C_SP_NB_2:
		PRINT_HERE;
		break;
	case C_TI_NA_2://��ȡʱ��
		ret=fun_C_TI_NA_2(farme_out, len_out);
		if(ret==0){
			has_class1_dat=true;
		}else{
			has_class1_dat=false;
		}
		//printf("farme_out[0]=%d\n",farme_out[0]);
		//PRINT_HERE;
		return 0;
		break;
	case C_CI_NR_2:
		PRINT_HERE;
		break;
	case C_CI_NS_2:
		PRINT_HERE;
		break;
	case C_SYN_TA_2:
		PRINT_HERE;
		break;
	case C_CI_NA_B_2:
		PRINT_HERE;
		break;
	case C_YC_TA_2:
		PRINT_HERE;
		break;
	case C_CI_NA_C_2:
		PRINT_HERE;
		break;
	case C_XL_NB_2:
		PRINT_HERE;
		break;
	case C_CI_NA_D_2:
		PRINT_HERE;
		break;
	default:
		PRINT_HERE;
	}
	//
	farme_out[0]=0xAB;
	len_out=1;
	return 0;
}
//���ݽ���֡
int Csd102::save_reci_farme(void * farme,int len)
{
	if(memcpy(this->reci_farme_bak,farme,len)==NULL) {
		PRINT_HERE;
		return -1;
	}
	this->reci_farme_bak_len=len;
	return 0;
}
//���ݷ���֡
int Csd102::save_tran_farme(void * farme,int len,
			    u8* bakfarme,int &bakfarme_len,bool &hasbaked)
{
	if(memcpy(bakfarme,farme,len)==NULL) {
		PRINT_HERE;
		return -1;
	}
	bakfarme_len=len;
	hasbaked=true;
	return 0;
}
int Csd102::clear_fcv(void)
{
	struct Short_farme farme;
	memcpy(&farme,this->reci_farme_bak,sizeof(farme));
	farme.c_down.fcv=0;
	return 0;
}
/*	�ٻ�2������,û��2������,������1������
 �ظ�M_NV_NA_2,fc:FN_M_NO_DAT û�����ٻ�������(������1������ACD=1)
	Reset Communication Unit
*/
int Csd102::fun_M_NV_NA_2(u8 *farme_out, int &len_out )const
{
	struct Short_farme * farme=(struct Short_farme *)farme_out;
	len_out=sizeof(struct Short_farme);
	farme->start_byte=START_SHORT_FARME;
	farme->farme_tail.end_byte=END_BYTE;
	farme->link_addr=this->link_addr;
	farme->c_up.funcode=FN_M_NO_DAT;
	farme->c_up.acd=1;
	farme->farme_tail.cs=check_sum((u8*)farme+sizeof(START_SHORT_FARME),
				       sizeof(union Ctrl_up )
				       +sizeof( link_addr_t));
	return 0;

}
/*	�ظ���·״̬����(FN_C_RLK FC9)��֡
	�ظ�: M_LKR_NA_2 FN_M_RSP FC11 ����·״̬��Ӧ
*/
int Csd102::fun_M_LKR_NA_2(u8 *farme_out, int &len_out )const
{
	struct Short_farme * farme=(struct Short_farme *)farme_out;
	len_out=sizeof(struct Short_farme);
	farme->start_byte=START_SHORT_FARME;
	farme->farme_tail.end_byte=END_BYTE;
	farme->link_addr=this->link_addr;
	farme->c_up.prm=0;//�ϴ�
	farme->c_up.funcode=FN_M_RSP;
	farme->c_up.acd=0;
	farme->c_up.dfc=0;//���Խ�������
	farme->farme_tail.cs=check_sum((u8*)farme+sizeof(START_SHORT_FARME),
				       sizeof(union Ctrl_up )
				       +sizeof( link_addr_t));
	return 0;

}
/*	��λԶ����·ȷ��֡ �ظ�C_RCU_NA_2(��λԶ����·[ͨѶ��Ԫ])
	Reset Communication Unit
*/
int Csd102::fun_M_CON_NA_2(u8 *farme_out, int &len_out )const
{
	struct Short_farme * farme=(struct Short_farme *)farme_out;
	len_out=sizeof(struct Short_farme);
	farme->start_byte=START_SHORT_FARME;
	farme->farme_tail.end_byte=END_BYTE;
	farme->c_up.funcode=FN_M_CON;
	farme->link_addr=this->link_addr;
	farme->farme_tail.cs=check_sum((u8*)farme+sizeof(START_SHORT_FARME),
				       sizeof(union Ctrl_up )
				       +sizeof( link_addr_t));
	return 0;

}
/*�����ն�ʱ��
 out:	farme_out	����/���͵���վ��֡
	len_out		֡��
return:	0	�ɹ�
*/
int Csd102::fun_C_TI_NA_2(u8 *farme_out, int &len_out )const
{

	struct m_tSystime systime;
	struct stFarme_C_TI_NA_2 *pfarme;
	const u8 InfoObj_num=1;//��Ϣ������
	GetSystemTime_RTC(&systime);
	pfarme =(struct stFarme_C_TI_NA_2 * )(farme_out);
	memset(pfarme,0,sizeof(struct stFarme_C_TI_NA_2));
	len_out=sizeof(struct stFarme_C_TI_NA_2);

	//pfarme->farme_head.len1=1;
	//printf("rtu time test:farme_head.len1=%d\n",pfarme->farme_head.len1);
	//����:
	//pfarme->farme_head.
	pfarme->farme_head.start_byte1=START_LONG_FARME;
	pfarme->farme_head.len1=sizeof(struct Lpdu_head)+
			sizeof(struct Tb) ;
	pfarme->farme_head.len2=pfarme->farme_head.len1;
	pfarme->farme_head.start_byte2=START_LONG_FARME;
	pfarme->lpdu_head.c_up.funcode=FN_M_SEND_DAT;
	pfarme->lpdu_head.c_up.prm=0;
	pfarme->lpdu_head.c_up.acd=0;
	pfarme->lpdu_head.c_up.dfc=0;
	pfarme->lpdu_head.link_addr=this->link_addr;
	pfarme->asdu_head.typ=M_TI_TA_2;
	pfarme->asdu_head.vaq.sq=0;
	pfarme->asdu_head.vaq.n=1;
	pfarme->asdu_head.cot.val=COT_REQUEST;
	pfarme->asdu_head.asdu_addr=(InfoObj_num/255)+1;//������Ϣ�������û����255,ֵ��1,��1��ʼ.
	pfarme->asdu_head.rad=0;
	//TODO ���������� ����Ū���ֵ
	pfarme->tb.ms=systime.msec;
	//pfarme->tb.eti=0;
	pfarme->tb.second=systime.sec;
	pfarme->tb.min=systime.min;
	pfarme->tb.hour=systime.hour;
	pfarme->tb.day=systime.day;
	pfarme->tb.month=systime.mon;
	pfarme->tb.year=systime.year;
	pfarme->tb.week=systime.week;
	pfarme->tb.su=0;//������ʱ��(��׼ʱ��)
	pfarme->tb.iv=0;//��Ч
	pfarme->tb.res1=0;//��������
	pfarme->tb.res2=0;
	pfarme->farme_tail.cs=check_sum(farme_out+sizeof(struct Farme_head)
					,len_out-sizeof(struct Farme_head)
					-sizeof(struct Farme_tail));
	pfarme->farme_tail.end_byte=END_BYTE;
	//
	printf("time=%d-%d-%d %d:%d:%d %dms \n",
	       systime.year,systime.mon,systime.day,
	       systime.hour,systime.min,systime.sec,systime.msec);
	//	printf("rtu time test:farme_head.len1=%d sizeof farme=%d len_out=%d\n"
	//	       "farme_out[0]= %x farme_out[1]= %x\n"
	//	       ,pfarme->farme_head.len1,sizeof(struct Farme_time),len_out,
	//	       farme_out[0],farme_out[1]);
	return 0;
#if 0
	unsigned char i,ptr,sum;

	m_ACD=0;
	//m_TI=M_TI_TA_GX2;
	m_VSQ=1;
	m_COT=0x05;
	ptr=0;
	m_transBuf.m_transceiveBuf[ptr++]=0x68;
	m_transBuf.m_transceiveBuf[ptr++]=0x10;
	m_transBuf.m_transceiveBuf[ptr++]=0x10;
	m_transBuf.m_transceiveBuf[ptr++]=0x68;
	m_transBuf.m_transceiveBuf[ptr++]=0x08;
	m_transBuf.m_transceiveBuf[ptr++]=c_Link_Address_L;
	m_transBuf.m_transceiveBuf[ptr++]=c_Link_Address_H;
	m_transBuf.m_transceiveBuf[ptr++]=m_TI;
	m_transBuf.m_transceiveBuf[ptr++]=m_VSQ;
	m_transBuf.m_transceiveBuf[ptr++]=m_COT;
	m_transBuf.m_transceiveBuf[ptr++]=c_Dev_Address_L;
	m_transBuf.m_transceiveBuf[ptr++]=c_Dev_Address_H;
	m_transBuf.m_transceiveBuf[ptr++]=c_Record_Addr;
	m_transBuf.m_transceiveBuf[ptr++]=0;
	m_transBuf.m_transceiveBuf[ptr++]=systime.sec<<2;
	m_transBuf.m_transceiveBuf[ptr++]=systime.min;
	m_transBuf.m_transceiveBuf[ptr++]=systime.hour;
	m_transBuf.m_transceiveBuf[ptr++]=systime.day;
	m_transBuf.m_transceiveBuf[ptr++]=systime.mon;
	m_transBuf.m_transceiveBuf[ptr++]=systime.year;
	sum=0;
	for(i=4; i<ptr; i++)
	{ sum+=m_transBuf.m_transceiveBuf[i]; }
	m_transBuf.m_transceiveBuf[ptr++]=sum;
	m_transBuf.m_transceiveBuf[ptr++]=0x16;
	m_transBuf.m_transCount=ptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	Clear_Continue_Flag();
	Clear_FrameFlags();
	return 0;
#endif
}
/*һ��У��ͳ���.
in:	a	����(u8 *)
	len	���鳤��(int)
out	��
return:		һ���ֽ�У���(u8)
		  */
u8 Csd102::check_sum(u8 const *a,int const len )const
{
	// i;
	u8 sum=0;
	for(int i=0; i<len; i++) {
		sum+=a[i];
	}
	return sum;
}
/*��ӡ�ַ�����*/
void Csd102::print_array(const u8 *transbuf,const int len)const
{
	int i;
	for(i=0; i<len; i++) {
		printf("%02X ",transbuf[i]);
	}
	printf("\n");
	return ;
}
