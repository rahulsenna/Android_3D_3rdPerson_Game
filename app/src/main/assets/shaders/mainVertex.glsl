#version 300 es
in vec3 inPosition;
in vec2 inUV;

out vec2 fragUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    fragUV = inUV;

    gl_Position = uProjection * uView * uModel * vec4(inPosition, 1.0);
}