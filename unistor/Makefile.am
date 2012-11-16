##  Process this file with automake to create Makefile.in

includedir =
pkgconfigdir =

BUILD_DATE=$(shell date +%F)


bin_PROGRAMS = unistor
unistor_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)" -W -Wall
unistor_SOURCES = UnistorApp.cpp\
		UnistorHandler4Dispatch.cpp\
		UnistorHandler4RecvWrite.cpp\
		unistor.cpp\
		UnistorHandler4Master.cpp\
		UnistorHandler4Trans.cpp\
		UnistorTask4Trans.cpp\
		UnistorHandler4Checkpoint.cpp\
		UnistorHandler4Recv.cpp\
		UnistorHandler4Zk.cpp\
		../common/UnistorStore.cpp\
		../common/CwxBinLogMgr.cpp\
		../common/UnistorConfig.cpp\
		../common/UnistorDef.cpp\
		../common/UnistorPoco.cpp\
		../common/UnistorTss.cpp\
		../common/UnistorCache.cpp\
		../common/UnistorConnector.cpp\
		../common/UnistorStoreBase.cpp\
		../common/UnistorSubscribe.cpp\
		../common/UnistorReadCacheEx2.cpp\
		../zk/ZkAdaptor.cpp\
		../zk/ZkLocker.cpp
unistor_LDADD=
unistor_LDFLAGS =-L@zk_dir@/lib -L@cwinux_dir@/lib -L@openssl_dir@/lib
LIBS=-lcwxapp -lcwxnet -lcwxcmn -lzookeeper_mt -lssl -lpthread -ldl -lrt -lz
INCLUDES=-I@cwinux_dir@/include/app -I@cwinux_dir@/include/cmn -I@cwinux_dir@/include/net -I@zk_dir@/include -I@openssl_dir@/include  -I. -I../common  -I../zk

## Clean up template repositories, etc.
clean-local:
	-rm -f *~ *.bak *.rpo *.sym lib*.*_pure_* core core.*
