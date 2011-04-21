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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

#include "psensor.h"
#include "ui.h"
#include "ui_appindicator.h"
#include "ui_pref.h"

static void cb_appindicator_show(GtkWidget *widget, gpointer data)
{
	struct ui_psensor *ui = (struct ui_psensor *)data;

	gtk_window_present(GTK_WINDOW(ui->main_window));

}

static void cb_appindicator_quit(GtkWidget *widget, gpointer data)
{
	ui_psensor_exit(data);
}

static void cb_appindicator_preferences(GtkWidget *widget, gpointer data)
{
	gdk_threads_enter();
	ui_pref_dialog_run((struct ui_psensor *)data);
	gdk_threads_leave();
}

GtkWidget *ui_appindicator_get_menu(struct ui_psensor *ui)
{
	GtkWidget *menu, *item;

	menu = gtk_menu_new();

	item = gtk_menu_item_new_with_label(_("Show"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(item,
			 "activate", G_CALLBACK(cb_appindicator_show), ui);

	item = gtk_menu_item_new_with_label(_("Preferences"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(item,
			 "activate", G_CALLBACK(cb_appindicator_preferences),
			 ui);

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_menu_item_new_with_label(_("Quit"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	g_signal_connect(item,
			 "activate", G_CALLBACK(cb_appindicator_quit), ui);

	gtk_widget_show_all(menu);

	return menu;
}

void ui_appindicator_update(struct ui_psensor *ui)
{
	struct psensor **sensor_cur = ui->sensors;
	AppIndicatorStatus status;
	int attention = 0;

	if (!ui->indicator)
		return;

	while (*sensor_cur) {
		struct psensor *s = *sensor_cur;

		if (s->alarm_enabled && s->alarm_raised) {
			attention = 1;
			break;
		}

		sensor_cur++;
	}

	status = app_indicator_get_status(ui->indicator);

	if (!attention && status == APP_INDICATOR_STATUS_ATTENTION)
		app_indicator_set_status
		    (ui->indicator, APP_INDICATOR_STATUS_ACTIVE);

	if (attention && status == APP_INDICATOR_STATUS_ACTIVE)
		app_indicator_set_status
		    (ui->indicator, APP_INDICATOR_STATUS_ATTENTION);
}

void ui_appindicator_init(struct ui_psensor *ui)
{
	GtkWidget *indicatormenu;

	ui->indicator
	    = app_indicator_new("psensor",
				"psensor",
				APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	indicatormenu = ui_appindicator_get_menu(ui);

	app_indicator_set_status(ui->indicator, APP_INDICATOR_STATUS_ACTIVE);
	app_indicator_set_attention_icon(ui->indicator, "psensor_hot");

	app_indicator_set_menu(ui->indicator, GTK_MENU(indicatormenu));
}
