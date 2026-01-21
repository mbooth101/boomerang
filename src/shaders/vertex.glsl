#version 300 es

in vec2 posCoord;
in vec2 texCoord;
out vec2 textureCoord;

uniform mat4 projection;
uniform float zoomLevel;

void main()
{
  textureCoord = texCoord;

  mat4 transform = mat4(1.0);
  transform[0][0] = zoomLevel;
  transform[1][1] = zoomLevel;

  gl_Position = projection * transform * vec4(posCoord, 0.0, 1.0);
}