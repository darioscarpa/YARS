#version 420

#include ubo_matrices.inc
#include ubo_materials.inc
#include ubo_lights.inc
#include ubo_deferred.inc
#include lighting.inc

#include gbuffer.inc
#include dlbuffer.inc

#include draw.inc

uniform float ambientLightLevel;

uniform int technique; 

const int SIMPLE_SUM                 = 0;
const int TRIANGLE_HIERARCHICAL_SKIP = 1;
const int PYRAMID_BUCKETS            = 2;
/*
const int SAMPLINGTECH_RANDOM_STOPATK    = 0;
const int SAMPLINGTECH_RANDOM_FO_STOPATK = 1;
const int SAMPLINGTECH_TRIANGLES_SOLIDANGLESKIP = 2;
const int SAMPLINGTECH_TRIANGLES_STOPATK = 3;
const int SAMPLINGTECH_RANDOM_PYRAMID    = 4;
const int SAMPLINGTECH_RANDOM_PYRAMID_FO = 5;
const int SAMPLINGTECH_PYRAMID_DSSO      = 6;
*/

// triangles:
//           interleaved, stop at K
//           non-interleaved, skip by solid Angle
// random samples
//           stop at K
//           

// debugging
uniform bool albedoEnabled;
uniform bool aoEnabled;
uniform bool ilEnabled;
uniform bool ambientLightEnabled;
uniform bool diffuseLightEnabled;
uniform bool specularLightEnabled;

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

layout(location = 0) out vec4 FragColor;

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

#include ambientOcclusion.inc
#include indirectLighting.inc
/////////////////////////////////////////////////////////////


