//======================================================================
// flmsg xml_io.cxx
//
// copyright 2012, W1HKJ
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
// xmlrpc interface to fldigi
//
// fetches current list of modem types from fldigi
// fetches current modem in use in fldigi
// sets fldigi modem-by-name when required
//
//======================================================================

#include <stdio.h>

#include <cstdlib>
#include <string>
#include <vector>
#include <queue>

#include <iostream>
#include <errno.h>

#include "flamp.h"
#include "flamp_dialog.h"
#include "xml_io.h"
#include "XmlRpc.h"
#include "status.h"
#include "debug.h"
#include "threads.h"

using namespace std;
using XmlRpc::XmlRpcValue;

#define DEFAULT_XMLRPC_TIMEOUT 15.0
#define INIT_XMLRPC_TIMEOUT 5.0

double xmlrpc_timeout = INIT_XMLRPC_TIMEOUT;

// these are get only
static const char* modem_get_name		     = "modem.get_name";
static const char* modem_get_names		     = "modem.get_names";

// these are set only
static const char* modem_set_by_name	     = "modem.set_by_name";
static const char* text_clear_tx		     = "text.clear_tx";
static const char* text_add_tx			     = "text.add_tx";
static const char* text_clear_rx		     = "text.clear_rx";
static const char* main_get_trx_state	     = "main.get_trx_state";
static const char* main_tx				     = "main.tx";
static const char* main_tune			     = "main.tune";
static const char* main_rx				     = "main.rx";
static const char* main_abort			     = "main.abort";
static const char* main_get_rsid             = "main.get_rsid";
static const char* main_set_rsid             = "main.set_rsid";
static const char* main_get_char_rates       = "main.get_char_rates";
static const char* main_get_tx_timing        = "main.get_tx_timing";
static const char* main_get_char_timing      = "main.get_char_timing";
static const char* io_in_use                 = "io.in_use";
static const char* io_enable_arq             = "io.enable_arq";
static const char* io_enable_kiss            = "io.enable_kiss";

static XmlRpc::XmlRpcClient* client;

#define XMLRPC_UPDATE_INTERVAL  200
#define XMLRPC_UPDATE_AFTER_WRITE 1000
#define XMLRPC_RETRY_INTERVAL 2000

extern int errno;

//=====================================================================
// socket ops
//=====================================================================
int update_interval = XMLRPC_UPDATE_INTERVAL;

string xmlcall = "";

void open_xmlrpc()
{
	pthread_mutex_lock(&mutex_xmlrpc);

	string addr;
	string port;

	// Check if address/port passed via command line

	if(progStatus.user_xmlrpc_addr.size())
		addr.assign(progStatus.user_xmlrpc_addr);
	else
		addr.assign(progStatus.xmlrpc_addr);

	if(progStatus.user_xmlrpc_port.size())
		port.assign(progStatus.user_xmlrpc_port);
	else
		port.assign(progStatus.xmlrpc_port);

	int server_port = atoi(port.c_str());

	client = new XmlRpc::XmlRpcClient( addr.c_str(), server_port );

	pthread_mutex_unlock(&mutex_xmlrpc);

	//	XmlRpc::setVerbosity(5); // 0...5
}

void close_xmlrpc()
{
	pthread_mutex_lock(&mutex_xmlrpc);

	delete client;
	client = NULL;

	pthread_mutex_unlock(&mutex_xmlrpc);
}

static inline void execute(const char* name, const XmlRpcValue& param, XmlRpcValue& result)
{
	if (client) {
		if (!client->execute(name, param, result, xmlrpc_timeout)) {
			xmlrpc_errno = errno;

			if(client->isFault())
				LOG_DEBUG("Server fault response!");

			throw XmlRpc::XmlRpcException(name);
		}
	}
	xmlrpc_errno = errno;
}

void set_xmlrpc_timeout(double value)
{
	pthread_mutex_lock(&mutex_xmlrpc);
	if(value < DEFAULT_XMLRPC_TIMEOUT) return;
	xmlrpc_timeout = value;
	pthread_mutex_unlock(&mutex_xmlrpc);
}

