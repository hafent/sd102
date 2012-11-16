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
	void show_wait(u32 &stat);
	int sync_head(const u8 *buf, int &farme_len) const;
	int separate_msg(u8 *readbuf, int &len);
	int verify_farme(const u8 *dat, const int len) const;
	union Ctrl_down get_ctrl_field(const u8 *farme,const int farme_len);
	int confirm(u8 *farme_out,int &len_out)const;
	int nack(u8 *farme_out,int &len_out)const;
	int Transfer(const u8 *farme,const int farme_len);
	int process_short_frame(const u8 *recifarme, const int len,
				u8 *farme_out, int &len_out) const;
	int process_long_frame(const u8 *farme_in,const int len_in,
			       u8 *farme_out, int &len_out);
	//���๦��
	int fun_C_TI_NA_2(u8 *farme_out, int &len_out) const;
	int fun_M_CON_NA_2(u8 *farme_out, int &len_out )const;
	int fun_M_NV_NA_2(u8 *farme_out, int &len_out )const;
	int fun_M_LKR_NA_2(u8 *farme_out, int &len_out )const;
	u8 check_sum(const u8 *a,const int len ) const;
	//���ݵ� ����֡
	u8 reci_farme_bak[4+255+2];//֡
	int reci_farme_bak_len;//֡
	//���ݵ� ����֡
	u8 tran_farme_bak[4+255+2];//֡
	int tran_farme_bak_len;//֡
	//����֡

	int tran_farme_len;//֡
	int clear_fcv(void);
	void print_array(const u8 *transbuf,const int len) const;
	int save_reci_farme(void *farme,int len);
	int save_tran_farme(void *farme,int len,
			    u8 *bakfarme, int &bakfarme_len, bool &hasbaked);
private:
	u32 status;
	u16 link_addr;
	union Ctrl_down c_bak;//�뱸��֡����
	bool exist_backup_frame;//������Ч�ı���֡.�ʼʱ��û�б��ݵ�,ֻ����ȷ�����һ��֮��Żᱣ�汸��
	bool has_class1_dat;
	bool has_class2_dat;

#endif
};
#endif // SD102_H
