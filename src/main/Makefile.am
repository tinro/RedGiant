SUBDIRS = handler model parser service

bin_PROGRAMS = redgiant
redgiant_SOURCES = main.cc
redgiant_LDADD = handler/libhandler.a model/libmodel.a parser/libparser.a service/libservice.a

AM_CPPFLAGS = -I$(srcdir)
AM_LDFLAGS = -levent -llog4cxx

