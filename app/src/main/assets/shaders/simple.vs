#version 300 es
in vec3 inPosition;

out vec2 fragUV;

uniform mat4 view;
uniform mat4 projection;

void main() {
    
    gl_Position = projection * view * vec4(inPosition, 1.0);
}