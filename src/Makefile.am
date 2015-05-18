# Copyright (c) 2008 Dave Freese, W1HKJ (w1hkj AT w1hkj DOT com)

bin_PROGRAMS = fm_msngr

fm_msngr_CPPFLAGS = @FM_MSNGR_BUILD_CPPFLAGS@
fm_msngr_CXXFLAGS = @FM_MSNGR_BUILD_CXXFLAGS@
fm_msngr_CFLAGS = $(fm_msngr_CXXFLAGS)
fm_msngr_LDFLAGS = @FM_MSNGR_BUILD_LDFLAGS@
fm_msngr_LDADD = @FM_MSNGR_BUILD_LDADD@

MINGW32_SRC = fm_msngr-rc.rc include/fm_msngrrc.h \
	utils/mingw.c include/mingw.h include/compat.h \
	utils/getsysinfo.c

# We distribute these but do not always compile them
EXTRA_fm_msngr_SOURCES = $(fm_msngr_WIN32_RES_SRC) $(MINGW32_SRC)

fm_msngr_SOURCES =

if MINGW32
if HAVE_WINDRES
  fm_msngr_SOURCES += $(MINGW32_SRC)
endif
endif

########################################################################


fm_msngr_VERSION_MAJOR = @FM_MSNGR_VERSION_MAJOR@
fm_msngr_VERSION_MINOR = @FM_MSNGR_VERSION_MINOR@
fm_msngr_VERSION_PATCH = @FM_MSNGR_VERSION_PATCH@
fm_msngr_VERSION = @FM_MSNGR_VERSION@

.EXPORT_ALL_VARIABLES: nsisinst appbundle

# Sources that are generated,
BUILT_SOURCES =
# not distributed,
nodist_fm_msngr_SOURCES = $(BUILT_SOURCES)
# and deleted by the clean targets
CLEANFILES = $(BUILT_SOURCES)
CLEAN_LOCAL =

if MINGW32
if HAVE_WINDRES
.rc.o:
	$(WINDRES) -Iinclude -I../data/win32 $< -O coff $@
endif
endif

install-data-local:
	if test -f ../data/fm_msngr.xpm; then \
	    $(mkinstalldirs) $(DESTDIR)/$(datadir)/pixmaps; \
	    $(INSTALL_DATA) ../data/fm_msngr.xpm $(DESTDIR)/$(datadir)/pixmaps; \
	fi
	if test -f ../data/fm_msngr.desktop; then \
	    $(mkinstalldirs) $(DESTDIR)/$(datadir)/applications; \
	    $(INSTALL_DATA) ../data/fm_msngr.desktop $(DESTDIR)/$(datadir)/applications; \
	fi

uninstall-local:
	rm -f $(DESTDIR)/$(datadir)/pixmaps/fm_msngr.xpm
	rm -f $(DESTDIR)/$(datadir)/applications/fm_msngr.desktop

# FL_SRC = fm_msngr_dialog.fl

