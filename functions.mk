

# add -l prefix to libs
define expand_libflags
$(foreach libname,$1,-l $(libname))

endef


#$1_LIBS:=$(call expand_libflags,$3)
#args	$1: name $2: sources $3: dep libs
define make_executable
ALL_TARGETS=$(ALL_TARGETS) $(BINDIR)$1
$1_SOURCES:=$$(abspath $2)
$1_OBJECTS:=$$($1_SOURCES:$(PROJDIR)%.cpp=$(OBJDIR)%.o)
$1_OBJECTS:=$$($1_OBJECTS:$(PROJDIR)%.c=$(OBJDIR)%.o)
$1_LIBS:=$(call expand_libflags,$3)
$(BINDIR)$1: $$($1_OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) -o $(BINDIR)$1 $$($1_OBJECTS) $$($1_LIBS) $(LDFLAGS)

endef

define make_sharedlib
ALL_TARGETS=$(ALL_TARGETS) $(LIBDIR)$1
$1_SOURCES:=$$(abspath $2)
$1_OBJECTS:=$$($1_SOURCES:$(PROJDIR)%.cpp=$(OBJDIR)%.o)
$1_OBJECTS:=$$($1_OBJECTS:$(PROJDIR)%.c=$(OBJDIR)%.o)
$1_LIBS:=$(call expand_libflags,$3)
$(LIBDIR)$1: $$($1_OBJECTS)
	@mkdir -p $(LIBDIR)
	$(CXX) -shared -o $(LIBDIR)$1 $$($1_OBJECTS) $$($1_LIBS) $(LDFLAGS)

endef

define make_staticlib

endef

#define run_command

#$(foreach var,$1,echo ${var}:; echo ${${var}}; echo;)
define print_vars
$(foreach var,$1,$(info ${var}: ${${var}}))

endef 

