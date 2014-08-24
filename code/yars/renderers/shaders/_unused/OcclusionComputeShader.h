#ifndef OCCLUSIONCOMPUTESHADER_H
#define OCCLUSIONCOMPUTESHADER_H

#include "../../ShaderHandling.h"
#include "../../Renderable.h"

class OcclusionComputeShader : public ComputeShaderProgram {
	
public:
	///////////////////////////////////////////////////////////////////////
	enum UniformLocId {
		ambientLightLevel,	
		mousePos,
		
		aoMultiplier,
		ilMultiplier,
		areaMultiplier,
		solidAngleMultiplier,

		albedoEnabled,
		aoEnabled,
		ilEnabled,
		ambientLightEnabled,
		diffuseLightEnabled,
		specularLightEnabled,

		showAreas,
		showSampling,
		showSamplingDensity,

		
		uKernelOffsets,
		uKernelSkip,
		uKernelSolidAngle,
		uKernelLen,
		uKernelWeight,

		samplingTech,
		// sampling params
		halfSamplesPerDir,
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
		TEX_SCATTERED,
		TEX_REFLECTED,
		TEX_SIZE
	};
	enum UniformBufferObjectId { 
		UBO_MATRICES,
		UBO_LIGHTS,
		UBO_SCRSIZE,	
		UBO_SIZE
	};


	enum FragmentSubroutineUniformLocId {		
		//randomGenerator,
		samplingPattern,
		aoCalculation,
		ilCalculation,
		SUBUNI_FRAG_SIZE
	};
		
	inline GLuint u(UniformLocId i) const { return uniformLocs[i]; }
	
	///////////////////////////////////////////////////////////////////////
	OcclusionComputeShader() : ComputeShaderProgram("directionalOcclusion.comp") {
		// input
		glBindAttribLocation(getId(), Renderable::v_positionLoc,  "positionNDC");
		
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
		setTextureSampler(TEX_RANDOM,   "randomSampler",   GBuffer::GBUFFER_TEXTURE_AVAILABLE);
		setTextureSampler(TEX_SCATTERED,   "scatteredSampler",   GBuffer::GBUFFER_TEXTURE_AVAILABLE + 1 + DirectLightBuffer::DLBUFFER_TEXTURE_TYPE_DIFFUSE);
		setTextureSampler(TEX_REFLECTED,   "reflectedSampler",   GBuffer::GBUFFER_TEXTURE_AVAILABLE + 1 + DirectLightBuffer::DLBUFFER_TEXTURE_TYPE_SPECULAR);
		//setTextureSampler(TEX_RANDOM,   "randomSampler",   Renderable::tu_random);
		//setTextureSamplerUnit(TEX_POSITION, GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);			
							
		// UNIFORMS
		uniformLocs.resize(UniformLocId::SIZE);
		uniformLocs[ambientLightLevel] = glGetUniformLocation(getId(), "ambientLightLevel");
		uniformLocs[mousePos]          = glGetUniformLocation(getId(), "mousePos");
		
		uniformLocs[albedoEnabled]         = glGetUniformLocation(getId(), "albedoEnabled");
		uniformLocs[aoEnabled]             = glGetUniformLocation(getId(), "aoEnabled");
		uniformLocs[ilEnabled]             = glGetUniformLocation(getId(), "ilEnabled");
		uniformLocs[ambientLightEnabled]   = glGetUniformLocation(getId(), "ambientLightEnabled");
		uniformLocs[diffuseLightEnabled]   = glGetUniformLocation(getId(), "diffuseLightEnabled");
		uniformLocs[specularLightEnabled]  = glGetUniformLocation(getId(), "specularLightEnabled");
		
		uniformLocs[showAreas]     = glGetUniformLocation(getId(), "showAreas");
		uniformLocs[showSampling]  = glGetUniformLocation(getId(), "showSampling");
		uniformLocs[showSamplingDensity]  = glGetUniformLocation(getId(), "showSamplingDensity");

		uniformLocs[aoMultiplier]         = glGetUniformLocation(getId(), "aoMultiplier");
		uniformLocs[ilMultiplier]         = glGetUniformLocation(getId(), "ilMultiplier");
		uniformLocs[areaMultiplier]       = glGetUniformLocation(getId(), "areaMultiplier");
		uniformLocs[solidAngleMultiplier] = glGetUniformLocation(getId(), "solidAngleMultiplier");
		
				
		uniformLocs[diskDisplacement]  = glGetUniformLocation(getId(), "diskDisplacement");
		uniformLocs[distMax]           = glGetUniformLocation(getId(), "distMax");
		uniformLocs[radLength]         = glGetUniformLocation(getId(), "radLength");
		uniformLocs[halfSamplesPerDir] = glGetUniformLocation(getId(), "halfSamplesPerDir");

		uniformLocs[samplingTech]      = glGetUniformLocation(getId(), "samplingTech");

		uniformLocs[uKernelOffsets]    = glGetUniformLocation(getId(), "uKernelOffsets");
		uniformLocs[uKernelSkip]       = glGetUniformLocation(getId(), "uKernelSkip");
		uniformLocs[uKernelSolidAngle] = glGetUniformLocation(getId(), "uKernelSolidAngle");
		uniformLocs[uKernelLen]        = glGetUniformLocation(getId(), "uKernelLen");
		uniformLocs[uKernelWeight]     = glGetUniformLocation(getId(), "uKernelWeight");
					
	};
	~OcclusionComputeShader() {};
	///////////////////////////////////////////////////////////////////////	
	inline void setAmbientLightLevel(float level) { setFloatUniform(ambientLightLevel, level); };

