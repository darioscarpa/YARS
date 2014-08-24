#version 420

#include ubo_matrices.inc
#include ubo_materials.inc
#include ubo_lights.inc
#include ubo_deferred.inc
#include lighting.inc

#include utils.inc

#include saao_gbuffer.inc

saao_gBufData receiverData;
saao_gBufData currOccluderData;

uniform float ambientLightLevel;

uniform int technique; 

const int TECH_SASSAO      = 0;
const int TECH_ALCHEMY     = 1;


/////////////////////////////////////////////////////
// input

// all input is taken by texture sampling

/////////////////////////////////////////////////////
// output

//layout(location = 0) out vec4 aoColor;
layout(location = 0) out float aoColor;

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

float computeFragColor( const in vec2 coords ) {
	if (receiverData.depth == 1.0) {
		discard;
	}
	/////////////////////////////////////////////
	
	/////////////////////////////////////////////
	//vec3  receiverPosition = receiverData.posCS;
	vec3 receiverPosition = screenSpaceToCamSpace(coords, receiverData.depth);
	vec3  receiverNormal   = receiverData.normal;
	float receiverArea     = receiverData.area*areaMultiplier;	
	/////////////////////////////////////////////
		
	mat3 kernelBasis = getOrthoBasis(coords, receiverNormal);

	calcSamplesPos(coords, halfSamplesPerDir, radLength, receiverPosition, receiverNormal);

	/////////////////////////////////////////////	
	float ambientOcclusion = 0.0;
	
	/////////////////////////////////////////////	
	int samples   = halfSamplesPerDir * 2 * 4;	
	float invSamples = 1.0 / samples;

	
	for (int i = 0; i < PYRAMID_DIVS; ++i) {
		//pyrOcclusion[i]  = 0;
		pyrSolidAngle[i] = 0;
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
		//gBufCalcPosCS(samplePosSS, currOccluderData);
		
		//// GET SAMPLE POINT VALUES FROM GBUFFER /////////////////////////////////////////
		//vec3 emitterPosition = currOccluderData.posCS; 
		vec3 emitterPosition = screenSpaceToCamSpace(samplePosSS, currOccluderData.depth);
				
		vec3  recToEmi    = emitterPosition-receiverPosition;
		float recToEmiLen = length(recToEmi);
		
		if ( recToEmiLen > distMax ) continue; // sample too far, skip

		//////////////////////////////////////////////////////////////////////////////////
		vec3  recToEmiNorm = normalize(recToEmi);
		float costhetaR = clamp(dot( receiverNormal,  recToEmiNorm ), 0.0, 1.0) ;
	
		if (costhetaR < diskDisplacement) continue; // angle bias, skip
		//////////////////////////////////////////////////////////////////////////////////

		vec3 intersection;
		int triId = getGlobalIntersectingTriangleId(recToEmi,
								     kernelBasis,
									 receiverPosition,
									 emitterPosition,
									 intersection);
		if (triId==-1) continue;  // no triangle intersection, skip

		//////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////////////

		gBufFetchNormalArea(samplePosSS, currOccluderData);		
		float emitterArea     = currOccluderData.area*areaMultiplier;				

		//////////////////////////////////////////////////////////////////////////////////
		float distSquared = recToEmiLen*recToEmiLen + 1e-16;
		
		//////////
		float solidAngleApprox  = (costhetaR * emitterArea) / distSquared;
		//////////		
		//vec3 emitterNormal   = currOccluderData.normal;		
		//float costhetaE = abs(dot( emitterNormal,  -recToEmiNorm ));
		//float solidAngleApprox = (costhetaR * costhetaE * emitterArea) / distSquared;	
		//////////
		//////////////////////////////////////////////////////////////////////////////////

		float falloff = solidAngle_falloff(recToEmiLen);
		solidAngleApprox *= falloff;
	
		//////////////////////////////////////////////////////////////////////////////////
		float solidAngle = solidAngleMultiplier * solidAngleApprox;
		
		/////////////////
		pyrSolidAngle[triId] += solidAngle;
		pyrSolidAngle[triId] = min(pyrSolidAngle[triId], PYRAMID_DIVS_SOLID_ANGLE);
		/////////////////
		//float falloff = solidAngle_falloff(recToEmiLen);
		//pyrOcclusion[triId]  += min(solidAngle, PYRAMID_DIVS_SOLID_ANGLE) * falloff;
		/////////////////		
		//////////////////////////////////////////////////////////////////////////////////
	}	
		
	//////////////////////////////////////////////////////////////////////////////////
	float sa = 0.0;
	ambientOcclusion = 0.0;
	for (int i = 0; i < PYRAMID_DIVS; ++i) {
		//ambientOcclusion += pyrOcclusion[i];
		sa += pyrSolidAngle[i];				
	}
	
	//ambientOcclusion = sa / (2*PI);	
	ambientOcclusion = sa * INV_2PI;
	/////////////////////////////////////////////	

	///////////////////////////////////////////////		
	float scaledAmbientOcclusion = (1-ambientOcclusion*aoMultiplier);	
	///////////////////////////////////////////////
	return scaledAmbientOcclusion;	
}

