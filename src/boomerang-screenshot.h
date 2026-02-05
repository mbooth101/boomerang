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

#ifndef BOOMERANG_SCREENSHOT_H_
#define BOOMERANG_SCREENSHOT_H_

#include <gio/gio.h>

G_BEGIN_DECLS

#define BOOMERANG_TYPE_SCREENSHOT (boomerang_screenshot_get_type ())

G_DECLARE_FINAL_TYPE (BoomerangScreenshot, boomerang_screenshot, BOOMERANG, SCREENSHOT, GObject)

void boomerang_screenshot_take (BoomerangScreenshot *screenshot, GCancellable *cancellable,
                                GAsyncReadyCallback callback, gpointer data);

char *boomerang_screenshot_finish (BoomerangScreenshot *screenshot, GAsyncResult *result, GError **error);

G_END_DECLS

#endif /* BOOMERANG_SCREENSHOT_H_ */

