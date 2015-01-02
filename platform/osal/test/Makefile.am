include $(top_srcdir)/osal.am

LIB_OSAL=../posix_src/libosalposix.a ../src/libosal.a
testdir=test
#bin_PROGRAMS =  

test_PROGRAMS = coov3test opentest utf8test threadtest mutextest singlelinkedlisttest stack printtest alltests

alltests_SOURCES = testtest.c testoe.c testcase.c
alltests_LDADD = $(LIB_OSAL)	

coov3test_SOURCES = coov3test.c
coov3test_LDADD = $(LIB_OSAL)

printtest_SOURCES = printtest.c
printtest_LDADD = $(LIB_OSAL) 
#-lrt

stack_SOURCES = stack.c
stack_LDADD = $(LIB_OSAL) 
#-lrt

singlelinkedlisttest_SOURCES = singlelinkedlisttest.c
singlelinkedlisttest_LDADD = $(LIB_OSAL) 
#-lrt

mutextest_SOURCES = mutextest.c
mutextest_LDADD = $(LIB_OSAL) 
#-lrt

threadtest_SOURCES = threadtest.c
threadtest_LDADD = $(LIB_OSAL) 
#-lrt 

opentest_SOURCES = opentest.c
opentest_LDADD = $(LIB_OSAL) 
#-lrt

utf8test_SOURCES = utf8test.c
utf8test_LDADD = $(LIB_OSAL) 
#-lrt
