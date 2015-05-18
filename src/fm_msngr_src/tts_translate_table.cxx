// ----------------------------------------------------------------------------
// tts_translate_table.cxx
//
// Copyright (C) 2015  Robert Stiles, KK5VD
//
// This file is a part of Frame Messenger.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <errno.h>

#include "fm_msngr.h"
#include "tts_translate_table.h"
#include "gettext.h"
#include "debug.h"

static const char *default_hamspeak_table[] = {
	(char *) _("10M:10 Meter"),
	(char *) _("15M:15 Meter"),
	(char *) _("17M:17 Meter"),
	(char *) _("2M:2 Meter"),
	(char *) _("20M:20 Meter"),
	(char *) _("30M:30 Meter"),
	(char *) _("40M:40 Meter"),
	(char *) _("60M:60 Meter"),
	(char *) _("80M:80 Meter"),
	(char *) _("160M:160 Meter"),
	(char *) _("AA:All after"),
	(char *) _("AA:All after"),
	(char *) _("AA:All after"),
	(char *) _("AB:All before"),
	(char *) _("ABT:About"),
	(char *) _("ADR:Address"),
	(char *) _("AGN:Again"),
	(char *) _("ANT:Antenna"),
	(char *) _("AR: A R "),
	(char *) _("B4:Before"),
	(char *) _("BCI:Broadcast Interference"),
	(char *) _("BCL:Broadcast Listener"),
	(char *) _("BK: B K "),
	(char *) _("BN: B N "),
	(char *) _("BTU:back to you,"),
	(char *) _("BUG:Semi-Automatic key"),
	(char *) _("C:Yes"),
	(char *) _("CFM:Confirm"),
	(char *) _("CK:Ckeck"),
	(char *) _("CL:Going off the air"),
	(char *) _("CLD:Called"),
	(char *) _("CLG:Calling"),
	(char *) _("CQ: C Q, "),
	(char *) _("CU:See you"),
	(char *) _("CUL:See you later"),
	(char *) _("CW:Continuous wave"),
	(char *) _("DLD:Delivered"),
	(char *) _("DE: Dee E, "),
	(char *) _("DLVD:Delivered"),
	(char *) _("DR:Dear"),
	(char *) _("DX: D X "),
	(char *) _("ES:And"),
	(char *) _("FB:Fine Business"),
	(char *) _("FM:Frequency Modulation"),
	(char *) _("GA:Go ahead"),
	(char *) _("GLDX:Good luck D X"),
	(char *) _("GM:Good morning"),
	(char *) _("GN:Good night"),
	(char *) _("GND:Ground"),
	(char *) _("GUD:Good"),
	(char *) _("HI:laugh"),
	(char *) _("HR:Here"),
	(char *) _("HV:Have"),
	(char *) _("HW:How"),
	(char *) _("KN: K N "),
	(char *) _("LID:A poor operator"),
	(char *) _("MA:Millamperes"),
	(char *) _("MILS:Millamperes"),
	(char *) _("MSG:Message"),
	(char *) _("N:No"),
	(char *) _("NCS:Net Control Station"),
	(char *) _("ND:Nothing Doing"),
	(char *) _("NIL:Nothing"),
	(char *) _("NM:No more"),
	(char *) _("NR:Number"),
	(char *) _("NW:Now"),
	(char *) _("OB:Old boy"),
	(char *) _("OC:Old chap"),
	(char *) _("OM:Old man"),
	(char *) _("OP:Operator"),
	(char *) _("OPR:Operator"),
	(char *) _("OT: O T "),
	(char *) _("PBL:Preamble"),
	(char *) _("PSE:Please"),
	(char *) _("PWR:Power"),
	(char *) _("PX:Press"),
	(char *) _("QRG: Q R G "),
	(char *) _("QRH: Q R H "),
	(char *) _("QRI: Q R I "),
	(char *) _("QRJ: Q R J "),
	(char *) _("QRK: Q R K "),
	(char *) _("QRL: Q R L "),
	(char *) _("QRM: Q R M "),
	(char *) _("QRN: Q R N "),
	(char *) _("QRO: Q R O "),
	(char *) _("QRP: Q R P "),
	(char *) _("QRQ: Q R Q "),
	(char *) _("QRRR: Q R R R "),
	(char *) _("QRS: Q R S "),
	(char *) _("QRT: Q R T "),
	(char *) _("QRU: Q R U "),
	(char *) _("QRV: Q R V "),
	(char *) _("QRW: Q R W "),
	(char *) _("QRX: Q R X "),
	(char *) _("QRZ: Q R Z "),
	(char *) _("QSA: Q S A "),
	(char *) _("QSB: Q S B "),
	(char *) _("QSD: Q S D "),
	(char *) _("QSG: Q S G "),
	(char *) _("QSK: Q S K "),
	(char *) _("QSL: Q S L "),
	(char *) _("QSM: Q S M "),
	(char *) _("QSO: Q S O "),
	(char *) _("QSP: Q S P "),
	(char *) _("QST: Q S T "),
	(char *) _("QSV: Q S V "),
	(char *) _("QSX: Q S X "),
	(char *) _("QSY: Q S Y "),
	(char *) _("QSZ: Q S Z "),
	(char *) _("QTA: Q T A "),
	(char *) _("QTB: Q T B "),
	(char *) _("QTC: Q T C "),
	(char *) _("QTH: Q T H "),
	(char *) _("QTR: Q T R "),
	(char *) _("RCD:Received"),
	(char *) _("RCVR:Receiver"),
	(char *) _("REF:Reference"),
	(char *) _("RFI:Radio frequency interference"),
	(char *) _("RIG:Station equipment"),
	(char *) _("RTTY:Radio teletype"),
	(char *) _("RX:Receiver"),
	(char *) _("SASE:Self-addressed, stamped envelope"),
	(char *) _("SED:Said"),
	(char *) _("SIG: S I G "),
	(char *) _("SINE: S I N E "),
	(char *) _("SK:Silent Key"),
	(char *) _("SKED:Schedule"),
	(char *) _("SRI:Sorry"),
	(char *) _("SSB:Single Side Band"),
	(char *) _("SVC:Service"),
	(char *) _("T:Zero"),
	(char *) _("TFC:Traffic"),
	(char *) _("TKS:Thanks"),
	(char *) _("TMW:Tomorrow"),
	(char *) _("TNX:Thanks"),
	(char *) _("TT:That"),
	(char *) _("TU:Thank you"),
	(char *) _("TVI:Television interference"),
	(char *) _("TX:Transmitter"),
	(char *) _("TXT:Text"),
	(char *) _("UR:You're"),
	(char *) _("URS:Yours"),
	(char *) _("VFO:Variable Frequency Oscillator"),
	(char *) _("VY:Very"),
	(char *) _("WA:Word after"),
	(char *) _("WB:Word before"),
	(char *) _("WD:Word"),
	(char *) _("WDS:Words"),
	(char *) _("WKD:Worked"),
	(char *) _("WKG:Working"),
	(char *) _("WL: W L "),
	(char *) _("WUD:Would"),
	(char *) _("WX:Weather"),
	(char *) _("XCVR:Transceiver"),
	(char *) _("XMTR:Transmitter"),
	(char *) _("XTAL:Crystal"),
	(char *) _("XYL:Wife"),
	(char *) _("YL:Young lady"),
	(char *) 0
};

