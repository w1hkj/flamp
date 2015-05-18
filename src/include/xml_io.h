// =====================================================================
//
// xml_io.h
//
// Author: Dave Freese, W1HKJ
// Copyright: 2010 to 2014
//
// This file is part of FM_MSNGR.
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

#ifndef XML_IO_H
#define XML_IO_H


extern std::string get_char_rates(void);
extern std::string get_char_timing(int character);
extern std::string get_io_mode(void);
extern std::string get_rsid_state(void);
extern std::string get_rx_data();
extern std::string get_trx_state();
extern std::string get_tx_char_n_timing(int character, int count);
extern std::string get_tx_duration();
extern std::string get_tx_timing(std::string data);

extern void * xmlrpc_loop(void *d);
extern void close_xmlrpc();
extern void enable_arq(void);
extern void enable_kiss(void);
extern void open_xmlrpc();
extern void send_abort(void);
extern void send_clear_rx(void);
extern void send_clear_tx(void);
extern void send_new_modem(std::string modem);
extern void send_report(std::string report);
extern void send_rsid(void);
extern void send_rx(void);
extern void send_tune(void);
extern void send_tx(void);
extern void set_rsid(void);
extern void set_xmlrpc_timeout_default(void);
extern void set_xmlrpc_timeout(double value);

#endif
