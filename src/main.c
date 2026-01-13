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

#include <glib.h>
#include <stdlib.h>

#include "boomerang-screenshot.h"

static GMainLoop *loop;

static void
callback (GObject *source, GAsyncResult *result, gpointer data)
{
  GError *error = NULL;
  char *screenshot_uri = boomerang_screenshot_finish (BOOMERANG_SCREENSHOT (source), result, &error);

  if (screenshot_uri)
    {
      g_print ("Screenshot URI: %s\n", screenshot_uri);
      char *filename = g_filename_from_uri (screenshot_uri, NULL, NULL);
      if (filename)
        {
          g_print ("Screenshot saved to: %s\n", filename);
          g_free (filename);
        }
      else
        {
          g_print ("Error unable to parse URI: %s\n", screenshot_uri);
        }
      g_free (screenshot_uri);
    }
  else
    {
      g_print ("Error: %s\n", error->message);
    }

  g_main_loop_quit (loop);
}

int
main (int argc, char **argv)
{
  loop = g_main_loop_new (NULL, FALSE);

  BoomerangScreenshot *screenshot = g_object_new (BOOMERANG_TYPE_SCREENSHOT, NULL);
  boomerang_screenshot_take (screenshot, NULL, callback, NULL);

  g_main_loop_run (loop);

  g_main_loop_unref (loop);

  return EXIT_SUCCESS;
}

