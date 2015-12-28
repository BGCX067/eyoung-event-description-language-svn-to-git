RELEASE           := 0

UNAME := $(shell uname -a)
ifneq ($(findstring x86_64, $(UNAME)), x86_64)
  $(error ONLY FOR X86_64)
endif

CC                 := g++
CFLAGS             += -g -fPIC -Wall -Werror
C_INCLUDE_FLAG     += -I
C_OUTPUT_FLAG      += -o
C_COMPILE_FLAG     += -c
OBJ_COMPILE_FLAG   += -nostdlib -r
LD                 := g++
LDFLAGS            += -lpthread -ldl -L/usr/X11R6/lib
LD_LIBDIR_FLAG     += -L
LD_SHARED_FLAG     += -shared -fPIC
LD_OUTPUT_FLAG     += -o
LD_LIBLINK_PREFIX  := -l
AR                 := ar
ARFLAGS            := -rsc
LIBPREFIX          := lib
SHARED_LIBSUFFIX   := .so
STATIC_LIBSUFFIX   := .a
OPENCL_LLIBS       := OpenCL
BUILDDIR           := obj
X86_64_DIR         := x86_64
INCLUDEDIRS        += $(DEPTH)/src/include
OPENCL_INCLUDEDIRS += $(AMDAPPSDKROOT)/include
OPENCL_LIBDIRS     += $(AMDAPPSDKROOT)/lib/$(X86_64_DIR)
LIBDIRS            += $(DEPTH)/lib
CL_SUFFIX	   	   := .cl
C_SUFFIX           := .c
COBJ_SUFFIX        := .o
CPP_SUFFIX         := .cpp
CPPOBJ_SUFFIX      := .oo

MKDIR              := mkdir -p
RM	               := rm -rf
MAKE	           := make
INSTALL			   := install
COPY               := cp -r -f

ifeq ($(RELEASE), 1)
CFLAGS             += -O2 -funroll-all-loops -fexpensive-optimizations -ffast-math -finline-functions -frerun-loop-opt -static-libgcc
endif

ifeq ($(BUILD_TYPE), obj)
OBJ_OUTPUT         := _$(TARGET)$(COBJ_SUFFIX)
OUTPUT             := $(BUILDDIR)/$(OBJ_OUTPUT)
endif
ifeq ($(BUILD_TYPE), bin)
EXE_OUTPUT         := $(TARGET)
OUTPUT             := $(BUILDDIR)/$(EXE_OUTPUT) 
endif
ifeq ($(BUILD_TYPE), lib)
STATIC_LIB_OUTPUT  := $(LIBPREFIX)$(TARGET)$(STATIC_LIBSUFFIX)
OUTPUT		       := $(BUILDDIR)/$(STATIC_LIB_OUTPUT)
endif
ifeq ($(BUILD_TYPE), dynlib)
SHARED_LIB_OUTPUT  := $(LIBPREFIX)$(TARGET)$(SHARED_LIBSUFFIX)
OUTPUT		       := $(BUILDDIR)/$(SHARED_LIB_OUTPUT)
LDFLAGS            += $(LD_SHARED_FLAG)
endif

SDK_UTIL_LIBS      := $(LIBDIRS)/$(LIBPREFIX)SDKUtil$(STATIC_LIBSUFFIX)
INSTALL_BIN_DIR    := $(DEPTH)/bin
INSTALL_LIB_DIR    := $(DEPTH)/lib
DEPNDS_INC_DIR     += -I$(DEPTH)/include
DEPNDS_INC_DIR     += $(foreach f,$(INCLUDEDIRS),-I$(f))
ADDSDKLIBS         += $(foreach f,$(LLIBS),$(LD_LIBLINK_PREFIX)$(f))
ADDSDKLIBDIRS      += $(foreach f,$(LIBDIRS),$(LD_LIBDIR_FLAG)$(f))
ADDSDKINCDIRS      += $(foreach f,$(INCLUDEDIRS),$(C_INCLUDE_FLAG)$(f))
ADDSDKLIBS         += $(foreach f,$(OPENCL_LLIBS),$(LD_LIBLINK_PREFIX)$(f))
ADDSDKLIBDIRS      += $(LD_LIBDIR_FLAG)$(OPENCL_LIBDIRS)
ADDSDKINCDIRS      += $(C_INCLUDE_FLAG)$(OPENCL_INCLUDEDIRS) $(C_INCLUDE_FLAG)$(DEPTH)/include
LDFLAGS            += $(ADDSDKLIBS) $(ADDSDKLIBDIRS) 
CFLAGS             += $(ADDSDKINCDIRS) 

