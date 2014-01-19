//======================================================================
//	amp.h
//
//  Author(s):
//	Dave Freese, W1HKJ, Copyright (C) 2010, 2013
//	Robert Stiles, KK5VD, Copyright (C) 2013
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with the program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// =====================================================================


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

#define FILE_CRC_FLAG 1
#define ID_CRC_FLAG   2
#define SIZE_CRC_FLAG 4
#define PROG_CRC_FLAG 8

#define CMD_FLAG   0x1
#define FLMSG_FLAG 0x2

#include "crc16.h"
#include "timeops.h"
#include "file_io.h"

extern const char *sz_flmsg;
extern const char *sz_cmd;
extern const char *sz_flamp;
extern void preamble_detected(void); // Located in flamp.cxx

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
    std::string xmtbase;
    std::string xmtunproto;
	std::string xmtfilename_fullpath;

	std::vector<std::string> header_string_array;
	std::vector<std::string> data_string_array;

	std::string file_hash(void);
	std::string program_header(void);
	std::string file_header(void);
	std::string id_header(void);
	std::string desc_header(void);
	std::string size_header(void);
	std::string data_block(int index);
	std::string data_eof(void);
	std::string data_eot(void);

	struct stat tx_statbuf;

	int xmtnumblocks;
	int xmtblocksize;
	int xmt_repeat; // repeat n time; default 1
	int repeat_header; // repeat header; default 1
	int blocksize;
	int fsize;
    int base_conversion_index;
	bool use_compression;
	bool use_forced_compression;
    bool preamble_detected_flag;

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

	bool preamble_detected(void) { return preamble_detected_flag; }
	void reset_preamble_detection(void) { preamble_detected_flag = false; }

	//transmit

	void tx_blocksize(int n) { blocksize = n; }
	int  tx_blocksize() { return blocksize; }

    int  tx_base_conv_index() { return base_conversion_index; }
    void tx_base_conv_index(int val) { base_conversion_index = val; }

    std::string tx_base_conv_str() { return xmtbase; }
    void tx_base_conv_str(std::string &str) { xmtbase.assign(str); }
    void tx_base_conv_str(const char *str) { xmtbase.assign(str); }

	std::string xmt_buffer() { return xmtbuffer; }
	void xmt_buffer(std::string &str) { xmtbuffer = str; }

	std::string xmt_unproto_string() { return xmtunproto; }
	void xmt_unproto_string(std::string &str) { xmtunproto = str; }

	void xmt_unproto(void);

	void xmt_data(std::string &str) {
		xmtdata.assign(str);
		fsize = xmtdata.length();
		xmtnumblocks = xmtdata.length() / xmtblocksize + (xmtdata.length() % xmtblocksize ? 1 : 0);
	}

	std::string xmt_data(void) { return xmtdata; };
	std::string xmt_hash(void) { return xmthash; };

	void xmt_fname(std::string fn) {
		xmtfilename.assign(fn);
		time_stamp(&tx_statbuf.st_mtime);
	}

	bool xmt_stat(struct stat *stat_storage) {
		if(stat_storage) {
			memcpy(stat_storage, &tx_statbuf, sizeof(tx_statbuf));
			return true;
		}
		return false;
	}

	bool xmt_file_modified(void) {
		struct stat cur_stat;
		if(xmtfilename_fullpath.size() > 0) {
			stat(xmtfilename_fullpath.c_str(), &cur_stat);
			if(cur_stat.st_mtime == tx_statbuf.st_mtime)
				return false;
		}
		return true;
	}

	std::string xmt_fname() { return xmtfilename; }

	std::string xmt_full_path_fname() { return xmtfilename_fullpath; }

	void xmt_full_path_fname(string fname) {
		xmtfilename_fullpath = fname;
		stat(xmtfilename_fullpath.c_str(), &tx_statbuf);
		time_stamp(&tx_statbuf.st_mtime);
	}

	std::string xmt_string();

	int xmt_vector_string(void);
	std::vector<std::string> xmt_vector_header(void) { return header_string_array; }
	std::vector<std::string> xmt_vector_data(void) { return data_string_array; }

	void xmt_descrip(std::string desc) { xmtdesc = desc; }
	std::string xmt_descrip() { return xmtdesc; }

	void xmt_tosend_clear(void) { tosend.clear(); report_buffer.clear(); }
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

	void forced_compress(bool comp) { use_forced_compression = comp; }
	bool forced_compress() { return use_forced_compression; }

	void time_stamp(time_t *tm = NULL);

	void repeat(int n) { xmt_repeat = n; }
	int  repeat() { return xmt_repeat; }

	void header_repeat(int n) { repeat_header = n; }
	int  header_repeat() { return repeat_header; }

	//void tx_parse_report(std::string s);
	void tx_parse_report(void);

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
	int rx_crc_flags;

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
	void append_report(std::string s);
    void rx_parse_id(std::string data);
	bool rx_parse_line(int ltype, char *crc, std::string data);
	bool rx_completed() {
		return (rx_ok_blocks > 0 ? ((rx_ok_blocks == rxnumblocks) && (rx_crc_flags == 0)) : false);
	}

	int rx_size() { return rxfilesize; }
	int rx_nblocks() { return rxnumblocks; }
	int rx_blocksize_int() { return rxblocksize; }

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
	std::string rx_hash(string s) { rxhash = s; return s; }
    std::string rx_parse_hash_line(string data);

	const char* rx_sz_percent() {
		static const char empty[] = "";
		if (rxnumblocks == 0 || rx_ok_blocks == 0) return empty;
		static char percent[6];
        int nokb = rx_ok_blocks;
        int nrxb = rxnumblocks + 1;
        if(!rx_crc_flags)
            nokb++;
		snprintf(percent, sizeof(percent), "%3.0f %%", 100.0*nokb/nrxb);
		//		snprintf(percent, sizeof(percent), "%3.0f %%", 100.0*rx_ok_blocks/rxnumblocks);
		return percent;
	}
	float rx_percent() {
        int nokb = rx_ok_blocks;
        int nrxb = rxnumblocks + 1;
        if(!rx_crc_flags)
            nokb++;
		if (rxnumblocks == 0) return 0;
		return 100.0*nokb/nrxb;
		//		return 100.0*rx_ok_blocks/rxnumblocks;
	}
};

#endif
