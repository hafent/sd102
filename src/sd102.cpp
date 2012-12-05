/**
 * @file:sd102.h
 *
 山东102规约 实现文件 引用 DL/T719-2000（IEC60870-5-102：1996）
 引用 GB/T 18657.2-2002 等效与 IEC60870-5-2:1990 链路传输规则*/
#include <sys/msg.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include "define.h"
#include "sys_utl.h"
#include "loopbuf.h"
#include "rtclocklib.h"
#include "HisDB.h"
#include "log.h"
#include "Hisfile.h"
#include "sd102.h"
#include <stdarg.h>
#include <iostream>
#include <time.h>
#include <errno.h>
#pragma pack(1)
#define SHOW_MSG 1 //显示接收和发送的报文
//
int err_no = 0;
enum err_no_e {
	ERR_WORRY_START_BYTE,  //起始码错误,不是0x10 和0x68
	ERR_
};
extern "C" CProtocol *
CreateCProto_sd102()
{
	printf(PREFIX"%d.%d.%d\n", MAJOR, MINOR, PATCHLEVEL);
	BUILD_INFO
	printf(PREFIX"create so lib\n");
	return new Csd102;
}

Csd102::Csd102()
{
	printf(PREFIX"struct Csd102\n");
	BUILD_INFO
	syn_char_num = 6;
//Syn_Head_Rece_Flag=0;
	m_ACD = ACD_NO_ACCESS;
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
		//this->acd = ACD_NO_DAT;
		//this->last_typ = TYP_M_UNUSED;
	this->link_addr = 0;
	this->status = 0;
	while (!qclass1.empty()) {
		qclass1.pop();
	}
	while (!qclass2.empty()) {
		qclass2.pop();
	}
	spon = 60;

//开始时备份帧应该被清空
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
	//last_typ = TYP_M_UNUSED;
	c_Link_Address_H = tmp_portcfg->m_ertuaddr>>8;
	c_Link_Address_L = (unsigned char) tmp_portcfg->m_ertuaddr;
	m_checktime_valifalg = tmp_portcfg->m_checktime_valiflag;
	m_suppwd = tmp_portcfg->m_usrsuppwd;
	m_pwd1 = tmp_portcfg->m_usrpwd1;
	m_pwd2 = tmp_portcfg->m_usrpwd2;
	m_retransmit_table = tmp_portcfg->m_retransmit_mtr;		//转发表
	retran_table_valid = (m_retransmit_table.empty()) ? 0 : 1;
	m_Max_Mtrnum = (!retran_table_valid) ?
	                                       (sysConfig->meter_num) :
	                                       (m_retransmit_table.size());
	this->link_addr = tmp_portcfg->m_ertuaddr;
	return 0;
}

//终端(从站)主动发送.在山东102,可以用於非平衡传输(协议规定)不需要
void Csd102::SendProc(void)
{
#if 1 //实现事件的上送,自发/突发 的实现,逻辑上不是很妥当
	struct m_tSystime systime;
	//模拟终端自发数据,保存数据到2类数据队列,主站轮询时发现有数据,就应该招过去.
	//用于发送终端事件?
	if (spon>600&&qclass2.size()<CLASS2MAX) {
		spon = 0;
		struct Frame f;
		struct stFrame_M_SP_TA_2* fsp =
		                (struct stFrame_M_SP_TA_2*) f.dat;
		f.len = sizeof(*fsp);
		fsp->head.start_byte1 = START_LONG_FRAME;
		fsp->head.start_byte2 = START_LONG_FRAME;
		fsp->head.len1 = sizeof(struct stFrame_M_SP_TA_2)
		                -sizeof(Frame_head)-sizeof(Frame_tail);
		fsp->head.len2 = fsp->head.len1;
		fsp->udat_head.cf_m.acd = !qclass1.empty();
		fsp->udat_head.cf_m.dfc = DFC_NOT_FULL;
		fsp->udat_head.cf_m.fcn = FCN_M_SEND_DAT;
		fsp->udat_head.cf_m.prm = PRM_UP;
		fsp->udat_head.cf_m.res = CF_RES;
		fsp->udat_head.link_addr = this->link_addr;
		fsp->duid.rad = RAD_ALL_SP_INFO;
		fsp->duid.typ = TYP_M_SP_TA_2;
		fsp->duid.vsq.n = 1;
		fsp->duid.vsq.sq = SQ_Similar;
		fsp->duid.cot.cause = COT_SPONT;
		GetSystemTime_RTC(&systime);
		getsystime(fsp->obj[0].tb, systime);
		//printf("####");
		showtime(fsp->obj[0].tb);
		fsp->tail.end_byte = END_BYTE;
		this->qclass2.push(f);
	} else {
		spon++;
	}
#endif
	return;
}

//接收(从主站发来的)报文 主处理流程
int Csd102::ReciProc(void)
{
#if 0 //just for receive base debug
	m_transBuf.m_transceiveBuf[0]=0x12;
	m_transBuf.m_transceiveBuf[1]=0x34;
	m_transBuf.m_transceiveBuf[2]=0x56;
	m_transBuf.m_transceiveBuf[3]=0x78;
	m_transBuf.m_transCount=4;
#endif
	int ret = -1;
	union Ctrl_c c;
	//1. get frame [会修改类成员]
	if (this->separate_msg(reci_frame)!=0) {
		//show_wait(status);
		return 0;
	}
	//2. verify frame
	if (this->verify_frame(reci_frame)!=0) {
		printf("verify err,ignord\n");
		return 0;
	}
#if SHOW_MSG
	printf(PREFIX"RECE:");
	print_array(reci_frame.dat, reci_frame.len);
#endif
	//测试时不验证重发. "false" is for debug
	if (false&&need_resend(reci_frame_bak, reci_frame)) {  //链路重发
	//重发本文的发送帧,重发
		printf(PREFIX"重发Resend Farme\n");
		//前后FCB应该不一样
		transfer(send_frame_bak);
		return 0;
	}
	//分类
	c.val = this->get_ctrl_field(reci_frame);
	switch (c.fcn) {
	//S2: **** 2类服务(写指令)
	case FCN_C_RCU:
		printf(PREFIX"S2: 0-复位链路\n");
		//TODO 可能终端施行复位操作,暂时就清除保存帧
		clear_fcb(reci_frame_bak);
		ret = 0;
		//rcu();
		if (ret!=0) {
			PRINT_HERE
			return 2100;
		}
		//终端初始化链路结束,给主站发送确认报文
		printf(PREFIX"S2: 0-确认\n");
		ack(send_frame);
//		ret = fun_M_CON_NA_2(send_frame);//响应帧
//		if (ret!=0) {
//			PRINT_HERE
//			return 2101;
//		}
		//初始化结束,返回初始化结束帧,并且置初始化结束报文为1类数据
		make_M_EI_NA_2(qclass1);
		break;
	case FCN_C_TRANS_DAT:
		printf(PREFIX"S2: 3-数据传输\n");
		//应答之后即刻 开始处理事务
		//TODO 应答之前是否要处理什么??
		printf(PREFIX"S2: 0-确认\n");
		ack(send_frame);
		if (reci_frame.dat[0]!=START_LONG_FRAME) {
			printf(PREFIX"####FN_C_TRANS_DAT,START_SHORT_FRAME\n");
			PRINT_HERE
			return 2200;
		}
		ret =
		                process_request(reci_frame, this->qclass1, this->qclass2);
		if (ret!=0) {
			PRINT_HERE
		}
		break;
		//S3: **** 3类服务(读数据)
	case FCN_C_RLK:		// 链路请求 FCV=0
		printf(PREFIX"S3: 9-请求链路状态\n");
		if (c.fcv!=0) {
			printf(PREERR"c.fcv!=0\n");
			PRINT_HERE
		}
		printf(PREFIX"S3: 11-回复链路状态\n");
		make_M_LKR_NA_2(send_frame);		//回复链路状态
		//PRINT_HERE
		break;
	case FCN_C_PL1:		//召唤1级数据
		printf(PREFIX"S3: 10-请求1类数据\n");
		/*FIXME 现在是在请求传输数据的时候就生成了数据帧,
		 应该改为在 1.请求数据时只应答,并保存数据帧到数据缓冲区.
		 2.在召唤数据时发送数据,并且生成各种头信息.(理清逻辑!)
		 流程: 	有 镜像1?
		 发镜像1
		 退出
		 有 1类?
		 发1类
		 退出
		 有 镜像2?
		 发镜像2
		 退出
		 所有数据帧均发送完成.
		 */
		if (this->qclass1.empty()) {  //没有数据,否定应答
			printf(PREFIX"NACK 否定回答 qsize=%d\n", qclass1.size());
			nack(send_frame);
		} else {
			printf(PREFIX"S3: 8-发送1类数据,qsize=%d\n",
			                qclass1.size());
			//有数据,发送数据
			send_frame = this->qclass1.front();  //出队列的也可能是之前的镜像帧
			this->qclass1.pop();
		}
		break;
	case FCN_C_PL2:
		printf(PREFIX"S3: 11-请求2类数据\n");
		if (!this->qclass2.empty()) {
			printf(PREFIX"S3: 8-发送2类数据,qsize=%d\n",
			                qclass2.size());
			send_frame = this->qclass2.front();
			this->qclass2.pop();
		} else {
			if (!this->qclass1.empty()) {
				printf(PREFIX"S3: 9-没有2类数据,但有1类数据 \n");
				make_M_NV_NA_2(send_frame);
				//print_array(send_frame.dat, send_frame.len);
			} else {
				printf(PREFIX"NACK 没有2类数据,也没有1类数据 \n");
				nack(send_frame);
			}
		}
		break;
	case FCN_C_RES1:
		printf(PREFIX"S3: 12-保留\n");
		PRINT_HERE
		break;
	case FCN_C_RES2:
		printf(PREFIX"S3: 13-保留\n");
		PRINT_HERE
		break;
	default:
		printf(PREERR"意外的服务类型: c.funcode=%d\n", c.fcn);
		PRINT_HERE
		break;
	}
	//SEND:	//发送帧
	transfer(send_frame);
	//对于帧计数有效的帧才考虑超时-重发机制
	if (c.fcv==1) {
		copyframe(send_frame_bak, send_frame);
		copyframe(reci_frame_bak, reci_frame);
	}
	return 0;
}
/*复制帧, 从sf复制到df
 */
