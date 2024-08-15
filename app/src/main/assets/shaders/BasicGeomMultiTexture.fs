#version 320 es
precision mediump float;

// Interpolated values from the vertex shaders
in vec2 UV;
flat in int instanceID;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler[15];

void main(){
    int textureIndex = instanceID % 15; // Cycle through textures

	// Output color = color of the texture at the specified UV
	color = texture(myTextureSampler[textureIndex], UV).rgb;
}
