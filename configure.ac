dnl configure.ac for srcpd
dnl Revision: $Revision: 1790 $
dnl Last update: $Date: 2024-05-03 08:55:23 +0200 (Fr, 03. Mai 2024) $

dnl Process this file with autoconf (>=2.60) to produce a configure script.

AC_INIT([srcpd], [2.1.7], [srcpd-devel@lists.sourceforge.net])
AC_CANONICAL_HOST
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_SRCDIR([src/srcpd.c])

AM_INIT_AUTOMAKE([dist-bzip2 dist-zip dist-xz])

AC_PREFIX_DEFAULT(/usr/local)

dnl this is from scmxx
AH_TEMPLATE(
    [NO_CFMAKERAW],
    [define this to include own cfmakeraw()]
)
AC_CHECK_FUNC(
    cfmakeraw,
    [],
    [AC_DEFINE(NO_CFMAKERAW)]
)

dnl Checks for programs.
dnl AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CPP
AC_PROG_INSTALL

dnl counter for number of configured device modules
NUM_MODULES=0

dnl Option to disable DDL support
AC_ARG_ENABLE(ddl,
    AS_HELP_STRING([--disable-ddl], [do not compile ddl module]),
    [with_DLL="$enableval"], [with_DDL=yes])
if test "$with_DDL" = "yes"; then
   AC_DEFINE(USE_DDL,1, [define to 1 if you want ddl support])
   NUM_MODULES=`expr $NUM_MODULES + 1`
fi
AM_CONDITIONAL(USE_DDL, test "$with_DDL" = yes)

dnl Option to disable DDL S88 support
AC_ARG_ENABLE(ddls88,
    AS_HELP_STRING([--disable-ddls88], [do not compile ddl-s88 module]),
    [with_DLL88="$enableval"], [with_DDLS88=yes])
if test "$with_DDLS88" = "yes"; then
   AC_DEFINE(USE_DDLS88,1, [define to 1 if you want ddl-s88 support])
   NUM_MODULES=`expr $NUM_MODULES + 1`
fi
AM_CONDITIONAL(USE_DDLS88, test "$with_DDLS88" = yes)

dnl Option to disable HSI S88 support
AC_ARG_ENABLE(hsi88,
    AS_HELP_STRING([--disable-hsi88], [do not compile hsi-88 module]),
    [with_HSI88="$leval"], [with_HSI88=yes])
if test "$with_HSI88" = "yes"; then
   AC_DEFINE(USE_HSI88,1,[define to 1 if you want hsi-88 support])
   NUM_MODULES=`expr $NUM_MODULES + 1`
fi
AM_CONDITIONAL(USE_HSI88, test "$with_HSI88" = yes)

dnl Option to disable I2C support
AC_ARG_ENABLE(i2cdev,
    AS_HELP_STRING([--disable-i2cdev], [do not compile i2c-dev module]),
    [with_I2C="$enableval"], [with_I2C=yes])
if test "$with_I2C" = "yes"; then
   AC_DEFINE(USE_I2C,1, [define to 1 if you want i2c-dev support])
   NUM_MODULES=`expr $NUM_MODULES + 1`
fi
AM_CONDITIONAL(USE_I2C, test "$with_I2C" = yes)

dnl Option to disable Intellibox support
AC_ARG_ENABLE(intellibox,
    AS_HELP_STRING([--disable-intellibox], [do not compile intellibox module]),
    [with_IB="$enableval"], [with_IB=yes])
if test "$with_IB" = "yes"; then
   AC_DEFINE(USE_IB,1,[define to 1 if you want intellibox support])
   NUM_MODULES=`expr $NUM_MODULES + 1`
fi
AM_CONDITIONAL(USE_IB, test "$with_IB" = yes)

dnl Option to disable Lenz support
AC_ARG_ENABLE(li100,
    AS_HELP_STRING([--disable-li100], [do not compile li100 module]),
    [with_LI100="$enableval"], [with_LI100=yes])
if test "$with_LI100" = "yes"; then
   AC_DEFINE(USE_LI100,1,[define to 1 if you want li100 (RS232 + USB) support])
   NUM_MODULES=`expr $NUM_MODULES + 1`
fi
AM_CONDITIONAL(USE_LI100, test "$with_LI100" = yes)

dnl Option to disable Loconet support
AC_ARG_ENABLE(loconet,
    AS_HELP_STRING([--disable-loconet], [do not compile loconet module]),
    [with_LOCONET="$enableval"], [with_LOCONET=yes])
if test "$with_LOCONET" = "yes"; then
   AC_DEFINE(USE_LOCONET,1, [define to 1 if you want loconet support])
   NUM_MODULES=`expr $NUM_MODULES + 1`
fi
AM_CONDITIONAL(USE_LOCONET, test "$with_LOCONET" = yes)

dnl Option to disable loopback support
AC_ARG_ENABLE(loopback,
    AS_HELP_STRING([--disable-loopback], [do not compile loopback module]),
    [with_LOOPBACK="$enableval"], [with_LOOPBACK=yes])
