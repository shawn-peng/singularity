
include defs.mk
include functions.mk
include rules.mk

export PROJDIR= $(abspath .)/
export ALL_TARGETS

#TARGETS=client server

SOURCES:= $(shell find $(SRCDIR) -name '*.cpp')

SUBDIRS:= $(shell ls -d */)

.PHONY: all clean subdirs info

all: subdirs info $(ALL_TARGETS)#$(ALL_TAGETS)#$(addprefix $(BINDIR), $(BINARIES))

info: subdirs
	$(eval $(call print_vars,ALL_TARGETS))

#-rm build/* -rf
clean: 
	-rm $(BINDIR)* -rf
	-rm $(LIBDIR)* -rf
	-rm $(OBJDIR)* -rf

#$(eval $(foreach d,$(SUBDIRS),make -C $(d);))

.PHONY: src scanner protocols example
subdirs: src scanner protocols example
#	make -C src -f src/Makefile $*
#	make -C src/ $*

src: scanner protocols

protocols: scanner

example: protocols

src scanner protocols example:
	make -C $@ $*

#$(BINDIR)%: 

$(foreach target,$(BINARIES),$(eval $(call make_target,$(target))))

%:
	make -f $@.mk $*

VAR/TEST/A=This is a var test

test:
	$(info $(VAR/TEST/A))
	@echo

depend:
	 makedepend *.cpp

