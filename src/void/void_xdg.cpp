/* void_xdg.cpp
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

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
//#include <GL/gl.h>
//#include <GL/glext.h>
//#include <GL/glew.h>

#include <EGL/eglext.h>
#include <pixman-1/pixman.h>

#include "void.hpp"

using namespace wayland;
using namespace wayland::detail;

void void_zxdg_shell_v6::bind(resource_t res, void *data) {
	std::cout << "client bind void_zxdg_shell_v6" << std::endl;

	auto r = new zxdg_shell_v6_resource_t(res);

	r->on_get_xdg_surface() = [&](zxdg_surface_v6_resource_t res,
			surface_resource_t wlsurf_res) {
		auto p = new void_zxdg_surface_v6(compositor);
		p->bind(res);
		p->bind_wlsurface(wlsurf_res);
	};
}

void void_zxdg_surface_v6::bind(zxdg_surface_v6_resource_t res) {
	resource = res;

	res.on_get_toplevel() = [&](zxdg_toplevel_v6_resource_t top_res) {
		auto p = new void_zxdg_toplevel_v6(compositor);
		p->bind(top_res);
		p->bind_surface(res);
	};
}

void void_zxdg_toplevel_v6::bind(zxdg_toplevel_v6_resource_t res) {
	resource = res;
	res.set_user_data(this);

	res.on_set_parent() = [&](zxdg_toplevel_v6_resource_t parent_res) {
		if (!parent_res) {
			return;
		}

		// add reference to parent
		auto parent_toplevel = (void_zxdg_toplevel_v6 *)parent_res.get_user_data();
		parent_toplevel->children.push_back(this);
	};

	res.on_set_title() = [&](std::string title) {
		cout << "setting title: " << title << endl;
		this->title = title;
	};

	res.on_set_app_id() = [&](std::string appid) {
		cout << "setting appid: " << appid << endl;
		this->appid = appid;
	};

	res.on_set_maximized() = [&]() {
		this->maximized = true;
		int w, h;
		w = compositor->get_width();
		h = compositor->get_height();
		
		cout << "setting maximized (" << w << ", " << h << ")" << endl;
		array_t states{zxdg_toplevel_v6_state::maximized};
		resource.send_configure(w, h, states);
	};
}


