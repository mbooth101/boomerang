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

#ifndef BOOMERANG_APPLICATION_H_
#define BOOMERANG_APPLICATION_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BOOMERANG_TYPE_APPLICATION (boomerang_application_get_type ())

G_DECLARE_FINAL_TYPE (BoomerangApplication, boomerang_application, BOOMERANG, APPLICATION, GtkApplication)

int boomerang_application_get_status (BoomerangApplication *self);

G_END_DECLS

#endif /* BOOMERANG_APPLICATION_H_ */

