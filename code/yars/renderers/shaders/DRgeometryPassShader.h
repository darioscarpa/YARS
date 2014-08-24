#ifndef DRGEOMETRYPASSSHADER_H
#define DRGEOMETRYPASSSHADER_H

#include "../../ShaderHandling.h"
#include "../../Renderable.h"

#include "sub/AreaCalculatorShaderSub.h"

class DRgeometryPassShader : public GfxShaderProgram {	
public:
	///////////////////////////////////////////////////////////////////////
	enum UniformBufferObjectId {
		 UBO_MATRICES, 
		 UBO_MATERIALS,		 
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

	enum GeometrySubroutineUniformLocId {		
		areaCalculation,
		SUBUNI_GEOM_SIZE
	};

	inline GLuint u(UniformLocId i) const { return uniformLocs[i]; }

	///////////////////////////////////////////////////////////////////////
	DRgeometryPassShader() : GfxShaderProgram("d_geometryPass", "deferred.vert", "deferred.frag", "deferred.geom") {		
		// input
		glBindAttribLocation(getId(), Renderable::v_positionLoc,  "position");
		glBindAttribLocation(getId(), Renderable::v_normalLoc,    "normal");
		glBindAttribLocation(getId(), Renderable::v_uvLoc,        "texCoord");
		glBindAttribLocation(getId(), Renderable::v_bitangentLoc, "bitangent");

		// output 
		glBindFragDataLocation(getId(), GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE,  "diffuseTexData");
		glBindFragDataLocation(getId(), GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL,   "normalTexData");
		glBindFragDataLocation(getId(), GBuffer::GBUFFER_TEXTURE_TYPE_SPECULAR, "specularTexData");
			
		link();
		
		// UBOs
		uboBindings.resize(UBO_SIZE);
		setUBObinding(UBO_MATRICES,  "Matrices",  Renderable::ubo_matrices);
		setUBObinding(UBO_MATERIALS, "Material",  Renderable::ubo_material);				

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

		geometrySubroutines.resize(SUBUNI_GEOM_SIZE);
		geometrySubroutines[areaCalculation] = std::unique_ptr<ShaderSubroutine>(new AreaCalculatorShaderSub());

		initSubroutines();
	};

	~DRgeometryPassShader() {};
	///////////////////////////////////////////////////////////////////////	
	inline void setAreaCalculation(AreaCalculatorShaderSub::OptionValues aoid) { setGeometrySubroutine(areaCalculation, aoid); }
	inline void setTextureMapping(bool enabled)   {	setBoolUniform(textureMappingEnabled,  enabled); };	
	inline void setNormalMapping(bool enabled)    {	setBoolUniform(normalMappingEnabled,   enabled); };
	inline void setSpecularMapping(bool enabled)  {	setBoolUniform(specularMappingEnabled, enabled); };
	inline void setOpacityMapping(bool enabled)   {	setBoolUniform(opacityMappingEnabled,  enabled); };
};

#endif
