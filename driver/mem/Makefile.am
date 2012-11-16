## This is the shared library to be built
lib_LTLIBRARIES = libuni_mem.la

## Define the source file for the module
libuni_mem_la_SOURCES = UnistorStoreMem.cpp\
			UnistorStoreMemCache.cpp\
		 	../../common/UnistorConfig.cpp\
			../../common/UnistorStoreBase.cpp\
                        ../../common/CwxBinLogMgr.cpp\
                        ../../common/UnistorPoco.cpp\
			../../common/UnistorTss.cpp\
                        ../../common/UnistorCache.cpp\
			../../common/UnistorReadCacheEx2.cpp\
			../../common/UnistorSubscribe.cpp
libuni_mem_la_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)" -W -Wall
libuni_mem_la_LDFLAGS =-L@cwinux_dir@/lib 
LIBS=-lcwxcmn -ldb -lpthread -ldl -lrt -lz
INCLUDES=-I@cwinux_dir@/include/cmn  -I. -I../../common 
