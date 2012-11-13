/* File encode:	 GB2312
   filename:	sd102.h
山东102规约 实现文件 引用 DL/T719-2000（IEC60870-5-102：1996）
引用 GB/T 18657.2-2002 等效与 IEC60870-5-2:1990 链路传输规则*/
#include <sys/msg.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h> //字节序相关
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
	//开始时备份帧应该被清空
	memset(this->bak_farme,0x00,sizeof(bak_farme)*sizeof(u8));
	this->bak_farme_len=0;
}
Csd102::~Csd102()
{
	memset(this->bak_farme,0x00,sizeof(bak_farme)*sizeof(u8));
	this->bak_farme_len=0;
}
//终端(从站)主动发送.在山东102,非平衡传输中(协议规定)不需要
void Csd102::SendProc(void)
{
	return;
}
//接收(从主站发来的)报文
int Csd102::ReciProc(void)
{
	u8 len,reallen;
	int msglen;
	u8 readbuf[1024];
	splitmsg(readbuf,msglen);
	printf("after pro:");
	print_array(readbuf,msglen);
	return 0;

	/*#从缓冲区截取正确的报文
	  #判断报文类型
	  #检验报文
	  #根据分类, 采集发送,或者发送镜像帧,或者不应答等.
	*/
}
/*	分离出正确的报文.经过处理之后准确无误的报文被保存在
	readbuf[len]数组中,传递出来
输入:	类变量 m_recvBuf 输入
输出:	readbuf	数组
	len	数组长度
返回值:	0-成功
	非0-失败
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
/*分离报文子操作,按照格式分离,TODO 分离过程待改善*/
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
/*一般校验和程序.
输入:	a	数组(u8*)
	len	数组长度(int)
输出	无
返回值:		一个字节校验和(u8)
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
/*打印字符数组*/
void Csd102::print_array(u8 *transbuf,int len)
{
	int i;
	for(i=0; i<len; i++) {
		printf("%02X ",transbuf[i]);
	}
	printf("\n");
	return ;
}
//解析 FT1.2 固定帧长帧
int Csd102::process_short_frame(u8 * sfarme, int len)
{
	struct Short_farme farme;
	memcpy(&farme,sfarme,sizeof(struct Short_farme));//copy farme
	farme.link_addr=ntohs(farme.link_addr);//字节序转换
	if(farme.c_down.prm==0){ //下行
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
// 解析 FT1.2 变帧长帧
int Csd102::process_long_frame(u8 *sfarme,int len)
{
	struct Lpdu_head lpdu_head;
	//跳过帧头,复制链路数据头,利用其中的控制字节,分类判断
	memcpy(&lpdu_head,sfarme+(sizeof(struct Long_farme_head)),
	       sizeof(struct Lpdu_head)); //copy lpdu头
	lpdu_head.link_addr=ntohs(lpdu_head.link_addr);//字节序转换
	if(lpdu_head.c_down.prm==0){ //下行
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
