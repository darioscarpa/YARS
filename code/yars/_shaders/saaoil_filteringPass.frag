#ifndef BLUR_INC
#define BLUR_INC

#version 420

#include ubo_matrices.inc
#include ubo_deferred.inc

#include saaoil_gbuffer.inc
#include saaoil_ilaobuffer.inc

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

saaoil_gBufData currPixelData;

vec4 bilateralBlur( const in vec2 texCoord, 
					const in float depth,
					const in vec3 normal,
					const in float kd, 
					const in float kn) {

	vec4 color  = vec4(0.0) ;
	float weightSum = 0.0 ;
	
	for ( int i = -kernelSize; i <= kernelSize ; i++) {
		for ( int j = -kernelSize; j <= kernelSize ; j++) {		
			vec2 tc = texCoord + ( vec2(i, j) * invScreenSize);
		
			vec4  sampleColor  = ilaoBufGet(tc);		
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

void mainrand() {

	vec2 coords = gl_FragCoord.xy * invScreenSize;
	//blurredAo = vec4(getRandom(coords), 0, 1);
	
	//blurredAo = vec4(myrand(coords.xy), myrand(coords.yx), 1.0, 1);


	//blurredAo = vec4(1.0, 1.0, 0, 1);
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
	vec3  scaledIndirectLight     = vec3(0.0) ;
	vec3  ambientLight   = vec3(0.0);
	vec3  directLight    = vec3(0.0);
	vec3  albedo         = vec3(1.0);
	vec3  occludedDirectLight = vec3(0.0);

	gBufFetchNormalArea( coords, currPixelData );
	
	gBufFetchAlbedo( coords, currPixelData );	
	if (albedoEnabled)      albedo = currPixelData.albedo;
	
	gBufFetchDirectLight( coords, currPixelData );
	if (directLightEnabled) directLight = currPixelData.dl;
	
	vec4 ilaoData = ilaoBufGet(coords);
	if (blurEnabled) ilaoData = bilateralBlur(coords, currPixelData.depth, currPixelData.normal, kdepth, knormal);
	if (aoEnabled) scaledAmbientOcclusion = ilaoData.a;
	if (ilEnabled) scaledIndirectLight    = ilaoData.rgb;

	/*if (coords.x + coords.y > 1)  {
		scaledAmbientOcclusion = 1.0;
	}*/

	
	if (ambientLightEnabled) ambientLight  = albedo * ambientLightLevel * scaledAmbientOcclusion;	
	vec3 color = ambientLight + directLight + scaledIndirectLight;
	//color += vec3(drawLine(gl_FragCoord.xy, vec2(0,1) , vec2(1,0), 2));
	blurredAo = vec4(color, 1.0);	
	///////////////////////////	
}

//////////////////////////////////////////////////////////////////

#endif