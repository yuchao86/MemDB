## This is the shared library to be built
lib_LTLIBRARIES = libuni_bdbc.la

## Define the source file for the module
libuni_bdbc_la_SOURCES = UnistorStoreBdbc.cpp\
		 	../../common/UnistorConfig.cpp\
			../../common/UnistorStoreBase.cpp\
                        ../../common/CwxBinLogMgr.cpp\
                        ../../common/UnistorPoco.cpp\
			../../common/UnistorTss.cpp\
                        ../../common/UnistorCache.cpp\
			../../common/UnistorReadCacheEx2.cpp\
			../../common/UnistorSubscribe.cpp
libuni_bdbc_la_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)" -W -Wall
libuni_bdbc_la_LDFLAGS =-L@xml_expat_dir@/lib -L@cwinux_dir@/lib -L@bdb_dir@/lib
LIBS=-lcwxcmn -ldb -lpthread -ldl -lrt -lz
INCLUDES=-I@cwinux_dir@/include/cmn  -I@bdb_dir@/include -I. -I../../common 
