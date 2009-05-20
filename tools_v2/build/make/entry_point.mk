
$(if $(filter $(MAKE_VERSION),3.81),,\
  $(error These makefile templates requires GNU make version 3.81.))

default: 

.PHONY:: FORCE

FORCE:

ifeq ($(strip $(SHOW_CMDS)),)
quiet := @
else
quiet := 
endif

include $(MK)/basic_defs.mk

#########################################################

# $(call normalize_path,path)
define normalize_path
	$(subst \,/,$1)
endef


SRC_ROOT ?= $(PROJECT_DIR)

SRC_ROOT := $(strip $(call normalize_path,$(SRC_ROOT)))

# $(info Entry point curdir = $(CURDIR))

define install-hdr
  $(quiet) $(INSTALL) $(INSTALL_OPTS) $(1) $(2)
endef




define OBJ_TEMPLATE
$(2)/%.$(OBJ_EXT): $(1)/%.cpp
	$$(call make-cpp-obj-and-depend,$$<,$$@,$$(subst .$(OBJ_EXT),.d,$$@),$$(XPFLAGS),$$(XXFLAGS))

$(2)/%.$(OBJ_EXT): $(1)/%.s
	$$(call make-asm-obj,$$<,$$@,$$(subst .$(OBJ_EXT),.d,$$@),$$(XPFLAGS),$$(XXFLAGS))

$(2)/%.$(OBJ_EXT): $(1)/%.c
	$$(call make-c-obj-and-depend,$$<,$$@,$$(subst .$(OBJ_EXT),.d,$$@),$$(XPFLAGS),$$(XXFLAGS))
endef


HOST_ARCH := $(shell uname)
ifeq ($(HOST_ARCH),Linux)
    HOST_ARCH := linux
endif
ifeq ($(HOST_ARCH),WindowsNT)
    HOST_ARCH := win32
endif


ifndef ARCHITECTURE
 ifdef ARCH
   ARCHITECTURE := $(ARCH)
 else
   ARCHITECTURE := $(HOST_ARCH)
 endif
endif

BUILD_ARCH := $(ARCHITECTURE)

include $(MK)/$(HOST_ARCH).mk

include $(MK)/$(TOOLSET).mk

# optionally include platform settings
-include $(MK)/platforms/$(BUILD_ARCH).mk


ifeq ($(strip $(BUILD_TARGET)),)
  OUTPUT_DIR_COMPONENT := $(BUILD_ARCH)
else
  OUTPUT_DIR_COMPONENT := $(BUILD_ARCH)/$(BUILD_TARGET)
endif

BUILD_ROOT := $(subst \,/,$(BUILD_ROOT))
SRC_ROOT   := $(strip $(call clean-path,$(SRC_ROOT)))
BUILD_ROOT := $(strip $(call clean-path,$(BUILD_ROOT)))


$(if $(findstring $(SRC_ROOT),$(CURDIR)),,$(error The source root, $(SRC_ROOT), is not a prefix of the current dir $(CURDIR)))

DESTDIR := $(BUILD_ROOT)/installed_lib/$(OUTPUT_DIR_COMPONENT)
INCDESTDIR := $(subst \,/,$(BUILD_ROOT)/installed_include)

ifneq ($(strip $(MK_INFO)),)
$(info HOST_ARCH = $(HOST_ARCH))
$(info BUILD_ARCH = $(BUILD_ARCH))
ifneq (strip($(BUILD_TARGET)),)
$(info BUILD_TARGET = $(BUILD_TARGET))
endif
$(info TOOLSET = $(TOOLSET))
$(info BUILD_ROOT = $(BUILD_ROOT))
$(info INCDESTDIR = $(INCDESTDIR))
endif

# $(call delete_file,file_name)
define delete_file
	$(shell $(RM) $1)
endef

# $(call create_objdir,$(@D))
define create_objdir
	$(shell $(MKDIR) $1)
endef

$(INCDESTDIR)/ALL_HDRS_INSTALLED:
	@echo [make] Making sure all headers are installed...
	$(call create_objdir,$(@D))
	$(quiet) touch $@

$(DESTDIR)/ALL_LIBS_INSTALLED:
	@echo [make] Making sure all libs are installed...
	$(call create_objdir,$(@D))
	$(quiet) touch $@


ifneq ($(strip $(BUILD_ROOT)),)
clean::
	$(quiet) $(RMDIR) $(BUILD_ROOT)
endif
CMD_COMPLETION_TARGETS += clean

define generate_dep
	@echo "\$$(warning reading DEP)" > DEP
endef

IGNORE_HDRS_INSTALL_TARGETS := clean completion_targets android_make cml2-compile menu-config silent-config old-config

#$(call include_hdr_installed,target) 
define include_hdr_installed 
#Install headers only if the target is not a part of IGNORE_HDRS_INSTALL_TARGETS 
        include $(INCDESTDIR)/ALL_HDRS_INSTALLED 
        include $(DESTDIR)/ALL_LIBS_INSTALLED 
endef 
  