#define TR_BUF_SIZE 128

typedef struct _translator {
	char search_string[TR_BUF_SIZE];
	char replace_string[TR_BUF_SIZE];
	_translator() {
		memset(search_string, 0, TR_BUF_SIZE);
		memset(replace_string, 0, TR_BUF_SIZE);
	}
} TR_ARRAY;

static TR_ARRAY *_tr_array = (TR_ARRAY *)0;
static int number_of_table_entries = 0;

static std::string translate_filepath;
static bool table_loaded = false;

#define READ_BUFFER_SIZE 2048


/** **************************************************************
 * \brief Remove leading/trailing white spaces and quotes.
 * \param buffer Destination buffer
 * \param limit passed buffer size
 * \param upper_case_flag convert to upper case if set.
 * \return number of characters in the returned buffer.
 *****************************************************************/
int trim(char *buffer, size_t limit, bool upper_case_flag)
{
	char *s      = (char *)0;
	char *e      = (char *)0;
	char *dst    = (char *)0;
	size_t count = 0;

	if(!buffer || limit < 1) {
		return 0;
	}

	for(count = 0; count < limit; count++)
		if(buffer[count] == 0) break;

	if(count < 1) return 0;

	s = buffer;
	e = &buffer[count-1];

	for(size_t i = 0; i < count; i++) {
		if((*s <= ' ') || (*s == '"')) s++;
		else break;
	}

	while(e > s) {
		if((*e <= ' ') || (*e == '"'))
			*e = 0;
		else
			break;
		e--;
	}

	dst = buffer;

	if(upper_case_flag)
		for(; s <= e; s++) *dst++ = toupper(*s);
	else
		for(; s <= e; s++) *dst++ = *s;

	*dst = 0;

	return strnlen(buffer, READ_BUFFER_SIZE);
}

