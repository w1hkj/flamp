# Copyright (c) 2008 Dave Freese, W1HKJ (w1hkj AT w1hkj DOT com)

bin_PROGRAMS = flamp

flamp_CPPFLAGS = @FLAMP_BUILD_CPPFLAGS@
flamp_CXXFLAGS = @FLAMP_BUILD_CXXFLAGS@
flamp_CFLAGS = $(flamp_CXXFLAGS)
flamp_LDFLAGS = @FLAMP_BUILD_LDFLAGS@
flamp_LDADD = @FLAMP_BUILD_LDADD@

MINGW32_SRC = flamp-rc.rc include/flamprc.h \
	utils/mingw.c include/mingw.h include/compat.h \
	utils/getsysinfo.c

# We distribute these but do not always compile them
EXTRA_flamp_SOURCES = $(flamp_WIN32_RES_SRC) $(MINGW32_SRC)

flamp_SOURCES =

if MINGW32
if HAVE_WINDRES
  flamp_SOURCES += $(MINGW32_SRC)
endif
endif

########################################################################


flamp_VERSION_MAJOR = @FLAMP_VERSION_MAJOR@
flamp_VERSION_MINOR = @FLAMP_VERSION_MINOR@
flamp_VERSION_PATCH = @FLAMP_VERSION_PATCH@
flamp_VERSION = @FLAMP_VERSION@

.EXPORT_ALL_VARIABLES: nsisinst appbundle

# Sources that are generated,
BUILT_SOURCES =
# not distributed,
nodist_flamp_SOURCES = $(BUILT_SOURCES)
# and deleted by the clean targets
CLEANFILES = $(BUILT_SOURCES)
CLEAN_LOCAL =

if MINGW32
if HAVE_WINDRES
.rc.o:
	$(WINDRES) -I$(srcdir)/include -I$(srcdir)/../data/win32 $< -O coff $@
endif
endif

install-data-local:
	if test -f $(srcdir)/../data/flamp.xpm; then \
	    $(mkinstalldirs) $(DESTDIR)/$(datadir)/pixmaps; \
	    $(INSTALL_DATA) $(srcdir)/../data/flamp.xpm $(DESTDIR)/$(datadir)/pixmaps; \
	fi
	if test -f $(srcdir)/../data/flamp.desktop; then \
	    $(mkinstalldirs) $(DESTDIR)/$(datadir)/applications; \
	    $(INSTALL_DATA) $(srcdir)/../data/flamp.desktop $(DESTDIR)/$(datadir)/applications; \
	fi

uninstall-local:
	rm -f $(DESTDIR)/$(datadir)/pixmaps/flamp.xpm
	rm -f $(DESTDIR)/$(datadir)/applications/flamp.desktop

# FL_SRC = flamp_dialog.fl

