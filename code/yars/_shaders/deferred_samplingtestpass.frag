#version 420

#include ubo_matrices.inc
#include ubo_materials.inc
#include ubo_lights.inc
#include ubo_deferred.inc
#include lighting.inc

//uniform vec2 invScreenSize;
uniform vec2 mousePos;

uniform float ambientLightLevel;

/////////////////////////////////////////////////////
// input

// all input is taken by texture sampling

/////////////////////////////////////////////////////
// output

layout(location = 0) out vec4 FragColor;

/////////////////////////////////////////////////////

// AO params
uniform bool  aoEnabled;
uniform bool  aoOnly;
uniform int   aoSamples; // half of the samples taken for each of 4 directions (so if aoSamples = 1 -> 8 samples, aoSamples 2 -> 16) : samples = 2*4*aoSamples
uniform float radLength; 
uniform float diskDisplacement;
uniform float distMax;

uniform bool scaleByEmitterArea;

#include gbuffer.inc
#include deferred_lighting.inc
//////////////////////////

#include ambientOcclusion.inc
#include indirectLighting.inc

//////////////////////////

/*
vec3 getSampleSquareColor(in vec3 emitterPosition, in vec3 emitterNormal, float emitterArea,
						  in vec3 receiverPosition, in vec3 receiverNormal, float receiverArea) {
		sampled = calcDirectLight(samplesPos[i]);
		//sampled = vec3( 1 - calcAmbientOcclusion( receiverPosition, receiverNormal, samplesPos[i] ) );
		//sampled = vec3( 1 - bunnell_calcAmbientOcclusion(receiverPosition, receiverNormal, samplesPos[i]) );	
}
*/

