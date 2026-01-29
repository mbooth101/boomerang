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
application_screenshot_cb (GObject *source, GAsyncResult *result, gpointer data)
{
  BoomerangApplication *self = BOOMERANG_APPLICATION (data);

  GError *error = NULL;
  char *screenshot_uri = boomerang_screenshot_finish (BOOMERANG_SCREENSHOT (source), result, &error);

  char *filename = NULL;
  if (screenshot_uri)
    {
      filename = g_filename_from_uri (screenshot_uri, NULL, NULL);
      if (!filename)
        {
          g_printerr ("Error: Unable to parse URI %s\n", screenshot_uri);
          self->status = 1;
        }
      g_free (screenshot_uri);
    }
  else
    {
      g_printerr ("Error: %s\n", error->message);
      self->status = 1;
    }

  if (filename)
    {
      g_print ("Screenshot saved to: %s\n", filename);

      self->window = gtk_application_window_new (GTK_APPLICATION (self));
      gtk_window_set_title (GTK_WINDOW (self->window), APPLICATION_NAME);
      gtk_window_fullscreen (GTK_WINDOW (self->window));

      self->canvas = g_object_new (BOOMERANG_TYPE_CANVAS, NULL);
      gtk_widget_set_focusable (self->canvas, TRUE);
      gtk_widget_set_hexpand (self->canvas, TRUE);
      gtk_widget_set_vexpand (self->canvas, TRUE);
      gtk_window_set_child (GTK_WINDOW (self->window), self->canvas);
      boomerang_canvas_set_filename (BOOMERANG_CANVAS (self->canvas), filename);

      gtk_window_present (GTK_WINDOW (self->window));

      g_free (filename);
    }

  /* releasing now that the window is shown, if we didn't present a window due to something going wrong with the
   * screenshotting process, then the application will exit */
  g_application_release (G_APPLICATION (self));
}

static void
boomerang_application_activate (GApplication *app)
{
  BoomerangApplication *self = BOOMERANG_APPLICATION (app);

  /* window won't exist until after the screenshot is taken */
  if (self->window)
    {
      gtk_window_present (GTK_WINDOW (self->window));
    }
}

static void
boomerang_application_startup (GApplication *app)
{
  G_APPLICATION_CLASS (boomerang_application_parent_class)->startup (app);

  BoomerangApplication *self = BOOMERANG_APPLICATION (app);

  /* hold the application until we hear back from the screenshot service to avoid showing the window without
   * anything to render */
  g_application_hold (G_APPLICATION (self));

  self->screenshot = g_object_new (BOOMERANG_TYPE_SCREENSHOT, NULL);
  boomerang_screenshot_take (self->screenshot, NULL, application_screenshot_cb, self);
}

static void
boomerang_application_class_init (BoomerangApplicationClass *klass)
{
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);
  app_class->activate = boomerang_application_activate;
  app_class->startup = boomerang_application_startup;
}

static void
boomerang_application_init (BoomerangApplication *self)
{
  self->status = 0;

  g_action_map_add_action_entries (G_ACTION_MAP (self), app_actions, G_N_ELEMENTS (app_actions), self);
  gtk_application_set_accels_for_action (GTK_APPLICATION (self), "app.quit", (const char *[]) { "<Control>q", "Escape", NULL });
}

int
boomerang_application_get_status (BoomerangApplication *self)
{
  return self->status;
}