if HAVE_FLUID
flgen: $(FL_SRC)
	(cd $(srcdir)/include; \
	for f in $(FL_SRC); do \
	    c=$${f%.fl}.cxx; \
		h=$${f%.fl}.h; \
		h=$${h##*/}; \
	    $(FLUID) -c -o ../$$c -h $$h ../$$f; \
	done)
endif


INSTALLER_FILE = flamp-$(flamp_VERSION)_setup.exe
APPBUNDLE=flamp-$(flamp_VERSION)
APPBUNDLE_NOLIBS=$(APPBUNDLE)-nolibs

if HAVE_NSIS
nsisinst: $(bin_PROGRAMS)
	sh $(srcdir)/../scripts/mknsisinst.sh "$(srcdir)/../data" .
    CLEANFILES += $(INSTALLER_FILE)
endif

if DARWIN
appbundle: $(bin_PROGRAMS)
	sh $(srcdir)/../scripts/mkappbundle.sh "$(srcdir)/../data" .
    CLEAN_LOCAL += $(APPBUNDLE_NOLIBS) $(APPBUNDLE) $(APPBUNDLE)*.dmg
endif

clean-local:
	-rm -rf $(CLEAN_LOCAL)

# Sources that we build. It is OK to have headers here.
flamp_SOURCES += \
	$(srcdir)/flamp.cxx \
	$(srcdir)/flamp_dialog.cxx \
	$(srcdir)/lzma/Alloc.h \
	$(srcdir)/lzma/Alloc.c \
	$(srcdir)/lzma/LzFind.c \
	$(srcdir)/lzma/LzFind.h \
	$(srcdir)/lzma/LzHash.h \
	$(srcdir)/lzma/LzmaDec.c \
	$(srcdir)/lzma/LzmaDec.h \
	$(srcdir)/lzma/LzmaEnc.c \
	$(srcdir)/lzma/LzmaEnc.h \
	$(srcdir)/lzma/LzmaLib.c \
	$(srcdir)/lzma/LzmaLib.h \
	$(srcdir)/lzma/Types.h \
	$(srcdir)/utils/amp.cxx \
	$(srcdir)/utils/base64.cxx \
	$(srcdir)/utils/base128.cxx \
	$(srcdir)/utils/base256.cxx \
	$(srcdir)/utils/debug.cxx \
	$(srcdir)/utils/socket.cxx \
	$(srcdir)/utils/status.cxx \
	$(srcdir)/utils/timeops.cxx \
	$(srcdir)/utils/util.cxx \
	$(srcdir)/utils/file_io.cxx \
	$(srcdir)/utils/xml_io.cxx \
	$(srcdir)/utils/global_amp.cxx \
	$(srcdir)/utils/circular_queue.cxx \
	$(srcdir)/utils/tagSearch.cxx \
	$(srcdir)/utils/time_table.cxx \
	$(srcdir)/utils/transmit_camp.cxx \
	$(srcdir)/utils/ztimer.cxx \
	$(srcdir)/utils/script_parsing.cxx \
	$(srcdir)/utils/run_scripts.cxx \
	$(srcdir)/widgets/calendar.cxx \
	$(srcdir)/widgets/combo.cxx \
	$(srcdir)/widgets/date.cxx \
	$(srcdir)/widgets/fileselect.cxx \
	$(srcdir)/widgets/flinput2.cxx \
	$(srcdir)/widgets/flslider2.cxx \
	$(srcdir)/widgets/Fl_BlockMap.cxx \
	$(srcdir)/widgets/Fl_Native_File_Chooser.cxx \
	$(srcdir)/widgets/Fl_Text_Buffer_mod.cxx \
	$(srcdir)/widgets/Fl_Text_Editor_mod.cxx \
	$(srcdir)/widgets/Fl_Text_Display_mod.cxx \
	$(srcdir)/widgets/FTextView.cxx \
	$(srcdir)/widgets/icons.cxx \
	$(srcdir)/widgets/missing_strings.c \
	$(srcdir)/widgets/pixmaps.cxx \
	$(srcdir)/widgets/pixmaps_tango.cxx \
	$(srcdir)/widgets/font_browser.cxx \
	$(srcdir)/widgets/hamcast_group.cxx \
	$(srcdir)/include/hamcast_group.h \
	$(srcdir)/xmlrpcpp/XmlRpc.h \
	$(srcdir)/xmlrpcpp/XmlRpcClient.cpp \
	$(srcdir)/xmlrpcpp/XmlRpcClient.h \
	$(srcdir)/xmlrpcpp/XmlRpcDispatch.cpp \
	$(srcdir)/xmlrpcpp/XmlRpcDispatch.h \
	$(srcdir)/xmlrpcpp/XmlRpcException.h \
	$(srcdir)/xmlrpcpp/XmlRpcMutex.cpp \
	$(srcdir)/xmlrpcpp/XmlRpcMutex.h \
	$(srcdir)/xmlrpcpp/XmlRpcServer.cpp \
	$(srcdir)/xmlrpcpp/XmlRpcServer.h \
	$(srcdir)/xmlrpcpp/XmlRpcServerConnection.cpp \
	$(srcdir)/xmlrpcpp/XmlRpcServerConnection.h \
	$(srcdir)/xmlrpcpp/XmlRpcServerMethod.cpp \
	$(srcdir)/xmlrpcpp/XmlRpcServerMethod.h \
	$(srcdir)/xmlrpcpp/XmlRpcSocket.cpp \
	$(srcdir)/xmlrpcpp/XmlRpcSocket.h \
	$(srcdir)/xmlrpcpp/XmlRpcSource.cpp \
	$(srcdir)/xmlrpcpp/XmlRpcSource.h \
	$(srcdir)/xmlrpcpp/XmlRpcThread.cpp \
	$(srcdir)/xmlrpcpp/XmlRpcThread.h \
	$(srcdir)/xmlrpcpp/XmlRpcThreadedServer.cpp \
	$(srcdir)/xmlrpcpp/XmlRpcThreadedServer.h \
	$(srcdir)/xmlrpcpp/XmlRpcUtil.cpp \
	$(srcdir)/xmlrpcpp/XmlRpcUtil.h \
	$(srcdir)/xmlrpcpp/XmlRpcValue.cpp \
	$(srcdir)/xmlrpcpp/XmlRpcValue.h \
	$(srcdir)/xmlrpcpp/base64.h

# Additional source files that are distributed
EXTRA_DIST = \
	$(srcdir)/config.h \
	$(srcdir)/utils/mingw.c \
	$(srcdir)/utils/time_table.dat \
	$(srcdir)/widgets/flnfc_common.cxx \
	$(srcdir)/widgets/Fl_Native_File_Chooser_FLTK.cxx \
	$(srcdir)/widgets/Fl_Native_File_Chooser_MAC.cxx \
	$(srcdir)/widgets/Fl_Native_File_Chooser_WIN32.cxx \
	$(srcdir)/include/amp.h \
	$(srcdir)/include/ascii.h \
	$(srcdir)/include/base64.h \
	$(srcdir)/include/base128.h \
	$(srcdir)/include/base256.h \
	$(srcdir)/include/calendar.h \
	$(srcdir)/include/combo.h \
	$(srcdir)/include/compat.h \
	$(srcdir)/include/compat-mingw.h \
	$(srcdir)/include/crc16.h \
	$(srcdir)/include/date.h \
	$(srcdir)/include/debug.h \
	$(srcdir)/include/dirent-check.h \
	$(srcdir)/include/fileselect.h \
	$(srcdir)/include/fl_lock.h \
	$(srcdir)/include/flmisc.h \
	$(srcdir)/include/flamp_config.h \
	$(srcdir)/include/flamprc.h \
	$(srcdir)/include/flinput2.h \
	$(srcdir)/include/flslider2.h \
	$(srcdir)/include/font_browser.h \
	$(srcdir)/include/gettext.h \
	$(srcdir)/include/icons.h \
	$(srcdir)/include/flamp.h \
	$(srcdir)/include/flamp_dialog.h \
	$(srcdir)/include/missing_strings.h \
	$(srcdir)/include/socket.h \
	$(srcdir)/include/mingw.h \
	$(srcdir)/include/pixmaps.h \
	$(srcdir)/include/status.h \
	$(srcdir)/include/threads.h \
	$(srcdir)/include/timeops.h \
	$(srcdir)/include/util.h \
	$(srcdir)/include/file_io.h \
	$(srcdir)/include/xml_io.h \
	$(srcdir)/include/global_amp.h \
	$(srcdir)/include/circular_queue.h \
	$(srcdir)/include/tagSearch.h \
	$(srcdir)/include/time_table.h \
	$(srcdir)/include/transmit_camp.h \
	$(srcdir)/include/ztimer.h \
	$(srcdir)/include/script_parsing.h \
	$(srcdir)/include/FL/Fl_Native_File_Chooser_FLTK.H \
	$(srcdir)/include/FL/Fl_Native_File_Chooser_MAC.H \
	$(srcdir)/include/FL/Fl_Native_File_Chooser.H \
	$(srcdir)/include/FL/Fl_Native_File_Chooser_WIN32.H \
	$(srcdir)/widgets/Fl_Text_Buffer_mod_1_1.cxx \
	$(srcdir)/widgets/Fl_Text_Buffer_mod_1_3.cxx \
	$(srcdir)/widgets/Fl_Text_Display_mod_1_1.cxx \
	$(srcdir)/widgets/Fl_Text_Display_mod_1_3.cxx \
	$(srcdir)/widgets/Fl_Text_Editor_mod_1_1.cxx \
	$(srcdir)/widgets/Fl_Text_Editor_mod_1_3.cxx \
	$(srcdir)/include/Fl_BlockMap.h \
	$(srcdir)/include/Fl_Text_Buffer_mod.H \
	$(srcdir)/include/Fl_Text_Buffer_mod_1_1.H \
	$(srcdir)/include/Fl_Text_Buffer_mod_1_3.H \
	$(srcdir)/include/Fl_Text_Display_mod.H \
	$(srcdir)/include/Fl_Text_Display_mod_1_1.H \
	$(srcdir)/include/Fl_Text_Display_mod_1_3.H \
	$(srcdir)/include/Fl_Text_Editor_mod.H \
	$(srcdir)/include/Fl_Text_Editor_mod_1_1.H \
	$(srcdir)/include/Fl_Text_Editor_mod_1_3.H \
	$(srcdir)/include/FTextView.h

# Additional non-source files that are distributed
#EXTRA_DIST += \
#	$(srcdir)/../data/template.rtf
#	$(srcdir)/../flamp-doc/flamp-quick-guide.html

# Additional source files that support non-Linux cross compilation
EXTRA_DIST += \
	$(srcdir)/../ChangeLog \
	$(srcdir)/../data/flamp.desktop \
	$(srcdir)/../data/flamp.xpm \
	$(srcdir)/../data/flamp.png \
	$(srcdir)/../data/win32/fl_app.nsi \
	$(srcdir)/../data/win32/flamp.ico \
	$(srcdir)/../scripts/build4mingw.sh \
	$(srcdir)/../scripts/builddist.sh \
	$(srcdir)/../scripts/mkappbundle.sh \
	$(srcdir)/../scripts/mknsisinst.sh \
	$(srcdir)/../data/mac/Info.plist.in \
	$(srcdir)/../data/mac/flamp.icns \
	$(flamp_WIN32_SRC) \
	$(flamp_FL_SRC)
