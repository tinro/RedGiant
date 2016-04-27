TESTS = test
check_PROGRAMS = $(TESTS)
test_SOURCES = test_main.cc cached_buffer_test.cc
test_LDADD =

AM_CPPFLAGS = $(CPPUNIT_CFLAGS) -I$(srcdir) -I$(srcdir)/.. -I$(srcdir)/../../main
AM_LDFLAGS = $(CPPUNIT_LIBS) -llog4cxx
