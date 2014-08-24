#ifndef UBO_LIGHTS_INC
#define UBO_LIGHTS_INC

//////////////////////////////////////////////////////////////////

struct DirLight {
	vec3 color;
	float pad1;

	vec3 dirCS;		
	float pad2;
};

struct PointLight {
	vec3 color;
	float pad1;
	
	vec3 posCS;				
	float pad2;

	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;	
	float pad3;

};

struct SpotLight {
	vec3 color;
	float pad1;

	vec3 posCS;	
	float cosCutoffOuter;

	vec3 dirCS;	
	//float cutoff;
	float cosCutoffInner;

	float exponent;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;	
};

const int POINT_LIGHTS_MAX = 5;
const int SPOT_LIGHTS_MAX  = 5;
const int DIR_LIGHTS_MAX   = 2;

layout(std140) uniform Lights {
    float      numLights;
	float      numPointLights;
	float      numSpotLights;
	float      numDirLights;

	PointLight pointLights[POINT_LIGHTS_MAX];
	SpotLight  spotLights[SPOT_LIGHTS_MAX];
	DirLight   dirLights[DIR_LIGHTS_MAX];	
};

/*
layout(std140) uniform Lights {
	PointLight pointLights[POINT_LIGHTS_MAX];	
	//SpotLight  spotLights[SPOT_LIGHTS_MAX];
	//DirLight   directionalLights[DIR_LIGHTS_MAX];
};
*/

//////////////////////////////////////////////////////////////////
#endif
