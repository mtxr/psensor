/*
 * Copyright (C) 2017 jeanfi@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

/*
 * Retrieve temperature from the chip BCM2835 which is mostly
 * used by the Raspberry PI3 and not support by lmsensor.
 */
#ifndef _PSENSOR_BCM2835_H_
#define _PSENSOR_BCM2835_H_

#include <bool.h>
#include <psensor.h>

static inline bool bcm2835_is_supported(void) { return true; }

void bcm2835_psensor_list_update(struct psensor **);
void bcm2835_psensor_list_append(struct psensor ***, int);
void bcm2835_cleanup(void);

#endif
