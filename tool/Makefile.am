##  Process this file with automake to create Makefile.in

includedir =
pkgconfigdir =

BUILD_DATE=$(shell date +%F)


bin_PROGRAMS =kv_add kv_set kv_update kv_inc kv_del kv_exist kv_get kv_list kv_import binlog kv_export
kv_add_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)"  -W -Wall -Wno-deprecated
kv_add_SOURCES = kv_add.cpp ../common/UnistorPoco.cpp XmlParse.cpp
kv_add_LDADD= 
kv_add_LDFLAGS = -L@cwinux_dir@/lib 

kv_set_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)"  -W -Wall -Wno-deprecated
kv_set_SOURCES = kv_set.cpp ../common/UnistorPoco.cpp XmlParse.cpp
kv_set_LDADD=
kv_set_LDFLAGS = -L@cwinux_dir@/lib 

kv_update_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)"  -W -Wall -Wno-deprecated
kv_update_SOURCES = kv_update.cpp ../common/UnistorPoco.cpp XmlParse.cpp
kv_update_LDADD=
kv_update_LDFLAGS = -L@cwinux_dir@/lib 

kv_inc_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)"  -W -Wall -Wno-deprecated
kv_inc_SOURCES = kv_inc.cpp ../common/UnistorPoco.cpp XmlParse.cpp
kv_inc_LDADD=
kv_inc_LDFLAGS = -L@cwinux_dir@/lib 

kv_del_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)"  -W -Wall -Wno-deprecated
kv_del_SOURCES = kv_del.cpp ../common/UnistorPoco.cpp XmlParse.cpp
kv_del_LDADD=
kv_del_LDFLAGS = -L@cwinux_dir@/lib 

kv_exist_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)"  -W -Wall -Wno-deprecated
kv_exist_SOURCES = kv_exist.cpp ../common/UnistorPoco.cpp XmlParse.cpp
kv_exist_LDADD=
kv_exist_LDFLAGS = -L@cwinux_dir@/lib

kv_get_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)"  -W -Wall -Wno-deprecated
kv_get_SOURCES = kv_get.cpp ../common/UnistorPoco.cpp XmlParse.cpp
kv_get_LDADD=
kv_get_LDFLAGS = -L@cwinux_dir@/lib 

kv_list_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)"  -W -Wall -Wno-deprecated
kv_list_SOURCES = kv_list.cpp ../common/UnistorPoco.cpp XmlParse.cpp
kv_list_LDADD=
kv_list_LDFLAGS = -L@cwinux_dir@/lib

kv_import_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)"  -W -Wall -Wno-deprecated
kv_import_SOURCES = kv_import.cpp ../common/UnistorPoco.cpp XmlParse.cpp
kv_import_LDADD=
kv_import_LDFLAGS = -L@cwinux_dir@/lib

kv_export_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)"  -W -Wall -Wno-deprecated
kv_export_SOURCES = kv_export.cpp ../common/UnistorPoco.cpp ../common/UnistorStore.cpp ../common/UnistorTss.cpp\
		    ../common/UnistorStoreBase.cpp ../common/UnistorReadCacheEx2.cpp ../common/UnistorCache.cpp \
		    ../common/UnistorConfig.cpp ../common/CwxBinLogMgr.cpp ../common/UnistorDef.cpp\
		    ../common/UnistorSubscribe.cpp XmlParse.cpp
kv_export_LDADD=
kv_export_LDFLAGS = -L@cwinux_dir@/lib

binlog_CPPFLAGS = -D_BUILD_DATE="$(BUILD_DATE)"  -W -Wall -Wno-deprecated
binlog_SOURCES = binlog.cpp CwxBinlogOp.cpp ../common/CwxBinLogMgr.cpp
binlog_LDADD=
binlog_LDFLAGS = -L@cwinux_dir@/lib

LIBS=-lcwxnet -lcwxcmn -lexpat -lz -lpthread -ldl
INCLUDES=-I@cwinux_dir@/include/net -I@cwinux_dir@/include/cmn   -I@cwinux_dir@/include/parse -I../common


## Clean up template repositories, etc.
clean-local:
	-rm -f *~ *.bak *.rpo *.sym lib*.*_pure_* core core.*