ifeq ($(MAKECMDGOALS),) 
   $(eval $(call include_hdr_installed)) #Install headers in case of default target 
else 
   ifeq (clean,$(filter clean,$(MAKECMDGOALS))) 
      ifneq (,$(filter-out clean,$(MAKECMDGOALS))) #Report an error if clean is used with another target. 
         $(error Multiple targets with clean not supported. Please use the targets separately.)) 
      endif 
   else #Check if any target is not in IGNORE_HDRS_INSTALL_TARGETS 
       ifneq ($(strip $(filter-out $(IGNORE_HDRS_INSTALL_TARGETS), $(MAKECMDGOALS))),) 
           $(eval $(call include_hdr_installed)) 
       endif 
   endif 
endif 


#  If BYPASS_COMBINED_COMPILE_AND_DEPEND is defined, the following
# definitions will be used
#########################################################

ifndef combined-cxx-compile-depend
  $(info default definition of combined-cxx-compile-depend)
  define combined-cxx-compile-depend
    $(call make-depend,$1,$2,$3,$4)
    $(quiet) $(CXX) $4 $5 $(CPPFLAGS) $(INCDIRS) $(CXXFLAGS) $(CO)$2 $1
  endef
endif


define make-cpp-obj-and-depend
	@echo [make] Building $2
	@echo [make] Using $1
	$(call create_objdir,$(@D))
	$(call combined-cxx-compile-depend,$1,$2,$3,$4,$5)
	@echo [make] Done
endef

define make-asm-obj
	@echo [make] Building $2
	@echo [make] Using $1
	$(call create_objdir,$(@D))
	$(call assembly-compile,$1,$2,$3,$4,$5)
endef

#########################################################

ifndef combined-cc-compile-depend
  $(info default definition of combined-cc-compile-depend)
  define combined-cc-compile-depend
    $(call make-depend,$1,$2,$3,$4)
    $(quiet) $(CC) $4 $5 $(CPPFLAGS) $(INCDIRS) $(CXXFLAGS) $(CO)$2 $1
  endef
endif


define make-c-obj-and-depend
	@echo [make] Building $2
	@echo [make] Using $1
	$(call create_objdir,$(@D))
	$(call combined-cc-compile-depend,$1,$2,$3,$4,$5)
	@echo [make] Done
endef

#########################################################
# 
# gnu-win32 make seems to hold the file handle on the directory open after a wildcard operation
# and that interfered with removing the directory.
# Hence we would avoid any wildcard operation in doing make clean.
#
define set-src-and-obj-names
SRC_FILENAMES := $(notdir $1)
REMOTE_SRCS := $$(filter-out $$(SRC_FILENAMES), $1)

REMOTE_DIRS := $$(sort $$(abspath $$(addprefix $2/,$$(dir $$(REMOTE_SRCS)))))

CSRCS := $$(filter %.c,$$(SRC_FILENAMES))
CCSRCS := $$(filter %.cc,$$(SRC_FILENAMES))
CPPSRCS := $$(filter %.cpp,$$(SRC_FILENAMES))
CXXSRCS := $$(filter %.C,$$(SRC_FILENAMES))
ASMSRCS := $$(filter %.s,$$(SRC_FILENAMES))
COBJS  := $$(CSRCS:%.c=$$(OBJDIR)/%.$$(OBJ_EXT))
CPPOBJS := $$(CPPSRCS:%.cpp=$$(OBJDIR)/%.$$(OBJ_EXT))
CXXOBJS := $$(CXXSRCS:%.C=$$(OBJDIR)/%.$$(OBJ_EXT))
CCOBJS := $$(CCSRCS:%.cc=$$(OBJDIR)/%.$$(OBJ_EXT))
ASMOBJS := $$(ASMSRCS:%.s=$$(OBJDIR)/%.$$(OBJ_EXT))
COMPILED_OBJS := $$(COBJS) $$(CXXOBJS) $$(CCOBJS) $$(CPPOBJS) $$(ASMOBJS)

DEPS := $$(COMPILED_OBJS:%.$$(OBJ_EXT)=%.d)
ifneq "$(MAKECMDGOALS)" "clean"
  FOUND_DEPS := $$(wildcard $$(DEPS))
endif
MISSING_DEPS := $$(filter-out $$(FOUND_DEPS), $$(DEPS))
FORCED_OBJS := $$(MISSING_DEPS:%.d=%.$$(OBJ_EXT))

endef





#########################################################

LIB_DIRS += $(LIBCOMPFLAG)$(BUILD_ROOT)/installed_lib/$(OUTPUT_DIR_COMPONENT)



-include $(MK)/dot.mk

DOCDESTDIR := $(BUILD_ROOT)/doc

docs: $(DOCDESTDIR)/ALL_DOCS_INSTALLED

$(DOCDESTDIR)/ALL_DOCS_INSTALLED: 
	@echo [make] Making sure all docs are installed...
	$(call create_objdir,$(@D))
	$(quiet) touch $@

.PHONY:: docs


completion_targets:
	@echo $(call purge_duplicates,$(CMD_COMPLETION_TARGETS))

.PHONY:: completion_targets

