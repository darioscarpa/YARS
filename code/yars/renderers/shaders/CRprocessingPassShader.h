#ifndef CRBLENDINGPASSSHADER_H
#define CRBLENDINGPASSSHADER_H

#include "../../ShaderHandling.h"
#include "../../Renderable.h"

#include "../gdlBuffer.h"

#include "sub/RandomGeneratorShaderSub.h"
#include "sub/D2DilShaderSub.h"
#include "sub/D2DaoShaderSub.h"
#include "sub/SamplingPatternShaderSub.h"



class CRprocessingPassShader : public GfxShaderProgram {
	
public:
	///////////////////////////////////////////////////////////////////////
	enum UniformLocId {
		ambientLightLevel,	
		mousePos,
		rayOrigin,
		
		doEnabled,
		aoMultiplier,
		ilMultiplier,
		areaMultiplier,
		solidAngleMultiplier,

		albedoEnabled,
		aoEnabled,
		ilEnabled,
		ambientLightEnabled,
		directLightEnabled,		

		showAreas,
		showSampling,
		showSamplingDensity,
		showZoomedArea,

		uKernelOffsets,
		uKernelSkip,
		uKernelSolidAngle,
		uKernelWeight,

		technique,
		// sampling params
		halfSamplesPerDir,
		radLength, 
		diskDisplacement,
		distMax,
		SIZE
	};
	enum TextureSamplerId { 
		TEX_ALBEDO,
		TEX_DIRLIGHT,
		TEX_NORMAL,
		TEX_DEPTH,		
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
	CRprocessingPassShader(Shader::compConstMap_t compileConsts) : GfxShaderProgram("c_processingPass", "deferred_fullscreenquad.vert", "c_processgdlbuf.frag", nullptr, compileConsts) {		
		// input
		glBindAttribLocation(getId(), Renderable::v_positionLoc,  "positionNDC");

		// output
		glBindFragDataLocation(getId(), 0, "FragColor");
		
		
		link();

		// UBOs		
		uboBindings.resize(UBO_SIZE);
		setUBObinding(UBO_MATRICES, "Matrices",  Renderable::ubo_matrices);
		setUBObinding(UBO_LIGHTS,   "Lights",    Renderable::ubo_lights);
		setUBObinding(UBO_SCRSIZE,  "Deferred",  Renderable::ubo_deferred);		
						
		// TEXTURE SAMPLERS
		textureSamplers.resize(TEX_SIZE);
		setTextureSampler(TEX_ALBEDO,   "albedoSampler",      GdlBuffer::GDLBUFFER_TEXTURE_TYPE_ALBEDO);
		setTextureSampler(TEX_DIRLIGHT, "directLightTexData", GdlBuffer::GDLBUFFER_TEXTURE_TYPE_DIRECTLIGHT);
		setTextureSampler(TEX_NORMAL,   "normalSampler",      GdlBuffer::GDLBUFFER_TEXTURE_TYPE_NORMAL);
		setTextureSampler(TEX_DEPTH,    "depthSampler",       GdlBuffer::GDLBUFFER_TEXTURE_TYPE_DEPTH);		
							
		// UNIFORMS
		uniformLocs.resize(UniformLocId::SIZE);
		uniformLocs[ambientLightLevel] = glGetUniformLocation(getId(), "ambientLightLevel");
		uniformLocs[mousePos]          = glGetUniformLocation(getId(), "mousePos");
		uniformLocs[rayOrigin]         = glGetUniformLocation(getId(), "rayOrigin");
		
		uniformLocs[albedoEnabled]         = glGetUniformLocation(getId(), "albedoEnabled");
		uniformLocs[aoEnabled]             = glGetUniformLocation(getId(), "aoEnabled");
		uniformLocs[ilEnabled]             = glGetUniformLocation(getId(), "ilEnabled");
		uniformLocs[ambientLightEnabled]   = glGetUniformLocation(getId(), "ambientLightEnabled");
		uniformLocs[directLightEnabled]    = glGetUniformLocation(getId(), "directLightEnabled");
		uniformLocs[doEnabled]             = glGetUniformLocation(getId(), "doEnabled");
		
		
		uniformLocs[showAreas]            = glGetUniformLocation(getId(), "showAreas");
		uniformLocs[showSampling]         = glGetUniformLocation(getId(), "showSampling");
		uniformLocs[showSamplingDensity]  = glGetUniformLocation(getId(), "showSamplingDensity");
		uniformLocs[showZoomedArea]       = glGetUniformLocation(getId(), "showZoomedArea");

		uniformLocs[aoMultiplier]         = glGetUniformLocation(getId(), "aoMultiplier");
		uniformLocs[ilMultiplier]         = glGetUniformLocation(getId(), "ilMultiplier");
		uniformLocs[areaMultiplier]       = glGetUniformLocation(getId(), "areaMultiplier");
		uniformLocs[solidAngleMultiplier] = glGetUniformLocation(getId(), "solidAngleMultiplier");
		
				
		uniformLocs[diskDisplacement]  = glGetUniformLocation(getId(), "diskDisplacement");
		uniformLocs[distMax]           = glGetUniformLocation(getId(), "distMax");
		uniformLocs[radLength]         = glGetUniformLocation(getId(), "radLength");
		uniformLocs[halfSamplesPerDir] = glGetUniformLocation(getId(), "halfSamplesPerDir");

		uniformLocs[technique]         = glGetUniformLocation(getId(), "technique");

		uniformLocs[uKernelOffsets]    = glGetUniformLocation(getId(), "uKernelOffsets");
		uniformLocs[uKernelSkip]       = glGetUniformLocation(getId(), "uKernelSkip");
		uniformLocs[uKernelSolidAngle] = glGetUniformLocation(getId(), "uKernelSolidAngle");
		uniformLocs[uKernelWeight]     = glGetUniformLocation(getId(), "uKernelWeight");
		
				
		fragmentSubroutines.resize(SUBUNI_FRAG_SIZE);
		//fragmentSubroutines[randomGenerator] = std::unique_ptr<ShaderSubroutine>(new RandomGeneratorShaderSub());
		fragmentSubroutines[samplingPattern] = std::unique_ptr<ShaderSubroutine>(new SamplingPatternShaderSub());
		fragmentSubroutines[ilCalculation]   = std::unique_ptr<ShaderSubroutine>(new D2DilShaderSub());
		fragmentSubroutines[aoCalculation]   = std::unique_ptr<ShaderSubroutine>(new D2DaoShaderSub());
		
		initSubroutines();
					
	};
	~CRprocessingPassShader() {};
	///////////////////////////////////////////////////////////////////////	
	inline void setAmbientLightLevel(float level) { setFloatUniform(ambientLightLevel, level); };

