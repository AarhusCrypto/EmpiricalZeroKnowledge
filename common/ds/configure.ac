AC_INIT([ds],[1.0])
AC_CONFIG_SRCDIR([src/hashmap.c])
AM_INIT_AUTOMAKE
AC_PREREQ(2.12)
AM_CONFIG_HEADER(config.h:config.in)
AC_CHECK_LIB([osal],[l2b,b2l,zeromem,mcpy,mcmp,osal_strlen,osal_sprintf],,
[AC_MSG_ERROR([Could not find osal])],[])
AC_CHECK_LIB([osalposix],[OperatingEnvironment_LinuxNew],,[AC_MSG_ERROR([Could not find OSAL])],[-losal -lpthread])
AC_PROG_CC
AC_PROG_RANLIB

AC_OUTPUT([Makefile 
          headers/Makefile
          src/Makefile
	      test/Makefile
          ])


