lib_LTLIBRARIES = libvideoloader.la

libvideoloader_la_SOURCES = VideoLoader.cpp
			
libvideoloader_la_CPPFLAGS = -I$(top_srcdir)/include \
			      $(OPENCV_CFLAGS) 

libvideoloader_la_LIBADD =  $(top_builddir)/src/misc/libmisc.la 

libvideoloader_la_LDFLAGS = $(OPENCV_LIBS) 

