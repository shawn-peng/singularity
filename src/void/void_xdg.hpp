/* void_xdg.hpp
 *
 * Copyright (c) 2016-2017 Yisu Peng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>

#include <iostream>
#include <queue>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <thread>

#include <wayland-util.hpp>
#include <wayland-shm.hpp>

#include <wayland-server.hpp>
#include <xdg_shell_unstable_v6-server-protocol.hpp>

#include <pixman-1/pixman.h>

#include "wrapper.hpp"
//#include "void.hpp"

class void_compositor;
class void_surface;


class void_zxdg_surface_v6 {
private:
	wayland::zxdg_surface_v6_resource_t resource;
	void_compositor *compositor;
	void_surface *wlsurf;
	bool surface_grabbing;

public:
	void_zxdg_surface_v6(void_compositor *c)
		: compositor(c),
		surface_grabbing(false)
	{
	}

	void bind(wayland::zxdg_surface_v6_resource_t surf);

	void bind_wlsurface(wayland::surface_resource_t wlsurf_res) {
		//wlsurf_res = surf;
		wlsurf = (void_surface *)wlsurf_res.get_user_data();
	}
};

class void_zxdg_toplevel_v6 {
private:
	wayland::zxdg_toplevel_v6_resource_t resource;
	void_compositor *compositor;
	void_zxdg_surface_v6 *surface;
	std::list<void_zxdg_toplevel_v6 *> children;
	std::string title;
	bool maximized;
	std::string appid;

public:
	void_zxdg_toplevel_v6(void_compositor *c)
		: compositor(c)
	{
	}

	void bind(wayland::zxdg_toplevel_v6_resource_t res);
	void bind_surface(wayland::zxdg_surface_v6_resource_t surf_res) {
		surface = (void_zxdg_surface_v6 *)surf_res.get_user_data();
	}
};

/**
 * 		GLOBALS
 */

class void_zxdg_shell_v6 : public wayland::global_t {
private:
	wayland::display_server_t display;
	void_compositor *compositor;

public:
	void_zxdg_shell_v6(wayland::display_server_t disp,
			void_compositor *c)
		: global_t(disp, wayland::detail::zxdg_shell_v6_interface, 1, this, NULL),
		display(disp),
		compositor(c)
	{
	}

	virtual void bind(wayland::resource_t res, void *data);
};





