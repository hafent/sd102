/*  filename:	sd102.h
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
#pragma pack(1)
//
extern "C" CProtocol *
CreateCProto_sd102()
{
	//PRINT_HERE
	printf(PREFIX"create so lib\n");
	return new Csd102;
}

Csd102::Csd102()
{
	printf(PREFIX"struct Csd102\n");
	syn_char_num = 6;
//Syn_Head_Rece_Flag=0;
	m_ACD = CF_ACD_NO_DAT;
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
	this->acd = CF_ACD_NO_DAT;
	this->last_typ = M_UNUSED;
	this->link_addr = 0;
	this->exist_backup_frame = false;
	this->has_class1_dat = false;
	this->has_class2_dat = false;
	this->status = 0;
	while (!qclass1.empty()) {
		qclass1.pop();
	}
	while (!qclass2.empty()) {
		qclass2.pop();
	}

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

	last_typ = M_UNUSED;
	has_class1_dat = false;
	has_class2_dat = false;
	c_Link_Address_H = tmp_portcfg->m_ertuaddr >> 8;
	c_Link_Address_L = (unsigned char) tmp_portcfg->m_ertuaddr;

	m_checktime_valifalg = tmp_portcfg->m_checktime_valiflag;
	m_suppwd = tmp_portcfg->m_usrsuppwd;
	m_pwd1 = tmp_portcfg->m_usrpwd1;
	m_pwd2 = tmp_portcfg->m_usrpwd2;
	m_retransmit_table = tmp_portcfg->m_retransmit_mtr;		//转发表
	retran_table_valid = (m_retransmit_table.empty()) ? 0 : 1;
	m_Max_Mtrnum = (!retran_table_valid) ?
			(sysConfig->meter_num) : (m_retransmit_table.size());
	//本类的
	this->exist_backup_frame = false;
	this->link_addr = tmp_portcfg->m_ertuaddr;
	return 0;
}

//终端(从站)主动发送.在山东102,非平衡传输中(协议规定)不需要
void Csd102::SendProc(void)
{
	return;
}
//显示等待状态(可选)
void Csd102::show_wait(u32 &stat)const
{
	printf("\rsd102:Wait for farme.");
	switch (stat % 4) {
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

	int ret = -1;
	union Ctrl_down c2;
	union Ctrl_down c;
	//1. get frame [会修改类成员]
	ret = this->separate_msg(&reci_frame);
	if (ret != 0) {
		show_wait(status);
		return 0;
	}
	//2. verify frame
	ret = this->verify_frame(reci_frame);
	if (ret != 0) {
		printf("verify err,ignord\n");
		return 0;
	}
	//正式成为接收的报文
	printf(PREFIX"Rece[%3d]:", reci_frame.len);
	print_array(reci_frame.dat, reci_frame.len);

	//重发否?
	c2.val = this->get_ctrl_field(reci_frame_bak);
	c.val = this->get_ctrl_field(reci_frame);
	//TODO 写成 函数 is_resend
	bool chkresend=false ;//测试时不验证重发.
	if (chkresend && (c.fcv == 1) && (c.fcb == c2.fcb)) {     //链路重发
		//重发本文的发送帧,重发
		printf("Resend Farme\n");
		//前后FCB应该不一样
		printf("c.fcb=%d c_bak.fcb=%d || c.fcv=%d c_bak.fcv=%d\n",
				c.fcb, c2.fcb, c.fcv, c2.fcv);
		transfer(send_frame_bak);
		return 0;
	}
	//分类
	switch (c.funcode) {
	//S2: **** 2类服务(写指令)
	case FN_C_RCU:
		ret=0;//TODO 终端施行复位操作
		if(ret==0){
			fun_M_CON_NA_2(&send_frame);
		}
		//goto SEND;
		break;
	case FN_C_TRANS_DAT:
		//应答之后即刻 开始处理事务
		//TODO 应答之前是否要处理什么??
		ack(send_frame);
		switch (reci_frame.dat[0]) {
		case START_SHORT_FRAME:
			process_short_frame(reci_frame, &send_frame);
			printf(PREFIX"####FN_C_TRANS_DAT,START_SHORT_FRAME\n");
			PRINT_HERE
			break;
		case START_LONG_FRAME:
			ret=process_long_frame(reci_frame, this->qclass1);
			if(ret!=0){
				;
			}
			break;
		default:
			PRINT_HERE
			break;
		}
		PRINT_HERE
		break;
		//S3: **** 3类服务(读数据)
	case FN_C_RLK:		// 链路请求 FCV=0
		if (c.fcv != 0) {
			printf("c.fcv!=0\n");
			PRINT_HERE
		}
		fun_M_LKR_NA_2(&send_frame);		//回复链路状态
		PRINT_HERE
		break;
	case FN_C_PL1:		//召唤1级数据
		if (this->qclass1.empty()) { //没有数据,否定应答
			printf("no class1 dat qsize=%d\n", qclass1.size());
			nack(send_frame);
		} else {
			//有数据,发送数据
			send_frame = this->qclass1.front(); //出队列的也可能是之前的镜像帧
			this->qclass1.pop();
			printf("send class1 dat [%d]:%X %X %X %X\n",
					send_frame.len, send_frame.dat[0],
					send_frame.dat[1], send_frame.dat[2],
					send_frame.dat[3]);
			printf("queue size is %d\n", qclass1.size());
		}
		break;
	case FN_C_PL2:
		if (!this->qclass2.empty()) {
			printf("send qclass2_dat\n");
			//TODO 发送2级数据
			PRINT_HERE
		} else {
			if (!this->qclass1.empty()) {
				printf("no qclass2 dat ,has qdat1 \n");
				fun_M_NV_NA_2(&send_frame);
				//print_array(send_frame.dat, send_frame.len);
			} else {
				printf("no class 1,2 dat \n");
				nack(send_frame);
			}
		}
		break;
	case FN_C_RES1:
		PRINT_HERE
		break;
	case FN_C_RES2:
		PRINT_HERE
		break;
	default:
		printf("c.funcode=%d\n", c.funcode);
		PRINT_HERE
		break;
	}
	//SEND:	//发送帧
	transfer(send_frame);
	//对于S3类 保存接收帧用于检测数据丢失,保存发送帧用于在数据丢失时重发
	if (c.funcode==FN_C_RLK || c.funcode==FN_C_PL1 ||c.funcode== FN_C_PL2) {	//单字符帧不需要重发
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
	f.dat[0] = SINGLE_CHARACTER;
	f.len = sizeof(SINGLE_CHARACTER);
	return 0;
}
/*否定应答	形成否定帧(nack)
 */
