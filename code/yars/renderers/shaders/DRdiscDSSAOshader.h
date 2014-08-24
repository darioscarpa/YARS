#ifndef DRDISCDSSAOSHADER_H
#define DRDISCDSSAOSHADER_H

#include "../../ShaderHandling.h"
#include "../../Renderable.h"

#include "../gBuffer.h"

class DRdiscDSSAOshader : public GfxShaderProgram {
	
public:
	///////////////////////////////////////////////////////////////////////
	enum UniformLocId {
		invScreenSize,
		mousePos,
		// AO params
		aoSamples,
		radLength, 
		diskDisplacement,
		distMax,
		SIZE
	};
	enum TextureSamplerId { 
		TEX_ALBEDO,
		TEX_NORMAL,
		TEX_SPECULAR,
		//TEX_POSITION,
		TEX_DEPTH,
		TEX_RANDOM,		
		TEX_SIZE
	};
	enum UniformBufferObjectId { 
		UBO_MATRICES,
		UBO_LIGHTS,
		UBO_SCRSIZE,
	//	UBO_MATERIALS,
		UBO_SIZE
	};
	
	inline GLuint u(UniformLocId i) const { return uniformLocs[i]; }
	
	///////////////////////////////////////////////////////////////////////
	DRdiscDSSAOshader(Shader::compConstMap_t compileConsts) : GfxShaderProgram("d_discDSSAO", "deferred_fullscreenquad.vert", "deferred_discDSSAOpass.frag", nullptr, compileConsts) {		
		// input
		glBindAttribLocation(getId(), Renderable::v_positionLoc,  "positionNDC");
		
		link();

		// UBOs		
		uboBindings.resize(UBO_SIZE);
		setUBObinding(UBO_MATRICES, "Matrices",  Renderable::ubo_matrices);
		setUBObinding(UBO_LIGHTS,   "Lights",    Renderable::ubo_lights);
		setUBObinding(UBO_SCRSIZE,  "Deferred",  Renderable::ubo_deferred);
		//setUBObinding(UBO_MATERIALS, "Material",  Renderable::ubo_material);
						
		// TEXTURE SAMPLERS
		textureSamplers.resize(TEX_SIZE);
		setTextureSampler(TEX_ALBEDO,   "diffuseSampler",  GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
		setTextureSampler(TEX_NORMAL,   "normalSampler",   GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
		setTextureSampler(TEX_SPECULAR, "specularSampler", GBuffer::GBUFFER_TEXTURE_TYPE_SPECULAR);
		setTextureSampler(TEX_DEPTH,    "depthSampler",    GBuffer::GBUFFER_TEXTURE_TYPE_DEPTH);
		setTextureSampler(TEX_RANDOM,   "randomSampler",   GBuffer::GBUFFER_TEXTURE_AVAILABLE);
		//setTextureSampler(TEX_RANDOM,   "randomSampler",   Renderable::tu_random);
		//setTextureSamplerUnit(TEX_POSITION, GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);			
							
		// UNIFORMS
		uniformLocs.resize(UniformLocId::SIZE);
		uniformLocs[invScreenSize]     = glGetUniformLocation(getId(), "invScreenSize");
		uniformLocs[mousePos]          = glGetUniformLocation(getId(), "mousePos");		

		uniformLocs[diskDisplacement]  = glGetUniformLocation(getId(), "diskDisplacement");
		uniformLocs[distMax]           = glGetUniformLocation(getId(), "distMax");
		uniformLocs[radLength]         = glGetUniformLocation(getId(), "radLength");
		uniformLocs[aoSamples]         = glGetUniformLocation(getId(), "aoSamples");
	};
	~DRdiscDSSAOshader() {};
	///////////////////////////////////////////////////////////////////////	
	inline void setMousePos(glm::vec2 mouseCoords)    { setVec2Uniform(mousePos, mouseCoords); }
	
	//inline void setNumPointLights(int n)              { setIntUniform(numPointLights, n);      };
		
	inline void setNumSamples(int samples)            { setIntUniform(aoSamples,           samples); };
	inline void setMaxSamplingRadius(float samples)   { setFloatUniform(radLength,         samples); };
	inline void setMaxDiskDisplacement(float samples) { setFloatUniform(diskDisplacement,  samples); };
	inline void setMaxDiskDistance(float samples)     { setFloatUniform(distMax,           samples); };
};

#endif
