// =====================================================================
//
// link_io.cxx
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

#include "ax25_io.h"

/** ********************************************************
 *
 ***********************************************************/
Ax25_io::Ax25_io()
{
	_proto = PROTO_NONE;
	_tx_modem_data_ready = false;
	_tx_host_data_ready  = false;

	_from_callsign.clear();
	_to_callsign.clear();
	_via_path.clear();

}

/** ********************************************************
 *
 ***********************************************************/
Ax25_io::~Ax25_io()
{

}

/** ********************************************************
 * \brief Retreive data and sent to the modem.
 * \param data pointer to data storage. If passed as a null a
 * buffer will be allocated with 'new char[count+1]'. Which must
 * be release when it's no longer needed.
 * \param count The number of bytes in the passed buffer pointer
 * if 'data' not null or the number of bytes allocated by this 
 * routine.
 * \par NOTE:
 * If the buffer passed is to small to accomidate the data 
 * packet repeated calls are required until 'tx_data_ready()
 * returns false. The data is then appended to another buffer 
 * in the calling routine.
 ***********************************************************/
char * Ax25_io::tx_modem_data(char *data, size_t &count)
{
	return data;
}

/** ********************************************************
 * \brief Retreive data from the modem to be processed.
 * \param data buffer of the received data.
 * \param count number of bytes in the buffer.
 * \par NOTE:
 * This routine must be overwritten.
 ***********************************************************/
void Ax25_io::rx_modem_data(char *data, size_t &count)
{

}

/** ********************************************************
 * \brief Retreive data and sent to the host.
 * \param data pointer to data storage. If passed as a null a
 * buffer will be allocated with 'new char[count+1]'. Which must
 * be release when it's no longer needed.
 * \param count The number of bytes in the passed buffer pointer
 * if 'data' not null or the number of bytes allocated by this
 * routine.
 * \par NOTE:
 * If the buffer passed is to small to accomidate the data
 * packet repeated calls are required until 'tx_host_data_ready()
 * returns false. The data is then appended to another buffer
 * in the calling routine.
 ***********************************************************/
char * Ax25_io::tx_host_data(char *data, size_t &count)
{
	return data;
}

/** ********************************************************
 * \brief Retreive data from the host to be processed.
 * \param data buffer of the received data.
 * \param count number of bytes in the buffer.
 * \par NOTE:
 * This routine must be overwritten.
 ***********************************************************/
void Ax25_io::rx_host_data(char *data, size_t &count)
{

}
