#version 420

#include ubo_matrices.inc
#include ubo_materials.inc
#include ubo_lights.inc
#include ubo_deferred.inc
#include lighting.inc

#include gdlbuffer.inc

gdlbufData receiverData;
gdlbufData currEmitterData;

gdlbufData receiverDataSampling;
gdlbufData currEmitterDataSampling;
gdlbufData mousePosData;
gdlbufData rayOriginData;



#include draw.inc

uniform float ambientLightLevel;

uniform int technique; 

const int SIMPLE_SUM                 = 0;
const int TRIANGLE_HIERARCHICAL_SKIP = 1;
const int PYRAMID_BUCKETS            = 2;

// debugging
uniform bool albedoEnabled;
uniform bool doEnabled;
uniform bool aoEnabled;
uniform bool ilEnabled;
uniform bool ambientLightEnabled;
uniform bool directLightEnabled;



uniform bool showAreas;
uniform bool showSampling;
uniform bool showSamplingDensity;
uniform bool showZoomedArea;

// sampling visualization
uniform vec2 mousePos;
uniform vec2 rayOrigin;

/////////////////////////////////////////////////////
// input

// all input is taken by texture sampling

/////////////////////////////////////////////////////
// output

layout(location = 0) out vec4 aoColor;

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

#include sampling.inc
#include ambientOcclusion.inc
#include indirectLighting.inc
/////////////////////////////////////////////////////////////


float computeSolidAngle( const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea, 
		                 const in vec3 receiverPosition, const in vec3 receiverNormal) {						 	
	//float coveredSolidAngle = solidAngle_simple(emitterPosition,  emitterNormal, emitterArea, receiverPosition, receiverNormal);
	float coveredSolidAngle = solidAngle_approx(emitterPosition,  emitterNormal, emitterArea, receiverPosition, receiverNormal);
	
	//coveredSolidAngle = 0.098;
	//coveredSolidAngle = 0.099;

	//0.3926 -> 0.3927
	//coveredSolidAngle = 0.3926;		//black
	//coveredSolidAngle = 0.3927;	

	//coveredSolidAngle = 1.57;
	//coveredSolidAngle = 1.58;

	return coveredSolidAngle*solidAngleMultiplier;                        
}

float computeSampleAmbientOcclusion(const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea, 
								    const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea, const in float sampleWeight) {
	
	float sampleOcclusion = diskToDiskOcclusion(emitterPosition, emitterNormal, emitterArea, 
													receiverPosition, receiverNormal, receiverArea);
	//float sampleOcclusion = emitterPosition.z-receiverPosition.z;	
	return sampleOcclusion*sampleWeight;
}

vec3 computeSampleIndirectLight(const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea, 
								 const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea, 
								 const in vec3 emitterDirectDiffuseLighting, const in float sampleWeight) {
	
	vec3  sampleIndirectLight = diskToDiskRadiance(emitterDirectDiffuseLighting, emitterPosition, emitterNormal, emitterArea,
													receiverPosition, receiverNormal, receiverArea);	
	return sampleIndirectLight*sampleWeight;
}

/////////////////////////////////////////////////////////////



