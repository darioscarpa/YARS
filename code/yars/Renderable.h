#ifndef RENDERABLE_H
#define RENDERABLE_H

// OPENGL
#include "util/inc_opengl.h"

#include <string>

class Renderer;

class Renderable {
public:
	virtual void init() = 0;
	virtual void destroy() = 0;
	virtual void render (Renderer *boundRenderer, bool withMaterials = true) = 0;
	
	inline const std::string& getLabel() const { return label; }
	inline void setLabel(const std::string& l) { label = l; }

	// shaders/renderable objects binding
	// allows using the same VAOs with shaders using different data

	// vertex data
	static const GLuint v_positionLoc  = 0;
	static const GLuint v_normalLoc    = 1;
	static const GLuint v_tangentLoc   = 4;
	static const GLuint v_bitangentLoc = 2;
	static const GLuint v_uvLoc        = 3;

	// texture units
	static const GLuint tu_diffuse     = 0;
	static const GLuint tu_normalMap   = 1;
	static const GLuint tu_specularMap = 2;
	static const GLuint tu_opacityMap  = 3;

	//static const GLuint tu_random      = 20;

	// uniform buffer objects
	static const GLuint ubo_matrices   = 1;
	static const GLuint ubo_material   = 2;
	static const GLuint ubo_lights     = 3;
	static const GLuint ubo_deferred   = 4;

private:
	std::string label;
};

#endif