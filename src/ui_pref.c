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

#include "ui.h"
#include "cfg.h"
#include "ui_pref.h"
#include "ui_color.h"
#include "compat.h"

struct ui_pref {
	/* Main container widget */
	GtkWidget *widget;

	GtkWidget *w_opacity;

	GtkWidget *w_graph_fgcolor;
	GtkWidget *w_graph_bgcolor;

	GtkWidget *w_sensorlist_position;

	GtkWidget *w_window_decoration_enabled;
	GtkWidget *w_window_keep_below_enabled;

	GtkWidget *w_graph_update_interval;
	GtkWidget *w_graph_monitoring_duration;

	GtkWidget *w_sensor_update_interval;
};

GdkColor *color_to_gdkcolor(struct color *color)
{
	GdkColor *c = malloc(sizeof(GdkColor));

	c->red = color->red;
	c->green = color->green;
	c->blue = color->blue;

	return c;
}

void ui_pref_add_label(GtkWidget *table,
		       const char *text, guint col, guint row)
{
	GtkWidget *alig;
	guint xpad;

	alig = gtk_alignment_new(0, 0.5, 0, 0);

	if (col == 0)
		xpad = 24;
	else
		xpad = 4;

	gtk_alignment_set_padding(GTK_ALIGNMENT(alig), 4, 4, xpad, 4);

	gtk_container_add(GTK_CONTAINER(alig), GTK_WIDGET(gtk_label_new(text)));

	gtk_table_attach(GTK_TABLE(table),
			 alig,
			 col, col + 1,
			 row, row + 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);
}

GtkWidget *ui_pref_add_check_button(GtkWidget * table,
				    const char *text, int enabled, guint row)
{
	GtkWidget *btn, *alig;

	btn = gtk_check_button_new_with_label(text);
	if (enabled)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(btn), TRUE);

	alig = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alig), 4, 4, 24, 4);
	gtk_container_add(GTK_CONTAINER(alig), GTK_WIDGET(btn));

	gtk_table_attach(GTK_TABLE(table),
			 alig,
			 0, 2,
			 row, row + 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);

	return btn;
}

void ui_pref_add_section_title(GtkWidget *table, const char *title, guint row)
{
	char *markup;
	GtkWidget *alig, *label;

	markup = malloc(3 + strlen(title) + 4 + 1);
	sprintf(markup, "<b>%s</b>", title);

	alig = gtk_alignment_new(0, 0, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alig), 8, 4, 8, 4);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), markup);
	gtk_container_add(GTK_CONTAINER(alig), label);

	gtk_table_attach(GTK_TABLE(table),
			 alig,
			 0, 2,
			 row, row + 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);

	free(markup);
}

GtkWidget *ui_pref_add_color_button(GtkWidget * table,
				    const char *text,
				    struct color *color, guint row)
{
	GtkWidget *alig, *btn;
	GdkColor *gdkcolor;

	ui_pref_add_label(table, text, 0, row);

	alig = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alig), 4, 4, 4, 4);
	gdkcolor = color_to_gdkcolor(color);
	btn = ui_pref_create_color_button(gdkcolor);
	free(gdkcolor);

	gtk_container_add(GTK_CONTAINER(alig), btn);

	gtk_table_attach(GTK_TABLE(table),
			 alig,
			 1, 2,
			 row, row + 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);

	return btn;
}

GtkWidget *ui_pref_add_spin_button(GtkWidget * table,
				   int value, int min, int max, guint row)
{
	GtkWidget *alig;
	GtkSpinButton *spin_button;

	spin_button = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(min,
								     max, 1));

	gtk_widget_set_sensitive(GTK_WIDGET(spin_button), TRUE);
	gtk_spin_button_set_value(spin_button, value);
	alig = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alig), 4, 4, 4, 4);
	gtk_container_add(GTK_CONTAINER(alig), GTK_WIDGET(spin_button));
	gtk_table_attach(GTK_TABLE(table),
			 alig,
			 1, 2,
			 row, row + 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);
	return GTK_WIDGET(spin_button);
}

GtkWidget *ui_pref_create_color_button(GdkColor * color)
{
	return gtk_color_button_new_with_color(color);
}

struct ui_pref *ui_pref_create_main_widget(struct ui_psensor *ui_psensor)
{
	GtkWidget *table, *w_opacity, *label, *hbox, *alig;
	struct ui_pref *pref;
	struct config *cfg;
	guint row;

	cfg = ui_psensor->config;

	pref = malloc(sizeof(struct ui_pref));

	table = gtk_table_new(14, 2, FALSE);
	gtk_table_set_col_spacing(GTK_TABLE(table), 0, 8);

	row = 0;

	/* Graph Colors section */
	ui_pref_add_section_title(table, _("Graph Colors"), row++);

	pref->w_graph_fgcolor
	    = ui_pref_add_color_button(table,
				       _("Foreground:"),
				       cfg->graph_fgcolor, row++);

	pref->w_graph_bgcolor
	    = ui_pref_add_color_button(table,
				       _("Background:"),
				       cfg->graph_bgcolor, row++);

	ui_pref_add_label(table, _("Background opacity:"), 0, row++);

