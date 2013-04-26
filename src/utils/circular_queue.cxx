// circular_queue.cxx
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>

#include "util.h"
#include "circular_queue.h"

using namespace std;

// NOTE: this implementation of Circular_queue will ONLY process a SINGLE
//       instantiation!!

Circular_queue::Circular_queue(
	int po2,
	const char *mList[], int mlCount,
	int (*_matchFunc)(void *, char *),
	int (*_readDataFrom)(void *))
{
	buffer_size = (1 << po2);
	buffer = new char[buffer_size];
	if (!buffer) {
		throw CircQueException("Cannot allocate buffer");
	}

	bufferCount = 0;

	index_mask = 1;
	for(int i = 1; i < po2; i++)
		index_mask |= (index_mask << 1);

	memset(buffer, 0, buffer_size);
	inhibitDataOut = CQUE_HOLD;

	stringMatchingList(mList, mlCount);

	matchFound = _matchFunc;
	readData   = _readDataFrom;

	inhibitDataOut = CQUE_RESUME;

	exit_thread = 0;

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&condition, NULL);

	int perr = pthread_create(&thread, 0, circularQueueParser, this);
	if (perr) {
		throw CircQueException(perr, "Cannot create thread");
	}

}

Circular_queue::~Circular_queue()
{
    signal();

	pthread_mutex_lock(&mutex);
	exit_thread = 1;
	pthread_mutex_unlock(&mutex);

	pthread_join(thread, NULL);
	pthread_cond_destroy(&condition);

	delete [] buffer;
	delete [] patternMatchList;
}

void Circular_queue::stopDataOut()
{
	inhibitDataOut = CQUE_HOLD;
}

void Circular_queue::startDataOut()
{
	inhibitDataOut = CQUE_RESUME;
}

void Circular_queue::addToQueue(char *_buffer, int _size)
{
	if(!_buffer || _size < 1) return;

	pthread_mutex_lock(&mutex);

	while(_size > 0) {
		if (bufferCount >= buffer_size) {
			stalled = 1;
			break;
		}
		write_index &= index_mask;
		buffer[write_index++] = *_buffer++;
		_size--;
		bufferCount++;
	}

	pthread_mutex_unlock(&mutex);
}

int  Circular_queue::lookAheadCRC(char *_buffer, int _size, unsigned int *crcVal, int *reset)
{
	if(!_buffer || _size < 1 || !crcVal || !reset) return 0;

	int count = 0;
	int temp_index = 0;
	int buffer_count = 0;
	unsigned int crcval = *crcVal;
	char *cPtr = _buffer;

	pthread_mutex_lock(&mutex);

	temp_index = read_index;
	buffer_count = bufferCount;

	if(*reset) {
		crcval = 0xFFFF;
		*reset = 0;
	}

	if (buffer_count > 0) {
		while(!exit_thread) {

			if (count >= _size) break;

			temp_index &= index_mask;

			if (buffer_count > 0)  {

				*cPtr = buffer[temp_index++];
				cPtr[1] = 0;

				buffer_count--;
				count++;

				crcval ^= (char ) *cPtr;
				for (int i = 0; i < 8; ++i) {
					if (crcval & 1)
						crcval = (crcval >> 1) ^ 0xA001;
					else
						crcval = (crcval >> 1);
				}

				cPtr++;
			} else
			   break;

		}
	}

	pthread_mutex_unlock(&mutex);

	readQueData(buffer_count);

	*crcVal = crcval & 0xFFFF;

	return count;

}

int Circular_queue::lookAhead(char *_buffer, int _size)
{
	if(!_buffer || _size < 1) return 0;

	int count = 0;
	int temp_index = 0;
	int buffer_count = 0;
	char *cPtr = _buffer;

	pthread_mutex_lock(&mutex);

	temp_index = read_index;
	buffer_count = bufferCount;

	if(buffer_count > 0) {
		while(!exit_thread) {

			if(count >= _size) break;

			temp_index &= index_mask;

			if(buffer_count > 0)  {
				*cPtr = buffer[temp_index++];
				cPtr[1] = 0;
				cPtr++;
				buffer_count--;
				count++;
			} else
			   break;
		}
	}

	pthread_mutex_unlock(&mutex);

	readQueData(buffer_count);

	return count;
}

int Circular_queue::readQueData(int buffer_count)
{
	if(buffer_count < 32 && readData) {
		(*readData)((void *)this);
	}

	return 0;
}

int Circular_queue::adjustReadQueIndex(int count)
{
	if (count < 1) return 0;

	pthread_mutex_lock(&mutex);

	if(count >= bufferCount)
		count = bufferCount;

	bufferCount -= count;
	read_index += count;
	read_index &= index_mask;

	if(bufferCount <= 0) {
		bufferCount = 0;
		write_index = read_index;
	}

	pthread_mutex_unlock(&mutex);

	readQueData(bufferCount);

	return count;
}


int Circular_queue::lookAheadToTerminator(char *_buffer, char terminator, int maxLen)
{
	if (!_buffer || maxLen < 1) return 0;

	int count = 0;

	char *cPtr = _buffer;

	pthread_mutex_lock(&mutex);

	int temp_index   = read_index;
	int buffer_count = bufferCount;

	if (buffer_count > 0) {
		while (!exit_thread) {

			if(count >= maxLen)
				break;

			temp_index &= index_mask;

			if (buffer_count > 0)  {
				*cPtr = buffer[temp_index++];
				cPtr[1] = 0;
				buffer_count--;
				count++;
			} else
			   break;

			if(*cPtr == terminator) {
				break;
			}

			cPtr++;
		}
	}

	pthread_mutex_unlock(&mutex);

	readQueData(buffer_count);

	return count;
}

