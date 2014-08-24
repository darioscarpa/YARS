#ifndef FORWARDSHADER_H
#define FORWARDSHADER_H

#include "../../util/vsGLInfoLib.h"

#include "../../ShaderHandling.h"
#include "../../Renderable.h"

class ForwardShader : public GfxShaderProgram {
public:	
	///////////////////////////////////////////////////////////////////////
	enum UniformBufferObjectId {
		UBO_MATRICES,
		UBO_MATERIALS,
		UBO_LIGHTS,
		UBO_SIZE
	};
	
	enum TextureSamplerId { 
		TEX_DIFFUSE,
		TEX_NORMAL,
		TEX_SPECULAR,
		TEX_OPACITY,
		TEX_SIZE
	};
		
	enum UniformLocId { 
		textureMappingEnabled, 
		normalMappingEnabled,
		specularMappingEnabled,
		opacityMappingEnabled,
		ambientLightLevel,
		enabledLighting,
		SIZE
	};		

	inline GLuint u(UniformLocId i) const { return uniformLocs[i]; }

	///////////////////////////////////////////////////////////////////////
	ForwardShader()	: GfxShaderProgram("forwardShader", "forward.vert", "forward.frag") {		
		// vertex attribs
		glBindAttribLocation(getId(), Renderable::v_positionLoc,  "position");
		glBindAttribLocation(getId(), Renderable::v_normalLoc,    "normal");
		glBindAttribLocation(getId(), Renderable::v_uvLoc,        "texCoord");
		glBindAttribLocation(getId(), Renderable::v_bitangentLoc, "bitangent");
		
		link();
		VSGLInfoLib::getUniformsInfo(getId());
		
		// UBOs
		uboBindings.resize(UBO_SIZE);
		setUBObinding(UBO_MATRICES,  "Matrices", Renderable::ubo_matrices);
		setUBObinding(UBO_MATERIALS, "Material", Renderable::ubo_material);
		setUBObinding(UBO_LIGHTS,    "Lights",   Renderable::ubo_lights);

		// TEXTURE SAMPLERS	
		textureSamplers.resize(TEX_SIZE);
		setTextureSampler(TEX_DIFFUSE,  "texUnit",            Renderable::tu_diffuse);
		setTextureSampler(TEX_NORMAL,   "normalMapUnit",      Renderable::tu_normalMap);
		setTextureSampler(TEX_SPECULAR, "specularMapSampler", Renderable::tu_specularMap);
		setTextureSampler(TEX_OPACITY,  "alphaMaskSampler",   Renderable::tu_opacityMap);
						
		// UNIFORMS		
		uniformLocs.resize(UniformLocId::SIZE);
		uniformLocs[textureMappingEnabled]  = glGetUniformLocation(getId(), "textureMappingEnabled");
		uniformLocs[normalMappingEnabled]   = glGetUniformLocation(getId(), "normalMappingEnabled");
		uniformLocs[specularMappingEnabled] = glGetUniformLocation(getId(), "specularMappingEnabled");
		uniformLocs[opacityMappingEnabled]  = glGetUniformLocation(getId(), "alphaMappingEnabled");
		uniformLocs[ambientLightLevel]      = glGetUniformLocation(getId(), "ambientLightLevel");
		uniformLocs[enabledLighting]        = glGetUniformLocation(getId(), "enabledLighting");					
	};
	~ForwardShader() {};	
	///////////////////////////////////////////////////////////////////////		

	inline void setAmbientLightLevel(float level) { setFloatUniform(ambientLightLevel,     level);   };
	inline void setLightingEnabled(bool enabled)  {	setBoolUniform(enabledLighting,        enabled); };
	inline void setTextureMapping(bool enabled)   {	setBoolUniform(textureMappingEnabled,  enabled); };	
	inline void setNormalMapping(bool enabled)    {	setBoolUniform(normalMappingEnabled,   enabled); };
	inline void setSpecularMapping(bool enabled)  {	setBoolUniform(specularMappingEnabled, enabled); };
	inline void setOpacityMapping(bool enabled)   {	setBoolUniform(opacityMappingEnabled,  enabled); };	
};

#endif
