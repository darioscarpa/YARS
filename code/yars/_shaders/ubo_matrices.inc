#ifndef UBO_MATRICES_INC
#define UBO_MATRICES_INC

//////////////////////////////////////////////////////////////////

layout (std140) uniform Matrices {
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 modelViewMatrix;
	mat4 mvpMatrix;
	mat4 invProjMatrix;
	mat4 normalMatrix;
};

//////////////////////////////////////////////////////////////////
#endif
