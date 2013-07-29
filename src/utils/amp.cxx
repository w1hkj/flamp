//======================================================================
//	amp.cxx
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

#include <list>

#include "config.h"

#include "amp.h"
#include "threads.h"
#include "debug.h"

#define nuline "\n"

const char * cAmp::ltypes[] = {
	"<FILE ", "<ID ", "<DTTM ", "<SIZE ", "<DESC ", "<DATA ", "<PROG ", "<CNTL "
};


cAmp::cAmp(std::string str, std::string fname)
{
	xmtbuffer.assign(str);
	xmtfilename.assign(fname);
	xmtcall.clear();
	xmtinfo.clear();
	xmtdttm.clear();
	xmtstring.clear();
	xmthash.clear();
	tosend.clear();
	report_buffer.clear();

	use_compression = false;
	use_forced_compression = false;

	if (xmtfilename.empty()) xmtfilename.assign("UNKNOWN.txt");
	xmtblocksize = 64;
	xmtdesc = "";
	xmt_repeat = 1;
	repeat_header = 1;
	fsize = xmtdata.length();
	xmtnumblocks = xmtdata.length() / xmtblocksize + (xmtdata.length() % xmtblocksize ? 1 : 0);

	rx_crc_flags = ( FILE_CRC_FLAG | ID_CRC_FLAG | SIZE_CRC_FLAG );

	rxbuffer.clear();
	rxblocks.clear();
	rxfilename.assign("Unassigned");
	rxdata.clear();
	rxstring.clear();
	rxdttm.clear();
	rxdesc.clear();
	rxcall_info.clear();
	rxhash.clear();
	rxnumblocks = rxblocksize = rxfilesize = rx_ok_blocks = 0;
}

cAmp::~cAmp() { }

void cAmp::clear_rx()
{
	rxbuffer.clear();
	rxblocks.clear();
	rxfilename.clear();
	rxdata.clear();
	rxstring.clear();
	rxdttm.clear();
	rxdesc.clear();
	rxcall_info.clear();
	rx_rcvd.clear();
	rxnumblocks = rxblocksize = rxfilesize = rx_ok_blocks = 0;
}

std::string cAmp::file_hash()
{
	std::string chksumdata;
	std::string filename;
	char buf[32];

	filename.assign(xmtdttm).append(":").append(xmtfilename);

	chksumdata.assign(filename);

	if(compress() || forced_compress())
		chksumdata.append("1");
	else
		chksumdata.append("0");

	chksumdata.append(tx_base_conv_str());

	// to_string not available
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%d", xmtblocksize);

	chksumdata.append(buf);

	return chksum.scrc16(chksumdata);
}