vec3 computeFragColor( const in vec2 coords, const in bool showSamplingIntensity ) {
	if (receiverData.depth == 1.0) {
		discard;
	}
	/////////////////////////////////////////////
	
	/////////////////////////////////////////////
	vec3  receiverPosition = receiverData.posCS;
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
		pyrDist[i] = 10e5;
		pyrOcclusion[i]  = 0;
		pyrSolidAngle[i] = 0;

		pyrDirLight[i]   = vec3(0.0);
		pyrIndLight[i]   = vec3(0.0);
		pyrSamplesCont[i] = 0;
		pyrOcclusorCont[i] = 0;
	}

	///////////////////////////
	
	vec3 directRadianceSum   = vec3(0);
	vec3 occluderRadianceSum = vec3(0);
	vec3 directAntiRadianceSum = vec3(0);

	float bounceSingularity = distMax;
	float singularity = distMax;
	float depthBias = 0.0001;
	
	for (int i = 0; i < samples; ++i) {	
		//// CALCULATE SAMPLE POSITION IN SS /////////////////////////////////////////
		vec2  samplePosSS     = samplesPos[i];
		vec3  samplePosCS     = samplesPosCS[i];

		gdlbufGet(samplePosSS, currEmitterData);
		
		//// GET SAMPLE POINT VALUES FROM GBUFFER /////////////////////////////////////////
		vec3 emitterPosition = currEmitterData.posCS; 
		vec3 emitterNormal   = currEmitterData.normal;

		vec3  recToEmi    = emitterPosition-receiverPosition;
		float recToEmiLen = length(recToEmi);
		
		//////////////////////////////////////////////////////////////////////////////////
		vec3 intersection;
		int triId = getGlobalIntersectingTriangleId(recToEmi,
								     kernelBasis,
									 receiverPosition,
									 emitterPosition,
									 intersection);
		if (triId==-1) continue;
		//////////////////////////////////////////////////////////////////////////////////
		if ( recToEmiLen <= distMax ) {
			
			float emitterArea     = currEmitterData.area*areaMultiplier;				
		
			vec3 intersection;
			int triId = getGlobalIntersectingTriangleId(recToEmi,
								     kernelBasis,
									 receiverPosition,
									 emitterPosition,
									 intersection);
			if (triId==-1) continue;

			float solidAngle = computeSolidAngle( emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal);
			/////////////////
			pyrSolidAngle[triId] += solidAngle;
			pyrSolidAngle[triId] = min(pyrSolidAngle[triId], PYRAMID_DIVS_SOLID_ANGLE);
			/////////////////
			float falloff = solidAngle_falloff(recToEmiLen);
			pyrOcclusion[triId]  += min(solidAngle, PYRAMID_DIVS_SOLID_ANGLE) * falloff;
			/////////////////
			//pyrIndLight[triId] += currEmitterData.dl * formFactor(recToEmi, recToEmiLen, receiverNormal, emitterNormal, emitterArea);
			pyrIndLight[triId] +=  currEmitterData.dl * formFactor_d2d(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, triArea);
			//pyrIndLight[triId] += currEmitterData.dl * bunnell_calcFormFactor(recToEmi, recToEmiLen*recToEmiLen, receiverNormal, emitterNormal, emitterArea);
			/////////////////
		}	
		
		//////////////////////////////////////////////////////////////////////////////////
	}	

	///////////////////////////
	vec3 scattered = vec3(0);
	vec3 reflected = vec3(0);
		
	MaterialDesc surfaceMtl = MaterialDesc(receiverData.albedo, vec3(0.0), vec3(0.0), vec3(0.0), 0.0);	
	vec3 eyeCS = -receiverData.posCS;;

	for (int i = 0 ; i < numPointLights; ++i) {
	    vec3 lightPos   = pointLights[i].posCS;
		vec3 recToLight = lightPos - receiverPosition;
		
		vec3 intersection;
		int triId = getGlobalIntersectingTriangleId(recToLight, kernelBasis, receiverPosition, lightPos, intersection);

		pointLight(pointLights[i], surfaceMtl, receiverData.normal, eyeCS, scattered, reflected);			
		if (triId != -1) {
			directRadianceSum += scattered * (1.0-pyrSolidAngle[triId]/PYRAMID_DIVS_SOLID_ANGLE);
		} else {
			directRadianceSum += scattered;
		}
	}	

	for (int i = 0 ; i < numSpotLights; ++i) {
	    vec3 lightPos   = spotLights[i].posCS;
		vec3 recToLight = lightPos - receiverPosition;
		
		vec3 intersection;
		int triId = getGlobalIntersectingTriangleId(recToLight, kernelBasis, receiverPosition, lightPos, intersection);
			
		spotLight(spotLights[i], surfaceMtl, receiverData.normal, eyeCS, scattered, reflected);				
		if (triId != -1) {
			directRadianceSum += scattered * (1.0-pyrSolidAngle[triId]/PYRAMID_DIVS_SOLID_ANGLE);
		} else {
			directRadianceSum += scattered;		
		}	
	}		
	for (int i = 0 ; i < numDirLights; ++i) {
		vec3 lightPos   = receiverPosition - dirLights[i].dirCS;
	    vec3 recToLight = lightPos - receiverPosition;
		
		vec3 intersection;
		int triId = getGlobalIntersectingTriangleId(recToLight, kernelBasis, receiverPosition, lightPos, intersection);

		dirLight(dirLights[i], surfaceMtl, receiverData.normal, eyeCS, scattered, reflected);
			
		if (triId != -1) {
			directRadianceSum += scattered * (1.0-pyrSolidAngle[triId]/PYRAMID_DIVS_SOLID_ANGLE);
		} else {
			directRadianceSum += scattered;
		}
	}
		
	//////////////////////////////////////////////////////////////////////////////////
	float sa = 0.0;
	ambientOcclusion = 0.0;
	indirectLight    = vec3(0.0);	
	for (int i = 0; i < PYRAMID_DIVS; ++i) {
		ambientOcclusion += pyrOcclusion[i];
		indirectLight    += pyrIndLight[i];	
		sa += pyrSolidAngle[i];		
	}
	
	ambientOcclusion = sa / (2*PI);	
	//ambientOcclusion = ambientOcclusion / samples;

	directRadianceSum /= PYRAMID_DIVS;
		
	//directRadianceSum   = max(vec3(0), directRadianceSum);
	occluderRadianceSum = max(vec3(0), occluderRadianceSum);
	directAntiRadianceSum  = max(vec3(0), directAntiRadianceSum);
		
	//vec3 radianceSum = vec3(0.0);
	//if (doEnabled) radianceSum -=  directAntiRadianceSum;
	//if (doEnabled) radianceSum += receiverData.dl + directAntiRadianceSum; //directRadianceSum;
	//if (ilEnabled) radianceSum += occluderRadianceSum;

	/////////////////////////////////////////////	

	///////////////////////////////////////////////	
	float scaledAmbientOcclusion  = 1.0 ;
	vec3  scaledIndirectLight     = vec3(0.0) ;
	
	scaledAmbientOcclusion = (1-ambientOcclusion*aoMultiplier);
	scaledIndirectLight    = indirectLight*ilMultiplier;

	///////////////////////////////////////////////
	vec3 color = vec3(scaledAmbientOcclusion);

	// global output
	aoColor = vec4(scaledIndirectLight, scaledAmbientOcclusion);
	//aoColor = vec4(vec3(scaledAmbientOcclusion), 1.0);
		
	//aoColor = vec4(1.0, 0.0, 0.0, 1.0);
	return clamp(color, 0.0, 1.0);
}

