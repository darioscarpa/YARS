#version 420

#include ubo_matrices.inc
#include ubo_lights.inc

/////////////////////////////////////////////////////
uniform int pointLightId;
/////////////////////////////////////////////////////
// input

in VertexData {
	//vec3 singlepointLightDirCS;
	//vec3 pointLightDirCS;

	//vec3 pointLightsDirCS[POINT_LIGHTS_MAX];

	vec3 normalCS;
	vec3 tangentCS;
	vec3 bitangentCS;
	
	vec2 TexCoord;

	vec4 eyeCS;
} vertexIn;

/////////////////////////////////////////////////////
// output

layout(location = 0) out vec4 outputcol;

void main() {	
	outputcol.rgb = pointLights[pointLightId].color;	
	outputcol.a   = 1.0;
}
