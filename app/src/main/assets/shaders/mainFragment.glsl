#version 300 es
precision mediump float;

in vec2 fragUV;

//uniform sampler2D uTexture;

out vec4 outColor;

void main() {
    //outColor = texture(uTexture, fragUV);
    outColor = vec4(1,1,0,1);

}