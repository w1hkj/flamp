# Copyright (c) 2008 Dave Freese, W1HKJ (w1hkj AT w1hkj DOT com)

bin_PROGRAMS = fllnk

fllnk_CPPFLAGS = @FLLNK_BUILD_CPPFLAGS@
fllnk_CXXFLAGS = @FLLNK_BUILD_CXXFLAGS@
fllnk_CFLAGS = $(fllnk_CXXFLAGS)
fllnk_LDFLAGS = @FLLNK_BUILD_LDFLAGS@
fllnk_LDADD = @FLLNK_BUILD_LDADD@

MINGW32_SRC = fllnk-rc.rc include/fllnkrc.h \
	utils/mingw.c include/mingw.h include/compat.h \
	utils/getsysinfo.c

# We distribute these but do not always compile them
EXTRA_fllnk_SOURCES = $(fllnk_WIN32_RES_SRC) $(MINGW32_SRC)

fllnk_SOURCES =

if MINGW32
if HAVE_WINDRES
  fllnk_SOURCES += $(MINGW32_SRC)
endif
endif

########################################################################


fllnk_VERSION_MAJOR = @FLLNK_VERSION_MAJOR@
fllnk_VERSION_MINOR = @FLLNK_VERSION_MINOR@
fllnk_VERSION_PATCH = @FLLNK_VERSION_PATCH@
fllnk_VERSION = @FLLNK_VERSION@

.EXPORT_ALL_VARIABLES: nsisinst appbundle

# Sources that are generated,
BUILT_SOURCES =
# not distributed,
nodist_fllnk_SOURCES = $(BUILT_SOURCES)
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
	if test -f $(srcdir)/../data/fllnk.xpm; then \
	    $(mkinstalldirs) $(DESTDIR)/$(datadir)/pixmaps; \
	    $(INSTALL_DATA) $(srcdir)/../data/fllnk.xpm $(DESTDIR)/$(datadir)/pixmaps; \
	fi
	if test -f $(srcdir)/../data/fllnk.desktop; then \
	    $(mkinstalldirs) $(DESTDIR)/$(datadir)/applications; \
	    $(INSTALL_DATA) $(srcdir)/../data/fllnk.desktop $(DESTDIR)/$(datadir)/applications; \
	fi

uninstall-local:
	rm -f $(DESTDIR)/$(datadir)/pixmaps/fllnk.xpm
	rm -f $(DESTDIR)/$(datadir)/applications/fllnk.desktop

# FL_SRC = fllnk_dialog.fl

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


INSTALLER_FILE = fllnk-$(fllnk_VERSION)_setup.exe
APPBUNDLE=fllnk-$(fllnk_VERSION)
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
fllnk_SOURCES += \
	$(srcdir)/fllnk.cxx \
	$(srcdir)/fllnk_dialog.cxx \
	$(srcdir)/utils/debug.cxx \
	$(srcdir)/utils/socket.cxx \
	$(srcdir)/utils/status.cxx \
	$(srcdir)/utils/util.cxx \
	$(srcdir)/utils/file_io.cxx \
	$(srcdir)/utils/xml_io.cxx \
	$(srcdir)/utils/ztimer.cxx \
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
	$(srcdir)/xmlrpcpp/XmlRpcValue.h

# Additional source files that are distributed
EXTRA_DIST = \
	$(srcdir)/config.h \
	$(srcdir)/utils/mingw.c \
	$(srcdir)/widgets/flnfc_common.cxx \
	$(srcdir)/widgets/Fl_Native_File_Chooser_FLTK.cxx \
	$(srcdir)/widgets/Fl_Native_File_Chooser_MAC.cxx \
	$(srcdir)/widgets/Fl_Native_File_Chooser_WIN32.cxx \
	$(srcdir)/include/ascii.h \
	$(srcdir)/include/calendar.h \
	$(srcdir)/include/combo.h \
	$(srcdir)/include/compat.h \
	$(srcdir)/include/compat-mingw.h \
	$(srcdir)/include/date.h \
	$(srcdir)/include/debug.h \
	$(srcdir)/include/dirent-check.h \
	$(srcdir)/include/fileselect.h \
	$(srcdir)/include/fl_lock.h \
	$(srcdir)/include/flmisc.h \
	$(srcdir)/include/fllnk_config.h \
	$(srcdir)/include/fllnkrc.h \
	$(srcdir)/include/flinput2.h \
	$(srcdir)/include/flslider2.h \
	$(srcdir)/include/font_browser.h \
	$(srcdir)/include/gettext.h \
	$(srcdir)/include/icons.h \
	$(srcdir)/include/fllnk.h \
	$(srcdir)/include/fllnk_dialog.h \
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
	$(srcdir)/include/ztimer.h \
	$(srcdir)/include/script_parsing.h \
	$(srcdir)/include/FL/Fl_Native_File_Chooser_FLTK.H \
	$(srcdir)/include/FL/Fl_Native_File_Chooser_MAC.H \
	$(srcdir)/include/FL/Fl_Native_File_Chooser.H \
	$(srcdir)/widgets/Fl_Text_Buffer_mod.cxx \
	$(srcdir)/widgets/Fl_Text_Display_mod.cxx \
	$(srcdir)/widgets/Fl_Text_Editor_mod.cxx \
	$(srcdir)/include/Fl_BlockMap.h \
	$(srcdir)/include/Fl_Text_Buffer_mod.H \
	$(srcdir)/include/Fl_Text_Display_mod.H \
	$(srcdir)/include/Fl_Text_Editor_mod.H \
	$(srcdir)/include/FTextView.h

# Additional non-source files that are distributed
#EXTRA_DIST += \
#	$(srcdir)/../data/template.rtf
#	$(srcdir)/../fllnk-doc/fllnk-quick-guide.html

# Additional source files that support non-Linux cross compilation
EXTRA_DIST += \
	$(srcdir)/../ChangeLog \
	$(srcdir)/../data/fllnk.desktop \
	$(srcdir)/../data/fllnk.xpm \
	$(srcdir)/../data/fllnk.png \
	$(srcdir)/../data/win32/fl_app.nsi \
	$(srcdir)/../data/win32/fllnk.ico \
	$(srcdir)/../scripts/build4mingw.sh \
	$(srcdir)/../scripts/builddist.sh \
	$(srcdir)/../scripts/mkappbundle.sh \
	$(srcdir)/../scripts/mknsisinst.sh \
	$(srcdir)/../data/mac/Info.plist.in \
	$(srcdir)/../data/mac/fllnk.icns \
	$(fllnk_WIN32_SRC) \
	$(fllnk_FL_SRC)
