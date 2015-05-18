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

#include "fm_msngr_src/fm_port_io.h"
#include "fm_msngr_src/call_table.h"

#include "debug.h"
#include "util.h"
#include "gettext.h"
#include "status.h"

/** *******************************************************************
 * \brief Return a handle to the link list.
 * \return void
 **********************************************************************/
inline bool call_table::check_handle(void * handle, const char *called_from = (const char *) "")
{
	if(!handle) {
		LOG_INFO("func: %s: %s", called_from, _("Invalid handle."));
		return false;
	}

	int _idx = *((int *)handle);
	no_of_tabel_entires = cTable.size();

	if(_idx < 0 || _idx >= no_of_tabel_entires) {
		LOG_INFO("func: %s: %s", called_from, _("Invalid handle reference."));
		return false;
	}

	return true;
}

/** *******************************************************************
 * \brief Return a handle to the link list.
 * \return void
 **********************************************************************/
void call_table::delete_handle(void * handle)
{
	guard_lock calldata_lock(&mutex_calldata);
	if(!handle) return;
	free(handle);
}

/** *******************************************************************
 * \brief Return a handle to the link list.
 * \return void
 **********************************************************************/
void * call_table::handle(void)
{
	guard_lock calldata_lock(&mutex_calldata);

	int **_data_handle = (int **) 0;
	size_t size = (sizeof(int *) + sizeof(int)) * 2;

	_data_handle = (int **) malloc(size);

	if(!_data_handle) {
		LOG_INFO("%s", _("read_handle(): Handle create error."));
		return (void *)0;
	}

	memset(_data_handle, 0, size);
	return (void *) _data_handle;
}

/** *******************************************************************
 * \brief Point to the begining of the list for reading.
 * \return void
 **********************************************************************/
bool call_table::top(void *handle)
{
	guard_lock calldata_lock(&mutex_calldata);

	if(!handle) return false;
	if(cTable.empty()) return false;

	int *_data_handle = (int *) handle;
	int _idx = *_data_handle;

	_idx = 0;
	*_data_handle = _idx;

	return true;
}

/** *******************************************************************
 * \brief Move the list pointer to the next item.
 * \return void
 **********************************************************************/
bool call_table::prev(void *handle)
{
	guard_lock calldata_lock(&mutex_calldata);

	if(!handle) return false;
	int *_data_handle = (int *) handle;
	bool limit_reached = false;

	(*_data_handle)--;
	no_of_tabel_entires = cTable.size();
	if(*_data_handle >= no_of_tabel_entires) {
		*_data_handle = no_of_tabel_entires - 1;
		limit_reached = true;
	}

	if(*_data_handle < 0) {
		*_data_handle = 0;
		limit_reached = true;
	}

	if(limit_reached)
		return false;

	return true;
}

/** *******************************************************************
 * \brief Move the list pointer to the next item.
 * \return void
 **********************************************************************/
bool call_table::next(void *handle)
{
	guard_lock calldata_lock(&mutex_calldata);

	if(!handle) return false;
	int *_data_handle = (int *) handle;
	bool limit_reached = false;

	(*_data_handle)++;
	no_of_tabel_entires = cTable.size();
	if(*_data_handle >= no_of_tabel_entires) {
		*_data_handle = no_of_tabel_entires - 1;
		limit_reached = true;
	}

	if(*_data_handle < 0) {
		*_data_handle = 0;
		limit_reached = true;
	}

	if(limit_reached)
		return false;

	return true;
}

/** *******************************************************************
 * \brief Return the callsign of the selected item.
 * \return std::string callsign.
 **********************************************************************/
std::string call_table::call(void *handle)
{
	guard_lock calldata_lock(&mutex_calldata);
	if(!handle) return std::string("");

	int *_data_handle = (int *) handle;
	int _idx = *_data_handle;

	if((_idx > -1) && (_idx < no_of_tabel_entires)) {
		return cTable[_idx]->call;
	}

	return std::string("");
}

/** *******************************************************************
 * \brief Set the callsign of the currntly selected item.
 * \return bool true = changed, false = no change
 **********************************************************************/
bool call_table::call(void *handle, std::string _call)
{
	guard_lock calldata_lock(&mutex_calldata);
	int *_data_handle = (int *) handle;
	int _idx = *_data_handle;

	if((_idx > -1) && (_idx < no_of_tabel_entires)) {
		cTable[_idx]->call.assign(_call);
		return true;
	}

	return false;
}