int Csd102::copyframe(struct Frame &df, const struct Frame sf) const
        {
	memcpy(df.dat, sf.dat, sizeof(struct Frame));
	return 0;
}

/*S2:发送/确认帧 的终端应答(确认) 形成确认帧
 */
int Csd102::ack(struct Frame &f) const
        {
	f.dat[0] = START_SC_FRAME;
	f.len = sizeof(START_SC_FRAME);
	return 0;
}
/*S3的 没有数据的 否定应答	形成否定帧(nack)
 */
int Csd102::nack(struct Frame &f) const
        {
	f.dat[0] = START_SC_FRAME;
	f.len = sizeof(START_SC_FRAME);
	return 0;
}

/*发送帧 将 f 通过复制到 m_transBuf 结构体发送.
 in:	f	通用帧,字节数组+长度
 out:	m_transBuf	修改的发送缓冲区结构体,发送数据[类成员,隐含使用!]
 */
int Csd102::transfer(const struct Frame f)
{
#if SHOW_MSG
	printf(PREFIX"SEND:");
	print_array(f.dat, f.len);
#endif
	memcpy(this->m_transBuf.m_transceiveBuf, f.dat, f.len);
	this->m_transBuf.m_transCount = f.len;
	return 0;
}

/* 从帧中获取控制域,一个字节
 in:	f	帧/字节流
 out:
 return:	控制字节(union Ctrl_down)
 */
u8 Csd102::get_ctrl_field(const struct Frame f) const
        {
	union Ctrl_c c;
	if (f.len<(int) sizeof(struct Short_frame)) {
		//PRINT_HERE;
		//printf("frame length(%d) is too small\n",farme_len);
		return c.val;
	}
	switch (f.dat[0]) {
	case START_SHORT_FRAME:
		c.val = f.dat[0+sizeof(u8)];	//开始字节后面既是控制域
		break;
	case START_LONG_FRAME:
		c.val = f.dat[0+sizeof(struct Frame_head)];	//帧头后面既是控制域
		break;
	default:
		c.val = 0x00;
		printf(PREFIX"没有备份帧\n");
		PRINT_HERE
		break;
	}
	return c.val;
}

/*	初步分离出正确的报文.帧前的数据清除,帧尾的数据保留在缓冲队列.
 高效的过滤大部分不合格的报文
 经过处理之后准确无误的报文被保存在 readbuf[len] 数组中,传递出来
 In/Out:	m_recvBuf 输入缓冲区/修改 [注意:被隐含的使用]
 out:		f	通用帧结构
 return:	0-成功
 非0-失败
 */
