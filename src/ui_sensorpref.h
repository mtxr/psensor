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

#ifndef _PSENSOR_UI_SENSORPREF_H_
#define _PSENSOR_UI_SENSORPREF_H_

#include "color.h"
#include "psensor.h"
#include "ui_sensorlist.h"

#include <gtk/gtk.h>

struct ui_sensorpref {
	/* Main container */
	GtkWidget *widget;

	GtkWidget *w_name;
	GtkWidget *w_enabled;
	GtkWidget *w_alarm_limit;
	GtkWidget *w_alarm_enabled;
	GtkWidget *w_color;

	/* Needed to change color/enabled in the list of sensors */
	struct ui_sensorlist *ui_sensorlist;
};

/*
  Create a gtk widget to edit preferences about a given sensor.

  The widget does not modify sensor struct.
*/
struct ui_sensorpref *ui_sensorpref_create(struct ui_sensorlist *,
					   struct psensor *sensor);

#endif
