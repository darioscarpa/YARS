#ifndef SAAOIL_ILAOBUFFER_INC
#define SAAOIL_ILAOBUFFER_INC

//////////////////////////////////////////////////////////////////

uniform	sampler2D ilaoSampler;

/// ILAOBUFFER data access/unpacking /////////////////////

float ilaoBufGetOcclusion(const in vec2 coords) {
	return texture(ilaoSampler, coords).a;
}

vec3 ilaoBufGetIndirectLight(const in vec2 coords) {
	return texture(ilaoSampler, coords).rgb;
}

vec4 ilaoBufGet(const in vec2 coords) {
	return texture(ilaoSampler, coords);
}

//////////////////////////////////////////////////////////////////
void ilaoBufFetchIndirectLight(const in vec2 coords, inout vec4 ilao) {
	ilao = texture(ilaoSampler, coords);
}
//////////////////////////////////////////////////////////////////
#endif
