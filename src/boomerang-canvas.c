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

#include <epoxy/gl.h>

struct _BoomerangCanvas
{
  GtkGLArea parent_instance;

  GLuint program;
  GLuint vao;
  GLuint vbo;

  GLfloat projection[16];
};

G_DEFINE_FINAL_TYPE (BoomerangCanvas, boomerang_canvas, GTK_TYPE_GL_AREA)

/* two triangles that cover the entire viewport, given here in vec2s */
static const GLfloat geometry[] = {
  // clang-format off
  -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
  -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
  // clang-format on
};

static GLuint
create_shader (int shader_type, const char *src, GError **error)
{
  GLuint shader = glCreateShader (shader_type);
  glShaderSource (shader, 1, &src, NULL);
  glCompileShader (shader);

  GLint compile_status;
  glGetShaderiv (shader, GL_COMPILE_STATUS, &compile_status);
  if (compile_status == GL_FALSE)
    {
      GLint log_len;
      glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &log_len);

      char *buffer = g_malloc (log_len + 1);
      glGetShaderInfoLog (shader, log_len, NULL, buffer);
      g_set_error (error, GDK_GL_ERROR, GDK_GL_ERROR_COMPILATION_FAILED,
                   "Compilation error in %s shader:\n%s",
                   shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment", buffer);
      g_free (buffer);

      glDeleteShader (shader);
      return 0;
    }

  return shader;
}

static GLuint
create_program (const char *vertex_src, const char *fragment_src, GError **error)
{
  GLuint vertex = create_shader (GL_VERTEX_SHADER, vertex_src, error);
  if (!vertex)
    {
      return 0;
    }

  GLuint fragment = create_shader (GL_FRAGMENT_SHADER, fragment_src, error);
  if (!fragment)
    {
      glDeleteShader (vertex);
      return 0;
    }

  GLuint program = glCreateProgram ();
  glAttachShader (program, vertex);
  glAttachShader (program, fragment);
  glLinkProgram (program);

  GLint link_status;
  glGetProgramiv (program, GL_LINK_STATUS, &link_status);
  if (link_status == GL_FALSE)
    {
      GLint log_len;
      glGetProgramiv (program, GL_INFO_LOG_LENGTH, &log_len);

      char *buffer = g_malloc (log_len + 1);
      glGetProgramInfoLog (program, log_len, NULL, buffer);
      g_set_error (error, GDK_GL_ERROR, GDK_GL_ERROR_LINK_FAILED,
                   "Linkage error in shader program:\n%s", buffer);
      g_free (buffer);

      glDeleteShader (vertex);
      glDeleteShader (fragment);
      glDeleteProgram (program);
      return 0;
    }

  glDeleteShader (vertex);
  glDeleteShader (fragment);
  return program;
}

static void
canvas_realize (GtkWidget *widget)
{
  BoomerangCanvas *self = BOOMERANG_CANVAS (widget);

  gtk_gl_area_make_current (GTK_GL_AREA (widget));

  glClearColor (0, 0, 0, 1.0);
  glFrontFace (GL_CW);
  glCullFace (GL_BACK);
  glEnable (GL_CULL_FACE);

  /* initialise shader program */

  GBytes *vertex_source = g_resources_lookup_data (
      "/shaders/vertex.glsl", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
  GBytes *fragment_source = g_resources_lookup_data (
      "/shaders/fragment.glsl", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

  GError *error = NULL;
  self->program = create_program (
      g_bytes_get_data (vertex_source, NULL),
      g_bytes_get_data (fragment_source, NULL), &error);
  if (!self->program)
    {
      gtk_gl_area_set_error (GTK_GL_AREA (widget), error);
      g_error_free (error);
    }

  g_bytes_unref (vertex_source);
  g_bytes_unref (fragment_source);

  /* initialise geometry buffers */

  glGenVertexArrays (1, &self->vao);
  glBindVertexArray (self->vao);

  glGenBuffers (1, &self->vbo);
  glBindBuffer (GL_ARRAY_BUFFER, self->vbo);
  glBufferData (GL_ARRAY_BUFFER, sizeof (geometry), geometry, GL_STATIC_DRAW);

  GLint vertex = glGetAttribLocation (self->program, "vertex");
  glBindVertexArray (self->vao);
  glVertexAttribPointer (vertex, 2, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray (vertex);
}

static void
canvas_unrealize (GtkWidget *widget)
{
  BoomerangCanvas *self = BOOMERANG_CANVAS (widget);

  gtk_gl_area_make_current (GTK_GL_AREA (widget));

  if (self->vbo)
    glDeleteBuffers (1, &self->vbo);
  if (self->vao)
    glDeleteVertexArrays (1, &self->vao);
  if (self->program)
    glDeleteProgram (self->program);
}

static void
canvas_resize (GtkGLArea *widget, int width, int height)
{
  BoomerangCanvas *self = BOOMERANG_CANVAS (widget);

  glViewport (0, 0, (GLint) width, (GLint) height);

  /* compute an orthographic projection */
  GLfloat left = -1.0f;
  GLfloat right = 1.0f;
  GLfloat bottom = -1.0f;
  GLfloat top = 1.0f;
  GLfloat near = -1.0f;
  GLfloat far = 1.0f;
  self->projection[0] = 2.0f / (right - left);
  self->projection[1] = 0.0f;
  self->projection[2] = 0.0f;
  self->projection[3] = 0.0f;
  self->projection[4] = 0.0f;
  self->projection[5] = 2.0f / (top - bottom);
  self->projection[6] = 0.0f;
  self->projection[7] = 0.0f;
  self->projection[8] = 0.0f;
  self->projection[9] = 0.0f;
  self->projection[10] = -2.0f / (far - near);
  self->projection[11] = 0.0f;
  self->projection[12] = -(right + left) / (right - left);
  self->projection[13] = -(top + bottom) / (top - bottom);
  self->projection[14] = -(far + near) / (far - near);
  self->projection[15] = 1.0f;
}

static gboolean
canvas_render (GtkGLArea *widget, GdkGLContext *context)
{
  BoomerangCanvas *self = BOOMERANG_CANVAS (widget);

  glClear (GL_COLOR_BUFFER_BIT);

  glUseProgram (self->program);
  glBindVertexArray (self->vao);

  GLint projection_loc = glGetUniformLocation (self->program, "projection");
  glUniformMatrix4fv (projection_loc, 1, GL_FALSE, &self->projection[0]);

  // TODO update fragment uniforms

  glDrawArrays (GL_TRIANGLES, 0, 6);

  glBindVertexArray (0);
  glUseProgram (0);

  glFlush ();

  return TRUE;
}

static void
boomerang_canvas_class_init (BoomerangCanvasClass *klass)
{
  GtkGLAreaClass *glarea_class = GTK_GL_AREA_CLASS (klass);
  glarea_class->resize = canvas_resize;
  glarea_class->render = canvas_render;
}

static void
boomerang_canvas_init (BoomerangCanvas *self)
{
  g_signal_connect (self, "realize", G_CALLBACK (canvas_realize), NULL);
  g_signal_connect (self, "unrealize", G_CALLBACK (canvas_unrealize), NULL);
}

