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
#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <getopt.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <microhttpd.h>

#ifdef HAVE_GTOP
#include "sysinfo.h"
#endif

#ifdef HAVE_LUA
#include "server_lua.h"
#endif

#include "psensor_json.h"
#include "plib/url.h"
#include "plib/plib_io.h"
#include "server.h"

static const char *program_name;

#define DEFAULT_PORT 3131

#define PAGE_NOT_FOUND \
"<html><body><p>Page not found - Go to <a href='/index.lua'>Main page</a>\
</p></body>"

static struct option long_options[] = {
	{"version", no_argument, 0, 'v'},
	{"help", no_argument, 0, 'h'},
	{"port", required_argument, 0, 'p'},
	{"wdir", required_argument, 0, 'w'},
	{0, 0, 0, 0}
};

static struct server_data server_data;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void print_version()
{
	printf("psensor-server %s\n", VERSION);
	printf(_("Copyright (C) %s wpitchoune@gmail.com\n\
License GPLv2: GNU GPL version 2 or later \
<http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n"),
	       "2010-2011");
}

void print_help()
{
	printf(_("Usage: %s [OPTION]...\n"), program_name);

	puts(_("psensor-server is an HTTP server "
	       "for monitoring hardware sensors remotely."));

	puts("");
	puts("Options:");
	puts(_("\
  -h, --help          display this help and exit\n\
  -v, --version       display version information and exit"));

	puts("");

	puts(_("\
  -p,--port=PORT webserver port\n\
  -w,--wdir=DIR  directory containing webserver pages"));

	puts("");

	printf(_("Report bugs to: %s\n"), PACKAGE_BUGREPORT);
	puts("");
	printf(_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
}

/*
  Returns '1' if the path denotates a Lua file, otherwise returns 0.
 */
int is_path_lua(const char *path)
{
	char *dot = rindex(path, '.');

	if (dot && !strcasecmp(dot, ".lua"))
		return 1;

	return 0;
}

/*
  Returns the file path corresponding to a given URL
*/
char *get_path(const char *url, const char *www_dir)
{
	const char *p;
	char *res;

	if (!strlen(url) || !strcmp(url, ".") || !strcmp(url, "/"))
		p = "/index.lua";
	else
		p = url;

	res = malloc(strlen(www_dir)+strlen(p)+1);

	strcpy(res, www_dir);
	strcat(res, p);

	return res;
}

static int cbk_http_request(void *cls,
			    struct MHD_Connection *connection,
			    const char *url,
			    const char *method,
			    const char *version,
			    const char *upload_data,
			    size_t *upload_data_size, void **ptr)
{
	static int dummy;
	struct MHD_Response *response;
	int ret;
	char *nurl;
	unsigned int resp_code;
	char *page = NULL;

	if (strcmp(method, "GET"))
		return MHD_NO;	/* unexpected method */

	if (&dummy != *ptr) {
		/* The first time only the headers are valid, do not
		   respond in the first round... */
		*ptr = &dummy;
		return MHD_YES;
	}

	if (*upload_data_size)
		return MHD_NO;	/* upload data in a GET!? */

	*ptr = NULL;		/* clear context pointer */

	nurl = url_normalize(url);

	pthread_mutex_lock(&mutex);

	if (!strcmp(nurl, URL_BASE_API_1_0_SENSORS)) {
		page = sensors_to_json_string(server_data.sensors);

	} else if (!strncmp(nurl, URL_BASE_API_1_0_SENSORS,
			    strlen(URL_BASE_API_1_0_SENSORS))
		   && nurl[strlen(URL_BASE_API_1_0_SENSORS)] == '/') {

		char *sid = nurl + strlen(URL_BASE_API_1_0_SENSORS) + 1;
		struct psensor *s
			= psensor_list_get_by_id(server_data.sensors, sid);

		if (s)
			page = sensor_to_json_string(s);

	} else {
		char *fpath = get_path(nurl, server_data.www_dir);
		int n = strlen(nurl);

		if (is_path_lua(fpath)) {
#if HAVE_LUA
			page = lua_to_html_page(&server_data, fpath);
#else
			page = strdup(_("ERROR: Lua support not enabled\n"));
#endif
		} else {
			page = file_get_content(fpath);
		}

		free(fpath);
	}

	if (page) {
		resp_code = MHD_HTTP_OK;
	} else {
		page = strdup(PAGE_NOT_FOUND);
		resp_code = MHD_HTTP_NOT_FOUND;
	}

	pthread_mutex_unlock(&mutex);

	response = MHD_create_response_from_data(strlen(page),
						 (void *)page, MHD_YES, MHD_NO);

	ret = MHD_queue_response(connection, resp_code, response);
	MHD_destroy_response(response);

	free(nurl);

	return ret;
}

int main(int argc, char *argv[])
{
	struct MHD_Daemon *d;
	int port = DEFAULT_PORT;
	int optc;
	int cmdok = 1;

	program_name = argv[0];

	setlocale(LC_ALL, "");

#if ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	server_data.www_dir = DEFAULT_WWW_DIR;

	while ((optc = getopt_long(argc, argv,
				   "vhp:w:", long_options, NULL)) != -1) {
		switch (optc) {
		case 'w':
			if (optarg)
				server_data.www_dir = strdup(optarg);
			break;
		case 'p':
			if (optarg)
				port = atoi(optarg);
			break;
		case 'h':
			print_help();
			exit(EXIT_SUCCESS);
		case 'v':
			print_version();
			exit(EXIT_SUCCESS);
		default:
			cmdok = 0;
			break;
		}
	}

	if (!cmdok || optind != argc) {
		fprintf(stderr, _("Try `%s --help' for more information.\n"),
			program_name);
		exit(EXIT_FAILURE);
	}

	if (!lmsensor_init()) {
		fprintf(stderr, _("ERROR: failed to init lm-sensors\n"));
		exit(EXIT_FAILURE);
	}

	server_data.sensors = get_all_sensors(1);

	if (!*server_data.sensors)
		fprintf(stderr, _("ERROR: no sensors detected\n"));

	d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
			     port,
			     NULL, NULL, &cbk_http_request, server_data.sensors,
			     MHD_OPTION_END);
	if (!d) {
		fprintf(stderr, _("ERROR: Fail to create web server\n"));
		exit(EXIT_FAILURE);
	}

	printf(_("Web server started on port: %d\n"), port);
	printf(_("WWW directory: %s\n"), server_data.www_dir);
	printf(_("URL: http://localhost:%d\n"), port);

	while (1) {
		pthread_mutex_lock(&mutex);

#ifdef HAVE_GTOP
		sysinfo_update(&server_data.cpu_rate);
#endif
		psensor_list_update_measures(server_data.sensors);

		pthread_mutex_unlock(&mutex);
		sleep(5);
	}

	MHD_stop_daemon(d);

	return 0;
}