vec3 getSampleSquareColor( const in vec2 currPixelSC ) {
	vec2 mousePixelTC = mousePos * invScreenSize;
		
	gdlbufGet(mousePixelTC, receiverDataSampling);
	vec3  receiverPosition = receiverDataSampling.posCS;
	vec3  receiverNormal   = receiverDataSampling.normal;
	float receiverArea     = receiverDataSampling.area*areaMultiplier;

	//vec3  receiverPosition = gdlbufGetPosition(currPixelSC); //receiverData.posCS;
	//vec3  receiverNormal   = gdlbufGetNormal(currPixelSC);   //receiverData.normal;
	//float receiverArea     = gdlbufGetArea(currPixelSC)*areaMultiplier;     //receiverData.area*areaMultiplier;

	calcSamplesPos(mousePixelTC, halfSamplesPerDir, radLength, receiverPosition, receiverNormal);	

	int samples   = halfSamplesPerDir * 2 * 4;		
			
	for(int i = 0; i < samples; ++i) {
	    vec2 currSamplePos = samplesPos[i] * screenSize;
		
		float horiz = abs(currSamplePos.x - currPixelSC.x);
		float vert  = abs(currSamplePos.y - currPixelSC.y);
		
		float sampleVisSize = 3.0;
		
		if ( horiz <= sampleVisSize && vert <= sampleVisSize ) {	
		
			gdlbufGet(samplesPos[i], currEmitterDataSampling);	
			float emitterArea   = currEmitterDataSampling.area*areaMultiplier;
			
			vec3 sampled = vec3(0.0);
			
			vec3  emitterPosition = currEmitterDataSampling.posCS;
			vec3  V = emitterPosition - receiverPosition;
				
			float dist = length(V);	
			
			if (dist < distMax) {
			
				vec3 recToEmi    = emitterPosition-receiverPosition;
				mat3 kernelBasis = getOrthoBasis(mousePixelTC, receiverNormal);
						
				vec3 intersection;
				int triId = getGlobalIntersectingTriangleId(
				//int triId = getIntersectingTriangleId(
									 recToEmi,
								     kernelBasis,
									 receiverPosition,
									 emitterPosition,
									 intersection);

			    if (triId != -1) {
					vec3  emitterNormal = currEmitterDataSampling.normal;

					float sa_val = computeSolidAngle( emitterPosition, emitterNormal,  emitterArea, 
							  receiverPosition, receiverNormal);
					sampled = vec3(sa_val/PYRAMID_DIVS_SOLID_ANGLE);

				} else {
					sampled = vec3(1,1,0);
				}
								
				if (horiz <= sampleVisSize-1 && vert <= sampleVisSize-1) {										
					if (triId!=-1) {
						sampled = colorid[triId];
					} else {
						sampled = vec3(-1);
					}					
				}
			} else {
				sampled = vec3(1.0, 0.0, 0.0);
				if (horiz <= sampleVisSize-1 && vert <= sampleVisSize-1) {
					sampled = vec3(-1);
				}
			}	
					
			return sampled;				
		} 
	}
	return vec3(-1.0);
}

