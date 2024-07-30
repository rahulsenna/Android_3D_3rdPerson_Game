#version 300 es
layout (location = 0) in vec2 a_position;
#ifdef GL_FRAGMENT_PRECISION_HIGH
  precision highp float;
#else
  precision mediump float;
#endif

layout(location = 0) out vec4 o_color;
out vec3 near;
out vec3 far;

uniform mat4 view;
uniform mat4 projection;

vec3 unproject_point(float x, float y, float z) {
    mat4 inv = inverse(projection * view);
    vec4 unproj_point = inv * vec4(x, y, z, 1.f);
    return unproj_point.xyz / unproj_point.w;
}

void main() {
    vec2 p = a_position;
    near = unproject_point(p.x, p.y, -1.f);
    far  = unproject_point(p.x, p.y,  1.f);
    gl_Position = vec4(a_position, 0.0f, 1.0f);
}