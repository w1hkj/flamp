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

#include "config.h"
#include "amp.h"
#include "global_amp.h"

/** ********************************************************
 *
 ***********************************************************/
cAmpGlobal::cAmpGlobal()
{
	amp = 0;
	amp_array.clear();
	locked = false;

	pthread_mutex_init(&mutex_amp, NULL);
}

/** ********************************************************
 *
 ***********************************************************/
cAmpGlobal::~cAmpGlobal()
{
	if(locked)
		unlock();
	pthread_mutex_destroy(&mutex_amp);
}

/** ********************************************************
 *
 ***********************************************************/
cAmp * cAmpGlobal::get_amp(void)
{
	lock();
	cAmp *ret = amp;
	unlock();
	return ret;
}

/** ********************************************************
 *
 ***********************************************************/
int cAmpGlobal::get_index(void)
{
	lock();
	int index = 0;
	int count = amp_array.size();
	int ret = 0;
	for(index = 0; index < count; index++) {
		if(amp == amp_array[index])
			ret = index + 1;
	}
	unlock();
	return ret;
}

/** ********************************************************
 *
 ***********************************************************/
bool cAmpGlobal::set(int pos)
{
	lock();
	int count = amp_array.size();
	int ret   = false;
	if((pos > count) || (pos < 1))
		amp = (cAmp *)0;
	else
		amp = amp_array[pos - 1];
	if(amp)
		ret = true;
	unlock();

	return ret;
}

/** ********************************************************
 *
 ***********************************************************/
bool cAmpGlobal::set(cAmp *src_amp)
{
	lock();
	bool ret = false;
	amp = src_amp;
	if(amp)
		ret = true;
	unlock();

	return ret;
}

/** ********************************************************
 *
 ***********************************************************/
int cAmpGlobal::amp2index(cAmp *src_amp)
{
	lock();
	int index = 0;
	int count = amp_array.size();
	int ret = 0;
	for(index = 0; index < count; index++) {
		if(src_amp == amp_array[index])
			ret = index + 1;
	}
	unlock();

	return ret;
}

/** ********************************************************
 *
 ***********************************************************/
cAmp *cAmpGlobal::index2amp(int pos)
{
	lock();
	cAmp *tmp = (cAmp *)0;
	int count = amp_array.size();
	if(pos > 0 && pos <= count) {
		pos--;
		tmp = amp_array[pos];
	}
	unlock();
	return tmp;
}

/** ********************************************************
 *
 ***********************************************************/
bool cAmpGlobal::add(cAmp *nu_amp)
{
	lock();
	bool ret = false;
	if(nu_amp) {
		amp_array.push_back(nu_amp);
		ret = true;
	}
	unlock();

	return ret;
}

/** ********************************************************
 *
 ***********************************************************/
bool cAmpGlobal::remove(cAmp *src_amp)
{
	lock();
	bool ret = false;

	if(src_amp) {
		int index = 0;
		int count = amp_array.size();


		for(index = 0; index < count; index++) {
			if(src_amp == amp_array[index]) {
				amp_array[index] = 0;
				amp_array.erase(amp_array.begin() + index);
				if(src_amp == amp) amp = 0;
				//if(src_amp) delete src_amp;
				ret = true;
			}
		}
	}

	unlock();
	return ret;
}

/** ********************************************************
 *
 ***********************************************************/
bool cAmpGlobal::remove(int pos)
{
	lock();
	int count = amp_array.size();
	bool ret = false;
	cAmp *tmp = (cAmp *)0;

	if(pos > 0 && pos <= count) {
		pos--;
		tmp = amp_array[pos];
		amp_array[pos] = 0;
		amp_array.erase(amp_array.begin() + pos);
		if(amp == tmp) amp = 0;
		if(tmp) delete tmp;
		ret = true;
	}
	unlock();
	return ret;
}

/** ********************************************************
 *
 ***********************************************************/
void cAmpGlobal::free(cAmp *tmp)
{
	lock();
	if(amp) delete amp;
	unlock();
}

/** ********************************************************
 *
 ***********************************************************/
void cAmpGlobal::free_all(void)
{
	lock();
	cAmp *amp = 0;
	int size = amp_array.size();
	for (int i = 0; i < size; i++) {
		amp = amp_array[i];
		if(amp)
			delete amp;
	}
	amp_array.clear();
	unlock();
}

/** ********************************************************
 *
 ***********************************************************/
size_t cAmpGlobal::size(void)
{
	lock();
	size_t count = (size_t) amp_array.size();
	unlock();
	return count;
}

