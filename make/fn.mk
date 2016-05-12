fnPrefix = $(foreach n,$(strip $(2)),$(1)$($(subst &,,$(filter &%,$(n))))$(filter-out &%,$(n)))
fnSuffix = $(foreach n,$(strip $(2)),$($(subst &,,$(filter &%,$(n))))$(filter-out &%,$(n))$(1))
fnAbsPath = $(patsubst %,$(CURDIR)/%,$(filter-out /%,$(1)))$(filter /%,$(1))
fnDelimit = $(patsubst %:,%,$(subst : $(),$(1),$(call fnSuffix,$(1),$(2))))

fn.cp    = $(call print.COMMAND.normal,cp,$(2),cp -p $(1) $(2))
fn.mkdir = $(call print.COMMAND.normal,mkdir,$(1),mkdir -p $(1))
fn.rmfr  = $(call print.COMMAND.normal,rm,$(1),rm -fr $(1))
