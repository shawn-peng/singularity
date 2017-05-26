
PROJDIR:= $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/

BUILDDIR=$(PROJDIR)build/
BINDIR=$(BUILDDIR)bin/
LIBDIR=$(BUILDDIR)lib/
OBJDIR=$(BUILDDIR)obj/
INCDIR=$(PROJDIR)include/
SRCDIR=$(PROJDIR)src/

DEPDIR=$(PROJDIR)dependencies/
WLPPDIR=$(DEPDIR)waylandpp/

OPTS= -fpermissive -std=c++11 -fPIC -g -pthread #-fkeep-inline-functions

INCLUDES= -I/usr/local/include/ -I/usr/include/pixman-1/ -I/usr/include/libdrm/ \
		  -I$(INCDIR) -I$(WLPPDIR)include/

LIBPATHS= -L/usr/local/lib -L$(WLPPDIR)/build/lib/ -L$(LIBDIR)

COMMONLIBS= -lm -lwayland-egl -lEGL -lwld -lswc
#LIBS= -lm -lwayland-client -lwayland-server -lwayland-egl -lwayland-cursor -lEGL -lGL -lwld -lswc

MACROS=

CFLAGS= $(OPTS) $(INCLUDES) $(MACROS)
CXXFLAGS= $(OPTS) $(INCLUDES) $(MACROS)
LDFLAGS= $(OPTS) $(LIBPATHS) $(COMMONLIBS) #-Wl,-rpath=$(WLPPDIR)build/lib/