if HAVE_FLUID
flgen: $(FL_SRC)
	(cd include; \
	for f in $(FL_SRC); do \
	    c=$${f%.fl}.cxx; \
		h=$${f%.fl}.h; \
		h=$${h##*/}; \
	    $(FLUID) -c -o ../$$c -h $$h ../$$f; \
	done)
endif


INSTALLER_FILE = fm_msngr-$(fm_msngr_VERSION)_setup.exe
APPBUNDLE=fm_msngr-$(fm_msngr_VERSION)
APPBUNDLE_NOLIBS=$(APPBUNDLE)-nolibs

if HAVE_NSIS
nsisinst: $(bin_PROGRAMS)
	sh ../scripts/mknsisinst.sh "../data" .
    CLEANFILES += $(INSTALLER_FILE)
endif

if DARWIN
appbundle: $(bin_PROGRAMS)
	sh ../scripts/mkappbundle.sh "../data" .
    CLEAN_LOCAL += $(APPBUNDLE_NOLIBS) $(APPBUNDLE) $(APPBUNDLE)*.dmg
endif

clean-local:
	-rm -rf $(CLEAN_LOCAL)

# Sources that we build. It is OK to have headers here.
fm_msngr_SOURCES += \
	fm_msngr.cxx \
	fm_msngr_src/fm_dialog.cxx \
	fm_msngr_src/fm_control.cxx \
	fm_msngr_src/fm_dialog.h \
	fm_msngr_src/fm_control.h \
	fm_msngr_src/tts_translate_table.cxx \
	fm_msngr_src/tts_translate_table.h \
	fm_msngr_src/fm_control_tts.cxx \
	fm_msngr_src/fm_control_tts.h \
	fm_msngr_src/fm_config_tts.cxx \
	fm_msngr_src/fm_config_tts.h \
	fm_msngr_src/call_table.cxx \
	fm_msngr_src/call_table.h \
	fm_msngr_src/fm_config.cxx \
	fm_msngr_src/fm_config.h \
	fm_msngr_src/fm_port_io.cxx \
	fm_msngr_src/fm_port_io.h \
	fm_msngr_src/log_csv_config.cxx \
	fm_msngr_src/log_csv_config.h \
	fm_msngr_src/log_csv_control.cxx \
	fm_msngr_src/log_csv_control.h \
	fm_msngr_src/display_list_config.cxx \
	fm_msngr_src/display_list_config.h \
	fm_msngr_src/display_list_control.cxx \
	fm_msngr_src/display_list_control.h \
	fm_msngr_src/fm_ncs_config.cxx \
	fm_msngr_src/fm_ncs_config.h \
	fm_msngr_src/fm_ncs_control.cxx \
	fm_msngr_src/fm_ncs_control.h \
	fm_msngr_src/Fl_Text_Display_FM.cxx \
	fm_msngr_src/Fl_Text_Editor_FM.cxx \
	fm_msngr_src/Fl_Text_Display_FM.h \
	fm_msngr_src/Fl_Text_Editor_FM.h \
	utils/debug.cxx \
	utils/socket.cxx \
	utils/status.cxx \
	utils/util.cxx \
	utils/file_io.cxx \
	utils/xml_io.cxx \
	utils/threads.cxx \
	widgets/calendar.cxx \
	widgets/combo.cxx \
	widgets/date.cxx \
	widgets/fileselect.cxx \
	widgets/flinput2.cxx \
	widgets/flslider2.cxx \
	widgets/Fl_BlockMap.cxx \
	widgets/icons.cxx \
	widgets/missing_strings.c \
	widgets/pixmaps.cxx \
	widgets/pixmaps_tango.cxx \
	widgets/font_browser.cxx \
	xmlrpcpp/XmlRpc.h \
	xmlrpcpp/XmlRpcClient.cpp \
	xmlrpcpp/XmlRpcClient.h \
	xmlrpcpp/XmlRpcDispatch.cpp \
	xmlrpcpp/XmlRpcDispatch.h \
	xmlrpcpp/XmlRpcException.h \
	xmlrpcpp/XmlRpcMutex.cpp \
	xmlrpcpp/XmlRpcMutex.h \
	xmlrpcpp/XmlRpcServer.cpp \
	xmlrpcpp/XmlRpcServer.h \
	xmlrpcpp/XmlRpcServerConnection.cpp \
	xmlrpcpp/XmlRpcServerConnection.h \
	xmlrpcpp/XmlRpcServerMethod.cpp \
	xmlrpcpp/XmlRpcServerMethod.h \
	xmlrpcpp/XmlRpcSocket.cpp \
	xmlrpcpp/XmlRpcSocket.h \
	xmlrpcpp/XmlRpcSource.cpp \
	xmlrpcpp/XmlRpcSource.h \
	xmlrpcpp/XmlRpcThread.cpp \
	xmlrpcpp/XmlRpcThread.h \
	xmlrpcpp/XmlRpcThreadedServer.cpp \
	xmlrpcpp/XmlRpcThreadedServer.h \
	xmlrpcpp/XmlRpcUtil.cpp \
	xmlrpcpp/XmlRpcUtil.h \
	xmlrpcpp/XmlRpcValue.cpp \
	xmlrpcpp/XmlRpcValue.h \
	xmlrpcpp/Xmlbase64.h

# Additional source files that are distributed
EXTRA_DIST = \
	config.h \
	utils/mingw.c \
	widgets/flnfc_common.cxx \
	include/calendar.h \
	include/combo.h \
	include/compat.h \
	include/compat-mingw.h \
	include/date.h \
	include/debug.h \
	include/dirent-check.h \
	include/fileselect.h \
	include/fl_lock.h \
	include/flmisc.h \
	include/fm_msngrrc.h \
	include/flinput2.h \
	include/flslider2.h \
	include/font_browser.h \
	include/gettext.h \
	include/icons.h \
	include/fm_msngr.h \
	include/missing_strings.h \
	include/socket.h \
	include/mingw.h \
	include/pixmaps.h \
	include/status.h \
	include/threads.h \
	include/util.h \
	include/file_io.h \
	include/xml_io.h \
	include/Fl_BlockMap.h \
	../ChangeLog \
	../data/fm_msngr.desktop \
	../data/fm_msngr.xpm \
	../data/fm_msngr.png \
	../data/win32/fl_app.nsi \
	../data/win32/fm_msngr.ico \
	../scripts/build4mingw.sh \
	../scripts/builddist.sh \
	../scripts/mkappbundle.sh \
	../scripts/mknsisinst.sh \
	../data/mac/Info.plist.in \
	../data/mac/fm_msngr.icns \
	$(fm_msngr_WIN32_SRC) \
	$(fm_msngr_FL_SRC)
