VERSION_X=1
VERSION_Y=0
VERSION_Z=0
PROJECT=PlatformBase

#prebuild depends libs
#index 0 is project name
#index 1 is version
#index 2 is copy include flag
#index 3 is copy lib flag
PREBUILD_LIB_PATH=../../../../../../../../Versions/Baselib
PREBUILD_LIBS="libpcap 1.9.1 true true"
POSTBUILD_PATH=../../../../../../../../Versions/Baselib

#manually specify
INCLUDE=-I../../../../include \
-I../../../../include_internal \
-I../../../../deps/include \
-I../../../../deps/include/libpcap
LIBS=-L../../../../deps/lib -Wl,-rpath-link=../../../../deps/lib -lpcap
WARNING_SWITCH=-Wall -Wno-unused-function
DEFINES=

VERSION=$(VERSION_X).$(VERSION_Y).$(VERSION_Z)
REAL_NAME=lib$(PROJECT).so.$(VERSION)
SONAME=lib$(PROJECT).so.$(VERSION_X)
TARGET=../../../../bin/$(REAL_NAME)
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

%.o:%.cpp
	$(CXX) $(DEFINES) $(WARNING_SWITCH) -g -fPIC -o $@ -c $< $(INCLUDE) -std=c++11

%.o:%.c
	$(CC) $(DEFINES) $(WARNING_SWITCH) -g -fPIC $(INCLUDE) -o $@ -c $<

.PHONY:clean

clean:
	-rm -f $(COBJ) $(CPPOBJ) $(TARGET)
