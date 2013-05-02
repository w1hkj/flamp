//======================================================================
// flmsg xml_io.cxx
//
// copyright 2012, W1HKJ
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

#include "flamp.h"
#include "flamp_dialog.h"
#include "xml_io.h"
#include "XmlRpc.h"
#include "status.h"
#include "debug.h"
#include "threads.h"

using namespace std;
using XmlRpc::XmlRpcValue;

static const double TIMEOUT = 1.0;

// these are get only
static const char* modem_get_name		= "modem.get_name";
static const char* modem_get_names		= "modem.get_names";

// these are set only
static const char* modem_set_by_name	= "modem.set_by_name";
static const char* text_clear_tx		= "text.clear_tx";
static const char* text_add_tx			= "text.add_tx";
static const char* text_get_rx			= "rx.get_data";
static const char* main_get_trx_state	= "main.get_trx_state";

static XmlRpc::XmlRpcClient* client;

#define XMLRPC_UPDATE_INTERVAL  200
#define XMLRPC_UPDATE_AFTER_WRITE 1000
#define XMLRPC_RETRY_INTERVAL 2000

//=====================================================================
// socket ops
//=====================================================================
int update_interval = XMLRPC_UPDATE_INTERVAL;

string xmlcall = "";

void open_xmlrpc()
{
	int server_port = atoi(progStatus.xmlrpc_port.c_str());
	client = new XmlRpc::XmlRpcClient(
									  progStatus.xmlrpc_addr.c_str(),
									  server_port );
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
	if (client)
		if (!client->execute(name, param, result, TIMEOUT))
			throw XmlRpc::XmlRpcException(name);
}

// --------------------------------------------------------------------
// send functions
// --------------------------------------------------------------------

void send_new_modem()
{
	pthread_mutex_lock(&mutex_xmlrpc);
	try {
		XmlRpcValue mode(cbo_modes->value()), res;
		execute(modem_set_by_name, mode, res);
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s", e.getMessage().c_str());
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
		LOG_ERROR("%s", e.getMessage().c_str());
	}
	update_interval = XMLRPC_UPDATE_AFTER_WRITE;
	pthread_mutex_unlock(&mutex_xmlrpc);
}

// --------------------------------------------------------------------
// receive functions
// --------------------------------------------------------------------

static void set_combo(void *str)
{
 	string s = (char *)str;
 	if (s != cbo_modes->value() && valid_mode_check(s)) {
 		cbo_modes->value(s.c_str());
		progStatus.selected_mode = cbo_modes->index();
 		estimate();
 	}
}

string get_trx_state()
{
	XmlRpcValue status;
	XmlRpcValue query;
	static string response;
	try {
		execute(main_get_trx_state, query, status);
		string resp = status;
		response = resp;
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s", e.getMessage().c_str());
		throw;
	}
	return response;
}

string get_rx_data()
{
	XmlRpcValue status;
	XmlRpcValue query;
	std::vector<char> response;
	static string report;
	try {
		execute(text_get_rx, query, status);
		response = status;
		report.clear();
		for (size_t n = 0; n < response.size(); n++)
			report += response[n];
		return report;
	} catch (const XmlRpc::XmlRpcException& e) {
		LOG_ERROR("%s", e.getMessage().c_str());
		throw;
	}
}

static void get_fldigi_modem()
{
	if (!progStatus.sync_mode_fldigi_flamp) return;
	
	XmlRpcValue status;
	XmlRpcValue query;
	static string response;
	try {
		execute(modem_get_name, query, status);
		string resp = status;
		response = resp;
		if (!response.empty()) {
			Fl::awake(set_combo, (void *)response.c_str());
		}
	} catch (const XmlRpc::XmlRpcException& e) {
		 throw;
	}
}

bool fldigi_online = false;
bool logerr = true;

static void get_fldigi_modems()
{
	XmlRpcValue status, query;
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
		 // throw;
	}
}

void * xmlrpc_loop(void *d)
{
	fldigi_online = false;
	for (;;) {
		pthread_mutex_lock(&mutex_xmlrpc);
		try {
			if (fldigi_online)
				get_fldigi_modem();
			else
				get_fldigi_modems();
		} catch (const XmlRpc::XmlRpcException& e) {
			if (logerr) {
				LOG_ERROR("%s", e.getMessage().c_str());
				logerr = false;
			}
			fldigi_online = false;
			update_interval = XMLRPC_RETRY_INTERVAL;
		}
		pthread_mutex_unlock(&mutex_xmlrpc);
		MilliSleep(update_interval);
		if (update_interval != XMLRPC_UPDATE_INTERVAL)
			update_interval = XMLRPC_UPDATE_INTERVAL;
	}
	return NULL;
}
