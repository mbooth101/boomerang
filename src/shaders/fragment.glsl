#version 300 es
precision mediump float;

in vec2 textureCoord;
out vec4 fragColor;

uniform sampler2D screenshotTexture;
uniform vec2 resolution;
uniform vec2 pointer;
uniform bool fenabled;
uniform float fradius;

/* debug output */
uniform bool debugging;
uniform float fradiusStart;
uniform float fradiusTarget;

void main()
{
  vec2 pointerInvertY = vec2(pointer.x, resolution.y - pointer.y);

  /* fragment coord (c) and mouse pointer (p) as normalised device coordinates */
  float divisor = min(resolution.x, resolution.y);
  vec2 c = (2.0 * gl_FragCoord.xy - resolution) / divisor;
  vec2 p = (2.0 * pointerInvertY - resolution) / divisor;

  /* generate a blending factor gradient along the edge of the flashlight area, to
   * give an anti-aliased appearance to the edge of the vignette */
  float dist = distance(c, p);
  float delta = fwidth(dist) * 2.5;
  float alpha = smoothstep(fradius - delta, fradius + delta, dist);

  /* blend only when the flashlight is enabled, and clamp the factor to something less
   * than one so the vignette is always slightly transparent */
  float blend = fenabled ? min(alpha, 0.6) : 0.0;

  vec4 screenshot = texture(screenshotTexture, textureCoord);
  vec4 vignette = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 col = mix(screenshot, vignette, blend);

  vec4 startIndicator = vec4(0.0, 1.0, 0.0, 1.0);
  vec4 targetIndicator = vec4(1.0, 0.0, 0.0, 1.0);
  float start = float(debugging && length(c - p) >= fradiusStart - 0.0005 && length(c - p) <= fradiusStart + 0.0005);
  float target = float(debugging && length(c - p) >= fradiusTarget - 0.0005 && length(c - p) <= fradiusTarget + 0.0005);

  col = mix(col, targetIndicator, target);
  col = mix(col, startIndicator, start);
  fragColor = col;
}