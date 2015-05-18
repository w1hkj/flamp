// ----------------------------------------------------------------------------
// call_table.cxx
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

#ifndef __call_table__
#define __call_table__

#include <string>
#include <vector>
#include <list>
#include <time.h>
#include "threads.h"
#include "gettext.h"

// CSV Tag Names
#define CSV_TAG_COUNT 10

#define CALL_TAG          _("CALL")
#define NAME_TAG          _("NAME")
#define QTH_TAG           _("QTH")
#define STATE_TAG         _("STATE")
#define PROV_TAG          _("PROV")
#define LOC_TAG           _("LOC")
#define CHECK_IN_TAG      _("CHECK_IN")
#define CHECK_OUT_TAG     _("CHECK_OUT")
#define USER1_TAG         _("USER1")
#define USER2_TAG         _("USER2")

#define DATE_F 0x1
#define TIME_F 0x2
#define DAY_F  0x4
#define UTC_F  0x8

typedef enum {
	n_null = 0,
	n_char,
	n_short,
	n_int,
	n_long,
	n_bool,
	n_time_t,
	n_char_p,
	n_void_p
} DATA_TYPES;

typedef union {
	char d_char;
	short d_short;
	int d_int;
	long d_long;
	bool d_bool;
	const char *d_char_p;
	void *d_void_p;
	time_t d_time;
} UD;

typedef struct _data_ref {
	DATA_TYPES type;
	bool use;
	UD data;
} DATA_REF;


typedef enum _table_item_index {
	call_index = 0,
	name_index,
	qth_index,
	state_index,
	prov_index,
	loc_index,
	check_in_index,
	check_out_index,
	user1_index,
	user2_index,
	max_item_index
} CALL_INDEX;

#define MAX_CALL_LENGTH 32

typedef struct _call_table {
	double pos;
	int id;

	std::string call;
	std::string name;
	std::string qth;
	std::string state;
	std::string provence;
	std::string locator;
	std::string mstr;
	std::string user1;
	std::string user2;
	std::string text_log;

	bool logged_out;
	bool traffic;
	bool handled;
	bool qrz;

	time_t check_in_time;
	time_t check_out_time;

	_call_table() {
		pos = 0;
		id  = 0;

		call.clear();
		name.clear();
		qth.clear();
		state.clear();
		provence.clear();
		locator.clear();
		user1.clear();
		user2.clear();
		mstr.clear();
		text_log.clear();

		logged_out    = false;
		traffic       = false;
		qrz           = false;
		handled       = false;

		check_in_time  = 0;
		check_out_time = 0;

	}
} CALL_TABLE;

class call_table {

private:
	int current_index;
	int no_of_tabel_entires;
	std::vector<CALL_TABLE *> cTable;
	std::string current_call;

	pthread_mutex_t mutex_calldata;

	int _find_call_index(std::string _call);

public:
	void delete_handle(void * handle);
	void * handle(void);
	bool top(void * handle);
	bool next(void * handle);
	bool prev(void * handle);
	bool read(void * handle, CALL_TABLE * data);
	bool write(void * handle, CALL_TABLE * data);
	bool add(void * handle, CALL_TABLE * data);

	bool call(void *handle, std::string call);
	std::string call(void *handle);

	bool traffic(void *handle);
	void traffic(void *handle, bool set_state);

	time_t check_in_time(void *handle);
	void check_in_time(void *handle, time_t check_in_time);

	time_t check_out_time(void *handle);
	void check_out_time(void *handle, time_t check_out_time);

	bool handled(void *handle);
	void handled(void *handle, bool set_state);

	bool logged_out(void *handle);
	void logged_out(void *handle, bool set_state);

	bool qrz(void *handle);
	void qrz(void *handle, bool set_state);

	bool mstr(void *handle, std::string &mstr);

	bool find_call(void *handle, std::string call);
	bool find_id(void *handle, int id);

	bool update_log_from_fldigi(CALL_TABLE *list_item);
	bool update_log_from_flnet_via_fldigi(CALL_TABLE *list_item);

	int  add(std::string _call, double _pos);
	int  add(std::string _call, std::string _mstr, time_t check_in_time);

	int  find_call(std::string _call);
	bool find_id(int _id);

	bool is_logged_out(void);
	void is_logged_out(bool _flag);

	std::string call(void);
	bool call(std::string _call);

	double pos(void);
	void pos(double _pos);

	std::string mstr(void);
	void mstr(std::string _str);

	int id(void);
	inline bool check_handle(void * handle, const char *called_from);
	
	call_table();
	~call_table();

};

extern call_table *callTable;


#endif /* defined(__call_table__) */