	inline void setMousePos(glm::vec2 mouseCoords) { setVec2Uniform(mousePos, mouseCoords); }
	inline void setRayOriginPos(glm::vec2 mouseCoords) { setVec2Uniform(rayOrigin, mouseCoords); }

			
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
	inline void setDoEnabled(bool enabled)        { setBoolUniform(doEnabled, enabled);     };
	
	inline void setIlEnabled(bool enabled)        { setBoolUniform(ilEnabled, enabled);     };
	inline void setAmbientEnabled(bool enabled)   { setBoolUniform(ambientLightEnabled, enabled);  };
	inline void setDirectEnabled(bool enabled)    { setBoolUniform(directLightEnabled, enabled);  };
	

	//inline void setRandomGenerator(RandomGeneratorShaderSub::OptionValues randomGen) { setFragmentSubroutine(randomGenerator, randomGen); }
	inline void setSamplingPattern(SamplingPatternShaderSub::OptionValues samplingP) { setFragmentSubroutine(samplingPattern, samplingP); }
	inline void setIlCalculation(D2DilShaderSub::OptionValues ilid) { setFragmentSubroutine(ilCalculation, ilid); }
	inline void setAoCalculation(D2DaoShaderSub::OptionValues aoid) { setFragmentSubroutine(aoCalculation, aoid); }

	inline void setShowAreas(bool enabled)  { setBoolUniform(showAreas, enabled); };
	inline void setShowSampling(bool enabled)  { setBoolUniform(showSampling, enabled); };
	inline void setShowSamplingDensity(bool enabled)  { setBoolUniform(showSamplingDensity, enabled); };
	inline void setShowZoomedArea(bool enabled)  { setBoolUniform(showZoomedArea, enabled); };
	
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
