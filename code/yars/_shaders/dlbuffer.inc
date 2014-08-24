#ifndef DLBUFFER_INC
#define DLBUFFER_INC

//////////////////////////////////////////////////////////////////

uniform	sampler2D scatteredSampler;
uniform	sampler2D reflectedSampler;

/// DLBUFFER data access/unpacking /////////////////////

vec3 dlbufGetScatteredLight(const in vec2 coords) {
	return texture(scatteredSampler, coords).rgb;
}

vec3 dlbufGetReflectedLight(const in vec2 coords) {
	return texture(reflectedSampler, coords).rgb;
}

//////////////////////////////////////////////////////////////////
#endif
