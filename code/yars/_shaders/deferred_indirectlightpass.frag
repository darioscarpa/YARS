#version 420

#include ubo_matrices.inc
#include ubo_materials.inc
#include ubo_lights.inc
#include ubo_deferred.inc
#include lighting.inc

#include gbuffer.inc

uniform float ambientLightLevel;

/////////////////////////////////////////////////////
// input

// all input is taken by texture sampling

/////////////////////////////////////////////////////
// output

layout(location = 0) out vec4 FragColor;

/////////////////////////////////////////////////////

// AO params
uniform int   aoSamples; // half of the samples taken for each of 4 directions (so if aoSamples = 1 -> 8 samples, aoSamples 2 -> 16) : samples = 2*4*aoSamples
uniform float radLength; 
uniform float diskDisplacement;
uniform float distMax;

#include deferred_lighting.inc
#include indirectLighting.inc
/////////////////////////////////////////////////////////////

vec3 calcIndirectLightSampleContribution(const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea, const in vec2 samplePosSS) {
	vec3  emitterPosition = gbufGetPosition(samplePosSS);
	vec3  emitterNormal   = gbufGetNormal(samplePosSS);
	float emitterArea     = gbufGetArea(samplePosSS);
	vec3  emitterDirectDiffuseLighting =  calcDirectLight(samplePosSS);

	return diskToDiskRadiance(emitterDirectDiffuseLighting, emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
}

vec3 indirectLight(const in vec2 pixelPos, const in int halfSamplesPerDir) {
	//vec2 samplesPos[64]; //global
			
	vec3  receiverPosition = gbufGetPosition(pixelPos);
	vec3  receiverNormal   = gbufGetNormal(pixelPos);	
	float receiverArea     = gbufGetArea(pixelPos);	

	calcSamplesPos(pixelPos, halfSamplesPerDir, radLength, receiverPosition, receiverNormal);
	int samples   = halfSamplesPerDir * 2 * 4;
	
	vec3 indirectLight = vec3(0.0);
			
	for(int i = 0; i < samples; i++) {
		vec3 contrib =  calcIndirectLightSampleContribution(receiverPosition, receiverNormal, receiverArea, samplesPos[i]);
		indirectLight += contrib;	
	}
	indirectLight = indirectLight / samples;
	
	return indirectLight;
}

void indirectLightPass() {	
	vec2 coords = vec2(gl_FragCoord.x * invScreenSize.x, gl_FragCoord.y * invScreenSize.y);
	
	float zvalue = gbufGetDepth(coords);
	if (zvalue == 1.0) {
		discard;
	}	
	/////////////////////////////////////////////
	vec3 indLight = indirectLight(coords, aoSamples);
	FragColor = vec4(indLight, 1.0);	
}

void main() {	
	/*vec2 coords = vec2(gl_FragCoord.x * invScreenSize.x, gl_FragCoord.y * invScreenSize.y);
	
	float zvalue = gbufGetDepth(coords);
	if (zvalue == 1.0) {
		discard;
	}

	FragColor.rgb = calcDirectLight(coords);
	FragColor.a = 1.0;
	*/
	
	indirectLightPass();
}



//////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////
/*

vec3 bunnell_calcIndirectLight(vec3 receiverPosition, vec3 receiverNormal, vec2 samplePosSS) {
	vec3 emitterPosition = gbufGetPosition(samplePosSS);
	vec3 emitterNormal   = gbufGetNormal(samplePosSS);
	float emitterArea    = gbufGetArea(samplePosSS);

	vec3 directLight = calcDirectLight(samplePosSS);

	// V is the vector from receiver disk to emitter disk
	vec3 V = emitterPosition - receiverPosition;
	float d2 = dot(V, V) + 1e-16;
	V *= inversesqrt(d2);
	
	//return directLight*bunnell_calcFormFactor(V, d2, receiverNormal, emitterNormal, emitterArea/3.14);	
	//return directLight*bunnell_calcFormFactor(V, d2, receiverNormal, emitterNormal, emitterArea);	
	//return vec3(0.1)+10*directLight*bunnell_calcFormFactor(V, d2, receiverNormal, emitterNormal, emitterArea/3.14);	
	//return directLight;	
	return directLight*bunnell_calcFormFactor(V, d2, receiverNormal, emitterNormal, emitterArea);	
	
}


vec3 calcIndirectLight(vec3 receiverPosition, vec3 receiverNormal, vec2 samplePosSS)  {
	vec3 emitterPosition = gbufGetPosition(samplePosSS);
	vec3 emitterNormal   = gbufGetNormal(samplePosSS);
	float emitterArea    = gbufGetArea(samplePosSS);

	// V is the vector from receiver disk to emitter disk
	vec3 V = emitterPosition - receiverPosition;
	float dist = length(V);
	if (dist > distMax) return vec3(0.0f);
	//return formFactor(V, dist, receiverNormal, emitterNormal, emitterArea);

	vec3 directLight = calcDirectLight(samplePosSS);
	return directLight*formFactor(V, dist, receiverNormal, emitterNormal, emitterArea);		
}

*/
