#ifndef SAAOILFILTERINGPASSSHADER_H
#define SAAOILFILTERINGPASSSHADER_H

#include "../../ShaderHandling.h"

#include "../gdlBuffer.h"
#include "../aoBuffer.h"

class SaAoIlFilteringPassShader : public GfxShaderProgram {

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
		ilEnabled,
		doEnabled,
		blurEnabled,

		UNIFORM_SIZE
	};
	enum TextureSamplerId {
		TEX_AO,
		TEX_DIRLIGHT,
		TEX_NORMAL,
		TEX_DEPTH,
		TEX_SIZE
	};
	enum UniformBufferObjectId {
		UBO_MATRICES,
		UBO_SCRSIZE,
		UBO_SIZE
	};

	inline GLuint u(UniformLocId i) const { return uniformLocs[i]; }

	///////////////////////////////////////////////////////////////////////
	SaAoIlFilteringPassShader() : GfxShaderProgram("saaoil_filteringPass", "deferred_fullscreenquad.vert", "saaoil_filteringPass.frag") {
		// input
		glBindAttribLocation(getId(), Renderable::v_positionLoc,  "positionNDC");

		// output
		glBindFragDataLocation(getId(), 0, "blurredAo");

		link();

		// UBOs
		uboBindings.resize(UBO_SIZE);
		setUBObinding(UBO_MATRICES, "Matrices",  Renderable::ubo_matrices);
		setUBObinding(UBO_SCRSIZE,  "Deferred",  Renderable::ubo_deferred);

		// TEXTURE SAMPLERS
		textureSamplers.resize(TEX_SIZE);
		setTextureSampler(TEX_AO,       "ilaoSampler",		  GdlBuffer::GDLBUFFER_TEXTURE_AVAILABLE + AoBuffer::AOBUFFER_TEXTURE_TYPE_AO);
		setTextureSampler(TEX_DIRLIGHT, "directLightTexData", GdlBuffer::GDLBUFFER_TEXTURE_TYPE_DIRECTLIGHT);
		setTextureSampler(TEX_NORMAL,   "normalSampler",      GdlBuffer::GDLBUFFER_TEXTURE_TYPE_NORMAL);
		setTextureSampler(TEX_DEPTH,    "depthSampler",       GdlBuffer::GDLBUFFER_TEXTURE_TYPE_DEPTH);

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
		uniformLocs[ilEnabled]             = glGetUniformLocation(getId(), "ilEnabled");
		uniformLocs[doEnabled]             = glGetUniformLocation(getId(), "doEnabled");
		uniformLocs[blurEnabled]		   = glGetUniformLocation(getId(), "blurEnabled");



	};
	~SaAoIlFilteringPassShader() {};
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
	inline void setDoEnabled(bool enabled)        { setBoolUniform(doEnabled, enabled);     };
	inline void setIlEnabled(bool enabled)        { setBoolUniform(ilEnabled, enabled);     };

	inline void setBlurEnabled(bool enabled)      { setBoolUniform(blurEnabled, enabled);     };

};

#endif
