#version 420

#include ubo_matrices.inc
#include ubo_materials.inc
#include ubo_lights.inc
#include ubo_deferred.inc
#include lighting.inc

#include utils.inc

#include saaoil_gbuffer.inc

saaoil_gBufData receiverData;
saaoil_gBufData currOccluderData;

uniform float ambientLightLevel;

uniform int technique; 

const int TECH_SASSAO      = 0;
const int TECH_ALCHEMY     = 1;

/////////////////////////////////////////////////////
// input

// all input is taken by texture sampling

/////////////////////////////////////////////////////
// output

layout(location = 0) out vec4 ilAoColor;

/////////////////////////////////////////////////////

// AO params
uniform int   halfSamplesPerDir; // half of the samples taken for each of 4 directions (so if aoSamples = 1 -> 8 samples, aoSamples 2 -> 16) : samples = 2*4*aoSamples
uniform float radLength; 
uniform float diskDisplacement;
uniform float distMax;

uniform float aoMultiplier;
uniform float ilMultiplier;
uniform float areaMultiplier;
uniform float solidAngleMultiplier;

uniform float alchemy_k;
uniform float alchemy_ro;
uniform float alchemy_u;

#include sampling.inc
#include ambientOcclusion.inc
#include indirectLighting.inc

/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////

vec4 computeFragColor( const in vec2 coords ) {
	if (receiverData.depth == 1.0) {
		discard;
	}
	/////////////////////////////////////////////
	
	/////////////////////////////////////////////
	//vec3  receiverPosition = receiverData.posCS;
	vec3 receiverPosition  = screenSpaceToCamSpace(coords, receiverData.depth);
	vec3  receiverNormal   = receiverData.normal;
	float receiverArea     = receiverData.area*areaMultiplier;	
	/////////////////////////////////////////////
		
	mat3 kernelBasis = getOrthoBasis(coords, receiverNormal);

	calcSamplesPos(coords, halfSamplesPerDir, radLength, receiverPosition, receiverNormal);

	/////////////////////////////////////////////	
	float ambientOcclusion = 0.0;
	vec3  indirectLight    = vec3(0.0);	

	/////////////////////////////////////////////	
	int samples   = halfSamplesPerDir * 2 * 4;	
	float invSamples = 1.0 / samples;
		
	float sphereArea = 4 * PI * (radLength * radLength);
	float triArea    = (sphereArea*0.5) * INV_PYRAMID_DIVS;

	for (int i = 0; i < PYRAMID_DIVS; ++i) {
		pyrOcclusion[i]  = 0;
		pyrSolidAngle[i] = 0;
		pyrIndLight[i] = vec3(0);
	}

	///////////////////////////

	//float bounceSingularity = distMax;
	//float singularity = distMax;
	//float depthBias = 0.0001;
	
	for (int i = 0; i < samples; ++i) {	
		//// CALCULATE SAMPLE POSITION IN SS /////////////////////////////////////////
		vec2  samplePosSS     = samplesPos[i];
		vec3  samplePosCS     = samplesPosCS[i];

		//gdlbufGet(samplePosSS, currOccluderData);
		gBufFetchDepth(samplePosSS, currOccluderData);
	

		//// GET SAMPLE POINT VALUES FROM GBUFFER /////////////////////////////////////////
		//vec3 emitterPosition = currOccluderData.posCS; 
		vec3 emitterPosition = screenSpaceToCamSpace(samplePosSS, currOccluderData.depth);
		
		vec3  recToEmi    = emitterPosition-receiverPosition;
		float recToEmiLen = length(recToEmi);
		
		if ( recToEmiLen > distMax ) continue; // sample too far, skip

		//////////////////////////////////////////////////////////////////////////////////
		vec3 intersection;
		int triId = getGlobalIntersectingTriangleId(recToEmi,
								     kernelBasis,
									 receiverPosition,
									 emitterPosition,
									 intersection);
		if (triId==-1) continue;
		//////////////////////////////////////////////////////////////////////////////////
		
		gBufFetchNormalArea(samplePosSS, currOccluderData);		
		vec3 emitterNormal   = currOccluderData.normal;
		float emitterArea     = currOccluderData.area*areaMultiplier;				

		//////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////////////
		float solidAngle = solidAngleMultiplier * solidAngle_approx(emitterPosition,  emitterNormal, emitterArea, receiverPosition, receiverNormal);

		/////////////////
		pyrSolidAngle[triId] += solidAngle;
		pyrSolidAngle[triId] = min(pyrSolidAngle[triId], PYRAMID_DIVS_SOLID_ANGLE);
		/////////////////
		float falloff = solidAngle_falloff(recToEmiLen);
		pyrOcclusion[triId]  += min(solidAngle, PYRAMID_DIVS_SOLID_ANGLE) * falloff;
		/////////////////

		
		gBufFetchDirectLight(samplePosSS, currOccluderData);		
		//pyrIndLight[triId] +=  currOccluderData.dl * formFactor_d2d(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, triArea);
		pyrIndLight[triId] +=  currOccluderData.dl * formFactor_d2d(emitterPosition, emitterNormal, 1.0, receiverPosition, receiverNormal, 1.0);
		//////////////////////////////////////////////////////////////////////////////////
	}	

	
	//////////////////////////////////////////////////////////////////////////////////
	float sa = 0.0;
	ambientOcclusion = 0.0;
	indirectLight    = vec3(0.0);	
	for (int i = 0; i < PYRAMID_DIVS; ++i) {
		ambientOcclusion += pyrOcclusion[i];
		sa               += pyrSolidAngle[i];		
		indirectLight    += pyrIndLight[i];			
	}
	
	//ambientOcclusion = sa / (2*PI);	
	ambientOcclusion = sa * INV_2PI;
	
	//indirectLight = indirectLight * invSamples;
		
	/////////////////////////////////////////////	

	///////////////////////////////////////////////	
	float scaledAmbientOcclusion = (1-ambientOcclusion*aoMultiplier);
	vec3  scaledIndirectLight    = indirectLight*ilMultiplier;
	///////////////////////////////////////////////
	
	return vec4(scaledIndirectLight, scaledAmbientOcclusion);	
}