if test "$with_LOOPBACK" = "yes"; then
   AC_DEFINE(USE_LOOPBACK,1, [define to 1 if you want loopback support])
   NUM_MODULES=`expr $NUM_MODULES + 1`
fi
AM_CONDITIONAL(USE_LOOPBACK, test "$with_LOOPBACK" = yes)

dnl Option to disable M605X support
AC_ARG_ENABLE(m605x,
    AS_HELP_STRING([--disable-m605x], [do not compile m605x module]),
    [with_M605X="$enableval"], [with_M605X=yes])
if test "$with_M605X" = "yes"; then
   AC_DEFINE(USE_M605X,1, [define to 1 if you want m605x support])
   NUM_MODULES=`expr $NUM_MODULES + 1`
fi
AM_CONDITIONAL(USE_M605X, test "$with_M605X" = yes)

dnl Option to disable Selectrix support
AC_ARG_ENABLE(selectrix,
    AS_HELP_STRING([--disable-selectrix], [do not compile selectrix module]),
    [with_SELECTRIX="$enableval"], [with_SELECTRIX=yes])
if test "$with_SELECTRIX" = "yes"; then
   AC_DEFINE(USE_SELECTRIX,1,[define to 1 if you want selectrix support])
   NUM_MODULES=`expr $NUM_MODULES + 1`
fi
AM_CONDITIONAL(USE_SELECTRIX, test "$with_SELECTRIX" = yes)

dnl Option to disable Zimo support
AC_ARG_ENABLE(zimo,
    AS_HELP_STRING([--disable-zimo], [do not compile zimo module]),
    [with_ZIMO="$enableval"], [with_ZIMO=yes])
if test "$with_ZIMO" = "yes"; then
   AC_DEFINE(USE_ZIMO,1, [define to 1 if you want zimo support])
   NUM_MODULES=`expr $NUM_MODULES + 1`
fi
AM_CONDITIONAL(USE_ZIMO, test "$with_ZIMO" = yes)

dnl Option to disable DC-Car support
AC_ARG_ENABLE(dccar,
    AS_HELP_STRING([--disable-dccar], [do not compile dccar module]),
    [with_DCCAR="$enableval"], [with_DCCAR=yes])
if test "$with_DCCAR" = "yes"; then
   AC_DEFINE(USE_DCCAR,1, [define to 1 if you want dccar support])
   NUM_MODULES=`expr $NUM_MODULES + 1`
fi
AM_CONDITIONAL(USE_DCCAR, test "$with_DCCAR" = yes)

dnl test for getaddrinfo, not available under Cygwin but necessary for Loconet
if test "$with_LOCONET" = "yes"; then
    AC_SEARCH_LIBS(getaddrinfo, inet6,
        [AC_DEFINE(HAVE_GETADDRINFO,1,
            [define to 1 if you have the getaddrinfo function])],
        [AC_MSG_ERROR(getaddrinfo support missing)])
fi


dnl Checks for host dependend libraries.
case $host in
  *-*-linux*)
    ;;
  *-*-cygwin*)
    if test "$with_DDLS88" = "yes"; then
      AC_CHECK_LIB(ioperm, ioperm, , AC_MSG_ERROR(lib ioperm is missing))
    fi
    ;;
  *-*-freebsd*)
    ;;
  *)
    AC_MSG_WARN(System $host not known.)
    ;;
esac


dnl Checks for header files.
AC_CHECK_HEADERS(fcntl.h syslog.h errno.h pthread.h pwd.h sys/time.h \
                 signal.h termios.h netdb.h, ,
                 AC_MSG_ERROR(essential header file is missing))

AC_SYS_POSIX_TERMIOS
if test "x$ac_cv_sys_posix_termios" = "xno"; then
    AC_MSG_ERROR(No POSIX termios on this system)
fi

dnl Checks for host dependend header files.
case $host in
  *-*-linux*)
    if test "$with_DDLS88" = "yes"; then
          AC_CHECK_HEADERS(sys/io.h, ,
                       AC_MSG_ERROR(essential header file is missing))
    fi
    AC_CHECK_HEADERS(linux/serial.h, ,
                 AC_MSG_ERROR(essential header file is missing))
    if test "$with_I2C" = "yes"; then
    AC_CHECK_HEADERS(linux/i2c-dev.h, ,
                 AC_MSG_ERROR(essential header file is missing))
    fi
    ;;
  *-*-cygwin*)
    AC_CHECK_HEADERS(sys/io.h, ,
                 AC_MSG_ERROR(essential header file is missing))
    ;;
  *-*-freebsd*)
    AC_CHECK_HEADERS(dev/ppbus/ppi.h, ,
                 AC_MSG_ERROR(essential header file is missing))
    ;;
  *-*-sunos*) 
    AC_CHECK_HEADERS(sys/fileo.h,,
                 AC_MSG_ERROR(essential header file is missing))
    ;;
  *)
    ;;
esac

