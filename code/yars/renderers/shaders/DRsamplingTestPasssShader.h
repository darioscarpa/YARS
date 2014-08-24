#ifndef DRSAMPLINGLIGHTPASSSHADER_H
#define DRSAMPLINGLIGHTPASSSHADER_H

#include "../../ShaderHandling.h"
#include "../../Renderable.h"

#include "../gBuffer.h"

class DRsamplingTestPassShader : public GfxShaderProgram {
	
public:
	///////////////////////////////////////////////////////////////////////
	enum UniformLocId {
		invScreenSize,
		mousePos,

		uKernelOffsets,
		uKernelSkip,
		uKernelSolidAngle,
		uKernelWeight,

		samplingTech,

		// params
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

	enum SubroutineUniformLocId {
		samplingPattern,
		SUBUNI_FRAG_SIZE
	};

	/*
	enum SubroutineRandomGenerator {
		none,
		noiseFunc,
		texture,
		func,
		SUBIDX_RG_SIZE
	};
	enum SubroutineSamplingPattern {
		dssao,
		radial,
		SUBIDX_SP_SIZE
	};*/
	
	inline GLuint u(UniformLocId i) const { return uniformLocs[i]; }
	
	///////////////////////////////////////////////////////////////////////
	DRsamplingTestPassShader(Shader::compConstMap_t compileConsts) : GfxShaderProgram("d_sampleTestingPass", "deferred_fullscreenquad.vert", "deferred_samplingtestpass.frag", nullptr, compileConsts) {		
		// input
		glBindAttribLocation(getId(), Renderable::v_positionLoc,  "positionNDC");
		
		link();

		// UBOs		
		uboBindings.resize(UBO_SIZE);
		setUBObinding(UBO_MATRICES,  "Matrices",  Renderable::ubo_matrices);
		setUBObinding(UBO_MATRICES,  "Lights",    Renderable::ubo_lights);
		setUBObinding(UBO_MATRICES,  "Deferred",  Renderable::ubo_deferred);
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

		uniformLocs[samplingTech]      = glGetUniformLocation(getId(), "samplingTech");
		uniformLocs[uKernelOffsets]    = glGetUniformLocation(getId(), "uKernelOffsets");
		uniformLocs[uKernelSkip]       = glGetUniformLocation(getId(), "uKernelSkip");
		uniformLocs[uKernelSolidAngle] = glGetUniformLocation(getId(), "uKernelSolidAngle");
		uniformLocs[uKernelWeight]     = glGetUniformLocation(getId(), "uKernelWeight");

		fragmentSubroutines.resize(SUBUNI_FRAG_SIZE);
//		fragmentSubroutines[randomGenerator] = std::unique_ptr<ShaderSubroutine>(new RandomGeneratorShaderSub());
		fragmentSubroutines[samplingPattern] = std::unique_ptr<ShaderSubroutine>(new SamplingPatternShaderSub());

		initSubroutines();

		
	};
	~DRsamplingTestPassShader() {};
	///////////////////////////////////////////////////////////////////////	
	inline void setMousePos(glm::vec2 mouseCoords)   {setVec2Uniform(mousePos, mouseCoords); }
	
	inline void setSamples(int samples)            { setIntUniform(aoSamples,           samples); };
	inline void setRadLength(float samples)        { setFloatUniform(radLength,         samples); };
	inline void setDiskDisplacement(float samples) { setFloatUniform(diskDisplacement,  samples); };
	inline void setDistMax(float samples)          { setFloatUniform(distMax,           samples); };

	//inline void setRandomGenerator(RandomGeneratorShaderSub::OptionValues randomGen) { setFragmentSubroutine(randomGenerator, randomGen); }
	inline void setSamplingPattern(SamplingPatternShaderSub::OptionValues samplingP) { setFragmentSubroutine(samplingPattern, samplingP); }

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
