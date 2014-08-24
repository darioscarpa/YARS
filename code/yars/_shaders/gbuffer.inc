#ifndef GBUFFER_INC
#define GBUFFER_INC

//////////////////////////////////////////////////////////////////

uniform	sampler2D diffuseSampler;
uniform	sampler2D normalSampler;
uniform	sampler2D specularSampler;

uniform	sampler2D depthSampler;

/// GBUFFER data access/unpacking /////////////////////
void gbufGet(const in vec2 coords, out vec3 diffuse, out vec3 specular, out float shininess, out float diskArea, out vec3 normalCS) {
	vec4 diffuseTex    = texture(diffuseSampler, coords);
	vec4 specularTex   = texture(specularSampler, coords);
	vec4 normalTex     = texture(normalSampler, coords);

	diffuse   = diffuseTex.rgb;
	specular  = specularTex.rgb;
	shininess = specularTex.a;
	diskArea  = diffuseTex.a;
	normalCS  = normalTex.rgb;
}

vec3 gbufGetDiffuseColor(in vec2 coords) {
	return texture(diffuseSampler, coords).rgb;
}

void gbufGetSpecularColor(const in vec2 coords, out vec3 specColor, out float shininess) {
	vec4 data = texture(specularSampler, coords);
	specColor = data.rgb;
	shininess = data.a;
}

vec3 gbufGetSpecularColorX(const in vec2 coords) {
	return texture(specularSampler, coords).rgb;
}

float gbufGetSpecularShininess(const in vec2 coords) {
	return texture(specularSampler, coords).a;
}


vec3 gbufGetNormal(const in vec2 uv) {
	return texture(normalSampler, uv).xyz;
}

float gbufGetArea(const in vec2 uv) {
	return texture(diffuseSampler, uv).a;
	//return 1.0;
}

vec3 gbufGetPosition(const in vec2 uv) {		
	vec3 scrSpace;
	scrSpace.xy   = uv * 2.0 - 1.0;
	scrSpace.z    = texture2D(depthSampler, uv).x * 2.0 - 1.0;
	vec4 camSpace = invProjMatrix * vec4(scrSpace, 1.0);
	camSpace.xyz /= camSpace.w;
	return camSpace.xyz;
}

float gbufGetDepth(const in vec2 uv) {
	//return texture2D(depthSampler, uv).x;
	return texture2D(depthSampler, uv).x * 2.0 - 1.0;	
}

//////////////////////////////////////////////////////////////////
#endif
