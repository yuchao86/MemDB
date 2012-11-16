##  Process this file with automake to create Makefile.in

includedir =
pkgconfigdir = 

lib_LTLIBRARIES = libunistor.la
libunistor_la_CPPFLAGS = -W -Wall

libunistor_la_SOURCES = unistor_poco.c
libunistor_la_LDFLAGS = 

unistorincludedir=$(prefix)/include/
unistorinclude_HEADERS=unistor_poco.h

	
INCLUDES=-I@libcwinux_c_dir@/include -I$(srcdir)

## Clean up template repositories, etc.
clean-local:
	-rm -f *~ *.bak *.rpo *.sym lib*.*_pure_* core core.*
