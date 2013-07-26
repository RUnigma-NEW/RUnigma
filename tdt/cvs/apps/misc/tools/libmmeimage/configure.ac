AC_INIT(libmmeimage,1-0,,libmmeimage-1.0)
AM_INIT_AUTOMAKE(libmmeimage,1.0)

TUXBOX_APPS

AC_PROG_CC
AC_PROG_CXX
AM_PROG_LIBTOOL

AC_ARG_ENABLE([multicom406], 
        [AS_HELP_STRING(--enable-multicom406,    enable multicom406 stuff)], 
        [case "${enableval}" in 
                yes) enable_multicom406=true ;; 
                no) enable_multicom406=false ;; 
                *) AC_MSG_ERROR([bad value ${enableval} for --enable-multicom406]) ;; 
        esac]) 
AM_CONDITIONAL([ENABLE_MULTICOM406], [test x$enable_multicom406 = xtrue])
AC_SUBST(MULTICOM406)

AC_ARG_ENABLE([multicom324], 
        [AS_HELP_STRING(--enable-multicom324,    enable multicom324 stuff)], 
        [case "${enableval}" in 
                yes) enable_multicom324=true ;; 
                no) enable_multicom324=false ;; 
                *) AC_MSG_ERROR([bad value ${enableval} for --enable-multicom324]) ;; 
        esac]) 
AM_CONDITIONAL([ENABLE_MULTICOM324], [test x$enable_multicom324 = xtrue])
AC_SUBST(MULTICOM324)



AC_OUTPUT([
Makefile
])