/** *******************************************************************
 * \brief Set the state of logged
 * \param handle Handle for Data Access
 * \param set_state value to set traffic state.
 * \return void
 **********************************************************************/
void call_table::logged_out(void *handle, bool set_state)
{
	guard_lock calldata_lock(&mutex_calldata);

	int *_data_handle = (int *) handle;
	int _idx = 0;

	if(!check_handle(handle, __func__)) return;

	no_of_tabel_entires = cTable.size();
	_idx = *_data_handle;
	if((_idx > -1) && (_idx < no_of_tabel_entires))
		cTable[_idx]->logged_out = set_state;
}

/** *******************************************************************
 * \brief Return the state of logged out.
 * \param handle Handle for Data Access
 * \return void
 **********************************************************************/
bool call_table::logged_out(void *handle)
{
	guard_lock calldata_lock(&mutex_calldata);

	int *_data_handle = (int *) handle;
	int _idx = 0;

	if(!check_handle(handle, __func__)) return false;

	_idx = *_data_handle;
	if((_idx > -1) && (_idx < no_of_tabel_entires))
		return cTable[_idx]->logged_out;

	return false;
}


/** *******************************************************************
 * \brief Set the state of qrz
 * \param handle Handle for Data Access
 * \param set_state value to set traffic state.
 * \return void
 **********************************************************************/
void call_table::qrz(void *handle, bool set_state)
{
	guard_lock calldata_lock(&mutex_calldata);

	int *_data_handle = (int *) handle;
	int _idx = 0;

	if(!check_handle(handle, __func__)) return;

	no_of_tabel_entires = cTable.size();
	_idx = *_data_handle;
	if((_idx > -1) && (_idx < no_of_tabel_entires))
		cTable[_idx]->qrz = set_state;
}

/** *******************************************************************
 * \brief Return the state of qrz
 * \param handle Handle for Data Access
 * \return void
 **********************************************************************/
bool call_table::qrz(void *handle)
{
	guard_lock calldata_lock(&mutex_calldata);

	int *_data_handle = (int *) handle;
	int _idx = 0;

	if(!check_handle(handle, __func__)) return false;

	no_of_tabel_entires = cTable.size();
	_idx = *_data_handle;
	if((_idx > -1) && (_idx < no_of_tabel_entires))
		return cTable[_idx]->qrz;

	return false;
}

/** *******************************************************************
 * \brief Set the state of handled
 * \param handle Handle for Data Access
 * \param set_state value to set traffic state.
 * \return void
 **********************************************************************/
void call_table::handled(void *handle, bool set_state)
{
	guard_lock calldata_lock(&mutex_calldata);

	int *_data_handle = (int *) handle;
	int _idx = 0;

	if(!check_handle(handle, __func__)) return;

	no_of_tabel_entires = cTable.size();
	_idx = *_data_handle;
	if((_idx > -1) && (_idx < no_of_tabel_entires))
		cTable[_idx]->handled = set_state;
}

/** *******************************************************************
 * \brief Return the state of handled
 * \param handle Handle for Data Access
 * \return void
 **********************************************************************/
bool call_table::handled(void *handle)
{
	guard_lock calldata_lock(&mutex_calldata);

	int *_data_handle = (int *) handle;
	int _idx = 0;

	if(!check_handle(handle, __func__)) return false;

	_idx = *_data_handle;
	if((_idx > -1) && (_idx < no_of_tabel_entires))
		return cTable[_idx]->handled;

	return false;
}

/** *******************************************************************
 * \brief Return the state of traffic
 * \param handle Handle for Data Access
 * \return void
 **********************************************************************/
bool call_table::mstr(void *handle, std::string &mstr)
{
	guard_lock calldata_lock(&mutex_calldata);

	int *_data_handle = (int *) handle;
	int _idx = 0;

	if(!check_handle(handle, __func__)) return false;

	_idx = *_data_handle;
	if((_idx > -1) && (_idx < no_of_tabel_entires)) {
		mstr.assign(cTable[_idx]->mstr);
	}

	return true;
}


/** *******************************************************************
 * \brief Return the check in time.
 * \param handle Handle for Data Access
 * \return time_t time
 **********************************************************************/
