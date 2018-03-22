PROJDIR = ../../

include $(PROJDIR)defs.mk
include $(PROJDIR)functions.mk
include $(PROJDIR)rules.mk

#TARGET = void

LIBS = wayland-server++ pixman-1 input dl EGL wayland-client++ wayland-egl++ wayland-cursor++ wayland-shm++ xdg_shell_unstable_v6-server++ GLESv2 wayland-server

LDFLAGS += -Wl,-E

SRCS = \
	   void.cpp \
	   void_xdg.cpp \
	   wrapper.cpp \




$(eval $(call make_executable,void,$(SRCS),$(LIBS)))

$(eval $(call print_vars,ALL_TARGETS))

all: $$(ALL_TARGETS)





