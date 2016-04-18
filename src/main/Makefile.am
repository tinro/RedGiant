bin_PROGRAMS = redgiant
redgiant_SOURCES = main.cc
redgiant_LDADD = handler/libhandler.a service/libservice.a utils/libutils.a

SUBDIRS = handler service utils

AM_CPPFLAGS = -I$(srcdir)
AM_LDFLAGS = -levent -llog4cxx

