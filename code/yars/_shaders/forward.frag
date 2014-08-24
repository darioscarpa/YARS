#version 420

#include ubo_matrices.inc
#include ubo_materials.inc
#include ubo_lights.inc


/////////////////////////////////////////////////////

//uniform vec3 directionalLightDir;

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

uniform float ambientLightLevel;
uniform bool enabledLighting;
/////////////////////////////////////////////////////
// input

in VertexData {
	//vec3 singlepointLightDirCS;
	//vec3 pointLightDirCS;

	//vec3 pointLightsDirCS[POINT_LIGHTS_MAX];

	vec3 normalCS;
	vec3 tangentCS;
	vec3 bitangentCS;
	
	vec2 TexCoord;

	vec4 eyeCS;
} vertexIn;

/////////////////////////////////////////////////////
// output

layout(location = 0) out vec4 outputcol;

/////////////////////////////////////////////////////

#include lighting.inc

void main() {	
	/////////////////////////////////////////////
	vec4 opacityMapColor  = texture(alphaMaskSampler, vertexIn.TexCoord);
	vec4 diffuseTexColor  = texture(texUnit, vertexIn.TexCoord);
	vec3 normalMapColor   = texture(normalMapUnit, vertexIn.TexCoord).rgb;
	vec4 specularMapColor = texture(specularMapSampler, vertexIn.TexCoord);	
	/////////////////////////////////////////////
	vec4 diffuseColor  = diffuse;	
	vec4 specularColor = specular;	
	vec3 ambientColor  = vec3(diffuse); // vec3(ambient); //
	vec4 emissiveColor = emissive;
	/////////////////////////////////////////////	
	if (alphaMappingEnabled && opacityMapId > 0 ) {
		if (opacityMapColor.r < 0.2) 
			discard;		
	}
	/////////////////////////////////////////////	
	if (textureMappingEnabled && diffuseTexId > 0) {
		diffuseColor = diffuseTexColor;
		ambientColor = diffuseTexColor.rgb;		
	}
	/////////////////////////////////////////////		
	if (specularMappingEnabled && specularMapId > 0) {			
		specularColor = specularMapColor;		
	}			
    /////////////////////////////////////////////	
    vec3 n = normalize(vertexIn.normalCS);
     	
	if (normalMappingEnabled && normalMapId > 0) {	// bring tangent space normal to CS
		// don't ask me about this
		vec3 normalSample = normalMapColor;
		normalSample.b = -normalSample.b;
		n = normalize(normalSample*2.0 - 1.0);

		// calc transformation from TS to CS
		//vec3 tangCS = normalize(vertexIn.tangentCS);
		vec3 bitangCS = normalize(vertexIn.bitangentCS);
		vec3 normCS = normalize(vertexIn.normalCS);
		//vec3 bitangCS = normalize(cross(tangCS, normCS));
		vec3 tangCS = normalize(cross(bitangCS,normCS));
		mat3 TBN = mat3(tangCS, bitangCS, normCS);

		// apply transformation
		n = TBN * n;
		n = -normalize(n);		
	} 
	/////////////////////////////////////////////
	    
    // light intensity factors init
    //vec3 Ambient  = vec3(ambientLightLevel);
    vec3 Diffuse  = vec3(0.0);
    vec3 Specular = vec3(0.0);

	MaterialDesc mtl = MaterialDesc(diffuseColor.rgb, ambientColor.rgb, specular.rgb, emissive.rgb, shininess);
	
	/*MaterialDesc mtl;
	mtl.diffuse   = diffuseColor.rgb;
	mtl.ambient   = ambientColor.rgb;
	mtl.specular  = specularColor.rgb;
	mtl.emissive  = emissiveColor.rgb;
	mtl.shininess = shininess; // from UBO
	*/
	
	if (enabledLighting) {		
		calcAllDirectLighting(mtl, n, vec3(vertexIn.eyeCS), Diffuse, Specular);	
	}
	  
	vec3 Ambient =  ambientColor * ambientLightLevel;

	vec3 color = Ambient + Diffuse + Specular;
    color = clamp( color, 0.0, 1.0 );
    
	outputcol = vec4(color, 1.0);

	//outputcol.rgb = mtl.diffuse; //Diffuse;
	//outputcol.rgb = mtl.specular; //Diffuse;

	//output.rgb = Specular.rgb;
	//output.xyz = n;	
}
