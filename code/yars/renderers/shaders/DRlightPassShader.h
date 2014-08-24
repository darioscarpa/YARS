#ifndef DRLIGHTPASSSHADER_H
#define DRLIGHTPASSSHADER_H

#include "../../ShaderHandling.h"
#include "../../Renderable.h"

#include "../gBuffer.h"

#include "sub/RandomGeneratorShaderSub.h"
#include "sub/SamplingPatternShaderSub.h"
#include "sub/D2DaoShaderSub.h"

class DRlightPassShader : public GfxShaderProgram {
	
public:
	///////////////////////////////////////////////////////////////////////
	enum UniformLocId {
		ambientLightLevel,	
				
		uKernelOffsets,
		uKernelSkip,
		uKernelSolidAngle,
		uKernelWeight,

		samplingTech,
		
		// AO params
		aoEnabled,
		aoOnly,
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
		UBO_SCRSIZE,
	//	UBO_LIGHTS,
	//	UBO_MATERIALS,
		UBO_SIZE
	};
	
	enum FragmentSubroutineUniformLocId {
		//randomGenerator,
		samplingPattern,
		aoCalculation,
		SUBUNI_FRAG_SIZE
	};	

	inline GLuint u(UniformLocId i) const { return uniformLocs[i]; }
	
	///////////////////////////////////////////////////////////////////////
	DRlightPassShader(Shader::compConstMap_t compileConsts) : GfxShaderProgram("d_lightPass", "deferred_fullscreenquad.vert", "deferred_lightpass.frag", nullptr, compileConsts) {		
		// input
		glBindAttribLocation(getId(), Renderable::v_positionLoc,  "positionNDC");
		
		link();

		// UBOs		
		uboBindings.resize(UBO_SIZE);
		setUBObinding(UBO_MATRICES,  "Matrices",  Renderable::ubo_matrices);
		setUBObinding(UBO_SCRSIZE,   "Deferred",  Renderable::ubo_deferred);
		//setUBObinding(UBO_MATRICES,  "Lights",    Renderable::ubo_lights);
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
		uniformLocs[ambientLightLevel] = glGetUniformLocation(getId(), "ambientLightLevel");
		
		uniformLocs[diskDisplacement]  = glGetUniformLocation(getId(), "diskDisplacement");
		uniformLocs[distMax]           = glGetUniformLocation(getId(), "distMax");
		uniformLocs[radLength]         = glGetUniformLocation(getId(), "radLength");
		uniformLocs[aoEnabled]         = glGetUniformLocation(getId(), "aoEnabled");
		uniformLocs[aoOnly]            = glGetUniformLocation(getId(), "aoOnly");
		uniformLocs[aoSamples]         = glGetUniformLocation(getId(), "aoSamples");
		
		uniformLocs[samplingTech]      = glGetUniformLocation(getId(), "samplingTech");
		uniformLocs[uKernelOffsets]    = glGetUniformLocation(getId(), "uKernelOffsets");
		uniformLocs[uKernelSkip]       = glGetUniformLocation(getId(), "uKernelSkip");
		uniformLocs[uKernelSolidAngle] = glGetUniformLocation(getId(), "uKernelSolidAngle");
		uniformLocs[uKernelWeight]     = glGetUniformLocation(getId(), "uKernelWeight");

		fragmentSubroutines.resize(SUBUNI_FRAG_SIZE);
		//fragmentSubroutines[randomGenerator] = std::unique_ptr<ShaderSubroutine>(new RandomGeneratorShaderSub());
		fragmentSubroutines[samplingPattern] = std::unique_ptr<ShaderSubroutine>(new SamplingPatternShaderSub());
		fragmentSubroutines[aoCalculation]   = std::unique_ptr<ShaderSubroutine>(new D2DaoShaderSub());

		initSubroutines();		
		
	};
	~DRlightPassShader() {};
	///////////////////////////////////////////////////////////////////////	
	inline void setAmbientLightLevel(float level) { setFloatUniform(ambientLightLevel, level); };
		
	inline void setAmbientOcclusion(bool enabled) {	setBoolUniform(aoEnabled, enabled);        };
	inline void setShowingOnlyAO(bool enabled)    {	setBoolUniform(aoOnly, enabled);           };

	inline void setDSSAOsamples(int samples)            { setIntUniform(aoSamples,           samples); };
	inline void setDSSAOradLength(float samples)        { setFloatUniform(radLength,         samples); };
	inline void setDSSAOdiskDisplacement(float samples) { setFloatUniform(diskDisplacement,  samples); };
	inline void setDSSAOdistMax(float samples)          { setFloatUniform(distMax,           samples); };

	//inline void setRandomGenerator(RandomGeneratorShaderSub::OptionValues randomGen) { setFragmentSubroutine(randomGenerator, randomGen); }
	inline void setSamplingPattern(SamplingPatternShaderSub::OptionValues samplingP) { setFragmentSubroutine(samplingPattern, samplingP); }
	inline void setAoCalculation(D2DaoShaderSub::OptionValues aoid) { setFragmentSubroutine(aoCalculation, aoid); }


	///////////////////////////////////////////////////////////////////////		
	inline void setEmisphereSamplingTechnique(HemisphereSampling::KernelMode_t id) { setIntUniform(samplingTech, id); };
	inline void setSampleKernel(const std::vector<glm::vec3>& kernel) {
		glUniform3fv(uniformLocs[uKernelOffsets],    kernel.size(), reinterpret_cast<const GLfloat*>(&kernel[0]));		
	};
	inline void setSampleKernelDetails(
		                         const std::vector<int>& skipTable,
								 const std::vector<float>& solidangleTable,
								 const std::vector<float>& sampleWeightTable
								 ) {
		glUniform1iv(uniformLocs[uKernelSkip],       skipTable.size(), reinterpret_cast<const GLint*>(&skipTable[0]));
		glUniform1fv(uniformLocs[uKernelSolidAngle], solidangleTable.size(), reinterpret_cast<const GLfloat*>(&solidangleTable[0]));
		glUniform1fv(uniformLocs[uKernelWeight],     sampleWeightTable.size(), reinterpret_cast<const GLfloat*>(&sampleWeightTable[0]));
	};
	
};

#endif
