#ifndef INDIRECTLIGHTING_INC
#define INDIRECTLIGHTING_INC

//////////////////////////////////////////////////////////////////

#include formulas.inc

#include sampling.inc

#include subroutinesConfig.inc

#ifdef USE_SUBROUTINE_ILCALCULATOR

subroutine vec3    ilCalculator(const in vec3 emitterDirectDiffuseLighting, const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea,
						        const in vec3 receiverPosition, in vec3 receiverNormal, const in float receiverArea); 
subroutine uniform ilCalculator diskToDiskRadiance;

#endif

////////////////////////////// ////////////////////////////////

#ifdef USE_SUBROUTINE_ILCALCULATOR
subroutine (ilCalculator) 
#endif
vec3 d2dr_bunnell(const in vec3 emitterDirectDiffuseLighting, const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea,
						  const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea) {
	// V is the vector from receiver disk to emitter disk
	vec3 V = emitterPosition - receiverPosition;
	float d2 = dot(V, V) + 1e-16;
	V *= inversesqrt(d2);

	float formF = bunnell_calcFormFactor(V, d2, receiverNormal, emitterNormal, emitterArea);	
	return emitterDirectDiffuseLighting*formF;
}

#ifdef USE_SUBROUTINE_ILCALCULATOR
subroutine (ilCalculator) 
#endif
vec3 d2dr_dssao(const in vec3 emitterDirectDiffuseLighting, const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea,
						  const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea) {
	
	// V is the vector from receiver disk to emitter disk
	vec3 V = emitterPosition - receiverPosition;
	float dist = length(V);
	if (dist > distMax) return vec3(0.0f);

	float formF = formFactor(V, dist, receiverNormal, emitterNormal, emitterArea);
	return emitterDirectDiffuseLighting*formF;
}

#ifdef USE_SUBROUTINE_ILCALCULATOR
subroutine (ilCalculator) 
#endif
vec3 d2dr_dssao2(in vec3 emitterDirectDiffuseLighting, const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea,
						 const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea) {
	
	// V is the vector from receiver disk to emitter disk
	vec3 V = emitterPosition - receiverPosition;
	float dist = length(V);
	if (dist > distMax) return vec3(0.0f);

	float minarea = min(emitterArea, receiverArea);

	float formF = formFactor(V, dist, receiverNormal, emitterNormal, minarea);
	return emitterDirectDiffuseLighting*formF;	
}

#ifdef USE_SUBROUTINE_ILCALCULATOR
subroutine (ilCalculator) 
#endif
vec3 xd2dr_test(in vec3 emitterDirectDiffuseLighting, const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea,
						  const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea) {
	
	vec3 V = emitterPosition - receiverPosition;
	float d2 = dot(V, V) + 1e-16;
	V *= inversesqrt(d2);

	float formF = bunnell_calcFormFactor(V, d2, receiverNormal, emitterNormal, emitterArea);	
	return emitterDirectDiffuseLighting*formF/emitterArea;
}


#ifdef USE_SUBROUTINE_ILCALCULATOR
subroutine (ilCalculator) 
#endif
vec3 d2dr_test(const in vec3 emitterDirectDiffuseLighting, const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea,
						 const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea) {
	
	// V is the vector from receiver disk to emitter disk
	vec3 V = emitterPosition - receiverPosition;
	float dist = length(V);
	if (dist > distMax) return vec3(0.0f);

	float formF = diskToPointFormFactor(V, receiverNormal, emitterNormal, emitterArea);

	//float formF = dssoFormFactor(V, dist, receiverNormal, emitterNormal, emitterArea);

	return emitterDirectDiffuseLighting*formF / emitterArea;
}

//////////////////////////////////////////////////////////////////

#ifndef USE_SUBROUTINE_ILCALCULATOR

uniform int diskToDiskRadianceSelector;

vec3 diskToDiskRadiance(const in vec3 emitterDirectDiffuseLighting, const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea,
						  const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea) {
	//return d2dr_bunnell(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
	//return d2dr_dssao(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
	//return d2dr_dssao2(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
	//return d2dr_dssao_noArea(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
	switch(diskToDiskRadianceSelector) {
		case 0: return d2dr_bunnell(emitterDirectDiffuseLighting, emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
		//case 0: return d2dr_bunnell(emitterDirectDiffuseLighting, receiverPosition, receiverNormal, receiverArea, emitterPosition, emitterNormal, emitterArea);
		case 1: return d2dr_dssao(emitterDirectDiffuseLighting, emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
		//case 2: return d2dr_dssao_noArea(emitterDirectDiffuseLighting, emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
		case 2: return d2dr_test(emitterDirectDiffuseLighting, emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
	}
}

#endif


//////////////////////////////////////////////////////////////////
#endif
