/**
 * @file: sd102.h
 */
#ifndef SD102_H
#define SD102_H
#include "CBASE102s.h"
#include "sd102_stFrame.h"//结构定义
#include "sd102_ctDuid.h"//常量定义
#include "sd102_ctUdat.h"
#include "sd102_ctStart.h"
#include <queue>
#define VER
#ifndef MAJOR
#define MAJOR 0
#endif
#ifndef MINOR
#define MINOR 0
#endif
#ifndef PATCHLEVEL
#define PATCHLEVEL 0
#endif
#define PREFIX "[sd102]" //打印用的前缀,方便区分
#define PREERR "[sd102 ERR]"//
//标准发布日期,山东电力集团公司 发布 2011年3月24日
#define  STANDARD_YEAR (11)
#define  STANDARD_MONTH (3)
//厂商编号,产品编号
#define FACT_ID (123)
#define PRODUCT_ID (456)
#define CLASS2MAX (256) //最大数据存储量:256帧单点数据
//最大用户数据区长度,由len=1字节限制
#define MAX_UDAT_LEN (255)
//最大帧长
#define MAX_FRAME_LEN (4+MAX_UDAT_LEN+2) //sd102最大幀長,帧头+len+帧尾
//通用的帧格式,
struct Frame {
	u8 dat[MAX_FRAME_LEN];
	int len;
};
//tou 电量　文件头 //TODO　这是存储方面的头文件，以后应该修改定义到存储方面
struct touFilehead{
	u8 year;
	u8 month;
	u8 day;
	u8 save_cycle_lo;//存储周期[分钟]
	u8 save_cycle_hi;//存储周期[分钟]
	u8 save_number;
	u8 save_flag1;
	u8 save_flag2;
	u8 save_flag3;
	u8 save_flag4;
};
//某单独电量结构,如 总电量 或者 谷电量
struct Ti{
	int val;
	union{
		u8 Iv;
		struct{
			u8 res:7;//保留 取0
			u8 iv:1;//有效标志,1-有效
		};
	};
};
//tou 总尖峰平谷
struct Ti_Category{
	struct Ti total;
	struct Ti tip;
	struct Ti peak;
	struct Ti flat;
	struct Ti valley;
};
struct Tou{
	struct Ti_Category FA;//正有
	struct Ti_Category RA;//反有
	struct Ti_Category FR;//正无
	struct Ti_Category RR;//反无
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
	int getsystime(struct Ta &t,const struct m_tSystime systime) const;
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
	//TODO　读写数据库（文件）相关的本应该有DB来完成
	void print_tou_head(const struct touFilehead  filehead)const;
	void print_tou_dat(const struct Tou  tou) const;
	u32 get_min(Ta ta)const;
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
