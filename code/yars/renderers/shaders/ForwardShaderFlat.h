#ifndef FORWARDSHADERFLAT_H
#define FORWARDSHADERFLAT_H

#include "../../ShaderHandling.h"
#include "../../Renderable.h"

class ForwardShaderFlat : public GfxShaderProgram {
public:	
	///////////////////////////////////////////////////////////////////////
	enum UniformBufferObjectId {
		UBO_MATRICES,
		UBO_LIGHTS,
		UBO_SIZE
	};
	
	enum TextureSamplerId { 
		TEX_SIZE
	};
		
	enum UniformLocId { 
		pointLightId,
		SIZE
	};		

	inline GLuint u(UniformLocId i) const { return uniformLocs[i]; }

	///////////////////////////////////////////////////////////////////////
	ForwardShaderFlat()	: GfxShaderProgram("forwardShader_flat", "forward.vert", "forward_flat.frag") {		
		// vertex attribs
		glBindAttribLocation(getId(), Renderable::v_positionLoc,  "position");
		glBindAttribLocation(getId(), Renderable::v_normalLoc,    "normal");
		glBindAttribLocation(getId(), Renderable::v_uvLoc,        "texCoord");
		glBindAttribLocation(getId(), Renderable::v_bitangentLoc, "bitangent");
		
		link();
		
		// UBOs
		uboBindings.resize(UBO_SIZE);
		setUBObinding(UBO_MATRICES,  "Matrices", Renderable::ubo_matrices);
		setUBObinding(UBO_LIGHTS,    "Lights",   Renderable::ubo_lights);

		// TEXTURE SAMPLERS	
		textureSamplers.resize(TEX_SIZE);
						
		// UNIFORMS		
		uniformLocs.resize(UniformLocId::SIZE);
		uniformLocs[pointLightId]        = glGetUniformLocation(getId(), "pointLightId");				
	};
	~ForwardShaderFlat() {};	
	///////////////////////////////////////////////////////////////////////		

	inline void setPointLightId(int id) { setIntUniform(pointLightId, id); }
	
};

#endif