time_t call_table::check_in_time(void *handle)
{
	guard_lock calldata_lock(&mutex_calldata);

	int *_data_handle = (int *) handle;
	int _idx = 0;

	if(!check_handle(handle, __func__)) return 0;

	_idx = *_data_handle;
	if((_idx > -1) && (_idx < no_of_tabel_entires))
		return cTable[_idx]->check_in_time;

	return 0;
}

/** *******************************************************************
 * \brief Set the check in time.
 * \param handle Handle for Data Access
 * \param set_check_in_time Value to Set
 * \return void
 **********************************************************************/
void call_table::check_in_time(void *handle, time_t set_check_in_time)
{
	guard_lock calldata_lock(&mutex_calldata);

	int *_data_handle = (int *) handle;
	int _idx = 0;

	if(!check_handle(handle, __func__)) return;

	_idx = *_data_handle;
	if((_idx > -1) && (_idx < no_of_tabel_entires))
		cTable[_idx]->check_in_time = set_check_in_time;
}

/** *******************************************************************
 * \brief Return the check in time.
 * \param handle Handle for Data Access
 * \return time_t time
 **********************************************************************/
time_t call_table::check_out_time(void *handle)
{
	guard_lock calldata_lock(&mutex_calldata);

	int *_data_handle = (int *) handle;
	int _idx = 0;

	if(!check_handle(handle, __func__)) return 0;

	_idx = *_data_handle;
	if((_idx > -1) && (_idx < no_of_tabel_entires))
		return cTable[_idx]->check_out_time;

	return 0;
}

/** *******************************************************************
 * \brief Set the check in time.
 * \param handle Handle for Data Access
 * \param set_check_in_time Value to Set
 * \return void
 **********************************************************************/
void call_table::check_out_time(void *handle, time_t set_check_out_time)
{
	guard_lock calldata_lock(&mutex_calldata);

	int *_data_handle = (int *) handle;
	int _idx = 0;

	if(!check_handle(handle, __func__)) return;

	_idx = *_data_handle;
	if((_idx > -1) && (_idx < no_of_tabel_entires))
		cTable[_idx]->check_out_time = set_check_out_time;
}

/** *******************************************************************
 * \brief Set the state of traffic
 * \param handle Handle for Data Access
 * \param set_state value to set traffic state.
 * \return void
 **********************************************************************/
void call_table::traffic(void *handle, bool set_state)
{
	guard_lock calldata_lock(&mutex_calldata);

	int *_data_handle = (int *) handle;
	int _idx = 0;

	if(!check_handle(handle, __func__)) return;

	_idx = *_data_handle;
	if((_idx > -1) && (_idx < no_of_tabel_entires))
		cTable[_idx]->traffic = set_state;
}

/** *******************************************************************
 * \brief Return the state of traffic
 * \param handle Handle for Data Access
 * \return void
 **********************************************************************/
bool call_table::traffic(void *handle)
{
	guard_lock calldata_lock(&mutex_calldata);

	int *_data_handle = (int *) handle;
	int _idx = 0;

	if(!check_handle(handle, __func__)) return false;

	_idx = *_data_handle;
	if((_idx > -1) && (_idx < no_of_tabel_entires))
		return cTable[_idx]->traffic;

	return false;
}

/** *******************************************************************
 * \brief Update table entry wholesale.
 * \return true sucess, false failure.
 **********************************************************************/
bool call_table::write(void * handle, CALL_TABLE * copy_from)
{
	if(!check_handle(handle, __func__)) return false;

	int *_data_handle = (int *) handle;
	int _idx = *_data_handle;

	if(!copy_from) {
		LOG_INFO("%s", _("Invalid copy_to pointer."));
		return false;
	}

	CALL_TABLE *copy_to = cTable[_idx];
	if(!copy_from) {
		LOG_INFO("%s", _("Invalid copy_from pointer."));
		return false;
	}

	copy_to->pos = copy_from->pos;
	copy_to->id  = copy_from->id;
	copy_to->call.assign(copy_from->call);
	copy_to->name.assign(copy_from->name);
	copy_to->qth.assign(copy_from->qth);
	copy_to->state.assign(copy_from->state);
	copy_to->provence.assign(copy_from->provence);
	copy_to->locator.assign(copy_from->locator);
	copy_to->user1 = copy_from->user1;
	copy_to->user2 = copy_from->user2;
	copy_to->text_log = copy_from->text_log;

	copy_to->logged_out = copy_from->logged_out;
	copy_to->traffic = copy_from->traffic;
	copy_to->handled = copy_from->handled;
	copy_to->qrz = copy_from->qrz;

	copy_to->check_in_time = copy_from->check_in_time;
	copy_to->check_out_time = copy_from->check_out_time;

	return true;
}

