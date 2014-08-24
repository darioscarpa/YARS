#ifndef BLUR_INC
#define BLUR_INC

#version 420

#include ubo_matrices.inc
#include ubo_deferred.inc

#include gdlbuffer.inc
#include aobuffer.inc

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

gdlbufData currPixelData;

vec4 bilateralBlurDir(const in vec2 texCoord, 
					//mat4 near, 
					//mat4 far, 
					const in vec2 blurDir, 
					const in float depth,
					const in vec3 normal,
					const in float kd, 
					const in float kn) {

	//vec4  data  = texture2D( gbufSampler , texCoord );
	//float depth = length (ReadViewPos_1( data, texCoord, near, far )) ;
	
	//vec3 normal = ReadViewNormal_4 ( data ) ;
	
	//float depth = gdlbufGetDepth(texCoord);		
	//vec3 normal = gdlbufGetNormal(texCoord); 
	
	vec4 color  = vec4(0.0) ;

	float weightSum = 0.0 ;
	for ( int i = -3; i <= 3 ; i++) {
		vec2 tc = texCoord + ( invScreenSize * float(i) ) ;
		
		//vec4  sampleColor  = texture2D(PixelColor_6_4_5 , tc);
		vec4  sampleColor  = aobufGet(tc);
		
		//vec4  sampleData   = texture2D(gbufSampler, tc);		
		//float sampleDepth  = length(ReadViewPos_1 (sampleData , tc , near , far ) );
		//vec3  sampleNormal = ReadViewNormal_4(sampleData);

		float sampleDepth  = gdlbufGetDepth(tc);	
		vec3  sampleNormal = gdlbufGetNormal(tc);
				
		float weight =
			pow( 1.0 / ( 1.0 + abs(depth-sampleDepth) ) , kd ) *
			pow( ( dot(normal, sampleNormal) * 0.5 ) + 0.5, kn ) ;
		
		color = color + ( sampleColor * weight ) ;
		weightSum = weightSum + weight ;
	}
	color = color / weightSum ;
	return color ;
}



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
		
			vec4  sampleColor  = aobufGet(tc);		
			float sampleDepth  = gdlbufGetDepth(tc);	
			vec3  sampleNormal = gdlbufGetNormal(tc);
				
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

void main() {
	vec2 coords = gl_FragCoord.xy * invScreenSize;	
	
	gdlbufGet(coords, currPixelData);	
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

	if (albedoEnabled)      albedo = currPixelData.albedo;
	if (directLightEnabled) directLight = currPixelData.dl;
	
	vec4 aoData = aobufGet(coords);
	if (blurEnabled) aoData = bilateralBlur(coords, currPixelData.depth, currPixelData.normal, kdepth, knormal);
	if (aoEnabled) scaledAmbientOcclusion = aoData.a;
	if (ilEnabled) scaledIndirectLight    = aoData.rgb;
	
	if (ambientLightEnabled) ambientLight  = albedo * ambientLightLevel * scaledAmbientOcclusion;	
	vec3 color = ambientLight + directLight + scaledIndirectLight;
	blurredAo = vec4(color, 1.0);
	///////////////////////////

	/*
	vec2 blurdir = vec2(1.0, 0.0);
	blurredAo = bilateralBlur(coords, blurdir, currPixelData.depth, currPixelData.normal, kdepth, knormal);
	blurdir = vec2(0.0, 1.0);
	blurredAo += bilateralBlur(coords, blurdir, currPixelData.depth, currPixelData.normal, kdepth, knormal);
	blurredAo.a = 1.0;
	*/
	
	/*
	blurredAo = aobufGet(coords);
	blurredAo = bilateralBlur(coords, currPixelData.depth, currPixelData.normal, kdepth, knormal);	

	vec3 color = blurredAo.rgb;
	//color *= currPixelData.albedo;
	//color += currPixelData.dl;
	//blurredAo = vec4(color, 1.0);

	*/
	///////////////////////////
	
	//blurredAo = vec4(currPixelData.normal, 1.0);
	
	//blurredAo =  vec4(aobufGet(coords).rgb, 1.0);	
	//blurredAo = vec4( vec3(aobufGetOcclusion(coords)) , 1.0);

	//blurredAo =  vec4(gdlbufGetPosition(coords), 1.0);
	//blurredAo =  vec4(gdlbufGetNormal(coords), 1.0);
	//blurredAo = vec4(1.0, 0.0, 0.0, 0.0);
}