/** *******************************************************************
 * \brief Get the file path to the translation table.
 * \return std::string path
 **********************************************************************/
std::string get_translator_file_path(void)
{
	std::string base_path = HomeDir;
	int count = 0;
	int slash_type = 0;
	bool end_found = false;
	bool slash_flag = false;

	// Check to see if there is a slash or backslash aleady appeneded,
	// add if not.

#if _WIN32
	slash_type = '\\';
#else
	slash_type = '/';
#endif

	while(count > 0) {
		if((base_path[count] == '\\') || (base_path[count] == '/')) {
			slash_flag = true;
			break;
		}

		if(base_path[count] >= ' ') {
			end_found = true;
			break;
		}

		count--;
	}

	if(end_found && !slash_flag)
		base_path += slash_type;

	base_path.append(TRANSLATE_FILENAME);

	return base_path;
}

/** *******************************************************************
 * \brief Write the default table.
 * \param path write file path information.
 * \return FILE *fd for the file created.
 * Must close and reopen file in read mode for return value.
 **********************************************************************/
static FILE * create_default_table(std::string path)
{
	if(path.empty())
		return (FILE *)0;

	FILE *_fd = fopen(path.c_str(), "w");

	if(!_fd) {
		LOG_INFO("ErrNo: %d %s %s", errno, "Default hamspeak translation file write fail", path.c_str());
		return (FILE *)0;
	}

	int count = sizeof(default_hamspeak_table) / sizeof(char *);
	int index = 0;

	fprintf(_fd, "%s\n", TRANSLATE_FILE_TAG);
	fprintf(_fd, "%d\n", count);

	while(default_hamspeak_table[index]) {
		if(ferror(_fd)) {
			LOG_INFO("ErrNo: %d %s\n", errno, strerror(errno));
			break;
		}

		fprintf(_fd, "%s\n", default_hamspeak_table[index]);

		if(index++ >= count) break;
	}

	fclose(_fd);

	_fd = fopen(path.c_str(), "r");

	return _fd;
}

/** *******************************************************************
 * \brief Table entry parser.
 * \param line_no Current line being prcess in the file.
 * \param index Table index position.
 * \param count Maximum number of table entries.
 * \param read_buffer Data to parse.
 * \return bool false abort processing. true continue.
 **********************************************************************/
