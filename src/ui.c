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

#include "cfg.h"
#include "ui.h"

void on_destroy(GtkWidget *widget, gpointer data)
{
	ui_psensor_exit();
}

void ui_psensor_exit()
{
	gtk_main_quit();
}

GtkWidget *ui_window_create(struct ui_psensor * ui)
{
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GdkScreen *screen;
	GdkColormap *colormap;
	GdkPixbuf *icon;
	GtkIconTheme *icon_theme;

	gtk_window_set_default_size(GTK_WINDOW(window), 800, 200);

	gtk_window_set_title(GTK_WINDOW(window),
			     _("Psensor - Temperature Monitor"));
	gtk_window_set_role(GTK_WINDOW(window), "psensor");

	screen = gtk_widget_get_screen(window);

	if (ui->config->alpha_channel_enabled
	    && gdk_screen_is_composited(screen)) {

		colormap = gdk_screen_get_rgba_colormap(screen);
		if (colormap)
			gtk_widget_set_colormap(window, colormap);
		else
			ui->config->alpha_channel_enabled = 0;
	} else {
		ui->config->alpha_channel_enabled = 0;
	}

	icon_theme = gtk_icon_theme_get_default();
	icon = gtk_icon_theme_load_icon(icon_theme, "psensor", 48, 0, NULL);
	if (icon)
		gtk_window_set_icon(GTK_WINDOW(window), icon);
	else
		fprintf(stderr, _("ERROR: Failed to load psensor icon.\n"));

	g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), ui);

	gtk_window_set_decorated(GTK_WINDOW(window),
				 ui->config->window_decoration_enabled);

	gtk_window_set_keep_below(GTK_WINDOW(window),
				  ui->config->window_keep_below_enabled);

	return window;
}
