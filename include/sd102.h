#ifndef __NULL_H__
#define __NULL_H__
#include "CBASE102s.h"
#include "sd102_struct.h"
extern "C" CProtocol *CreateCProto_ProNULL();

class Csd102 :public CBASE102
{
public:
	Csd102();
	~Csd102();
	void SendProc(void);
	int ReciProc(void);
	virtual int Init(struct stPortConfig *tmp_portcfg);
private:
	int splitmsg(u8 *readbuf,int &len);
	int GX102s_Synchead(u8 * databuf);
	int process_short_frame(u8 * databuf);
	u8 check_sum(u8 * a,int len );
	u8 bak_farme[253];//±¸·ÝÖ¡
	void print_array(u8 *transbuf,int len);
};
#endif
