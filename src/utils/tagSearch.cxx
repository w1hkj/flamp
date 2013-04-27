// tagSearch.cxx
//
//  Author(s):
//    Robert Stiles, KK5VD, Copyright (C) 2013
//    Dave Freese, W1HKJ, Copyright (C) 2013
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

#include "tagSearch.h"

// This version performs a direct match on the first four bytes in the string.

void * tag_search_parser(void *ptr);

const char * searchTags[] = {
	"<FILE ",
	"<ID ",
	"<DESC ",
	"<DATA ",
	"<PROG ",
	"<CNTL ",
	"<SIZE ",
	(char *)0
};

TagSearch::TagSearch()
{

}

TagSearch::TagSearch(int (*_data_stream)(void *), int (*_process_que)(void *))
{
	search_tag_count = (sizeof(searchTags) / (sizeof(char *) - 1));
	stringMatchingList(searchTags, search_tag_count);
	setUp(18, _process_que, _data_stream, tag_search_parser);
}

TagSearch::~TagSearch()
{
	delete [] patternMatchList;
}

void TagSearch::stringMatchingList(const char *mList[], int mlCount)
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
	} else {
		throw TagSearchException("Tag matching allocation error");
	}
}

void * tag_search_parser(void *ptr)
{
	TagSearch *ts_ptr = (TagSearch *)ptr;
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

	if(!ts_ptr) {
		return (void *)0;
	}

	if(!ts_ptr->matchFound || !ts_ptr->patternMatchList) {
		return (void *)0;
	}

	ts_ptr->thread_running = 1;

	buffer_size = sizeof(uint32_t);
	memset(buffer, 0, sizeof(buffer));

	while(!ts_ptr->thread_exit()) {

		found = 0;
		readCount = ts_ptr->lookAheadForCharacter('<', &found);

		if(readCount < 1)
			ts_ptr->milliSleep(50);

		if(found) {

			if(readCount > 0) {
				readCount--;
				ts_ptr->adjustReadQueIndex(readCount);
			}

			buffer_count = 0;
			oldCount = -1;

   			while(buffer_count < buffer_size && !ts_ptr->thread_exit()) {

				buffer_count = ts_ptr->lookAhead(buffer, buffer_size);

				if(buffer_count != oldCount)
					ts_ptr->timeOut(tm_time, 10, TIME_SET);
				else
					ts_ptr->milliSleep(100);

				if(ts_ptr->timeOut(tm_time, 10, TIME_COUNT))
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

			for (int index = 0; index < ts_ptr->listCount; index++) {
				matchTo = ts_ptr->patternMatchList[index].match;

				if(matchTo != match)
					continue;

				readCount = 0;

				if(ts_ptr->inhibitDataOut == CQUE_RESUME) {
					offset = (ts_ptr->matchFound)((void *)ts_ptr);
					break;
				}
			}
		}

		if(readCount > 0)
		   ts_ptr->adjustReadQueIndex(readCount);

		ts_ptr->milliSleep(50);

	}

	ts_ptr->thread_running = 0;

	return ptr;
}