vec3 samplingTest( const in vec2 currPixelSC, const in vec2 mousePixelSC, const in vec2 invScrSize, const in int halfSamplesPerDir, const in float radLength ) {
	//vec2 samplesPos[64]; //global
	
	vec2 mousePixelTC = mousePixelSC * invScrSize;
	
	vec3  receiverPosition = gbufGetPosition(mousePixelTC);
	vec3  receiverNormal   = gbufGetNormal(mousePixelTC);	
	float receiverArea     = gbufGetArea(mousePixelTC);

	calcSamplesPos(mousePixelTC, halfSamplesPerDir, radLength, receiverPosition, receiverNormal);

	int samples   = halfSamplesPerDir * 2 * 4;

	
	/*
	samplesPos[0] = vec2(400, 400);
	samplesPos[1] = vec2(501, 500);
	samplesPos[2] = vec2(402, 450);
	samplesPos[3] = vec2(403, 400);
	samplesPos[4] = vec2(400, 431);
	samplesPos[5] = vec2(401, 501);
	samplesPos[6] = vec2(402, 201);
	samplesPos[7] = vec2(403, 701);
	*/
	
	 

	for(int i = 0; i < samples; i++) {		
		//if ( samplesPos[i].x == currPixelSC.x && samplesPos[i].y == currPixelSC.y)  sampled = 1.0;
		
		vec2 currSamplePos = samplesPos[i] / invScrSize;

		float horiz = abs(currSamplePos.x - currPixelSC.x);
		float vert  = abs(currSamplePos.y - currPixelSC.y);
		
		float emitterArea   = gbufGetArea(samplesPos[i]);
		
		float sampleVisSize = 5.0;
		//float sampleVisSize = sqrt(emitterArea/3.14);
		
		if ( horiz <= sampleVisSize && vert <= sampleVisSize ) {	
				vec3 sampled;

				vec3  emitterNormal = gbufGetNormal(samplesPos[i]);
				
				float normalDiff = clamp( dot(receiverNormal, emitterNormal), 0.0, 1.0);

				vec3  emitterPosition = gbufGetPosition(samplesPos[i]);	
				vec3 V = emitterPosition - receiverPosition;
				
				float dist = length(V);
				if (dist > distMax) break;
				if (dist > distMax) {
					sampled = vec3(0);
				} else {
					sampled = vec3(normalDiff, 1-normalDiff, 0.0);
				}
				
				if (horiz <= sampleVisSize-1 && vert <= sampleVisSize-1) {
					//sampled = diskToDiskOcclusion(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
					//sampled = diskToDiskRadiance(emitterDirectDiffuseLighting, emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);

					vec3 emitterDirectDiffuseLighting = calcDirectLight(samplesPos[i]);	

					//sampled = emitterDirectDiffuseLighting;		
					
					//sampled  = d2dr_bunnell(emitterDirectDiffuseLighting, emitterPosition, emitterNormal, 1.0, receiverPosition, receiverNormal, receiverArea);
					//sampled  = d2dr_bunnell(emitterDirectDiffuseLighting, emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
					sampled  = d2dr_dssao(emitterDirectDiffuseLighting, emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
					//sampled  = d2dr_dssao_noArea(emitterDirectDiffuseLighting, emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
					
					//sampled  = vec3( 1 - d2do_bunnell(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea));
					//sampled  = vec3( 1 - d2do_dssao(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea));
					//sampled  = vec3( 1 - d2do_dssao_noArea(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea));
					
					//sampled = vec3( 1 - calcAmbientOcclusion( receiverPosition, receiverNormal, samplesPos[i] ) );
					//sampled = vec3( 1 - bunnell_calcAmbientOcclusion(receiverPosition, receiverNormal, samplesPos[i]) );	
				}
				return sampled;				
		} 
	}
	return vec3(-1.0);
}

vec3 getSampleSquareColor( const in vec2 currPixelSC, const in vec2 mousePixelSC, const in vec2 invScrSize, const in int halfSamplesPerDir, const in float radLength ) {
	return samplingTest(currPixelSC, mousePixelSC, invScrSize, halfSamplesPerDir, radLength);
}

vec3 getNonSampleSquareColor(const in vec2 coords)  {
	//return vec3(gbufGetDepth(coords));
	//return gbufGetNormal(coords);	
	//return gbufGetDiffuseColor(coords);	
	//return vec3(gbufGetArea(coords));	
	return calcDirectLight(coords);
	//return vec3(1.0, 0.0, 0.0);
}

void main() {
	
	vec2 currPixelSC  = vec2(gl_FragCoord.x, gl_FragCoord.y);
	vec2 mousePixelSC = vec2(mousePos.x, mousePos.y);
	
	vec2 currPixelTC  = vec2(gl_FragCoord.x * invScreenSize.x, gl_FragCoord.y * invScreenSize.y);

	//FragColor = vec4(calcDirectLight(currPixelTC), 1.0);
	//return;
			
	const ivec2 zoomSize = ivec2(500, 400);

	//int zoomX = int((1/invScreenSize.x)-zoomSize.x) ;
	//int zoomY = int((1/invScreenSize.y)-zoomSize.y) ;
	int zoomX = int( screenSize.x - zoomSize.x ) ;
	int zoomY = int( screenSize.y - zoomSize.y ) ;
	
	ivec2 zoomZero  = ivec2(zoomX, zoomY);
	vec2  zoomCenter = vec2(zoomZero) + zoomSize*0.5;

	int zoomXoffset = int(gl_FragCoord.x - zoomZero.x);
	int zoomYoffset = int(gl_FragCoord.y - zoomZero.y);
	
	if (zoomXoffset > 0 && zoomYoffset > 0) { // pixel in zoom area
		
		currPixelSC.x = mousePos.x + (gl_FragCoord.x - float(zoomZero.x))*0.5 - zoomSize.x*0.25;
		currPixelSC.y = mousePos.y + (gl_FragCoord.y - float(zoomZero.y))*0.5 - zoomSize.y*0.25;
	
		vec3 sampled = getSampleSquareColor(currPixelSC, mousePixelSC, invScreenSize, aoSamples, radLength);		
		
		vec3 nonsampled = getNonSampleSquareColor(currPixelSC*invScreenSize);

		//FragColor = vec4(nonsampled,1.0);
		if (sampled == vec3(-1)) {
			FragColor = vec4(nonsampled, 1.0);	
		} else {
			FragColor = vec4(sampled, 1.0);	
		}
		
		/*
		vec3 sampled = getSampleSquareColor(currPixelSC, mousePixelSC, invScreenSize, aoSamples, radLength);		
		if (sampled == vec3(-1)) {
			sampled = getNonSampleSquareColor(currPixelSC*invScreenSize);			
		}
		//sampled = getNonSampleSquareColor(currPixelSC*invScreenSize);		
		FragColor = vec4(sampled, 1.0);			
		*/

	} else {
		
		vec3 sampled = getSampleSquareColor( currPixelSC, mousePixelSC, invScreenSize, aoSamples, radLength);	
		if (sampled == vec3(-1)) discard;
		FragColor = vec4(sampled, 1.0);		
		
		//FragColor = vec4(calcDirectLight(currPixelSC*invScreenSize), 1.0);
	}
}

