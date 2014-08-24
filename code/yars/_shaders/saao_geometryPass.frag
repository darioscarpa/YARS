#version 420

#include ubo_matrices.inc
#include ubo_materials.inc
#include ubo_lights.inc

#include lighting.inc

// diffuse texture
uniform	sampler2D texUnit;
uniform bool textureMappingEnabled;

// normal mapping
uniform	sampler2D normalMapUnit;
uniform bool normalMappingEnabled;

// specular mapping
uniform	sampler2D specularMapSampler;
uniform bool specularMappingEnabled;

// alpha masking
uniform	sampler2D alphaMaskSampler;
uniform bool alphaMappingEnabled;

/////////////////////////////////////////////////////
// input

in VertexData {
	vec4 posCS;
	vec2 texCoord;

	vec3 normalCS;
	vec3 tangentCS;
	vec3 bitangentCS;	
	
	vec4 eyeCS;

	//dssao
	float diskArea;
} vertexIn;

/////////////////////////////////////////////////////
// output

layout(location = 0) out vec4 diffuseTexData;
layout(location = 1) out vec4 normalTexData;
layout(location = 2) out vec4 specularTexData;


/////////////////////////////////////////////////////

void main() {
	/////////////////////////////////////////////
	vec4 opacityMapColor  = texture(alphaMaskSampler, vertexIn.texCoord);
	vec4 diffuseTexColor  = texture(texUnit, vertexIn.texCoord);
	vec3 normalMapColor   = texture(normalMapUnit, vertexIn.texCoord).rgb;
	vec4 specularMapColor = texture(specularMapSampler, vertexIn.texCoord);	
	/////////////////////////////////////////////
	if (alphaMappingEnabled && opacityMapId > 0 && opacityMapColor.r < 0.2) {
		discard;		
	}
	/////////////////////////////////////////////
	vec4 diffuseColor  = diffuse;
	vec4 specularColor = specular;
	vec4 emissiveColor = emissive;
	/////////////////////////////////////////////
	if (textureMappingEnabled && diffuseTexId > 0) {
		diffuseColor = diffuseTexColor;
	}
	/////////////////////////////////////////////
	
    vec3 n = normalize(vertexIn.normalCS);
    
	if (normalMappingEnabled && normalMapId > 0) {	// bring tangent space normal to CS
		// don't ask me about this
		vec3 normalSample = normalMapColor;
		normalSample.z = -normalSample.z;
		n = normalize(normalSample*2.0 - 1.0);

		// calc transformation from TS to CS
		vec3 bitangCS = normalize(vertexIn.bitangentCS);
		vec3 normCS   = normalize(vertexIn.normalCS);
		vec3 tangCS   = normalize(cross(bitangCS,normCS));
		mat3 TBN = mat3(tangCS, bitangCS, normCS);

		// apply transformation
		n = TBN * n;
		n = -normalize(n);		
	} 
	
	/////////////////////////////////////////////	
	if (specularMappingEnabled && specularMapId > 0) {			
		specularColor = specularMapColor;
	}			
	/////////////////////////////////////////////
	
	diffuseTexData   = diffuseColor;
	specularTexData  = vec4(specularColor.rgb, shininess);
    normalTexData    = vec4(n, vertexIn.diskArea);
}