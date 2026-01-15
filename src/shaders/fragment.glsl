#version 300 es
precision mediump float;

in vec2 textureCoord;

out vec4 fragColor;

uniform sampler2D screenshotTexture;

void main() {
  fragColor = texture(screenshotTexture, textureCoord);
}