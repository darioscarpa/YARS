#version 420

#include ubo_matrices.inc
#include ubo_materials.inc
#include ubo_lights.inc
#include ubo_deferred.inc
#include lighting.inc

#include gbuffer.inc

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
uniform int   aoSamples; // half of the samples taken for each of 4 directions (so if aoSamples = 1 -> 8 samples, aoSamples 2 -> 16) : samples = 2*4*aoSamples
uniform float radLength; 
uniform float diskDisplacement;
uniform float distMax;

#include ambientOcclusion.inc
/////////////////////////////////////////////////////////////


vec3 calcDirectLight(const in vec2 coords) {	
	float zvalue = gbufGetDepth(coords);
	if (zvalue == 1.0) {
		discard; //FragColor = vec4(1.0, 0.0, 0.0, 1.0); return;
	}
		
	/////////////////////////////////////////////
	
	vec3 diffuseColor = gbufGetDiffuseColor(coords);

	vec3 specularColor;
	float shininess;
	gbufGetSpecularColor(coords, specularColor, shininess);

	vec3 normalCS = gbufGetNormal(coords);

	vec3 eyeCS = -gbufGetPosition(coords);

	/////////////////////////////////////////////

	/////////////////////////////////////////////		
    vec3 n = normalize(normalCS);
	
    // Clear the light intensity accumulators
    vec3 Diffuse  = vec3(0.0);
    vec3 Specular = vec3(0.0);
	
	/*
	PointLight pl;
	pl.color = vec3(1.0, 1.0, 1.0);
	pl.posCS = vec3(0.0, 0.0, 0.0);				
	pl.constantAttenuation = 1.0;
	pl.linearAttenuation = 0.02;
	pl.quadraticAttenuation = 0.08;	
	
	MaterialDesc mtl;
	mtl.diffuse   = diffuseColor.rgb; 
	mtl.ambient   = vec3(0.0);
	mtl.specular  = specularColor.rgb;
	mtl.emissive  = vec3(0.0);
	mtl.shininess = shininess; 
	*/
	MaterialDesc mtl = MaterialDesc(diffuseColor.rgb, vec3(0.0), specularColor.rgb, vec3(0.0), shininess);

	for (int i = 0 ; i < numPointLights; ++i) {
			vec3 scattered;
			vec3 reflected;
			//pointLight(pl, mtl, n, eyeCS, scattered, reflected);
			pointLight(pointLights[i], mtl, n, eyeCS, scattered, reflected);
			Diffuse  += scattered;
			Specular += reflected;
	}
	
	vec3 color;		
	color = vec3( Diffuse );
	color = clamp( color, 0.0, 1.0 );
	return color;
	//return color*vec3(0.5, 1.0, 0.5);
	//return vec3(0.0, 0.7, 0.0);
}

/////////////////////////////////////////////////////////////


//vec3 calcLin(sample) {
vec3 calcLin(const in vec3 receiverPosition, const in vec3 receiverNormal, const in vec3 emitterPosition, const in vec3 emitterNormal, const float emitterArea, const in vec2 samplesPosTC) {
	return calcDirectLight(samplesPosTC);	
}

/*
//float calcVis(sample) {
float calcVis(in vec3 receiverPosition, in vec3 receiverNormal, in vec3 emitterPosition, in vec3 emitterNormal, float emitterArea, in vec2 samplesPosTC) {
	//return 1.0;
	//return formFactor(V, dist, receiverNormal, emitterNormal, emitterArea);
	return bunnell_calcSolidAngle(V, d2, receiverNormal, emitterNormal, emitterArea);
}
*/


void main() {	
	vec2 coords = vec2(gl_FragCoord.x * invScreenSize.x, gl_FragCoord.y * invScreenSize.y);
	
	float zvalue = gbufGetDepth(coords);
	if (zvalue == 1.0) {
		discard;
	}

	//vec2 samplesPos[64];

	int halfSamplesPerDir = aoSamples;
	
	vec3 receiverPosition = gbufGetPosition(coords);
	vec3 receiverNormal   = gbufGetNormal(coords);	

	calcSamplesPos(coords, halfSamplesPerDir, radLength, receiverPosition, receiverNormal);

	int samples   = halfSamplesPerDir * 2 * 4;
	
	vec3 directLight = vec3(0.0);
	
	for(int i = 0; i < samples; i++) {
		vec2 currSample      = samplesPos[i];
		vec3 emitterPosition = gbufGetPosition(currSample);
		vec3 emitterNormal   = gbufGetNormal(currSample);
		float emitterArea    = gbufGetArea(currSample);
				
		vec3 V = emitterPosition - receiverPosition;
		float d2 = dot(V, V) + 1e-16; //quadrato distanza tra dischi
		V *= inversesqrt(d2); // normalizzazione del vettore tra i dischi
	
		vec3  Lin = calcLin(receiverPosition, receiverNormal, emitterPosition, emitterNormal, emitterArea, currSample);
		
		//float Vis = calcVis(receiverPosition, receiverNormal, emitterPosition, emitterNormal, emitterArea, currSample)/2;

		//float Vis = formFactor(V, d2, receiverNormal, emitterNormal, emitterArea);
		float Vis = formFactor(V, sqrt(d2), receiverNormal, emitterNormal, emitterArea);
		//float Vis = bunnell_calcSolidAngle(V, d2, receiverNormal, emitterNormal, emitterArea);
		
		directLight += Lin * (1-Vis); // * max(0, dot(receiverNormal, emitterNormal));
		//directLight += Lin * Vis; // * max(0, dot(receiverNormal, emitterNormal));
	}	
	FragColor.rgb = directLight/samples;
	//FragColor.rgb = directLight/3.14;
	FragColor.a = 1.0;
}

