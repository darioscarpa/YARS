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
uniform bool  aoOnly;
uniform bool  aoEnabled;
uniform int   aoSamples; // half of the samples taken for each of 4 directions (so if aoSamples = 1 -> 8 samples, aoSamples 2 -> 16) : samples = 2*4*aoSamples
uniform float radLength; 
uniform float diskDisplacement;
uniform float distMax;

//////////////////////////

#include ambientOcclusion.inc

/////////////////////////////////////////////////////////////

void main_zpos() {
	vec2 coords = vec2(gl_FragCoord.x * invScreenSize.x, gl_FragCoord.y * invScreenSize.y);	
	
	float depth = gbufGetDepth(coords);
	FragColor.rgb = vec3(depth); 
	FragColor.a   = 1.0;
}

void main_pos() {
	vec2 coords = vec2(gl_FragCoord.x * invScreenSize.x, gl_FragCoord.y * invScreenSize.y);	
	FragColor.rgb = gbufGetPosition(coords);	
}

void main_random() {
	//vec2 coords = vec2(gl_FragCoord.x * invScreenSize.x, gl_FragCoord.y * invScreenSize.y);		
	//FragColor.rg = getRandom(coords);

	vec2 coords = vec2(gl_FragCoord.x, gl_FragCoord.y);		
	FragColor.rg = getRandom(coords);
}

void main_area() {
	vec2 coords = vec2(gl_FragCoord.x * invScreenSize.x, gl_FragCoord.y * invScreenSize.y);
	//FragColor = vec4(vec3(gbufGetArea(coords)/3.14), 1.0);
	FragColor = vec4(vec3(gbufGetArea(coords)), 1.0);
}

/////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////
float calcAmbientOcclusionSampleContribution(const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea, const in vec2 samplePosSS) {
	vec3  emitterPosition = gbufGetPosition(samplePosSS);
	vec3  emitterNormal   = gbufGetNormal(samplePosSS);
	float emitterArea     = gbufGetArea(samplePosSS);

	return diskToDiskOcclusion(emitterPosition, emitterNormal, emitterArea, receiverPosition, receiverNormal, receiverArea);
}


float ambientOcclusion(const in vec2 pixelPos, const in int halfSamplesPerDir) {
	//vec2 samplesPos[64]; //global
	
	vec3  receiverPosition = gbufGetPosition(pixelPos);
	vec3  receiverNormal   = gbufGetNormal(pixelPos);	
	float receiverArea     = gbufGetArea(pixelPos);	

	calcSamplesPos(pixelPos, halfSamplesPerDir, radLength, receiverPosition, receiverNormal);

	int samples   = halfSamplesPerDir * 2 * 4;
	
	float ao = 0.0;
	for(int i = 0; i < samples; i++) {
		ao +=  calcAmbientOcclusionSampleContribution(receiverPosition, receiverNormal, receiverArea, samplesPos[i]);		
	}

	ao = ao / samples;	//compute average	
	return ao;
}

void ambientPass() {	
	vec2 coords = vec2(gl_FragCoord.x * invScreenSize.x, gl_FragCoord.y * invScreenSize.y);

	float zvalue = gbufGetDepth(coords);
	if (zvalue == 1.0) {
		discard;
	}

	/////////////////////////////////////////////
	    
    // calculate the ambient factor
	float ambientFactor  = ambientLightLevel;
	
	float dssao    = 0.0;
	if (aoEnabled) {
		dssao          = 1 - ambientOcclusion(coords, aoSamples);		
		ambientFactor  = ambientFactor * dssao;
	}

	/////////////////////////////////////////////
	if (aoOnly) {
		FragColor = vec4(vec3(dssao), 1.0);
	} else {
		vec3 ambientColor = gbufGetDiffuseColor(coords);	
		vec3 color        = ambientColor * ambientFactor;
		color = clamp( color, 0.0, 1.0 );
		FragColor = vec4(color, 1.0);
	}    
}

void main() {
	//main_area();
	//main_pos();
	//main_zpos();
	//main_random(); 	
	ambientPass(); 
}

