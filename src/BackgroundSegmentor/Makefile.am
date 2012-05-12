lib_LTLIBRARIES = libbackseg.la

libbackseg_la_SOURCES = BackgroundSegmentor.cpp 
	
libbackseg_la_CPPFLAGS = -I$(top_srcdir)/include \
			  $(OPENCV_CFLAGS) \
		         -std=c++0x

libbackseg_la_LIBADD =  $(top_builddir)/src/misc/libmisc.la

libbackseg_la_LDFLAGS = $(OPENCV_LIBS) 

