################################################
# This template is for prebuilt libraries that needs to be exported.

PREBUILT_LIBRARIES := $(notdir $(strip $(PREBUILT_LIBS)))
LOCAL_PREBUILT_LIBS := $(PREBUILT_LIBRARIES:%=$(BUILD_ROOT)/installed_lib/$(BUILD_ARCH)/%)
$(foreach lib, $(strip $(PREBUILT_LIBS)), $(eval $(call INST_TEMPLATE,$(notdir $(lib)),$(abspath $(dir $(lib))),$(DESTDIR))))
$(foreach lib, $(strip $(PREBUILT_LIBRARIES)),$(eval $(patsubst lib%,%,$(basename $(lib))_fullname) := $(DESTDIR)/$(lib)))
$(DESTDIR)/ALL_LIBS_INSTALLED: $(LOCAL_PREBUILT_LIBS)

#So far tested with static libraries
LIB_EXT := $(STAT_LIB_EXT)

#Install prebuilt PV libs
PREBUILT_LIBRARIES_PV := $(notdir $(strip $(PREBUILT_LIBS_PV))) 
LOCAL_PREBUILT_LIBS_PV := $(PREBUILT_LIBRARIES_PV:%=$(BUILD_ROOT)/installed_lib/$(BUILD_ARCH)/%)
$(foreach lib, $(strip $(PREBUILT_LIBS_PV)), $(eval $(call INST_TEMPLATE,$(notdir $(lib)),$(LOCAL_PATH)/$(dir $(lib)),$(DESTDIR)))) 
$(foreach lib, $(strip $(PREBUILT_LIBRARIES_PV)),$(eval $(patsubst lib%,%,$(basename $(lib))_fullname) := $(DESTDIR)/$(subst .$(LIB_EXT),$(LIB_DEBUG_EXT).$(LIB_EXT),$(lib))))
$(DESTDIR)/ALL_LIBS_INSTALLED: $(LOCAL_PREBUILT_LIBS_PV)

#Install prebuilt external libs
PREBUILT_LIBRARIES_EXTERNAL := $(notdir $(strip $(PREBUILT_LIBS_EXTERNAL)))
LOCAL_PREBUILT_LIBS_EXTERNAL := $(PREBUILT_LIBRARIES_EXTERNAL:%=$(BUILD_ROOT)/installed_lib/$(BUILD_ARCH)/%)
$(foreach lib, $(strip $(PREBUILT_LIBS_EXTERNAL)), $(eval $(call INST_TEMPLATE,$(notdir $(lib)),$(LOCAL_PATH)/$(dir $(lib)),$(DESTDIR)))) 
$(foreach lib, $(strip $(PREBUILT_LIBRARIES_EXTERNAL)),$(eval $(patsubst lib%,%,$(basename $(lib))_fullname) := $(DESTDIR)/$(lib)))
$(DESTDIR)/ALL_LIBS_INSTALLED: $(LOCAL_PREBUILT_LIBS_EXTERNAL)

################################################
