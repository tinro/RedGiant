TESTS = test
check_PROGRAMS = $(TESTS)
test_SOURCES = test_main.cc event_request_context_test.cc event_response_writer_test.cc server_test.cc
test_LDADD = ../../main/service/libservice.a ../../main/utils/libutils.a

AM_CPPFLAGS = $(CPPUNIT_CFLAGS) -I$(srcdir) -I$(srcdir)/.. -I$(srcdir)/../../main
AM_LDFLAGS = $(CPPUNIT_LIBS) -levent -llog4cxx
