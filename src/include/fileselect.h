#ifndef FILESELECT_H
#define FILESELECT_H

#include <config.h>

#ifdef __APPLE__
#undef NATIVE_CHOOSER
#else
#define NATIVE_CHOOSER 1
#endif

#ifdef NATIVE_CHOOSER
class Fl_Native_File_Chooser;
#include <FL/Fl_Native_File_Chooser.H>
#else
class Fl_File_Chooser;
#include <FL/Fl_File_Chooser.H>
#endif

namespace FSEL {

	void create(void);
	void destroy(void);
	const char* select(const char* title, const char* filter, const char* def = 0);
	const char* saveas(const char* title, const char* filter, const char* def = 0);
	const char* dir_select(const char* title, const char* filter, const char* def = 0);

}

#endif // FILESELECT_H
