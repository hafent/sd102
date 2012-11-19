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
#include <stdarg.h>
#pragma pack(1)
//
extern "C" CProtocol *CreateCProto_sd102()
{
	PRINT_HERE
	;
	//return NULL;
	printf("**** create sd1021111\n");
	return new Csd102;
}

Csd102::Csd102()
{
	syn_char_num = 6;
//Syn_Head_Rece_Flag=0;
	m_ACD = 0;
	Command = 0;
	m_Resend = 0;
	Continue_Flag = 0;
	Send_DataEnd = 0;
	Send_num = 0;
	Send_Times = 0;
	Send_RealTimes = 0;
	SendT_RealTimes = 0;
	Send_Total = 0;
	Info_Size = 0;
	c_FCB = 0;
	c_FCB_Tmp = 0xff;
	c_TI_tmp = 0xff;
	memset(mirror_buf, 0, sizeof(mirror_buf));
	PRINT_HERE

	link_addr = 0;
	this->exist_backup_frame = false;
	this->has_class1_dat = false;
	this->has_class2_dat = false;
	this->status = 0;
	set_has_mirror_farme(false);

//开始时备份帧应该被清空
//memset(this->reci_farme_bak,0x00,sizeof(reci_farme_bak)*sizeof(u8));
//this->reci_farme_bak_len=0;
}
Csd102::~Csd102()
{
	PRINT_HERE
	;
//memset(this->reci_farme_bak,0x00,sizeof(reci_farme_bak)*sizeof(u8));
//this->reci_farme_bak_len=0;
}
int Csd102::Init(struct stPortConfig *tmp_portcfg)
{

	last_typ = M_UNUSED;
	has_class1_dat = false;
	has_class2_dat = false;
	c_Link_Address_H = tmp_portcfg->m_ertuaddr>>8;
	c_Link_Address_L = (unsigned char) tmp_portcfg->m_ertuaddr;
	link_addr = tmp_portcfg->m_ertuaddr;
	m_checktime_valifalg = tmp_portcfg->m_checktime_valiflag;
	m_suppwd = tmp_portcfg->m_usrsuppwd;
	m_pwd1 = tmp_portcfg->m_usrpwd1;
	m_pwd2 = tmp_portcfg->m_usrpwd2;
	m_retransmit_table = tmp_portcfg->m_retransmit_mtr;		//转发表
	retran_table_valid = (m_retransmit_table.empty()) ? 0 : 1;
	m_Max_Mtrnum = (!retran_table_valid) ?
	                                       (sysConfig->meter_num) :
	                                       (m_retransmit_table.size());
	this->exist_backup_frame = false;
	return 0;
}

