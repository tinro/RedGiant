SUBDIRS = data handler index query ranking service

bin_PROGRAMS = redgiant
redgiant_SOURCES = main.cc
redgiant_LDADD = handler/libhandler.a service/libservice.a query/libquery.a ranking/libranking.a index/libindex.a data/libdata.a -levent -llog4cxx

AM_CPPFLAGS = -I$(srcdir)