/////////////////////////////////////////////////////////////




vec4 computeFragColor_alchemy( const in vec2 coords ) {
	if (receiverData.depth == 1.0) {
		discard;
	}
	/////////////////////////////////////////////
	
	/////////////////////////////////////////////
	//vec3  receiverPosition = receiverData.posCS;
	vec3 receiverPosition  = screenSpaceToCamSpace(coords, receiverData.depth);
	vec3  receiverNormal   = receiverData.normal;
	float receiverArea     = receiverData.area*areaMultiplier;	
	/////////////////////////////////////////////
		
	mat3 kernelBasis = getOrthoBasis(coords, receiverNormal);

	calcSamplesPos(coords, halfSamplesPerDir, radLength, receiverPosition, receiverNormal);

	/////////////////////////////////////////////	
	float ambientOcclusion = 0.0;
	vec3  indirectLight    = vec3(0.0);	

	float alchemySum = 0.0;
	/////////////////////////////////////////////	
	int samples   = halfSamplesPerDir * 2 * 4;	
	float invSamples = 1.0 / samples;
		
	for (int i = 0; i < samples; ++i) {	
		//// CALCULATE SAMPLE POSITION IN SS /////////////////////////////////////////
		vec2  samplePosSS     = samplesPos[i];
		vec3  samplePosCS     = samplesPosCS[i];

		//gdlbufGet(samplePosSS, currOccluderData);
		gBufFetchDepth(samplePosSS, currOccluderData);
	

		//// GET SAMPLE POINT VALUES FROM GBUFFER /////////////////////////////////////////
		//vec3 emitterPosition = currOccluderData.posCS; 
		vec3 emitterPosition = screenSpaceToCamSpace(samplePosSS, currOccluderData.depth);
		
		vec3  recToEmi    = emitterPosition-receiverPosition;
		float recToEmiLen = length(recToEmi);
		
		if ( recToEmiLen > distMax ) continue; // sample too far, skip

		//////////////////////////////////////////////////////////////////////////////////
				
		
		alchemySum += alchemy( emitterPosition, receiverPosition, receiverNormal);
		
		gBufFetchNormalArea(samplePosSS, currOccluderData);		
		vec3 emitterNormal   = currOccluderData.normal;

		gBufFetchDirectLight(samplePosSS, currOccluderData);		
		indirectLight +=  currOccluderData.dl * formFactor_d2d(emitterPosition, emitterNormal, 1.0, receiverPosition, receiverNormal, 1.0);
		//////////////////////////////////////////////////////////////////////////////////
	}	

	
	//////////////////////////////////////////////////////////////////////////////////
	// ALCHEMY	
	
	ambientOcclusion =  pow( (2*alchemy_ro/samples)*alchemySum, alchemy_k);
	
	//indirectLight = indirectLight * invSamples;

	/////////////////////////////////////////////	

	///////////////////////////////////////////////	
	float scaledAmbientOcclusion = (1-ambientOcclusion*aoMultiplier);
	vec3  scaledIndirectLight    = indirectLight*ilMultiplier;
	///////////////////////////////////////////////
	
	return vec4(scaledIndirectLight, scaledAmbientOcclusion);	
}

/////////////////////////////////////////////////////////////

void main() {
	vec2 coordsTC = gl_FragCoord.xy * invScreenSize; 

	gBufFetchNormalArea(coordsTC, receiverData);
	gBufFetchDepth(coordsTC, receiverData);
	
	//gdlbufGet(coordsTC, receiverData);
	
	if (technique == TECH_SASSAO) ilAoColor = computeFragColor(coordsTC);	
	else /*if (technique == TECH_ALCHEMY) */ ilAoColor = computeFragColor_alchemy(coordsTC);	

	//ilAoColor = computeFragColor(coordsTC);	
	//ilAoColor = computeFragColor_alchemy(coordsTC);	
}