/*
vec3 getSampleSquareColor( in vec2 currPixelSC ) {
	return vec3(0.0);
	
	switch(technique) {
		case SIMPLE_SUM                 : return getSampleSquareColor_randomStopAtK( currPixelSC );	
		case TRIANGLE_HIERARCHICAL_SKIP : return getSampleSquareColor_triangleSkip( currPixelSC );		
		case PYRAMID_BUCKETS            : return getSampleSquareColor_randomPyramidStopAtK( currPixelSC );	
	}
}
*/


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


vec3 debugSamplingOctantTriangle(const in vec2 currPixel, const in int octant, 
                                 const in vec2 mousePosTC,
								 const in vec3 rPosition,
								 const in vec2 rayOriginTC,
								 const in vec3 ePosition,
								 const in mat3 kernelBasis ) {

	vec3 dirCS       = ePosition - rPosition;
	
	// setup triangle vertices
	vec3 triverts[3];
	switch(octant) {
		case 0:
			triverts[0] = vec3(1, 0, 0) * radLength;
			triverts[1] = vec3(0, 1, 0) * radLength;
			triverts[2] = vec3(0, 0, 1) * radLength;
			break;
		case 1:
			triverts[0] = vec3(-1, 0, 0) * radLength;
			triverts[1] = vec3(0, 1, 0) * radLength;
			triverts[2] = vec3(0, 0, 1) * radLength;
			break;
		case 2:
			triverts[0] = vec3(-1, 0, 0) * radLength;
			triverts[1] = vec3(0, 1, 0) * radLength;
			triverts[2] = vec3(0, 0, -1) * radLength;
			break;
		case 3:
			triverts[0] = vec3(1, 0, 0) * radLength;
			triverts[1] = vec3(0, 1, 0) * radLength;
			triverts[2] = vec3(0, 0, -1) * radLength;
			break;
	}
	
	// transform vertices pos in CS
	// calc SS pos too
	vec2 trivertsSS[3];
	for (int i = 0; i < 3; ++i) {
		triverts[i] = rPosition + kernelBasis*triverts[i];
		trivertsSS[i] = camSpaceToScreenSpaceTC(triverts[i]);
	}

	// edge vectors - in camera space
	vec3 v0v1 = triverts[1] - triverts[0];
	vec3 v0v2 = triverts[2] - triverts[0];
	vec3 v1v2 = triverts[2] - triverts[1];
	
	// calc subdivision points in cam space and project to SS
	const int n = TRIANGLE_DIVS;
	const float incr = 1.0/n;
	
	vec2 v0v1_subdivSS[TRIANGLE_DIVS];
	vec2 v0v2_subdivSS[TRIANGLE_DIVS];
	vec2 v1v2_subdivSS[TRIANGLE_DIVS];

	for (int i = 0; i < n; ++i) {		
		v0v1_subdivSS[i] = camSpaceToScreenSpaceTC(triverts[0] + v0v1 * incr * i);
		v0v2_subdivSS[i] = camSpaceToScreenSpaceTC(triverts[0] + v0v2 * incr * i);
		v1v2_subdivSS[i] = camSpaceToScreenSpaceTC(triverts[1] + v1v2 * incr * i);
	}
	
	// output color accumulator	
	float val = 0;
	
	// draw internal lines
	int idiv = 1;
	int idx  = 0;
	while (idiv < n) {
		idx = n-idiv;

		//v0v1 parallels
		val += drawLine( currPixel, v1v2_subdivSS[idiv], v0v2_subdivSS[idiv], 0.5);

		//v0v2 parallels
		val += drawLine( currPixel, v1v2_subdivSS[idiv], v0v1_subdivSS[idx], 0.5);
		
		//v1v2 parallels
		val += drawLine( currPixel, v0v2_subdivSS[idiv], v0v1_subdivSS[idiv], 0.5);

		idiv++;		
	}
	
	// draw triangle edges
	val += drawLine(currPixel, trivertsSS[0], trivertsSS[1], 1);
	val += drawLine(currPixel, trivertsSS[1], trivertsSS[2], 1);
	val += drawLine(currPixel, trivertsSS[2], trivertsSS[0], 1);
	
	return colorid_q[octant] * val;
}