/////////////////////////////////////////////////////////////
float computeFragColor_alchemy( const in vec2 coords ) {
	if (receiverData.depth == 1.0) {
		discard;
	}
	/////////////////////////////////////////////
	
	/////////////////////////////////////////////
	//vec3  receiverPosition = receiverData.posCS;
	vec3 receiverPosition = screenSpaceToCamSpace(coords, receiverData.depth);
	vec3  receiverNormal   = receiverData.normal;
	/////////////////////////////////////////////
		
	mat3 kernelBasis = getOrthoBasis(coords, receiverNormal);

	calcSamplesPos(coords, halfSamplesPerDir, radLength, receiverPosition, receiverNormal);

	/////////////////////////////////////////////	
	float ambientOcclusion = 0.0;
	
	/////////////////////////////////////////////	
	int samples   = halfSamplesPerDir * 2 * 4;	
	float invSamples = 1.0 / samples;

	float alchemySum = 0;

	for (int i = 0; i < samples; ++i) {	
		//// CALCULATE SAMPLE POSITION IN SS /////////////////////////////////////////
		vec2  samplePosSS     = samplesPos[i];
		vec3  samplePosCS     = samplesPosCS[i];

		//gdlbufGet(samplePosSS, currOccluderData);
		
		gBufFetchDepth(samplePosSS, currOccluderData);
		//gBufCalcPosCS(samplePosSS, currOccluderData);
		
		//// GET SAMPLE POINT VALUES FROM GBUFFER /////////////////////////////////////////
		//vec3 emitterPosition = currOccluderData.posCS; 
		vec3 emitterPosition = screenSpaceToCamSpace(samplePosSS, currOccluderData.depth);
				
		vec3  recToEmi    = emitterPosition-receiverPosition;
		float recToEmiLen = length(recToEmi);
		
		if ( recToEmiLen > distMax ) continue; // sample too far, skip

		//////////////////////////////////////////////////////////////////////////////////
		vec3  recToEmiNorm = normalize(recToEmi);
		float costhetaR = clamp(dot( receiverNormal,  recToEmiNorm ), 0.0, 1.0) ;
	
		if (costhetaR < diskDisplacement) continue; // angle bias, skip
		//////////////////////////////////////////////////////////////////////////////////

		//alchemySum += alchemy( emitterPosition, receiverPosition, receiverNormal);
		float dotp = max( 0, dot(recToEmi, receiverNormal) + receiverPosition.z * alchemy_u); //0.0001);
		float dist2 = dot(recToEmi,recToEmi);
		alchemySum += dotp/dist2;
				
		//////////////////////////////////////////////////////////////////////////////////
		float distSquared = recToEmiLen*recToEmiLen + 1e-16;
		
		//////////////////////////////////////////////////////////////////////////////////
	}	
		
	
	//////////////////////////////////////////////////////////////////////////////////
	// ALCHEMY	
	ambientOcclusion =  pow( (2*alchemy_ro/samples)*alchemySum, alchemy_k);

	/////////////////////////////////////////////	

	///////////////////////////////////////////////	
	float scaledAmbientOcclusion = (1-ambientOcclusion*aoMultiplier);
	///////////////////////////////////////////////
	
	return scaledAmbientOcclusion;	
}
/////////////////////////////////////////////////////////////

void main() {
	vec2 coordsTC = gl_FragCoord.xy * invScreenSize; 

	//gdlbufGet(coordsTC, receiverData);

	gBufFetchNormalArea(coordsTC, receiverData);
	gBufFetchDepth(coordsTC, receiverData);
	//gBufCalcPosCS(coordsTC, receiverData);	
	
	//if (coordsTC.x + coordsTC.y < 1)  {
	//	aoColor = computeFragColor(coordsTC);	
	//} else {
		//aoColor = 1.0;
	//}

	
	if (technique == TECH_SASSAO) aoColor = computeFragColor(coordsTC);	
	else /*if (technique == TECH_ALCHEMY) */ aoColor = computeFragColor_alchemy(coordsTC);	

	//aoColor = computeFragColor(coordsTC);	
	//aoColor = computeFragColor_alchemy(coordsTC);	
}

/////////////////////////////////////////////////////////////