//////////////////////////////////////////////////////////////////


/*

float linearizeDepth(in float depth) {
	float C = projMatrix[2][2];
	float D = projMatrix[3][2];
	float near = D / (C - 1.0);
	float far  = D / (C + 1.0);

	return (2.0 * near) / (far + near - depth * (far - near));
} 


vec3 ToViewSpace( vec3 imagePosition, mat4 near, mat4 far ) {
	vec3 farTL = far[0].xyz ;
	vec3 farTR = far[1].xyz ;
	vec3 farBL = far[2].xyz ;
	vec3 farBR = far[3].xyz ;
	vec3 nearTL = near[0].xyz ;
	vec3 nearTR = near[1].xyz ;
	vec3 nearBL = near[2].xyz ;
	vec3 nearBR = near[3].xyz ;
	float farZ = farTL.z ;
	float nearZ = nearTL.z ;
	float ratio = nearZ / farZ ;
	float linearDepth = imagePosition.z ;
	float linearDist = ( ( 1.0 - ratio ) * linearDepth ) + ratio;
	vec2 farDiag = farTR.xy - farBL.xy;
	vec3 farPos  = farBL + vec3 ( farDiag * imagePosition.xy, 0.0 ) ;
	return farPos * linearDist;
}

float ReadLinearDepth_1 ( sampler2D depthBuffer , vec2 texCoord ) {
	vec4 PackedDepth = texture2D ( depthBuffer , texCoord ) ;
	return UnpackFloat_2 (PackedDepth ) ;
}

vec3 ReadViewPos_1( vec4 packedData, vec2 texCoord, mat4 near, mat4 far) {
	float linearDepth = UnpackFloat16_2(packedData.zw);
	return ToViewSpace_3 ( vec3(texCoord, linearDepth ), near, far);
}

*/

/*
vec4 bilateralFilter() {
	vec4 color  = vec4(0.0);
	vec2 center = gl_TexCoord[0].xy;
	vec2 sample;
	float sum = 0.0;
	float coefG, coefZ, finalCoef;
	float Zp = getDepth(center);

	const float epsilon = 0.01;
		
	for(int i = -(kernelSize-1)/2; i <= (kernelSize-1)/2; i++) {
		for(int j = -(kernelSize-1)/2; j <= (kernelSize-1)/2; j++) {
			sample = center + vec2(i,j) / texSize;
			coefG = gaussianCoef(i,j);
			float zTmp = getDepth(sample);
			coefZ = 1.0 / (epsilon + abs(Zp - zTmp));
			finalCoef = coefG * coefZ;
			sum += finalCoef;
			color += finalCoef * texture2D(aoSampler, sample);
		}
	}

	return color / sum;
} 


vec4 bilateralFilter2(vec2 center, float Zp, float occlusionValue) {
	vec4 color  = vec4(0.0);
	//vec2 center = gl_TexCoord[0].xy;
	vec2 sample;
	float sum = 0.0;
	float coefG, coefZ, finalCoef;
	//float Zp = getDepth(center);

	const float epsilon = 0.01;

	const int kernelSize = 4;
			
	for(int i = -(kernelSize-1)/2; i <= (kernelSize-1)/2; i++) {
		for(int j = -(kernelSize-1)/2; j <= (kernelSize-1)/2; j++) {
			sample = center + vec2(i,j) * invScreenSize; // / texSize;
			coefG = gaussianCoef(i,j);
			
			float zTmp = getDepth(sample);
			coefZ = 1.0 / (epsilon + abs(Zp - zTmp));
			
			finalCoef = coefG * coefZ;
			
			sum   += finalCoef;
			color += finalCoef * texture2D(aoSampler,sample);
		}
	}

	return color / sum;
} 
*/

#endif