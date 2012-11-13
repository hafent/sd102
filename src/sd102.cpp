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
//
extern "C" CProtocol *CreateCProto_ProNULL()
{
	return  new Csd102;
}

Csd102::Csd102()
{
	//��ʼʱ����֡Ӧ�ñ����
	memset(this->bak_farme,0x00,sizeof(bak_farme)*sizeof(u8));
	this->bak_farme_len=0;
}
Csd102::~Csd102()
{
	memset(this->bak_farme,0x00,sizeof(bak_farme)*sizeof(u8));
	this->bak_farme_len=0;
}
//�ն�(��վ)��������.��ɽ��102,��ƽ�⴫����(Э��涨)����Ҫ
void Csd102::SendProc(void)
{
	return;
}
//����(����վ������)����
int Csd102::ReciProc(void)
{
	u8 len,reallen;
	int msglen;
	u8 readbuf[1024];
	splitmsg(readbuf,msglen);
	printf("after pro:");
	print_array(readbuf,msglen);
	return 0;

	/*#�ӻ�������ȡ��ȷ�ı���
	  #�жϱ�������
	  #���鱨��
	  #���ݷ���, �ɼ�����,���߷��;���֡,���߲�Ӧ���.
	*/
}
/*	�������ȷ�ı���.��������֮��׼ȷ����ı��ı�������
	readbuf[len]������,���ݳ���
����:	����� m_recvBuf ����
���:	readbuf	����
	len	���鳤��
����ֵ:	0-�ɹ�
	��0-ʧ��
*/
int Csd102::splitmsg(u8 *readbuf,int &len)
{
	len=get_num(&m_recvBuf);
	if(!len) {
		return -1;
	}
	Syn_Head_Rece_Flag = 0;
	while(len>=syn_char_num) {
		copyfrom_buf(readbuf, &m_recvBuf, len);
		if(!GX102s_Synchead(readbuf)) {
			if(0x68==*readbuf) {
				expect_charnum=*(readbuf+1)+6;
			} else {
				expect_charnum=6;
			}
			Syn_Head_Rece_Flag=1;
			break;
		} else {
			pop_char(&m_recvBuf);
			len=get_num(&m_recvBuf);
		}
	}
	return 0;
}
/*���뱨���Ӳ���,���ո�ʽ����,TODO ������̴�����*/
int Csd102::GX102s_Synchead(u8 * databuf)
{
	if((*databuf==0x68)&&(*(databuf+3)==0x68)) {
		return 0;
	} else if((*databuf==0x10)/*&&(*(databuf+2)==c_Dev_Address_L)&&(*(databuf+3)==c_Dev_Address_H)*/) {
		return 0;
	} else {
		return -1;
	}
}
/*һ��У��ͳ���.
����:	a	����(u8*)
	len	���鳤��(int)
���	��
����ֵ:		һ���ֽ�У���(u8)
  */
u8 Csd102::check_sum(u8 * a,int len )
{
	int i;
	u8 sum=0;
	for(i=0; i<len; i++) {
		sum+=a[i];
	}
	return sum;
}
/*��ӡ�ַ�����*/
void Csd102::print_array(u8 *transbuf,int len)
{
	int i;
	for(i=0; i<len; i++) {
		printf("%02X ",transbuf[i]);
	}
	printf("\n");
	return ;
}
//���� FT1.2 �̶�֡��֡
int Csd102::process_short_frame(u8 * sfarme, int len)
{
	struct Short_farme farme;
	memcpy(&farme,sfarme,sizeof(struct Short_farme));//copy farme
	farme.link_addr=ntohs(farme.link_addr);//�ֽ���ת��
	if(farme.c_down.prm==0){ //����
		PRINT_HERE
		return -1;
	}
	switch(farme.c_down.funcode){
	case FN_C_RS:
		PRINT_HERE
		break;
	case FN_C_TD:
		PRINT_HERE
		break;
	case FN_C_CL:
		PRINT_HERE
		break;
	case FN_C_CC1:
		PRINT_HERE
		break;
	case FN_C_CC2:
		PRINT_HERE
		break;
	case FN_C_RES1:
		PRINT_HERE
		break;
	case FN_C_RES2:
		PRINT_HERE
		break;
	default:
		PRINT_HERE
		break;
	}
	return 0;
}
// ���� FT1.2 ��֡��֡
int Csd102::process_long_frame(u8 *sfarme,int len)
{
	struct Lpdu_head lpdu_head;
	//����֡ͷ,������·����ͷ,�������еĿ����ֽ�,�����ж�
	memcpy(&lpdu_head,sfarme+(sizeof(struct Long_farme_head)),
	       sizeof(struct Lpdu_head)); //copy lpduͷ
	lpdu_head.link_addr=ntohs(lpdu_head.link_addr);//�ֽ���ת��
	if(lpdu_head.c_down.prm==0){ //����
		PRINT_HERE
		return -1;
	}
	switch(lpdu_head.c_down.funcode){
	case FN_C_RS:
		PRINT_HERE
		break;
	case FN_C_TD:
		PRINT_HERE
		break;
	case FN_C_CL:
		PRINT_HERE
		break;
	case FN_C_CC1:
		PRINT_HERE
		break;
	case FN_C_CC2:
		PRINT_HERE
		break;
	case FN_C_RES1:
		PRINT_HERE
		break;
	case FN_C_RES2:
		PRINT_HERE
		break;
	default:
		PRINT_HERE
		break;
	}
	return 0;
}
