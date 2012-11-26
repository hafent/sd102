/**
  * @file: sd102.h
 */
#ifndef SD102_H
#define SD102_H
#include "CBASE102s.h"
#include "typedefine.h"
#include "sd102_struct.h"
#include "sd102_frame.h"
#include <queue>
#define PREFIX "[sd102]" //答应用的前缀,方便区分
//标准发布日期,山东电力集团公司 发布 2011年3月24日
#define  STANDARD_YEAR (11)
#define  STANDARD_MONTH (3)
//厂商编号,产品编号
#define FACT_ID (0)
#define PRODUCT_ID (0)
//最大帧长
#define MAX_FRAME_LEN (4+255+2) //sd102最大幀長,帧头+len+帧尾
struct Frame {
	u8 dat[MAX_FRAME_LEN];
	int len;
};
//1级数据
struct class1dat {
	u8 dat[MAX_FRAME_LEN];
	int len;
};
extern "C" CProtocol *CreateCProto_sd102();
//class Csd102 :public CProtocol
class Csd102: public CBASE102 {

public:
	Csd102();
	~Csd102();

	void SendProc(void);
	int ReciProc(void);
	virtual int Init(struct stPortConfig *tmp_portcfg);
#if 1
private:
	void show_wait(u32 &stat)const;
	int sync_head(const u8 *buf, int &farme_len) const;
	int separate_msg(struct Frame *f);
	int verify_frame(const struct Frame f) const;
	u8 get_ctrl_field(const struct Frame  f)const;
	int ack(struct Frame &f) const;
	int nack(struct Frame &f) const;
	int transfer(const struct Frame f);
	int process_short_frame(const struct Frame fin,
		struct Frame* f_out) const;
	int process_long_frame(const  struct Frame fin ,
			std::queue<struct Frame> &q);
	int getsystime(const struct m_tSystime systime,struct Tb * t)const;
	int getsystime(const struct m_tSystime systime,struct Ta * t)const;
	// 8.4.7 分类功能
	//监视方向上的**过程信息** 处理函数
	int  fun_M_SP_TA_2(const u8 *farme_in, const int len_in,
			u8 *farme_out,	int &len_out) const;
	int fun_M_IT_TA_2(const struct Frame frame_in,
			std::queue <struct Frame> &q) const;
	int fun_M_IT_TD_2(const struct Frame fi,
			std::queue<struct Frame> &q) const;
	int fun_M_IT_TA_B_2(const struct Frame fi,
			std::queue<struct Frame> &q) const;
	int fun_M_YC_TA_2(const struct Frame fi,
			std::queue<struct Frame> &q) const;
	int fun_M_XL_TA_2(const struct Frame fi,
			std::queue<struct Frame> &q) const;
	int fun_M_IT_TA_C_2(const struct Frame fi,
			std::queue<struct Frame> &q) const;
	int fun_M_IT_TA_D_2(const struct Frame fi,
			std::queue<struct Frame> &q) const;

	//监视方向上的**系统信息** 处理函数
	int fun_M_EI_NA_2(std::queue<struct Frame> &q) const;
	int fun_P_MP_NA_2(std::queue<struct Frame> &q) const;
	int fun_M_TI_TA_2(std::queue<struct Frame> &q) const;
//??
	int fun_M_CON_NA_2(struct Frame *f) const;
	int fun_M_NV_NA_2(struct Frame *f) const;
	int fun_M_LKR_NA_2(struct Frame *f) const;
	int fun_M_SYN_TA_2(const struct Frame fi,
			std::queue<struct Frame> &q) const;
	u8 check_sum(const u8 *a, const int len) const;


	int clear_fcv(void);
	void print_array(const u8 *transbuf, const int len) const;
	int copyframe(struct Frame &df, const struct Frame sf) const;
private:
	typ_t last_typ;
	u8 acd;
	struct Frame mirror_farme;
	struct Frame fout;
	struct Frame send;
	//bool has_mirror_farme;
	//int mirror_farme;
	//u8 mframe[MAX_FARME_LEN];
	//int mfarme_len;
	u32 status;	//用于显示接收数据状态.不是很重要
	u16 link_addr;
	//union Ctrl_down c_bak;//与备份帧冗余
	bool exist_backup_frame;	//存在有效的备份帧.最开始时是没有备份的,只有正确传输过一次之后才会保存备份
	bool has_class1_dat;
	bool has_class2_dat;
	//备份的(上次接收的帧
	struct Frame reci_frame_bak;
	//备份的(上次发送的帧
	struct Frame send_frame_bak;
	//本次发送的帧
	struct Frame send_frame;
	//本次接收的帧
	struct Frame reci_frame;
	std::queue <struct Frame> qclass1;
	std::queue <struct Frame> qclass2;
	struct Frame c1_dat;
#endif
};
#endif // SD102_H
