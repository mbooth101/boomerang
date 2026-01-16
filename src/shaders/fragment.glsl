#version 300 es
precision mediump float;

in vec2 textureCoord;
out vec4 fragColor;

uniform sampler2D screenshotTexture;
uniform vec2 resolution;
uniform vec2 pointer;

/* signed distance function for the flashlight vignette */
float flashlight(in vec2 point, in float radius)
{
  return length(point) - radius;
}

void main()
{
  vec2 pointerInvertY = vec2(pointer.x, resolution.y - pointer.y);

  vec2 c = (2.0 * gl_FragCoord.xy - resolution) / resolution.y;
  vec2 p = (2.0 * pointerInvertY - resolution) / resolution.y;

  vec3 col = texture(screenshotTexture, textureCoord).xyz;

  /* dim everything outside the flashlight area */
  float d = flashlight(c - p, 0.5);
  col = (d < 0.0) ? col : col * 0.5;

  fragColor = vec4(col, 1.0);
}