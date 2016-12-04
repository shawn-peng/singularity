#CFLAGS=
#CXXFLAGS= -I
LDFLAGS= -L/usr/local/lib -lm -lwayland-client -lwayland-egl -lwayland-cursor -lEGL -lGL -lwld -lswc

CXXFLAGS=-I/usr/local/include/ -fpermissive
BINARIES=void

BUILDDIR=./build/
BINDIR=$(BUILDDIR)bin/
OBJDIR=$(BUILDDIR)obj/
SRCDIR=./src/

SOURCES := $(shell find $(SRCDIR) -name '*.cpp')

all: $(addprefix $(BINDIR), $(BINARIES))

clean: 
	 -rm build/* -rf

#$(BINDIR)%: 
define make_target

$1_SOURCES:=$(shell find $(SRCDIR)$1/ -name '*.cpp')
$1_OBJECTS:=$$($1_SOURCES:$(SRCDIR)%.cpp=$(OBJDIR)%.o)
$(BINDIR)$1: $$($1_OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) -o $(BINDIR)$1 $$($1_OBJECTS) $(LDFLAGS)

endef

$(foreach target,$(BINARIES),$(eval $(call make_target,$(target))))

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $< $(CXXFLAGS) -o $@


depend:
	 makedepend *.cpp

