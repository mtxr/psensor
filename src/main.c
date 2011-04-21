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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include <sensors/sensors.h>
#include <sensors/error.h>

#include "config.h"

#include "cfg.h"
#include "hdd.h"
#include "psensor.h"
#include "graph.h"
#include "ui.h"
#include "ui_sensorlist.h"
#include "ui_color.h"
#include "lmsensor.h"
#include "ui_pref.h"

#ifdef HAVE_NVIDIA
#include "nvidia.h"
#endif

#ifdef HAVE_REMOTE_SUPPORT
#include "rsensor.h"
#endif

#ifdef HAVE_APPINDICATOR
#include "ui_appindicator.h"
#endif

#ifdef HAVE_LIBNOTIFY
#include "ui_notify.h"
#endif

#include "compat.h"

static const char *program_name;

void print_version()
{
	printf("psensor %s\n", VERSION);
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

	puts(_("psensor is a GTK application for monitoring hardware sensors, "
	       "including temperatures and fan speeds."));

	puts("");
	puts("Options:");
	puts(_("\
  -h, --help          display this help and exit\n\
  -v, --version       display version information and exit"));

	puts("");

	puts(_("\
  -u, --url=URL       \
the URL of the psensor-server, example: http://hostname:3131"));

	puts("");

	printf(_("Report bugs to: %s\n"), PACKAGE_BUGREPORT);
	puts("");
	printf(_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
}

static void
cb_preferences(gpointer data, guint callback_action, GtkWidget *item)
{
	ui_pref_dialog_run((struct ui_psensor *)data);
}

static GtkItemFactoryEntry menu_items[] = {
	{"/Preferences",
	 NULL, cb_preferences, 0, "<Item>"},

	{"/sep1",
	 NULL, NULL, 0, "<Separator>"},

	{"/Quit",
	 "", ui_psensor_exit, 0, "<StockItem>", GTK_STOCK_QUIT},
};

static gint nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);

/*
  Updates the size of the sensor values if different than the
  configuration.
 */
void
update_psensor_values_size(struct psensor **sensors, struct config *cfg)
{
	struct psensor **cur;

	cur = sensors;
	while (*cur) {
		struct psensor *s = *cur;

		if (s->values_max_length != cfg->sensor_values_max_length)
			psensor_values_resize(s,
					      cfg->sensor_values_max_length);

		cur++;
	}
}

void update_psensor_measures(struct ui_psensor *ui)
{
	struct psensor **sensors = ui->sensors;
	struct config *cfg = ui->config;

	while (1) {
		gdk_threads_enter();

		if (!sensors)
			return;

		update_psensor_values_size(sensors, ui->config);

		psensor_list_update_measures(sensors);
#ifdef HAVE_REMOTE_SUPPORT
		remote_psensor_list_update(sensors);
#endif
#ifdef HAVE_NVIDIA
		nvidia_psensor_list_update(sensors);
#endif

		gdk_threads_leave();

		sleep(cfg->sensor_update_interval);
	}
}

gboolean ui_refresh_thread(gpointer data)
{
	struct config *cfg;
	gboolean ret;
	struct ui_psensor *ui = (struct ui_psensor *)data;

	ret = TRUE;
	cfg = ui->config;

	gdk_threads_enter();

	graph_update(ui->sensors, ui->w_graph, ui->config);

	ui_sensorlist_update(ui->ui_sensorlist);

#ifdef HAVE_APPINDICATOR
	ui_appindicator_update(ui);
#endif

	if (ui->graph_update_interval != cfg->graph_update_interval) {
		ui->graph_update_interval = cfg->graph_update_interval;
		ret = FALSE;
	}

	gdk_threads_leave();

	if (ret == FALSE)
		g_timeout_add(1000 * ui->graph_update_interval,
			      ui_refresh_thread, ui);

	return ret;
}

gboolean
on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	struct ui_psensor *ui_psensor = (struct ui_psensor *)data;

	graph_update(ui_psensor->sensors,
		     ui_psensor->w_graph, ui_psensor->config);

	return FALSE;
}

