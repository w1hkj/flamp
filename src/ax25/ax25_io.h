// =====================================================================
//
// ax25_io.h
//
// Author(s):
//	Robert Stiles, KK5VD, Copyright (C) 2014
//
// This file is part of FLLNK.
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
#ifndef __fllnk_ax25_io__
#define __fllnk_ax25_io__

#include <iostream>

typedef enum {
    PROTO_NONE = 0,
	PROTO_AX25,
	PROTO_ARQ
} PROTO_TYPE;

class Ax25_io {
private:

	PROTO_TYPE _proto;
	bool _tx_modem_data_ready;
	bool _tx_host_data_ready;

	std::string _from_callsign;
	std::string _to_callsign;
	std::string _via_path;

	void proto(PROTO_TYPE value) { if(_proto == PROTO_NONE) _proto = value; }

	~Ax25_io();

public:
	Ax25_io();

	void from_callsign(std::string value) { _from_callsign.assign(value); }
	void to_callsign(std::string value) { _from_callsign.assign(value); }
	void via_path(std::string value) { _from_callsign.assign(value); }

	std::string from_callsign(void) { return _from_callsign; }
	std::string to_callsign(void) { return _from_callsign; }
	std::string via_path(void) { return _from_callsign; }

	virtual bool tx_modem_data_ready(void) {return _tx_modem_data_ready; }
    virtual char * tx_modem_data(char *data, size_t &count);

    virtual void rx_modem_data(char *data, size_t &count);

	virtual bool tx_host_data_ready(void) {return _tx_host_data_ready; }
	virtual char * tx_host_data(char *data, size_t &count);

    virtual void rx_host_data(char *data, size_t &count);

	PROTO_TYPE proto(void) { return _proto; }
};

#endif /* defined(__fllnk_link_io__) */
