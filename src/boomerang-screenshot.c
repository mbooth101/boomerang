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

#include "boomerang-screenshot.h"

#define PORTAL_BUS "org.freedesktop.portal.Desktop"

struct _BoomerangScreenshot
{
  GObject parent_instance;

  GTask *task;
  GDBusConnection *conn;

  char *object_path;
  guint signal_id;
  gulong cancelled_id;
};

G_DEFINE_FINAL_TYPE (BoomerangScreenshot, boomerang_screenshot, G_TYPE_OBJECT)

static void
boomerang_screenshot_class_init (BoomerangScreenshotClass *klass)
{
}

static void
boomerang_screenshot_init (BoomerangScreenshot *self)
{
}

static void
screenshot_cleanup (BoomerangScreenshot *bs)
{
  g_clear_signal_handler (&bs->cancelled_id, g_task_get_cancellable (bs->task));
  g_object_unref (bs->task);

  if (bs->signal_id)
    g_dbus_connection_signal_unsubscribe (bs->conn, bs->signal_id);

  g_free (bs->object_path);

  g_object_unref (bs->conn);
}

static void
screenshot_take_response_cb (GDBusConnection *bus, const char *sender_name, const char *object_path, const char *interface_name, const char *signal_name, GVariant *parameters, gpointer data)
{
  BoomerangScreenshot *bs = BOOMERANG_SCREENSHOT (data);

  unsigned int response;
  g_autoptr (GVariant) results = NULL;
  g_variant_get (parameters, "(u@a{sv})", &response, &results);

  if (response == 0)
    {
      const char *uri = NULL;
      if (g_variant_lookup (results, "uri", "&s", &uri))
        g_task_return_pointer (bs->task, g_strdup (uri), g_free);
      else
        g_task_return_new_error (bs->task, G_IO_ERROR, G_IO_ERROR_FAILED, "Unable to retrieve URI to screenshot");
    }
  else if (response == 1)
    {
      g_task_return_new_error (bs->task, G_IO_ERROR, G_IO_ERROR_CANCELLED, "Screenshot taking was cancelled");
    }
  else
    {
      g_task_return_new_error (bs->task, G_IO_ERROR, G_IO_ERROR_FAILED, "Failed to take screenshot");
    }

  screenshot_cleanup (bs);
}

static void
screenshot_cancelled_cb (GCancellable *cancellable, gpointer data)
{
  BoomerangScreenshot *bs = BOOMERANG_SCREENSHOT (data);

  /* cancel the in-progress screenshot request, a response signal will not be sent */
  g_dbus_connection_call (bs->conn, PORTAL_BUS,
                          bs->object_path,
                          "org.freedesktop.portal.Request",
                          "Close", NULL, NULL,
                          G_DBUS_CALL_FLAGS_NONE, -1,
                          NULL, NULL, NULL);

  g_task_return_new_error (bs->task, G_IO_ERROR, G_IO_ERROR_CANCELLED, "Screenshot taking was cancelled");

  screenshot_cleanup (bs);
}

static void
screenshot_take_cb (GObject *object, GAsyncResult *result, gpointer data)
{
  BoomerangScreenshot *bs = BOOMERANG_SCREENSHOT (data);

  /* portal methods return the object path for the request whose response signal we need to observe for the results of
   * the call, but we discard it here because we computed it earlier and are already subscribed */

  GError *error = NULL;
  g_autoptr (GVariant) ret_val = g_dbus_connection_call_finish (G_DBUS_CONNECTION (object), result, &error);

  if (error)
    {
      g_task_return_error (bs->task, error);
      screenshot_cleanup (bs);
    }
}

static void
screenshot_take (BoomerangScreenshot *self)
{
  /* compute object path on which to listen for the request response signal */
  g_autofree char *token = g_strdup_printf ("boomerang_%d", g_random_int_range (0, G_MAXINT));
  g_autofree char *sender = g_strdup (g_dbus_connection_get_unique_name (self->conn) + 1);
  for (int i = 0; sender[i]; i++)
    if (sender[i] == '.')
      sender[i] = '_';
  self->object_path = g_strconcat ("/org/freedesktop/portal/desktop/request/", sender, "/", token, NULL);

  /* subscribe to the request response signal on the object path computed above */
  self->signal_id = g_dbus_connection_signal_subscribe (self->conn, PORTAL_BUS,
                                                        "org.freedesktop.portal.Request",
                                                        "Response",
                                                        self->object_path,
                                                        NULL,
                                                        G_DBUS_SIGNAL_FLAGS_NO_MATCH_RULE,
                                                        screenshot_take_response_cb,
                                                        self,
                                                        NULL);

  /* register cancellation callback */
  GCancellable *cancellable = g_task_get_cancellable (self->task);
  if (cancellable)
    self->cancelled_id = g_signal_connect (cancellable, "cancelled", G_CALLBACK (screenshot_cancelled_cb), self);

  /* create input parameters for the screenshot portal request */
  GVariantBuilder *builder = g_variant_builder_new (G_VARIANT_TYPE ("a{sv}"));
  g_variant_builder_add (builder, "{sv}", "interactive", g_variant_new_boolean (FALSE));
  g_variant_builder_add (builder, "{sv}", "handle_token", g_variant_new_string (token));
  GVariant *params = g_variant_new ("(sa{sv})", "", builder);
  g_variant_builder_unref (builder);

  /* https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.Screenshot.html */
  g_dbus_connection_call (self->conn, PORTAL_BUS,
                          "/org/freedesktop/portal/desktop",
                          "org.freedesktop.portal.Screenshot",
                          "Screenshot", params, G_VARIANT_TYPE ("(o)"),
                          G_DBUS_CALL_FLAGS_NONE, -1,
                          NULL, screenshot_take_cb, self);
}

void
boomerang_screenshot_take (BoomerangScreenshot *self, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data)
{
  g_return_if_fail (BOOMERANG_IS_SCREENSHOT (self));

  self->task = g_task_new (self, cancellable, callback, data);

  // TODO pass error and check for error
  self->conn = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);

  screenshot_take (self);
}

char *
boomerang_screenshot_finish (BoomerangScreenshot *self, GAsyncResult *result, GError **error)
{
  g_return_val_if_fail (BOOMERANG_IS_SCREENSHOT (self), NULL);
  g_return_val_if_fail (g_task_is_valid (result, self), NULL);

  return g_task_propagate_pointer (G_TASK (result), error);
}

