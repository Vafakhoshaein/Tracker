bin_PROGRAMS = tracker

tracker_SOURCES = main.cpp\
 	          mainwindow.cpp\
		  moc_mainwindow.cpp\
		  toolbox.cpp\
		  moc_toolbox.cpp\
		  settingview.cpp\
		  moc_settingview.cpp\
		  sourceview.cpp\
		  moc_sourceview.cpp

tracker_CPPFLAGS = -I$(top_srcdir)/include \
		   -I$(top_srcdir)/src/Controller \
		   -I$(top_srcdir)/src/Tracker \
		   -I$(top_srcdir)/src/IPCameraCapture \
		   -I$(top_srcdir)/src/VideoLoader \
		   -I$(top_srcdir)/src/DBManager \
		     $(OPENCV_CFLAGS) 

tracker_LDADD =  $(top_builddir)/src/misc/libmisc.la \
		 $(top_builddir)/src/Controller/libcontroller.la \
		 $(top_builddir)/src/IPCameraCapture/libipcamcapture.la \
		 $(top_builddir)/src/Tracker/libtracker.la \
		 $(top_builddir)/src/VideoLoader/libvideoloader.la \
		 $(top_builddir)/src/DBManager/libdbmanager.la\
		 $(top_builddir)/src/BackgroundSegmentor/libbackseg.la

tracker_LDFLAGS = $(OPENCV_LIBS) 

#EXTRA_DIST = $(nodist_tracker_SOURCES:moc_%.cpp=%.h) 
#EXTRA_DIST += $(FORMS:%.ui=ui_%.h)


moc_mainwindow.cpp: mainwindow.h
	moc $< -o $@

moc_toolbox.cpp: toolbox.h
	moc $< -o $@

moc_settingview.cpp: settingview.h
	moc $< -o $@

moc_sourceview.cpp: sourceview.h
	moc $< -o $@

