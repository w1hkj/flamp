//======================================================================
//	amp.cxx
//
//  Author(s):
//	Dave Freese, W1HKJ, Copyright (C) 2010, 2011, 2012, 2013
//	Robert Stiles, KK5VD, Copyright (C) 2013
//
// This file is part of FLAMP.
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
//
// =====================================================================

#include <list>

#include "config.h"

#include "amp.h"
#include "debug.h"

#define nuline "\n"

const char * cAmp::ltypes[] = {
	"<FILE ", "<ID ", "<DTTM ", "<SIZE ", "<DESC ", "<DATA ", "<PROG ", "<CNTL "
};

cAmp::cAmp(cAmp *src_amp)
{
	if(!src_amp) return;

	pthread_mutex_lock(&src_amp->mutex_amp_io);

	pthread_mutex_init(&mutex_amp_io, NULL);

	int index = 0;
	int count = 0;

	cAmp_type = src_amp->cAmp_type;

	//static const char *ltypes[];

	// transmit
	xmtfilename.assign(src_amp->xmtfilename);
	xmtbuffer.assign(src_amp->xmtbuffer);
	xmtdata.assign(src_amp->xmtdata);
	xmtstring.assign(src_amp->xmtstring);
	xmtdttm.assign(src_amp->xmtdttm);
	xmtdesc.assign(src_amp->xmtdesc);
	xmtcall.assign(src_amp->xmtcall);
	xmtinfo.assign(src_amp->xmtinfo);
	xmthash.assign(src_amp->xmthash);
	tosend.assign(src_amp->tosend);
	report_buffer.assign(src_amp->report_buffer);
    xmtbase.assign(src_amp->xmtbase);
    xmtunproto.assign(src_amp->xmtunproto);
	xmtfilename_fullpath.assign(src_amp->xmtfilename_fullpath);
	modem.assign(src_amp->modem);
	xmtcallto.assign(src_amp->xmtcallto);

	count = header_string_array.size();
	for(index = 0; index < count; index++) {
		header_string_array.push_back(src_amp->header_string_array[index]);
	}

	count = data_string_array.size();
	for(index = 0; index < count; index++) {
		data_string_array.push_back(src_amp->data_string_array[index]);
	}

	memcpy(&tx_statbuf, &src_amp->tx_statbuf, sizeof(struct stat));

	xmtnumblocks = src_amp->xmtnumblocks;
	xmtblocksize = src_amp->xmtblocksize;
	xmt_repeat = src_amp->xmt_repeat;
	repeat_header = src_amp->repeat_header;
	blocksize = src_amp->blocksize;
	fsize = src_amp->fsize;
    base_conversion_index = src_amp->base_conversion_index;

    use_compression = src_amp->use_compression;
	use_forced_compression = src_amp->use_forced_compression;
    preamble_detected_flag = src_amp->preamble_detected_flag;
    use_unproto = src_amp->use_unproto;


	Ccrc16 chksum = src_amp->chksum;

	// receive

	rxfilename.assign(src_amp->rxfilename);
	rxbuffer.assign(src_amp->rxbuffer);
	rxdata.assign(src_amp->rxdata);
	rxstring.assign(src_amp->rxstring);
	rxdttm.assign(src_amp->rxdttm);
	rxdesc.assign(src_amp->rxdesc);
	rxcall_info.assign(src_amp->rxcall_info);
	rxhash.assign(src_amp->rxhash);
	rxprogname.assign(src_amp->rxprogname);
	rx_rcvd.assign(src_amp->rx_rcvd);
	_rx_raw_file.assign(src_amp->_rx_raw_file);
	_rx_raw_id.assign(src_amp->_rx_raw_id);
	_rx_raw_size.assign(src_amp->_rx_raw_size);
	_rx_raw_desc.assign(src_amp->_rx_raw_desc);
	_rx_raw_prog.assign(src_amp->_rx_raw_prog);
	_rx_raw_cntl.assign(src_amp->_rx_raw_cntl);

	rxnumblocks  = src_amp->rxnumblocks;
	rxblocksize  = src_amp->rxblocksize;
	rxfilesize   = src_amp->rxfilesize;
	rx_ok_blocks = src_amp->rx_ok_blocks;
	rx_crc_flags = src_amp->rx_crc_flags;

	memcpy(temp_buffer, src_amp->temp_buffer, TEMP_BUFFER_SIZE + 1);

	rxblocks     = src_amp->rxblocks;
	rxDataHeader = src_amp->rxDataHeader;

	pthread_mutex_unlock(&src_amp->mutex_amp_io);
}

