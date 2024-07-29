#version 300 es
precision mediump float;
in vec2 P;               // position attr from the vbo

uniform mat4 projection; // projection matrix
uniform mat4 view;       // modelview matrix

out vec3 vertexPosition; // vertex position for the fragment shader

void main() {
	vertexPosition = vec3(P.x,0,P.y);
	gl_Position = projection * view * vec4(P.x,0,P.y, 1);
}