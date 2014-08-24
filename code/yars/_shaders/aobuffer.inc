#ifndef AOBUFFER_INC
#define AOBUFFER_INC

//////////////////////////////////////////////////////////////////

uniform	sampler2D aoSampler;

/// AOBUFFER data access/unpacking /////////////////////

float aobufGetOcclusion(const in vec2 coords) {
	return texture(aoSampler, coords).a;
}

vec3 aobufGetIndirectLight(const in vec2 coords) {
	return texture(aoSampler, coords).rgb;
}

vec4 aobufGet(const in vec2 coords) {
	return texture(aoSampler, coords);
}

//////////////////////////////////////////////////////////////////
#endif
