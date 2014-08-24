#ifndef UTILS_INC
#define UTILS_INC

vec3 screenSpaceToCamSpace(const in vec2 scrcoords, const in float depth) {
	vec3 scrSpace = vec3(scrcoords * 2.0 - 1.0, depth);	
	vec4 camSpace = invProjMatrix * vec4(scrSpace, 1.0);
	return camSpace.xyz / camSpace.w;
}

#endif