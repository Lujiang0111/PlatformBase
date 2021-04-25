VERSION_X=1
VERSION_Y=1
VERSION_Z=2
PROJECT=PlatformBase

ifneq (,$(wildcard ../../../../../../../../compile.dep))
	include ../../../../../../../../compile.dep
else
	COMPILE_RELEASE_ITEM=-fstack-protector-all -fPIC -g
endif

#prebuild depends libs
#index 0 is project name
#index 1 is version
#index 2 is copy include flag
#index 3 is copy lib flag
#index 4 is whether exactly match version or not
PREBUILD_LIB_PATH=../../../../../../../../Versions/Baselib
PREBUILD_LIBS_BASE=""
POSTBUILD_PATH=../../../../../../../../Versions/Baselib

ENABLE_PCAP=

ifdef ENABLE_PCAP
PREBUILD_LIBS_PCAP="libpcap 1.9.1 true true false"
endif
PREBUILD_LIBS=$(PREBUILD_LIBS_BASE)$(PREBUILD_LIBS_PCAP)

#manually specify
INCLUDE=-I../../../../include \
-I../../../../include_internal \
-I../../../../deps/include \
-I../../../../deps/include/libpcap

WARNING_SWITCH=-Wall -Wno-unused-function
LIBS=-L../../../../deps/lib -Wl,-rpath-link=../../../../deps/lib
DEFINES=

ifdef ENABLE_PCAP
DEFINES+=-DENABLE_PCAP=1
LIBS+=-lpcap
endif

VERSION=$(VERSION_X).$(VERSION_Y).$(VERSION_Z)
REAL_NAME=lib$(PROJECT).so.$(VERSION)
SONAME=lib$(PROJECT).so.$(VERSION_X)
TARGET=../../../../bin/$(REAL_NAME)
TARGET_RELEASE=../../../../bin/release/$(REAL_NAME)
CC:=gcc
CXX:=g++
SRC_DIRS=../../../../src ../../../../src/*
CSRC=$(foreach TMP_SRC_DIRS, $(SRC_DIRS), $(wildcard $(TMP_SRC_DIRS)/*.c))
COBJ:=$(CSRC:.c=.o)
CPPSRC=$(foreach TMP_SRC_DIRS, $(SRC_DIRS), $(wildcard $(TMP_SRC_DIRS)/*.cpp))
CPPOBJ:=$(CPPSRC:.cpp=.o)

all:prebuild $(TARGET)
prebuild:
	chmod +x *.sh
	./prebuild.sh $(PREBUILD_LIB_PATH) $(PREBUILD_LIBS)

$(TARGET):$(COBJ) $(CPPOBJ)
	$(CXX) -shared -Wl,-soname,$(SONAME) -Wall -g -o $(TARGET) $(COBJ) $(CPPOBJ) $(LIBS)
	./postbuild.sh $(POSTBUILD_PATH) $(PROJECT) $(VERSION) $(TARGET)

release:prebuild_r $(TARGET_RELEASE)
prebuild_r:
	chmod +x *.sh
	./prebuild.sh $(PREBUILD_LIB_PATH) $(PREBUILD_LIBS) release

$(TARGET_RELEASE):$(COBJ) $(CPPOBJ)
	$(CXX) -shared -Wl,-soname,$(SONAME)$(LINK_RELEASE_WL_ADD) -Wall -g -o $(TARGET_RELEASE) $(COBJ) $(CPPOBJ) $(LIBS);\
	strip -s $(TARGET_RELEASE)
	./postbuild.sh $(POSTBUILD_PATH) $(PROJECT) $(VERSION) $(TARGET_RELEASE) release

%.o:%.cpp
	$(CXX) $(DEFINES) $(WARNING_SWITCH) $(COMPILE_RELEASE_ITEM) -o $@ -c $< $(INCLUDE) -std=c++11

%.o:%.c
	$(CC) $(DEFINES) $(WARNING_SWITCH) $(COMPILE_RELEASE_ITEM) $(INCLUDE) -o $@ -c $<

.PHONY:clean

clean:
	-rm -f $(COBJ) $(CPPOBJ) $(TARGET) $(TARGET_RELEASE)
