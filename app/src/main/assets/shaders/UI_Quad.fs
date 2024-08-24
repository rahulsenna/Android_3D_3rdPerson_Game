#version 320 es
precision mediump float;

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D uTextureUI;
uniform float uOpacityUI;

void main()
{
    vec4 Color = texture(uTextureUI, TexCoords);
	FragColor = vec4(Color.rgb, Color.a * uOpacityUI);
}