TESTS = test
check_PROGRAMS = $(TESTS)
test_SOURCES = test_main.cc default_model_test.cc feature_mapping_model_test.cc model_manager_test.cc
test_LDADD = $(CPPUNIT_LIBS) -llog4cxx ../../main/ranking/libranking.a ../../main/data/libdata.a

AM_CPPFLAGS = $(CPPUNIT_CFLAGS) -I$(srcdir) -I$(srcdir)/.. -I$(srcdir)/../../main