float computeSolidAngle( const in vec3 emitterPosition,  const in vec3 emitterNormal, const in float emitterArea, 
		                 const in vec3 receiverPosition, const in vec3 receiverNormal) {						 	
	//float coveredSolidAngle = solidAngle_simple(emitterPosition,  emitterNormal, emitterArea, receiverPosition, receiverNormal);
	float coveredSolidAngle = solidAngle_approx(emitterPosition,  emitterNormal, emitterArea, receiverPosition, receiverNormal);
	//float coveredSolidAngle = alchemy(emitterPosition,  receiverPosition, receiverNormal);
	

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

#include deferred_blending_triangleSkip.inc
#include deferred_blending_triangleStopAtK.inc
#include deferred_blending_randomStopAtK.inc
#include deferred_blending_randomPyramidStopAtK.inc
#include deferred_blending_dsso.inc

/////////////////////////////////////////////////////////////

vec3 computeFragColor( const in vec2 coords, const in bool showSamplingIntensity ) {
	float zvalue = gbufGetDepth(coords);
	if (zvalue == 1.0) {
		discard;
	}
	/////////////////////////////////////////////
	
	/////////////////////////////////////////////
	vec3  receiverPosition = gbufGetPosition(coords);
	vec3  receiverNormal   = gbufGetNormal(coords);	
	float receiverArea     = gbufGetArea(coords)*areaMultiplier;	
	/////////////////////////////////////////////
		
	mat3 kernelBasis = getOrthoBasis(coords, receiverNormal);

	calcSamplesPos(coords, halfSamplesPerDir, radLength, receiverPosition, receiverNormal);

	/////////////////////////////////////////////	
	float ambientOcclusion = 0.0;
	vec3  indirectLight    = vec3(0.0);	
	
	switch(technique) {
		case SIMPLE_SUM                 : technique_randomStopAtK(kernelBasis, receiverPosition, receiverNormal, receiverArea, ambientOcclusion, indirectLight);         break;
		case TRIANGLE_HIERARCHICAL_SKIP : technique_triangleSkip(kernelBasis, receiverPosition, receiverNormal, receiverArea, ambientOcclusion, indirectLight);          break;
		case PYRAMID_BUCKETS            : technique_randomPyramidStopAtK(kernelBasis, receiverPosition, receiverNormal, receiverArea, ambientOcclusion, indirectLight);  break; 
	}

	/*
	switch(samplingTech) {
		case SAMPLINGTECH_RANDOM_STOPATK           : technique_randomStopAtK(kernelBasis, receiverPosition, receiverNormal, receiverArea, ambientOcclusion, indirectLight);         break;
		case SAMPLINGTECH_RANDOM_FO_STOPATK        : technique_randomStopAtK(kernelBasis, receiverPosition, receiverNormal, receiverArea, ambientOcclusion, indirectLight);         break;
		case SAMPLINGTECH_TRIANGLES_SOLIDANGLESKIP : technique_triangleSkip(kernelBasis, receiverPosition, receiverNormal, receiverArea, ambientOcclusion, indirectLight);          break;
		case SAMPLINGTECH_TRIANGLES_STOPATK        : technique_triangleStopAtK(kernelBasis, receiverPosition, receiverNormal, receiverArea, ambientOcclusion, indirectLight);       break;
		case SAMPLINGTECH_RANDOM_PYRAMID           : technique_randomPyramidStopAtK(kernelBasis, receiverPosition, receiverNormal, receiverArea, ambientOcclusion, indirectLight);  break;
		case SAMPLINGTECH_RANDOM_PYRAMID_FO        : technique_randomPyramidStopAtK(kernelBasis, receiverPosition, receiverNormal, receiverArea, ambientOcclusion, indirectLight);  break; 
		case SAMPLINGTECH_PYRAMID_DSSO             : technique_randomPyramidDsso(kernelBasis, receiverPosition, receiverNormal, receiverArea, ambientOcclusion, indirectLight);  break; 
	}
	*/
	
	/////////////////////////////////////////////	    
    /*
    float scaledAmbientOcclusion = (1-ambientOcclusion*aoMultiplier);
	float scaledIndirectLight    = indirectLight*ilMultiplier;
	vec3 ambientLight   = gbufGetDiffuseColor(coords) * ambientLightLevel * scaledAmbientOcclusion;
	vec3 diffuseLight   = dlbufGetScatteredLight(coords) + scaledIndirectLight;
	vec3 specularLight  = dlbufGetReflectedLight(coords);
     */		
	///////////////////////////////////////////////	
	float scaledAmbientOcclusion  = 1.0 ;
	vec3  scaledIndirectLight     = vec3(0.0) ;
	vec3 ambientLight   = vec3(0.0);
	vec3 diffuseLight   = vec3(0.0);
	vec3 specularLight  = vec3(0.0);
	vec3 albedo         = vec3(1.0);
	
	if (albedoEnabled) albedo = gbufGetDiffuseColor(coords);
	if (aoEnabled) scaledAmbientOcclusion = (1-ambientOcclusion*aoMultiplier);
	if (ilEnabled) scaledIndirectLight    = indirectLight*ilMultiplier;

	if (ambientLightEnabled)  ambientLight  = albedo * ambientLightLevel * scaledAmbientOcclusion;
	if (diffuseLightEnabled)  diffuseLight  = dlbufGetScatteredLight(coords);
	if (specularLightEnabled) specularLight = dlbufGetReflectedLight(coords);	
	///////////////////////////////////////////////

	vec3 color = ambientLight + specularLight + diffuseLight  + scaledIndirectLight;
	//vec3 color = ambientLight + specularLight + (diffuseLight  + scaledIndirectLight) * scaledAmbientOcclusion;
	
	//if (showSamplingIntensity)
	//	return vec3(1-float(takenSamples)/float(samples));
	//else
	
	return clamp(color, 0.0, 1.0);
}


vec3 getSampleSquareColor( const in vec2 currPixelSC ) {
	switch(technique) {
		case SIMPLE_SUM                 : return getSampleSquareColor_randomStopAtK( currPixelSC );	
		case TRIANGLE_HIERARCHICAL_SKIP : return getSampleSquareColor_triangleSkip( currPixelSC );		
		case PYRAMID_BUCKETS            : return getSampleSquareColor_randomPyramidStopAtK( currPixelSC );	
	}
	/*switch(samplingTech) {
		case SAMPLINGTECH_RANDOM_STOPATK      : return getSampleSquareColor_randomStopAtK( currPixelSC );	
		case SAMPLINGTECH_RANDOM_FO_STOPATK   : return getSampleSquareColor_randomStopAtK( currPixelSC );	
		case SAMPLINGTECH_TRIANGLES_SOLIDANGLESKIP : return getSampleSquareColor_triangleSkip( currPixelSC );		
		case SAMPLINGTECH_TRIANGLES_STOPATK        : return getSampleSquareColor_triangleStopAtK( currPixelSC );		
		case SAMPLINGTECH_RANDOM_PYRAMID      : return getSampleSquareColor_randomPyramidStopAtK( currPixelSC );	
		case SAMPLINGTECH_RANDOM_PYRAMID_FO   : return getSampleSquareColor_randomPyramidStopAtK( currPixelSC );	
		case SAMPLINGTECH_PYRAMID_DSSO        : return getSampleSquareColor_randomPyramidDsso( currPixelSC );	
	}*/
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


vec3 debugSamplingOctantTriangle(const in vec2 currPixel, in int octant, 
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
		val += drawLine( currPixel, v1v2_subdivSS[idiv], v0v2_subdivSS[idiv], 2.5);

		//v0v2 parallels
		val += drawLine( currPixel, v1v2_subdivSS[idiv], v0v1_subdivSS[idx], 2.5);
		
		//v1v2 parallels
		val += drawLine( currPixel, v0v2_subdivSS[idiv], v0v1_subdivSS[idiv], 2.5);

		idiv++;		
	}
	
	// draw triangle edges
	val += drawLine(currPixel, trivertsSS[0], trivertsSS[1], 3);
	val += drawLine(currPixel, trivertsSS[1], trivertsSS[2], 3);
	val += drawLine(currPixel, trivertsSS[2], trivertsSS[0], 3);
	
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
		ret += vec3(drawLine(currPixel, intersectionPointTC, mousePosTC, 2));		
	
		// draw intersection point	
		ret += vec3( drawPoint(currPixel, intersectionPointTC*screenSize, 3));

		// draw line: intersection point -> mousepos
		ret += colorid[triId] * drawLine(currPixel, intersectionPointTC, rayOriginTC, 2);
	}
	return ret;
	
}

