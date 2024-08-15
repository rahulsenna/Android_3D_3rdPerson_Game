#version 320 es
precision mediump float;

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in mat4 instanceMatrix;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 VP;
flat out int instanceID;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  VP * instanceMatrix * vec4(vertexPosition_modelspace,1);
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
	instanceID = gl_InstanceID;

}

