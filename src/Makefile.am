## Process this file with automake to produce Makefile.in

GIT_VERSION = $(shell sh -c 'git describe --abbrev=4 --dirty --always')

ACLOCAL_AMFLAGS = -I ../m4

lib_LTLIBRARIES = libgframe.la

libgframe_includedir = $(includedir)/gframe

libgframe_include_HEADERS = \
        include/database.h \
        include/configreader.h \
	include/glib.h \
	include/output.h \
	include/mainbase.h

libgframe_la_SOURCES = \
        configreader.cpp \
        glib.cpp \
        output.cpp \
        mainbase.cpp

CPPFLAGS += $(GFRAME_CPPFLAGS)
LDFLAGS += $(GFRAME_LDFLAGS)
LIBS += $(GFRAME_LIBS)