	w_opacity = gtk_hscale_new_with_range(0, 1, 0.1);
	gtk_scale_set_draw_value(GTK_SCALE(w_opacity), FALSE);
	gtk_range_set_value(GTK_RANGE(w_opacity), cfg->graph_bg_alpha);

	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), _("<i>Min</i>"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), w_opacity, TRUE, TRUE, 0);
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), _("<i>Max</i>"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(table),
			 hbox,
			 0, 2,
			 row, row + 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 28, 4);
	row++;

	/* Graph section */
	ui_pref_add_section_title(table, _("Graph"), row++);

	ui_pref_add_label(table, _("Graph update interval:"), 0, row);
	pref->w_graph_update_interval
	    = ui_pref_add_spin_button(table,
				      cfg->graph_update_interval, 1, 60, row++);

	ui_pref_add_label(table, _("Graph monitoring duration:"), 0, row);
	pref->w_graph_monitoring_duration
		= ui_pref_add_spin_button(table,
					  cfg->graph_monitoring_duration,
					  1, 24 * 60,	/* 24h */
					  row++);

	/* Sensor section */
	ui_pref_add_section_title(table, _("Sensor"), row++);

	ui_pref_add_label(table, _("Measure update interval:"), 0, row);
	pref->w_sensor_update_interval
	    = ui_pref_add_spin_button(table,
				      cfg->sensor_update_interval,
				      1, 60, row++);

	/* Interface section */
	ui_pref_add_section_title(table, _("Interface"), row++);

	ui_pref_add_label(table, _("Position of sensors table:"), 0, row);
	pref->w_sensorlist_position = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(pref->w_sensorlist_position),
				  _("Right"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(pref->w_sensorlist_position),
				  _("Left"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(pref->w_sensorlist_position),
				  _("Top"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(pref->w_sensorlist_position),
				  _("Bottom"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(pref->w_sensorlist_position),
				 cfg->sensorlist_position);

	alig = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alig), 4, 4, 4, 4);
	gtk_container_add(GTK_CONTAINER(alig), pref->w_sensorlist_position);
	gtk_table_attach(GTK_TABLE(table),
			 alig,
			 1, 2,
			 row, row + 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);
	row++;

	pref->w_window_decoration_enabled = ui_pref_add_check_button
		(table, _("Hide window decoration"),
		 !cfg->window_decoration_enabled, row++);

	pref->w_window_keep_below_enabled = ui_pref_add_check_button
		(table,
		 _("Keep window below"),
		 cfg->window_keep_below_enabled, row++);

	pref->widget = table;
	pref->w_opacity = w_opacity;

	return pref;
}

void ui_pref_dialog_run(struct ui_psensor *ui)
{
	GtkWidget *diag, *content_area;
	gint result;
	struct ui_pref *pref;
	struct config *cfg;

	cfg = ui->config;

	diag = gtk_dialog_new_with_buttons(_("Edit Preferences"),
					   GTK_WINDOW(ui->main_window),
					   GTK_DIALOG_MODAL |
					   GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_STOCK_OK,
					   GTK_RESPONSE_ACCEPT,
					   GTK_STOCK_CANCEL,
					   GTK_RESPONSE_REJECT, NULL);

	content_area = gtk_dialog_get_content_area(GTK_DIALOG(diag));

	pref = ui_pref_create_main_widget(ui);

	gtk_container_add(GTK_CONTAINER(content_area), pref->widget);
	gtk_widget_show_all(content_area);

	result = gtk_dialog_run(GTK_DIALOG(diag));

	if (result == GTK_RESPONSE_ACCEPT) {
		double value;
		GdkColor color;

		gtk_color_button_get_color
		    (GTK_COLOR_BUTTON(pref->w_graph_fgcolor), &color);
		color_set(cfg->graph_fgcolor,
			  color.red, color.green, color.blue);

		gtk_color_button_get_color
		    (GTK_COLOR_BUTTON(pref->w_graph_bgcolor), &color);
		color_set(cfg->graph_bgcolor,
			  color.red, color.green, color.blue);

		value = gtk_range_get_value(GTK_RANGE(pref->w_opacity));
		cfg->graph_bg_alpha = value;

		if (value == 1.0)
			cfg->alpha_channel_enabled = 0;
		else
			cfg->alpha_channel_enabled = 1;

		cfg->sensorlist_position
		    = gtk_combo_box_get_active
		    (GTK_COMBO_BOX(pref->w_sensorlist_position));

		cfg->window_decoration_enabled = !gtk_toggle_button_get_active
		    GTK_TOGGLE_BUTTON(pref->w_window_decoration_enabled);

		cfg->window_keep_below_enabled
		    = gtk_toggle_button_get_active
		    (GTK_TOGGLE_BUTTON(pref->w_window_keep_below_enabled));

		gtk_window_set_decorated(GTK_WINDOW(ui->main_window),
					 cfg->window_decoration_enabled);

		gtk_window_set_keep_below(GTK_WINDOW(ui->main_window),
					  cfg->window_keep_below_enabled);

		cfg->sensor_update_interval
		    = gtk_spin_button_get_value_as_int
		    (GTK_SPIN_BUTTON(pref->w_sensor_update_interval));

		cfg->graph_update_interval
		    = gtk_spin_button_get_value_as_int
		    (GTK_SPIN_BUTTON(pref->w_graph_update_interval));

		cfg->graph_monitoring_duration
		    = gtk_spin_button_get_value_as_int
		    (GTK_SPIN_BUTTON(pref->w_graph_monitoring_duration));

		cfg->sensor_values_max_length
		    =
		    (cfg->graph_monitoring_duration * 60) /
		    cfg->sensor_update_interval;

		ui_main_box_create(ui);

		config_save(cfg);
	}

	gtk_widget_destroy(diag);

	free(pref);
}