	inline void setMousePos(glm::vec2 mouseCoords) { setVec2Uniform(mousePos, mouseCoords); }
			
	inline void setNumSamples(int samples)          { setIntUniform(halfSamplesPerDir,   samples); };
	inline void setMaxSamplingRadius(float samples) { setFloatUniform(radLength,         samples); };
	inline void setDiskDisplacement(float samples)  { setFloatUniform(diskDisplacement,  samples); };
	inline void setDistMax(float samples)           { setFloatUniform(distMax,           samples); };

	inline void setAoMultiplier(float mul)          { setFloatUniform(aoMultiplier,   mul); };
	inline void setIlMultiplier(float mul)          { setFloatUniform(ilMultiplier,   mul); };
	inline void setAreaMultiplier(float mul)        { setFloatUniform(areaMultiplier, mul); };
	inline void setSolidAngleMultiplier(float mul)  { setFloatUniform(solidAngleMultiplier, mul); };

	inline void setAlbedoEnabled(bool enabled)    { setBoolUniform(albedoEnabled, enabled); };
	inline void setAoEnabled(bool enabled)        { setBoolUniform(aoEnabled, enabled);     };
	inline void setIlEnabled(bool enabled)        { setBoolUniform(ilEnabled, enabled);     };
	inline void setAmbientEnabled(bool enabled)   { setBoolUniform(ambientLightEnabled, enabled);  };
	inline void setDiffuseEnabled(bool enabled)   { setBoolUniform(diffuseLightEnabled, enabled);  };
	inline void setSpecularEnabled(bool enabled)  { setBoolUniform(specularLightEnabled, enabled); };

	inline void setShowAreas(bool enabled)  { setBoolUniform(showAreas, enabled); };
	inline void setShowSampling(bool enabled)  { setBoolUniform(showSampling, enabled); };
	inline void setShowSamplingDensity(bool enabled)  { setBoolUniform(showSamplingDensity, enabled); };
	
	/*
	inline void setSamplingTechnique(DoublePassBlendingDeferredRenderer::SamplingKernelMode_t id) { setIntUniform(halfSamplesPerDir, id); };
	
	inline void setRandomGenerator(RandomGeneratorShaderSub::OptionValues randomGen) { setFragmentSubroutine(randomGenerator, randomGen); }
	inline void setSamplingPattern(SamplingPatternShaderSub::OptionValues samplingP) { setFragmentSubroutine(samplingPattern, samplingP); }
	inline void setIlCalculation(D2DilShaderSub::OptionValues ilid) { setFragmentSubroutine(ilCalculation, ilid); }
	inline void setAoCalculation(D2DaoShaderSub::OptionValues aoid) { setFragmentSubroutine(aoCalculation, aoid); }
	*/
	
	inline void setSampleKernel(const std::vector<glm::vec3>& kernel) {
		glUniform3fv(uniformLocs[uKernelOffsets],    kernel.size(), reinterpret_cast<const GLfloat*>(&kernel[0]));		
	};
	inline void setSampleKernelDetails(
		                         const std::vector<int>& skipTable,
								 const std::vector<float>& solidangleTable,
								 //const std::vector<float>& vectorlenTable,
								 const std::vector<float>& sampleWeightTable
								 ) {
		glUniform1iv(uniformLocs[uKernelSkip],       skipTable.size(), reinterpret_cast<const GLint*>(&skipTable[0]));
		glUniform1fv(uniformLocs[uKernelSolidAngle], solidangleTable.size(), reinterpret_cast<const GLfloat*>(&solidangleTable[0]));
		//glUniform1fv(uniformLocs[uKernelLen],      kernel.size(), reinterpret_cast<const GLfloat*>(&vectorlenTable[0]));
		glUniform1fv(uniformLocs[uKernelWeight],     sampleWeightTable.size(), reinterpret_cast<const GLfloat*>(&sampleWeightTable[0]));
	};
};

#endif
