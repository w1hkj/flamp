//
//  tagSearch.h
//
//  Author(s):
//	Robert Stiles, KK5VD, Copyright (C) 2013
//	Dave Freese, W1HKJ, Copyright (C) 2013
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

#ifndef flamp_tag_search_h
#define flamp_tag_search_h

#include "circular_queue.h"
#include "util.h"

#define MATCH_FOUND	 1
#define MATCH_NOT_FOUND 0

#define STRING_MATCH_SIZE_LIMIT 16

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

class TagSearchException : public std::exception
{
public:
	TagSearchException(int err_ = 0)	 : err(err_), msg(err_to_str(err_)) { }
	TagSearchException(const char* msg_) : err(1),	msg(msg_)			 { }
	TagSearchException(int err_, const std::string& prefix)
	: err(err_), msg(std::string(prefix).append(": ").append(err_to_str(err_))) { }
	virtual ~TagSearchException() throw() { }
	const char*	 what(void) const throw() { return msg.c_str(); }
	int			 error(void) const { return err; }

protected:
	const char* err_to_str(int e) {
		return strerror(e);
	}
	int			 err;
	std::string	 msg;
};

class TagSearch : public Circular_queue
{

private:

	MATCH_STRING *patternMatchList;
	int matchMaxLen;
	int search_tag_count;
	int listCount;

public:
	TagSearch(void);
	TagSearch(int (*_readDataFrom)(void *),	int (*_matchFound)(void *));
	~TagSearch();

	void stringMatchingList(const char *mList[], int mlCount);

	friend
	void * tag_search_parser(void *ptr);

};

#endif // flamp_tag_search_h
