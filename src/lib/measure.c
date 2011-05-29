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

#include "measure.h"

struct measure *measures_create(int size)
{
	return calloc(size, sizeof(struct measure));
}

void measures_free(struct measure *measures)
{
	free(measures);
}

void measure_set_value(struct measure *m, double value)
{
	if (gettimeofday(&(m->time), NULL) == 0) {
		m->value = value;
	} else {
		m->value = 0;
		timerclear(&m->time);
	}
}

void measure_copy(struct measure *src, struct measure *dst)
{
	dst->time = src->time;
	dst->value = src->value;
}
