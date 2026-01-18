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

#define FL_RADIUS_INCREMENT 0.05

struct _BoomerangCanvas
{
  GtkGLArea parent_instance;

  char *filename;

  int scale_factor;

  bool flashlight_zoom;

  GLuint program;
  GLuint texture;
  GLuint vao;
  GLuint vbo;

  /* shader uniforms */
  GLfloat projection[16];
  GLfloat resolution[2];
  GLfloat pointer[2];
  GLint flashlight_enabled;
  GLfloat flashlight_radius;
};

G_DEFINE_FINAL_TYPE (BoomerangCanvas, boomerang_canvas, GTK_TYPE_GL_AREA)

/* two triangles that cover the entire viewport, given here as (x,y,u,v) tuples */
static const GLfloat geometry[] = {
  // clang-format off
  -1.0f, -1.0f, 0.0f, 1.0f, /* left bottom */
  -1.0f,  1.0f, 0.0f, 0.0f, /* left top */
   1.0f,  1.0f, 1.0f, 0.0f, /* right top */
  -1.0f, -1.0f, 0.0f, 1.0f, /* left bottom */
   1.0f,  1.0f, 1.0f, 0.0f, /* right top */
   1.0f, -1.0f, 1.0f, 1.0f, /* right bottom */
  // clang-format on
};

