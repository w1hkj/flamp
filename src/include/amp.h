//======================================================================
// amp.h
//
//======================================================================

#ifndef AMP_H
#define AMP_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <map>

#include "crc16.h"

class cAmp {
public:
enum { _FILE, _ID, _DTTM, _SIZE, _DESC, _DATA, _PROG, _CNTL };
typedef std::map<int, string> AMPmap;

private:
// both
	static const char *ltypes[];

// transmit
	std::string xmtfilename;
	std::string xmtbuffer;
	std::string xmtdata;
	std::string xmtstring;
	std::string xmtdttm;
	std::string xmtdesc;
	std::string xmtcall;
	std::string xmtinfo;
	std::string xmthash;
	std::string tosend; // designated blocks if not an ALL transfer
	std::string report_buffer;

	int xmtnumblocks;
	int xmtblocksize;
	int xmt_repeat; // repeat n time; default 1
	int blocksize;
	int fsize;

	bool use_compression;

// tx / rx
	Ccrc16 chksum;

	char *sz_num(int data) {
		static char sznum[20];
		snprintf(sznum, sizeof(sznum), "%d", data);
		return sznum;
	}
	char *sz_len(std::string data) {
		static char szlen[20];
		snprintf(szlen, sizeof(szlen), "%d", (int)data.length());
		return szlen;
	}
	char *sz_size() {
		static char szsize[40];
		snprintf(szsize, sizeof(szsize), "%d %d %d", fsize, xmtnumblocks, xmtblocksize);
		return szsize;
	}

public:
	cAmp(std::string str = "", std::string fname = "");
	~cAmp();

	void clear_rx();

//transmit
	void xmt_buffer(std::string &str) { xmtbuffer = str; }
	std::string xmt_buffer() { return xmtbuffer; }

	void xmt_data(std::string &str) {
		xmtdata.assign(str);
		fsize = xmtdata.length();
		xmtnumblocks = xmtdata.length() / xmtblocksize + (xmtdata.length() % xmtblocksize ? 1 : 0);
	}
	std::string xmt_data() { return xmtdata; };

	void xmt_fname(std::string fn) {
		xmtfilename.assign(fn);
		struct stat statbuf;
		stat(fn.c_str(), &statbuf);
		time_stamp(&statbuf.st_mtime);
	}
	std::string xmt_fname() { return xmtfilename; } 

	std::string xmt_string();

	void xmt_descrip(std::string desc) { xmtdesc = desc; }
	std::string xmt_descrip() { return xmtdesc; }

	void xmt_tosend(std::string str) { tosend = str; }
	std::string xmt_tosend() { return tosend; }

	void xmt_blocksize(int n) { xmtblocksize = n; }
	int  xmt_blocksize() { return xmtblocksize; }
	std::string xmt_numblocks() {
		xmtnumblocks = xmtdata.length() / xmtblocksize + (xmtdata.length() % xmtblocksize ? 1 : 0);
		return sz_num(xmtnumblocks);
	}

	void my_call(std::string call) { xmtcall.assign(call); }
	std::string my_call() { return xmtcall; }

	void my_info(std::string info) { xmtinfo.assign(info); }
	std::string my_info() { return xmtinfo; }

	void compress(bool comp) { use_compression = comp; }
	bool compress() { return use_compression; }

	void time_stamp(time_t *tm = NULL);

	void repeat(int n) {
		xmt_repeat = n;
	}
	int repeat() { return xmt_repeat; }

	void tx_parse_report(std::string s);

// receive
private:
	std::string rxfilename;
	std::string rxbuffer;
	std::string rxdata;
	std::string rxstring;
	std::string rxdttm;
	std::string rxdesc;
	std::string rxcall_info;
	std::string rxhash;
	std::string rxprogname;
	std::string rx_rcvd;

	int rxnumblocks;
	int rxblocksize;
	int rxfilesize;
	int rx_ok_blocks;
	AMPmap rxblocks;

	void rx_parse_dttm_filename(char *, std::string data);
	void rx_parse_desc(string data);
	void rx_parse_size(string data);

public:
	void rx_fname(std::string fn) {
		rxfilename.assign(fn);
	}
	std::string get_rx_fname() { return rxfilename; }

	void rx_append(std::string s) { rxbuffer.append(s); }

	bool hash(std::string s) { return (s == rxhash); }

	std::string rx_recvd_string();
	void rx_time_stamp(std::string ts) { rxdttm.assign(ts); }
	void rx_add_data(std::string data);
	void rx_parse_buffer();
	bool rx_parse_line(int ltype, char *crc, std::string data);
	bool rx_completed() { return (rx_ok_blocks > 0 ? (rx_ok_blocks == rxnumblocks) : false); }

	int rx_size() { return rxfilesize; }
	int rx_nblocks() { return rxnumblocks; }
	std::string rx_fsize() { return sz_num(rxfilesize); }
	std::string rx_blocksize() { return sz_num(rxblocksize); }
	std::string rx_numblocks() { return sz_num(rxnumblocks); }
	std::string rx_time_stamp() { return rxdttm; }
	std::string rx_desc() { return rxdesc; }
	std::string rx_callinfo() { return rxcall_info; }
	std::string rx_progname() { return rxprogname; }
	std::string rx_stats();
	std::string rx_blocks() { return rx_rcvd; }
	std::string rx_missing();
	std::string rx_report();
	std::string rx_hash() { return rxhash; }
	const char* rx_sz_percent() {
		static const char empty[] = "";
		if (rxnumblocks == 0 || rx_ok_blocks == 0) return empty;
		static char percent[6];
		snprintf(percent, sizeof(percent), "%3.0f %%", 100.0*rx_ok_blocks/rxnumblocks);
		return percent;
	}
	float rx_percent() {
		if (rxnumblocks == 0) return 0;
		return 100.0*rx_ok_blocks/rxnumblocks;
	}
};

#endif
