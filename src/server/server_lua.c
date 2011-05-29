/*
    Copyright (C) 2010-2011 wpitchoune@gmail.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301 USA
*/

#include <stdlib.h>
#include <string.h>

#include "server_lua.h"

#include "plib/plib_luatpl.h"

int init_lua(lua_State *L, void *data)
{
	struct server_data *server_data = data;
	struct psensor **s_cur;
	struct psensor **sensors = server_data->sensors;
	int i;

#ifdef HAVE_GTOP
	lua_newtable(L);
	lua_pushstring(L, "load");
	lua_pushnumber(L, server_data->cpu_rate);
	lua_settable(L, -3);
	lua_setglobal(L, "cpu");
#endif

	lua_newtable(L);

	s_cur = sensors;
	i = 1;
	while (*s_cur) {
		lua_pushnumber(L, i);

		lua_newtable(L);

		lua_pushstring(L, "name");
		lua_pushstring(L, (*s_cur)->name);
		lua_settable(L, -3);

		lua_pushstring(L, "measure_last");
		lua_pushnumber(L, psensor_get_current_value(*s_cur));
		lua_settable(L, -3);

		lua_pushstring(L, "measure_min");
		lua_pushnumber(L, (*s_cur)->min);
		lua_settable(L, -3);

		lua_pushstring(L, "measure_max");
		lua_pushnumber(L, (*s_cur)->max);
		lua_settable(L, -3);

		lua_settable(L, -3);

		s_cur++;
		i++;
	}

	lua_setglobal(L, "sensors");

	return 1;
}

char *lua_to_html_page(struct server_data *server_data, const char *fpath)
{
	char *page = NULL;
	struct luatpl_error err;

	err.message = NULL;

	page = luatpl_generate(fpath,
			       init_lua,
			       server_data,
			       &err);

	if (!page) {
		luatpl_fprint_error(stderr,
				    &err,
				    fpath,
				    "outstring");
		free(err.message);
	}

	return page;
}