int Csd102::nack(struct Frame &f) const
{
	f.dat[0] = SINGLE_CHARACTER;
	f.len = sizeof(SINGLE_CHARACTER);
	return 0;
}

/*发送帧 将 frame 通过复制到 m_transBuf 结构体发送.
 in:	frame	字节流
 farme_len	长度
 out:	m_transBuf	修改的发送缓冲区结构体,发送数据[类成员,隐含使用!!]
 */
int Csd102::transfer(const struct Frame f)
{
	printf(PREFIX"Send[%3d]:", f.len);
	print_array(f.dat, f.len);
	//printf("send: frame[0]=%x farme_len=%d \n",frame[0],farme_len);
	memcpy(this->m_transBuf.m_transceiveBuf, f.dat, f.len);
	this->m_transBuf.m_transCount = f.len;
	return 0;
}

/* 从帧中获取控制域,一个字节
 in:	frame	帧/字节流
 farme_len	字节流长度
 out:	none
 return:	控制字节(union Ctrl_down)
 */
u8 Csd102::get_ctrl_field(const struct Frame f)const
{

	union Ctrl_down c;
	if (f.len < (int) sizeof(struct Short_frame)) {
		//PRINT_HERE;
		//printf("frame length(%d) is too small\n",farme_len);
		return c.val;
	}
	switch (f.dat[0]) {
	case START_SHORT_FRAME:
		c.val = f.dat[0 + sizeof(START_SHORT_FRAME)];	//开始字节后面既是控制域
		break;
	case START_LONG_FRAME:
		c.val = f.dat[0 + sizeof(struct Frame_head)];	//帧头后面既是控制域
		break;
	default:
		if (f.dat[0] == 0 && f.dat[1] == 0 && f.dat[2] == 0) {
			printf("back-up frame is empty \n");
		} else {
			PRINT_HERE
		}
		break;
	}
	return c.val;
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
int Csd102::separate_msg(struct Frame *f)
{
	int frame_len = 0;
	bool syn_head_ok = false;
//Syn_Head_Rece_Flag = 0;
	f->len = get_num(&m_recvBuf);		//缓冲区长度
#if 0
			printf("*get_num=%d\n",len);
#endif
	if (!f->len) {
		return 1;		//长度不够/缓冲区没有数据,分割错误
	}
//1.查找
	while (f->len >= syn_char_num) {		//队列长度太短就不必再查找了
		copyfrom_buf(f->dat, &m_recvBuf, f->len);
#if 0
		printf("farme:");
		print_array(readbuf,len);
#endif
		if (!sync_head(f->dat, frame_len)) {		//查找到一帧
			syn_head_ok = true;
			break;
		} else {		//否则 1元素出队列 //防止垃圾数据出现在正常帧前面
			pop_char(&m_recvBuf);
			f->len = get_num(&m_recvBuf);
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
	f->len = get_num(&m_recvBuf);
	if (f->len) {		//截断到 帧尾,剩下的留在缓冲区
		f->len = get_buff(f->dat, &m_recvBuf, frame_len);
		if (f->len != frame_len) {
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
	if (buffer[0] == START_SHORT_FRAME) {
		int end_index = sizeof(struct Short_frame) - 1;
		if (buffer[end_index] == END_BYTE) {
			farme_len = sizeof(struct Short_frame);
			return 0;
		} else {
			return -1;
		}
	}
//变长帧
	if ((buffer[0] == START_LONG_FRAME) && (buffer[3] == START_LONG_FRAME)
			&& buffer[1] == buffer[2]) {
		int len = buffer[1];
		//最末尾的元素的index
		int end_index = 0 + sizeof(struct Frame_head)     //帧头
				+ len				//链路数据单元(LPDU)长度
				+ sizeof(struct Frame_tail)     //帧尾
		- 1;
		//printf("end_index=%d\n",end_index);
		if (buffer[end_index] == END_BYTE) {
			//printf("#end_index=%d\n",end_index);
			farme_len = sizeof(struct Frame_head) + len
					+ sizeof(struct Frame_tail);
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
int Csd102::verify_frame(const struct Frame f) const
{
	struct Short_frame* farme = NULL;     //用于固定帧
	struct Udat_head* lpdu_head = NULL;     //用于变长帧
	struct Frame_tail* farme_tail = NULL;     //帧尾,用于变长帧
	union Ctrl_down c;
	u16 farme_link_addr = 0;     //链路地址
	u8 farme_cs = 0;     //本帧的cs
	u8 cs = 0;     //计算得到的cs
	const u8 C_PRM_DOWN = 1;     //下行方向标志(1-下行;0-上行)
	switch (f.dat[0]) {     //分类讨论
	case START_SHORT_FRAME:
		farme = (struct Short_frame *) (f.dat);
		cs = check_sum(f.dat + sizeof(START_SHORT_FRAME),
				sizeof(union Ctrl_down) + sizeof(link_addr_t));
		//统一变量,由下面处理
		farme_link_addr = farme->link_addr;
		c.prm = farme->c_down.prm;
		farme_cs = farme->farme_tail.cs;
		break;
	case START_LONG_FRAME:
		lpdu_head = (struct Udat_head *) (f.dat
				+ sizeof(struct Frame_head));
		//分解帧尾
		farme_tail = (struct Frame_tail*) (f.dat + f.len
				- sizeof(struct Frame_tail));
		//校验:去掉头,(嘎嘣脆),去帧掉尾.校验中间部分
		cs = this->check_sum(f.dat + sizeof(struct Frame_head),
				f.len - sizeof(struct Frame_head)
						- sizeof(struct Frame_tail));
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
	if (farme_cs != cs) {
		PRINT_HERE
		printf("CS err farme_cs=0x%02X but "
				"Calculate cs=0x%02X ,Ignore.\n", farme_cs, cs);
		return 0x11;
	}
	//2.判断是否下行帧,終端只接收下行的帧,不是下行的一定时错误的。
	if (c.prm != C_PRM_DOWN) {
		PRINT_HERE
		printf("c.prm[%d] !=C_PRM_DOWN[%d] ,Ignore.\n", c.prm,
				C_PRM_DOWN);
		return 0x12;
	}
	//3.判断是否传递给本终端
	if (farme_link_addr != this->link_addr) {
		PRINT_HERE
		printf("link_addr=%d farme.link_addr=%d \n", link_addr,
				farme->link_addr);
		return 0x13;
	}
	return 0;     //4. 结束
}

/*解析 FT1.2 固定帧长帧
 in:	farme_in	输入帧
 len_in	输入帧长度
 out:	farme_out	输出帧
 len_out	输出帧长度
 return:	0	正确处理
 非0	处理失败
 */
int Csd102::process_short_frame(const struct Frame fin,
		struct Frame* f_out) const
{
	struct Short_frame *farme = (struct Short_frame *) &fin;
	struct Short_frame *frame_up = (struct Short_frame *) f_out;
	u8 cs = 0;
	if (fin.len < 6) {
		PRINT_HERE
		printf("length(%d) too small\n", fin.len);
	}
//分类解析
	switch (farme->c_down.funcode) {
	case FN_C_RCU:     //复位
//PRINT_HERE;
		frame_up->c_up.funcode = FN_M_CON;
		frame_up->c_up.acd = 0;
		frame_up->c_up.dfc = 0;
		frame_up->link_addr = link_addr;
		cs = check_sum((u8*) frame_up + sizeof(START_SHORT_FRAME),
				sizeof(union Ctrl_down) + sizeof(link_addr_t));
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
		cs = check_sum((u8*) frame_up + sizeof(START_SHORT_FRAME),
				sizeof(union Ctrl_down) + sizeof(link_addr_t));
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
/*解析 FT1.2 变帧长帧
 接收变长帧,按 type(TYP)分类处理下行报文(主站发出的)
 */
int Csd102::process_long_frame(const struct Frame fin,
		std::queue<struct Frame> &q)
{
	int ret;
	/*从输入帧中分解出一些数据单元
	 :Lpdu_head + Asdu_head + (信息体未解析) + Farme_tail*/
//	struct Udat_head *udat_head = (struct Udat_head *) (fin.dat
//			+ sizeof(struct Frame_head));
	struct Duid *asdu_head = (struct Duid *) (fin.dat
			+ sizeof(struct Frame_head) + sizeof(struct Udat_head));
//	struct Frame_tail *frame_tail = (struct Frame_tail *) (fin.dat + fin.len
//			- sizeof(struct Frame_tail));
#if 0
	printf("addr=%d , cf=%02X \n",
			udat_head->link_addr ,udat_head->c_down.val);
	printf("asdu_head->typ=%d , asdu_head->rad=%02X \n",
			duid->typ ,duid->rad);
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
		ret = fun_M_TI_TA_2(q);
		printf("c1size? %d\n", q.size());
		if (ret != 0) {
			PRINT_HERE
		}
		return 0;
		break;
	case C_CI_NR_2:     //读取电量
		//保存镜像帧  输入帧,至1类数据队列
		q.push(fin);
		// 读取数据至1类数据队列
		ret = fun_M_IT_TA_2(fin, q);
		if (ret != 0) {
			PRINT_HERE
		}
		return 0;
		break;
	case C_CI_NS_2:
		//保存镜像帧  输入帧,至1类数据队列
		q.push(fin);
		PRINT_HERE
		break;
	case C_SYN_TA_2:		//设置时间
		ret = fun_M_SYN_TA_2(fin, q);
		if (ret != 0) {
			PRINT_HERE
		}
		break;
	case C_CI_NA_B_2:
		//保存镜像帧  输入帧,至1类数据队列
		q.push(fin);
		PRINT_HERE
		break;
	case C_YC_TA_2:
		//保存镜像帧  输入帧,至1类数据队列
		q.push(fin);
		PRINT_HERE
		break;
	case C_CI_NA_C_2:
		//保存镜像帧  输入帧,至1类数据队列
		q.push(fin);
		PRINT_HERE
		break;
	case C_XL_NB_2:
		//保存镜像帧  输入帧,至1类数据队列
		q.push(fin);
		PRINT_HERE
		break;
	case C_CI_NA_D_2:
		//保存镜像帧  输入帧,至1类数据队列
		q.push(fin);
		PRINT_HERE
		break;
	default:
		PRINT_HERE
		break;
	}
//
	return 0;
}

int Csd102::clear_fcv(void)
{
	struct Short_frame frame;
	//memcpy(&frame, this->reci_frame_bak, sizeof(frame));
	frame.c_down.fcv = 0;
	return 0;
}
/* 从 系统时间(systime)中将时间复制到Tb时间结构中.
 * */
int Csd102::getsystime(const struct m_tSystime systime, struct Tb * t) const
{
	t->ms = systime.msec;
	//pframe->tb.eti=0;//费率
	t->second = systime.sec;
	t->min = systime.min;
	t->hour = systime.hour;
	t->day = systime.day;
	t->month = systime.mon;
	t->year = systime.year;
	t->week = systime.week;
	t->su = TB_STANDARD_TIME;     //非夏令时间(标准时间)
	t->iv = TB_VALID;     //有效
	t->res1 = TB_RESERVE1;     //备用置零
	t->res2 = TB_RESERVE2;
	return 0;
}
/* 从 系统时间(systime)中将时间复制到Ta时间结构中.
 * */
int Csd102::getsystime(const struct m_tSystime systime, struct Ta * t) const
{
	t->min = systime.min;
	t->hour = systime.hour;
	t->day = systime.day;
	t->month = systime.mon;
	t->year = systime.year;
	t->week = systime.week;
	t->su = TB_STANDARD_TIME;     //非夏令时间(标准时间)
	t->iv = TB_VALID;     //有效
	t->res1 = TB_RESERVE1;     //备用置零
	t->res2 = TB_RESERVE2;
	return 0;
}
#pragma  GCC diagnostic ignored  "-Wunused-parameter"
// ************** 在监视方向的**过程信息** *************
/*	M_SP_TA_2
 in:	frame_in
 len_in
 out:	frame_out	返回/发送到主站的帧
 len_out		帧长
 return:	0	成功
 */
int Csd102::fun_M_SP_TA_2(const u8 *frame_in, const int len_in, u8 *frame_out,
		int &len_out) const
{
	struct m_tSystime systime;
	GetSystemTime_RTC(&systime);
	int i = 0;
	int obj_num = 20;
	struct Duid* duid_in = (struct Duid*) frame_in
			+ sizeof(struct Frame_head) + sizeof(struct Udat_head);
	//FIXME : 按照地址区分是单点信息,还是一个时段的单点信息
	printf("sp addr: %d \n", duid_in->rad);
#if 0
	if (duid_in->rad == RAD_ALL_SP_INFO) {
#endif
	//全部单点信息
	printf("All single point infomation\n");
	struct stFrame_C_SP_NA_2* sp = (struct stFrame_C_SP_NA_2*) frame_in;

#if 0
} else {
	//指定时间范围的单点信息
	printf("Single point infomation with time and address\n");
	struct stFrame_C_SP_NB_2* sp =
	(struct stFrame_C_SP_NB_2*) frame_in;

}
#endif
	//单点信息的地址
	printf("single point RAD=%d\n", sp->duid.rad);
	//struct stFrame_M_SP_TA_2{
	struct Frame_head* f_head = (struct Frame_head*) frame_out;
	struct Udat_head* udat_head = (struct Udat_head*) frame_out
			+ sizeof(struct Frame_head);
	struct Duid* duid = (struct Duid*) udat_head + sizeof(struct Duid);
	//struct M_SP_TA_2_iObj** obj = (struct M_SP_TA_2_iObj **) duid;
	struct M_SP_TA_2_iObj obj[100];
	struct Ta *ta = (struct Ta *) duid
			+ obj_num * sizeof(struct M_SP_TA_2_iObj);
	struct Frame_tail* f_tail = (struct Frame_tail*) ta + sizeof(struct Ta);
	//};
	//struct Frame_head
	f_head->start_byte1 = START_LONG_FRAME;
	f_head->len1 = sizeof(struct Udat_head) + sizeof(struct Duid)
			+ sizeof(struct M_IT_TX_2_iObj) * obj_num
			+ sizeof(struct Ta);
	f_head->len2 = f_head->len1;
	f_head->start_byte2 = START_LONG_FRAME;
	//struct Lpdu_head
	udat_head->c_up.funcode = FN_M_SEND_DAT;
	udat_head->c_up.prm = CF_PRM_UP;
	udat_head->c_up.acd = this->acd;
	udat_head->c_up.dfc = CF_DFC_NOT_FULL;
	//struct Asdu_head
	duid->typ = M_SP_TA_2;
	duid->vaq.sq = VSQ_SQ_Similar;
	duid->vaq.n = obj_num;
	duid->cot.cause = COT_REQUEST;
	duid->cot.pn = COT_PN_ACK;
	duid->cot.t = COT_T_NOT_TEST;
	duid->rtu_addr = (obj_num / 256) + 1;	//FIXME
	duid->rad = 0;

	//FIXME 完成信息体
	for (i = 0; i < obj_num; i++) {
		obj[i].sp.spa = 1;
		obj[i].sp.spi = 1;
		obj[i].sp.spq = 3;
		getsystime(systime, &(obj[i].tb));
	}
	//struct Frame_tail
	f_tail->cs = this->check_sum(frame_out + sizeof(struct Frame_head),
			f_head->len1);
	f_tail->end_byte = END_BYTE;
	//frame_out
//调试返回帧
	frame_out[0] = 1;
	frame_out[1] = 2;
	frame_out[2] = 3;
	frame_out[3] = 4;
	frame_out[4] = 5;
	len_out = 5;
	return 0;
}

/*	M_IT_TA_2 带时标(T)的电量(IT)
 in:	frame_in
 len_in
 out:	frame_out	返回/发送到主站的帧
 len_out		帧长
 return:	0	成功
 */
int Csd102::fun_M_IT_TA_2(const struct Frame fi,
		std::queue<struct Frame> &q) const
{
	struct Frame frame_out;
	int i = 0;
	int fno;	//一帧发不完的时候,帧序号
	//TODO 数据过多时分帧发送
	//每帧最多可以包含的信息体个数,用户数据最多255,再根据单个信息体的大小确定
	//不同类型的帧,因为包含的信息体大小不同,可能造成最多可包含数信息体数量不同
	int maxperframe = 0;
	struct m_tSystime systime;
	struct stFrame_C_CI_NR_2 *fin = (struct stFrame_C_CI_NR_2 *) fi.dat;
	printf("fin->obj.start_ioa=%d\n", fin->obj.start_ioa);
	printf("fin->obj.end_io=%d\n", fin->obj.end_ioa);
	printf("time: %d:%d - %d:%d \n", fin->obj.Tstart.hour,
			fin->obj.Tstart.min, fin->obj.Tend.hour,
			fin->obj.Tend.min);
	const int obj_num = (fin->obj.end_ioa - fin->obj.start_ioa + 1);
	GetSystemTime_RTC(&systime);
	maxperframe = (255 - sizeof(struct Udat_head) - sizeof(struct Duid)
			- sizeof(struct Ta)) / sizeof(struct M_IT_TX_2_iObj);
	printf("obj_num=%d max_iObj_num=%d\n", obj_num,maxperframe);
	int frame_num=obj_num / maxperframe;//数据应该分解成多少帧[1..frame_num]
	for (fno = 0; fno < frame_num + 1; fno++) {// [0,frame_num]
		int n;	//本帧包含的信息体数量
		bool lastframe=(fno==frame_num);//是最后一帧吗
		if (lastframe) { //是最后一帧,个数为剩下的
			n = obj_num % maxperframe;
		} else { //不是最后一帧,数量为最大可以包含的数量
			n = maxperframe;
		}
		printf("n=%d\n",n);
		frame_out.len = sizeof(struct Frame_head) //
		+ sizeof(struct Udat_head) //
				+ sizeof(struct Duid) //
				+ sizeof(struct M_IT_TX_2_iObj) * n //
				+ sizeof(struct Ta) //
				+ sizeof(struct Frame_tail); //
		//struct stFrame_M_IT_TA_2{
		struct Frame_head* f_head =
				(struct Frame_head*) ((u8*) frame_out.dat);
		struct Udat_head* udat_head = (struct Udat_head*) ((u8*) f_head
				+ sizeof(struct Frame_head));
		struct Duid* duid = (struct Duid*) ((u8*) udat_head
				+ sizeof(struct Udat_head));
		struct M_IT_TX_2_iObj* obj =
				(struct M_IT_TX_2_iObj *) ((u8*) duid
						+ sizeof(struct Duid));
		struct Ta* ta = (struct Ta *) (frame_out.dat + frame_out.len
				- sizeof(struct Frame_tail) - sizeof(struct Ta));
		struct Frame_tail* f_tail = (struct Frame_tail*) (frame_out.dat
				+ frame_out.len - sizeof(struct Frame_tail));
		//};
		//struct Frame_head
		f_head->start_byte1 = START_LONG_FRAME;
		f_head->len1 = sizeof(struct Udat_head) + sizeof(struct Duid)
				+ sizeof(struct M_IT_TX_2_iObj) * n
				+ sizeof(struct Ta);
		f_head->len2 = f_head->len1;
		f_head->start_byte2 = START_LONG_FRAME;
		//struct Udat_head
		udat_head->c_up.funcode = FN_M_SEND_DAT;
		udat_head->c_up.prm = CF_PRM_UP;
		//是最后一帧吗?
		udat_head->c_up.acd = (lastframe) ? 0 : 1;
		udat_head->c_up.dfc = CF_DFC_NOT_FULL;
		udat_head->link_addr = this->link_addr;
		//struct Duid
		duid->vaq.n = n;
		duid->typ = M_IT_TA_2;
		duid->cot.cause =(lastframe) ?
				COT_ACT_TREMINATE : COT_REQUEST;
		duid->cot.pn = COT_PN_ACK;
		duid->cot.t = COT_T_NOT_TEST;
		duid->rad = 0;
		duid->rtu_addr = (obj_num / 256 + 1);
		//M_IT_TX_2_iObj
		for (i = 0; i < n; i++) {
			obj[i].ioa = fin->obj.start_ioa + fno * maxperframe + i;
			obj[i].it_power.dat = 0xFFFFFFFF;
			obj[i].it_power.d_status.val = 0xAA;	//TODO 其他事件待定
			//TODO 校验的源待定.
			obj[i].cs = check_sum((u8*) &obj[i], sizeof(struct It));
		}
		getsystime(systime, ta);
		//struct Frame_tail
		f_tail->cs = this->check_sum(
				frame_out.dat + sizeof(struct Frame_head),
				f_head->len1);
		f_tail->end_byte = END_BYTE;
//		printf("duid->c_up.funcode:%d", udat_head->c_up.funcode);
//		printf("f_tail->end_byte:%X", f_tail->end_byte);
		//print_array(frame_out.dat, frame_out.len);
		q.push(frame_out);
		printf("push to queue,qsize=%d\n", q.size());
	}
//
	return 0;
}
/*	M_IT_TD_2 周期复位记账(计费)电能累计量,每个量为四个八位位组
 * */
int Csd102::fun_M_IT_TD_2(const struct Frame fi,
		std::queue<struct Frame> &q) const
{     //M_IT_TD_2
	return 0;
}
/*	M_IT_TA_B_2 复费率记帐(计费)电能累计量
 * */
int Csd102::fun_M_IT_TA_B_2(const struct Frame fi,
		std::queue<struct Frame> &q) const
{     //M_IT_TA_B_2
	return 0;
}
/*	M_YC_TA_2 遥测
 * */
int Csd102::fun_M_YC_TA_2(const struct Frame fi,
		std::queue<struct Frame> &q) const
{
	return 0;
}
/*	M_XL_TA_2 需量
 * */
int Csd102::fun_M_XL_TA_2(const struct Frame fi,
		std::queue<struct Frame> &q) const
{
	return 0;
}
/*	M_IT_TA_C_2 月结算复费率电能累计量
 * */
int Csd102::fun_M_IT_TA_C_2(const struct Frame fi,
		std::queue<struct Frame> &q) const
{
	return 0;
}
/*	M_IT_TA_D_2 表计谐波数据
 * */
int Csd102::fun_M_IT_TA_D_2(const struct Frame fi,
		std::queue<struct Frame> &q) const
{
	return 0;
}
// ************** 在监视方向的**系统信息** *************
/*	M_EI_NA_2 初始化结束
 */
int Csd102::fun_M_EI_NA_2(std::queue<struct Frame> &q) const
{
	struct Frame f;
	struct stFrame_M_EI_NA_2 * frame = (struct stFrame_M_EI_NA_2 *) f.dat;
	const int num_iobj = 1;		//信息体个数:1个
	f.len = sizeof(struct stFrame_M_EI_NA_2);
	//head
	frame->farme_head.start_byte1 = START_LONG_FRAME;
	frame->farme_head.len1 = f.len - sizeof(struct Frame_head)
			- sizeof(struct Frame_tail);
	frame->farme_head.len2 = frame->farme_head.len1;
	frame->farme_head.start_byte2 = START_LONG_FRAME;
	//lpdu
	frame->lpdu_head.c_up.funcode = FN_M_SEND_DAT;		//FIXME: 待确认
	frame->lpdu_head.c_up.acd = 1;
	frame->lpdu_head.c_up.dfc = CF_DFC_NOT_FULL; //1 bit
	frame->lpdu_head.c_up.prm = CF_PRM_UP;  //1bit
	frame->lpdu_head.c_up.res = CF_RES;  //1bit
	frame->lpdu_head.link_addr = this->link_addr;
	//asdu
	frame->duid.typ = M_EI_NA_2;
	frame->duid.vaq.sq = VSQ_SQ_Similar;
	frame->duid.vaq.n = num_iobj;
	frame->duid.cot.cause = COT_INIT;
	frame->duid.cot.pn = COT_PN_ACK;
	frame->duid.cot.t = COT_T_NOT_TEST;
	frame->duid.rtu_addr = (num_iobj / 256) + 1;
	frame->duid.rad = 0;
	//information object
	frame->obj.ioa = 0;
	frame->obj.coi.coi = COI_RETOME_RESET;
	frame->obj.coi.pc = COI_PARAMETER_UNCHANGED;
	//tail
	frame->farme_tail.cs = this->check_sum(
			(u8*) frame + sizeof(struct Frame_head),
			frame->farme_head.len1);
	frame->farme_tail.end_byte = END_BYTE;
	print_array(f.dat, f.len);
	q.push(f);
	return 0;
}
/*	P_MP_NA_2 电能累计量数据终端设备的制造厂和产品规范
 */
int Csd102::fun_P_MP_NA_2(std::queue<struct Frame> &q) const
{
	struct Frame f;
	struct stFrame_P_MP_NA_2 * frame = (struct stFrame_P_MP_NA_2 *) f.dat;
	const int num_iobj = 1;		//信息体个数:1个
	f.len = sizeof(struct stFrame_P_MP_NA_2);
	//head
	frame->farme_head.start_byte1 = START_LONG_FRAME;
	frame->farme_head.len1 = f.len - sizeof(struct Frame_head)
			- sizeof(struct Frame_tail);
	frame->farme_head.len2 = frame->farme_head.len1;
	frame->farme_head.start_byte2 = START_LONG_FRAME;
	//lpdu
	frame->udat_head.c_up.funcode = FN_M_SEND_DAT;		//FIXME: 待确认
	frame->udat_head.c_up.acd = 1;
	frame->udat_head.c_up.dfc = CF_DFC_NOT_FULL; //1 bit
	frame->udat_head.c_up.prm = CF_PRM_UP;  //1bit
	frame->udat_head.c_up.res = CF_RES;  //1bit
	frame->udat_head.link_addr = this->link_addr;
	//asdu
	frame->duid.typ = P_MP_NA_2;
	frame->duid.vaq.sq = VSQ_SQ_Similar;
	frame->duid.vaq.n = num_iobj;
	frame->duid.cot.cause = COT_INIT;  //FIXME
	frame->duid.cot.pn = COT_PN_ACK;
	frame->duid.cot.t = COT_T_NOT_TEST;
	frame->duid.rtu_addr = (num_iobj / 256) + 1;
	frame->duid.rad = 0;
	//information object
	frame->obj.fcode = FACT_ID;  //
	frame->obj.pcode = PRODUCT_ID;  //
	frame->obj.dos.year = STANDARD_YEAR;  //标准
	frame->obj.dos.month = STANDARD_MONTH;
	//tail
	frame->farme_tail.cs = this->check_sum(
			(u8*) frame + sizeof(struct Frame_head),
			frame->farme_head.len1);
	frame->farme_tail.end_byte = END_BYTE;
	print_array(f.dat, f.len);
	q.push(f);
	return 0;

}
/*	M_TI_TA_2 返回终端时间帧
 out:	farme_out	返回/发送到主站的帧
 len_out		帧长
 return:	0	成功
 */
int Csd102::fun_M_TI_TA_2(std::queue<struct Frame> &q) const
{
	struct Frame f;
	struct m_tSystime systime;
	GetSystemTime_RTC(&systime);
	struct stFrame_M_TI_TA_2 *pf = (struct stFrame_M_TI_TA_2 *) (f.dat);
	const u8 iObj_num = 1;		//信息体数量
	f.len = sizeof(struct stFrame_M_TI_TA_2);

	//pframe->farme_head.len1=1;
	//printf("RTU time test:farme_head.len1=%d\n",pframe->farme_head.len1);
	//整合:
	pf->farme_head.start_byte1 = START_LONG_FRAME;
	pf->farme_head.len1 = sizeof(struct stFrame_M_TI_TA_2)
			- sizeof(struct Frame_head) - sizeof(struct Frame_tail);
	pf->farme_head.len2 = pf->farme_head.len1;
	pf->farme_head.start_byte2 = START_LONG_FRAME;
	pf->lpdu_head.c_up.funcode = FN_M_SEND_DAT;
	pf->lpdu_head.c_up.prm = CF_PRM_UP;
	pf->lpdu_head.c_up.acd = 1;
	pf->lpdu_head.c_up.dfc = CF_DFC_NOT_FULL;
	pf->lpdu_head.link_addr = this->link_addr;
	pf->duid.typ = M_TI_TA_2;
	pf->duid.vaq.sq = VSQ_SQ_Similar;
	pf->duid.vaq.n = iObj_num;     //一个信息体
	pf->duid.cot.cause = COT_REQUEST;     //cot传输原因
	pf->duid.cot.pn = COT_PN_ACK;     //
	pf->duid.cot.t = COT_T_NOT_TEST;     //不是测试(真正进行操作)
	//根据信息体的数量没增加255,值加1,从1开始.
	pf->duid.rtu_addr = (iObj_num / 256) + 1;
	pf->duid.rad = 0;
	//FIXME  把其他数据 意义弄清后赋值
	getsystime(systime, &(pf->obj.t));
	pf->farme_tail.cs = check_sum(f.dat + sizeof(struct Frame_head),
			pf->farme_head.len1);
	pf->farme_tail.end_byte = END_BYTE;
//
	printf("time=%d-%d-%d %d:%d:%d %dms \n", systime.year, systime.mon,
			systime.day, systime.hour, systime.min, systime.sec,
			systime.msec);
	q.push(f);
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
 */
int Csd102::fun_M_NV_NA_2(struct Frame *f) const
{
	struct Short_frame * frame = (struct Short_frame *) f->dat;
	f->len = sizeof(struct Short_frame);
	frame->start_byte = START_SHORT_FRAME;
	frame->c_up.funcode = FN_M_NO_DAT; //4bit
	frame->c_up.dfc = CF_DFC_NOT_FULL; //1 bit
	frame->c_up.acd = 1;		//有1级数据
	frame->c_up.prm = CF_PRM_UP;  //1bit
	frame->c_up.res = CF_RES;  //1bit
	frame->link_addr = this->link_addr;
	frame->farme_tail.cs = this->check_sum(
			(u8*) frame + sizeof(START_SHORT_FRAME),
			sizeof(union Ctrl_up) + sizeof(link_addr_t));
	frame->farme_tail.end_byte = END_BYTE;
	return 0;

}
/*	回复链路状态请求(FN_C_RLK FC9)的帧
 回复: M_LKR_NA_2 FN_M_RSP FC11 以链路状态回应
 */
int Csd102::fun_M_LKR_NA_2(struct Frame *f) const
{
	struct Short_frame * frame = (struct Short_frame *) f->dat;
	f->len = sizeof(struct Short_frame);
	frame->start_byte = START_SHORT_FRAME;
	frame->c_up.funcode = FN_M_RSP;
	frame->c_up.dfc = CF_DFC_NOT_FULL;		//可以接收数据
	frame->c_up.acd = this->acd;		//数据访问需求位
	frame->c_up.prm = CF_PRM_UP;		//上传
	frame->c_up.res = CF_RES;  //1bit
	frame->link_addr = this->link_addr;
	frame->farme_tail.cs = this->check_sum(
			(u8*) frame + sizeof(START_SHORT_FRAME),
			sizeof(union Ctrl_up) + sizeof(link_addr_t));
	frame->farme_tail.end_byte = END_BYTE;
	return 0;
}
/*	复位远方链路确认帧 回复C_RCU_NA_2(复位远方链路[通讯单元])
 Reset Communication Unit
 */
int Csd102::fun_M_CON_NA_2(struct Frame *f) const
{
	struct Short_frame * frame = (struct Short_frame *) f->dat;
	f->len = sizeof(struct Short_frame);
	frame->start_byte = START_SHORT_FRAME;
	frame->c_up.funcode = FN_M_CON;
	frame->c_up.dfc = CF_DFC_NOT_FULL;		//可以接收数据
	frame->c_up.acd = 0;		//数据访问需求位
	frame->c_up.prm = CF_PRM_UP;		//上传
	frame->c_up.res = CF_RES;  //1bit
	frame->farme_tail.cs = check_sum(
			(u8*) frame + sizeof(START_SHORT_FRAME),
			sizeof(union Ctrl_up) + sizeof(link_addr_t));
	frame->farme_tail.end_byte = END_BYTE;
	return 0;
}
/*	C10.2 M_SYN_TA_2 时间同步返回帧
 */
int Csd102::fun_M_SYN_TA_2(const struct Frame fi,
		std::queue<struct Frame> &q) const
{
	struct Frame fo;
	struct stFrame_C_SYN_TA_2 * pfo = (struct stFrame_C_SYN_TA_2 *) fo.dat;
	struct m_tSystime systime;
	GetSystemTime_RTC(&systime);
	getsystime(systime, &pfo->tb);
	//head
	pfo->farme_head.start_byte1 = START_LONG_FRAME;
	pfo->farme_head.len1 = sizeof(struct stFrame_C_SYN_TA_2)
			- sizeof(struct Frame_head) - sizeof(struct Frame_tail);
	pfo->farme_head.len2 = pfo->farme_head.len1;
	pfo->farme_head.start_byte2 = START_LONG_FRAME;
	//
	pfo->lpdu_head.c_up.funcode = FN_M_SEND_DAT;
	pfo->lpdu_head.c_up.prm = CF_PRM_UP;
	pfo->lpdu_head.c_up.acd = 1;
	pfo->lpdu_head.c_up.dfc = CF_DFC_NOT_FULL;
	pfo->lpdu_head.link_addr = this->link_addr;
	//
	fo.len = sizeof(struct stFrame_C_SYN_TA_2);
	q.push(fi);
	return 0;

}
#pragma  GCC diagnostic warning  "-Wunused-parameter"
/*一般校验和程序.
 in:	a	数组(u8 *)
 len	数组长度(int)
 out	无
 return:		一个字节校验和(u8)
 */
u8 Csd102::check_sum(u8 const *a, int const len) const
{
	u8 sum = 0;
	for (int i = 0; i < len; i++) {
		sum += a[i];
	}
	return sum;
}
/*打印字符数组*/
void Csd102::print_array(const u8 *a, const int len) const
{
	int i;
	printf("[%d] ", len);
	for (i = 0; i < len; i++) {
		printf("%02X ", a[i]);
	}
	printf("\n");
	return;
}

