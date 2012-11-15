/*File encode:	 GB2312
   filename:	sd102.h
*/
#ifndef SD102_H
#define SD102_H
#include "CBASE102s.h"
#include "sd102_struct.h"
#include "typedefine.h"
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
	int sync_head(u8 * buf, int &farme_len) const;
	int separate_msg(u8 *readbuf, int &len);
	int verify_farme(u8 * dat, int len) const;
	union Ctrl_down get_ctrl_field(u8* farme,int farme_len);
	int Transfer(u8* farme,int farme_len);
	int process_short_frame(const u8 *recifarme, const int len,
	                        u8 *farme_out, int &len_out) const;
	int process_long_frame(const u8 *farme_in,const int len_in,
	                       u8 *farme_out, int &len_out);
	//分类功能
	int fun_C_TI_NA_2(u8 *farme_out, int &len_out) const;
	u8 check_sum(u8 *a,int len ) const;
	//备份的 接收帧
	u8 reci_farme_bak[4+255+2];//帧
	int reci_farme_bak_len;//帧
	//备份的 发送帧
	u8 tran_farme_bak[4+255+2];//帧
	int tran_farme_bak_len;//帧
	//发送帧

	int tran_farme_len;//帧
	int clear_fcv(void);
	void print_array(u8 *transbuf,int len);
	int save_reci_farme(void *farme,int len);
	int save_tran_farme(void *farme,int len,
	                    u8 *bakfarme, int &bakfarme_len, bool &hasbaked);
private:
	u32 status;
	u16 link_addr;
	union Ctrl_down c_bak;//与备份帧冗余
	bool exist_backup_frame;//存在有效的备份帧.最开始时是没有备份的,只有正确传输过一次之后才会保存备份

#endif
};
#endif // SD102_H
