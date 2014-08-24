#ifndef GDLBUFFER_INC
#define GDLBUFFER_INC

//////////////////////////////////////////////////////////////////

uniform	sampler2D directLightTexData;
uniform	sampler2D normalSampler;
uniform	sampler2D depthSampler;
uniform	sampler2D albedoSampler;

//////////////////////////////////////////////////////////////////

/// GDLBUFFER data access/unpacking /////////////////////

struct gdlbufData {
	vec3  dl;
	vec3  albedo;
	vec3  normal;
	float area;
	float depth;
	vec3  posCS;
};

void gdlbufGet(const in vec2 uv, out gdlbufData data) {
	vec4  dldata = texture(directLightTexData, uv);
	vec4  ndata  = texture(normalSampler, uv);
	float depth  = texture2D(depthSampler, uv).x * 2.0 - 1.0;	

	vec3 scrSpace;
	scrSpace.xy   = uv * 2.0 - 1.0;
	scrSpace.z    = depth;

	vec4 camSpace = invProjMatrix * vec4(scrSpace, 1.0);
	camSpace.xyz /= camSpace.w;
	
	data.dl     = dldata.rgb;
	data.albedo = texture(albedoSampler, uv).rgb;
	data.normal = ndata.xyz;
	data.area   = dldata.a;
	data.depth  = depth; 
	data.posCS  = camSpace.xyz;
}

vec3 gdlbufGetAlbedo(const in vec2 uv) {
	return texture(albedoSampler, uv).rgb;
}

vec3 gdlbufGetNormal(const in vec2 uv) {
	return texture(normalSampler, uv).xyz;
}

vec3 gdlbufGetDirectLight(const in vec2 coords) {
	return texture(directLightTexData, coords).rgb;
}

float gdlbufGetArea(const in vec2 uv) {
	return texture(directLightTexData, uv).a;
	//return 1.0;
}

vec3 gdlbufGetPosition(const in vec2 uv) {		
	vec3 scrSpace;
	scrSpace.xy   = uv * 2.0 - 1.0;
	scrSpace.z    = texture2D(depthSampler, uv).x * 2.0 - 1.0;
	vec4 camSpace = invProjMatrix * vec4(scrSpace, 1.0);
	camSpace.xyz /= camSpace.w;
	return camSpace.xyz;
}

float gdlbufGetDepth(const in vec2 uv) {
	return texture2D(depthSampler, uv).x * 2.0 - 1.0;	
}

float gdlbufGetRawDepth(const in vec2 uv) {
	return texture2D(depthSampler, uv).x;
}

//////////////////////////////////////////////////////////////////
#endif
