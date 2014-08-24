#ifndef UBO_MATERIALS_INC
#define UBO_MATERIALS_INC

//////////////////////////////////////////////////////////////////

layout (std140) uniform Material {
	vec4   diffuse;
	vec4   ambient;
	vec4   specular;
	vec4   emissive;
	float  shininess;

	int    diffuseTexId;
	int    normalMapId;
	int    specularMapId;
	int    opacityMapId;
};


struct MaterialDesc {
	vec3  diffuse;
	vec3  ambient;
	vec3  specular;
	vec3  emissive;
	float shininess;
};

//////////////////////////////////////////////////////////////////
#endif
