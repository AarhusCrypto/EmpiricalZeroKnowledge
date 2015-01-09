include $(top_srcdir)/carena.am

bin_PROGRAMS = test tests gracefulshutdown testsend simpl	testsendsrv testsendcli threeway


threeway_SOURCES = threeway.c
threeway_LDADD = $(CARENA_LIB) 

testsendsrv_SOURCES = testsendsrv.c
testsendsrv_LDADD = $(CARENA_LIB) 

testsendcli_SOURCES = testsendcli.c
testsendcli_LDADD = $(CARENA_LIB) 

simpl_SOURCES = simple.c
simpl_LDADD = $(CARENA_LIB) 	

testsend_SOURCES = testsend.c
testsend_LDADD = $(CARENA_LIB) 

gracefulshutdown_SOURCES = gracefulshutdown.c
gracefulshutdown_LDADD = $(CARENA_LIB) 

tests_SOURCES = tests.c
tests_LDADD = $(CARENA_LIB) 

test_SOURCES = test.c
test_LDADD = $(CARENA_LIB) 
