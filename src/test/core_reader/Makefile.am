TESTS = test
check_PROGRAMS = $(TESTS)
test_SOURCES = test_main.cc dot_product_reader_test.cc wand_reader_test.cc

AM_CPPFLAGS = $(CPPUNIT_CFLAGS) -I$(srcdir) -I$(srcdir)/.. -I$(srcdir)/../../main
AM_LDFLAGS = $(CPPUNIT_LIBS) -llog4cxx