void cb_alarm_raised(struct psensor *sensor, void *data)
{
#ifdef HAVE_LIBNOTIFY
	if (sensor->enabled)
		ui_notify(sensor, (struct ui_psensor *)data);
#endif
}

void associate_colors(struct psensor **sensors)
{
	/* number of uniq colors */
#define COLORS_COUNT 8

	unsigned int colors[COLORS_COUNT][3] = {
		{0x0000, 0x0000, 0x0000},	/* black */
		{0xffff, 0x0000, 0x0000},	/* red */
		{0x0000, 0.0000, 0xffff},	/* blue */
		{0x0000, 0xffff, 0x0000},	/* green */

		{0x7fff, 0x7fff, 0x7fff},	/* grey */
		{0x7fff, 0x0000, 0x0000},	/* dark red */
		{0x0000, 0x0000, 0x7fff},	/* dark blue */
		{0x0000, 0x7fff, 0x0000}	/* dark green */
	};

	struct psensor **sensor_cur = sensors;
	int i = 0;
	while (*sensor_cur) {
		struct color default_color;
		color_set(&default_color,
			  colors[i % COLORS_COUNT][0],
			  colors[i % COLORS_COUNT][1],
			  colors[i % COLORS_COUNT][2]);

		(*sensor_cur)->color
		    = config_get_sensor_color((*sensor_cur)->id,
					      &default_color);

		sensor_cur++;
		i++;
	}
}

void associate_cb_alarm_raised(struct psensor **sensors, struct ui_psensor *ui)
{
	struct psensor **sensor_cur = sensors;
	while (*sensor_cur) {
		struct psensor *s = *sensor_cur;

		s->cb_alarm_raised = cb_alarm_raised;
		s->cb_alarm_raised_data = ui;

		if (is_temp_type(s->type)) {
			s->alarm_limit
			    = config_get_sensor_alarm_limit(s->id, 60);
			s->alarm_enabled
			    = config_get_sensor_alarm_enabled(s->id);
		} else {
			s->alarm_limit = 0;
			s->alarm_enabled = 0;
		}

		sensor_cur++;
	}
}

void associate_preferences(struct psensor **sensors)
{
	struct psensor **sensor_cur = sensors;
	while (*sensor_cur) {
		char *n;
		struct psensor *s = *sensor_cur;

		s->enabled = config_is_sensor_enabled(s->id);

		n = config_get_sensor_name(s->id);

		if (n)
			s->name = n;

		sensor_cur++;
	}
}

GtkWidget *ui_get_popupmenu(gpointer data)
{
	GtkItemFactory *item_factory;
	GtkWidget *menu;

	item_factory = gtk_item_factory_new(GTK_TYPE_MENU, "<main>", NULL);
	gtk_item_factory_create_items(item_factory,
				      nmenu_items, menu_items, data);
	menu = gtk_item_factory_get_widget(item_factory, "<main>");

	return menu;
}

int on_graph_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkWidget *menu;

	if (event->type != GDK_BUTTON_PRESS)
		return FALSE;

	menu = ui_get_popupmenu(data);

	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
		       event->button, event->time);

	return TRUE;
}


GtkWidget *create_graph_widget(struct ui_psensor * ui)
{
	GtkWidget *w_graph;

	w_graph = gtk_drawing_area_new();

	g_signal_connect(G_OBJECT(w_graph),
			 "expose-event", G_CALLBACK(on_expose_event), ui);

	gtk_widget_add_events(w_graph, GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(w_graph),
			   "button_press_event",
			   (GCallback) on_graph_clicked, ui);

	return w_graph;
}

