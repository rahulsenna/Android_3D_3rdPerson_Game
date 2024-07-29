#version 300 es
precision mediump float;

out vec4 color;

in vec3 vertexPosition;

void main() {
 float c = mod(
        floor(vertexPosition.x * 5.0) +
        floor(vertexPosition.z * 5.0),
        2.0
    );

	color = vec4(vec3(c/2.0 + 0.3), 1);
}
