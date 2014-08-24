#version 420

#include ubo_matrices.inc
#include ubo_materials.inc

uniform vec3 directionalLightDir;

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
	vec3  triCentroid;

} vertexIn;

/////////////////////////////////////////////////////
// output
/*layout(location = 0) out vec4 positionTexData;
layout(location = 1) out vec4 diffuseTexData;
layout(location = 2) out vec4 normalTexData;
layout(location = 3) out vec4 specularTexData;
//layout(location = 4) out vec4 uvTexData;
*/

/*
out vec4 diffuseTexData;
out vec4 normalTexData;
out vec4 specularTexData;
*/

layout(location = 0) out vec4 diffuseTexData;
layout(location = 1) out vec4 normalTexData;
layout(location = 2) out vec4 specularTexData;

/////////////////////////////////////////////////////


void xmain() {
	vec4 opacityMapColor  = texture(alphaMaskSampler, vertexIn.texCoord);
	vec4 diffuseTexColor  = texture(texUnit, vertexIn.texCoord);
	vec3 normalMapColor   = texture(normalMapUnit, vertexIn.texCoord).rgb;
	vec4 specularMapColor = texture(specularMapSampler, vertexIn.texCoord);
	
	//positionTexData = vertexIn.posCS;
	diffuseTexData  = diffuseTexColor;
	specularTexData = specularMapColor;
    normalTexData   = vec4(normalize(vertexIn.normalCS), 1.0);
    //uvTexData       = vec4(vertexIn.texCoord, 0.0, 1.0);	
}


void main() {
	/////////////////////////////////////////////
	vec4 opacityMapColor  = texture(alphaMaskSampler, vertexIn.texCoord);
	vec4 diffuseTexColor  = texture(texUnit, vertexIn.texCoord);
	vec3 normalMapColor   = texture(normalMapUnit, vertexIn.texCoord).rgb;
	vec4 specularMapColor = texture(specularMapSampler, vertexIn.texCoord);	
	/////////////////////////////////////////////
	vec4 diffuseColor  = diffuse;	
	vec4 specularColor = specular;	
	//vec4 ambientColor  = ambient;
	vec4 emissiveColor = emissive;
	/////////////////////////////////////////////	
	if (alphaMappingEnabled && opacityMapId > 0 ) {
		//output.a = opacityMapColor.r;	
		if (opacityMapColor.r < 0.2) 
			discard;		
	}
	/////////////////////////////////////////////	
	if (textureMappingEnabled && diffuseTexId > 0) {
		diffuseColor = diffuseTexColor;
		//ambientColor = diffuseTexColor*0.3;
		//ambientColor.a = 1.0;
	}
	/////////////////////////////////////////////	
	
    vec3 n = normalize(vertexIn.normalCS);
    
	if (normalMappingEnabled && normalMapId > 0) {	// bring tangent space normal to CS
		// don't ask me about this
		vec3 normalSample = normalMapColor;
		normalSample.b = -normalSample.b;
		n = normalize(normalSample*2.0 - 1.0);

		// calc transformation from TS to CS
		//vec3 tangCS = normalize(tangentCS);
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
	if (specularMappingEnabled && specularMapId > 0) {			
		specularColor = specularMapColor;
	}			
	/////////////////////////////////////////////
    
	//positionTexData  = vertexIn.posCS;	
    diffuseTexData   = diffuseColor;
	diffuseTexData.a = vertexIn.diskArea;

	/*
	float circumscribedRadius = vertexIn.diskArea;
	float distanceToCentroid = length(vec3(vertexIn.posCS) - vertexIn.triCentroid);
	diffuseTexData.a = circumscribedRadius - distanceToCentroid; 
	*/
	
	//diffuseTexData.a = 1-vertexIn.diskArea;
	//diffuseTexData.a =  0.1/length(vec3(vertexIn.posCS) - vertexIn.triCentroid);
	//diffuseTexData.a = vec3( vertexIn.diskArea / (length(vec3(vertexIn.posCS) - vertexIn.triCentroid) )  );
	//diffuseTexData.a = vec3( vertexIn.diskArea - (length(vec3(vertexIn.posCS) - vertexIn.triCentroid) )  );
	//diffuseTexData.a = vec3( vertexIn.diskArea / (length(vec3(vertexIn.posCS) - vertexIn.triCentroid) )  );
	//diffuseTexData.a = vec3( (length(vec3(vertexIn.posCS) - vertexIn.triCentroid) ) /vertexIn.diskArea  );
	//diffuseTexData.rgb = vec3( vertexIn.diskArea * (length(vec3(vertexIn.posCS) - vertexIn.triCentroid) )  );
	//diffuseTexData.rgb = vec3( vertexIn.diskArea );

	specularTexData  = vec4(specularColor.rgb, shininess);
	
	//specularTexData = vec4(vertexIn.diskArea, 0.0, 0.0, 1.0)*3;

    normalTexData = vec4(n, 1.0);
    //uvTexData = vec4(vertexIn.texCoord, 0.0, 1.0);
}