/** *******************************************************************
 * \brief Point to the begining of a link list for reading.
 * \return void
 **********************************************************************/
bool call_table::read(void *handle, CALL_TABLE *copy_to)
{
	guard_lock calldata_lock(&mutex_calldata);

	if(!check_handle(handle, __func__)) return false;

	int *_data_handle = (int *) handle;
	int _idx = *_data_handle;

	if(!copy_to) {
		LOG_INFO("%s", _("Invalid copy_to pointer."));
		return false;
	}

	CALL_TABLE *copy_from = cTable[_idx];
	if(!copy_from) {
		LOG_INFO("%s", _("Invalid copy_from pointer."));
		return false;
	}

	copy_to->pos = copy_from->pos;
	copy_to->id  = copy_from->id;
	copy_to->call.assign(copy_from->call);
	copy_to->name.assign(copy_from->name);
	copy_to->qth.assign(copy_from->qth);
	copy_to->state.assign(copy_from->state);
	copy_to->provence.assign(copy_from->provence);
	copy_to->locator.assign(copy_from->locator);
	copy_to->user1 = copy_from->user1;
	copy_to->user2 = copy_from->user2;
	copy_to->text_log = copy_from->text_log;

	copy_to->logged_out = copy_from->logged_out;
	copy_to->traffic = copy_from->traffic;
	copy_to->handled = copy_from->handled;
	copy_to->qrz = copy_from->qrz;

	copy_to->check_in_time = copy_from->check_in_time;
	copy_to->check_out_time = copy_from->check_out_time;

	return true;
}

/** *******************************************************************
 * \brief Use xmlprc to quiry FLDIGI for Callbok Information
 * \param list_item Pointer to assign data to.
 * \return 0 (fail) other item id number.
 **********************************************************************/
bool call_table::update_log_from_fldigi(CALL_TABLE *list_item)
{

	return false;
}

/** *******************************************************************
 * \brief Use xmlprc to quiry FLNET via FLDIGI for Callbok Information
 * \param list_item Pointer to assign data to.
 * \return 0 (fail) other item id number.
 **********************************************************************/
bool call_table::update_log_from_flnet_via_fldigi(CALL_TABLE *list_item)
{
	return false;
}

/** *******************************************************************
 * \brief Add an entry to the table.
 * \param handle Index reference handle.
 * \param copy_from data to copy from.
 * \return true success, false failure.
 **********************************************************************/
bool call_table::add(void * handle, CALL_TABLE *copy_from)
{
	guard_lock calldata_lock(&mutex_calldata);

	if(!handle || !copy_from) return false;

	int _idx = 0;
	bool new_flag = true;
	int *handle_ref = (int *)handle;

	if(copy_from->call.size() && cTable.size()) {
		_idx = _find_call_index(copy_from->call);
		if(_idx > -1 && _idx < cTable.size()) {
			new_flag = false;
			*handle_ref = _idx;
		}
	}

	CALL_TABLE *copy_to = (CALL_TABLE *)0;

	if(new_flag)
		copy_to = new CALL_TABLE;
	else
		copy_to = cTable[_idx];

	if(copy_to) {
		copy_to->name.assign(copy_from->name);
		copy_to->qth.assign(copy_from->qth);
		copy_to->state.assign(copy_from->state);
		copy_to->provence.assign(copy_from->provence);
		copy_to->locator.assign(copy_from->locator);
		copy_to->user1 = copy_from->user1;
		copy_to->user2 = copy_from->user2;
		copy_to->text_log = copy_from->text_log;
		copy_to->traffic = copy_from->traffic;
		copy_to->handled = copy_from->handled;
	}

	if(new_flag) {
		cTable.push_back(copy_to);
		no_of_tabel_entires = cTable.size();
		copy_to->id = no_of_tabel_entires;
		*handle_ref = copy_to->pos = no_of_tabel_entires - 1;
		copy_to->call.assign(copy_from->call);
		copy_to->check_in_time = copy_from->check_in_time;
	}

	return true;
}

