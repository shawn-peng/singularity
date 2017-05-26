#!/bin/bash

. conf

OPTS=--display=wayland-0

if [ "$1" == "-g" ]; then
	WAYLAND_DEBUG=server gdb --args build/bin/void ${OPTS}
else
	build/bin/void ${OPTS}
fi