int Csd102::separate_msg(struct Frame &f)
{
	int frame_len = 0;
	bool syn_head_ok = false;
//Syn_Head_Rece_Flag = 0;
	f.len = get_num(&m_recvBuf);		//缓冲区长度
#if 0
	                printf("*get_num=%d\n",len);
#endif
	if (!f.len) {
		return 1;		//长度不够/缓冲区没有数据,分割错误
	}
//1.查找
	while (f.len>=syn_char_num) {		//队列长度太短就不必再查找了
		copyfrom_buf(f.dat, &m_recvBuf, f.len);
#if 0
		printf("farme:");
		print_array(readbuf,len);
#endif
		if (!sync_head(f.dat, frame_len)) {		//查找到一帧
			syn_head_ok = true;
			break;
		} else {		//否则 1元素出队列 //防止垃圾数据出现在正常帧前面
			pop_char(&m_recvBuf);
			f.len = get_num(&m_recvBuf);
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
	f.len = get_num(&m_recvBuf);
	if (f.len) {		//截断到 帧尾,剩下的留在缓冲区
		f.len = get_buff(f.dat, &m_recvBuf, frame_len);
		if (f.len!=frame_len) {
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
/*分离报文子操作,按照格式分离,可以有效处理粘包,分离有误判的可能性,但很小
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
			farme_len = 0;
			return -1;
		}
	}
//变长帧
	if ((buffer[0]==START_LONG_FRAME)&&(buffer[3]==START_LONG_FRAME)
	                &&buffer[1]==buffer[2]) {
		u8 len = buffer[1];
		//最末尾的元素的index
		int end_index = sizeof(Frame_head)     //帧头
		+len			//链路数据单元(LPDU)长度
		                +sizeof(Frame_tail)     //帧尾
		-1;
		//printf("end_index=%d\n",end_index);
		if (buffer[end_index]==END_BYTE) {
			//printf("#end_index=%d\n",end_index);
			farme_len = sizeof(struct Frame_head)+len
			                +sizeof(struct Frame_tail);
			return 0;
		} else {
			farme_len = 0;
			return -1;
		}
	}
	farme_len = 0;
	return -1;
}
/* 进一步检验帧. 和校验,地址检测.不会写类的成员变量(const[this])
 检测 固定帧 和 变长帧
 in:	dat	数据数组
 len	数组长度
 return:	0-通过检测;非0-未通过检测
 */
int Csd102::verify_frame(const struct Frame f) const
        {
	struct Short_frame* farme = NULL;     //用于固定帧
	struct Udat_head* lpdu_head = NULL;     //用于变长帧
	struct Frame_tail* farme_tail = NULL;     //帧尾,用于变长帧
	union Ctrl_c c;
	u16 farme_link_addr = 0;     //链路地址
	u8 farme_cs = 0;     //本帧的cs
	u8 cs = 0;     //计算得到的cs
	switch (f.dat[0]) {     //分类讨论
	case START_SHORT_FRAME:
		farme = (struct Short_frame *) (f.dat);
		cs = check_sum(f.dat+sizeof(u8),
		                sizeof(union Ctrl_c)+sizeof(link_addr_t));
		//统一变量,由下面处理
		farme_link_addr = farme->link_addr;
		c.prm = farme->c_down.prm;
		farme_cs = farme->farme_tail.cs;
		break;
	case START_LONG_FRAME:
		lpdu_head = (struct Udat_head *) (f.dat+sizeof(Frame_head));
		//分解帧尾
		farme_tail = (struct Frame_tail*) (f.dat+f.len
		                -sizeof(Frame_tail));
		//校验:去掉头,(嘎嘣脆),去帧掉尾.校验中间部分
		cs = this->check_sum(f.dat+sizeof(Frame_head),
		                f.len-sizeof(Frame_head)-sizeof(Frame_tail));
		//统一
		farme_link_addr = lpdu_head->link_addr;
		c.prm = lpdu_head->cf_c.prm;
		farme_cs = farme_tail->cs;
		break;
	default:     //开始字符不对
		PRINT_HERE
		return 0x10;
		break;
	}
	//结束字符
	if (f.dat[f.len-1]!=END_BYTE) {
#if 0
		PRINT_HERE  //不需要打印出来
#endif
		return 0x11;
	}
	//1.和校验
	//printf("farme_cs=%02X  check sum=%02X \n",farme_cs,cs);
	if (farme_cs!=cs) {
#if 0 //调试期间 先忽略校验
		PRINT_HERE
		printf("CS err farme_cs=0x%02X but "
				"Calculate cs=0x%02X ,Ignore.\n", farme_cs, cs);
		return 0x12;
#endif
	}
	//2.判断是否下行帧,終端只接收下行的帧,不是下行的一定时错误的。
	if (c.prm!=PRM_DOWN) {
#if 0
		PRINT_HERE
		printf("c.prm[%d] !=C_PRM_DOWN[%d] ,Ignore.\n", c.prm,
				PRM_DOWN);
#endif
		return 0x13;
	}
	//3.判断是否传递给本终端
	if (farme_link_addr!=this->link_addr) {
#if 0
		PRINT_HERE
		printf("link_addr=%d farme.link_addr=%d \n", link_addr,
				farme->link_addr);
#endif
		return 0x14;
	}

	return 0;     //4. 结束
}

/*解析 FT1.2 固定帧长帧
 in:	fin	输入帧
 out:	f_out	输出帧
 return:	0	正确处理
 非0	处理失败
 */
int Csd102::process_short_frame(const struct Frame fin,
        struct Frame* f_out) const
        {
	PRINT_HERE
	printf("**** 处理短帧 ****\n");
	struct Short_frame *farme = (struct Short_frame *) &fin;
	struct Short_frame *frame_up = (struct Short_frame *) f_out;
	u8 cs = 0;
	if (fin.len<6) {
		PRINT_HERE
		printf("length(%d) too small\n", fin.len);
	}
//分类解析
	switch (farme->c_down.fcn) {
	case FCN_C_RCU:     //复位
//PRINT_HERE;
		frame_up->c_up.fcn = FCN_M_CON;
		frame_up->c_up.acd = 0;
		frame_up->c_up.dfc = 0;
		frame_up->link_addr = link_addr;
		cs = check_sum((u8*) frame_up+sizeof(START_SHORT_FRAME),
		                sizeof(union Ctrl_c)+sizeof(link_addr_t));
		frame_up->farme_tail.cs = cs;
		break;
	case FCN_C_TRANS_DAT:
		PRINT_HERE
		break;
	case FCN_C_RLK:		//请求链路状态
		frame_up->c_up.fcn = FCN_M_RSP;
		frame_up->c_up.acd = 0;
		frame_up->c_up.dfc = 0;
		frame_up->link_addr = link_addr;
		cs = check_sum((u8*) frame_up+sizeof(START_SHORT_FRAME),
		                sizeof(union Ctrl_c)+sizeof(link_addr_t));
		frame_up->farme_tail.cs = cs;
		PRINT_HERE
		;
		break;
	case FCN_C_PL1:
		PRINT_HERE
		break;
	case FCN_C_PL2:
		PRINT_HERE
		break;
	case FCN_C_RES1:
		PRINT_HERE
		break;
	case FCN_C_RES2:
		PRINT_HERE
		break;
	default:
		PRINT_HERE
		break;
	}
//memcpy(farme_out,&frame_up,sizeof(struct Short_farme));
//其他公共的信息 上行/开始/结束
	frame_up->start_byte = START_SHORT_FRAME;
	frame_up->farme_tail.end_byte = END_BYTE;
	frame_up->c_up.prm = 0;
	frame_up->c_up.res = 0;
	f_out->len = sizeof(struct Short_frame);
//	printf("send_farme.start_byte=%x T frame[0]=%x t_len=%d\n"
//	       ,send_farme.start_byte,T frame[0],t_len);
	return 0;
}
/*解析 FT1.2 变帧长帧 /处理请求,分类构造数据报文(可能是一系列).
 接收变长帧,按 type(TYP)分类处理下行报文(主站发出的)
 in	fin	输入的帧
 out	q1	输出到1类数据
 q2	输出到2类数据
 */
int Csd102::process_request(const struct Frame fin,
        std::queue<struct Frame> &q1,
        std::queue<struct Frame> &q2 __attribute__ ((unused)))
{
	int ret;
	int offset = 0;
	/*从输入帧中分解出一些数据单元
	 :Lpdu_head + Asdu_head + (信息体未解析) + Farme_tail*/
//	struct Udat_head *udat_head = (struct Udat_head *) (fin.dat
//			+ sizeof(struct Frame_head));
	offset = sizeof(Frame_head)+sizeof(Udat_head);
	struct Duid *uhead = (Duid *) (fin.dat+offset);
//	struct Frame_tail *frame_tail = (struct Frame_tail *) (fin.dat + fin.len
//			- sizeof(struct Frame_tail));
#if 0
	printf("addr=%d , cf=%02X \n",
			udat_head->link_addr ,udat_head->cf_c.val);
	printf("asdu_head->typ=%d , asdu_head->rad=%02X \n",
			duid->typ ,duid->rad);
	printf("farme_tail->cs=%02X , farme_tail->end_byte=%02X \n",
			frame_tail->cs ,frame_tail->end_byte);
#endif
	switch (uhead->typ) {
	case TYP_C_RD_NA_2:		//读制造厂和产品规范
		make_P_MP_NA_2(q1);
		PRINT_HERE
		break;
	case TYP_C_SP_NA_2:		//读单点数据
		ret = make_M_SP_TA_2(fin, q1);
		printf("1类数据队列 size %d\n", q1.size());
		if (ret!=0) {
			PRINT_HERE
		}
		break;
	case TYP_C_SP_NB_2:		//读单点数据
		ret = make_M_SP_TA_2(fin, q1);
		printf("1类数据队列 size %d\n", q1.size());
		if (ret!=0) {
			PRINT_HERE
		}
		break;
	case TYP_C_TI_NA_2:     //读取时间
		ret = make_M_TI_TA_2(q1);
		printf("1类数据队列 size %d\n", q1.size());
		if (ret!=0) {
			PRINT_HERE
		}
		break;
	case TYP_C_CI_NR_2:     //读取电量
		// 读取数据至1类数据队列
		ret = make_M_IT_TA_2(fin, q1);
		printf("1类数据队列 size %d\n", q1.size());
		if (ret!=0) {
			PRINT_HERE
		}
		break;
	case TYP_C_CI_NS_2:		//读记账复位电量
		make_M_IT_TD_2(fin, q1);
		PRINT_HERE
		break;
	case TYP_C_SYN_TA_2:		//设置时间
		ret = make_M_SYN_TA_2(fin, q1);
		if (ret!=0) {
			PRINT_HERE
		}
		break;
	case TYP_C_CI_NA_B_2:		//复费率记帐(计费)电能累计量
		make_M_IT_TA_B_2(fin, q1);
		PRINT_HERE
		break;
	case TYP_C_YC_TA_2:		//遥测
		ret = make_M_YC_TA_2(fin, q1);
		if (ret!=0) {
			PRINT_HERE
		}
		//PRINT_HERE
		break;
	case TYP_C_CI_NA_C_2:		//月结算复费率电能累计量
		make_M_IT_TA_C_2(fin, q1);
		PRINT_HERE
		break;
	case TYP_C_XL_NB_2:		//需量
		ret = make_M_XL_TA_2(fin, q1);
		if (ret!=0) {
			PRINT_HERE
		}
		//PRINT_HERE
		break;
	case TYP_C_CI_NA_D_2:		//谐波数据
		make_M_IT_TA_D_2(fin, q1);
		PRINT_HERE
		break;
	default:
		PRINT_HERE
		break;
	}
//
	return 0;
}

//清空备份帧,将备份帧全置0.
int Csd102::clear_fcb(struct Frame &fbak) const
        {

	memset(&fbak.dat, 0x00, MAX_FRAME_LEN);
	fbak.len = MAX_FRAME_LEN;
	//memcpy(&frame, this->reci_frame_bak, sizeof(frame));
	return 0;
}
/* 从 系统时间(systime)中将时间复制到Tb时间结构中.
 * */
int Csd102::getsystime(struct Tb &t, const struct m_tSystime systime) const
        {
	t.ms = systime.msec;
	//pframe->tb.eti=0;//费率
	t.second = systime.sec;
	t.min = systime.min;
	t.hour = systime.hour;
	t.day = systime.day;
	t.month = systime.mon;
	t.year = systime.year;
	t.week = systime.week;
	t.su = TB_STANDARD_TIME;     //非夏令时间(标准时间)
	t.iv = TB_VALID;     //有效
	t.res1 = TB_RESERVE1;     //备用置零
	t.res2 = TB_RESERVE2;
	return 0;
}
/* 从 系统时间(systime)中将时间复制到Ta时间结构中.
 * */
int Csd102::getsystime(struct Ta & t, const struct m_tSystime systime) const
        {
	t.min = systime.min;
	t.hour = systime.hour;
	t.day = systime.day;
	t.month = systime.mon;
	t.year = systime.year;
	t.week = systime.week;
	t.su = TB_STANDARD_TIME;     //非夏令时间(标准时间)
	t.iv = TB_VALID;     //有效
	t.res1 = TB_RESERVE1;     //备用置零
	t.res2 = TB_RESERVE2;
	return 0;
}
/* 将tm结构时间转化位 Ta结构时间
 * */
int Csd102::tm2ta(struct Ta & t, const struct tm time) const
        {
	t.min = time.tm_min;
	t.hour = time.tm_hour;
	t.day = time.tm_mday;
	t.month = time.tm_mon+1;
	t.year = time.tm_year+1900-2000;
	t.week = time.tm_wday;
	t.su = time.tm_isdst;     //非夏令时间(标准时间)
	t.iv = TB_VALID;     //有效
	t.res1 = TB_RESERVE1;     //备用置零
	t.res2 = TB_RESERVE2;
	return 0;
}
// 从Tb结构体设置.复制时间到(TMStruct)systime结构体,备用
int Csd102::setsystime(TMStruct &systime, const struct Tb t) const
        {
	systime.m_sec = t.ms;
	systime.second = t.second;
	systime.minute = t.min;
	systime.hour = t.hour;
	systime.dayofmonth = t.day;
	systime.month = t.month;
	systime.year = t.year;
	systime.dayofweek = t.week;
	return 0;
}

#pragma  GCC diagnostic ignored  "-Wunused-parameter"
// ************** 在监视方向的**过程信息** *************
/*	M_SP_TA_2	上传单点信息
 in:	fi	输入帧
 out:	q	输出队列
 return:	0	成功
 */
int Csd102::make_M_SP_TA_2(const struct Frame fi,
        std::queue<struct Frame> &q) const
        {
	struct Frame frame_out;
	// 数据过多时分帧发送
	//每帧最多可以包含的信息体个数,用户数据最多255,再根据单个信息体的大小确定
	//不同类型的帧,因为包含的信息体大小不同,可能造成最多可包含数信息体数量不同
	int maxperframe = 0;
	struct m_tSystime systime;
	//FIXME 区分出指令是  C_SP_NB_2 还是 C_SP_NA_2
	struct stFrame_C_SP_NB_2 *fin = (stFrame_C_SP_NB_2 *) fi.dat;
	if (time_range(fin->obj.starttime, fin->obj.endtime)/* 数据数据错误*/) {
		printf(PREFIX"input instruction err 输入指令无效\n");
		fin->duid.cot.cause = COT_ACTTREM;
		fin->farme_tail.cs = check_sum(*fin);
		q.push(fi);
		return 1;
	} else {	//有效 镜像帧 确认激活
		fin->duid.cot.cause = COT_ACTCON;
		fin->farme_tail.cs = check_sum(*fin);
		q.push(fi);
	}
	const int obj_num = 99;  //NOTE 信息体数量待计算
	maxperframe = (255-sizeof(Udat_head)-sizeof(Duid))
	                /sizeof(Obj_M_SP_TA_2);
	printf("总信息体数=%d 每帧最大信息体数=%d\n", obj_num, maxperframe);
	int frame_num = obj_num/maxperframe;	//数据应该分解成多少帧[1..frame_num]
	for (int fno = 0; fno<frame_num+1; fno++) {  // [0,frame_num]
		int n;	//本帧包含的信息体数量
		bool islastframe = (fno==frame_num);	//是最后一帧吗
		if (islastframe) {  //是最后一帧,个数为剩下的
			n = obj_num%maxperframe;
		} else {  //不是最后一帧,数量为最大可以包含的数量
			n = maxperframe;
		}
		printf("n=%d\n", n);
		frame_out.len = sizeof(Frame_head)+sizeof(Udat_head)
		                +sizeof(Duid)+sizeof(Obj_M_SP_TA_2)*n
		                +sizeof(Frame_tail);
		//struct stFrame_M_SP_TA_2{
		Frame_head* f_head = (Frame_head*) ((u8*) frame_out.dat);
		Udat_head* udat_head = (Udat_head*) ((u8*) f_head
		                +sizeof(Frame_head));
		Duid* duid = (Duid*) ((u8*) udat_head+sizeof(Udat_head));
		Obj_M_SP_TA_2* obj = (Obj_M_SP_TA_2 *) ((u8*) duid
		                +sizeof(Duid));
		Frame_tail* f_tail = (Frame_tail*) (frame_out.dat
		                +frame_out.len-sizeof(Frame_tail));
		//};
		f_head->start_byte1 = START_LONG_FRAME;
		f_head->len1 = sizeof(Udat_head)+sizeof(Duid)
		                +sizeof(Obj_M_SP_TA_2)*n;
		f_head->len2 = f_head->len1;
		f_head->start_byte2 = START_LONG_FRAME;
		udat_head->cf_m.fcn = FCN_M_SEND_DAT;
		udat_head->cf_m.prm = PRM_UP;
		udat_head->cf_m.acd = ACD_ACCESS;
		udat_head->cf_m.dfc = DFC_NOT_FULL;
		udat_head->cf_m.res = CF_RES;
		udat_head->link_addr = this->link_addr;
		duid->typ = TYP_M_SP_TA_2;
		duid->vsq.n = n;
		duid->vsq.sq = SQ_Similar;
		duid->cot.cause = COT_REQUEST;
		duid->cot.pn = COT_PN_ACK;
		duid->cot.t = COT_T_NOT_TEST;
		duid->rad = RAD_DEFAULT;
		duid->rtu_addr = makeaddr(obj_num);
		// obj_M_SP_TA_2[] :
		for (u8 i = 0; i<n; i++) {	//TODO 从文件读取数据/事件?
			int addr = 0;	//
			obj[i].sp.spa = 1;
			getsystime(obj[i].tb, systime);
		}
		GetSystemTime_RTC(&systime);

		f_tail->cs = this->check_sum(frame_out.dat+sizeof(Frame_head),
		                f_head->len1);
		f_tail->end_byte = END_BYTE;
		//		printf("duid->c_up.funcode:%d", udat_head->c_up.funcode);
		//		printf("f_tail->end_byte:%X", f_tail->end_byte);
		//print_array(frame_out.dat, frame_out.len);
		q.push(frame_out);		//这一帧加入到队列
		printf("push to queue,qsize=%d\n", q.size());
	}		//fno 一帧结束
	return 0;
}
void Csd102::print_tou_dat(const struct Tou tou) const
        {
	printf("%d[%d] ", tou.FA.total.val, tou.FA.total.iv);
	printf("%d[%d] ", tou.RA.total.val, tou.FA.total.iv);
	printf("%d[%d] ", tou.FR.total.val, tou.FA.total.iv);
	printf("%d[%d] ", tou.RR.total.val, tou.FA.total.iv);
	printf("\n");
	return;
}
void Csd102::print_tou_head(const struct touFilehead filehead) const
        {
	printf("tou file head:%d-%d-%d([%02X][%02X][%02X])\n"
			"             sc= hi: %d[%02X] lo: %d[%02X]\n"
			"             savenumber=%d[%02X]\n"
			"             save flag="
			"[%02X] [%02X] [%02X] [%02X]\n",
	                filehead.year, filehead.month, filehead.day,
	                filehead.year, filehead.month, filehead.day,
	                filehead.save_cycle_hi, filehead.save_cycle_hi,
	                filehead.save_cycle_lo, filehead.save_cycle_lo,
	                filehead.save_number, filehead.save_number,
	                filehead.save_flag1, filehead.save_flag2,
	                filehead.save_flag3, filehead.save_flag4
	                );
	return;
}

/*	M_IT_TA_2 发送带时标(T)的电量(IT)
 in:	fi	输入帧结构
 out:	q1	输出一系列数据帧到队列和头尾两个镜像帧
 return:	0	成功
 */
int Csd102::make_M_IT_TA_2(const struct Frame fi,
        std::queue<struct Frame> &q1) const
        {
	struct Frame fout;
	// 数据过多时分帧发送
	// 每帧最多可以包含的信息体个数,用户数据最多255,再根据单个信息体的大小确定
	// 不同类型的帧,因为包含的信息体大小不同,可能使最多可包含数信息体数量不同
	int maxperframe = 0;
	struct m_tSystime systime;
	struct stFrame_C_CI_NR_2 *fin = (stFrame_C_CI_NR_2 *) fi.dat;
	printf("fin->obj.start_ioa=%d\t", fin->obj.start_ioa);
	printf("fin->obj.end_io=%d\n", fin->obj.end_ioa);
	showtime(fin->obj.Tstart);
	showtime(fin->obj.Tend);
	if (this->format_ok(*fin)!=0/* 数据数据错误*/) {
		make_mirror_1(fin, false);
		printf(PREFIX"input instruction err 输入指令无效\n");
		q1.push(fi);
		return 1;
	} else {	//有效 镜像帧
		make_mirror_1(fin, true);
		q1.push(fi);
	}

	unsigned long T1, T2, T3, T4, e_val;
	GetSystemTime_RTC(&systime);
	struct Ta Tnow;
	getsystime(Tnow, systime);
	T1 = get_min(fin->obj.Tstart);
	T2 = get_min(fin->obj.Tend);
	T3 = get_min(Tnow);
	Ta taa;
	time_t tsec=T1*60L;
	struct tm t_ch;
	memset(&t_ch,0x00,sizeof(t_ch));
//	printf("1秒数=%d\n",tsec);
//	gmtime_r(&tsec,&t_ch);
//	printf("2秒数=%d\n",tsec);
//	printf("秒转日期 %d-%d-%d %d:%d:%d \n",
//		t_ch.tm_year,t_ch.tm_mon,t_ch.tm_mday
//		,t_ch.tm_hour,t_ch.tm_min,t_ch.tm_sec);
	printf("Time to min(start)=%ld\n", T1);
	printf("Time to min(end)=%ld\n", T2);
	int addr = fin->obj.end_ioa-fin->obj.start_ioa+1;
	u8 month = 0;
	u8 day = 0;


	int timestep_last = 0;//上次采集的周期,时间递增的步距

	int timestep=0;//最小采集周期,步距
	int snumber = 0;	//采样点个数
	struct Obj_M_IT_TX_2 objit;
	//向后移动一个周期(周期可能变化!)
	//查询表示方法是 查 00分钟 到 15分钟 表示 [00,15] 两端闭区间
	// 而采集表示方法是 第一点,00采样周期15分钟,那个[00,15) 区间都是由第一个采样点表示
	// [15,30) 由第二个采样点表示. 前闭后开.
	for (u32 t = T1; t<=T2; t += timestep) {
		snumber++;
		tsec=t*60;
		gmtime_r(&tsec,&t_ch);
		month=t_ch.tm_mon+1;
		day=t_ch.tm_mday;
		printf("[本条记录时间]:");
		showtime(t_ch);
		//Obj_M_IT_TX_2 obj[mtrno][sampleno];
		//最外层循环.按表号(文件)来循环 一般日期跨度小,
		for (int i = fin->obj.start_ioa; i<=fin->obj.end_ioa; i++) {//信息体号
			//mtrno=(i-1)/4;
			int mtrno = (i-1)/4;	//从信息体计算成表号
			int datclass = (i-1)%4;		//信息体对应在每个表不同数据分类
			//FIXME　重要改动:信息体数量＝采集时间／采集周期＊所采集的信息体范围
			std::string filename;
			GetFileName_Day(&filename, month, day, mtrno, TASK_TOU);
			std::cout<<"\t打开文件:"<<filename<<std::endl;
			int ret = 0;
			FILE *fp;
			touFilehead filehead;
			Tou toudat;
			memset(&toudat, 0x00, sizeof(toudat));
			fp = fopen(filename.c_str(), "rb");
			if (fp==NULL) {//应该是没有这块表,跳过继续寻找下一块表的数据
				perror("open file");
				//如果是没有文件的错误,则继续
				continue;
				//如果是其他错误,那应该注意了.
			}
			ret = fread(&filehead, sizeof(filehead), 1, fp);
			if (ret!=1) {
				printf("读取电量文件头错误:ret=%d", ret);
				PRINT_HERE
			}

			//TODO 比较头,是否是本年本月的数据,因为月是对保存时间(几个月)去摸的.
			//采样周期,分钟.
			//! 注意! 这里修改了外层循环的步距
			timestep_last=timestep;
			timestep = filehead.save_cycle_hi*256
			                +filehead.save_cycle_lo;
			int tmpstep=timestep;
//			if((t%(60*24))%timestep!=0){//如果不是某一小时的整数周期分钟,向后元整
//				//开始时间从1分钟变到5分钟,如果周期位5.
//				t=t+(timestep-t_ch.tm_min%timestep);
//				tsec=t*60;
//				gmtime_r(&tsec,&t_ch);
//				printf("向后元整");
//			}
			/*对于不同的表采集周期不一样的情况:
			 * 产生这种现象的原因可能是修改了表计个数,同是采样周期也修改了,
			 * 造成之前存储的值的某些表与现在的不一样,过了一个最大存储周期后
			 * 他们会变的相同的.
			 * 保证按最小的采集周期计算
			*/
			int bchangectl=false;
			if(timestep>timestep_last
					&& timestep_last!=0
					&& timestep!=0){
				//printf("改变周期 %d -> %d",timestep_last,timestep);

				timestep=timestep_last;

				bchangectl=true;
			}else{
				bchangectl=false;
			}
			 //如果:(T1-t)%timestep==0
			//时间=采样点*采样周期
			int timeoffset = 0;	//按时间的偏移量,时间步距=采样周期
			int sn = 0;	//采样点偏移量

			//距离这一天凌晨00:00的偏移量(分钟)/采样周期=文件中偏移的字节
			int Soffset = (t%(60*24))/timestep;
			int offset_mmin =(t%(60*24))%timestep;//一天中偏移了过少分钟
			if(offset_mmin!=0){//不正好是周期的整数倍
				t+=(timestep-offset_mmin);//时间加上几分钟,使到达分钟的整数倍.
				//重新计算现在的日期和时间.
				tsec=t*60;
				gmtime_r(&tsec,&t_ch);
				printf("时刻非整周期");
			}
			/*对于一下这种情况:
			 * 时刻 | 表1 | 表2 |
			 * 00:00| 有  | 有 |
			 * 00:05| 无  | 有 |
			 * 00:10| 无  | 有 |
			 * 00:15| 有  | 有 |
			 * 由于某些情况表1和表2的周期不一样.则采集时按照最小的周期采集,
			 * 没有保存数据的表1保留上次的数据,即5分钟的数据和10分钟的
			 * 数据都是0分钟的数据.且有效.
			 * */
			if(bchangectl){//周期改变了
				//printf("这条数据无效");
				Soffset=(t%(60*24))/tmpstep;
				//只是不偏移,但是数据还是生成的,只是和之前的数据是一样的
			}

			//filehead.day=9;
			//print_tou_head(filehead);
			fseek(fp, Soffset*sizeof(Tou), SEEK_CUR);//这个不会返回错误
			//读取这个时间点的数据
			ret = fread(&toudat, sizeof(toudat), 1, fp);
			if (ret!=1) {
				//如果到等于0,可能就是读到结尾了.
				printf("读取电量信息错误:ret=%d", ret);
				PRINT_HERE
			}
			objit.ioa=i;
			memcpy(&objit.it_power,&toudat.FA.total,sizeof(objit.it_power));
			objit.cs=0xFF;//校验暂时不进行.
			qIT.push(objit);
			printf("\t表号:%d ", mtrno);
			switch (datclass) {
			case 0:
				printf("类别0 正向有功\t");
				break;
			case 1:
				printf("类别1 反向有功\t");
				break;
			case 2:
				printf("类别2 正向无功\t");
				break;
			case 3:
				printf("类别3 反向无功\t");
				break;
			default:
				printf("类别X %d \t", datclass);
				PRINT_HERE
				break;
			}
			printf("周期=%d(分钟),偏移量:%d(个)tou 采样次数:%d个\n\t\t",
			                filehead.save_cycle_lo, Soffset, snumber);
			print_tou_dat(toudat);
			sn++;
			fclose(fp);
		}
		//所有请求的信息体历史文件都不存在.
		if(timestep==0){
			printf("没有请求的数据!\n");
			break;
		}
		//一次时间完成:(对应帧中的信息体公共时间单元)
		tm2ta(taa,t_ch);
		qTa.push(taa);
	}
	//这里开始检查写两个队列的数据是否正确:
	printf("记录队列(时间条目):\t\t\tqTa.size=%d\n",qTa.size());
	printf("数据队列(信息体个数*时间条目):\tqIT.size=%d\n",qIT.size());
//	ret = Search_CircleDBS(filename, 12,
//	                12, 1,
//	                &Save_Num, &Save_XL[0], TASK_TOU);
	//printf("ret=%d\n", ret);


	const int obj_num =qIT.size();//所有信息体的个数. (fin->obj.end_ioa-fin->obj.start_ioa+1);// 还要乘以采集时间/采集周期
	//一条时刻记录中包含的信息体数量,当传了几帧(因为一帧传不完)后,总数等于这个,
	//则要换帧,并且换Ta.
	const int rs_len =fin->obj.end_ioa-fin->obj.start_ioa+1;
	int cur_re_idx=0;//当前记录中的信息体个数(索引)指示这是本记录的第几个信息体,base on 0
	// 但前总的信息体索引从0到max 共max+1个
	int io_ind=0;
	//一帧最多可以包含的信息体个数(含)
	maxperframe = (MAX_UDAT_LEN-sizeof(Udat_head)-sizeof(Duid)-sizeof(Ta))
	                /sizeof(Obj_M_IT_TX_2);
	int meybe_err=0;
	printf("每帧最大信息体数=%d 每条记录包含的信息体数量%d\n", maxperframe,rs_len);
	while( !qIT.empty()
			&& meybe_err++ <256 //循环太多次的话可能就是出错了,或者数据量不正常的多.
			){			//直到数据传玩
		printf("*");
		int n;	//本帧包含的信息体数量
		//本帧个数待确定
		if (rs_len<maxperframe) {  //是最后一帧,个数为剩下的
			n = rs_len;
		} else {
			//记录级长-当前索引<=最大信息体数,则这帧可以讲剩余部分全部发完
			if((rs_len-1-cur_re_idx)<=maxperframe){
				n=rs_len-cur_re_idx;
			}else{
				n=maxperframe;
			}
		}
		printf("n=%d\n", n);
		fout.len = sizeof(Frame_head)+sizeof(Udat_head)
		                +sizeof(Duid)+sizeof(Obj_M_IT_TX_2)*n
		                +sizeof(Ta)+sizeof(Frame_tail);
		//struct stFrame_M_IT_TA_2{//将u8数组依次分割成所有帧结构
		int offset = 0;
		Frame_head* f_head = (Frame_head*) (fout.dat+offset);
		offset += sizeof(Frame_head);
		Udat_head* udat_head = (Udat_head*) (fout.dat+offset);
		offset += sizeof(Udat_head);
		Duid* duid = (Duid*) (fout.dat+offset);
		offset += sizeof(Duid);
		Obj_M_IT_TX_2* obj = (Obj_M_IT_TX_2 *) (fout.dat+offset);
		offset += sizeof(Obj_M_IT_TX_2)*n;
		Ta* ta = (Ta *) (fout.dat+offset);
		offset += sizeof(Ta);
		Frame_tail* f_tail = (Frame_tail*) (fout.dat+offset);
		//};
		f_head->start_byte1 = START_LONG_FRAME;
		f_head->len1 = fout.len-sizeof(Frame_head)-sizeof(Frame_tail);
		f_head->len2 = f_head->len1;
		f_head->start_byte2 = START_LONG_FRAME;
		udat_head->cf_m.fcn = FCN_M_SEND_DAT;
		udat_head->cf_m.prm = PRM_UP;
		udat_head->cf_m.acd = ACD_ACCESS;  //有数据
		udat_head->cf_m.dfc = DFC_NOT_FULL;
		udat_head->cf_m.res = CF_RES;
		udat_head->link_addr = this->link_addr;
		duid->typ = TYP_M_IT_TA_2;
		duid->vsq.n = n;
		duid->vsq.sq = SQ_Similar;
		duid->cot.cause = COT_REQUEST;
		duid->cot.pn = COT_PN_ACK;
		duid->cot.t = COT_T_NOT_TEST;
		duid->rad = RAD_DEFAULT;
		duid->rtu_addr = makeaddr(io_ind+1);//根据信息体索引填写地址,
		//从信息体队列弹出若干信息体(直到本时刻的信息体全发完,
		//或者发不完,已经超过最大帧长.
		int i=0;//本帧信息体现在数量,每帧都重置
		//M_IT_TX_2_iObj []
		while(!qIT.empty() 			//1.数据传玩了.
				&& i<maxperframe	//2.信息体数量过多,分帧
				&& cur_re_idx<rs_len	//3.本记录所有信息体读完,换一帧
				){
			/*正向有功 addr = (电表号)*4+1	;ti=0
			 *反向有功 addr = (电表号)*4+2	;ti=1
			 *正向无功 addr = (电表号)*4+3	;ti=2
			 *反向无功 addr = (电表号)*4+4	;ti=3
			 */
//			int addr = fin->obj.start_ioa-1+fno*maxperframe+i;
//			int mtrno = addr/4;		//表号 base 0
//			int ti = (addr-1)%4;		//电量类型
//			//数据无效标志,
//			bool iv;
//			obj[i].ioa = addr;
//			obj[i].it_power.dat = ti;
//			//TODO 其他事件待定
//			obj[i].it_power.d_status.val = 0xFF;
//			//TODO 校验的源待定.
//			obj[i].cs = check_sum((u8*) &obj[i], sizeof(It));
			printf("#");
			obj[i]=qIT.front();
			qIT.pop();
			i++;//最小的循环累加:一帧中信息体排列下标 0~maxperframe-1
			cur_re_idx++;//次级累加: 一条记录中信息体排列下标 0~rs_len-1
			io_ind++;//最大: 本次请求的数据的信息体排列下标 0~?
		}
		*ta=qTa.front();
		//一条时刻的记录已经发送完,换下一个时刻.
		if(cur_re_idx>=rs_len){
			cur_re_idx=0;
			qTa.pop();
		}
		f_tail->cs = this->check_sum(fout.dat+sizeof(Frame_head),
		                f_head->len1);
		f_tail->end_byte = END_BYTE;
//		printf("duid->cf_m.funcode:%d", udat_head->cf_m.funcode);
//		printf("f_tail->end_byte:%X", f_tail->end_byte);
		//print_array(fout.dat, fout.len);
		q1.push(fout);		//这一帧加入到队列
		printf("push to 1类数据队列 ,qsize=%d\n", q1.size());
		printf("结束:记录队列(时间条目):\t\t\tqTa.size=%d\n",qTa.size());
		printf("     数据队列(信息体个数*时间条目):\tqIT.size=%d\n",qIT.size());
	}		//fno 一帧结束

	//完成后还有一帧镜像帧,激活结束
	make_mirror_2(fin);
	q1.push(fi);
	return 0;
}
/*	M_IT_TD_2 周期复位记账(计费)电能累计量,每个量为四个八位位组
 * */
int Csd102::make_M_IT_TD_2(const struct Frame fi,
        std::queue<struct Frame> &q) const
        {     //M_IT_TD_2
	return 0;
}
/*	M_IT_TA_B_2 复费率记帐(计费)电能累计量
 * */
int Csd102::make_M_IT_TA_B_2(const struct Frame fi,
        std::queue<struct Frame> &q) const
        {     //M_IT_TA_B_2
	return 0;
}
/*	M_YC_TA_2 遥测
 * */
int Csd102::make_M_YC_TA_2(const struct Frame fi,
        std::queue<struct Frame> &q) const
        {
	struct Frame frame_out;
	// 数据过多时分帧发送
	//每帧最多可以包含的信息体个数,用户数据最多255,再根据单个信息体的大小确定
	//不同类型的帧,因为包含的信息体大小不同,可能造成最多可包含数信息体数量不同
	int maxperframe = 0;
	struct m_tSystime systime;
	struct stFrame_C_YC_TA_2 *fin = (stFrame_C_YC_TA_2 *) fi.dat;
	printf("fin->obj.start_ioa=%d\n", fin->obj.start_ioa);
	printf("fin->obj.end_io=%d\n", fin->obj.end_ioa);
	showtime(fin->obj.Tstart);
	showtime(fin->obj.Tend);
	if (this->format_ok(*(stFrame_C_CI_NR_2*) fin)!=0) {
		fin->duid.cot.cause = COT_ACTTREM;
		fin->farme_tail.cs = check_sum(*fin);
		printf(PREFIX"input instruction err 输入指令逻辑错误\n");
		q.push(fi);
		return 1;
	} else {	//有效 镜像帧
		fin->duid.cot.cause = COT_ACTCON;
		fin->farme_tail.cs = check_sum(*fin);
		q.push(fi);
	}
	const int obj_num = (fin->obj.end_ioa-fin->obj.start_ioa+1);
	maxperframe = (255-sizeof(Udat_head)-sizeof(Duid)-sizeof(Ta))
	                /sizeof(Obj_M_YC_TA_2);
	printf("总信息体个数(个)=%d 每帧最大信息体数(个)=%d\n", obj_num, maxperframe);
	int frame_num = obj_num/maxperframe;	//数据应该分解成多少帧[1..frame_num]
	for (int fno = 0; fno<frame_num+1; fno++) {  // [0,frame_num]
		int n;
		bool islastframe = (fno==frame_num);
		if (islastframe) {
			n = obj_num%maxperframe;
		} else {
			n = maxperframe;
		}

		frame_out.len = sizeof(Frame_head)+sizeof(Udat_head)
		                +sizeof(Duid)+sizeof(Obj_M_YC_TA_2)*n
		                +sizeof(Ta)+sizeof(Frame_tail);
		printf("本帧信息体个数n=%d 长度 len=%d 最长<=261\n", n, frame_out.len);
		//struct stFrame_M_YC_TA_2{
		Frame_head* fhead = (Frame_head*) ((u8*) frame_out.dat);
		Udat_head* ufead = (Udat_head*) ((u8*) fhead
		                +sizeof(Frame_head));
		Duid* duid = (Duid*) ((u8*) ufead+sizeof(Udat_head));
		Obj_M_YC_TA_2* obj = (Obj_M_YC_TA_2 *) ((u8*) duid
		                +sizeof(Duid));
		Ta* ta = (Ta *) (frame_out.dat+frame_out.len
		                -sizeof(Frame_tail)-sizeof(Ta));  //从后面算方便
		Frame_tail* f_tail = (Frame_tail*) (frame_out.dat
		                +frame_out.len-sizeof(Frame_tail));
		//};
		fhead->start_byte1 = START_LONG_FRAME;
		fhead->len1 = sizeof(Udat_head)+sizeof(Duid)
		                +sizeof(Obj_M_YC_TA_2)*n+sizeof(Ta);
		fhead->len2 = fhead->len1;
		fhead->start_byte2 = START_LONG_FRAME;
		ufead->cf_m.fcn = FCN_M_SEND_DAT;
		ufead->cf_m.prm = PRM_UP;
		ufead->cf_m.acd = ACD_ACCESS;
		ufead->cf_m.dfc = DFC_NOT_FULL;
		ufead->cf_m.res = CF_RES;
		ufead->link_addr = this->link_addr;
		duid->typ = TYP_M_YC_TA_2;
		duid->vsq.n = n;
		duid->vsq.sq = 0;
		duid->cot.cause =
		                (islastframe) ? COT_ACTTREM : COT_REQUEST;
		duid->cot.pn = COT_PN_ACK;
		duid->cot.t = COT_T_NOT_TEST;
		duid->rad = RAD_DEFAULT;
		duid->rtu_addr = makeaddr(obj_num);
		//M_YC_TA_2_InfoObj []
		for (u8 i = 0; i<n; i++) {
			/*（n - 1）×8 + 1总有功功率
			 （n - 1）×8 + 2 总无功功率
			 （n - 1）×8 + 3 A相电压
			 （n - 1）×8 + 4 B相电压
			 （n - 1）×8 + 5 C相电压
			 （n - 1）×8 + 6 A相电流
			 （n - 1）×8 + 7 B相电流
			 （n - 1）×8 + 8 C相电流
			 */
			int addr = fin->obj.start_ioa+fno*maxperframe+i;
			int mtrno = addr/8;		//表号 base 0
			//数据无效标志,
			bool iv = false;
			obj[i].ioa = addr;
			obj[i].rm.dat = 0xFF0000FF;
			obj[i].rm.dst.val = 0xAA;
			obj[i].rm.dst.iv = iv;
		}
		GetSystemTime_RTC(&systime);
		getsystime(*ta, systime);
		//struct Frame_tail
		f_tail->cs = this->check_sum(frame_out.dat+sizeof(Frame_head),
		                fhead->len1);
		f_tail->end_byte = END_BYTE;
		//		printf("duid->cf_m.funcode:%d", ufead->c_up.funcode);
		//		printf("f_tail->end_byte:%X", f_tail->end_byte);
		//print_array(frame_out.dat, frame_out.len);
		q.push(frame_out);			//这一帧加入到队列
		printf("push to queue,qsize=%d\n", q.size());
	}			//fno 一帧结束
	return 0;
}

/*	M_XL_TA_2 需量
 * */
int Csd102::make_M_XL_TA_2(const struct Frame fi,
        std::queue<struct Frame> &q) const
        {
	struct Frame frame_out;
	// 数据过多时分帧发送
	//每帧最多可以包含的信息体个数,用户数据最多255,再根据单个信息体的大小确定
	//不同类型的帧,因为包含的信息体大小不同,可能造成最多可包含数信息体数量不同
	int maxperframe = 0;
	struct m_tSystime systime;
	struct stFrame_C_XL_NB_2 *fin = (stFrame_C_XL_NB_2 *) fi.dat;
	printf("fin->obj.start_ioa=%d\n", fin->obj.start_ioa);
	printf("fin->obj.end_io=%d\n", fin->obj.end_ioa);
	showtime(fin->obj.Tstart);
	showtime(fin->obj.Tend);
	if (this->format_ok(*(stFrame_C_CI_NR_2*) fin)!=0) {
		fin->duid.cot.cause = COT_ACTTREM;
		fin->farme_tail.cs = check_sum(*fin);
		printf(PREFIX"input instruction err 输入指令逻辑错误\n");
		q.push(fi);
		return 1;
	} else {	//有效 镜像帧
		fin->duid.cot.cause = COT_ACTCON;
		fin->farme_tail.cs = check_sum(*fin);
		q.push(fi);
	}
	const int obj_num = (fin->obj.end_ioa-fin->obj.start_ioa+1);
	maxperframe = (255-sizeof(Udat_head)-sizeof(Duid)-sizeof(Ta))
	                /sizeof(Obj_M_XL_TA_2);
	printf("总信息体个数(个)=%d 每帧最大信息体数(个)=%d\n", obj_num, maxperframe);
	int frame_num = obj_num/maxperframe;	//数据应该分解成多少帧[1..frame_num]
	for (int fno = 0; fno<frame_num+1; fno++) {  // [0,frame_num]
		int n;
		bool islastframe = (fno==frame_num);
		if (islastframe) {
			n = obj_num%maxperframe;
		} else {
			n = maxperframe;
		}
		frame_out.len = sizeof(Frame_head)+sizeof(Udat_head)
		                +sizeof(Duid)+sizeof(Obj_M_XL_TA_2)*n
		                +sizeof(Ta)+sizeof(Frame_tail);
		printf("本帧信息体个数n=%d 长度 len=%d 最长<=261\n", n, frame_out.len);
		// struct stFrame_M_XL_TA_2{
		Frame_head* fhead = (Frame_head*) ((u8*) frame_out.dat);
		Udat_head* ufead = (Udat_head*) ((u8*) fhead
		                +sizeof(Frame_head));
		Duid* duid = (Duid*) ((u8*) ufead+sizeof(Udat_head));
		Obj_M_XL_TA_2* obj = (Obj_M_XL_TA_2 *) ((u8*) duid
		                +sizeof(Duid));
		Ta* ta = (Ta *) (frame_out.dat+frame_out.len
		                -sizeof(Frame_tail)-sizeof(Ta));
		Frame_tail* f_tail = (Frame_tail*) (frame_out.dat
		                +frame_out.len-sizeof(Frame_tail));
		// };
		fhead->start_byte1 = START_LONG_FRAME;
		fhead->len1 = sizeof(Udat_head)+sizeof(Duid)
		                +sizeof(Obj_M_XL_TA_2)*n+sizeof(Ta);
		fhead->len2 = fhead->len1;
		fhead->start_byte2 = START_LONG_FRAME;
		ufead->cf_m.fcn = FCN_M_SEND_DAT;
		ufead->cf_m.prm = PRM_UP;
		ufead->cf_m.acd = ACD_ACCESS;
		ufead->cf_m.dfc = DFC_NOT_FULL;
		ufead->cf_m.res = CF_RES;
		ufead->link_addr = this->link_addr;
		duid->typ = TYP_M_YC_TA_2;
		duid->vsq.n = n;
		duid->vsq.sq = 0;
		duid->cot.cause = COT_REQUEST;
		duid->cot.pn = COT_PN_ACK;
		duid->cot.t = COT_T_NOT_TEST;
		duid->rad = RAD_DEFAULT;
		duid->rtu_addr = makeaddr(obj_num);
		//M_YC_TA_2_InfoObj []
		for (u8 i = 0; i<n; i++) {
			/*正向有功 addr = (电表号)*4+1	;ti=0
			 *反向有功 addr = (电表号)*4+2	;ti=1
			 *正向无功 addr = (电表号)*4+3	;ti=2
			 *反向无功 addr = (电表号)*4+4	;ti=3
			 */
			int addr = fin->obj.start_ioa+fno*maxperframe+i;
			bool iv;
			obj[i].ioa = addr;
			obj[i].mmd.total_maxdemand = 0x0;
			/* 发生时间 */
		}
		GetSystemTime_RTC(&systime);
		getsystime(*ta, systime);
		//struct Frame_tail
		f_tail->cs = this->check_sum(frame_out.dat+sizeof(Frame_head),
		                fhead->len1);
		f_tail->end_byte = END_BYTE;
		//		printf("duid->cf_m.funcode:%d", ufead->c_up.funcode);
		//		printf("f_tail->end_byte:%X", f_tail->end_byte);
		//print_array(frame_out.dat, frame_out.len);
		q.push(frame_out);			//这一帧加入到队列
		printf("push to queue,qsize=%d\n", q.size());
	}			//fno 一帧结束
	return 0;
}
/*	M_IT_TA_C_2 月结算复费率电能累计量
 * */
int Csd102::make_M_IT_TA_C_2(const struct Frame fi,
        std::queue<struct Frame> &q) const
        {
	return 0;
}
/*	M_IT_TA_D_2 表计谐波数据
 * */
int Csd102::make_M_IT_TA_D_2(const struct Frame fi,
        std::queue<struct Frame> &q) const
        {
	return 0;
}
// ************** 在监视方向的**系统信息** *************
/*	M_EI_NA_2 初始化结束
 */
int Csd102::make_M_EI_NA_2(std::queue<struct Frame> &q) const
        {
	struct Frame f;
	struct stFrame_M_EI_NA_2 * frame = (struct stFrame_M_EI_NA_2 *) f.dat;
	const int num_iobj = 1;		//信息体个数:1个
	f.len = sizeof(struct stFrame_M_EI_NA_2);
	//head
	frame->farme_head.start_byte1 = START_LONG_FRAME;
	frame->farme_head.len1 = f.len-sizeof(struct Frame_head)
	                -sizeof(struct Frame_tail);
	frame->farme_head.len2 = frame->farme_head.len1;
	frame->farme_head.start_byte2 = START_LONG_FRAME;
	//lpdu
	frame->lpdu_head.cf_m.fcn = FCN_M_CON;		//
	frame->lpdu_head.cf_m.acd = ACD_NO_ACCESS;
	frame->lpdu_head.cf_m.dfc = DFC_NOT_FULL;  //1 bit
	frame->lpdu_head.cf_m.prm = PRM_UP;  //1bit
	frame->lpdu_head.cf_m.res = CF_RES;  //1bit
	frame->lpdu_head.link_addr = this->link_addr;
	//asdu
	frame->duid.typ = TYP_M_EI_NA_2;
	frame->duid.vsq.sq = SQ_Similar;
	frame->duid.vsq.n = num_iobj;
	frame->duid.cot.cause = COT_INIT;
	frame->duid.cot.pn = COT_PN_ACK;
	frame->duid.cot.t = COT_T_NOT_TEST;
	frame->duid.rtu_addr = makeaddr(num_iobj);
	frame->duid.rad = RAD_DEFAULT;
	//information object
	frame->obj.ioa = 0;
	frame->obj.coi.coi = COI_RETOME_RESET;
	frame->obj.coi.pc = COI_PARAMETER_UNCHANGED;
	//tail
	frame->farme_tail.cs = this->check_sum(*frame);
	frame->farme_tail.end_byte = END_BYTE;
	//print_array(f.dat, f.len);
	q.push(f);
	return 0;
}
/*	P_MP_NA_2 电能累计量数据终端设备的制造厂和产品规范
 */
int Csd102::make_P_MP_NA_2(std::queue<struct Frame> &q) const
        {
	struct Frame f;
	struct stFrame_P_MP_NA_2 * frame = (struct stFrame_P_MP_NA_2 *) f.dat;
	const int num_iobj = 1;		//信息体个数:1个
	f.len = sizeof(struct stFrame_P_MP_NA_2);
	frame->farme_head.start_byte1 = START_LONG_FRAME;
	frame->farme_head.len1 = f.len-sizeof(struct Frame_head)
	                -sizeof(struct Frame_tail);
	frame->farme_head.len2 = frame->farme_head.len1;
	frame->farme_head.start_byte2 = START_LONG_FRAME;
	//
	frame->udat_head.cf_m.fcn = FCN_M_SEND_DAT;	//
	frame->udat_head.cf_m.acd = ACD_ACCESS;
	frame->udat_head.cf_m.dfc = DFC_NOT_FULL;  //1 bit
	frame->udat_head.cf_m.prm = PRM_UP;  //1bit
	frame->udat_head.cf_m.res = CF_RES;  //1bit
	frame->udat_head.link_addr = this->link_addr;
	//
	frame->duid.typ = TYP_P_MP_NA_2;
	frame->duid.vsq.sq = SQ_Similar;
	frame->duid.vsq.n = num_iobj;
	frame->duid.cot.cause = COT_INIT;
	frame->duid.cot.pn = COT_PN_ACK;
	frame->duid.cot.t = COT_T_NOT_TEST;
	frame->duid.rtu_addr = makeaddr(num_iobj);
	frame->duid.rad = RAD_DEFAULT;
	//
	frame->obj.fcode = FACT_ID;  //
	frame->obj.pcode = PRODUCT_ID;  //
	frame->obj.dos.year = STANDARD_YEAR;  //标准
	frame->obj.dos.month = STANDARD_MONTH;
	//
	frame->farme_tail.cs = this->check_sum(*frame);
	frame->farme_tail.end_byte = END_BYTE;
	print_array(f.dat, f.len);
	q.push(f);
	return 0;

}
/* 电能累计量数据终端设备目前的系统时间 报文的制作.
 * */
/*	M_TI_TA_2 返回终端时间帧
 out:	q	返回保存到1类数据队列中
 return:	0	成功
 */
int Csd102::make_M_TI_TA_2(std::queue<struct Frame> &q1) const
        {
	struct Frame f;
	struct m_tSystime systime;
	GetSystemTime_RTC(&systime);
	struct stFrame_M_TI_TA_2 *fo = (struct stFrame_M_TI_TA_2 *) (f.dat);
	const u8 iObj_num = 1;		//信息体数量
	f.len = sizeof(struct stFrame_M_TI_TA_2);
	//整合:
	fo->farme_head.start_byte1 = START_LONG_FRAME;
	fo->farme_head.len1 = sizeof(stFrame_M_TI_TA_2)
	                -sizeof(Frame_head)-sizeof(Frame_tail);
	fo->farme_head.len2 = fo->farme_head.len1;
	fo->farme_head.start_byte2 = START_LONG_FRAME;
	//
	fo->udat_head.cf_m.fcn = FCN_M_SEND_DAT;
	fo->udat_head.cf_m.prm = PRM_UP;
	fo->udat_head.cf_m.acd = ACD_ACCESS;
	fo->udat_head.cf_m.dfc = DFC_NOT_FULL;
	fo->udat_head.link_addr = this->link_addr;
	//
	fo->duid.typ = TYP_M_TI_TA_2;
	fo->duid.vsq.sq = SQ_Similar;
	fo->duid.vsq.n = iObj_num;     //一个信息体
	fo->duid.cot.cause = COT_REQUEST;     //cot传输原因
	fo->duid.cot.pn = COT_PN_ACK;     //
	fo->duid.cot.t = COT_T_NOT_TEST;
	fo->duid.rtu_addr = makeaddr(iObj_num);
	fo->duid.rad = RAD_DEFAULT;
	//
	getsystime((fo->obj.t), systime);
	//
	fo->farme_tail.end_byte = END_BYTE;
	fo->farme_tail.cs = check_sum(*fo);
//
	printf("TI_TA 返回时间:");
	showtime(fo->obj.t);
	printf("\n");
	q1.push(f);
//	printf("RTU time test:farme_head.len1=%d sizeof frame=%d len_out=%d\n"
//	       "farme_out[0]= %x farme_out[1]= %x\n"
//	       ,pframe->farme_head.len1,sizeof(struct Farme_time),len_out,
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

/*	召唤2级数据,没有2级数据,但是有1级数据
 回复M_NV_NA_2,fc:FN_M_NO_DAT 没有所召唤的数据(但是有1级数据ACD=1)
 Reset Communication Unit
 in:
 out:	f	通用帧结构
 */
int Csd102::make_M_NV_NA_2(struct Frame &f) const
        {
	struct Short_frame * frame = (struct Short_frame *) f.dat;
	f.len = sizeof(struct Short_frame);
	frame->start_byte = START_SHORT_FRAME;
	frame->c_up.fcn = FCN_M_NO_DAT;  //4bit
	frame->c_up.dfc = DFC_NOT_FULL;  //1 bit
	frame->c_up.acd = ACD_ACCESS;		//有1级数据
	frame->c_up.prm = PRM_UP;  //1bit
	frame->c_up.res = CF_RES;  //1bit
	frame->link_addr = this->link_addr;
	frame->farme_tail.cs = this->check_sum(
	                (u8*) frame+sizeof(START_SHORT_FRAME),
	                sizeof(union Ctrl_m)+sizeof(link_addr_t));
	frame->farme_tail.end_byte = END_BYTE;
	return 0;

}

/*	回复链路状态请求(FN_C_RLK FC9)的帧
 回复: M_LKR_NA_2 FN_M_RSP FC11 以链路状态回应
 out:	f
 */
int Csd102::make_M_LKR_NA_2(struct Frame &f) const
        {
	struct Short_frame * frame = (struct Short_frame *) f.dat;
	f.len = sizeof(struct Short_frame);
	frame->start_byte = START_SHORT_FRAME;
	frame->c_up.fcn = FCN_M_RSP;
	frame->c_up.dfc = DFC_NOT_FULL;		//可以接收数据
	frame->c_up.acd = !this->qclass1.empty();		//数据访问需求位
	frame->c_up.prm = PRM_UP;		//上传
	frame->c_up.res = CF_RES;  //1bit
	frame->link_addr = /*0xffff;*/this->link_addr;
	frame->farme_tail.cs = this->check_sum(
	                (u8*) frame+sizeof(START_SHORT_FRAME),
	                sizeof(union Ctrl_m)+sizeof(link_addr_t));
	frame->farme_tail.end_byte = END_BYTE;
	return 0;
}

/*	C10.2 M_SYN_TA_2 时间同步返回帧
 */
int Csd102::make_M_SYN_TA_2(const struct Frame fi,
        std::queue<struct Frame> &q) const
        {
	struct Frame fo;
	struct stFrame_C_SYN_TA_2 * pfo = (stFrame_C_SYN_TA_2 *) fo.dat;
	struct m_tSystime systime;
	TMStruct settime;
	GetSystemTime_RTC(&systime);
	getsystime(pfo->tb, systime);
	Write_RTC_Time(&settime);
	//head
	pfo->farme_head.start_byte1 = START_LONG_FRAME;
	pfo->farme_head.len1 = sizeof(stFrame_C_SYN_TA_2)
	                -sizeof(Frame_head)-sizeof(Frame_tail);
	pfo->farme_head.len2 = pfo->farme_head.len1;
	pfo->farme_head.start_byte2 = START_LONG_FRAME;
	//
	pfo->lpdu_head.cf_m.fcn = FCN_M_SEND_DAT;
	pfo->lpdu_head.cf_m.prm = PRM_UP;
	pfo->lpdu_head.cf_m.acd = ACD_ACCESS;
	pfo->lpdu_head.cf_m.dfc = DFC_NOT_FULL;
	pfo->lpdu_head.link_addr = this->link_addr;
	//

	fo.len = sizeof(stFrame_C_SYN_TA_2);
	q.push(fi);
	return 0;

}
#pragma  GCC diagnostic warning  "-Wunused-parameter"
/* 构造一轮数据采集的前1镜像帧, 开始的镜像帧
 * in	f	输入帧(数据采集指定)
 * 	acd	输入帧是否逻辑正确,正确,则有数据,acd置1,错误这结束此轮数据采集,acd=0
 * out	f	稍作修改输出的镜像帧
 * */
template<typename T>
int Csd102::make_mirror_1(T pf, bool b_acd) const
        {
	pf->lpdu_head.cf_m.fcn = FCN_M_SEND_DAT;
	pf->lpdu_head.cf_m.prm = PRM_UP;
	pf->lpdu_head.cf_m.acd = b_acd ? ACD_ACCESS : ACD_NO_ACCESS;
	pf->lpdu_head.cf_m.dfc = DFC_NOT_FULL;
	pf->lpdu_head.cf_m.res = CF_RES;
	pf->duid.cot.cause = COT_ACTCON;
	pf->farme_tail.cs = check_sum(*pf);
	return 0;
}
/* 构造一轮数据采集的后1镜像帧, 结束的镜像帧
 * in	f	输入帧(数据采集指定)
 * out	f	稍作修改输出的镜像帧
 * */
template<typename T>
int Csd102::make_mirror_2(T pf) const
        {
	pf->lpdu_head.cf_m.fcn = FCN_M_SEND_DAT;
	pf->lpdu_head.cf_m.prm = PRM_UP;
	pf->lpdu_head.cf_m.acd = ACD_ACCESS;
	pf->lpdu_head.cf_m.dfc = DFC_NOT_FULL;
	pf->lpdu_head.cf_m.res = CF_RES;
	pf->duid.cot.cause = COT_ACTTREM;
	pf->farme_tail.cs = check_sum(*pf);
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
	u8 sum = 0;
	for (int i = 0; i<len; i++) {
		sum += a[i];
	}
	return sum;
}
/*求各种已知长度的特定用途帧的校验值(固定长度的长帧)
 * in	f	例如 stFrame_C_SP_NB_2 ,stFrame_C_TI_NA_2,stFrame_M_EI_NA_2等
 * 		这样的[确定长度的],[特定用途]的帧结构体,参见file:frame.h
 * 		必须要有帧头 farme_head 结构
 * out
 * return	u8一字节校验值
 * */
template<typename T>
u8 Csd102::check_sum(const T f) const
        {
	return check_sum((u8*) &f+sizeof(Frame_head), f.farme_head.len1);
}
/*打印字符数组*/
void Csd102::print_array(const u8 *a, const int len) const
        {
	int i;
	printf("[%d] ", len);
	for (i = 0; i<len; i++) {
		printf("%02X ", a[i]);
	}
	printf("\n");
	return;
}
//显示等待状态(可选)
void Csd102::show_wait(u32 &stat) const
        {
	printf("\r[sd102]:等待接收...");
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
/*比较开始时间和结束时间的正确性*/
int Csd102::time_range(const struct Ta starttime, const struct Ta endtime) const
        {
	u32 stime = Calc_Time_102(starttime.min, starttime.hour,
	                starttime.day, starttime.month, starttime.year);
	u32 etime = Calc_Time_102(endtime.min, endtime.hour, endtime.day,
	                endtime.month, endtime.year);
	if (stime==0) {
		printf("开始时间错误\n");
		return 1;
	}
	if (etime==0) {
		printf("结束时间错误\n");
		return 2;
	}
	if (stime>etime) {
		printf("开始晚于结束时间\n");
		return 3;
	}
	return 0;
}
/*开始时间和结束时间是否正确*/
int Csd102::time_range(const struct Obj_C_CI_XX_2 obj) const
        {
	u32 stime = Calc_Time_102(obj.Tstart.min, obj.Tstart.hour,
	                obj.Tstart.day, obj.Tstart.month, obj.Tstart.year);
	u32 etime = Calc_Time_102(obj.Tend.min, obj.Tend.hour, obj.Tend.day,
	                obj.Tend.month, obj.Tend.year);
	struct m_tSystime systime;
	GetSystemTime_RTC(&systime);
	u32 now = Calc_Time_102(systime.min, systime.hour, systime.day,
	                systime.mon, systime.year);
	if (stime==0) {
		printf("开始时间错误\n");
		return 1;
	}
	if (etime==0) {
		printf("结束时间错误\n");
		return 2;
	}
	if (stime>=etime) {
		printf("开始晚于结束时间\n");
		return 3;
	}
	if (now==0) {
		printf("当前时间计算错误\n");
		return 4;
	}
	if (etime>now) {
		printf("结束时间晚于当前时间\n");
		return 5;
	}
	return 0;
}
/*信息体地址发范围是否正确*/
int Csd102::ioa_range(const struct Obj_C_CI_XX_2 obj) const
        {
	if (obj.start_ioa<1u) {
		printf("开始信息体地址小于1\n");
		return 1;  //信息体地址从1开始编制的
	}
	if (obj.start_ioa>=obj.end_ioa) {
		printf("开始信息体地址大于结束信息体地址\n");
		return 2;
	}
	return 0;
}
/*输入帧 逻辑上是否正确*/
int Csd102::format_ok(struct stFrame_C_CI_NR_2 fin) const
        {
	if (this->time_range(fin.obj)!=0) {
		printf("时间起止范围错误\n");
		return 1;
	}
	if (this->ioa_range(fin.obj)!=0) {
		printf("信息体地址范围错误\n");
		return 2;
	}
	return 0;
}

int Csd102::print_err_msg(int msg) const
        {
	msg++;
	return 0;
}
/*7.2.4 电能累计量数据终端设备地址,
 * "信息体每超过一次255个信息点的情况".
 */
inline rtu_addr_t Csd102::makeaddr(int obj_num) const
        {
	if (((obj_num/255)+1)!=1) {  //多于255个信息体
		//PRINT_HERE
	}
	return (obj_num/255)+1;
}
inline void Csd102::showtime(const struct tm t) const
        {
	printf("tm %4d-%02d-%02d %02d:%02d:%02d timezone:%s\n",
			1900+t.tm_year, t.tm_mon+1, t.tm_mday,
	                t.tm_hour, t.tm_min,t.tm_sec,t.tm_zone);
	fflush(stdout);
	return;
}
inline void Csd102::showtime(const struct Ta t) const
        {
	printf("Ta %4d-%02d-%02d %02d:%02d \n",
	                2000+t.year, t.month, t.day, t.hour, t.min);
	fflush(stdout);
	return;
}
inline void Csd102::showtime(const struct Tb t) const
        {
	printf("Tb %4d-%02d-%02d %02d:%02d:%02d %4dms ",
	                2000+t.year, t.month, t.day,
	                t.hour, t.min, t.second, t.ms);
	fflush(stdout);
	return;
}
/* 通过比较 上次接收的帧和本次接收的帧 确定是否需要重新发送上次的发送帧
 * in	:	rf_bak	receive frame backup 备份的上次接收帧
 * 		rf	receive frame 本次的接收帧
 * return	true	需要重发
 * 		false	不需要重发
 * */
bool Csd102::need_resend(const struct Frame rf_bak, const struct Frame rf)
        const
        {
	union Ctrl_c cbak;
	union Ctrl_c c;
	cbak.val = this->get_ctrl_field(rf_bak);
	c.val = this->get_ctrl_field(rf);
	printf("c.fcb=%d c_bak.fcb=%d || c.fcv=%d c_bak.fcv=%d\n", c.fcb,
	                cbak.fcb, c.fcv, cbak.fcv);
	//前后两次的fcv都必须有效
	return (cbak.fcv==1)&&(c.fcv==1)&&(c.fcb==cbak.fcb);
}

/* 将Ta时间格式换算成 从1900年1月1日0时0分到目前为止的(分钟/秒)数.
 * return	0 错误
 * 		到目前为止的分钟数
 */
u32 Csd102::get_min(Ta ta) const
        {
	unsigned int mins, hours, days;
	unsigned char months, years;
	unsigned char circle = 0, leap_flag = 0;
	mins = ta.min;
	hours = ta.hour;
	days = ta.day-1;
	months = ta.month;
	u8 YEARL = ta.year+30;//base 2000-> base1970
	years = YEARL%100;
	if (mins>59)
		return 0;
	if (hours>23)
		return 0;
	if (days>30)
		return 0;
	if (months>12)
		return 0;
	if (years>99)
		return 0;     //year must be >=2000 && <=2050
	circle = years/4;
	days = circle*(366+365*3)+days;
	if ((years%4)==1)
		days = 365+days;
	else if ((years%4)==2) {
		days = days+365*2;
		leap_flag = 1;
	} else if ((years%4)==3)
		days = days+366+365*2;
	switch (months) {
	case 1:
		//days=days;
		break;
	case 2:
		days = days+31;
		break;
	case 3:
		days = days+(31+28);
		break;
	case 4:
		days = days+(31*2+28);
		break;
	case 5:
		days = days+(31*2+28+30);
		break;
	case 6:
		days = days+(31*3+28+30);
		break;
	case 7:
		days = days+(31*3+28+30*2);
		break;
	case 8:
		days = days+(31*4+28+30*2);
		break;
	case 9:
		days = days+(31*5+28+30*2);
		break;
	case 10:
		days = days+(31*5+28+30*3);
		break;
	case 11:
		days = days+(31*6+28+30*3);
		break;
	case 12:
		days = days+(31*6+28+30*4);
		break;
	default:
		break;
	}
	if ((months>2)&&(leap_flag==1))
		days = days+1;
	hours = 24*days+hours;
	mins = 60*hours+mins;
	return (mins);
}