void ui_main_box_create(struct ui_psensor *ui)
{
	struct config *cfg;
	GtkWidget *w_sensorlist;

	cfg = ui->config;

	if (ui->main_box) {
		ui_sensorlist_create_widget(ui->ui_sensorlist);

		gtk_container_remove(GTK_CONTAINER(ui->main_window),
				     ui->main_box);

		ui->w_graph = create_graph_widget(ui);
		ui->w_sensorlist = ui->ui_sensorlist->widget;
	}

	if (cfg->sensorlist_position == SENSORLIST_POSITION_RIGHT
	    || cfg->sensorlist_position == SENSORLIST_POSITION_LEFT)
		ui->main_box = gtk_hpaned_new();
	else
		ui->main_box = gtk_vpaned_new();

	w_sensorlist = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(w_sensorlist),
				       GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(w_sensorlist),
			  ui->ui_sensorlist->widget);

	gtk_container_add(GTK_CONTAINER(ui->main_window), ui->main_box);

	if (cfg->sensorlist_position == SENSORLIST_POSITION_RIGHT
	    || cfg->sensorlist_position == SENSORLIST_POSITION_BOTTOM) {
		gtk_paned_pack1(GTK_PANED(ui->main_box),
				GTK_WIDGET(ui->w_graph), TRUE, TRUE);
		gtk_paned_pack2(GTK_PANED(ui->main_box),
				w_sensorlist, FALSE, TRUE);
	} else {
		gtk_paned_pack1(GTK_PANED(ui->main_box),
				w_sensorlist, FALSE, TRUE);
		gtk_paned_pack2(GTK_PANED(ui->main_box),
				GTK_WIDGET(ui->w_graph), TRUE, TRUE);
	}

	gtk_widget_show_all(ui->main_box);
}

static struct option long_options[] = {
	{"version", no_argument, 0, 'v'},
	{"help", no_argument, 0, 'h'},
	{"url", required_argument, 0, 'u'},
	{0, 0, 0, 0}
};

int main(int argc, char **argv)
{
	struct ui_psensor ui;
	GError *error;
	GThread *thread;
	int err, optc;
	char *url = NULL;
	int cmdok = 1;

	program_name = argv[0];

	setlocale(LC_ALL, "");

#if ENABLE_NLS
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	while ((optc = getopt_long(argc, argv, "vhu:", long_options,
				   NULL)) != -1) {
		switch (optc) {
		case 'u':
			if (optarg)
				url = strdup(optarg);
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

	g_thread_init(NULL);
	gdk_threads_init();
	gdk_threads_enter();

	gtk_init(&argc, &argv);

#ifdef HAVE_LIBNOTIFY
	ui.notification_last_time = NULL;
#endif

	config_init();

	ui.config = config_load();

	err = lmsensor_init();
	if (!err) {
		fprintf(stderr, _("ERROR: lmsensor init failure: %s\n"),
			sensors_strerror(err));
		exit(EXIT_FAILURE);
	}

	if (url) {
#ifdef HAVE_REMOTE_SUPPORT
		rsensor_init();
		ui.sensors = get_remote_sensors(url, 600);
#else
		fprintf(stderr,
			_("ERROR: Not compiled with remote sensor support.\n"));
		exit(EXIT_FAILURE);
#endif
	} else {
#ifdef HAVE_NVIDIA
		struct psensor **tmp;

		tmp = get_all_sensors(600);
		ui.sensors = nvidia_psensor_list_add(tmp, 600);

		if (tmp != ui.sensors)
			free(tmp);
#else
		ui.sensors = get_all_sensors(600);
#endif
	}

	associate_preferences(ui.sensors);
	associate_colors(ui.sensors);
	associate_cb_alarm_raised(ui.sensors, &ui);

	/* main window */
	ui.main_window = ui_window_create(&ui);
	ui.main_box = NULL;

	/* drawing box */
	ui.w_graph = create_graph_widget(&ui);

	/* sensor list */
	ui.ui_sensorlist = ui_sensorlist_create(ui.sensors);

	ui_main_box_create(&ui);

	gtk_widget_show_all(ui.main_window);

	thread = g_thread_create((GThreadFunc) update_psensor_measures,
				 &ui, TRUE, &error);

	if (!thread)
		g_error_free(error);

	ui.graph_update_interval = ui.config->graph_update_interval;

	g_timeout_add(1000 * ui.graph_update_interval, ui_refresh_thread, &ui);

#ifdef HAVE_APPINDICATOR
	ui_appindicator_init(&ui);
#endif

	/* main loop */
	gtk_main();

	sensors_cleanup();

	psensor_list_free(ui.sensors);

	gdk_threads_leave();

	return 0;
}