void set_xmlrpc_timeout_default(void)
{
	xmlrpc_timeout = DEFAULT_XMLRPC_TIMEOUT;
}


// --------------------------------------------------------------------
// send functions
// --------------------------------------------------------------------
//extern std::string g_modem;

void send_new_modem(std::string modem)
{
	pthread_mutex_lock(&mutex_xmlrpc);

	//g_modem.assign(modem);

	try {
		XmlRpcValue mode(modem), res;
		execute(modem_set_by_name, mode, res);
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	update_interval = XMLRPC_UPDATE_AFTER_WRITE;
	pthread_mutex_unlock(&mutex_xmlrpc);
}

void send_clear_tx(void)
{
	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		XmlRpcValue res;
		execute(text_clear_tx, 0, res);
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	update_interval = XMLRPC_UPDATE_AFTER_WRITE;
	pthread_mutex_unlock(&mutex_xmlrpc);
}

void send_clear_rx(void)
{
	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		XmlRpcValue res;
		execute(text_clear_rx, 0, res);
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	update_interval = XMLRPC_UPDATE_AFTER_WRITE;
	pthread_mutex_unlock(&mutex_xmlrpc);
}

void send_report(string report)
{
	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		XmlRpcValue res, xml_str = report;
		execute(text_clear_tx, 0, res);
		execute(text_add_tx, xml_str, res);
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	update_interval = XMLRPC_UPDATE_AFTER_WRITE;
	pthread_mutex_unlock(&mutex_xmlrpc);
}

void send_tx(void)
{
	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		XmlRpcValue res;
		execute(main_tx, 0, res);
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	update_interval = XMLRPC_UPDATE_AFTER_WRITE;
	pthread_mutex_unlock(&mutex_xmlrpc);
}

void send_rx(void)
{
	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		XmlRpcValue res;
		execute(main_rx, 0, res);
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	update_interval = XMLRPC_UPDATE_AFTER_WRITE;
	pthread_mutex_unlock(&mutex_xmlrpc);
}

void set_rsid(void)
{
	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		XmlRpcValue res;
		execute(main_set_rsid, 0, res);
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	update_interval = XMLRPC_UPDATE_AFTER_WRITE;
	pthread_mutex_unlock(&mutex_xmlrpc);
}

void send_abort(void)
{
	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		XmlRpcValue res;
		execute(main_abort, 0, res);
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	update_interval = XMLRPC_UPDATE_AFTER_WRITE;
	pthread_mutex_unlock(&mutex_xmlrpc);
}

void send_tune(void)
{
	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		XmlRpcValue res;
		execute(main_tune, 0, res);
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	update_interval = XMLRPC_UPDATE_AFTER_WRITE;
	pthread_mutex_unlock(&mutex_xmlrpc);
}

std::string get_tx_timing(std::string data)
{
	XmlRpcValue status;
	XmlRpcValue xmlData((void *)data.c_str(), data.size());
	static string response;

	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		execute(main_get_tx_timing, xmlData, status);
		string resp = status;
		response = resp;
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	update_interval = XMLRPC_UPDATE_AFTER_WRITE;
	pthread_mutex_unlock(&mutex_xmlrpc);

	return response;
}

std::string get_char_timing(int character)
{
	pthread_mutex_lock(&mutex_xmlrpc);

	XmlRpcValue status;
	static string response;
	char buff[20];

	memset(buff, 0, sizeof(buff));

	snprintf(buff, sizeof(buff) - 1, "%d", character);

	string data;
	data.assign(buff);

	XmlRpcValue xmlData((void *) data.c_str(), data.size());

	try {
		execute(main_get_char_timing, xmlData, status);
		string resp = status;
		response = resp;
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}

	update_interval = XMLRPC_UPDATE_AFTER_WRITE * 2;
	pthread_mutex_unlock(&mutex_xmlrpc);

	return response;
}

void enable_arq(void)
{
	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		XmlRpcValue res;
		execute(io_enable_arq, 0, res);
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	update_interval = XMLRPC_UPDATE_AFTER_WRITE;
	pthread_mutex_unlock(&mutex_xmlrpc);
}

void enable_kiss(void)
{
	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		XmlRpcValue res;
		execute(io_enable_kiss, 0, res);
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	update_interval = XMLRPC_UPDATE_AFTER_WRITE;
	pthread_mutex_unlock(&mutex_xmlrpc);
}

// --------------------------------------------------------------------
// receive functions
// --------------------------------------------------------------------

string get_io_mode(void)
{
	XmlRpcValue status;
	XmlRpcValue query;
	static string response;

	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		execute(io_in_use, query, status);
		string resp = status;
		response = resp;
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
		response = "NIL";
	}
	pthread_mutex_unlock(&mutex_xmlrpc);

	xmlrpc_timeout = DEFAULT_XMLRPC_TIMEOUT;

	return response;
}

