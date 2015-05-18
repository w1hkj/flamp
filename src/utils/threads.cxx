// ----------------------------------------------------------------------------
// threads.cxx
//
// Copyright (C) 2007-2009
//		Stelios Bounanos, M0GLD
//
// This file is part of fldigi.
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include "config.h"
#include <stdexcept>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include "threads.h"


// Synchronization objects.

guard_lock::guard_lock(pthread_mutex_t* m) : mutex(m)
{
	pthread_mutex_lock(mutex);
}

guard_lock::~guard_lock(void)
{
	pthread_mutex_unlock(mutex);
}

syncobj::syncobj()
{
	pthread_mutex_init( & m_mutex, NULL );
	pthread_cond_init( & m_cond, NULL );
}

syncobj::~syncobj()
{
	pthread_mutex_destroy( & m_mutex );
	pthread_cond_destroy( & m_cond );
}

void syncobj::signal()
{
	int rc = pthread_cond_signal( &m_cond );
	if( rc )
	{
		throw std::runtime_error(strerror(rc));
	}
}

bool syncobj::wait( double seconds )
{
	struct timespec		ts;
	struct timeval		tp;

	gettimeofday(&tp, NULL);

	ts.tv_nsec = 0;
	ts.tv_sec  = tp.tv_sec + seconds;

	int rc = pthread_cond_timedwait(&m_cond, &m_mutex, &ts);
	
	switch( rc )
	{
		case 0 : return true ;
		default : throw std::runtime_error(strerror(rc));
		case ETIMEDOUT: return false ;
	}
}



