#version 300 es
precision mediump float;

in vec2 textureCoord;
out vec4 fragColor;

uniform sampler2D screenshotTexture;
uniform vec2 resolution;
uniform vec2 pointer;
uniform bool fenabled;

/* signed distance function for the flashlight vignette */
float flashlight(in vec2 point, in float radius)
{
  return length(point) - radius;
}

void main()
{
  vec2 pointerInvertY = vec2(pointer.x, resolution.y - pointer.y);

  float divisor = min(resolution.x, resolution.y);
  vec2 c = (2.0 * gl_FragCoord.xy - resolution) / divisor;
  vec2 p = (2.0 * pointerInvertY - resolution) / divisor;

  vec3 col = texture(screenshotTexture, textureCoord).xyz;

  /* dim everything outside the flashlight area */
  float d = (fenabled ? flashlight(c - p, 0.5) : 0.0);
  col = (d <= 0.0) ? col : col * 0.5;

  fragColor = vec4(col, 1.0);
}