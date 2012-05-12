lib_LTLIBRARIES = libipcamcapture.la

libipcamcapture_la_SOURCES = ipcameracapture.cpp
dist_libipcamcapture_la_SOURCES = moc_ipcameracapture.cpp 
	
libipcamcapture_la_CPPFLAGS = -I$(top_srcdir)/include \
			        $(OPENCV_CFLAGS)

libipcamcapture_la_LIBADD =  $(top_builddir)/src/misc/libmisc.la

libipcamcapture_la_LDFLAGS = $(OPENCV_LIBS) 

EXTRA_DIST = $(nodist_tracker_SOURCES:moc_%.cpp=%.h)

moc_%.cpp: %.h
	moc $< -o $@

