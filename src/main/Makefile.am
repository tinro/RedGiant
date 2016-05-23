SUBDIRS = data handler index parser pipeline query ranking service

bin_PROGRAMS = redgiant
redgiant_SOURCES = main.cc
redgiant_LDADD = handler/libhandler.a service/libservice.a pipeline/libpipeline.a query/libquery.a ranking/libranking.a index/libindex.a parser/libparser.a data/libdata.a -levent -llog4cxx

AM_CPPFLAGS = -I$(srcdir)

