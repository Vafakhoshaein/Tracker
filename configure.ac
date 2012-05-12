#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([tracker], [1.0], [vafakhoshaein@gmail.com])
AC_CONFIG_SRCDIR([src/gui/mainwindow.cpp])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])


# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor memset sqrt strdup])

# Check for opencv and glib 
PKG_CHECK_MODULES([OPENCV], opencv >= 2.0)
PKG_CHECK_MODULES([GLIB], glib-2.0 >= 0)

# Figure out the qt-directory
AC_MSG_CHECKING(QT4.1 directory)
QTDIR=xxx
AC_ARG_WITH([qt-dir],
             AC_HELP_STRING([--with-qt-dir=/path/to/Qt-4.7.4],
                            [to specify the path to the Qt-4.7.4 directory.]),
             [QTPATHS="$withval"],
             [QTPATHS="/usr"])

for x in $QTPATHS; do
    if test -d $x ; then
       QTDIR="$x"
    fi
done
if test $QTDIR = xxx ; then
   AC_MSG_ERROR(Could not locate QT 4.1)
fi
AC_MSG_RESULT($QTDIR)
host=`uname -a` # AC_CANONICAL_HOST is broken at the time of this writing.
case $host in
  MINGW32*)
    AC_MSG_NOTICE(MinGW detected.)
    QTLIBS="-L$QTDIR/lib -lopengl32 -lglu32 -lgdi32 -luser32 -lmingw32 -lqtmain -lQtOpenGL4 -lQtGui4 -lQtCore4 -lQtNetwork -mthreads -Wl,-enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc -Wl,-s -Wl,-s -Wl,-subsystem,windows"
    QTINC="-I$QTDIR/include --I$QTDIR/include/QtNetwork -I$QTDIR/include/QtCore -I$QTDIR/include/QtGui -I$QTDIR/include/QtOpenGL -DUNICODE -DQT_LARGEFILE_SUPPORT -DQT_DLL -DQT_NO_DEBUG -DQT_OPENGL_LIB -DQT_GUI_LIB -DQT_CORE_LIB -DQT_THREAD_SUPPORT -DQT_NEEDS_QMAIN -frtti -fexceptions"
    QTBIN="$QTDIR/bin"
    ;;
  *)
    AC_MSG_NOTICE("I'm assuming this is Linux")
    AC_PATH_XTRA
    QTLIBS="-Wl,-rpath,$QTDIR/lib -L$QTDIR/lib -lQtGui -lQtNetwork -lQtOpenGL -lQtCore $X_LIBS -lX11 -lXext -lXi $X_EXTRA_LIBS -lGLU -lGL -lpthread"
    QTINC="-I$QTDIR/include/qt4 -I$QTDIR/include/qt4/QtGui -I$QTDIR/include/qt4/QtNetwork -I$QTDIR/include/qt4/QtCore -I$QTDIR/include/qt4/QtOpenGL $X_CFLAGS -DQT_OPENGL_LIB -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED"
    QTBIN="$QTDIR/bin"
    ;;
esac

LIBS="$LIBS $QTLIBS"
INCLUDES="$INCLUDES $QTINC"
PATH="$PATH:$QTBIN"
CXXFLAGS="$CXXFLAGS $QTINC"

# Now we check whether we can actually build a Qt app
cat > myqt.h << EOF
#include <QObject>
class Test : public QObject
{
Q_OBJECT
public:
  Test() {}
  ~Test() {}
public slots:
  void receive() {}
signals:
  void send();
};
EOF

cat > myqt.cpp << EOF
#include "myqt.h"
#include <QApplication>
int main( int argc, char **argv )
{
  QApplication app( argc, argv );
  Test t;
  QObject::connect( &t, SIGNAL(send()), &t, SLOT(receive()) );
}
EOF

AC_MSG_CHECKING("does moc work?")
bnv_try_1="moc myqt.h -o moc_myqt.cpp"
AC_TRY_EVAL(bnv_try_1)
if test x"$ac_status" != x0; then
   AC_MSG_ERROR("moc doesn't work")
fi
AC_MSG_RESULT(yes)
AC_MSG_NOTICE([$CXX -c $CXXFLAGS -o moc_myqt.o moc_myqt.cpp])
AC_MSG_CHECKING(can I compile moc_myqt.cpp?)
bnv_try_2="$CXX -c $CXXFLAGS -o moc_myqt.o moc_myqt.cpp"
AC_TRY_EVAL(bnv_try_2)
if test x"$ac_status" != x0; then
   AC_MSG_ERROR("couldn't compile moc_myqt.cpp")
fi
AC_MSG_RESULT(yes)
AC_MSG_CHECKING("can I compile myqt.cpp?")
bnv_try_3="$CXX $QTINC -c $CXXFLAGS -o myqt.o myqt.cpp"
AC_TRY_EVAL(bnv_try_3)
if test x"$ac_status" != x0; then
   AC_MSG_ERROR("couldn't compile myqt.cpp")
fi
AC_MSG_RESULT(yes)
AC_MSG_CHECKING(can I link against QT?)
nv_try_4="$CXX $LIBS -o myqt myqt.o moc_myqt.o"
AC_TRY_EVAL(bnv_try_4)
if test x"$ac_status" != x0; then
   AC_MSG_ERROR("couldn't link")
fi
AC_MSG_RESULT(yes)

AC_MSG_CHECKING(for mkoctfile)
AC_TRY_EVAL(mkoctfile)
if test x"$ac_status" != x0; then
   AC_MSG_ERROR(mkoctfile is not in the path)
fi
AC_MSG_RESULT(yes)
rm -f moc_myqt.cpp myqt.h myqt.cpp myqt.o myqt moc_myqt.o

AC_CONFIG_FILES(Makefile\
		src/Makefile\
		src/misc/Makefile\
		src/Controller/Makefile\
		src/BackgroundSegmentor/Makefile\
		src/IPCameraCapture/Makefile\
		src/Tracker/Makefile\
		src/VideoLoader/Makefile\
		src/DBManager/Makefile\
		src/gui/Makefile)

AC_OUTPUT
