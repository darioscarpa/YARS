#version 420

#include ubo_matrices.inc
#include ubo_lights.inc


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
	//vec3 singlepointLightDirCS;
	//vec3 pointLightDirCS;

	//vec3 pointLightsDirCS[POINT_LIGHTS_MAX];
	//vec3 spotLightsDirCS[SPOT_LIGHTS_MAX];

	vec3 normalCS;
	vec3 tangentCS;
	vec3 bitangentCS;
	
	vec2 TexCoord;

	vec4 eyeCS;
} vertexOut;

/////////////////////////////////////////////////////

void main() {
	vertexOut.TexCoord = vec2(texCoord);
	
	//vec4 posCS  = modelViewMatrix * pos;

	//vertexOut.normalCS    = (normalMatrix * vec4(normal, 0.0)).xyz;
	//vertexOut.tangentCS   = (normalMatrix * vec4(tangent, 0.0)).xyz;
	//vertexOut.bitangentCS = (normalMatrix * vec4(bitangent, 0.0)).xyz;
	
	vec4 pos    = vec4(position, 1.0);
	gl_Position = mvpMatrix * pos;
		
		
	
	// for lightning in camera space
	vertexOut.normalCS    = (modelViewMatrix * vec4(normal, 0.0)).xyz;
	vertexOut.tangentCS   = (modelViewMatrix * vec4(tangent, 0.0)).xyz;
	vertexOut.bitangentCS = (modelViewMatrix * vec4(bitangent, 0.0)).xyz;

	// calc eye and point light direction in CS
	vec4 posCS  = modelViewMatrix * pos;

	
	vertexOut.eyeCS = -posCS;	// 'cause eyeposCS (0,0,0), so eyeposCS - posCS...

	/*
	for (int i = 0 ; i < POINT_LIGHTS_MAX; i++) {
		//pointLights[i].dirCS = vec3(vec4(pointLight[i].posCS, 1.0) - posCS);
		//pointLightsDirCS[i] = vec3(vec4(pointLights[i].posCS, 1.0) - posCS);
		vertexOut.pointLightsDirCS[i] = pointLights[i].posCS - vec3(posCS);
	}

	for (int i = 0 ; i < SPOT_LIGHTS_MAX; i++) {
		vertexOut.spotLightsDirCS[i] = spotLights[i].posCS - vec3(posCS);
	}
	*/
}