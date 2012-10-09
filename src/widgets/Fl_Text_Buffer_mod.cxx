#include <config.h>

#if FLAMP_FLTK_API_MAJOR == 1 && FLAMP_FLTK_API_MINOR < 3
#  include "Fl_Text_Buffer_mod_1_1.cxx"
#elif FLAMP_FLTK_API_MAJOR == 1 && FLAMP_FLTK_API_MINOR == 3
#  include "Fl_Text_Buffer_mod_1_3.cxx"
#endif
