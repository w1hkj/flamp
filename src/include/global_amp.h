//======================================================================
//	global_amp.cxx
//
//  Author(s):
//	Robert Stiles, KK5VD, Copyright (C) 2014
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

#ifndef __flamp_global_amp__
#define __flamp_global_amp__

//! Use to sync class cAmp global variables via a threaded mutex lock access.
class cAmpGlobal {

protected:
	cAmp *amp;                 //!< @brief Current selected receive cAmp pointer
	std::vector<cAmp *> amp_array;
	pthread_mutex_t mutex_amp; //!< @brief Mutex locks for pointer cAmp pointer access
	int locked;

public:
	cAmpGlobal();
	~cAmpGlobal();

	bool is_locked(void) { if(locked) return true; return false; }
	int lock_count(void) { return locked; }

	//! Access method for transmit cAmp pointer
	//! @param none (void)
	//! @return cAmp class pointer for current selected tx queue item.
	cAmp *get_amp(void);
	int get_index(void);

	int amp2index(cAmp *amp);
	cAmp *index2amp(int pos);

	//! Access method for transmit cAmp pointer.
	//! @param Set current selected TX cAmp ponter for global access.
	//! @return bool false if cAmp pointer NULL.
	bool set(cAmp *amp);
	bool set(int pos);

	//! Free allocaled cAmp memory.
	//! @param The cAmp pointer to be freed.
	//! @return none (void)
	void free(cAmp *amp);
	void free_all(void);

	bool add(cAmp *amp);
	bool remove(cAmp *amp);
	bool remove(int pos);
	size_t size(void);
	size_t count(void) { return size(); }

	void lock() {
		locked++;
		pthread_mutex_lock(&mutex_amp);
	}

	void unlock() {
		locked--;
		pthread_mutex_unlock(&mutex_amp);
	}
};


#endif /* defined(__flamp_global_amp__) */