C_FILES            = $(wildcard *$(C_SUFFIX))
C_OBJS             = $(C_FILES:%$(C_SUFFIX)=%$(COBJ_SUFFIX))
C_OBJS            := $(C_OBJS:%=$(BUILDDIR)/%)
CPP_FILES          = $(wildcard *$(CPP_SUFFIX))
CPP_OBJS           = $(CPP_FILES:%$(CPP_SUFFIX)=%$(CPPOBJ_SUFFIX))
CPP_OBJS          := $(CPP_OBJS:%=$(BUILDDIR)/%)
SUB_OBJS          := $(SUBMODS:%=$(BUILDDIR)/_%.o)

export LD_LIBRARY_PATH:=$(AMDAPPSDKROOT)/lib/$(X86_64_DIR):$(AMDAPPSDKSAMPLESROOT)/lib/$(X86_64_DIR):$(LD_LIBRARY_PATH)

all: $(BUILDDIR) $(SUBDIRS) $(OUTPUT) install
#recrusive make
$(SUBDIRS):
	$(MAKE) -C $@

#
#create obj dir
#
$(BUILDDIR):
ifneq ($(DEPTH),..)
	$(MKDIR) $(BUILDDIR)
endif

#
#clean rule
#
clean:
ifneq ($(DEPTH),..)
	$(RM) $(BUILDDIR)
endif
ifdef SUBDIRS
	for i in $(SUBDIRS); do make -C $$i clean; done
endif
ifeq ($(BUILD_TYPE), bin)
	$(RM) $(INSTALL_BIN_DIR)/$(EXE_OUTPUT)
ifdef CLFILES
	for f in $(CLFILES); do \
		$(RM) $(INSTALL_BIN_DIR)/$$f; \
	done
endif
endif
ifeq ($(BUILD_TYPE), lib)
	$(RM) $(INSTALL_LIB_DIR)/$(STATIC_LIB_OUTPUT)
endif
ifeq ($(BUILD_TYPE), dynlib)
	$(RM) $(INSTALL_LIB_DIR)/$(SHARED_LIB_OUTPUT)
endif

#
#build objective
#
ifeq ($(BUILD_TYPE), bin)
$(OUTPUT): $(C_OBJS) $(CPP_OBJS) $(SDK_UTIL_LIBS)
	$(LD) $(LD_OUTPUT_FLAG) $@ $(C_OBJS) $(CPP_OBJS) $(SUB_OBJS) $(LDFLAGS) 
endif
ifeq ($(BUILD_TYPE), obj)
$(OUTPUT): $(C_OBJS) $(CPP_OBJS) $(SDK_UTIL_LIBS)
	$(CC) $(CFLAGS) $(OBJ_COMPILE_FLAG) $(C_OUTPUT_FLAG) $@ $(C_OBJS) $(CPP_OBJS) $(SUB_OBJS)
endif
ifeq ($(TARGET), SDKUtil)
SDK_UTIL_LIBS := 
endif
ifeq ($(BUILD_TYPE), lib)
$(OUTPUT): $(C_OBJS) $(CPP_OBJS) $(SDK_UTIL_LIBS)
	$(AR) $(ARFLAGS) $@ $(C_OBJS) $(CPP_OBJS) $(SUB_OBJS)
endif
ifeq ($(BUILD_TYPE), dynlib)
$(OUTPUT): $(C_OBJS) $(CPP_OBJS) $(SDK_UTIL_LIBS)
	$(LD) $(LDFLAGS) $(ADDSDKLIBS) $(LD_OUTPUT_FLAG) $@ $(C_OBJS) $(CPP_OBJS) $(SUB_OBJS)
endif

#
#compile source files
#
$(BUILDDIR)/%$(CPPOBJ_SUFFIX): %$(CPP_SUFFIX)
	$(CC) $(CFLAGS) $(C_OUTPUT_FLAG) $@ $(C_COMPILE_FLAG) $<
$(BUILDDIR)/%$(COBJ_SUFFIX): %$(C_SUFFIX)
	$(CC) $(CFLAGS) $(C_OUTPUT_FLAG) $@ $(C_COMPILE_FLAG) $<

#
#install rule
#
install: $(OUTPUT)
ifeq ($(BUILD_TYPE), bin)
	$(INSTALL) -D $(OUTPUT) $(INSTALL_BIN_DIR)/$(EXE_OUTPUT)
endif 
ifdef CLFILES
	for f in $(CLFILES); do \
		$(COPY) $$f $(BUILDDIR); \
		$(INSTALL) -D $$f $(INSTALL_BIN_DIR)/$$f; \
	done
endif
ifeq ($(BUILD_TYPE), lib)
	$(INSTALL) -D $(OUTPUT) $(INSTALL_LIB_DIR)/$(STATIC_LIB_OUTPUT)
endif 
ifeq ($(BUILD_TYPE), dynlib)
	$(INSTALL) -D $(OUTPUT) $(INSTALL_LIB_DIR)/$(SHARED_LIB_OUTPUT)
endif
ifeq ($(BUILD_TYPE), obj)
	$(COPY) $(OUTPUT) ../$(BUILDDIR)
endif

.PHONY:  all $(SUBDIRS) clean 	 
