//======================================================================
//	amp.h
//
//  Author(s):
//	Dave Freese, W1HKJ, Copyright (C) 2010, 2013
//	Robert Stiles, KK5VD, Copyright (C) 2013, 2014
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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#define RX_AMP 0x1
#define TX_AMP 0x2

#define TEMP_BUFFER_SIZE 4096

#include "crc16.h"
#include "timeops.h"
#include "file_io.h"
#include "threads.h"

extern const char *sz_flmsg;
extern const char *sz_cmd;
extern const char *sz_flamp;
extern void preamble_detected(void);

//! @struct relay_data
//! Used to tranfer information via TX_FLDIGI_THREAD->data pointer
//! to process relay_data with interval timer.

//! @typedef RELAY_DATA
//! @see relay_data

typedef struct relay_data {
	class cAmp *amp;              //!< Enable relay fills using interval timer.
	std::string serial_data;
	vector<std::string>header;
	vector<std::string>data;
	relay_data() {
		serial_data.clear();
		header.clear();
		data.clear();
	}
} RELAY_DATA;

class cAmp {
public:
	enum { _FILE, _ID, _DTTM, _SIZE, _DESC, _DATA, _PROG, _CNTL };
	typedef std::map<int, std::string> AMPmap;
	cAmp(class cAmp *src_amp);

private:
	// both
	int cAmp_type;
	bool _update_required;
	bool _update_required_vector;
	bool _valid_tx_data;
	bool _valid_tx_vec_data;
	int thread_locks;
	pthread_mutex_t mutex_amp_io;
	void unlock(void);
	void lock(void);


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
	std::string modem;
	std::string xmtcallto;
	std::string sz_xfr_size;

	std::vector<std::string> header_string_array;
	std::vector<std::string> data_string_array;

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
	bool use_unproto;
	bool _unproto_markers;

	// tx / rx
	Ccrc16 chksum;

	char *sz_num(int data) {
		static char sznum[20];
		memset(sznum, 0, sizeof(sznum));
		snprintf(sznum, sizeof(sznum)-1, "%d", data);
		return sznum;
	}
	char *sz_len(std::string data) {
		static char szlen[20];
		memset(szlen, 0, sizeof(szlen));
		snprintf(szlen, sizeof(szlen)-1, "%d", (int)data.length());
		return szlen;
	}
	char *sz_size() {
		static char szsize[40];
		int file_size = xmtdata.size();
		memset(szsize, 0, sizeof(szsize));
		snprintf(szsize, sizeof(szsize)-1, "%d %d %d", file_size, xmtnumblocks, xmtblocksize);
		return szsize;
	}

	std::string reformat_missing_blocks(std::string &missing_blocks);
	void xmt_calc_numblocks();
	std::string _file_hash();
	void _time_stamp(time_t *tp);
	std::string _tx_string(void);

	std::string xmt_unproto_string(void);
	void xmt_unproto_string(std::string &str);

public:
	cAmp(std::string str = "", std::string fname = "");
	~cAmp();

	void clear_rx();
	bool update_required(void);

	void update_required(bool flag) {
		_update_required = flag;
	}

	bool preamble_detected(void) { return preamble_detected_flag; }
	void reset_preamble_detection(void) { preamble_detected_flag = false; }

	int amp_type(void) { return cAmp_type; }
	void amp_type(int value) { if(value == TX_AMP || value == RX_AMP) cAmp_type = value; }
	std::string file_hash(void);

	//transmit

	void callto(std::string n);
	std::string callto(void);

	void unproto(bool n);
	bool unproto(void);

	void tx_blocksize(int n);
	int  tx_blocksize(void);

	int  tx_base_conv_index(void);
	void tx_base_conv_index(int val);

	std::string tx_base_conv_str(void);
	void tx_base_conv_str(std::string &str);
	void tx_base_conv_str(const char *str);

	std::string xmt_buffer(void);
	void xmt_buffer(std::string &str);

