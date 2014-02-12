// tagSearch.cxx
//
//  Author(s):
//	Robert Stiles, KK5VD, Copyright (C) 2013
//	Dave Freese, W1HKJ, Copyright (C) 2013
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
	"<MISSING ",
	(char *)0
};

TagSearch::TagSearch()
{

}

TagSearch::TagSearch(int (*_data_stream)(void *), int (*_process_que)(void *))
{
	search_tag_count = (sizeof(searchTags)/sizeof(char *)) - 1;
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
			} else break;
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
	int shift_buffer_count = 0;
	int shift_buffer_size  = 0;
	int count = 0;
	int found = 1;
	int read_count = 0;
	int old_count = 0;
	time_t tm_time = 0;
	uint32_t match = 0;
	uint32_t matchTo = 0;

	char shift_buffer[sizeof(uint32_t) + 1];

	if(!ts_ptr) {
		return (void *)0;
	}

	if(!ts_ptr->matchFound || !ts_ptr->patternMatchList) {
		return (void *)0;
	}

	ts_ptr->thread_running = 1;

	shift_buffer_size = sizeof(uint32_t);
	memset(shift_buffer, 0, sizeof(shift_buffer));

	while(!ts_ptr->thread_exit()) {

		found = 0;
		read_count = ts_ptr->lookAheadForCharacter('<', &found);

		if(read_count < 1)
			ts_ptr->milliSleep(50);

		if(found) {

			if(read_count > 0) {
				read_count--;
				ts_ptr->adjustReadQueIndex(read_count);
			}

			shift_buffer_count = 0;
			old_count = -1;

			while(shift_buffer_count < shift_buffer_size && !ts_ptr->thread_exit()) {

				shift_buffer_count = ts_ptr->lookAhead(shift_buffer, shift_buffer_size);

				if(shift_buffer_count != old_count)
					ts_ptr->timeOut(tm_time, 10, TIME_SET);
				else
					ts_ptr->milliSleep(100);

				if(ts_ptr->timeOut(tm_time, 10, TIME_COUNT))
					break;

				old_count = shift_buffer_count;
			}

			match = 0;

			for(count = 0; count < shift_buffer_count; count++) {
				match |= (shift_buffer[count] & 0xff);

				if(count < (shift_buffer_count - 1))
					match <<= 8;
			}

			read_count = 1;

			for (int index = 0; index < ts_ptr->listCount; index++) {
				matchTo = ts_ptr->patternMatchList[index].match;

				if(matchTo != match)
					continue;
				
				read_count = 0;
				
				if(ts_ptr->inhibitDataOut == CQUE_RESUME) {
					offset = (ts_ptr->matchFound)((void *)ts_ptr);
					break;
				}
			}
		}
		
		if(read_count > 0)
			ts_ptr->adjustReadQueIndex(read_count);
		
		ts_ptr->milliSleep(50);
		
	}
	
	ts_ptr->thread_running = 0;
	
	return ptr;
}
