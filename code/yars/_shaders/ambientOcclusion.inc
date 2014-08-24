#ifndef AMBIENTOCCLUSION_INC
#define AMBIENTOCCLUSION_INC

//////////////////////////////////////////////////////////////////

#include formulas.inc

#include sampling.inc

#include subroutinesConfig.inc

#ifdef USE_SUBROUTINE_AOCALCULATOR

subroutine float   aoCalculator(const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea,
						        const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea) ;
subroutine uniform aoCalculator diskToDiskOcclusion;

#endif

//////////////////////////////////////////////////////////////////

#ifdef USE_SUBROUTINE_AOCALCULATOR
subroutine (aoCalculator) 
#endif
float d2do_bunnell(const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea,
				   const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea) {
	// V is the vector from receiver disk to emitter disk
	vec3 V = emitterPosition - receiverPosition;
	float d2 = dot(V, V) + 1e-16;
	V *= inversesqrt(d2);
	return bunnell_calcSolidAngle(V, d2, receiverNormal, emitterNormal, emitterArea/3.14);	
}

#ifdef USE_SUBROUTINE_AOCALCULATOR
subroutine (aoCalculator) 
#endif
float d2do_dssao(const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea,
				 const  in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea) {
	
	// V is the vector from receiver disk to emitter disk
	vec3 V = emitterPosition - receiverPosition;
	float dist = max(length(V), 1e-16);
	if (dist > distMax) return 0.0f;
	return formFactor(V, dist, receiverNormal, emitterNormal, emitterArea);
}

#ifdef USE_SUBROUTINE_AOCALCULATOR
subroutine (aoCalculator) 
#endif
float d2do_test(const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea,
				const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea) {

	/*
	vec3 V = receiverPosition - emitterPosition;
	float Vlen = length(V);
	float distSquared = Vlen*Vlen;
	V = normalize(V);
	emitterNormal = normalize(emitterNormal);

	return (emitterArea * clamp( dot(receiverNormal, V), 0.0, 1.0) ) / (PI * distSquared + emitterArea);
	*/
	//float coveredSolidAngle = clamp( dot( emitterNormal,  V ), 0.0, 1.0) * emitterArea / distSquared;

	//return coveredSolidAngle;

	float sa1 = solidAngle_simple(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal);
	float sa2 = solidAngle_simple(emitterPosition, -emitterNormal, emitterArea, receiverPosition, receiverNormal);
	//return max(sa1, sa2);
	return sa2;

	//return solidAngle_simple(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal);
}

//////////////////////////////////////////////////////////////////

#ifndef USE_SUBROUTINE_AOCALCULATOR

uniform int diskToDiskOcclusionSelector;

float diskToDiskOcclusion(const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea,
						  const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea) {
	switch(diskToDiskOcclusionSelector) {
		case 0: return d2do_bunnell(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
		case 1: return d2do_dssao(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
		case 2: return d2do_test(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
	}
}

#endif
//////////////////////////////////////////////////////////////////
#endif
