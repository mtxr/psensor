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
#include "ui_pref.h"
#include "ui_color.h"
#include "ui_sensorpref.h"

GtkWidget *ui_sensorpref_add_entry(GtkWidget * table,
				   const char *label,
				   const char *text, guint row)
{
	GtkWidget *alig, *entry;

	ui_pref_add_label(table, label, 0, row);

	alig = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alig), 4, 4, 4, 4);
	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), text);

	gtk_container_add(GTK_CONTAINER(alig), entry);

	gtk_table_attach(GTK_TABLE(table),
			 alig,
			 1, 2,
			 row, row + 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);

	return entry;

}

struct ui_sensorpref *ui_sensorpref_create(struct ui_sensorlist *ui_sensorlist,
					   struct psensor *sensor)
{
	struct ui_sensorpref *pref;
	const char *stype;
	GtkSpinButton *spin_button;
	GtkWidget *alig, *table, *w_alarm_enabled;

	pref = malloc(sizeof(struct ui_sensorpref));

	table = gtk_table_new(10, 2, FALSE);
	gtk_table_set_col_spacing(GTK_TABLE(table), 0, 0);

	/* Section Sensor Information */
	ui_pref_add_section_title(table, _("Sensor Information"), 0);

	ui_pref_add_label(table, _("Id:"), 0, 1);
	ui_pref_add_label(table, sensor->id, 1, 1);

	stype = psensor_type_to_str(sensor->type);
	ui_pref_add_label(table, _("Type:"), 0, 2);
	ui_pref_add_label(table, stype, 1, 2);

	pref->w_name = ui_sensorpref_add_entry(table,
					       _("Name:"), sensor->name, 3);

	/* Section graph */
	ui_pref_add_section_title(table, _("Graph"), 4);

	pref->w_enabled = ui_pref_add_check_button
		(table, _("Draw sensor curve"), sensor->enabled, 5);

	pref->w_color = ui_pref_add_color_button
		(table, _("Color:"), sensor->color, 6);

	/* Section Alarm */
	ui_pref_add_section_title(table, _("Alarm"), 7);

	w_alarm_enabled = ui_pref_add_check_button
		(table,
		 _("Activate desktop notifications"), sensor->alarm_enabled,
		 8);
	if (!is_temp_type(sensor->type))
		gtk_widget_set_sensitive(w_alarm_enabled, FALSE);

	ui_pref_add_label(table, _("Temperature limit:"), 0, 9);

	spin_button
	    = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 255, 1));
	if (!is_temp_type(sensor->type))
		gtk_widget_set_sensitive(GTK_WIDGET(spin_button), FALSE);
	gtk_spin_button_set_value(spin_button, sensor->alarm_limit);
	alig = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alig), 4, 4, 4, 4);
	gtk_container_add(GTK_CONTAINER(alig), GTK_WIDGET(spin_button));
	gtk_table_attach(GTK_TABLE(table),
			 alig,
			 1, 2,
			 9, 10,
			 GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);

	pref->widget = table;
	pref->w_alarm_limit = GTK_WIDGET(spin_button);
	pref->w_alarm_enabled = w_alarm_enabled;
	pref->ui_sensorlist = ui_sensorlist;

	return pref;
}
