TESTS = test
check_PROGRAMS = $(TESTS)
test_SOURCES = test_main.cc
test_LDADD = $(CPPUNIT_LIBS) -llog4cxx

AM_CPPFLAGS = $(CPPUNIT_CFLAGS) -I$(srcdir) -I$(srcdir)/.. -I$(srcdir)/../../main
