SUBDIRS = plib lib 

if JSON
SUBDIRS += libpsensor_json
endif

if LIBMICROHTTPD
if JSON
if LUA
SUBDIRS += server
endif
endif
endif

AM_CPPFLAGS = -pedantic -Werror -DDEFAULT_WWW_DIR=\""$(pkgdatadir)/www"\"\
	-I$(top_srcdir)/src/lib \
	$(GTK_CFLAGS)\
	$(GCONF_CFLAGS)\
	$(SENSORS_CFLAGS)

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

LIBS = \
	plib/libplib.a \
	lib/libpsensor.a \
	$(GTK_LIBS)\
	$(GCONF_LIBS)\
	$(SENSORS_LIBS)

bin_PROGRAMS = psensor
psensor_SOURCES = main.c\
	graph.c\
	ui_sensorlist.h\
	graph.h\
	ui_appindicator.h\
	ui_sensorpref.c\
	compat.h\
	ui_color.c\
	ui_pref.c\
	ui_sensorpref.h\
	cfg.c\
	ui_color.h\
	ui_pref.h\
	cfg.h\
	rsensor.h\
	ui.h ui.c\
	ui_sensorlist.c

if LIBNOTIFY
psensor_SOURCES += ui_notify.h ui_notify.c
LIBS += $(LIBNOTIFY_LIBS)
AM_CPPFLAGS += $(LIBNOTIFY_CFLAGS)
endif


if APPINDICATOR
psensor_SOURCES += ui_appindicator.c
LIBS +=	$(APPINDICATOR_LIBS)
AM_CPPFLAGS += $(APPINDICATOR_CFLAGS)
endif

if NVIDIA
AM_CPPFLAGS += $(NVIDIA_CFLAGS)
LIBS +=  $(NVIDIA_LIBS)
endif

if CURL
if JSON
psensor_SOURCES += rsensor.c plib/url.c plib/url.h
LIBS += $(CURL_LIBS) $(JSON_LIBS)
AM_CPPFLAGS += $(CURL_CFLAGS) $(JSON_CFLAGS)
endif
endif


dist_man_MANS = psensor.1

EXTRA_DIST = description.txt

psensor.1: main.c $(top_srcdir)/configure.ac
	$(MAKE) $(AM_MAKEFLAGS) psensor$(EXEEXT)
	help2man --include=description.txt -N --name="Temperature monitoring application" --output=psensor.1 ./psensor$(EXEEXT)

