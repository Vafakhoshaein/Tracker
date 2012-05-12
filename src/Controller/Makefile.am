lib_LTLIBRARIES = libcontroller.la

libcontroller_la_SOURCES = controller.cpp
			
noinst_HEADERS = controller.h

libcontroller_la_CPPFLAGS = -I$(top_srcdir)/include \
			    -I$(top_srcdir)/src/gui \
			     $(OPENCV_CFLAGS)\
			     $(GLIB_CFLAGS)

libcontroller_la_LIBADD =  $(top_builddir)/src/misc/libmisc.la 

libcontroller_la_LDFLAGS =  $(GLIB_LIBS) \
			  -lmysqlclient

