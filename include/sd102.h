/*File encode:	 GB2312
   filename:	sd102.h
*/
#ifndef SD102_H
#define SD102_H
#include "CBASE102s.h"
#include "sd102_struct.h"
#include "typedefine.h"
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
	int splitmsg(u8 *readbuf, int &len);
	int GX102s_Synchead(u8 * databuf);
	int process_short_frame(u8 * sfarme, int len);
	int process_long_frame(u8 *sfarme,int len);
	u8 check_sum(u8 * a,int len );
	u8 bak_farme[4+255+2];//备份帧
	u16 bak_farme_len;//备份帧长
	void print_array(u8 *transbuf,int len);
};
#endif // SD102_H
