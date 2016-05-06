TESTS = test
check_PROGRAMS = $(TESTS)
test_SOURCES = test_main.cc document_parser_test.cc
test_LDADD = $(CPPUNIT_LIBS) -llog4cxx ../../main/parser/libparser.a ../../main/model/libmodel.a

AM_CPPFLAGS = $(CPPUNIT_CFLAGS) -I$(srcdir) -I$(srcdir)/.. -I$(srcdir)/../../main