std::string cAmp::program_header(void)
{
	std::string temp;
	std::string xmit;

	temp.assign("{").append(xmthash).append("}");
	temp.append(PACKAGE_NAME).append(" ").append(PACKAGE_VERSION);
	xmit.assign(ltypes[_PROG]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	xmit.append(">").append(temp).append(nuline);

	return xmit;
}

std::string cAmp::file_header(void)
{
	std::string temp;
	std::string xmit;
	std::string filename;

	filename.assign(xmtdttm).append(":").append(xmtfilename);

	temp.assign("{").append(xmthash).append("}").append(filename);
	xmit.assign(ltypes[_FILE]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	xmit.append(">").append(temp).append(nuline);

	return xmit;
}

std::string cAmp::id_header(void)
{
	std::string temp;
	std::string xmit;

	temp.assign("{").append(xmthash).append("}");
	temp.append(xmtcall).append(" ").append(xmtinfo);
	xmit.assign(ltypes[_ID]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	xmit.append(">").append(temp).append(nuline);

	return xmit;
}

std::string cAmp::desc_header(void)
{
	std::string temp;
	std::string xmit;

	temp.assign("{").append(xmthash).append("}").append(xmtdesc);
	xmit.assign(ltypes[_DESC]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	xmit.append(">").append(temp).append(nuline);

	return xmit;
}

std::string cAmp::size_header(void)
{
	std::string temp;
	std::string xmit;

	temp.assign("{").append(xmthash).append("}").append(sz_size());
	xmit.assign(ltypes[_SIZE]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	xmit.append(">").append(temp).append(nuline);

	return xmit;
}

std::string cAmp::data_block(int index)
{
	std::string temp;
	std::string xmit;
	char blknbr[32];

	memset(blknbr, 0, sizeof(blknbr));
	snprintf(blknbr, sizeof(blknbr) - 1, "{%s:%d}", xmthash.c_str(), index);
	temp.assign(blknbr).append(xmtdata.substr((index - 1) * xmtblocksize, xmtblocksize));
	xmit.assign(ltypes[_DATA]).append(sz_len(temp)).append(" ");
	xmit.append(chksum.scrc16(temp)).append(">");
	xmit.append(temp).append(nuline);

	return xmit;
}

std::string cAmp::data_eof(void)
{
	std::string temp;
	std::string xmit;

	temp.assign("{").append(xmthash).append(":").append("EOF}");
	xmit.assign(ltypes[_CNTL]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	xmit.append(">").append(temp).append(nuline);

	return xmit;
}

std::string cAmp::data_eot(void)
{
	std::string temp;
	std::string xmit;

	temp.assign("{").append(xmthash).append(":").append("EOT}");
	xmit.assign(ltypes[_CNTL]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	xmit.append(">").append(temp).append(nuline);

	return xmit;
}

std::string cAmp::xmt_string() {
	std::string temp;
	std::string fileline;
	std::string filename;
	std::string statsline;
	std::string idline;
	std::string fstring; // file strings

	xmtstring.clear();

	xmthash = file_hash();

	xmtstring.assign(program_header());

	idline.assign(id_header());
	fileline.assign(file_header());
	statsline.assign(size_header());

	for (int i = 0; i < repeat_header; i++) {
		fstring.append(fileline);
		fstring.append(idline);
		fstring.append(statsline);
	}

	if (!xmtdesc.empty()) {
		fstring.append(desc_header());
	}


	if (tosend.empty()) {
		for (int i = 0; i < xmtnumblocks; i++) {
			fstring.append(data_block(i + 1));
		}
	} else {
		string blocks = tosend;
		int bnbr;
		while (!blocks.empty()) {
			if (sscanf(blocks.c_str(), "%d", &bnbr) == 1) {
				if (bnbr > 0 && bnbr <= xmtnumblocks) {
					fstring.append(data_block(bnbr));
				}
			}
			while (!blocks.empty() && isdigit(blocks[0]))
				blocks.erase(0,1);
			while (!blocks.empty() && !isdigit(blocks[0]))
				blocks.erase(0,1);
		}
	}

	fstring.append(data_eof());

	for (int i = 0; i < xmt_repeat; i++)
		xmtstring.append(fstring);

	xmtstring.append(data_eot());

	return xmtstring;
}

void cAmp::xmt_unproto(void)
{
	size_t pos = 0;
	int appendFlag = 0;
	int count = 0;
	std::string temp;

	const std::string cmdAppendMsg = "<_md>";
//	const std::string flmsgAppendMsg = "<_lmsg>";

	temp.assign(xmtbuffer);
	
	if(isPlainText(temp) == false)
		convert_to_plain_text(temp);

	pos = 0;
	do {
		pos = temp.find(sz_cmd, pos);
		if(pos != std::string::npos) {
			appendFlag |= CMD_FLAG;
			temp.replace(pos, cmdAppendMsg.size(), cmdAppendMsg);
			pos += 3;
		}
	} while(pos != std::string::npos);

//	pos = 0;
//	do {
//		pos = temp.find(sz_flmsg, pos);
//		if(pos != std::string::npos) {
//			appendFlag |= FLMSG_FLAG;
//			temp.replace(pos, flmsgAppendMsg.size(), flmsgAppendMsg);
//			pos += 3;
//		}
//	} while(pos != std::string::npos);

	if(appendFlag)
		temp.append("\nNOTICE: Command Character Substitution!\n");

	if((appendFlag & CMD_FLAG) != 0) {
		temp.append(cmdAppendMsg).append(" <-> \'_\' = \'c\'\n");
	}

//	if((appendFlag & FLMSG_FLAG) != 0) {
//		temp.append(flmsgAppendMsg).append(" <-> \'_\' = \'f\'\n");
//	}

	temp.append("\n");
	
	xmtunproto.assign(temp);

}

int cAmp::xmt_vector_string()
{
	xmthash = file_hash();

	header_string_array.clear();
	data_string_array.clear();

	for (int i = 0; i < repeat_header; i++) {
		header_string_array.push_back(program_header());
		header_string_array.push_back(file_header());
		header_string_array.push_back(id_header());
		header_string_array.push_back(size_header());

		if (!xmtdesc.empty()) {
			header_string_array.push_back(desc_header());
		}
	}

	for (int i = 0; i < xmt_repeat; i++) {
		if (tosend.empty()) {
			for (int i = 0; i < xmtnumblocks; i++) {
				data_string_array.push_back(data_block(i + 1));
			}
		} else {
			string blocks = tosend;
			int bnbr;
			while (!blocks.empty()) {
				if (sscanf(blocks.c_str(), "%d", &bnbr) == 1) {
					if (bnbr > 0 && bnbr <= xmtnumblocks) {
						data_string_array.push_back(data_block(bnbr));
					}
				}
				while (!blocks.empty() && isdigit(blocks[0]))
					blocks.erase(0,1);
				while (!blocks.empty() && !isdigit(blocks[0]))
					blocks.erase(0,1);
			}
		}
		data_string_array.push_back(data_eof());
	}

	data_string_array.push_back(data_eot());

	return (header_string_array.size() + data_string_array.size());
}

void cAmp::time_stamp(time_t *tp)
{
	static char szDt[80];
	time_t tmptr;
	tm sTime;
	time (&tmptr);
	if (tp == NULL)
		gmtime_r (&tmptr, &sTime);
	else
		gmtime_r (tp, &sTime);
	strftime(szDt, 79, "%Y%m%d%H%M%S", &sTime);
	xmtdttm = szDt;
}

void cAmp::rx_add_data(string data)
{
	int blknbr;

	if (rxhash != data.substr(1,4)) {
		LOG_DEBUG("Datablock not for %s", rxfilename.c_str());
		return;
	}
	if (sscanf(data.substr(6).c_str(), "%d", &blknbr) != 1) {
		LOG_ERROR("%s\ncannot convert %s to block #",
				  rxfilename.c_str(),
				  data.substr(6,3).c_str());
		return ;
	}

	if (rxblocks.find(blknbr) == rxblocks.end()) {
		size_t sp = data.find("}");
		if (sp == std::string::npos) return;
		char nstr[20];
		snprintf(nstr, sizeof(nstr), "%d", blknbr);
		rxblocks.insert(AMPmap::value_type(blknbr, data.substr(sp+1)));
		rx_rcvd.append(nstr).append(" ");
		LOG_INFO("file: %s  block %d\n%s", rxfilename.c_str(), blknbr, data.substr(sp+1).c_str());
		rx_ok_blocks++;
	}
}

std::string cAmp::rx_recvd_string()
{
	std::string retstr = "";
	if (!rx_completed()) return retstr;
	AMPmap::iterator iter;
	for (iter = rxblocks.begin(); iter != rxblocks.end(); iter++) {
		retstr.append(iter->second);
	}
	return retstr;
}

void cAmp::rx_parse_dttm_filename(char *crc, std::string data)
{
	LOG_WARN("%s : %s", crc, data.c_str());

	sscanf(data.c_str(), "{%4s}*", crc);

	size_t p = data.find("}");
	data.erase(0, p + 1);

	size_t pcolon = data.find(":");
	if (pcolon == std::string::npos) return;
	rxdttm = data.substr(0, pcolon);
	rxfilename = data.substr(pcolon + 1);
	rxhash = crc;
	rx_crc_flags &= ~FILE_CRC_FLAG;
}

std::string cAmp::rx_parse_hash_line(string data)
{
	char hashval[5];
	static string empty("");

	if (sscanf(data.c_str(), "{%4s}*", hashval) != 1) return empty;

	if (rxhash != hashval) {
		LOG_ERROR("%s", "not this file");
		return empty;
	}
	size_t sp = data.find("}");
	if (sp == std::string::npos) return empty;

	return data.substr(sp+1);
}

void cAmp::rx_parse_desc(string data)
{
	rxdesc = rx_parse_hash_line(data);
}

void cAmp::rx_parse_id(string data)
{
	rxcall_info = rx_parse_hash_line(data);
	rx_crc_flags &= ~ID_CRC_FLAG;
}

void cAmp::rx_parse_size(string data)
{
	char hashval[5];
	int fs, nb, bs;
	if (sscanf(data.c_str(), "{%4s}%d %d %d", hashval, &fs, &nb, &bs) != 4)
		return;
	if (rxhash != hashval) {
		LOG_ERROR("%s", "not this file");
		return;
	}
	rxfilesize = fs;
	rxnumblocks = nb;
	rxblocksize = bs;
	rx_crc_flags &= ~SIZE_CRC_FLAG;
}

bool cAmp::rx_parse_line(int ltype, char *crc, std::string data)
{
	switch (ltype) {
		case _FILE : rx_parse_dttm_filename(crc, data); break;
		case _DESC : rx_parse_desc(data); break;
		case _DATA : rx_add_data(data); break;
		case _SIZE : rx_parse_size(data); break;
		case _PROG : rxprogname = rx_parse_hash_line(data); break;
		case _ID   : rx_parse_id(data); break;
		case _CNTL :
		default : ;
	}

	return true;
}

void cAmp::rx_parse_buffer()
{
	if (rxbuffer.length() < 16) return;

	size_t p = 0, p1 = 0;
	int len;
	char crc[5];
	for (int n = _FILE; n <= _CNTL; n++) {
		p = rxbuffer.find(ltypes[n]);
		if (p != std::string::npos) {
			if (p > 0) rxbuffer.erase(0, p);
			if (sscanf(rxbuffer.substr(strlen(ltypes[n])).c_str(), "%d %4s", &len, crc) == 2) {
				if (len > 2048 + 6) { // exceeds maximum allowable length
					rxbuffer.erase(0,1);
					LOG_INFO("corrupt length %d", len);
					return;
				}
				p1 = rxbuffer.find(">", strlen(ltypes[n]));
				if (p1 == std::string::npos) {
					LOG_INFO("incomplete header %s", rxbuffer.substr(0,15).c_str());
					if (rxbuffer.length() > 15)
						rxbuffer.erase(0,1);
					return;
				}
				if (rxbuffer.length() >= p1 + 1 + len) {
					if (rx_parse_line(n, crc, rxbuffer.substr(p1 + 1, len))) {
						rxbuffer.erase(0, p1 + 1 + len);
						return;
					} else {
						rxbuffer.erase(0,1);
						return;
					}
				}
			}
		}
	}
}

std::string cAmp::rx_stats()
{
	char number[10];
	std::string stats;
	stats.clear();
	stats.append("program:    ").append(rxprogname).append("\n");
	stats.append("filename:   ").append(rxfilename).append("\n");
	stats.append("datetime:   ").append(rxdttm).append("\n");
	stats.append("desc:       ").append(rxdesc).append("\n");
	stats.append("call info:  ").append(rxcall_info).append("\n");
	snprintf(number, sizeof(number), "%d", rxfilesize);
	stats.append("file size:  ").append(number).append("\n");
	snprintf(number, sizeof(number), "%d", rxnumblocks);
	stats.append("nbr blocks: ").append(number).append("\n");
	stats.append("file hash:  ").append(rxhash).append("\n");

	std::map<int, std::string>::iterator iter;
	for (iter = rxblocks.begin(); iter != rxblocks.end(); iter++) {
		snprintf(number, sizeof(number),"%d",iter->first);
		stats.append("Data block # ").append(number).append(" : ");
		stats.append(iter->second).append("\n");
	}
	return stats;
}

std::string cAmp::rx_missing()
{
	std::string missing;
	char number[10];
	missing.clear();
	for (int i = 1; i <= rxnumblocks; i++) {
		if (rxblocks.find(i) == rxblocks.end()) {
			snprintf(number, sizeof(number), "%d", i);
			if (missing.empty()) missing.append(number);
			else missing.append(", ").append(number);
		}
	}
	return missing;
}

std::string cAmp::rx_report()
{
	std::string temp;
	char number[10];
	std::string missing;
	temp.clear();
	temp.assign("{").append(rxhash).append("}");
	missing.clear();
	for (int i = 1; i <= rxnumblocks; i++) {
		if (rxblocks.find(i) == rxblocks.end()) {
			snprintf(number, sizeof(number), "%d ", i);
			missing.append(number);
		}
	}

	if (rx_crc_flags > 0 && missing.empty()) missing = "PREAMBLE";
	else if (missing.empty()) missing = "CONFIRMED";

	temp.append(missing);
	std::string report("<MISSING ");
	report.append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	report.append(">").append(temp).append(nuline);
	return report;
}

void cAmp::tx_parse_report(std::string s)
{
	// parse the incoming text stream for instances of
	// <MISSING nn CCCC>{cccc}n0 n1 n2 n3 ... nN
	// where nn is data field length
	//       CCCC is crc16 of data field
	//       cccc is crc16 of associated file
	//       n1...nN are missing block numbers
	// append each valid occurance to the tosend string

	static const char *sz_missing = "<MISSING ";
	static const char *sz_preamble = "PREAMBLE";

	report_buffer.append(s);

	size_t p = 0, p1 = 0;
	int len;
	char crc[5];
	string preamble_block;

	p = report_buffer.find(sz_missing);

	while (p != std::string::npos) {
		if (p > 0) report_buffer.erase(0, p);
		if (sscanf(report_buffer.c_str(), "<MISSING %d %4s>", &len, crc) == 2) {
			p1 = report_buffer.find(">", strlen(sz_missing));
			if (p1 != std::string::npos) {
				if (report_buffer.length() >= p1 + 1 + len) {
					string data = report_buffer.substr(p1 + 1, len);
					if (xmthash == data.substr(1,4)) {
						if (strcmp(crc, chksum.scrc16(data.c_str()).c_str()) == 0) {
							if(data.find(sz_preamble) != std::string::npos) {
								preamble_block.assign(" 1");
								preamble_detected_flag = true;
							} else {
								tosend.append(" ").append(data.substr(6));
								LOG_INFO("%s missing: %s", xmtfilename.c_str(), tosend.c_str());
							}
						}
					}
				}
			}
		}
		p = report_buffer.find(sz_missing, p + 2);
	}

	// convert the updated tosend string to a vector of integers
	// removing any duplicate values in the process
	string blocks = tosend;

	if(preamble_block.size()) {
		blocks.append(preamble_block);
	    preamble_block.clear();
	}

	int iblock;
	list<int> iblocks;
	list<int>::iterator pblock;
	bool insert_ok = false;
	while (!blocks.empty()) {
		if (sscanf(blocks.c_str(), "%d", &iblock) == 1) {
			insert_ok = true;
			for (pblock = iblocks.begin(); pblock != iblocks.end(); pblock++)
				if (*pblock == iblock) { insert_ok = false; break; }
			if (insert_ok) iblocks.push_back(iblock);
		}
		while (!blocks.empty() && isdigit(blocks[0]))
			blocks.erase(0,1);
		while (!blocks.empty() && !isdigit(blocks[0]))
			blocks.erase(0,1);
	}
	// sort the vector and then reassemble as a string sequence of comma
	// delimited values
	iblocks.sort();
	tosend.clear();
	char szblock[10];
	for (pblock = iblocks.begin(); pblock != iblocks.end(); pblock++) {
		snprintf(szblock, sizeof(szblock), "%d", *pblock);
		if (tosend.empty()) tosend.append(szblock);
		else tosend.append(",").append(szblock);
	}
}

