/*
 * Copyright (C) 2017-2018 jeanfi@gmail.com
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
#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <bool.h>
#include <pio.h>
#include <bcm2835.h>

/*
 * Support of the BCM2835 chip which is mostly used by the
 * Raspberry PI3 and provides the temperature of the chip.
 */
static const char *PROVIDER_NAME = "BCM2835";

/*
 * If this file exists and contains bcm2835 it can be assumed
 * that a BCM2835 chip is present.
 */
static const char *SYS_RPI_DETECTION_FILE =
"/sys/devices/virtual/misc/hw_random/rng_available";
static const char *SYS_RPI_DETECTION_FILE_EXPECTED_CONTENT = "bcm2835";
/* this file contains the temperature of the chip in celcius * 1000 */
static char *SYS_THERMAL_TEMP = "/sys/class/thermal/thermal_zone0/temp";

static void log_provider_info(const char *str)
{
	log_info("%s: %s", PROVIDER_NAME, str);
}

static bool is_bcm2835_present(void)
{
	bool ret;
	char *str;

	ret = is_file(SYS_RPI_DETECTION_FILE);

	if (!ret) {
		log_debug("%s: %s does not exist.",
			  PROVIDER_NAME,
			  SYS_RPI_DETECTION_FILE);
		return false;
	}

	str = file_get_content(SYS_RPI_DETECTION_FILE);

	if (!str
	    || strncmp(str,
		       SYS_RPI_DETECTION_FILE_EXPECTED_CONTENT,
		       strlen(SYS_RPI_DETECTION_FILE_EXPECTED_CONTENT) - 1)) {
		log_debug("%s: type: %s.", PROVIDER_NAME, str);
		ret = false;
	} else {
		ret = true;
	}

	if (str)
		free(str);

	return ret;
}

void bcm2835_psensor_list_append(struct psensor ***sensors, int vl)
{
	struct psensor *p;

	log_fct_enter();

	if (is_bcm2835_present()) {
		log_provider_info(_("The BCM2835 (probably a Raspberry PI3) "
				    "has been detected"));

		p = psensor_create(strdup(PROVIDER_NAME),
				   strdup(PROVIDER_NAME),
				   strdup(PROVIDER_NAME),
				   SENSOR_TYPE_BCM2835 | SENSOR_TYPE_TEMP,
				   vl);

		psensor_list_append(sensors, p);
	} else {
		log_provider_info("The BCM2835 has not been detected.");
	}

	log_fct_exit();
}

static double bcm2835_update_temp(struct psensor *s)
{
	char *str, *end;
	long l;
	int n;

	log_fct_enter();

	str = file_get_content(SYS_THERMAL_TEMP);

	n = strlen(str) - 1;
	if (str[n] == '\n')
		str[n] = '\0';

	if (str) {
		l = strtol(str, &end, 10);
		if (*end != '\0')
			log_debug("%s: found invalid value: %s.",
				  PROVIDER_NAME,
				  str);
		else
			psensor_set_current_value(s, l / 1000);
		free(str);
	} else {
		log_err(_("Failed to get content of file %s."),
			SYS_THERMAL_TEMP);
	}

	log_fct_exit();

	return 0;
}

void bcm2835_psensor_list_update(struct psensor **sensors)
{
	struct psensor *s;

	log_fct_enter();

	for (; *sensors; sensors++) {
		s = *sensors;

		if (s->type & SENSOR_TYPE_REMOTE)
			continue;

		if (s->type & SENSOR_TYPE_BCM2835) {
			bcm2835_update_temp(s);
			break; /* only one possible sensor */
		}
	}

	log_fct_exit();
}
