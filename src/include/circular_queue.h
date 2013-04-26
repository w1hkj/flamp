//
//  circularQueue.h
//
//  Author(s): Robert Stiles, KK5VD, Copyright (C) 2013
//             Dave Freese, W1HKJ, Copyright (C) 2013
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
// along with the program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef flamp_circular_queue_h
#define flamp_circular_queue_h

#include <pthread.h>
#include <string>
#include <exception>
#include <cstring>
#include <time.h>

// NOTE: this implementation of Circular_queue will ONLY process a SINGLE
//       instantiation!!

#define CQUE_HOLD   1
#define CQUE_RESUME 0
#define CQUE_SLEEP_TIME 10
#define STRING_MATCH_SIZE_LIMIT 16

#define MATCH_FOUND     1
#define MATCH_NOT_FOUND 0

#define TIME_SET 1
#define TIME_COUNT 2

extern 	void *circularQueueParser(void *que);

#ifndef HAVE_STRNLEN
#define strnlen(a,b) (strlen(a) > (b) ? (b) : strlen(a))
#endif

struct MATCH_STRING {
	char string[STRING_MATCH_SIZE_LIMIT+1];
	int  string_size;
	int  tag_number;
	uint32_t match;
	MATCH_STRING() {
		memset (string, 0, STRING_MATCH_SIZE_LIMIT+1);
		string_size = 0;
		tag_number = 0;
		match = 0;
	}
};

class CircQueException : public std::exception
{
public:
	CircQueException(int err_ = 0)     : err(err_), msg(err_to_str(err_)) { }
	CircQueException(const char* msg_) : err(1),    msg(msg_)             { }
	CircQueException(int err_, const std::string& prefix)
		: err(err_), msg(std::string(prefix).append(": ").append(err_to_str(err_))) { }
	virtual ~CircQueException() throw() { }
		const char*     what(void) const throw() { return msg.c_str(); }
		int             error(void) const { return err; }

protected:
	const char* err_to_str(int e) {
		return strerror(e);
	}
	int             err;
	std::string     msg;
};


class Circular_queue {
private:
	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t  condition;

	int read_index;
	int write_index;

	int inhibitDataOut;
	int stalled;
	int buffer_size;
	int index_mask;
	int exit_thread;
	int thread_running;
	int listCount;
	int bufferCount;
	int matchMaxLen;
	char *buffer;
	MATCH_STRING *patternMatchList;
	int  (* matchFound)(void *, char *tag);
	int  (* readData)(void *);

public:
	Circular_queue (
		int po2,
		const char *mList[], int mlCount,
		int (*matchFunc)(void *, char *),
		int (*readDataFrom)(void*));
	~Circular_queue();
private:
	void stringMatchingList(const char **mList, int mlCount);

public:
	void startDataOut();
	void stopDataOut();
	void addToQueue(char *_buffer, int _size);
	int  lookAheadCRC(char *_buffer, int _size, unsigned int *crcVal, int *reset);
	int  lookAhead(char *_buffer, int _size);
	int  readQueData(int buffer_count);
	int  adjustReadQueIndex(int count);
	int  lookAheadToTerminator(char *_buffer, char terminator, int maxLen);
	int  lookAheadForCharacter(char character, int *found);
	void stopQueue();
	void resumeQueue();
    int  thread_exit() { return exit_thread; }
    void sleep(int seconds, int milliseconds);
    void milliSleep(int milliseconds)
    {
        sleep(0, milliseconds);
    }
    void signal(void);
    
    bool timeOut(time_t &timeValue, time_t seconds, int attribute);

friend
	void *circularQueueParser(void *que);

};


#endif // flamp_circular_queue_h