static GLuint
create_shader (GLenum shader_type, const char *src, GError **error)
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
init_rendering (GtkWidget *widget, GError **error)
{
  BoomerangCanvas *canvas = BOOMERANG_CANVAS (widget);

  gtk_gl_area_make_current (GTK_GL_AREA (widget));

  glClearColor (0, 0, 0, 1.0);
  glFrontFace (GL_CW);
  glCullFace (GL_BACK);
  glEnable (GL_CULL_FACE);

  canvas->flashlight_zoom = false;
  canvas->flashlight_enabled = 0;
  canvas->flashlight_radius = 0.3;

  /* initialise shader program */

  GBytes *vertex_source = g_resources_lookup_data (
      "/shaders/vertex.glsl", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
  GBytes *fragment_source = g_resources_lookup_data (
      "/shaders/fragment.glsl", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

  canvas->program = create_program (
      g_bytes_get_data (vertex_source, NULL),
      g_bytes_get_data (fragment_source, NULL), error);
  if (!canvas->program)
    return;

  g_bytes_unref (vertex_source);
  g_bytes_unref (fragment_source);

  /* initialise texture */

  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (canvas->filename, error);
  if (!pixbuf)
    return;

  int width = gdk_pixbuf_get_width (pixbuf);
  int height = gdk_pixbuf_get_height (pixbuf);
  int channels = gdk_pixbuf_get_n_channels (pixbuf);
  int format = (channels == 4 ? GL_RGBA : GL_RGB);

  glGenTextures (1, &canvas->texture);
  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_2D, canvas->texture);
  glTexImage2D (GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, gdk_pixbuf_get_pixels (pixbuf));
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  GLint screenshot_texture_loc = glGetUniformLocation (canvas->program, "screenshotTexture");
  glUniform1i (screenshot_texture_loc, canvas->texture);

  g_object_unref (pixbuf);

  /* initialise geometry buffers */

  glGenVertexArrays (1, &canvas->vao);
  glBindVertexArray (canvas->vao);

  glGenBuffers (1, &canvas->vbo);
  glBindBuffer (GL_ARRAY_BUFFER, canvas->vbo);
  glBufferData (GL_ARRAY_BUFFER, sizeof (geometry), geometry, GL_STATIC_DRAW);

  GLint poscoord = glGetAttribLocation (canvas->program, "posCoord");
  GLint texcoord = glGetAttribLocation (canvas->program, "texCoord");
  glBindVertexArray (canvas->vao);
  glVertexAttribPointer (poscoord, 2, GL_FLOAT, false, 4 * sizeof (GLfloat), (void *) 0);
  glVertexAttribPointer (texcoord, 2, GL_FLOAT, false, 4 * sizeof (GLfloat), (void *) 8);
  glEnableVertexAttribArray (poscoord);
  glEnableVertexAttribArray (texcoord);
}

static void
canvas_zoom (BoomerangCanvas *canvas, int direction)
{
  if (canvas->flashlight_zoom)
    {
      canvas->flashlight_radius += FL_RADIUS_INCREMENT * direction;
      if (canvas->flashlight_radius <= FL_RADIUS_INCREMENT)
        canvas->flashlight_radius = FL_RADIUS_INCREMENT;
    }
}

static void
canvas_motion (GtkEventControllerMotion *controller, gdouble x, gdouble y, gpointer data)
{
  BoomerangCanvas *canvas = BOOMERANG_CANVAS (data);

  /* use the scale factor to convert from widget coordinates to frame buffer coordinates */
  canvas->pointer[0] = x * canvas->scale_factor;
  canvas->pointer[1] = y * canvas->scale_factor;

  gtk_gl_area_queue_render (GTK_GL_AREA (data));
}

static gboolean
canvas_scroll (GtkEventControllerScroll *controller, gdouble dx, gdouble dy, gpointer data)
{
  BoomerangCanvas *canvas = BOOMERANG_CANVAS (data);

  canvas_zoom (canvas, -dy);

  gtk_gl_area_queue_render (GTK_GL_AREA (data));
  return TRUE;
}

static void
canvas_key_pressed (GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer data)
{
  BoomerangCanvas *canvas = BOOMERANG_CANVAS (data);

  /* holding ctrl zooms the flashlight area instead of the screenshot */
  if (keyval == GDK_KEY_Control_L || keyval == GDK_KEY_Control_R)
    canvas->flashlight_zoom = true;

  if (keyval == GDK_KEY_f)
    canvas->flashlight_enabled = canvas->flashlight_enabled ? 0 : 1;

  if (keyval == GDK_KEY_equal)
    canvas_zoom (canvas, 1);
  if (keyval == GDK_KEY_minus)
    canvas_zoom (canvas, -1);

  gtk_gl_area_queue_render (GTK_GL_AREA (data));
}

static void
canvas_key_released (GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer data)
{
  BoomerangCanvas *canvas = BOOMERANG_CANVAS (data);

  if (keyval == GDK_KEY_Control_L || keyval == GDK_KEY_Control_R)
    canvas->flashlight_zoom = false;
}

static void
canvas_realize (GtkWidget *widget)
{
  GError *error = NULL;
  init_rendering (widget, &error);
  if (error)
    {
      gtk_gl_area_set_error (GTK_GL_AREA (widget), error);
      g_error_free (error);
    }

  GtkEventController *motion_controller = gtk_event_controller_motion_new ();
  gtk_widget_add_controller (widget, motion_controller);
  g_signal_connect (motion_controller, "enter", G_CALLBACK (canvas_motion), widget);
  g_signal_connect (motion_controller, "motion", G_CALLBACK (canvas_motion), widget);

  GtkEventController *scroll_controller = gtk_event_controller_scroll_new (GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
  gtk_widget_add_controller (widget, scroll_controller);
  g_signal_connect (scroll_controller, "scroll", G_CALLBACK (canvas_scroll), widget);

  GtkEventController *key_controller = gtk_event_controller_key_new ();
  gtk_widget_add_controller (widget, key_controller);
  g_signal_connect (key_controller, "key-pressed", G_CALLBACK (canvas_key_pressed), widget);
  g_signal_connect (key_controller, "key-released", G_CALLBACK (canvas_key_released), widget);
}

static void
canvas_unrealize (GtkWidget *widget)
{
  BoomerangCanvas *canvas = BOOMERANG_CANVAS (widget);

  gtk_gl_area_make_current (GTK_GL_AREA (widget));

  if (canvas->vbo)
    glDeleteBuffers (1, &canvas->vbo);
  if (canvas->vao)
    glDeleteVertexArrays (1, &canvas->vao);
  if (canvas->texture)
    glDeleteTextures (1, &canvas->texture);
  if (canvas->program)
    glDeleteProgram (canvas->program);
}

static void
canvas_resize (GtkGLArea *widget, int width, int height)
{
  BoomerangCanvas *canvas = BOOMERANG_CANVAS (widget);

  canvas->resolution[0] = width;
  canvas->resolution[1] = height;

  /* this the same scale factor used by the underlying gl area widget to determine the size of the frame buffers,
   * and therefore the dimensions passed into this function */
  canvas->scale_factor = gtk_widget_get_scale_factor (GTK_WIDGET (widget));

  glViewport (0, 0, (GLint) width, (GLint) height);

  /* compute an orthographic projection */
  GLfloat left = -1.0f;
  GLfloat right = 1.0f;
  GLfloat bottom = -1.0f;
  GLfloat top = 1.0f;
  GLfloat near = -1.0f;
  GLfloat far = 1.0f;
  canvas->projection[0] = 2.0f / (right - left);
  canvas->projection[1] = 0.0f;
  canvas->projection[2] = 0.0f;
  canvas->projection[3] = 0.0f;
  canvas->projection[4] = 0.0f;
  canvas->projection[5] = 2.0f / (top - bottom);
  canvas->projection[6] = 0.0f;
  canvas->projection[7] = 0.0f;
  canvas->projection[8] = 0.0f;
  canvas->projection[9] = 0.0f;
  canvas->projection[10] = -2.0f / (far - near);
  canvas->projection[11] = 0.0f;
  canvas->projection[12] = -(right + left) / (right - left);
  canvas->projection[13] = -(top + bottom) / (top - bottom);
  canvas->projection[14] = -(far + near) / (far - near);
  canvas->projection[15] = 1.0f;
}

static gboolean
canvas_render (GtkGLArea *widget, GdkGLContext *context)
{
  BoomerangCanvas *canvas = BOOMERANG_CANVAS (widget);

  glClear (GL_COLOR_BUFFER_BIT);

  glUseProgram (canvas->program);
  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_2D, canvas->texture);
  glBindVertexArray (canvas->vao);

  GLint projection_loc = glGetUniformLocation (canvas->program, "projection");
  glUniformMatrix4fv (projection_loc, 1, GL_FALSE, &canvas->projection[0]);

  GLint resolution_loc = glGetUniformLocation (canvas->program, "resolution");
  glUniform2f (resolution_loc, canvas->resolution[0], canvas->resolution[1]);

  GLint pointer_loc = glGetUniformLocation (canvas->program, "pointer");
  glUniform2f (pointer_loc, canvas->pointer[0], canvas->pointer[1]);

  GLint fenabled_loc = glGetUniformLocation (canvas->program, "fenabled");
  glUniform1i (fenabled_loc, canvas->flashlight_enabled);

  GLint fradius_loc = glGetUniformLocation (canvas->program, "fradius");
  glUniform1f (fradius_loc, canvas->flashlight_radius);

  glDrawArrays (GL_TRIANGLES, 0, 6);

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
boomerang_canvas_init (BoomerangCanvas *canvas)
{
  g_signal_connect (canvas, "realize", G_CALLBACK (canvas_realize), NULL);
  g_signal_connect (canvas, "unrealize", G_CALLBACK (canvas_unrealize), NULL);
}

void
boomerang_canvas_set_filename (BoomerangCanvas *canvas, const char *filename)
{
  g_return_if_fail (BOOMERANG_IS_CANVAS (canvas));

  canvas->filename = g_strdup (filename);
}