static bool add_to_table(int line_no, int &index, int max_count, char *read_buffer)
{
	if(!_tr_array) return false;
	if(!read_buffer) return false;
	if(index >= max_count) return false;

	static char search_string[TR_BUF_SIZE];
	static char replace_string[TR_BUF_SIZE];

	char *cPtr   = (char *)0;
	char *endPtr = (char *)0;
	bool seperator_flag = false;
	int cindx = 0;

	cPtr = read_buffer;
	endPtr = &read_buffer[READ_BUFFER_SIZE];

	while(cPtr < endPtr) {
		search_string[cindx] = *cPtr;

		if(*cPtr == ':') {
			cPtr++;
			seperator_flag = true;
			break;
		}

		if((*cPtr == '\n') || (*cPtr == '\r') || (*cPtr == 0)) {
			break;
		}

		if(cindx++ >= TR_BUF_SIZE) {
			LOG_INFO("Line No %d: Data exceeds internal storage size.", line_no);
			return true;
		}

		cPtr++;
	}

	if(!seperator_flag) {
		LOG_INFO("Line No %d: Missing ':' between match and replacement", line_no);
		return true;
	}

	cindx = trim(search_string, cindx, true);
	strncpy(_tr_array[index].search_string, search_string, cindx);

	cindx = 0;
	while(cPtr < endPtr) {
		replace_string[cindx] = *cPtr;

		if((*cPtr == '\n') || (*cPtr == '\r') || (*cPtr == 0)) {
			break;
		}

		if(cindx++ >= TR_BUF_SIZE) {
			LOG_INFO("Line No %d: Data exceeds internal storage size.", line_no);
			return true;
		}

		cPtr++;
	}

	cindx = trim(replace_string, cindx, false);
	strncpy(_tr_array[index].replace_string, replace_string, cindx);
	index++;

	return true;
}

/** *******************************************************************
 * \brief Read the data from a file.
 **********************************************************************/
static void load_table(void)
{
	FILE *fd = (FILE *)0;
	std::string path = get_translator_file_path();
	static char read_buffer[READ_BUFFER_SIZE];
	int read_count = READ_BUFFER_SIZE - 1;
	int line_no = 0;
	int index = 0;
	int count = 0;

	table_loaded = false;

	fd = fopen(path.c_str(), "r");

	if(!fd) {
		fd = create_default_table(path);
	}

	if(!fd) {
		table_loaded = false;
		LOG_INFO("%s", "Read Error - Translation Table");
		return;
	}

	memset(read_buffer, 0, READ_BUFFER_SIZE);
	fgets(read_buffer, read_count, fd);
	trim(read_buffer, read_count, true);
	line_no++;

	if(strncmp(read_buffer, TRANSLATE_FILE_TAG, sizeof(TRANSLATE_FILE_TAG))) {
		LOG_INFO("Line No %d: Tag for file incorrect: IS %s S/B %s", line_no, \
				 read_buffer, TRANSLATE_FILE_TAG);
		fclose(fd);
		return;
	}

	memset(read_buffer, 0, READ_BUFFER_SIZE);
	fgets(read_buffer, read_count, fd);
	trim(read_buffer, read_count, true);
	line_no++;

	count = (int) strtol(read_buffer, NULL, 10);

	if(count < 1) {
		LOG_INFO("Line No %d: Line Count not set.", line_no);
		fclose(fd);
		return;
	}

	if(count > MAX_TABLE_ENTIES) {
		LOG_INFO("%s", "Table size restricted to 32K entries");
		count = MAX_TABLE_ENTIES;
	}

	_tr_array = new TR_ARRAY[count];

	if(!_tr_array) {
		LOG_INFO("%s", "Internal memory allocation error. Hamspeak Translation not available.");
		fclose(fd);
		return;
	}

	index = 0;
	number_of_table_entries = 0;

	while(1) {
		if(ferror(fd)) {
			LOG_INFO("Errno %d: %s", errno, strerror(errno));
			break;
		}

		if(feof(fd))
			break;

		memset(read_buffer, 0, READ_BUFFER_SIZE);
		fgets(read_buffer, read_count, fd);
		line_no++;

		if(!add_to_table(line_no, index, count, read_buffer)) {
			fclose(fd);
			return;
		}

		number_of_table_entries++;
	}

	table_loaded = true;

	fclose(fd);
}

