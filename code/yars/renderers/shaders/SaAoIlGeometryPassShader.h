#ifndef SAAOILGEOMETRYPASSSHADER_H
#define SAAOILGEOMETRYPASSSHADER_H

#include "../SaAoIlGbuffer.h"

#include "../../ShaderHandling.h"
#include "../../Renderable.h"

#include "sub/AreaCalculatorShaderSub.h"



class SaAoIlGeometryPassShader : public GfxShaderProgram {
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

	enum GeometrySubroutineUniformLocId {
		areaCalculation,
		SUBUNI_GEOM_SIZE
	};

	inline GLuint u(UniformLocId i) const { return uniformLocs[i]; }

	///////////////////////////////////////////////////////////////////////
	SaAoIlGeometryPassShader() : GfxShaderProgram("saaoil_geometryPass", "deferred.vert", "saaoil_geometryPass.frag", "saao_geometryPass.geom") {
		// input
		glBindAttribLocation(getId(), Renderable::v_positionLoc,  "position");
		glBindAttribLocation(getId(), Renderable::v_normalLoc,    "normal");
		glBindAttribLocation(getId(), Renderable::v_uvLoc,        "texCoord");
		glBindAttribLocation(getId(), Renderable::v_bitangentLoc, "bitangent");

		// output
		/*
		glBindFragDataLocation(getId(), GdlBuffer::GDLBUFFER_TEXTURE_TYPE_ALBEDO,      "albedoTexData");
		glBindFragDataLocation(getId(), GdlBuffer::GDLBUFFER_TEXTURE_TYPE_DIRECTLIGHT, "directLightTexData");
		glBindFragDataLocation(getId(), GdlBuffer::GDLBUFFER_TEXTURE_TYPE_NORMAL,      "normalTexData");
		*/
		
		glBindFragDataLocation(getId(), SaAoIlGBuffer::GBUFFER_TEXTURE_TYPE_ALBEDO,      "albedoTexData");
		glBindFragDataLocation(getId(), SaAoIlGBuffer::GBUFFER_TEXTURE_TYPE_DIRECTLIGHT, "directLightTexData");
		glBindFragDataLocation(getId(), SaAoIlGBuffer::GBUFFER_TEXTURE_TYPE_NORMAL,      "normalTexData");
		

		link();

		// UBOs
		uboBindings.resize(UBO_SIZE);
		setUBObinding(UBO_MATRICES,  "Matrices",  Renderable::ubo_matrices);
		setUBObinding(UBO_MATERIALS, "Material",  Renderable::ubo_material);
		setUBObinding(UBO_LIGHTS,    "Lights",    Renderable::ubo_lights);

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

	~SaAoIlGeometryPassShader() {};
	///////////////////////////////////////////////////////////////////////
	inline void setAreaCalculation(AreaCalculatorShaderSub::OptionValues aoid) { setGeometrySubroutine(areaCalculation, aoid); }
	inline void setTextureMapping(bool enabled)   {	setBoolUniform(textureMappingEnabled,  enabled); };
	inline void setNormalMapping(bool enabled)    {	setBoolUniform(normalMappingEnabled,   enabled); };
	inline void setSpecularMapping(bool enabled)  {	setBoolUniform(specularMappingEnabled, enabled); };
	inline void setOpacityMapping(bool enabled)   {	setBoolUniform(opacityMappingEnabled,  enabled); };
};

#endif