cAmp::cAmp(std::string str, std::string fname)
{
	pthread_mutex_init(&mutex_amp_io, NULL);

	xmtbuffer.assign(str);
	xmtfilename.assign(fname);
	xmtcall.clear();
	xmtinfo.clear();
	xmtdttm.clear();
	xmtstring.clear();
	xmthash.clear();
	tosend.clear();
	report_buffer.clear();
	memset(&tx_statbuf, 0, sizeof(tx_statbuf));

	use_compression = false;
	use_forced_compression = false;
	use_unproto = false;
	preamble_detected_flag = false;

	if (xmtfilename.empty()) xmtfilename.assign("UNKNOWN.txt");
	xmtblocksize = 64;
	xmtcallto = "QST";
	xmtbase = "base64";
	xmt_repeat = 1;
	repeat_header = 1;
	fsize = xmtdata.length();
	xmtnumblocks = xmtdata.length() / xmtblocksize + (xmtdata.length() % xmtblocksize ? 1 : 0);

	rx_crc_flags = ( FILE_CRC_FLAG | ID_CRC_FLAG | SIZE_CRC_FLAG );

	rxbuffer.clear();
	rxblocks.clear();
	rxDataHeader.clear();
	rxfilename.clear();
	rxdata.clear();
	rxstring.clear();
	rxdttm.clear();
	rxdesc.clear();
	rxcall_info.clear();
	rxhash.clear();
	_rx_raw_id.clear();
	_rx_raw_size.clear();
	_rx_raw_desc.clear();
	_rx_raw_prog.clear();
	_rx_raw_cntl.clear();

	rxnumblocks = rxblocksize = rxfilesize = rx_ok_blocks = 0;
	cAmp_type = 0;
}

cAmp::~cAmp()
{
	pthread_mutex_destroy(&mutex_amp_io);
}

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
	_rx_raw_id.clear();
	_rx_raw_size.clear();
	_rx_raw_desc.clear();
	_rx_raw_prog.clear();
	_rx_raw_cntl.clear();
}

std::string cAmp::file_hash()
{
	std::string chksumdata;
	std::string filename;
	char *buf = (char *)0;
	int bc = 32;

	chksumdata.clear();
	filename.clear();

	filename.assign(xmtdttm).append(":").append(xmtfilename);

	chksumdata.assign(filename);

	if(compress() || forced_compress())
		chksumdata.append("1");
	else
		chksumdata.append("0");

	chksumdata.append(tx_base_conv_str());

	buf = new char[bc];
	if(!buf)
		return string("");

	memset(buf, 0, bc);
	snprintf(buf, bc - 1, "%d", xmtblocksize);

	chksumdata.append(buf);
	xmthash = chksum.scrc16(chksumdata);

	delete [] buf;
	
	return xmthash;
}

