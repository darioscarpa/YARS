#version 420

#include ubo_matrices.inc

/////////////////////////////////////////////////////
// input
/*
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
*/

in vec3 position;
in vec3 normal;
in vec2 texCoord;
in vec3 tangent;
in vec3 bitangent;

/////////////////////////////////////////////////////
// output

out VertexData {
	vec4 posCS;
	vec2 texCoord;

	vec3 normalCS;
	vec3 tangentCS;
	vec3 bitangentCS;

	vec4 eyeCS;
} vertexOut;

/////////////////////////////////////////////////////

/////////////

uniform bool normalMappingEnabled;

/////////////
 
void main() {
	vertexOut.texCoord = texCoord;
	
	vec4 pos = vec4(position,1.0);
	gl_Position = mvpMatrix * pos;				
	
	// for lightning in camera space
	vertexOut.normalCS    = (modelViewMatrix * vec4(normal, 0.0)).xyz;
	vertexOut.tangentCS   = (modelViewMatrix * vec4(tangent, 0.0)).xyz;
	vertexOut.bitangentCS = (modelViewMatrix * vec4(bitangent, 0.0)).xyz;

	// calc eye and point light direction in CS
	vertexOut.posCS = modelViewMatrix * pos;
	vertexOut.eyeCS = -vertexOut.posCS;	// 'cause eyeposCS (0,0,0), so eyeposCS - posCS...
}