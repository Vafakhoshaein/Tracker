lib_LTLIBRARIES = libmisc.la

libmisc_la_SOURCES = connected-components/connectedComponent.cpp \
		     connected-components/FlagImage.c \
		     signals/signalprocessor.cpp \
		     signals/dispatcher.cpp \
		     fsm/finitestatemachine.c \
		     fsm/state.c \
	  	     fsm/transition.c \
		     common.cpp 
	
libmisc_la_CPPFLAGS = -I$(top_srcdir)/include \
		       $(OPENCV_CFLAGS) \
		       -std=c++0x

libmisc_la_LDFLAGS = $(OPENCV_LIBS) 
			  


#dnl libbeammisc_la_LIBTOOLFLAGS = --disable-shared