vec3 debugSamplingIntersection(const in vec2 currPixel,
                               const in vec2 mousePosTC,							   
							   const in vec3 rPosition, 
							   const in vec2 rayOriginTC,
							   const in vec3 ePosition,
							   const in mat3 kernelBasis) {
	vec3 dirCS       = ePosition - rPosition;
	////////////////////////////////////////////////////////////////////////////
	// find intersection point and triangle
	vec3 intersectionpoint;
	//int triId = getIntersectingTriangleId(dirCS, kernelBasis, rPosition, ePosition, intersectionpoint);	
	int triId = getGlobalIntersectingTriangleId(dirCS, kernelBasis, rPosition, ePosition, intersectionpoint);	
		
	vec2 intersectionPointTC = camSpaceToScreenSpaceTC(intersectionpoint);

	////////////////////////////////////////////////////////////////////////////
	vec3 ret = vec3(0.0);	

	if (triId!=-1) {
	//if (triId > -1 && triId < TRIANGLE_DIVS_SQUARED) {

		// draw line: emisphere center -> intersection point
		ret += vec3(drawLine(currPixel, intersectionPointTC, mousePosTC, 1));		
	
		// draw intersection point	
		ret += vec3( drawPoint(currPixel, intersectionPointTC*screenSize, 2));

		// draw line: intersection point -> mousepos
		ret += colorid[triId] * drawLine(currPixel, intersectionPointTC, rayOriginTC, 1);
	}
	return ret;
	
}

