#ifndef DRDIRECTLIGHTPASSSHADER_H
#define DRDIRECTLIGHTPASSSHADER_H

#include "../gBuffer.h"
#include "../DlBuffer.h"

class DRdirectLightPassShader : public GfxShaderProgram {
	
public:
	///////////////////////////////////////////////////////////////////////
	enum UniformLocId {
		UNI_SIZE
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
	DRdirectLightPassShader() : GfxShaderProgram("d_directLightPass", "deferred_fullscreenquad.vert", "deferred_directlightpass.frag") {		
		// input
		glBindAttribLocation(getId(), Renderable::v_positionLoc,  "positionNDC");
		
		// output
		glBindFragDataLocation(getId(), DlBuffer::DLBUFFER_TEXTURE_TYPE_DIFFUSE,  "ScatteredFragColor");
		glBindFragDataLocation(getId(), DlBuffer::DLBUFFER_TEXTURE_TYPE_SPECULAR, "ReflectedFragColor");
		
		link();

		// UBOs		
		uboBindings.resize(UBO_SIZE);
		setUBObinding(UBO_MATRICES, "Matrices",  Renderable::ubo_matrices);
		setUBObinding(UBO_LIGHTS,   "Lights",    Renderable::ubo_lights);
		setUBObinding(UBO_SCRSIZE,  "Deferred",  Renderable::ubo_deferred);		
						
		// TEXTURE SAMPLERS
		textureSamplers.resize(TEX_SIZE);
		setTextureSampler(TEX_ALBEDO,   "diffuseSampler",  GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
		setTextureSampler(TEX_NORMAL,   "normalSampler",   GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
		setTextureSampler(TEX_SPECULAR, "specularSampler", GBuffer::GBUFFER_TEXTURE_TYPE_SPECULAR);
		setTextureSampler(TEX_DEPTH,    "depthSampler",    GBuffer::GBUFFER_TEXTURE_TYPE_DEPTH);
							
		// UNIFORMS
		uniformLocs.resize(UniformLocId::UNI_SIZE);		

	};
	~DRdirectLightPassShader() {};
	///////////////////////////////////////////////////////////////////////		
};

#endif
