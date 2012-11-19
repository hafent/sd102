/*File encode:	 GB2312
   filename:	sd102.h
*/
#ifndef SD102_H
#define SD102_H
#include "CBASE102s.h"
#include "sd102_struct.h"
#include "typedefine.h"
#define MAX_FRAME_LEN (4+255+2) //最大幀長,帧头+len+帧尾

struct Farme {
	u8 dat[MAX_FRAME_LEN];
	int len;
};
//1级数据
struct class1dat{
	u8 dat[4+255+2];
	int len;
};
extern "C" CProtocol *CreateCProto_sd102();
//class Csd102 :public CProtocol
class Csd102 :public CBASE102
{

public:
	Csd102();
	~Csd102();

	void SendProc(void);
	int ReciProc(void);
	virtual int Init(struct stPortConfig *tmp_portcfg);
#if 1
private:
	void show_wait(u32 &stat);
	int sync_head(const u8 *buf, int &farme_len) const;
	int separate_msg(u8 *readbuf, int &len);
	int verify_frame(const u8 *c1_dat, const int len) const;
	union Ctrl_down get_ctrl_field(const u8 *farme,const int farme_len);
	int confirm(u8 *farme_out,int &len_out)const;
	int nack(u8 *farme_out,int &len_out)const;
	int transfer(const u8 *farme,const int farme_len);
	int process_short_frame(const u8 *recifarme, const int len,
				u8 *farme_out, int &len_out) const;
	int process_long_frame(const u8 *farme_in,const int len_in,
			       u8 *farme_out, int &len_out);
	//分类功能
	int fun_M_TI_TA_2(u8 *farme_out, int &len_out) const;
	int fun_M_CON_NA_2(u8 *farme_out, int &len_out )const;
	int fun_M_NV_NA_2(u8 *farme_out, int &len_out )const;
	int fun_M_LKR_NA_2(u8 *farme_out, int &len_out )const;
	int fun_M_IT_TA_2(const u8  *farme_in ,const int len_in,
				  u8 *farme_out, int &len_out )const;
	u8 check_sum(const u8 *a,const int len ) const;

	//备份的 接收帧
	u8 reci_frame_bak[MAX_FRAME_LEN];//帧
	int reci_frame_bak_len;//帧
	//备份的 发送帧
	u8 tran_frame_bak[MAX_FRAME_LEN];//帧
	int tran_frame_bak_len;//帧
	//发送帧

	int tran_frame_len;//帧
	int clear_fcv(void);
	void print_array(const u8 *transbuf,const int len) const;
	int save_reci_frame(void *farme,int len);
	int save_tran_frame(void *farme,int len,
			    u8 *bakfarme, int &bakfarme_len, bool &hasbaked);
	int save_dat(u8* ddat,const u8 *sdat,const int slen)const;
private:
	typ_t last_typ;
	struct Farme mirror_farme;
	bool has_mirror_farme;
	//int mirror_farme;
	//u8 mfarme[MAX_FARME_LEN];
	//int mfarme_len;
	u32 status;//用于显示接收数据状态.不是很重要
	u16 link_addr;
	//union Ctrl_down c_bak;//与备份帧冗余
	bool exist_backup_frame;//存在有效的备份帧.最开始时是没有备份的,只有正确传输过一次之后才会保存备份
	bool has_class1_dat;
	bool has_class2_dat;
	struct Farme c1_dat;
	void set_has_mirror_farme(bool val);
#endif
};
#endif // SD102_H
