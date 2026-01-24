#version 300 es
precision mediump float;

in vec2 posCoord;
in vec2 texCoord;
out vec2 textureCoord;

uniform mat4 projection;
uniform vec2 resolution;
uniform vec2 dragPosition;
uniform float zoomLevel;

void main()
{
  textureCoord = texCoord;

  /* add half the resolution so that 0,0 drag ends up at the origin in the
   * centre of the screen before converting to normalised device coords */
  vec2 dragPos = dragPosition + (resolution / 2.0);
  dragPos = (2.0 * dragPos - resolution) / resolution;

  mat4 transform = mat4(1.0);
  transform[0][0] = zoomLevel;
  transform[1][1] = zoomLevel;
  transform[3][0] = dragPos.x;
  transform[3][1] = dragPos.y;

  gl_Position = projection * transform * vec4(posCoord, 0.0, 1.0);
}