//终端(从站)主动发送.在山东102,非平衡传输中(协议规定)不需要
void Csd102::SendProc(void)
{
	return;
}
//显示等待状态
void Csd102::show_wait(u32 &stat)
{
	printf("\rsd102:Wait for farme.");
	switch (stat%4) {
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

//接收(从主站发来的)报文
int Csd102::ReciProc(void)
{
#if 0 //just for receive base debug
	m_transBuf.m_transceiveBuf[0]=0;
	m_transBuf.m_transceiveBuf[1]=1;
	m_transBuf.m_transceiveBuf[2]=2;
	m_transBuf.m_transceiveBuf[3]=3;
	m_transBuf.m_transCount=4;
#endif
//u8 len,real len;
	int ret = -1;
	u8 reci_farme[1024];			//receive frame
	int reci_len = 0;			//帧,帧长
//1. get frame [会修改类成员]
	ret = separate_msg(reci_farme, reci_len);
	if (ret!=0) {
		show_wait(status);
		return 0;
	}
	printf("Receive Farme[%d]:", reci_len);
	print_array(reci_farme, reci_len);
//2. verify frame
	ret = verify_frame(reci_farme, reci_len);
	if (ret!=0) {
		printf("verify err,ignord\n");
		return 0;
	}
//
	union Ctrl_down c2 = get_ctrl_field(reci_frame_bak, reci_frame_bak_len);
	union Ctrl_down c = get_ctrl_field(reci_farme, reci_len);
//以下情况开始需要回应
#if 0
//前后FCB应该不一样
	printf("c.fcb=%d c_bak.fcb=%d || c.fcv=%d c_bak.fcv=%d\n"
			,c.fcb,c2.fcb,c.fcv,c2.fcv);
#endif
	if (c.fcv==1&&c.fcb==c2.fcb&&exist_backup_frame) {     //链路重发
		//重发本文的发送帧,重发
		printf("Resend Farme\n");
		transfer(reci_frame_bak, reci_frame_bak_len);
		save_reci_frame(reci_farme, reci_len);
		save_tran_frame(reci_farme, reci_len,
		                this->tran_frame_bak,
		                this->tran_frame_bak_len,
		                this->exist_backup_frame);
		return 0;
	}
	int send_len = 0;
	u8 tran_farme[4+255+2];		//帧

//bool need_to_trans=false;
	/*
	 if("S2:发送/确认帧"){ //分类: 1.复位链路(FC0) 2.传输数据(FC3)

	 if(重复帧){
	 重发;
	 退出;
	 }else(
	 保存确认帧(复本)
	 }

	 1.复位链路(C_RCU_NA_2)(FC0):
	 回答:	复位帧(FC0)
	 发送&备份;

	 2.0传输数据(FC3)
	 回答:	确认	ACK	E5
	 发送;
	 并,开始处理数据,得到数据保存起来,备用
	 备份;

	 2.1时间同步(FC3):CF=0 1 FCB FCV 0 0 1 1(C SYN TA 2)
	 回答:	系统时间同步确认(M SYN TA 2)(-FC0)
	 发送&备份;

	 }

	 if("S3:请求/响应帧"){ //分类: 1.召唤1级(FC10) 2.召唤2级(FC11) 3.链路请求(FC9)

	 1.召唤1级数据(FC10):
	 if(有1级数据){
	 回答:	以数据回答(-FC8)
	 发送&备份;
	 }else{
	 回答:	NACK:	E5 (否定回答)NACK (Negative ACKnowledgment)
	 发送&备份;
	 }

	 2.召唤2级数据(FC11):
	 if(有2级数据){
	 回答:	以数据回答(-FC8)
	 }
	 if(没有2级数据){
	 if(有 1级数据){
	 回答:	(M_NV_NA_2)(-FC9)
	 }
	 if(没有1级数据){
	 回答:	NACK:	E5 (否定回答)NACK (Negative ACKnowledgment)
	 }
	 }

	 3.链路请求(C_LKR_NA_2)(FC9):
	 回答:	链路正常M_LKR_NA_2(FC11);
	 }

	 发送:
	 发送数据到主站;
	 备份:
	 保存发送帧和接收帧;
	 退出;
	 */
	switch (c.funcode) {
//S2 写指令
	case FN_C_RCU:
		//复位
		fun_M_CON_NA_2(tran_farme, send_len);
		// TODO 重置帧计数位

		PRINT_HERE
		goto SEND;
		break;
	case FN_C_TRANS_DAT:
		//立即应答 E5
		confirm(tran_farme, send_len);
		transfer(tran_farme, send_len);
		//应答之后即刻 开始处理事务
		//TODO 开始准备数据,分类讨论需要的数据
		switch (reci_farme[0]) {
		case START_SHORT_FRAME:
			process_short_frame(reci_farme, reci_len,
			                tran_farme, send_len);
			break;
		case START_LONG_FRAME:
			process_long_frame(reci_farme, reci_len,
			                tran_farme, send_len);
			//printf("receive: tran_farme[0]=%x send_len=%d \n",tran_farme[0],send_len);
			break;
		default:
			PRINT_HERE
			break;
		}
		//保存1级数据 备用
		save_dat(c1_dat.dat, tran_farme, send_len);
		c1_dat.len = send_len;
		printf("c1_dat.dat[0]=%x c1_dat.len=%d || send_len=%d\n"
		                , c1_dat.dat[0], c1_dat.len, send_len);
		//形成正确的备份帧.
		confirm(tran_farme, send_len);
		goto BACKUP;
		PRINT_HERE
		break;
		//S3 读数据
	case FN_C_RLK:
		fun_M_LKR_NA_2(tran_farme, send_len);		//回复链路状态
		PRINT_HERE
		break;
	case FN_C_PL1:
		if (has_class1_dat) {
			switch (last_typ) {
			case C_TI_NA_2:
				printf("send _class1_dat\n");
				memcpy(tran_farme, c1_dat.dat, c1_dat.len);
				send_len = c1_dat.len;
				last_typ = 0;
				//(c1_dat.dat,c1_dat.len,tran_farme,send_len);
				//Transfer(c1_dat.dat,c1_dat.len	);
				goto SEND;
			case C_CI_NR_2://读数据
				if (has_mirror_farme) {
					set_has_mirror_farme(false);
					printf("send mirror_farme\n");
					memcpy(tran_farme, mirror_farme.dat, mirror_farme.len);
					send_len = mirror_farme.len;
				} else {
					printf("send _class1_dat\n");
					memcpy(
					                tran_farme,
					                c1_dat.dat,
					                c1_dat.len);
					send_len = c1_dat.len;
					last_typ = 0;
				}
				//(c1_dat.dat,c1_dat.len,tran_farme,send_len);
				//Transfer(c1_dat.dat,c1_dat.len	);
				goto SEND;

			}

			//对于电量,先招2级,返回没有,招1级,返回镜像帧,
			//再招2级,又没有,再招1级,返回数据帧,
			//if()

			//TODO 发送1级数据
		} else {
			nack(tran_farme, send_len);
			goto SEND;
		}
		PRINT_HERE
		;
		break;
	case FN_C_PL2:
		if (has_class2_dat) {
			printf("send class2_dat\n");
			//TODO 发送2级数据
		} else {
			if (has_class1_dat) {
				printf("no class2 dat ,has_class1_dat\n");
				fun_M_NV_NA_2(tran_farme, send_len);
				goto SEND;
			} else {
				nack(tran_farme, send_len);
				goto SEND;
			}
		}
		PRINT_HERE
		;
		break;
	case FN_C_RES1:
		PRINT_HERE
		;
		break;
	case FN_C_RES2:
		PRINT_HERE
		;
		break;
	default:
		PRINT_HERE
		;
		break;
	}
	printf("c.FC=%d\n", c.funcode);

	switch (reci_farme[0]) {
	case START_SHORT_FRAME:
		process_short_frame(reci_farme, reci_len,
		                tran_farme, send_len);
		break;
	case START_LONG_FRAME:
		process_long_frame(reci_farme, reci_len,
		                tran_farme, send_len);
		//printf("receive: tran_farme[0]=%x send_len=%d \n",tran_farme[0],send_len);
		break;
	default:
		PRINT_HERE
		;
		break;
	}
	SEND:			//发送帧
	transfer(tran_farme, send_len);

#if 0
//回复一些东西 表示收到了
	m_transBuf.m_transceiveBuf[0]=0xab;
	m_transBuf.m_transCount=1;
#endif
	BACKUP:     //保存 备份 接收帧和发送帧 ,仅用于丢失重发机制
//保存 接收到的帧
	save_reci_frame(reci_farme, reci_len);
	save_tran_frame(tran_farme, send_len,
	                this->tran_frame_bak, this->tran_frame_bak_len,
	                this->exist_backup_frame);
	return 0;
}
/*保存数据
 */
int Csd102::save_dat(u8* ddat, const u8* sdat, const int slen) const
        {
	memcpy(ddat, sdat, slen);
//printf(" slen=%d  \n",slen);
	return 0;
}

/*S2:发送/确认帧 的终端应答(确认) 形成确认帧
 */
int Csd102::confirm(u8 *farme_out, int &len_out) const
        {
	farme_out[0] = SINGLE_CHARACTER;
	len_out = sizeof(SINGLE_CHARACTER);
	return 0;
}
/*否定应答	形成否定帧(nack)
 */
int Csd102::nack(u8 *farme_out, int &len_out) const
        {
	farme_out[0] = SINGLE_CHARACTER;
	len_out = sizeof(SINGLE_CHARACTER);
	return 0;
}

/*发送帧 将 frame 通过复制到 m_transBuf 结构体发送.
 in:	frame	字节流
 farme_len	长度
 out:	m_transBuf	修改的发送缓冲区结构体,发送数据[类成员,隐含使用!!]
 */
int Csd102::transfer(const u8* farme, const int farme_len)
{
	printf("Send Farme[%d]:", farme_len);
	print_array(farme, farme_len);
//printf("send: frame[0]=%x farme_len=%d \n",frame[0],farme_len);
	memcpy(this->m_transBuf.m_transceiveBuf, farme, farme_len);
	this->m_transBuf.m_transCount = farme_len;
	return 0;
}

/* 从帧中获取控制域,一个字节
 in:	frame	帧/字节流
 farme_len	字节流长度
 out:	none
 return:	控制字节(union Ctrl_down)
 */
union Ctrl_down Csd102::get_ctrl_field(const u8* farme, const int farme_len)
{

	union Ctrl_down c;
	if (farme_len<(int) sizeof(struct Short_frame)) {
		//PRINT_HERE;
		//printf("frame length(%d) is too small\n",farme_len);
		return c;
	}
	switch (farme[0]) {
	case START_SHORT_FRAME:
		c.val = farme[0+sizeof(START_SHORT_FRAME)];	//开始字节后面既是控制域
		break;
	case START_LONG_FRAME:
		c.val = farme[0+sizeof(struct Frame_head)];	//帧头后面既是控制域
		break;
	default:
		if (farme[0]==0&&farme[1]==0&&farme[2]==0) {
			printf("back-up frame is empty \n");
		} else {
			PRINT_HERE
			;
		}
		break;
	}
	return c;
}

/*	初步分离出正确的报文.帧前的数据清除,帧尾的数据保留在缓冲队列.
 高效的过滤大部分不合格的报文
 经过处理之后准确无误的报文被保存在 readbuf[len] 数组中,传递出来
 m_recvBuf:	In/Out	输入缓冲区/修改 [注意:被隐含的使用]
 readbuf:	Out	数组
 len:	Out		数组长度
 return:	0-成功
 非0-失败
 */
int Csd102::separate_msg(u8 *readbuf, int &len)
{
	int frame_len = 0;
	bool syn_head_ok = false;
//Syn_Head_Rece_Flag = 0;
	len = get_num(&m_recvBuf);		//缓冲区长度
#if 0
	                printf("*get_num=%d\n",len);
#endif
	if (!len) {
		return 1;		//长度不够/缓冲区没有数据,分割错误
	}
//1.查找
	while (len>=syn_char_num) {		//队列长度太短就不必再查找了
		copyfrom_buf(readbuf, &m_recvBuf, len);
#if 0
		printf("farme:");
		print_array(readbuf,len);
#endif
		if (!sync_head(readbuf, frame_len)) {		//查找到一帧
			syn_head_ok = true;
			break;
		} else {		//否则 1元素出队列 //防止垃圾数据出现在正常帧前面
			pop_char(&m_recvBuf);
			len = get_num(&m_recvBuf);
#if 0
			printf("else get_num=%d\n",len);
#endif
		}
	}
//2.判断
	if (!syn_head_ok) {
		return 2;
	}
//2.2找到帧,处理队列
	len = get_num(&m_recvBuf);
	if (len>=frame_len) {		//截断到 帧尾,剩下的留在缓冲区
		len = get_buff(readbuf, &m_recvBuf, frame_len);
		if (len!=frame_len) {
			PRINT_HERE
			printf("error:farme!=len\n");
			return 3;
		}
		//printf("剩余 %d\n",get_num(&m_recvBuf));
		/*
		 printf("gx102 receive:");
		 for(int i=0;i<reallen;i++)
		 printf(" %02x",readbuf[i]);
		 printf("\n");
		 */
		//return len;
	}
	return 0;
}
/*分离报文子操作,按照格式分离,
 通过简单的比较帧头,起始字节,结束字节,帧长 这几样
 // in:	buffer 输入数字
 out:	farme_len 分离成功则输出帧长,失败则输出0
 return:	0-成功;	非0-失败;
 */
int Csd102::sync_head(const u8 *buffer, int &farme_len) const
        {
	farme_len = 0;
//定长帧
	if (buffer[0]==START_SHORT_FRAME) {
		int end_index = sizeof(struct Short_frame)-1;
		if (buffer[end_index]==END_BYTE) {
			farme_len = sizeof(struct Short_frame);
			return 0;
		} else {
			return -1;
		}
	}
//变长帧
	if ((buffer[0]==START_LONG_FRAME)&&(buffer[3]==START_LONG_FRAME)
	                &&buffer[1]==buffer[2]) {
		int len = buffer[1];
		//最末尾的元素的index
		int end_index = 0+sizeof(struct Frame_head)     //帧头
		                +len				//链路数据单元(LPDU)长度
		                +sizeof(struct Frame_tail)     //帧尾
		-1;
		//printf("end_index=%d\n",end_index);
		if (buffer[end_index]==END_BYTE) {
			//printf("#end_index=%d\n",end_index);
			farme_len = sizeof(struct Frame_head)+len
			                +sizeof(struct Frame_tail);
			return 0;
		} else {
			return -1;
		}
	}
	return -1;
}
/* 进一步检验帧. 和校验,地址检测.不会写类的成员变量(const[this])
 检测 固定帧 和 变长帧
 in:	dat	数据数组
 len	数组长度
 return:	0-通过检测;非0-未通过检测
 */
int Csd102::verify_frame(const u8 *dat, const int len) const
        {
	struct Short_frame* farme;     //用于固定帧
	struct Lpdu_head* lpdu_head;     //用于变长帧
	struct Frame_tail* farme_tail;     //帧尾,用于变长帧
	union Ctrl_down c;
	u16 farme_link_addr = 0;     //链路地址
	u8 farme_cs = 0;     //本帧的cs
	u8 cs = 0;     //计算得到的cs
	const u8 C_PRM_DOWN = 1;     //下行方向标志(1-下行;0-上行)
	switch (dat[0]) {     //分类讨论
	case START_SHORT_FRAME:
		farme = (struct Short_frame *) (dat);
		cs = check_sum(dat+sizeof(START_SHORT_FRAME),
		                sizeof(union Ctrl_down)+sizeof(link_addr_t));
		//统一变量,由下面处理
		farme_link_addr = farme->link_addr;
		c.prm = farme->c_down.prm;
		farme_cs = farme->farme_tail.cs;
		break;
	case START_LONG_FRAME:
		lpdu_head =(struct Lpdu_head *) (dat
		                                +sizeof(struct Frame_head));
		//分解帧尾
		farme_tail = (struct Frame_tail*)
		                (dat+len-sizeof(struct Frame_tail));
		//校验:去掉头,(嘎嘣脆),去帧掉尾.校验中间部分
		cs = check_sum(dat+sizeof(struct Frame_head),
		                len-sizeof(struct Frame_head)
		                                -sizeof(struct Frame_tail));
		//统一
		farme_link_addr = lpdu_head->link_addr;
		c.prm = lpdu_head->c_down.prm;
		farme_cs = farme_tail->cs;
		break;
	default:
		PRINT_HERE
		return 0x10;
		break;
	}
//1.和校验
//printf("farme_cs=%02X  check sum=%02X \n",farme_cs,cs);
	if (farme_cs!=cs) {
		PRINT_HERE
		printf("CS err farme_cs=0x%02X but "
			"Calculate cs=0x%02X ,Ignore.\n",
		                farme_cs,
		                cs);
		return 0x11;
	}
//2.判断是否下行帧,終端只接收下行的幀,不是下行的一定時錯誤的。
	if (c.prm!=C_PRM_DOWN) {
		PRINT_HERE
		printf("c.prm[%d] !=C_PRM_DOWN[%d] ,Ignore.\n",
		c.prm, C_PRM_DOWN);
		return 0x12;
	}
//3.判断是否传递给本终端
	if (farme_link_addr!=this->link_addr) {
		PRINT_HERE
		printf("link_addr=%d farme.link_addr=%d \n",
		                link_addr, farme->link_addr);
		return 0x13;
	}
	return 0;//4. 结束
}

/*解析 FT1.2 固定帧长帧
 in:	farme_in	输入帧
 len_in	输入帧长度
 out:	farme_out	输出帧
 len_out	输出帧长度
 return:	0	正确处理
 非0	处理失败
 */
int Csd102::process_short_frame(const u8 *farme_in, const int len_in,
        u8 *farme_out, int &len_out) const
        {
	struct Short_frame *farme = (struct Short_frame *) farme_in;
	struct Short_frame *frame_up = (struct Short_frame *) farme_out;
	u8 cs = 0;
	if (len_in<6) {
		PRINT_HERE
		;
		printf("length(%d) too small\n", len_in);
	}
//分类解析
	switch (farme->c_down.funcode) {
	case FN_C_RCU:     //复位
//PRINT_HERE;
		frame_up->c_up.funcode = FN_M_CON;
		frame_up->c_up.acd = 0;
		frame_up->c_up.dfc = 0;
		frame_up->link_addr = link_addr;
		cs = check_sum((u8*) frame_up+sizeof(START_SHORT_FRAME),
		                sizeof(union Ctrl_down)+sizeof(link_addr_t));
		frame_up->farme_tail.cs = cs;
		break;
	case FN_C_TRANS_DAT:
		PRINT_HERE
		break;
	case FN_C_RLK:		//请求链路状态
//if()
		frame_up->c_up.funcode = FN_M_RSP;
		frame_up->c_up.acd = 0;
		frame_up->c_up.dfc = 0;
		frame_up->link_addr = link_addr;
		cs = check_sum((u8*) frame_up+sizeof(START_SHORT_FRAME),
		                sizeof(union Ctrl_down)+sizeof(link_addr_t));
		frame_up->farme_tail.cs = cs;
		//PRINT_HERE;
		break;
	case FN_C_PL1:
		PRINT_HERE
		break;
	case FN_C_PL2:
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
//memcpy(farme_out,&farme_up,sizeof(struct Short_farme));
//其他公共的信息 上行/开始/结束
	frame_up->start_byte = START_SHORT_FRAME;
	frame_up->farme_tail.end_byte = END_BYTE;
	frame_up->c_up.prm = 0;
	frame_up->c_up.res = 0;
	len_out = sizeof(struct Short_frame);
//	printf("send_farme.start_byte=%x T frame[0]=%x t_len=%d\n"
//	       ,send_farme.start_byte,T frame[0],t_len);
	return 0;
}
/*解析 FT1.2 变帧长帧
 接收变长帧,按 type(TYP)分类处理下行报文(主站发出的)
 */
int Csd102::process_long_frame(u8 const *farme_in, int const len_in,
        u8 *farme_out, int &len_out)
{
	int ret;
	/*从输入帧中分解出一些数据单元
	 :Lpdu_head + Asdu_head + (信息体未解析) + Farme_tail*/
	struct Lpdu_head *lpdu_head = (struct Lpdu_head *)
	                (farme_in+sizeof(struct Frame_head));
	struct Asdu_head *asdu_head = (struct Asdu_head *)
	                (farme_in+sizeof(struct Frame_head)+
	                                sizeof(struct Lpdu_head));
	struct Frame_tail *frame_tail = (struct Frame_tail *)
	                (farme_in+
	                                len_in-sizeof(struct Frame_tail));
#if 0
	printf("addr=%d , cf=%02X \n",
			lpdu_head->link_addr ,lpdu_head->c_down.val);
	printf("asdu_head->typ=%d , asdu_head->rad=%02X \n",
			asdu_head->typ ,asdu_head->rad);
	printf("farme_tail->cs=%02X , farme_tail->end_byte=%02X \n",
			frame_tail->cs ,frame_tail->end_byte);
#endif
	last_typ = asdu_head->typ;
	switch (asdu_head->typ) {
	case C_RD_NA_2:
		PRINT_HERE
		break;
	case C_SP_NA_2:
		PRINT_HERE
		break;
	case C_SP_NB_2:
		PRINT_HERE

		break;
	case C_TI_NA_2:     //读取时间
		ret = fun_M_TI_TA_2(farme_out, len_out);
		if (ret==0) {
			has_class1_dat = true;
		} else {
			has_class1_dat = false;
		}
		//printf("farme_out[0]=%d\n",farme_out[0]);
		//PRINT_HERE;
		return 0;
		break;
	case C_CI_NR_2:		//读取电量
//保存镜像帧 之后用于返回
		set_has_mirror_farme(true);
		save_dat(mirror_farme.dat, farme_in, len_in);
		mirror_farme.len = len_in;
		ret = fun_M_IT_TA_2(farme_in, len_in, farme_out, len_out);
		if (ret==0) {
			has_class1_dat = true;
		} else {
			has_class1_dat = false;
		}
		//PRINT_HERE;
		return 0;
		break;
	case C_CI_NS_2:
		PRINT_HERE
		;
		break;
	case C_SYN_TA_2:
		PRINT_HERE
		;
		break;
	case C_CI_NA_B_2:
		PRINT_HERE
		;
		break;
	case C_YC_TA_2:
		PRINT_HERE
		;
		break;
	case C_CI_NA_C_2:
		PRINT_HERE
		;
		break;
	case C_XL_NB_2:
		PRINT_HERE
		;
		break;
	case C_CI_NA_D_2:
		PRINT_HERE
		;
		break;
	default:
		PRINT_HERE
		;
		break;
	}
//
	farme_out[0] = 0xAB;
	len_out = 1;
	return 0;
}
//备份接收帧
int Csd102::save_reci_frame(void * farme, int len)
{
	if (memcpy(this->reci_frame_bak, farme, len)==NULL) {
		PRINT_HERE
		;
		return -1;
	}
	this->reci_frame_bak_len = len;
	return 0;
}
//备份发送帧
int Csd102::save_tran_frame(void * frame, int len,
        u8* bakframe, int &bakframe_len, bool &hasbaked)
{
	if (memcpy(bakframe, frame, len)==NULL) {
		PRINT_HERE
		;
		return -1;
	}
	bakframe_len = len;
	hasbaked = true;
	return 0;
}
int Csd102::clear_fcv(void)
{
	struct Short_frame frame;
	memcpy(&frame, this->reci_frame_bak, sizeof(frame));
	frame.c_down.fcv = 0;
	return 0;
}
/*	召唤2级数据,没有2级数据,但是有1级数据
 回复M_NV_NA_2,fc:FN_M_NO_DAT 没有所召唤的数据(但是有1级数据ACD=1)
 Reset Communication Unit
 */
int Csd102::fun_M_NV_NA_2(u8 *frame_out, int &len_out) const
        {
	struct Short_frame * frame = (struct Short_frame *) frame_out;
	len_out = sizeof(struct Short_frame);
	frame->start_byte = START_SHORT_FRAME;
	frame->farme_tail.end_byte = END_BYTE;
	frame->link_addr = this->link_addr;
	frame->c_up.funcode = FN_M_NO_DAT;
	frame->c_up.acd = 1;		//有1级数据
	frame->farme_tail.cs = check_sum((u8*) frame+sizeof(START_SHORT_FRAME),
	                sizeof(union Ctrl_up)
	                                +sizeof(link_addr_t));
	return 0;

}
/*	回复链路状态请求(FN_C_RLK FC9)的帧
 回复: M_LKR_NA_2 FN_M_RSP FC11 以链路状态回应
 */
int Csd102::fun_M_LKR_NA_2(u8 *frame_out, int &len_out) const
        {
	struct Short_frame * frame = (struct Short_frame *) frame_out;
	len_out = sizeof(struct Short_frame);
	frame->start_byte = START_SHORT_FRAME;
	frame->farme_tail.end_byte = END_BYTE;
	frame->link_addr = this->link_addr;
	frame->c_up.prm = 0;		//上传
	frame->c_up.funcode = FN_M_RSP;
	frame->c_up.acd = 0;
	frame->c_up.dfc = 0;		//可以接收数据
	frame->farme_tail.cs = check_sum((u8*) frame+sizeof(START_SHORT_FRAME),
	                sizeof(union Ctrl_up)
	                                +sizeof(link_addr_t));
	return 0;

}
/*	复位远方链路确认帧 回复C_RCU_NA_2(复位远方链路[通讯单元])
 	 Reset Communication Unit
 */
int Csd102::fun_M_CON_NA_2(u8 *frame_out, int &len_out) const
        {
	struct Short_frame * farme = (struct Short_frame *) frame_out;
	len_out = sizeof(struct Short_frame);
	farme->start_byte = START_SHORT_FRAME;
	farme->farme_tail.end_byte = END_BYTE;
	farme->c_up.funcode = FN_M_CON;
	farme->link_addr = this->link_addr;
	farme->farme_tail.cs = check_sum((u8*) farme+sizeof(START_SHORT_FRAME),
	                sizeof(union Ctrl_up)
	                                +sizeof(link_addr_t));
	return 0;

}
/*返回终端时间帧 M_TI TA_2
 out:	farme_out	返回/发送到主站的帧
 len_out		帧长
 return:	0	成功
 */
int Csd102::fun_M_TI_TA_2(u8 *frame_out, int &len_out) const
        {
	struct m_tSystime systime;
	struct stFrame_M_TI_TA_2 *pframe;
	const u8 InfoObj_num = 1;		//信息体数量
	GetSystemTime_RTC(&systime);
	pframe = (struct stFrame_M_TI_TA_2 *) (frame_out);
	memset(pframe, 0, sizeof(struct stFrame_M_TI_TA_2));
	len_out = sizeof(struct stFrame_M_TI_TA_2);

	//pfarme->farme_head.len1=1;
	//printf("RTU time test:farme_head.len1=%d\n",pfarme->farme_head.len1);
	//整合:
	pframe->farme_head.start_byte1 = START_LONG_FRAME;
	pframe->farme_head.len1 = sizeof(struct Lpdu_head)+
	                sizeof(struct Tb);
	pframe->farme_head.len2 = pframe->farme_head.len1;
	pframe->farme_head.start_byte2 = START_LONG_FRAME;
	pframe->lpdu_head.c_up.funcode = FN_M_SEND_DAT;
	pframe->lpdu_head.c_up.prm = 0;
	pframe->lpdu_head.c_up.acd = 1;
	pframe->lpdu_head.c_up.dfc = 0;
	pframe->lpdu_head.link_addr = this->link_addr;
	pframe->asdu_head.typ = M_TI_TA_2;
	pframe->asdu_head.vaq.sq = 0;
	pframe->asdu_head.vaq.n = 1;//一个信息体
	pframe->asdu_head.cot.cause = COT_REQUEST;     //cot传输原因
	pframe->asdu_head.cot.pn = 0;     //
	pframe->asdu_head.cot.t = 0;     //不是测试(真正进行操作)
	pframe->asdu_head.asdu_addr = (InfoObj_num/256)+1;     //根据信息体的数量没增加255,值加1,从1开始.
	pframe->asdu_head.rad = 0;
	//TODO 把其他数据 意义弄清后赋值
	pframe->tb.ms = systime.msec;
	//pfarme->tb.eti=0;//费率
	pframe->tb.second = systime.sec;
	pframe->tb.min = systime.min;
	pframe->tb.hour = systime.hour;
	pframe->tb.day = systime.day;
	pframe->tb.month = systime.mon;
	pframe->tb.year = systime.year;
	pframe->tb.week = systime.week;
	pframe->tb.su = 0;     //非夏令时间(标准时间)
	pframe->tb.iv = 0;     //有效
	pframe->tb.res1 = 0;  //备用置零
	pframe->tb.res2 = 0;
	pframe->farme_tail.cs = check_sum(frame_out+sizeof(struct Frame_head)
	                , len_out-sizeof(struct Frame_head)
	                -sizeof(struct Frame_tail));
	pframe->farme_tail.end_byte = END_BYTE;
//
	printf("time=%d-%d-%d %d:%d:%d %dms \n",
	                systime.year, systime.mon, systime.day,
	                systime.hour, systime.min, systime.sec, systime.msec);
//	printf("RTU time test:farme_head.len1=%d sizeof frame=%d len_out=%d\n"
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
	{	sum+=m_transBuf.m_transceiveBuf[i];}
	m_transBuf.m_transceiveBuf[ptr++]=sum;
	m_transBuf.m_transceiveBuf[ptr++]=0x16;
	m_transBuf.m_transCount=ptr;
	m_LastSendBytes=m_transBuf.m_transCount;
	Clear_Continue_Flag();
	Clear_FrameFlags();
	return 0;
#endif
}
/*带时标(T)的电量(IT) 信息fun_M_IT_TA_2
 in:	frame_in
 len_in
 out:	frame_out	返回/发送到主站的帧
 len_out		帧长
 return:	0	成功
 */
int Csd102::fun_M_IT_TA_2(const u8 *frame_in, const int len_in,
        u8 *frame_out, int &len_out) const
        {
	struct stFrame_C_CI_NR_2 *fin = (struct stFrame_C_CI_NR_2 *) frame_in;
	printf("fin->start_addr=%d\n", fin->start_addr);
	printf("fin->end_addr=%d\n", fin->end_addr);
//调试返回帧
	frame_out[0] = 1;
	frame_out[1] = 2;
	frame_out[2] = 3;
	frame_out[3] = 4;
	frame_out[4] = 5;
	len_out = 5;
	return 0;
}
/*一般校验和程序.
 in:	a	数组(u8 *)
 len	数组长度(int)
 out	无
 return:		一个字节校验和(u8)
 */
u8 Csd102::check_sum(u8 const *a, int const len) const
        {
// i;
	u8 sum = 0;
	for (int i = 0; i<len; i++) {
		sum += a[i];
	}
	return sum;
}
/*打印字符数组*/
void Csd102::print_array(const u8 *transbuf, const int len) const
        {
	int i;
	for (i = 0; i<len; i++) {
		printf("%02X ", transbuf[i]);
	}
	printf("\n");
	return;
}
void Csd102::set_has_mirror_farme(bool val)
{
	this->has_mirror_farme=val;
}
