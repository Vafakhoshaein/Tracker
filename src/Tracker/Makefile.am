lib_LTLIBRARIES = libtracker.la

libtracker_la_SOURCES = filteredpoint.cpp \
			trace.cpp \
			eventtracker.cpp
			
libtracker_la_CPPFLAGS = -I$(top_srcdir)/include \
			  $(GLIB_CFLAGS) \
			  $(OPENCV_CFLAGS) \
		         -std=c++0x

libtracker_la_LIBADD =  $(top_builddir)/src/misc/libmisc.la \
			$(top_builddir)/src/BackgroundSegmentor/libbackseg.la

libtracker_la_LDFLAGS = $(OPENCV_LIBS) \
			$(GLIB_LIBS)
			