std::string cAmp::program_header(void)
{
	std::string temp;
	std::string xmit;

	xmit.clear();
	temp.clear();

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

	xmit.clear();
	temp.clear();
	filename.clear();

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

	xmit.clear();
	temp.clear();

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

	xmit.clear();
	temp.clear();

	temp.assign("{").append(xmthash).append("}").append(xmtdesc);
	xmit.assign(ltypes[_DESC]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	xmit.append(">").append(temp).append(nuline);

	return xmit;
}

std::string cAmp::size_header(void)
{
	std::string temp;
	std::string xmit;

	xmit.clear();
	temp.clear();

	temp.assign("{").append(xmthash).append("}").append(sz_size());
	xmit.assign(ltypes[_SIZE]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	xmit.append(">").append(temp).append(nuline);

	return xmit;
}

std::string cAmp::data_block(int index)
{
	char *blknbr = (char *)0;
	int bc = 48;
	std::string temp;
	std::string xmit;
	std::string local_hash;

	xmit.clear();
	temp.clear();

	local_hash = file_hash();

	blknbr = new char[bc];
	if(!blknbr)
		return temp;

	memset(blknbr, 0, bc);
	snprintf(blknbr, bc - 1, "{%s:%d}", local_hash.c_str(), index);

	temp.assign(blknbr).append(xmtdata.substr((index - 1) * xmtblocksize, xmtblocksize));
	xmit.assign(ltypes[_DATA]).append(sz_len(temp)).append(" ");
	xmit.append(chksum.scrc16(temp)).append(">");
	xmit.append(temp).append(nuline);


	delete [] blknbr;
	return xmit;
}

std::string cAmp::data_eof(void)
{
	std::string temp;
	std::string xmit;

	xmit.clear();
	temp.clear();

	temp.assign("{").append(xmthash).append(":").append("EOF}");
	xmit.assign(ltypes[_CNTL]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	xmit.append(">").append(temp).append(nuline);

	return xmit;
}

std::string cAmp::data_eot(void)
{
	std::string temp;
	std::string xmit;

	xmit.clear();
	temp.clear();

	temp.assign("{").append(xmthash).append(":").append("EOT}");
	xmit.assign(ltypes[_CNTL]).append(sz_len(temp)).append(" ").append(chksum.scrc16(temp));
	xmit.append(">").append(temp).append(nuline);

	return xmit;
}

std::string cAmp::xmt_string() {

	pthread_mutex_lock(&mutex_amp_io);

	std::string temp;
	std::string fileline;
	std::string filename;
	std::string statsline;
	std::string idline;
	std::string fstring; // file strings
	std::string call_from_to;

	xmtstring.clear();
	temp.clear();
	fileline.clear();
	filename.clear();
	statsline.clear();
	idline.clear();
	fstring.clear(); // file strings
	call_from_to.clear();

	if(xmtcallto.empty())
		xmtcallto.assign("QST");

	call_from_to.assign(xmtcallto).append(" DE ").append(xmtcall).append("\n\n");

	xmtstring.assign(call_from_to);
	xmtstring.append(program_header());

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

	pthread_mutex_unlock(&mutex_amp_io);

	return xmtstring;
}

void cAmp::xmt_unproto(bool unproto_markers)
{
	size_t pos = 0;
	int appendFlag = 0;
	std::string temp;
	std::string call_from_to = "";
	int index = 0;

	const std::string cmdAppendMsg = "<_md>";

	temp.assign(xmtbuffer);

	if(isPlainText(temp) == false)
		convert_to_plain_text(temp);

	if(xmtcallto.empty())
		xmtcallto.assign("QST");

	call_from_to.assign(xmtcallto).append(" DE ").append(xmtcall).append("\n\n");

	pos = 0;
	do {
		pos = temp.find(sz_cmd, pos);
		if(pos != std::string::npos) {
			appendFlag |= CMD_FLAG;
			temp.replace(pos, cmdAppendMsg.size(), cmdAppendMsg);
			pos += 3;
		}
	} while(pos != std::string::npos);

	if(appendFlag)
		temp.append("\nNOTICE: Command Character Substitution!\n");

	if((appendFlag & CMD_FLAG) != 0) {
		temp.append(cmdAppendMsg).append(" <-> \'_\' = \'c\'\n");
	}

	temp.append("\n");

	xmtunproto.clear();

	for(index = 0; index < xmt_repeat; index++) {
		xmtunproto.append("\n").append(call_from_to);

		if(unproto_markers)
			xmtunproto.append("--- start ---\n");

		xmtunproto.append(temp);

		if(unproto_markers)
				xmtunproto.append("--- end ---\n");

		if(index > (xmt_repeat - 1))
			xmtunproto.append("\n").append(call_from_to);
	}
}

int cAmp::xmt_vector_string(bool header_modem, bool unproto_markers)
{
	pthread_mutex_lock(&mutex_amp_io);

	std::string call_from_to = "";
	std::string up_string;
	header_string_array.clear();
	data_string_array.clear();
	int j = 0;
	int i = 0;
	int no_of_elements = 0;

	if(xmtcallto.empty())
		xmtcallto.assign("QST");

	call_from_to.assign(xmtcallto).append(" DE ").append(xmtcall).append("\n\n");

	header_string_array.clear();
	data_string_array.clear();

	no_of_elements = (repeat_header * 6) + (xmt_repeat * xmtnumblocks) + 20;
	header_string_array.reserve((repeat_header * 6) + 10);
	data_string_array.reserve(no_of_elements);

	//no_of_elements = data_string_array.capacity();

	if(unproto() == true) {
		std::string call_from_to;
		int index = 0;
		int count = 0;
		int length = 0;
		int stride = xmtblocksize;

		xmt_unproto(unproto_markers);
		count = (int) xmtunproto.size();
		index = 0;
		data_string_array.push_back(call_from_to);
		
		do {
			up_string = xmtunproto.substr(index, stride);
			length = up_string.size();

			if(length > 0)
				data_string_array.push_back(up_string);

			if(length < stride)
				break;

			index += stride;

		} while(index < count);

	} else {
		if(header_modem) {
			header_string_array.push_back(call_from_to);

			for (i = 0; i < repeat_header; i++) {
				header_string_array.push_back(program_header());
				header_string_array.push_back(file_header());
				header_string_array.push_back(id_header());
				header_string_array.push_back(size_header());

				if (!xmtdesc.empty()) {
					header_string_array.push_back(desc_header());
				}
			}
		} else { // !progStatus.use_header_modem
			data_string_array.push_back(call_from_to);

			for (i = 0; i < repeat_header; i++) {
				data_string_array.push_back(program_header());
				data_string_array.push_back(file_header());
				data_string_array.push_back(id_header());
				data_string_array.push_back(size_header());

				if (!xmtdesc.empty()) {
					data_string_array.push_back(desc_header());
				}
			}
		}


		for (i = 0; i < xmt_repeat; i++) {
			if (tosend.empty()) {
				for (j = 0; j < xmtnumblocks; j++) {
					up_string = data_block(j + 1);
					data_string_array.push_back(up_string);
				}
			} else {
				std::string blocks;
				int bnbr;
				blocks.assign(tosend);
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
	}

	pthread_mutex_unlock(&mutex_amp_io);

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

		// Reassemble data header for relay operations
		memset(temp_buffer, 0, 48);
		std::string tmp_data = chksum.scrc16(data);
		int count = data.size();
		snprintf(temp_buffer, 47, "<DATA %d %s>%s", count, tmp_data.c_str(), data.substr(0, sp+1).c_str());
		rxDataHeader.insert(AMPmap::value_type(blknbr, std::string(temp_buffer)));

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
	int count = 0;
	std::string local_crc = "";
	int temp_buffer_local_size = 48;

	if(ltype != _DATA) {
		// Reassemble data for relay operations.
		if(temp_buffer_local_size > TEMP_BUFFER_SIZE)
			temp_buffer_local_size = TEMP_BUFFER_SIZE - 1;

		memset(temp_buffer, 0, temp_buffer_local_size);
		temp_buffer_local_size--;
		count = data.size();
		local_crc = chksum.scrc16(data);
	}

	switch (ltype) {
		case _FILE: rx_parse_dttm_filename(crc, data);
					snprintf(temp_buffer, temp_buffer_local_size, "<FILE %d %s>", count, local_crc.c_str());
					_rx_raw_file.assign(temp_buffer).append(data);
					break;

		case _DESC: rx_parse_desc(data);
					snprintf(temp_buffer, temp_buffer_local_size, "<DESC %d %s>", count, local_crc.c_str());
					_rx_raw_desc.assign(temp_buffer).append(data);
		            break;

		case _DATA: rx_add_data(data);
		            break;

		case _SIZE: rx_parse_size(data);
					snprintf(temp_buffer, temp_buffer_local_size, "<SIZE %d %s>", count, local_crc.c_str());
					_rx_raw_size.assign(temp_buffer).append(data);
		            break;

		case _PROG: rxprogname = rx_parse_hash_line(data);
					snprintf(temp_buffer, temp_buffer_local_size, "<PROG %d %s>", count, local_crc.c_str());
					_rx_raw_prog.assign(temp_buffer).append(data);
		            break;

		case _ID:   rx_parse_id(data);
					snprintf(temp_buffer, temp_buffer_local_size, "<ID %d %s>", count, local_crc.c_str());
					_rx_raw_id.assign(temp_buffer).append(data);
		            break;
		case _CNTL:
					snprintf(temp_buffer, temp_buffer_local_size, "<CNTL %d %s>", count, local_crc.c_str());
					_rx_raw_cntl.assign(temp_buffer).append(data);
		default:;
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

void cAmp::append_report(std::string s)
{
	report_buffer.append(s);
}

//void cAmp::tx_parse_report(std::string s)
void cAmp::tx_parse_report(void)
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

	size_t p = 0, p1 = 0;
	int len;
	char crc[5];
	string preamble_block;
	string data;

	p = report_buffer.find(sz_missing);

	while (p != std::string::npos) {
		if (p > 0) report_buffer.erase(0, p);
		if (sscanf(report_buffer.c_str(), "<MISSING %d %4s>", &len, crc) == 2) {
			p1 = report_buffer.find(">", strlen(sz_missing));
			if (p1 != std::string::npos) {
				if (report_buffer.length() >= p1 + 1 + len) {
					data.assign(report_buffer.substr(p1 + 1, len));
					if (xmthash == data.substr(1,4)) {
						if (strcmp(crc, chksum.scrc16(data.c_str()).c_str()) == 0) {
							if(data.find(sz_preamble) != std::string::npos) {
								preamble_block.assign(" 0");
								preamble_detected_flag = true;
							} else {
								tosend.append(" ").append(data.substr(6));
								if(cAmp_type == TX_AMP)
									LOG_INFO("%s missing: %s", xmtfilename.c_str(), tosend.c_str());
								else
									LOG_INFO("%s missing: %s", rxfilename.c_str(), tosend.c_str());

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

	tosend = reformat_missing_blocks(blocks);

}

std::string cAmp::reformat_missing_blocks(std::string &missing_blocks)
{
	std::string to_send_local;

	int iblock;
	list<int> iblocks;
	list<int>::iterator pblock;
	bool insert_ok = false;
	while (!missing_blocks.empty()) {
		if (sscanf(missing_blocks.c_str(), "%d", &iblock) == 1) {
			insert_ok = true;
			for (pblock = iblocks.begin(); pblock != iblocks.end(); pblock++)
				if (*pblock == iblock) { insert_ok = false; break; }
			if (insert_ok) iblocks.push_back(iblock);
		}
		while (!missing_blocks.empty() && isdigit(missing_blocks[0]))
			missing_blocks.erase(0,1);
		while (!missing_blocks.empty() && !isdigit(missing_blocks[0]))
			missing_blocks.erase(0,1);
	}
	// sort the vector and then reassemble as a string sequence of comma
	// delimited values
	iblocks.sort();
	to_send_local.clear();
	char szblock[10];
	for (pblock = iblocks.begin(); pblock != iblocks.end(); pblock++) {
		snprintf(szblock, sizeof(szblock), "%d", *pblock);
		if (to_send_local.empty()) to_send_local.append(szblock);
		else to_send_local.append(",").append(szblock);
	}

	return to_send_local;
}

std::string cAmp::tx_relay_string(std::string callfrom, std::string missing_blocks)
{
	AMPmap::iterator ihead;
	AMPmap::iterator idata;
	char *block_flags = (char *)0;
	int index = 0;
	int blknbr = 0;
	int count = 0;
	std::string blocks;
	std::string the_data;
	std::string callto_from;
	std::string temp_data;

	if(callfrom.empty()) {
		return xmtstring;
	}

	the_data.clear();
	xmtstring.clear();

	callto_from.assign("\nDE ").append(callfrom).append("\n");
	callto_from.append("\nFLAMP Relay\n\n");

	blocks = reformat_missing_blocks(missing_blocks);

	if(blocks.empty()) {
		if(_rx_raw_prog.size())
			the_data.append(_rx_raw_prog).append("\n");

		if(_rx_raw_file.size())
			the_data.append(_rx_raw_file).append("\n");

		if(_rx_raw_id.size())
			the_data.append(_rx_raw_id).append("\n");

		if(_rx_raw_size.size())
			the_data.append(_rx_raw_size).append("\n");

		if(_rx_raw_desc.size())
			the_data.append(_rx_raw_desc).append("\n");

		for (idata = rxblocks.begin(), ihead = rxDataHeader.begin();
			 idata != rxblocks.end(), ihead != rxDataHeader.end();
			 idata++, ihead++) {
			the_data.append(ihead->second);
			the_data.append(idata->second).append("\n");
		}

	} else {

		count = rxnumblocks;

		if(count < 1) {
			count = 0;
			for (idata = rxblocks.begin(); idata != rxblocks.end(); idata++) {
				if(idata->first > count)
					count = idata->first;
			}
		}

		block_flags = new char [count + 2];

		if(!block_flags)
			return xmtstring;

		memset(block_flags, 0, count + 2);

		while (!blocks.empty()) {
			if(sscanf(blocks.c_str(), "%d", &blknbr) == 1) {
				if((blknbr >= 0) && (blknbr <= count))
					block_flags[blknbr] = 1;
			}
			while (!blocks.empty() && isdigit(blocks[0]))
				blocks.erase(0,1);
			while (!blocks.empty() && !isdigit(blocks[0]))
				blocks.erase(0,1);
        }

		if(block_flags[0]) {
			if(_rx_raw_prog.size())
				the_data.append(_rx_raw_prog).append("\n");

			if(_rx_raw_file.size())
				the_data.append(_rx_raw_file).append("\n");

			if(_rx_raw_id.size())
				the_data.append(_rx_raw_id).append("\n");

			if(_rx_raw_size.size())
				the_data.append(_rx_raw_size).append("\n");

			if(_rx_raw_desc.size())
				the_data.append(_rx_raw_desc).append("\n");
		}

		for(index = 1; index < count; index++) {
			if(block_flags[index]) {
				idata = rxblocks.find(index);
				ihead = rxDataHeader.find(index);

				if(idata == rxblocks.end() || ihead == rxDataHeader.end())
					continue;

				the_data.append(ihead->second);
				the_data.append(idata->second).append("\n");
			}
		}

		delete [] block_flags;
	}

	if(the_data.size()) {
		char cntl[32];
		char tmp[48];
		std::string crc;

		memset(cntl, 0, sizeof(cntl));
		memset(tmp, 0, sizeof(tmp));

		snprintf(cntl, sizeof(cntl)-1, "{%s:EOF}", xmthash.c_str());
		temp_data.assign(cntl);
		crc = chksum.scrc16(temp_data);
		snprintf(tmp, sizeof(tmp)-1, "<CNTL %d %s>", (int) temp_data.size(), crc.c_str());
		the_data.append(tmp).append(temp_data).append("\n");

		snprintf(cntl, sizeof(cntl)-1, "{%s:EOT}", xmthash.c_str());
		temp_data.assign(cntl);
		crc = chksum.scrc16(temp_data);
		snprintf(tmp, sizeof(tmp)-1, "<CNTL %d %s>", (int) temp_data.size(), crc.c_str());
		the_data.append(tmp).append(temp_data).append("\n");
		
		the_data.append("\nDE ").append(callfrom).append(" K\n\n\n");

		xmtstring.assign(callto_from).append(the_data);
		
	}

	return xmtstring;
}

int cAmp::tx_relay_vector(std::string callfrom, std::string missing_blocks)
{
	AMPmap::iterator ihead;
	AMPmap::iterator idata;
	char *block_flags = (char *)0;
	int index = 0;
	int blknbr = 0;
	int count = 0;
	std::string blocks;
	std::string the_data;
	std::string callto_from;
	std::string temp_data;

	if(callfrom.empty()) {
		return 0;
	}

	header_string_array.clear();
	data_string_array.clear();

	callto_from.assign("\nDE ").append(callfrom).append("\n");
	callto_from.append("\nFLAMP Relay\n\n");
	data_string_array.push_back(callto_from);

	blocks = reformat_missing_blocks(missing_blocks);

	if(blocks.empty()) {
		if(_rx_raw_prog.size()) {
			the_data.assign(_rx_raw_prog).append("\n");
			data_string_array.push_back(the_data);
		}

		if(_rx_raw_file.size())  {
			the_data.assign(_rx_raw_file).append("\n");
			data_string_array.push_back(the_data);
		}

		if(_rx_raw_id.size())  {
			the_data.assign(_rx_raw_id).append("\n");
			data_string_array.push_back(the_data);
		}

		if(_rx_raw_size.size())  {
			the_data.assign(_rx_raw_size).append("\n");
			data_string_array.push_back(the_data);
		}

		if(_rx_raw_desc.size())  {
			the_data.assign(_rx_raw_desc).append("\n");
			data_string_array.push_back(the_data);
		}

		for (idata = rxblocks.begin(), ihead = rxDataHeader.begin();
			 idata != rxblocks.end(), ihead != rxDataHeader.end();
			 idata++, ihead++) {
			the_data.assign(ihead->second).append(idata->second).append("\n");
			data_string_array.push_back(the_data);
		}
	} else {

		count = rxnumblocks;

		if(count < 1) {
			count = 0;
			for (idata = rxblocks.begin(); idata != rxblocks.end(); idata++) {
				if(idata->first > count)
					count = idata->first;
			}
		}

		block_flags = new char [count + 2];

		if(!block_flags)
			return 0;

		memset(block_flags, 0, count + 2);

		while (!blocks.empty()) {
			if(sscanf(blocks.c_str(), "%d", &blknbr) == 1) {
				if((blknbr >= 0) && (blknbr <= count))
					block_flags[blknbr] = 1;
			}
			while (!blocks.empty() && isdigit(blocks[0]))
				blocks.erase(0,1);
			while (!blocks.empty() && !isdigit(blocks[0]))
				blocks.erase(0,1);
        }

		if(block_flags[0]) {
			if(_rx_raw_prog.size()) {
				the_data.assign(_rx_raw_prog).append("\n");
				data_string_array.push_back(the_data);
			}

			if(_rx_raw_file.size())  {
				the_data.assign(_rx_raw_file).append("\n");
				data_string_array.push_back(the_data);
			}

			if(_rx_raw_id.size())  {
				the_data.assign(_rx_raw_id).append("\n");
				data_string_array.push_back(the_data);
			}

			if(_rx_raw_size.size())  {
				the_data.assign(_rx_raw_size).append("\n");
				data_string_array.push_back(the_data);
			}

			if(_rx_raw_desc.size())  {
				the_data.assign(_rx_raw_desc).append("\n");
				data_string_array.push_back(the_data);
			}
		}

		for(index = 1; index < count; index++) {
			if(block_flags[index]) {
				idata = rxblocks.find(index);
				ihead = rxDataHeader.find(index);

				if(idata == rxblocks.end() || ihead == rxDataHeader.end())
					continue;
				the_data.assign(ihead->second).append(idata->second).append("\n");
				data_string_array.push_back(the_data);
			}
		}

		delete [] block_flags;
	}

	if(the_data.size()) {
		char cntl[32];
		char tmp[48];
		std::string crc;

		memset(cntl, 0, sizeof(cntl));
		memset(tmp, 0, sizeof(tmp));

		snprintf(cntl, sizeof(cntl)-1, "{%s:EOF}", xmthash.c_str());
		temp_data.assign(cntl);
		crc = chksum.scrc16(temp_data);
		snprintf(tmp, sizeof(tmp)-1, "<CNTL %d %s>", (int) temp_data.size(), crc.c_str());
		the_data.assign(tmp).append(temp_data).append("\n");
		data_string_array.push_back(the_data);

		snprintf(cntl, sizeof(cntl)-1, "{%s:EOT}", xmthash.c_str());
		temp_data.assign(cntl);
		crc = chksum.scrc16(temp_data);
		snprintf(tmp, sizeof(tmp)-1, "<CNTL %d %s>", (int) temp_data.size(), crc.c_str());
		the_data.assign(tmp).append(temp_data).append("\n");
		data_string_array.push_back(the_data);

		//the_data.assign("\nDE ").append(callfrom).append(" K\n\n\n");
		//data_string_array.push_back(the_data);

	}

	return data_string_array.size();
}