vec3 debugSampling(const in vec2 currPixelSC, const in vec3 color) {
	vec3 ret = color;
	vec3 sampled = getSampleSquareColor(currPixelSC);
	if (sampled != vec3(-1)) {			
		ret = sampled;
	} else {
		//FragColor = vec4(computeFragColor(currPixelSC*invScreenSize, showSamplingDensity), 1.0);	
		vec2 mousePosTC  = mousePos * invScreenSize;
		vec2 rayOriginTC = rayOrigin * invScreenSize;
		/////////////////////////////////////////////////////////////////////////////////////////
							
		gdlbufGet(mousePosTC, mousePosData);

		vec3 rPosition = mousePosData.posCS;
		vec3 rNormal   = mousePosData.normal;	

		gdlbufGet(rayOriginTC, rayOriginData);
		vec3 ePosition   = rayOriginData.posCS;
					
		mat3 kernelBasis = getOrthoBasis(mousePosTC, rNormal);
		/////////////////////////////////////////////////////////////////////////////////////////		

// pyramid				
		vec3 ctriangles;
		ctriangles  = debugSamplingOctantTriangle(currPixelSC, 0, mousePosTC, rPosition, rayOriginTC, ePosition, kernelBasis);
		ctriangles += debugSamplingOctantTriangle(currPixelSC, 1, mousePosTC, rPosition, rayOriginTC, ePosition, kernelBasis);
		ctriangles += debugSamplingOctantTriangle(currPixelSC, 2, mousePosTC, rPosition, rayOriginTC, ePosition, kernelBasis);
		ctriangles += debugSamplingOctantTriangle(currPixelSC, 3, mousePosTC, rPosition, rayOriginTC, ePosition, kernelBasis);

		if (ctriangles != vec3(0)) ret = ctriangles;
		
// intersection line/point
		vec3 cintline = debugSamplingIntersection(currPixelSC, mousePosTC, rPosition, rayOriginTC, ePosition, kernelBasis);
		if (cintline != vec3(0)) ret = cintline;

// normal
		vec3 n = rayOriginData.normal;
		vec2 normalEnd = camSpaceToScreenSpaceTC(ePosition+n);
		ret += vec3(drawLine(currPixelSC, rayOriginTC , normalEnd, 1));
	}	
	return ret;
}


/////////////////////////////////////////////////////////////


vec2 zoomedPosition() {
	vec2 ret = vec2(gl_FragCoord.x, gl_FragCoord.y);
	if (showZoomedArea) {
		const ivec2 zoomSize = ivec2(500, 400);
		int zoomX = int( screenSize.x - zoomSize.x ) ;
		int zoomY = int( screenSize.y - zoomSize.y ) ;
	
		ivec2 zoomZero  = ivec2(zoomX, zoomY);
		vec2 zoomCenter = vec2(zoomZero) + zoomSize*0.5;

		int zoomXoffset = int(ret.x - zoomZero.x);
		int zoomYoffset = int(ret.y - zoomZero.y);
	
		if (zoomXoffset > 0 && zoomYoffset > 0) { // pixel in zoom area		
			ret.x = mousePos.x + (ret.x - float(zoomZero.x))*0.5 - zoomSize.x*0.25;
			ret.y = mousePos.y + (ret.y - float(zoomZero.y))*0.5 - zoomSize.y*0.25;
		}	
	}
	return ret;
}


void main() {
	vec2 coordsSS = zoomedPosition();
	vec2 coordsTC = coordsSS * invScreenSize; 

	gdlbufGet(coordsTC, receiverData);
	
	vec3 color;	
	if (showAreas) {
		color = vec3(receiverData.area*areaMultiplier);
	} else {
		color = computeFragColor(coordsTC, showSamplingDensity);

		if (showSampling) color = debugSampling(coordsSS, color);
	}
		
	//aoColor = vec4(color, 1.0);	
}


//////////////////

