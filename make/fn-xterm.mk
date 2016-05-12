ifeq ($(TERM),xterm)

print.GENERIC = @echo "$(subst $1,[33m$1[0m,$(subst $2,[32m$2[0m,$(subst $3,[36m$3[0m,$(subst $4,[1;30m$4[0m,$5))))"; $5

print.COMMAND.normal = @echo "$(subst $1,[33m$1[0m,$(subst $2,[32m$2[0m,$3))"; $3
print.COMMAND.bold   = @echo "$(subst $1,[33m$1[0m,$(subst $2,[1;32m$2[0m,$3))"; $3
print.COMMAND.dim    = @echo "$(subst $1,[33m$1[0m,$(subst $2,[1;30m$2[0m,$3))"; $3

print.COMMAND.black   = @echo "$(subst $1,[33m$1[0m,$(subst $2,[30m$2[0m,$3))"; $3
print.COMMAND.red     = @echo "$(subst $1,[33m$1[0m,$(subst $2,[31m$2[0m,$3))"; $3
print.COMMAND.green   = @echo "$(subst $1,[33m$1[0m,$(subst $2,[32m$2[0m,$3))"; $3
print.COMMAND.brown   = @echo "$(subst $1,[33m$1[0m,$(subst $2,[33m$2[0m,$3))"; $3
print.COMMAND.blue    = @echo "$(subst $1,[33m$1[0m,$(subst $2,[34m$2[0m,$3))"; $3
print.COMMAND.magenta = @echo "$(subst $1,[33m$1[0m,$(subst $2,[35m$2[0m,$3))"; $3
print.COMMAND.cyan    = @echo "$(subst $1,[33m$1[0m,$(subst $2,[36m$2[0m,$3))"; $3
print.COMMAND.lgray   = @echo "$(subst $1,[33m$1[0m,$(subst $2,[37m$2[0m,$3))"; $3

print.HEADER = @echo "[1;33m$(1):[0m [32m$(2)[0m"

value.ERROR  = [31mERROR: [0m[1;33m$(1)[0m
value.ERROR2 = [31mERROR: [0m$(1): [1;33m$(2)[0m

print.LINK = @echo "$(subst $1,[33m$1[0m,$(subst $2,[36m$2[0m,$(subst $3,[1;30m$3[0m,$4)))"; $4
print.STATIC = @echo "$(subst $1,[33m$1[0m,$(subst $2,[36m$2[0m,$(subst $3,[1;30m$3[0m,$4)))"; $4
#print.STATIC = @echo "$(subst $1,[33m$1[0m,$(subst $2,[32m$2[0m,$3))"; $3

else

print.GENERIC = $5

print.COMMAND.normal = $3
print.COMMAND.bold   = $3
print.COMMAND.dim    = $3

print.COMMAND.black   = $3
print.COMMAND.red     = $3
print.COMMAND.green   = $3
print.COMMAND.brown   = $3
print.COMMAND.blue    = $3
print.COMMAND.magenta = $3
print.COMMAND.cyan    = $3
print.COMMAND.lgray   = $3

print.HEADER = @echo "=== $(1): $(2)"

value.ERROR  = ERROR: $(1)
value.ERROR2 = ERROR: $(1): $(2)

print.LINK = $4
print.STATIC = $4

endif