void debugSampling(const in vec2 currPixelSC) {
	vec3 sampled = getSampleSquareColor(currPixelSC);
	//if (sampled != vec3(-1)) {			
	//	FragColor = vec4(sampled, 1.0);			
	//} else {
		FragColor = vec4(computeFragColor(currPixelSC*invScreenSize, showSamplingDensity), 1.0);	
		vec2 mousePosTC  = mousePos * invScreenSize;
		vec2 rayOriginTC = rayOrigin * invScreenSize;
		/////////////////////////////////////////////////////////////////////////////////////////

		vec3 rPosition   = gbufGetPosition(mousePosTC);
		vec3 rNormal     = gbufGetNormal(mousePosTC);		
		vec3 ePosition   = gbufGetPosition(rayOriginTC);	
			
		mat3 kernelBasis = getOrthoBasis(mousePosTC, rNormal);
		/////////////////////////////////////////////////////////////////////////////////////////
	
// pyramid				
		vec3 ctriangles;
		ctriangles  = debugSamplingOctantTriangle(currPixelSC, 0, mousePosTC, rPosition, rayOriginTC, ePosition, kernelBasis);
		ctriangles += debugSamplingOctantTriangle(currPixelSC, 1, mousePosTC, rPosition, rayOriginTC, ePosition, kernelBasis);
		ctriangles += debugSamplingOctantTriangle(currPixelSC, 2, mousePosTC, rPosition, rayOriginTC, ePosition, kernelBasis);
		ctriangles += debugSamplingOctantTriangle(currPixelSC, 3, mousePosTC, rPosition, rayOriginTC, ePosition, kernelBasis);

		if (ctriangles != vec3(0)) FragColor = vec4(ctriangles, 1.0);
		
// intersection line/point
		vec3 cintline = debugSamplingIntersection(currPixelSC, mousePosTC, rPosition, rayOriginTC, ePosition, kernelBasis);
		if (cintline != vec3(0)) FragColor = vec4(cintline, 1.0);

// normal
		//vec3 p = gbufGetPosition(rayOriginTC); 
		vec3 n = gbufGetNormal(rayOriginTC); 
		vec2 normalEnd = camSpaceToScreenSpaceTC(ePosition+n);
		FragColor += vec4( vec3(drawLine(currPixelSC, rayOriginTC , normalEnd, 1)), 1.0);
	//}	
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

	//vec2 coords = vec2(gl_FragCoord.x * invScreenSize.x, gl_FragCoord.y * invScreenSize.y);

	if (showAreas) {
		FragColor   = vec4(vec3(gbufGetArea(coordsTC)*areaMultiplier), 1.0);
	} else if (showSampling) {
		debugSampling(coordsSS);
	} else {
		FragColor = vec4(computeFragColor(coordsTC, showSamplingDensity), 1.0);
	}
	
	//FragColor = vec4(gbufGetDiffuseColor(coordsTC), 1.0);		
	//FragColor = vec4(gbufGetNormal(coordsTC), 1.0);		
	//FragColor = vec4(dlbufGetScatteredLight(coordsTC), 1.0);
	//FragColor = vec4(dlbufGetReflectedLight(coordsTC), 1.0);

	//FragColor = vec4(dlbufGetScatteredLight(coordsTC)+dlbufGetReflectedLight(coordsTC), 1.0);
	//FragColor = vec4(vec3(1-ambientOcclusion), 1.0);
}

//////////////////
