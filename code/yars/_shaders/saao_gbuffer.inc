#ifndef SAAO_GBUFFER_INC
#define SAAO_GBUFFER_INC

//////////////////////////////////////////////////////////////////

uniform	sampler2D diffuseSampler;
uniform	sampler2D normalSampler;
uniform	sampler2D specularSampler;

uniform	sampler2D depthSampler;

struct saao_gBufData {
	// fetched
	vec3  diffuse;
	
	vec3  specular;
	float shininess;
	
	vec3  normal;
	float area;
	
	// computed
	float rawDepth;
	float depth;
	vec3  posCS;
};

//////////////////////////////////////////////////////////////////
void gBufFetch(const in vec2 coords, out saao_gBufData data) {
	vec4  ddata = texture(diffuseSampler, coords);
	vec4  sdata = texture(specularSampler, coords);
	vec4  ndata = texture(normalSampler, coords);

	float rawdepth = texture2D(depthSampler, coords).x;

	vec3 scrSpace;
	scrSpace.xy   = coords * 2.0 - 1.0;
	scrSpace.z    = rawdepth * 2.0 - 1.0;

	vec4 camSpace = invProjMatrix * vec4(scrSpace, 1.0);
	camSpace.xyz /= camSpace.w;
		
	data.diffuse = ddata.rgb;
	data.specular = sdata.rgb;
	data.shininess = sdata.a;

	data.normal = ndata.xyz;
	data.area   = ndata.a;

	data.rawDepth = rawdepth;
	data.depth  = scrSpace.z; 
	data.posCS  = camSpace.xyz;
}

//////////////////////////////////////////////////////////////////
void gBufFetchDiffuse(const in vec2 coords, inout saao_gBufData data) {
	vec4  ddata = texture(diffuseSampler, coords);
	data.diffuse = ddata.rgb;
}

void gBufFetchSpecular(const in vec2 coords, inout saao_gBufData data) {
	vec4  sdata = texture(specularSampler, coords);
	data.specular  = sdata.rgb;
	data.shininess = sdata.a;
}

void gBufFetchNormalArea(const in vec2 coords, inout saao_gBufData data) {
	vec4  ndata = texture(normalSampler, coords);
	data.normal = ndata.rgb;
	data.area   = ndata.a;
}

void gBufFetchDepth(const in vec2 coords, inout saao_gBufData data) {	
	data.rawDepth = texture2D(depthSampler, coords).x;
	data.depth    = data.rawDepth * 2.0 - 1.0;
}


// precondition: depth filled
/*void gBufCalcPosCS(const in vec2 coords, inout saao_gBufData data) {
	vec3 scrSpace = vec3(coords * 2.0 - 1.0, data.depth);	
	vec4 camSpace = invProjMatrix * vec4(scrSpace, 1.0);
	data.posCS = camSpace.xyz / camSpace.w;
}

// precondition: depth filled
void gBufCalcPosCS(const in vec2 coords, inout ssao_gBufData data) {
	vec3 scrSpace;
	scrSpace.xy   = coords * 2.0 - 1.0;
	scrSpace.z    = data.depth * 2.0 - 1.0;
	
	vec4 camSpace = invProjMatrix * vec4(scrSpace, 1.0);
	data.posCS = camSpace.xyz / camSpace.w;
}
*/

//////////////////////////////////////////////////////////////////

/// GBUFFER data access/unpacking /////////////////////
void gBufGet(const in vec2 coords, out vec3 diffuse, out vec3 specular, out float shininess, out float diskArea, out vec3 normalCS) {
	vec4 diffuseTex    = texture(diffuseSampler, coords);
	vec4 specularTex   = texture(specularSampler, coords);
	vec4 normalTex     = texture(normalSampler, coords);

	diffuse   = diffuseTex.rgb;
	
	specular  = specularTex.rgb;
	shininess = specularTex.a;
	
	normalCS  = normalTex.rgb;
	diskArea  = normalTex.a;	
}

vec3 gBufGetDiffuseColor(const in vec2 coords) {
	return texture(diffuseSampler, coords).rgb;
}

void gBufGetSpecularColor(const in vec2 coords, out vec3 specColor, out float shininess) {
	vec4 data = texture(specularSampler, coords);
	specColor = data.rgb;
	shininess = data.a;
}

vec3 gBufGetSpecularColorX(const in vec2 coords) {
	return texture(specularSampler, coords).rgb;
}

float gBufGetSpecularShininess(const in vec2 coords) {
	return texture(specularSampler, coords).a;
}


vec3 gBufGetNormal(const in vec2 coords) {
	return texture(normalSampler, coords).xyz;
}

float gBufGetArea(const in vec2 coords) {
	return texture(diffuseSampler, coords).a;
	//return 1.0;
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
	//return texture2D(depthSampler, coords).x;
	return texture2D(depthSampler, coords).x * 2.0 - 1.0;	
}

//////////////////////////////////////////////////////////////////

#endif