/** *******************************************************************
 * \brief Add an entry to the table.
 * \param _call the callsign to add.
 * \param _text cursor position of the most recent entry.
 * \return < 0 (fail) other item id number.
 **********************************************************************/
int call_table::add(std::string _call, std::string _mstr, time_t check_in_time)
{
	guard_lock calldata_lock(&mutex_calldata);

	if(cTable.size() > 0) {
		int _idx = _find_call_index(_call);
		if(_idx > -1 && _idx < cTable.size()) {
			cTable[_idx]->mstr.assign(_mstr);
			return cTable[_idx]->id;
		}
	}

	for(int i = 0; i < _call.size(); i++)
		_call[i] = toupper(_call[i]);

	CALL_TABLE *_newCall = (CALL_TABLE *)0;

	_newCall = new CALL_TABLE;

	if(!_newCall) return -1;

	_newCall->call.assign(_call);
	_newCall->mstr.assign(_mstr);
	_newCall->check_in_time = check_in_time;

	cTable.push_back(_newCall);
	no_of_tabel_entires = cTable.size();
	_newCall->id = no_of_tabel_entires;
	_newCall->pos = no_of_tabel_entires - 1;
	return no_of_tabel_entires;
}

/** *******************************************************************
 * \brief Search for callsign in table, using mutex lock.
 * \param handle Used for data access.
 * \param _call the callsign to search for
 * \return true found (handle reference updated). false otherwise.
 **********************************************************************/
bool call_table::find_call(void *handle, std::string _call)
{
	guard_lock calldata_lock(&mutex_calldata);

	if(_call.empty()) return false;
	no_of_tabel_entires = cTable.size();
	if(no_of_tabel_entires < 1) return false;

	if(!handle) {
		LOG_INFO("%s", _("Invalid handle."));
		return false;
	}

	int *_data_handle = (int *) handle;

	for(int i = 0; i < _call.size(); i++)
		_call[i] = toupper(_call[i]);

	int index = 0;
	for(index = 0; index < no_of_tabel_entires; index++) {
		if(cTable[index]) {
			if(!cTable[index]->call.empty()) {
				if(_call == cTable[index]->call) {
					*_data_handle = index;
					return true;
				}
			}
		}
	}

	return false;
}

/** *******************************************************************
 * \brief Search for ID number in table, using mutex lock.
 * \param handle Used for data access.
 * \param id the number to search for.
 * \return true found (handle reference updated). false otherwise.
 **********************************************************************/
bool call_table::find_id(void *handle, int _id)
{
	guard_lock calldata_lock(&mutex_calldata);

	if(_id < 0) return false;
	if(cTable.size() < 1) return false;

	if(!handle) {
		LOG_INFO("%s", _("Invalid handle."));
		return false;
	}

	int *_data_handle = (int *) handle;

	if(!*_data_handle) {
		LOG_INFO("%s", _("Invalid handle reference."));
		return false;
	}

	no_of_tabel_entires = cTable.size();

	if((_id > 0) && (_id <= no_of_tabel_entires))
		return true;

	return false;
}

/** *******************************************************************
 * \brief Search for callsign in table, using mutex lock.
 * \param _call the callsign to search for
 * \return item id
 **********************************************************************/
int call_table::find_call(std::string _call)
{
	guard_lock calldata_lock(&mutex_calldata);
	return cTable[_find_call_index(_call)]->id;
}

/** *******************************************************************
 * \brief Search for callsign in table, sans mutex lock.
 * \param _call the callsign to search for
 * \return index position (not id number)
 **********************************************************************/
int call_table::_find_call_index(std::string _call)
{
	if(_call.empty())  return 0;
	if(cTable.empty()) return 0;

	no_of_tabel_entires = cTable.size();

	if((current_index > -1) && (current_index < no_of_tabel_entires)) {
		if(!cTable[current_index]->call.empty())
			if(_call == cTable[current_index]->call)
				return current_index;
	}

	int index = 0;
	for(index = 0; index < no_of_tabel_entires; index++) {
		if(cTable[index]) {
			if(!cTable[index]->call.empty()) {
				if(_call == cTable[index]->call) {
					return index;
				}
			}
		}
	}

	return -1;
}

/** *******************************************************************
 * \brief Search for id in table
 * \param _call the callsign to search for
 * \return item id
 **********************************************************************/
