##  Process this file with automake to create Makefile.in

includedir =
pkgconfigdir =

BUILD_DATE=$(shell date +%F)


bin_PROGRAMS = unistor_bench
unistor_bench_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)" -W -Wall  -Wno-deprecated
unistor_bench_SOURCES = \
	../../common/UnistorPoco.cpp  ../../common/UnistorTss.cpp ../../common/UnistorDef.cpp\
	UnistorBenchApp.cpp  UnistorBenchConfig.cpp  unistor_bench.cpp  UnistorEventHandler.cpp
unistor_bench_LDADD=
unistor_bench_LDFLAGS =-L@cwinux_dir@/lib 
LIBS=-lcwxapp -lcwxnet -lcwxcmn -lpthread -ldl -lrt -lz
INCLUDES=-I@cwinux_dir@/include/app -I@cwinux_dir@/include/cmn -I@cwinux_dir@/include/net -I../../common

## Clean up template repositories, etc.
clean-local:
	-rm -f *~ *.bak *.rpo *.sym lib*.*_pure_* core core.*
