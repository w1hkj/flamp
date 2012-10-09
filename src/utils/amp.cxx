//======================================================================
// amp.cxx
//
//======================================================================

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

	if (xmtfilename.empty()) xmtfilename.assign("UNKNOWN.txt");
	xmtblocksize = 64;
	xmtdesc = "";
	xmt_repeat = 1;
	fsize = xmtdata.length();
	xmtnumblocks = xmtdata.length() / xmtblocksize + (xmtdata.length() % xmtblocksize ? 1 : 0);

	rxbuffer.clear();
	rxblocks.clear();
	rxfilename.clear();
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

std::string cAmp::xmt_string() {
	std::string temp;
	std::string fstring; // file strings
	xmtstring.clear();

	temp.assign(PACKAGE_NAME).append(" ").append(PACKAGE_VERSION);
	xmtstring.append(ltypes[_PROG]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	xmtstring.append(">").append(temp).append(nuline);

	temp.assign(xmtdttm).append(":").append(xmtfilename);
	xmthash = chksum.scrc16(temp);
	fstring.append(ltypes[_FILE]).append(sz_len(temp)).append(" ").append(xmthash);
	fstring.append(">").append(temp).append(nuline);

	temp.assign(xmtcall).append(" ").append(xmtinfo);
	fstring.append(ltypes[_ID]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	fstring.append(">").append(temp).append(nuline);

	temp.assign("{").append(xmthash).append("}").append(sz_size());
	fstring.append(ltypes[_SIZE]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	fstring.append(">").append(temp).append(nuline);

	temp.assign("{").append(xmthash).append("}").append(xmtdesc);
	if (!xmtdesc.empty()) {
		fstring.append(ltypes[_DESC]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
		fstring.append(">").append(temp).append(nuline);
	}

	char blknbr[20];
	if (tosend.empty()) {
		for (int i = 0; i < xmtnumblocks; i++) {
			snprintf(blknbr, sizeof(blknbr), "{%s:%d}", xmthash.c_str(), i+1);
			temp.assign(blknbr).append(xmtdata.substr(i*xmtblocksize, xmtblocksize));
			fstring.append(ltypes[_DATA]).append(sz_len(temp)).append(" ");
			fstring.append(chksum.scrc16(temp)).append(">");
			fstring.append(temp).append(nuline);
		}
	} else {
		string blocks = tosend;
		int bnbr;
		while (!blocks.empty()) {
			if (sscanf(blocks.c_str(), "%d", &bnbr) == 1) {
				if (bnbr > 0 && bnbr <= xmtnumblocks) {
					snprintf(blknbr, sizeof(blknbr), "{%s:%d}", xmthash.c_str(), bnbr);
					temp.assign(blknbr).append(xmtdata.substr((bnbr -1)* xmtblocksize, xmtblocksize));
					fstring.append(ltypes[_DATA]).append(sz_len(temp)).append(" ");
					fstring.append(chksum.scrc16(temp)).append(">");
					fstring.append(temp).append(nuline);
				}
			}
			while (!blocks.empty() && isdigit(blocks[0]))
				blocks.erase(0,1);
			while (!blocks.empty() && !isdigit(blocks[0]))
				blocks.erase(0,1);
		}
	}

	temp = "{EOF}";
	fstring.append(ltypes[_CNTL]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	fstring.append(">").append(temp).append(nuline);

	for (int i = 0; i < xmt_repeat; i++)
		xmtstring.append(fstring);

	temp = "{EOT}";
	xmtstring.append(ltypes[_CNTL]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	xmtstring.append(">").append(temp).append(nuline);

	return xmtstring;
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
	size_t pcolon = data.find(":");
	if (pcolon == std::string::npos) return;
	rxdttm = data.substr(0, pcolon);
	rxfilename = data.substr(pcolon + 1);
	rxhash = crc;
}

void cAmp::rx_parse_desc(string data)
{
	char hashval[5];
	if (sscanf(data.c_str(), "{%4s}*", hashval) != 1) return;
	if (rxhash != hashval) { 
		LOG_ERROR("%s", "not this file"); 
		return; 
	}
	size_t sp = data.find("}");
	if (sp == std::string::npos) return;
	rxdesc = data.substr(sp+1);
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
}

bool cAmp::rx_parse_line(int ltype, char *crc, std::string data)
{
	if (strcmp(crc, chksum.scrc16(data.c_str()).c_str()) == 0) {
		switch (ltype) {
			case _FILE : rx_parse_dttm_filename(crc, data); break;
			case _DESC : rx_parse_desc(data); break;
			case _DATA : rx_add_data(data); break;
			case _SIZE : rx_parse_size(data); break;
			case _PROG : rxprogname = data; break;
			case _ID   : rxcall_info = data; break; 
			case _CNTL :
			default : ;
		}
		return true;
	}
	LOG_ERROR("Checksum failed: %s\ndata: %s", crc, data.c_str());
	return false;
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
	temp.assign("{").append(rxhash).append("}");
	missing.clear();
	for (int i = 1; i <= rxnumblocks; i++) {
		if (rxblocks.find(i) == rxblocks.end()) {
			snprintf(number, sizeof(number), "%d ", i);
			missing.append(number);
		}
	}
	if (missing.empty()) missing = "CONFIRMED";
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
	report_buffer.assign(s);

	size_t p = 0, p1 = 0;
	int len;
	char crc[5];
	p = report_buffer.find(sz_missing);

	while (p != std::string::npos) {
		if (p > 0) report_buffer.erase(0, p);
		if (sscanf(report_buffer.substr(strlen(sz_missing)).c_str(), "%d %4s", &len, crc) == 2) {
			p1 = report_buffer.find(">", strlen(sz_missing));
			if (p1 != std::string::npos) {
				if (report_buffer.length() >= p1 + 1 + len) {
					string data = report_buffer.substr(p1 + 1, len);
					if (xmthash == data.substr(1,4)) {
						if (strcmp(crc, chksum.scrc16(data.c_str()).c_str()) == 0) {
							tosend.append(" ").append(data.substr(6));
							LOG_INFO("%s missing: %s", xmtfilename.c_str(), tosend.c_str());
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
