/**
 * @file: sd102.h
 */
#ifndef SD102_H
#define SD102_H
#include "CBASE102s.h"
//结构定义
#include "sd102_stFrame.h"
//常量定义
#include "sd102_ctDuid.h"
#include "sd102_ctUdat.h"
#include "sd102_ctStart.h"
#include <queue>
#define PREFIX "[sd102]" //打印用的前缀,方便区分
#define PREERR "[sd102 ERR]"//
//标准发布日期,山东电力集团公司 发布 2011年3月24日
#define  STANDARD_YEAR (11)
#define  STANDARD_MONTH (3)
//厂商编号,产品编号
#define FACT_ID (123)
#define PRODUCT_ID (456)
//最大用户数据区长度,由len=1字节限制
#define MAX_UDAT_LEN (255)
//最大帧长
#define MAX_FRAME_LEN (4+MAX_UDAT_LEN+2) //sd102最大幀長,帧头+len+帧尾
//通用的帧格式,
struct Frame {
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
private:
	void show_wait(u32 &stat) const;
	int sync_head(const u8 *buf, int &farme_len) const;
	int separate_msg(struct Frame &f);
	int verify_frame(const struct Frame f) const;
	u8 get_ctrl_field(const struct Frame f) const;
	int ack(struct Frame &f) const;
	int nack(struct Frame &f) const;
	int transfer(const struct Frame f);
	int process_short_frame(const struct Frame fin,
	        struct Frame* f_out) const;
	int process_request(const struct Frame fin,
	        std::queue<struct Frame> &q1,
	        std::queue<struct Frame> &q2);
	int getsystime(struct Tb &t,const struct m_tSystime systime) const;
	int getsystime(struct Ta &t , const struct m_tSystime systime) const;
	int setsystime(TMStruct &systime, const struct Tb t) const;
	// 8.4.7 分类功能
	//监视方向上的**过程信息** 处理函数
	int make_M_SP_TA_2(const struct Frame fi,
	        std::queue<struct Frame> &q) const;
	int make_M_IT_TA_2(const struct Frame fi,
	        std::queue<struct Frame> &q1) const;
	int make_M_IT_TD_2(const struct Frame fi,
	        std::queue<struct Frame> &q) const;
	int make_M_IT_TA_B_2(const struct Frame fi,
	        std::queue<struct Frame> &q) const;
	int make_M_YC_TA_2(const struct Frame fi,
	        std::queue<struct Frame> &q) const;
	int make_M_XL_TA_2(const struct Frame fi,
	        std::queue<struct Frame> &q) const;
	int make_M_IT_TA_C_2(const struct Frame fi,
	        std::queue<struct Frame> &q) const;
	int make_M_IT_TA_D_2(const struct Frame fi,
	        std::queue<struct Frame> &q) const;
	//监视方向上的**系统信息** 处理函数
	int make_M_EI_NA_2(std::queue<struct Frame> &q) const;
	int make_P_MP_NA_2(std::queue<struct Frame> &q) const;
	int make_M_TI_TA_2(std::queue<struct Frame> &q) const;
	// C9.2 在监视方向上的固定帧长的链路服务数据单元
	int make_M_CON_NA_2(struct Frame &f) const;
	int make_M_NV_NA_2(struct Frame &f) const;
	int make_M_LKR_NA_2(struct Frame &f) const;
	int make_M_SYN_TA_2(const struct Frame fi,
	        std::queue<struct Frame> &q) const;
	//
	u8 check_sum(const u8 *a, const int len) const;
	int format_ok(struct stFrame_C_CI_NR_2 fin) const;
	int time_range(const struct Obj_C_CI_XX_2 obj) const;
	int ioa_range(const struct Obj_C_CI_XX_2 obj) const;
	rtu_addr_t makeaddr(int obj_num) const;
	void showtime(const struct Ta t) const;
	void showtime(const struct Tb t) const;
	bool need_resend(const struct Frame rf_bak,const struct Frame rf) const;
	int time_range(const struct Ta starttime, const struct Ta endtime) const;
	int print_err_msg(int msg) const;
	template<typename T> u8 check_sum(const T f) const;
	template<typename T>int make_mirror_1( T pf,bool b_acd) const;
	template<typename T>int make_mirror_2( T pf) const;
	int clear_fcb(struct Frame &fbak) const;
	void print_array(const u8 *transbuf, const int len) const;
	int copyframe(struct Frame &df, const struct Frame sf) const;
private:
	//typ_t last_typ;
	//u8 acd;
	u32 spon;//模擬突發的信息
	u32 status;	//用于显示接收数据状态.不是很重要
	link_addr_t link_addr;
	//备份的(上次接收的帧
	struct Frame reci_frame_bak;
	//备份的(上次发送的帧
	struct Frame send_frame_bak;
	//本次接收的帧
	struct Frame reci_frame;
	//本次发送的帧
	struct Frame send_frame;
	struct Frame mirror_farme;
	std::queue<struct Frame> qclass1;
	std::queue<struct Frame> qclass2;
};
#endif // SD102_H
