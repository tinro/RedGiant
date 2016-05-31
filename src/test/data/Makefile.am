TESTS = test
check_PROGRAMS = $(TESTS)
test_SOURCES = test_main.cc document_id_test.cc document_parser_test.cc feature_space_test.cc feature_space_manager_test.cc
test_LDADD = $(CPPUNIT_LIBS) -llog4cxx ../../main/data/libdata.a

AM_CPPFLAGS = $(CPPUNIT_CFLAGS) -I$(srcdir) -I$(srcdir)/.. -I$(srcdir)/../../main
