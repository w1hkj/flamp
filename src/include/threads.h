#ifndef THREADS_H_
#define THREADS_H_

#include "config.h"

#include <pthread.h>
#include <stdint.h>

#include "fl_lock.h"

/// This ensures that a mutex is always unlocked when leaving a function or block.
class guard_lock
{
public:
	guard_lock(pthread_mutex_t* m);
	~guard_lock(void);
private:
	pthread_mutex_t* mutex;
};

// This wraps together a mutex and a condition variable which are used
/// together very often for queues etc...
class syncobj
{
	pthread_mutex_t m_mutex ;
	pthread_cond_t m_cond ;
public:
	syncobj();
	~syncobj();
	pthread_mutex_t * mtxp(void) { return & m_mutex; }
	void signal();
	bool wait( double seconds );
};

#endif // !THREADS_H_