	int convert_to_plain_text(std::string &_buffer);
	int convert_to_plain_text(char *_src, char *_dst, size_t count);

	void unproto_markers(bool markers);
	void _xmt_unproto(void); // for internal non thread locking.

	void xmt_data(std::string &str);

	std::string xmt_data(void);
	std::string xmt_hash(void);

	void xmt_fname(std::string fn);

	bool xmt_stat(struct stat *stat_storage);
	bool xmt_file_modified(void);

	std::string xmt_fname(void);
	std::string xmt_full_path_fname();

	void xmt_full_path_fname(string fname);
	std::string xmt_string(bool use_locks);

	std::string xmt_descrip(void);

	int xmt_vector_string(bool header_modem, bool unproto_markers);
	std::vector<std::string> &xmt_vector_header(void);
	std::vector<std::string> &xmt_vector_data(void);

	void xmt_descrip(std::string desc);

	void xmt_tosend_clear(void);
	void xmt_tosend(std::string str) ;
	std::string xmt_tosend(void);

	void xmt_blocksize(int n);
	int  xmt_blocksize(void);

	std::string xmt_numblocks();

	void my_call(std::string call) ;
	std::string my_call(void);

	void my_info(std::string info);
	std::string my_info();

	void compress(bool comp);
	bool compress(void);

	void forced_compress(bool comp);
	bool forced_compress(void);

	void time_stamp(time_t *tm = NULL);

	void repeat(int n);
	int  repeat(void);

	void header_repeat(int n);
	int  header_repeat(void);

	//void tx_parse_report(std::string s);
	void tx_parse_report(void);

	void amp_update(void);
	std::string tx_string(std::string t_string);
	std::string estimate(void);
	void _estimate(void);

	std::string xmt_modem(void);
	void xmt_modem(std::string _m);

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
	std::string relay_blocks;
	std::string _rx_raw_file;
	std::string _rx_raw_id;
	std::string _rx_raw_size;
	std::string _rx_raw_desc;
	std::string _rx_raw_prog;
	std::string _rx_raw_cntl;

	int rxnumblocks;
	int rxblocksize;
	int rxfilesize;
	int rx_ok_blocks;
	int rx_crc_flags;

	char temp_buffer[TEMP_BUFFER_SIZE+1];

	AMPmap rxblocks;
	AMPmap rxDataHeader;

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
	void rx_to_tx_hash(void) { xmthash.assign(rxhash); }

	void rx_relay_blocks(std::string str) {
		relay_blocks.assign(reformat_missing_blocks(str));
	}

	std::string rx_relay_blocks(void) { return relay_blocks; }

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

	std::string rx_raw_file(void) { return _rx_raw_file; }
	std::string rx_raw_id(void)   { return _rx_raw_id;   }
	std::string rx_raw_size(void) { return _rx_raw_size; }
	std::string rx_raw_desc(void) { return _rx_raw_desc; }
	std::string rx_raw_prog(void) { return _rx_raw_prog; }
	std::string rx_raw_cntl(void) { return _rx_raw_cntl; }

	std::string tx_relay_string(std::string callfrom, std::string missig_blocks);
	int tx_relay_vector(std::string callfrom, std::string missing_blocks);

	const char* rx_sz_percent() {
		static const char empty[] = "  0 %";
		if (rxnumblocks == 0 || rx_ok_blocks == 0) return empty;		static char percent[6];
		int nokb = rx_ok_blocks;
		int nrxb = rxnumblocks + 1;
		if(!rx_crc_flags)
			nokb++;
		snprintf(percent, sizeof(percent), "%3.0f %%", 100.0*nokb/nrxb);
		return percent;
	}
	float rx_percent() {
		int nokb = rx_ok_blocks;
		int nrxb = rxnumblocks + 1;
		if(!rx_crc_flags)
			nokb++;
		if (rxnumblocks == 0) return 0;
		return 100.0*nokb/nrxb;
	}
};

#endif
