#ifndef DRLIGHTPASSSHADERSPOTLIGHT_H
#define DRLIGHTPASSSHADERSPOTLIGHT_H

#include "../../ShaderHandling.h"
#include "../../Renderable.h"

#include "../gBuffer.h"

class DRlightPassShaderSpotLight : public GfxShaderProgram {
	
public:
	///////////////////////////////////////////////////////////////////////
	enum UniformLocId {
		spotLightId,
		invScreenSize,
		SIZE
	};
	enum TextureSamplerId { 
		TEX_ALBEDO,
		TEX_NORMAL,
		TEX_SPECULAR,
		TEX_DEPTH,
		TEX_SIZE
	};
	enum UniformBufferObjectId { 
		UBO_MATRICES,
		UBO_LIGHTS,	
		UBO_SCRSIZE,
		UBO_SIZE
	};
	
	inline GLuint u(UniformLocId i) const { return uniformLocs[i]; }
	
	///////////////////////////////////////////////////////////////////////
	DRlightPassShaderSpotLight() : GfxShaderProgram("d_lightPassSpotLight", "deferred_fullscreenquad.vert", "deferred_lightpass_spotlight.frag") {		
		// input
		glBindAttribLocation(getId(), Renderable::v_positionLoc,  "positionNDC");
		
		link();

		// UBOs		
		uboBindings.resize(UBO_SIZE);
		setUBObinding(UBO_MATRICES,  "Matrices",  Renderable::ubo_matrices);
		setUBObinding(UBO_LIGHTS,    "Lights",    Renderable::ubo_lights);
		setUBObinding(UBO_SCRSIZE,   "Deferred",  Renderable::ubo_deferred);
		//setUBObinding(UBO_MATERIALS, "Material",  Renderable::ubo_material);
						
		// TEXTURE SAMPLERS
		textureSamplers.resize(TEX_SIZE);
		setTextureSampler(TEX_ALBEDO,   "diffuseSampler",  GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
		setTextureSampler(TEX_NORMAL,   "normalSampler",   GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
		setTextureSampler(TEX_SPECULAR, "specularSampler", GBuffer::GBUFFER_TEXTURE_TYPE_SPECULAR);
		setTextureSampler(TEX_DEPTH,    "depthSampler",    GBuffer::GBUFFER_TEXTURE_TYPE_DEPTH);
							
		// UNIFORMS
		uniformLocs.resize(UniformLocId::SIZE);
		uniformLocs[spotLightId]   = glGetUniformLocation(getId(), "spotLightId");
		uniformLocs[invScreenSize] = glGetUniformLocation(getId(), "invScreenSize");
		
	};
	~DRlightPassShaderSpotLight() {};
	///////////////////////////////////////////////////////////////////////	
	inline void setSpotLightId(int id)               { setIntUniform(spotLightId, id); };	
};

#endif
