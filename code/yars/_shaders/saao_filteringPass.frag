#ifndef SAAO_FILTERINGPASS_INC
#define SAAO_FILTERINGPASS_INC

#version 420

#include ubo_matrices.inc
#include ubo_materials.inc
#include ubo_lights.inc
#include ubo_deferred.inc
#include lighting.inc

#include utils.inc

#include saao_gbuffer.inc
#include saao_aobuffer.inc

//////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// input

// by texture sampling

/////////////////////////////////////////////////////
// output

layout(location = 0) out vec4 blurredAo;

/////////////////////////////////////////////////////

uniform int kernelSize;
uniform float kdepth, knormal;

// debugging/compositing
uniform float ambientLightLevel;
uniform bool albedoEnabled;
uniform bool ambientLightEnabled;
uniform bool directLightEnabled;

uniform bool doEnabled;
uniform bool aoEnabled;
uniform bool ilEnabled;

uniform bool blurEnabled;


/////////////////////////////////////////////////////

saao_gBufData currPixelData;

float bilateralBlur( const in vec2 texCoord, 
					const in float depth,
					const in vec3 normal,
					const in float kd, 
					const in float kn) {

	float color  = 0.0 ;
	float weightSum = 0.0 ;
	
	for ( int i = -kernelSize; i <= kernelSize ; i++) {
		for ( int j = -kernelSize; j <= kernelSize ; j++) {		
			vec2 tc = texCoord + ( vec2(i, j) * invScreenSize);
		
			float sampleColor = aoBufGetOcclusion(tc);
			float sampleDepth  = gBufGetDepth(tc);	
			vec3  sampleNormal = gBufGetNormal(tc);
				
			float weight =
				pow( 1.0 / ( 1.0 + abs(depth-sampleDepth) ) , kd ) *
				pow( ( dot(normal, sampleNormal) * 0.5 ) + 0.5, kn ) ;
		
			color = color + ( sampleColor * weight ) ;
			weightSum = weightSum + weight ;
		}
	}
	color = color / weightSum ;
	return color ;
}

#include draw.inc

void main() {
	vec2 coords = gl_FragCoord.xy * invScreenSize;
	
	gBufFetchDepth(coords, currPixelData);
	
	if (currPixelData.depth == 1.0) {
		discard;
	}
	
	///////////////////////////
	float scaledAmbientOcclusion  = 1.0 ;
	vec3  ambientLight   = vec3(0.0);
	vec3  directLight    = vec3(0.0);
	vec3  albedo         = vec3(1.0);
			
	gBufFetchNormalArea( coords, currPixelData );
	gBufFetchDiffuse( coords, currPixelData );
	
	if (albedoEnabled)      albedo = currPixelData.diffuse;
			
	if (directLightEnabled) {
		gBufFetchSpecular( coords, currPixelData );						
		MaterialDesc mtl = MaterialDesc(currPixelData.diffuse, vec3(0.0), currPixelData.specular, vec3(0.0), currPixelData.shininess);
		//MaterialDesc mtl = MaterialDesc(vec3(1,0,0), vec3(0.0), vec3(0), vec3(0.0), 0);
		
		/////////////////////////////////////////////
		vec3 scattered = vec3(0);
		vec3 reflected = vec3(0);

		vec3 eyeCS = -screenSpaceToCamSpace(coords, currPixelData.depth);
		calcAllDirectLighting(mtl, currPixelData.normal, eyeCS, scattered, reflected);

		directLight = scattered + reflected;
		//directLight = scattered;
		//directLight = reflected;
	}
			
	if (ambientLightEnabled) {
		if (aoEnabled) {
			float aoData = aoBufGetOcclusion(coords);				
			if (blurEnabled) {
				aoData = bilateralBlur(coords, currPixelData.depth, currPixelData.normal, kdepth, knormal);
			}
			scaledAmbientOcclusion = aoData;
		}		
		ambientLight  = albedo * ambientLightLevel * scaledAmbientOcclusion;	
	}
	vec3 color = ambientLight + directLight;
	//color += vec3(drawLine(gl_FragCoord.xy, vec2(0,1) , vec2(1,0), 2));	
	blurredAo = vec4(color, 1.0);
	//blurredAo = vec4(ambientLight + directLight, 1.0);
	///////////////////////////	
}


