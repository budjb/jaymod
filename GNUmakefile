ifeq ($(PROJECT),)
   PROJECT = .
endif

ifeq ($(PLATFORM),)
    ifeq ($(VARIANT),)
        BUILD/ = $(PROJECT/)build/
    else
        BUILD/ = $(PROJECT/)build-$(VARIANT)/
    endif
else
    ifeq ($(VARIANT),)
        BUILD/ = $(PROJECT/)build.$(PLATFORM)/
    else
        BUILD/ = $(PROJECT/)build.$(PLATFORM)-$(VARIANT)/
    endif
endif

###############################################################################

MODULES += src/lua
MODULES += src/base
MODULES += src/cgame
MODULES += src/game
MODULES += src/ui
MODULES += pak/pak
MODULES += pkg/pkg
MODULES += doc/book

###############################################################################

all::

###############################################################################

-include $(BUILD/)make/project.mk

ifdef PROJECT.name
include $(PROJECT)/make/main.mk
endif

###############################################################################

$(BUILD/)make/project.mk: $(PROJECT/)project/info.py $(PROJECT/)project/info.db
	$(call print.HEADER,GENERATING,$@)
	@mkdir -p $(dir $@)
	@$< -mk $(PROJECT/)project/info.db > $@
