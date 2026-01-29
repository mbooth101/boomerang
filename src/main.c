/* Copyright 2026 Mat Booth
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "config.h"

#include "boomerang-application.h"

int
main (int argc, char **argv)
{
  g_autoptr (BoomerangApplication) app = g_object_new (BOOMERANG_TYPE_APPLICATION,
                                                       "application-id", APPLICATION_ID,
                                                       "flags", G_APPLICATION_DEFAULT_FLAGS,
                                                       NULL);
  char version[80];
  g_snprintf (version, sizeof (version), "%s", PACKAGE_VERSION);
  g_application_set_version (G_APPLICATION (app), version);

  int status = g_application_run (G_APPLICATION (app), argc, argv);
  if (boomerang_application_get_status (app) != 0)
    {
      status = boomerang_application_get_status (app);
    }
  return status;
}

