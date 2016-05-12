PROJECT/ ?= $(PROJECT)/

PROJECT.rar = $(BUILD/)$(PROJECT.packageBase).rar

###############################################################################

include $(PROJECT/)make/docbook.mk
include $(PROJECT/)make/fn-xterm.mk
include $(PROJECT/)make/fn.mk

include $(PROJECT/)make/platform/$(PROJECT.platformNamef)

ifneq ($(VARIANT),)
include $(PROJECT/)make/variant/$(PROJECT.platformNamef)-$(VARIANT)
endif

include $(MODULES:%=$(PROJECT/)%.defs)

###############################################################################

PROJECT.platspecific = unknown
ifeq ($(PROJECT.platformName),Linux)
    PROJECT.platspecific = linux
endif
ifeq ($(PROJECT.platformName),OSX)
    PROJECT.platspecific = osx
endif
ifeq ($(PROJECT.platformName),Windows)
    PROJECT.platspecific = win32
endif

###############################################################################

BUILD.output += $(BUILD/)make/project.mk
BUILD.output += $(BUILD/)m4/project.m4
BUILD.output += $(PROJECT.rar)

BUILD.dirs = $(sort $(dir $(BUILD.output)))

###############################################################################

.DELETE_ON_ERROR:
.SUFFIXES:

###############################################################################

.PHONY: clean pak.clean pkg.clean
.PHONY: all default debug release
.PHONY: specials world
.PHONY: pkg doc

###############################################################################

all:: $(BUILD.dirs) $(BUILD/)m4/project.m4

clean::
	rm -fr $(BUILD/)

cleanall::
	rm -rf build*

$(BUILD.dirs):
	$(call fn.mkdir,$@)

###############################################################################

default::
	$(MAKE) pkg VARIANT=

debug::
	$(MAKE) pkg VARIANT=debug

release::
	$(MAKE) pkg VARIANT=release

###############################################################################

doc:: $(BUILD.dirs)

###############################################################################

$(BUILD/)m4/project.m4: $(PROJECT/)project/info.py $(PROJECT/)project/info.db
	$(call print.HEADER,GENERATING,$@)
	@mkdir -p $(dir $@)
	@$< -m4 $(PROJECT/)project/info.db > $@

###############################################################################

include $(MODULES:%=$(PROJECT/)%.rules)