int Circular_queue::lookAheadForCharacter(char character, int *found)
{
	if (!found) return 0;

	int count = 0;
    char valueRead = 0;

	pthread_mutex_lock(&mutex);

	int temp_index   = read_index;
	int buffer_count = bufferCount;

	if (buffer_count > 0) {
        *found = 0;

		while (!exit_thread) {

			temp_index &= index_mask;

			if (buffer_count > 0)  {
				valueRead = buffer[temp_index++];
				buffer_count--;
                count++;
			} else
			   break;

			if(valueRead == character) {
				*found = 1;
                break;
			}
		}
	}

	pthread_mutex_unlock(&mutex);

	readQueData(buffer_count);

	return count;
}

void Circular_queue::stopQueue()
{
	inhibitDataOut = CQUE_HOLD;
}

void Circular_queue::resumeQueue()
{
	inhibitDataOut = CQUE_RESUME;
}

bool Circular_queue::timeOut(time_t &timeValue, time_t seconds, int attribute)
{
    time_t currentTime = time(NULL);
    time_t ExpTime = timeValue + seconds;
    bool ret = false;
    
    switch(attribute) {
    case TIME_SET:
        timeValue = currentTime;
        ret = true;
        break;
        
    case TIME_COUNT:
        if(currentTime > ExpTime) {
            timeValue = 0;
            ret = true;
        }
        break;
    }
    
    if(timeValue == 0 && seconds > 0)
        timeValue = currentTime + seconds;
    
    return false;
}

void Circular_queue::sleep(int seconds, int milliseconds)
{
	struct timespec		ts;
	struct timeval		tp;

	gettimeofday(&tp, NULL);

	ts.tv_nsec = (tp.tv_usec * 1000) + milliseconds * 1000000;
	ts.tv_sec  = tp.tv_sec + seconds;

	pthread_mutex_lock(&mutex);
	pthread_cond_timedwait(&condition, &mutex, &ts);
	pthread_mutex_unlock(&mutex);
}

void Circular_queue::signal(void)
{
	pthread_cond_signal(&condition);
}

// Specific use parts of the code.

void Circular_queue::stringMatchingList(const char **mList, int mlCount)
{
	int size = 0;
	matchMaxLen = 0;
	patternMatchList = new MATCH_STRING[mlCount];

    if(patternMatchList) {
        for (int index = 0; index < mlCount; index++) {
            if (mList[index]) {
                size = (int)strnlen(mList[index], STRING_MATCH_SIZE_LIMIT);
                matchMaxLen = size > matchMaxLen ? size : matchMaxLen;
                if (size) {
                    patternMatchList[index].string_size = size;
                    memcpy(patternMatchList[index].string, mList[index], size);
                }
                const char *cPtr = mList[index];
                int val = 0;
                patternMatchList[index].match = 0;
                for (size_t index2 = 0; index2 < sizeof(uint32_t); index2++) {
                    val = cPtr[index2];
                    patternMatchList[index].match |= (val & 0xff);
                    if(index2 < (sizeof(uint32_t) - 1))
                        patternMatchList[index].match <<= 8;
                }
            }
        }

        listCount = mlCount;
    }
}

void * circularQueueParser(void *ptr)
{
	Circular_queue *que = (Circular_queue *)ptr;
	int offset = 0;
	int buffer_count = 0;
	int buffer_size  = 0;
	int count = 0;
	int found = 1;
    int readCount = 0;
    int oldCount = 0;
    time_t tm_time = 0;
	uint32_t match = 0;
	uint32_t matchTo = 0;

	char buffer[sizeof(uint32_t) + 1];

	if(!que) {
		return (void *)0;
	}

	if(!que->matchFound || !que->patternMatchList) {
		return (void *)0;
	}

	que->thread_running = 1;

	buffer_size = sizeof(uint32_t);
	memset(buffer, 0, sizeof(buffer));

	while(!que->exit_thread) {

        found = 0;
        readCount = que->lookAheadForCharacter('<', &found);

		if(readCount < 1)
			que->milliSleep(50);

        if(found) {

            if(readCount > 0) {
                readCount--;
                que->adjustReadQueIndex(readCount);
            }

            buffer_count = 0;
            oldCount = -1;
            
   			while(buffer_count < buffer_size && !que->exit_thread) {
				
                buffer_count = que->lookAhead(buffer, buffer_size);
                
                if(buffer_count != oldCount)
                    que->timeOut(tm_time, 10, TIME_SET);
                else
                    que->milliSleep(100);

                if(que->timeOut(tm_time, 10, TIME_COUNT))
                    break;
                   
                oldCount = buffer_count;
			}

            match = 0;

            for(count = 0; count < buffer_count; count++) {
                match |= (buffer[count] & 0xff);

                if(count < (buffer_count - 1))
                    match <<= 8;
            }

            readCount = 1;

            for (int index = 0; index < que->listCount; index++) {
                matchTo = que->patternMatchList[index].match;

                if(matchTo != match)
                    continue;

                readCount = 0;

                if(que->inhibitDataOut == CQUE_RESUME) {
                    char *tag = que->patternMatchList[index].string;
                    offset = (que->matchFound)((void *)que, tag);
                    break;
                }
            }
		}

        if(readCount > 0)
           que->adjustReadQueIndex(readCount);

		que->milliSleep(50);

	}

	que->thread_running = 0;

	return ptr;
}
