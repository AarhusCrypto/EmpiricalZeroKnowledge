define([svnversion], esyscmd([sh -c "svnversion|tr -d '\n'"]))dnl
define([buildtime], esyscmd([sh -c "date +\"build:%Y-%m-%d %H:%M:%S\"|tr -d '\n'"]))dnl
AC_DEFINE(SVN_REVISION, "svnversion", [SVN Revision])
AC_DEFINE(BUILD_TIME,"buildtime",[Build Time])dnl
AC_DEFINE(CODENAME, "CircuitTree", [CODE Name])
AC_INIT([rtz14],[1.0])
AC_CONFIG_SRCDIR([src/rtz14.c])
AM_INIT_AUTOMAKE
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
AC_PROG_CC
AM_PROG_AS
AC_PROG_RANLIB

AC_CHECK_LIB([pthread],[pthread_create],,[AC_MSG_ERROR([Could not find pthread])],)
AC_CHECK_LIB([osal],[l2b],,[AC_MSG_ERROR([Could not find OSal])],-lpthread)
AC_CHECK_LIB([osalposix],[OperatingEnvironment_New],,[AC_MSG_ERROR([Could not find OSalPosix])],-lpthread)
AC_CHECK_LIB([datetime],[DateTime_New],,[AC_MSG_ERROR([Could not find DateTime])], -losalposix -losal -lpthread)
AC_CHECK_LIB([encoding],[hs2bs],,[AC_MSG_ERROR([Could not find Encoding])],-lds -losalposix -losal -lpthread)
AC_CHECK_LIB([ds],[HashMap_new],,[AC_MSG_ERROR([Could not find Data Structures])],-losalposix -losal -lpthread)
AC_CHECK_LIB([utils],[Options_New],,[AC_MSG_ERROR([Could not find Utilities])],-lds -losalposix -losal -lpthread)
AC_CHECK_LIB([carena],[CArena_new],,[AC_MSG_ERROR([Could not find CArena])],-lutils -lencoding -lds -losalposix -losal -lpthread)



# List the makefiles that configure should care about creating (in cooperation with automake)
AC_OUTPUT([Makefile 
          include/Makefile
          src/Makefile
	  test/Makefile
          ])


