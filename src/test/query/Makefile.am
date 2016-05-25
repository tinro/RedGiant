TESTS = test
check_PROGRAMS = $(TESTS)
test_SOURCES = test_main.cc simple_query_executor_test.cc
test_LDADD = $(CPPUNIT_LIBS) -llog4cxx ../../main/query/libquery.a ../../main/ranking/libranking.a ../../main/index/libindex.a ../../main/data/libdata.a

AM_CPPFLAGS = $(CPPUNIT_CFLAGS) -I$(srcdir) -I$(srcdir)/.. -I$(srcdir)/../../main
