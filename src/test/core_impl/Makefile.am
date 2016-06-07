TESTS = test
check_PROGRAMS = $(TESTS)
test_SOURCES = test_main.cc base_index_impl_test.cc expire_table_test.cc freezable_posting_list_test.cc
test_LDADD = $(CPPUNIT_LIBS) -llog4cxx

AM_CPPFLAGS = $(CPPUNIT_CFLAGS) -I$(srcdir) -I$(srcdir)/.. -I$(srcdir)/../../main
