TESTS = test
check_PROGRAMS = $(TESTS)
test_SOURCES = test_main.cc model_manager_test.cc
test_LDADD = $(CPPUNIT_LIBS) -llog4cxx ../../main/ranking/libranking.a ../../main/index/libindex.a

AM_CPPFLAGS = $(CPPUNIT_CFLAGS) -I$(srcdir) -I$(srcdir)/.. -I$(srcdir)/../../main
