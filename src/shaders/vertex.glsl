#version 300 es

in vec2 posCoord;
in vec2 texCoord;
out vec2 textureCoord;

uniform mat4 projection;

void main()
{
  textureCoord = texCoord;

  gl_Position = projection * vec4(posCoord, 0.0, 1.0);
}