## This is the shared library to be built
lib_LTLIBRARIES = libuni_empty.la

## Define the source file for the module
libuni_empty_la_SOURCES = UnistorStoreEmpty.cpp\
		 	../../common/UnistorConfig.cpp\
			../../common/UnistorStoreBase.cpp\
                        ../../common/CwxBinLogMgr.cpp\
                        ../../common/UnistorPoco.cpp\
			../../common/UnistorTss.cpp\
                        ../../common/UnistorCache.cpp\
			../../common/UnistorReadCacheEx2.cpp\
			../../common/UnistorSubscribe.cpp
libuni_empty_la_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)" -W -Wall
libuni_empty_la_LDFLAGS =-L@cwinux_dir@/lib 
LIBS=-lcwxcmn -ldb -lpthread -ldl -lrt -lz
INCLUDES=-I@cwinux_dir@/include/cmn  -I. -I../../common 