static void set_combo(void *str)
{
	string s = (char *)str;

	if(progStatus.use_header_modem != 0) return;

	if (s != cbo_modes->value() && valid_mode_check(s)) {
		cbo_modes->value(s.c_str());
		progStatus.selected_mode = cbo_modes->index();
		g_modem.assign(cbo_modes->value());
		cbo_modes->do_callback();
	}
}

string get_rsid_state(void)
{
	XmlRpcValue status;
	XmlRpcValue query;
	static string response;

	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		execute(main_get_rsid, query, status);
		string resp = status;
		response = resp;
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	pthread_mutex_unlock(&mutex_xmlrpc);

	return response;
}

string get_trx_state()
{
	XmlRpcValue status;
	XmlRpcValue query;
	static string response;

	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		execute(main_get_trx_state, query, status);
		string resp = status;
		response = resp;
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
		response = "ABORT";
	}
	pthread_mutex_unlock(&mutex_xmlrpc);

	return response;
}

string get_char_rates()
{
	XmlRpcValue status;
	XmlRpcValue query;
	static string response;

	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		execute(main_get_char_rates, query, status);
		string resp = status;
		response = resp;
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	pthread_mutex_unlock(&mutex_xmlrpc);

	return response;
}

static void get_fldigi_modem()
{
	if (!progStatus.sync_mode_fldigi_flamp) return;

	XmlRpcValue status;
	XmlRpcValue query;
	static string response;

	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		execute(modem_get_name, query, status);
		string resp = status;
		response = resp;
		if (!response.empty()) {
			Fl::awake(set_combo, (void *)response.c_str());
		}
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	pthread_mutex_unlock(&mutex_xmlrpc);
}

bool fldigi_online = false;
bool logerr = true;

static void get_fldigi_modems()
{
	XmlRpcValue status, query;

	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		string fldigi_modes("");
		execute(modem_get_names, query, status);
		for (int i = 0; i < status.size(); i++) {
			fldigi_modes.append((std::string)status[i]).append("|");
		}
		update_cbo_modes(fldigi_modes);
		fldigi_online = true;
		logerr = true;
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
	}
	pthread_mutex_unlock(&mutex_xmlrpc);
}

void * xmlrpc_loop(void *d)
{
	fldigi_online = false;
	for (;;) {
		try {
			if (fldigi_online)
				get_fldigi_modem();
			else
				get_fldigi_modems();
		} catch (const XmlRpc::XmlRpcException& e) {
			if (logerr) {
				LOG_ERROR("%s xmlrpc_errno = %d", e.getMessage().c_str(), xmlrpc_errno);
				logerr = false;
			}

			pthread_mutex_lock(&mutex_xmlrpc);
			fldigi_online = false;
			update_interval = XMLRPC_RETRY_INTERVAL;
			pthread_mutex_unlock(&mutex_xmlrpc);

		}

		MilliSleep(update_interval);

		pthread_mutex_lock(&mutex_xmlrpc);
		if (update_interval != XMLRPC_UPDATE_INTERVAL)
			update_interval = XMLRPC_UPDATE_INTERVAL;
		pthread_mutex_unlock(&mutex_xmlrpc);

	}
	return NULL;
}
