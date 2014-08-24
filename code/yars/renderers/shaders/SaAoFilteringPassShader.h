#ifndef SAAOFILTERINGPASSSHADER_H
#define SAAOFILTERINGPASSSHADER_H

#include "../../ShaderHandling.h"

//#include "../gdlBuffer.h"
#include "../gBuffer.h"
#include "../aoBuffer.h"

class SaAoFilteringPassShader : public GfxShaderProgram {

public:
	///////////////////////////////////////////////////////////////////////
	enum UniformLocId {
		kdepth,
		knormal,
		kernelSize,

		ambientLightLevel,
		albedoEnabled,
		ambientLightEnabled,
		directLightEnabled,
		aoEnabled,		
		blurEnabled,

		UNIFORM_SIZE
	};
	enum TextureSamplerId {		
		TEX_DIFFUSE,
		TEX_NORMAL,
		TEX_SPECULAR,		
		TEX_DEPTH,
		TEX_AO,
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
	SaAoFilteringPassShader() : GfxShaderProgram("saao_filteringPass", "deferred_fullscreenquad.vert", "saao_filteringPass.frag") {
		// input
		glBindAttribLocation(getId(), Renderable::v_positionLoc,  "positionNDC");

		// output
		glBindFragDataLocation(getId(), 0, "blurredAo");

		link();

		// UBOs
		uboBindings.resize(UBO_SIZE);
		setUBObinding(UBO_MATRICES, "Matrices",  Renderable::ubo_matrices);
		setUBObinding(UBO_SCRSIZE,  "Deferred",  Renderable::ubo_deferred);
		setUBObinding(UBO_LIGHTS,   "Lights",    Renderable::ubo_lights);

		// TEXTURE SAMPLERS
		textureSamplers.resize(TEX_SIZE);
		setTextureSampler(TEX_DIFFUSE,  "diffuseSampler",  GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
		setTextureSampler(TEX_SPECULAR, "specularSampler", GBuffer::GBUFFER_TEXTURE_TYPE_SPECULAR);
		setTextureSampler(TEX_NORMAL,   "normalSampler",   GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
		setTextureSampler(TEX_DEPTH,    "depthSampler",    GBuffer::GBUFFER_TEXTURE_TYPE_DEPTH);
		setTextureSampler(TEX_AO,       "aoSampler",	   GBuffer::GBUFFER_TEXTURE_AVAILABLE + AoBuffer::AOBUFFER_TEXTURE_TYPE_AO);
		

		// UNIFORMS
		uniformLocs.resize(UNIFORM_SIZE);
		uniformLocs[kdepth]     = glGetUniformLocation(getId(), "kdepth");
		uniformLocs[knormal]    = glGetUniformLocation(getId(), "knormal");
		uniformLocs[kernelSize] = glGetUniformLocation(getId(), "kernelSize");

		uniformLocs[ambientLightLevel] = glGetUniformLocation(getId(), "ambientLightLevel");
		uniformLocs[albedoEnabled]         = glGetUniformLocation(getId(), "albedoEnabled");
		uniformLocs[ambientLightEnabled]   = glGetUniformLocation(getId(), "ambientLightEnabled");
		uniformLocs[directLightEnabled]    = glGetUniformLocation(getId(), "directLightEnabled");
		uniformLocs[aoEnabled]             = glGetUniformLocation(getId(), "aoEnabled");
		uniformLocs[blurEnabled]		   = glGetUniformLocation(getId(), "blurEnabled");



	};
	~SaAoFilteringPassShader() {};
	///////////////////////////////////////////////////////////////////////

	inline void setKdepth(float v)    { setFloatUniform(kdepth,     v); };
	inline void setKnormal(float v)   { setFloatUniform(knormal,    v); };
	inline void setKernelSize(int s)  { setIntUniform(kernelSize, s); }

	///////////////////////////////////////////////////////////////////////

	inline void setAmbientEnabled(bool enabled)   { setBoolUniform(ambientLightEnabled, enabled);  };
	inline void setAmbientLightLevel(float level) { setFloatUniform(ambientLightLevel, level); };

	inline void setDirectEnabled(bool enabled)    { setBoolUniform(directLightEnabled, enabled);  };

	inline void setAlbedoEnabled(bool enabled)    { setBoolUniform(albedoEnabled, enabled); };
	inline void setAoEnabled(bool enabled)        { setBoolUniform(aoEnabled, enabled);     };	

	inline void setBlurEnabled(bool enabled)      { setBoolUniform(blurEnabled, enabled);     };

};

#endif
