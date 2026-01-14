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

#include "boomerang-canvas.h"

struct _BoomerangCanvas
{
  GtkGLArea parent_instance;
};

G_DEFINE_FINAL_TYPE (BoomerangCanvas, boomerang_canvas, GTK_TYPE_GL_AREA)

static void
boomerang_canvas_class_init (BoomerangCanvasClass *klass)
{
}

static void
boomerang_canvas_init (BoomerangCanvas *self)
{
}

