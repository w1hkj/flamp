// =====================================================================
//
// status.h
//
// Author: Dave Freese, W1HKJ
// Copyright: 2010
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  It is
// copyright under the GNU General Public License.
//
// You should have received a copy of the GNU General Public License
// along with the program; if not, write to the Free Software
// Foundation, Inc.
// 59 Temple Place, Suite 330
// Boston, MA  02111-1307 USA
//
// =====================================================================

#ifndef _status_H
#define _status_H

#include <string>

using namespace std;

struct status {
	int		mainX;
	int		mainY;
	string my_call;
	string my_info;
	string socket_addr;
	string socket_port;
	string xmlrpc_addr;
	string xmlrpc_port;

	bool use_compression;
	int  encoder;
	int  selected_mode;

	int blocksize;
	int repeatNN;

	bool sync_mode_flamp_fldigi;
	bool sync_mode_fldigi_flamp;
	bool fldigi_xmt_mode_change;

	int repeat_every;
	bool repeat_at_times;
	string repeat_times;
	bool repeat_forever;

	void saveLastState();
	void loadLastState();
};

extern status progStatus;

#endif