bool call_table::find_id(int _id)
{
	guard_lock calldata_lock(&mutex_calldata);

	if((_id > 0) && (_id <= no_of_tabel_entires)) {
		return true;
	}

	return false;
}

/** *******************************************************************
 * \brief return if logged state of the currntly selected item.
 * \return item id
 **********************************************************************/
bool call_table::is_logged_out(void)
{
	guard_lock calldata_lock(&mutex_calldata);

	if((current_index > -1) && (current_index < no_of_tabel_entires)) {
		return cTable[current_index]->logged_out;
	}

	return false;
}

/** *******************************************************************
 * \brief Set the logged state of the currntly selected item.
 * \param _flag the state to set.
 * \return void
 **********************************************************************/
void call_table::is_logged_out(bool _flag)
{
	guard_lock calldata_lock(&mutex_calldata);

	if((current_index > -1) && (current_index < no_of_tabel_entires)) {
		cTable[current_index]->logged_out = _flag;
	}
}

/** *******************************************************************
 * \brief Return the callsign of the selected item.
 * \return std::string callsign.
 **********************************************************************/
std::string call_table::call(void)
{
	guard_lock calldata_lock(&mutex_calldata);

	if((current_index > -1) && (current_index < no_of_tabel_entires)) {
		return cTable[current_index]->call;
	}

	return std::string("");
}

/** *******************************************************************
 * \brief Set the callsign of the currntly selected item.
 * \return bool true = changed, false = no change
 **********************************************************************/
bool call_table::call(std::string _call)
{
	guard_lock calldata_lock(&mutex_calldata);

	if((current_index > -1) && (current_index < no_of_tabel_entires)) {
		cTable[current_index]->call.assign(_call);
	}
	return false;
}

/** *******************************************************************
 * \brief Return the cursor position of the most recent position on the
 * currenlty select item.
 **********************************************************************/
double call_table::pos(void)
{
	guard_lock calldata_lock(&mutex_calldata);

	if((current_index > -1) && (current_index < no_of_tabel_entires)) {
		return cTable[current_index]->pos;
	}

	return -1;
}

/** *******************************************************************
 * \brief Set the cursor position of the currntly selected item.
 * \return void
 **********************************************************************/
void call_table::pos(double _pos)
{
	guard_lock calldata_lock(&mutex_calldata);

	if((current_index > -1) && (current_index < no_of_tabel_entires)) {
		cTable[current_index]->pos = _pos;
	}
}

/** *******************************************************************
 * \brief Return the ID number for the associated callsign.
 * \return void
 **********************************************************************/
int call_table::id(void)
{
	guard_lock calldata_lock(&mutex_calldata);

	if((current_index > -1) && (current_index < no_of_tabel_entires)) {
		return cTable[current_index]->id;
	}

	return 0;
}

/** *******************************************************************
 * \brief Return the search string associated with the callsign
 * \return void
 **********************************************************************/
std::string call_table::mstr(void)
{
	guard_lock calldata_lock(&mutex_calldata);

	if((current_index > -1) && (current_index < no_of_tabel_entires)) {
		return cTable[current_index]->mstr;
	}

	return std::string("");
}

/** *******************************************************************
 * \brief Set the search string associated with the callsign
 * \return void
 **********************************************************************/
void call_table::mstr(std::string _str)
{
	guard_lock calldata_lock(&mutex_calldata);

	if(_str.empty()) return;
	if((current_index > -1) && (current_index < no_of_tabel_entires)) {
		cTable[current_index]->mstr.assign(_str);
	}
}

/** *******************************************************************
 * \brief Allocate/Initialize call table.
 **********************************************************************/
call_table::call_table()
{
    pthread_mutex_init(&mutex_calldata, (const pthread_mutexattr_t *)0);

	no_of_tabel_entires = 0;
	current_index = -1;
	current_call.clear();
}

/** *******************************************************************
 * \brief Deallocate call table data.
 **********************************************************************/
call_table::~call_table()
{
	{
		guard_lock calldata_lock(&mutex_calldata);

		int index = 0;
		int count = cTable.size();

		for(index = 0; index < count; index++) {
			if(cTable[index])
				delete cTable[index];
		}

		no_of_tabel_entires = 0;
		current_index = -1;
		cTable.clear();
	}

    pthread_mutex_destroy(&mutex_calldata);
}
