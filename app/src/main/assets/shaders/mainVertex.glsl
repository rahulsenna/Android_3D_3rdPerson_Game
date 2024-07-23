#version 300 es
in vec3 inPosition;
in vec2 inUV;

out vec2 fragUV;

uniform mat4 uProjection;
uniform mat4 transform;

void main() {
    fragUV = inUV;

    gl_Position = uProjection * transform*vec4(inPosition, 1.0);
}