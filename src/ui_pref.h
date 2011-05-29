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

#ifndef _PSENSOR_UI_PREF_H_
#define _PSENSOR_UI_PREF_H_

#include "ui.h"

void ui_pref_add_section_title(GtkWidget *table, const char *title, guint row);

GtkWidget *ui_pref_add_color_button(GtkWidget *table,
				    const char *text,
				    struct color *color, guint row);

void ui_pref_add_label(GtkWidget *table,
		       const char *text, guint col, guint row);

GtkWidget *ui_pref_add_check_button(GtkWidget *table,
				    const char *text, int enabled, guint row);

void ui_pref_dialog_run(struct ui_psensor *);
void ui_pref_set_color_button(GtkWidget *btn, struct color *color);
GtkWidget *ui_pref_create_color_button(GdkColor * color);
GdkColor *color_to_gdkcolor(struct color *color);

#endif
