SUBDIRS = handler index model parser pipeline query ranking service

bin_PROGRAMS = redgiant
redgiant_SOURCES = main.cc
redgiant_LDADD = handler/libhandler.a service/libservice.a pipeline/libpipeline.a index/libindex.a query/libquery.a ranking/libranking.a parser/libparser.a model/libmodel.a -levent -llog4cxx

AM_CPPFLAGS = -I$(srcdir)