dnl Check for types possibly not defined in header files.
AC_TYPE_PID_T
AC_TYPE_SIGNAL

dnl Checks for structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

AC_MSG_CHECKING([for type of build])
AC_ARG_ENABLE(debug,
        AS_HELP_STRING([--enable-debug], [enable debug mode [default=no]]),
 enable_debug=$enableval, enable_debug=no)
if test "x$enable_debug" = "xyes"; then
   CFLAGS="$CFLAGS -g -Wall -Wextra -Wunused-variable -DDEBUG"
   AC_MSG_RESULT(debug)
else
   if test "x$CFLAGS" = "x"; then
      CFLAGS="-O2 -Wall -Wextra"
   fi
   AC_MSG_RESULT(release)
fi

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_CHECK_FUNCS(gettimeofday socket)

AM_PATH_XML2([2.4.0],
             [CPPFLAGS="$XML_CPPFLAGS $CPPFLAGS" LIBS="$XML_LIBS $LIBS"],
             AC_MSG_ERROR(no matching library found))

ACX_PTHREAD([LIBS="$PTHREAD_LIBS $LIBS"
             CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
             CC="$PTHREAD_CC"],
             AC_MSG_ERROR(threads library not found))

dnl Checks for host dependend library functions.
case $host in
  *-*-linux*)
    if test "$with_DDLS88" = "yes"; then
      AC_CHECK_FUNC(ioperm, , AC_MSG_ERROR(support missing))
    fi
    ;;
  *-*-cygwin*)
    if test "$with_DDLS88" = "yes"; then
      AC_CHECK_FUNC(ioperm, , AC_MSG_ERROR(support missing))
    fi
    ;;
  *-*-freebsd*)
    ;;
  *)
    ;;
esac

AC_MSG_CHECKING([whether to enable ipv6])
AC_ARG_ENABLE(ipv6,
        AS_HELP_STRING([--enable-ipv6], [enable IPv6 extensions]),
[ case "$enableval" in
        no)
                AC_MSG_RESULT(no)
                ipv6=no
                ;;
        *)
                AC_MSG_RESULT(yes)
                AC_DEFINE(ENABLE_IPV6, [1], [Activate IP V6])
                ipv6=yes
                ;;
  esac ],
AC_TRY_RUN([ /* AF_INET6 available check */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
int main()
{
 if (socket(AF_INET6, SOCK_STREAM, 0) < 0)
   exit(1);
 else
   exit(0);
}
],
  AC_MSG_RESULT(yes)
  AC_DEFINE(ENABLE_IPV6, [1], [Activate IP V6])
  ipv6=yes,
  AC_MSG_RESULT(no)
  ipv6=no,
  AC_MSG_RESULT(no)
  ipv6=no
))

AC_CONFIG_FILES(Makefile src/Makefile man/Makefile man/de/Makefile)
AC_OUTPUT

echo
echo "Supported devices:"
if test "$with_DDL"       = "yes"; then echo "    ddl"; fi
if test "$with_DDLS88"    = "yes"; then echo "    ddl-s88"; fi
if test "$with_HSI88"     = "yes"; then echo "    hsi-88"; fi
if test "$with_I2C"       = "yes"; then echo "    i2c-dev"; fi
if test "$with_IB"        = "yes"; then echo "    intellibox"; fi
if test "$with_LI100"     = "yes"; then echo "    li100"; fi
if test "$with_LOCONET"   = "yes"; then echo "    loconet"; fi
if test "$with_LOOPBACK"  = "yes"; then echo "    loopback"; fi
if test "$with_M605X"     = "yes"; then echo "    m605x"; fi
if test "$with_SELECTRIX" = "yes"; then echo "    selectrix"; fi
if test "$with_ZIMO"      = "yes"; then echo "    zimo"; fi
if test "$with_DCCAR"     = "yes"; then echo "    dccar"; fi
echo
echo "Unsupported devices:"
if test "$with_DDL"       != "yes"; then echo "    ddl"; fi
if test "$with_DDLS88"    != "yes"; then echo "    ddl-s88"; fi
if test "$with_HSI88"     != "yes"; then echo "    hsi-88"; fi
if test "$with_I2C"       != "yes"; then echo "    i2c-dev"; fi
if test "$with_IB"        != "yes"; then echo "    intellibox"; fi
if test "$with_LI100"     != "yes"; then echo "    li100"; fi
if test "$with_LOCONET"   != "yes"; then echo "    loconet"; fi
if test "$with_LOOPBACK"  != "yes"; then echo "    loopback"; fi
if test "$with_M605X"     != "yes"; then echo "    m605x"; fi
if test "$with_SELECTRIX" != "yes"; then echo "    selectrix"; fi
if test "$with_ZIMO"      != "yes"; then echo "    zimo"; fi
if test "$with_DCCAR"     != "yes"; then echo "    dccar"; fi

echo
if test $NUM_MODULES == 0; then
    echo 'Warning: No device module configured; srcpd will not run properly!'
else
    echo "Run 'make' to continue."
fi
echo