void nobranchmain() {
	vec2 coords = gl_FragCoord.xy * invScreenSize;
	
	gBufFetchDepth(coords, currPixelData);
	
	if (currPixelData.depth == 1.0) {
		discard;
	}
	
	///////////////////////////
	float scaledAmbientOcclusion  = 1.0 ;
	vec3  ambientLight   = vec3(0.0);
	vec3  directLight    = vec3(0.0);
	vec3  albedo         = vec3(1.0);
			
	gBufFetchDiffuse( coords, currPixelData );
	
	albedo = currPixelData.diffuse;
	
	
	gBufFetchNormalArea( coords, currPixelData );
	
	gBufFetchSpecular( coords, currPixelData );						
	MaterialDesc mtl = MaterialDesc(currPixelData.diffuse, vec3(0.0), currPixelData.specular, vec3(0.0), currPixelData.shininess);
	
	/////////////////////////////////////////////
	vec3 scattered = vec3(0);
	vec3 reflected = vec3(0);

	vec3 eyeCS = -screenSpaceToCamSpace(coords, currPixelData.depth);
	calcAllDirectLighting(mtl, currPixelData.normal, eyeCS, scattered, reflected);

	directLight = scattered + reflected;
	
	float aoData = aoBufGetOcclusion(coords);				
	aoData = bilateralBlur(coords, currPixelData.depth, currPixelData.normal, kdepth, knormal);
	scaledAmbientOcclusion = aoData;
	ambientLight  = albedo * ambientLightLevel * scaledAmbientOcclusion;	
	
	vec3 color = ambientLight + directLight;
	blurredAo = vec4(color, 1.0);
	///////////////////////////	
}
//////////////////////////////////////////////////////////////////


void xxxmain() {
	vec2 coords = gl_FragCoord.xy * invScreenSize;
	
	gBufFetchDepth(coords, currPixelData);
	gBufFetchNormalArea( coords, currPixelData );
	gBufFetchDiffuse( coords, currPixelData );
	
	gBufFetchSpecular( coords, currPixelData );						

	MaterialDesc mtl = MaterialDesc(currPixelData.diffuse, vec3(0.0), currPixelData.specular, vec3(0.0), currPixelData.shininess);
	//MaterialDesc mtl = MaterialDesc(vec3(1,0,0), vec3(0.0), vec3(0), vec3(0.0), 0);
		
	/////////////////////////////////////////////
	vec3 scattered = vec3(0);
	vec3 reflected = vec3(0);

	vec3 eyeCS = -screenSpaceToCamSpace(coords, currPixelData.depth);
	calcAllDirectLighting(mtl, currPixelData.normal, eyeCS, scattered, reflected);

	blurredAo = vec4(scattered + reflected, 1.0);
}

void testmain() {
	currPixelData = saao_gBufData(vec3(0), vec3(0), 0, vec3(0), 0, 0, 0, vec3(0));
	
	vec2 coords = gl_FragCoord.xy * invScreenSize;
	
	gBufFetchDepth(coords, currPixelData);
	
	if (currPixelData.depth == 1.0) {
		discard;
	}
	
	gBufFetchDiffuse( coords, currPixelData );
	gBufFetchNormalArea( coords, currPixelData );
	gBufFetchSpecular( coords, currPixelData );	
	
	//float aoData = aoBufGetOcclusion(coords);	
	
	//blurredAo = vec4(currPixelData.normal, 1.0);
	//blurredAo = vec4(vec3(currPixelData.area), 1.0);
	//blurredAo = vec4(vec3(aoData), 1.0);
	//blurredAo = vec4(currPixelData.diffuse, 1.0);
	//blurredAo = vec4(currPixelData.specular, 1.0);
	
	blurredAo = vec4(vec3(currPixelData.depth), 1.0);
	//blurredAo = texture(diffuseSampler, coords);

	//blurredAo = vec4(gBufGetDiffuseColor(coords), 1.0);
	
}
#endif