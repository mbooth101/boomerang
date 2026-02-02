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
#include "boomerang-canvas.h"
#include "boomerang-screenshot.h"

struct _BoomerangApplication
{
  GtkApplication parent_instance;

  BoomerangScreenshot *screenshot;

  GtkWidget *window;
  GtkWidget *canvas;

  char *filename;

  int status;
};

G_DEFINE_FINAL_TYPE (BoomerangApplication, boomerang_application, GTK_TYPE_APPLICATION)

static void
application_quit_action (GSimpleAction *action, GVariant *parameter, gpointer data)
{
  BoomerangApplication *self = data;
  g_assert (BOOMERANG_IS_APPLICATION (self));
  g_application_quit (G_APPLICATION (self));
}

static const GActionEntry app_actions[] = {
  { "quit", application_quit_action },
};

static void
boomerang_application_create_canvas (BoomerangApplication *app)
{
  g_print ("Loading screenshot: %s\n", app->filename);

  app->window = gtk_application_window_new (GTK_APPLICATION (app));
  gtk_window_set_title (GTK_WINDOW (app->window), APPLICATION_NAME);
  gtk_window_fullscreen (GTK_WINDOW (app->window));

  app->canvas = g_object_new (BOOMERANG_TYPE_CANVAS, NULL);
  gtk_widget_set_focusable (app->canvas, TRUE);
  gtk_widget_set_hexpand (app->canvas, TRUE);
  gtk_widget_set_vexpand (app->canvas, TRUE);
  gtk_window_set_child (GTK_WINDOW (app->window), app->canvas);
  boomerang_canvas_set_filename (BOOMERANG_CANVAS (app->canvas), app->filename);

  gtk_window_present (GTK_WINDOW (app->window));
}

static void
boomerang_application_screenshot_cb (GObject *source, GAsyncResult *result, gpointer data)
{
  BoomerangApplication *app = BOOMERANG_APPLICATION (data);

  GError *error = NULL;
  char *screenshot_uri = boomerang_screenshot_finish (BOOMERANG_SCREENSHOT (source), result, &error);

  if (screenshot_uri)
    {
      app->filename = g_filename_from_uri (screenshot_uri, NULL, NULL);
      if (!app->filename)
        {
          g_printerr ("Error: Unable to parse URI %s\n", screenshot_uri);
          app->status = 1;
        }
      g_free (screenshot_uri);
    }
  else
    {
      g_printerr ("Error: %s\n", error->message);
      app->status = 1;
    }

  if (app->filename)
    boomerang_application_create_canvas (app);

  /* releasing now that the window is shown, if we didn't present a window due to something going wrong with the
   * screenshotting process, then the application will exit */
  g_application_release (G_APPLICATION (app));
}

static void
boomerang_application_activate (GApplication *application)
{
  BoomerangApplication *app = BOOMERANG_APPLICATION (application);

  if (app->window)
    {
      gtk_window_present (GTK_WINDOW (app->window));
      return;
    }

  /* if a filename was not passed on the command line, take a screenshot using the freedesktop portal service,
   * otherwise show the window immediately */
  if (!app->filename)
    {
      /* hold the application until we hear back from the screenshot service to avoid showing the window without
       * anything to render */
      g_application_hold (G_APPLICATION (app));
      app->screenshot = g_object_new (BOOMERANG_TYPE_SCREENSHOT, NULL);
      boomerang_screenshot_take (app->screenshot, NULL, boomerang_application_screenshot_cb, app);
    }
  else
    {
      boomerang_application_create_canvas (app);
    }
}

static void
boomerang_application_class_init (BoomerangApplicationClass *klass)
{
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);
  app_class->activate = boomerang_application_activate;
}

static void
boomerang_application_init (BoomerangApplication *app)
{
  app->status = 0;

  g_action_map_add_action_entries (G_ACTION_MAP (app), app_actions, G_N_ELEMENTS (app_actions), app);
  gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.quit", (const char *[]) { "<Control>q", "Escape", NULL });

  GOptionEntry app_options[] = {
    { "screenshot", 's', G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME, &app->filename, "Path to screenshot file", "FILENAME" },
    G_OPTION_ENTRY_NULL
  };
  g_application_add_main_option_entries (G_APPLICATION (app), app_options);
}

int
boomerang_application_get_status (BoomerangApplication *app)
{
  return app->status;
}

BoomerangApplication *
boomerang_application_new (void)
{
  BoomerangApplication *app = g_object_new (BOOMERANG_TYPE_APPLICATION,
                                            "application-id", APPLICATION_ID,
                                            "flags", G_APPLICATION_DEFAULT_FLAGS,
                                            NULL);

  char version[80];
  g_snprintf (version, sizeof (version), "%s", PACKAGE_VERSION);
  g_application_set_version (G_APPLICATION (app), version);

  return app;
}

