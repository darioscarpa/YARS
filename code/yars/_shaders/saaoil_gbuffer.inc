#ifndef SAAOIL_GBUFFER_INC
#define SAAOIL_GBUFFER_INC

//////////////////////////////////////////////////////////////////

uniform	sampler2D directLightTexData;
uniform	sampler2D normalSampler;
uniform	sampler2D depthSampler;
uniform	sampler2D albedoSampler;

//////////////////////////////////////////////////////////////////

struct saaoil_gBufData {
	// fetched
	vec3  dl;
	
	vec3  albedo;
	
	vec3  normal;
	float area;
	
	// computed
	float rawDepth;
	float depth;
	vec3  posCS;
};

void gBufFetch(const in vec2 coords, out saaoil_gBufData data) {
	vec4  dldata = texture(directLightTexData, coords);
	vec4  albedo = texture(albedoSampler, coords);
	vec4  ndata  = texture(normalSampler, coords);

	float rawdepth = texture2D(depthSampler, coords).x;

	vec3 scrSpace;
	scrSpace.xy   = coords * 2.0 - 1.0;
	scrSpace.z    = rawdepth * 2.0 - 1.0;

	vec4 camSpace = invProjMatrix * vec4(scrSpace, 1.0);
	camSpace.xyz /= camSpace.w;
		
	data.dl     = dldata.rgb;
	
	data.albedo = albedo.rgb;

	data.normal = ndata.xyz;
	data.area   = ndata.a;

	data.rawDepth = rawdepth;
	data.depth  = scrSpace.z; 
	data.posCS  = camSpace.xyz;
}

//////////////////////////////////////////////////////////////////
void gBufFetchDirectLight(const in vec2 coords, inout saaoil_gBufData data) {
	vec4  dldata = texture(directLightTexData, coords);
	data.dl = dldata.rgb;
}

void gBufFetchAlbedo(const in vec2 coords, inout saaoil_gBufData data) {
	vec4  albedo = texture(albedoSampler, coords);
	data.albedo = albedo.rgb;
}

void gBufFetchNormalArea(const in vec2 coords, inout saaoil_gBufData data) {
	vec4  ndata = texture(normalSampler, coords);
	data.normal = ndata.rgb;
	data.area   = ndata.a;
}

void gBufFetchDepth(const in vec2 coords, inout saaoil_gBufData data) {	
	data.rawDepth = texture2D(depthSampler, coords).x;
	data.depth    = data.rawDepth * 2.0 - 1.0;
}

///////////////////////////////////////////////////////////////

vec3 gBufGetAlbedo(const in vec2 coords) {
	return texture(albedoSampler, coords).rgb;
}

vec3 gBufGetNormal(const in vec2 coords) {
	return texture(normalSampler, coords).xyz;
}

vec3 gBufGetDirectLight(const in vec2 coords) {
	return texture(directLightTexData, coords).rgb;
}

float gBufGetArea(const in vec2 coords) {
	return texture(normalSampler, coords).a;	
}

vec3 gBufGetPosition(const in vec2 coords) {		
	vec3 scrSpace;
	scrSpace.xy   = coords * 2.0 - 1.0;
	scrSpace.z    = texture2D(depthSampler, coords).x * 2.0 - 1.0;
	vec4 camSpace = invProjMatrix * vec4(scrSpace, 1.0);
	camSpace.xyz /= camSpace.w;
	return camSpace.xyz;
}

float gBufGetDepth(const in vec2 coords) {
	return texture2D(depthSampler, coords).x * 2.0 - 1.0;	
}

float gBufGetRawDepth(const in vec2 coords) {
	return texture2D(depthSampler, coords).x;
}

//////////////////////////////////////////////////////////////////
#endif
