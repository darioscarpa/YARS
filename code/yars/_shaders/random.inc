#ifndef RANDOM_INC
#define RANDOM_INC

//////////////////////////////////////////////////////////////////

#include noise.inc


///////////// random gen //////////////
float rand(const in vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float random( const in vec2 p ) {
  // We need irrationals for pseudo randomness.
  // Most (all?) known transcendental numbers will (generally) work.
  const vec2 r = vec2(
    23.1406926327792690,  // e^pi (Gelfond's constant)
     2.6651441426902251); // 2^sqrt(2) (Gelfond–Schneider constant)
  return fract( cos( mod( 123456789., 1e-7 + 256. * dot(p,r) ) ) );  
}

/*
highp float rand(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}
*/
///////////////////////////////////////

/*

#include subroutinesConfig.inc

uniform	sampler2D randomSampler;

#ifdef USE_SUBROUTINE_RANDOMGENERATOR

subroutine vec2    RandomGenerator(vec2 coords);
subroutine uniform RandomGenerator getRandom;

#endif

#ifdef USE_SUBROUTINE_RANDOMGENERATOR
subroutine (RandomGenerator)
#endif
vec2 getRandomFixedValue(vec2 coords) {
	return vec2(1,1);
}

#ifdef USE_SUBROUTINE_RANDOMGENERATOR
subroutine (RandomGenerator)
#endif
vec2 getRandomByNoiseFunc(vec2 coords) {
	//return vec2(1,1);
	//return getRandomByNoiseFunc(coords);
	float r = snoise(coords);	
	return vec2(r, r);
}


#ifdef USE_SUBROUTINE_RANDOMGENERATOR
subroutine (RandomGenerator)
#endif
vec2 getRandomByTexture(vec2 coords) {
	//return vec2(1,1);
	vec2 rvec = texelFetch(randomSampler, ivec2(mod(gl_FragCoord.x, 128), mod(gl_FragCoord.y, 128)), 0).xy * 2.0 - 1.0;	
	return normalize(rvec); 
}

#ifdef USE_SUBROUTINE_RANDOMGENERATOR
subroutine (RandomGenerator)
#endif
vec2 getRandomByPseudoRandFunc(vec2 coords) {
	//return vec2(1,1);
	//return getRandomByPseudoRandFunc(coords);
	float rx = rand(coords);	
	float ry = rand(vec2(coords.y, coords.x));
	return normalize(vec2(rx,ry));
}


#ifndef USE_SUBROUTINE_RANDOMGENERATOR


uniform int getRandomSelector;

vec2 getRandom(vec2 coords) {
	switch(getRandomSelector) {
		case 0: return getRandomFixedValue(coords);		        
		case 1: return getRandomByNoiseFunc(coords);				
		case 2: return getRandomByTexture(coords);				
		case 3: return getRandomByPseudoRandFunc(coords);			    
	}		
}

#endif

*/

//////////////////////////////////////////////////////////////////


vec2 getRandom(const in vec2 coords) {
	float rx = rand(coords);	
	float ry = rand(vec2(coords.y, coords.x));
	return normalize(vec2(rx,ry));
}

vec3 getRandom3D(const in vec2 coords) {
	float rx = rand(coords);	
	float ry = rand(vec2(coords.y, coords.x));
	float rz = rand(vec2(rx, ry));
	return normalize(vec3(rx,ry,rz));
}

/*
vec2 getRandom(const in vec2 coords) {
	//return getRandomFixedValue(coords);
	//return getRandomFixedValue(coords);
	//return getRandomByTexture(coords);
	return getRandomByPseudoRandFunc(coords);
}
*/

//////////////////////////////////////////////////////////////////
#endif
