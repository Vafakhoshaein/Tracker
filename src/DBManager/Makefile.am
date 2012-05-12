lib_LTLIBRARIES = libdbmanager.la

libdbmanager_la_SOURCES = DBManager.cpp
			
noinst_HEADERS = DBManager.h

libdbmanager_la_CPPFLAGS = -I$(top_srcdir)/include \
			     $(GLIB_CFLAGS)

libdbmanager_la_LIBADD =  $(top_builddir)/src/misc/libmisc.la 

libdbmanager_la_LDFLAGS =  $(GLIB_LIBS) \
			  -lmysqlclient