/** *******************************************************************
 * \brief Load the translation table from a file.
 * FORMAT:
 *
 *      First line: HAMSPEAK (FILE ID)
 *     Second Line: Number of entries in the file. Needed to set the
 *                  allocation size.
 * Remaining Lines:
 * <Search String> : <Translate String><LF>
 * Example:
 * QSO:Q S O
 * SK:Silent Key
 * TU:Thank You
 * BTU:Back To You
 **********************************************************************/
void translate_loader(void)
{
	if(!table_loaded) {
		load_table();
	}
}

/** *******************************************************************
 * \brief Release Allocated Memory.
 **********************************************************************/
void close_translate_hamspeak(void)
{
	if(_tr_array) {
		TR_ARRAY *tmp = _tr_array;
		_tr_array = (TR_ARRAY *) 0;
		delete [] tmp;
	}
}


/** *******************************************************************
 * \brief Translate possible ham radio callsign.
 * \param message possible hamradio callsign data storage.
 * \param translated_callsign storage if translated.
 * \return string matches a possible callsign and was translated (true)
 * otherwise false.
 **********************************************************************/
bool translate_callsign(std::string callsign, std::string &translated_callsign)
{
	bool numbers_flag = false;
	bool letters_flag = false;
	int count = callsign.length();
	int index = 0;
	std::string tmp = "";

	translated_callsign.clear();

	// Check for the signature.

	for(index = 0; index < count; index++) {

		if(callsign[index] >= '0' && callsign[index] <= '9') {
			numbers_flag = true;
			continue;
		}

		if(callsign[index] >= 'A' && callsign[index] <= 'Z') {
			letters_flag = true;
			continue;
		}

		if(callsign[index] >= 'a' && callsign[index] <= 'z') {
			letters_flag = true;
			continue;
		}

		// Any other characters should not be in the callsign
		return false;
	}

	// Does it look like a callsign?
	if(letters_flag && numbers_flag);
	else return false;

	// Add a space after each character so the TTS engine doesnt treat it like a word.
	for(index = 0; index < count; index++) {
		translated_callsign += callsign[index];
		translated_callsign += ' ';
	}

	return true;
}

/** *******************************************************************
 * \brief Translate a word if a match is found.
 * \param a_word Source word for translation
 * \param tmp translated word storage.
 * \return true = match found, otherwie false.
 **********************************************************************/
bool check_for_match(std::string a_word, std::string &tmp)
{
	int index = 0;
	int count = a_word.length();

	if(count < 1) return false;

	// Matches are done in uppercase charaters.
	for(index = 0; index < count; index++)
		a_word[index] = toupper(a_word[index]);

	for(index = 0; index < number_of_table_entries; index++) {
		if(a_word == _tr_array[index].search_string) {
			tmp.assign(_tr_array[index].replace_string);
			return true;
		}
	}

	// If a match is not found above, check for callsigns.
	return translate_callsign(a_word, tmp);
}

/** *******************************************************************
 * \brief Translate ham radio lingo for TTS.
 **********************************************************************/
void translate_hamspeak(std::string &message)
{
	if(!table_loaded) return;
	
	std::string translated_string = "";
	std::string a_word = "";
	std::string tmp = "";
	
	int index = 0;
	int count = message.length();
	
	while(index < count) {
		a_word.clear();
		
		while(index < count) {
			if(message[index] <= ' ') index++;
			else break;
		}
		
		while(index < count) {
			if(message[index] > ' ') {
				a_word += message[index];
				index++;
			}
			else break;
		}
		
		if(check_for_match(a_word, tmp)) {
			translated_string.append(tmp);
		} else {
			translated_string.append(a_word);
		}
		
		translated_string.append(" ");
	}
	
	message.assign(translated_string);
}
