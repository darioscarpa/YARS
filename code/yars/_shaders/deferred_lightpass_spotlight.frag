#version 420

#include ubo_matrices.inc
#include ubo_materials.inc
#include ubo_lights.inc
#include ubo_deferred.inc
#include lighting.inc

/////////////////////////////////////////////////////

#include gbuffer.inc

/////////////////////////////////////////////////////

uniform int spotLightId;

/////////////////////////////////////////////////////
// input

// all input is taken by texture sampling

/////////////////////////////////////////////////////
// output

layout(location = 0) out vec4 FragColor;

/////////////////////////////////////////////////////

void main() {	
	/////////////////////////////////////////////
	
	vec2 coords = vec2(gl_FragCoord.x * invScreenSize.x, gl_FragCoord.y * invScreenSize.y);

	/////////////////////////////////////////////
	
	vec3 diffuseColor = gbufGetDiffuseColor(coords);

	vec3 specularColor;
	float shininess;
	gbufGetSpecularColor(coords, specularColor, shininess);

	vec3 normalCS = gbufGetNormal(coords);

	vec3 eyeCS = -gbufGetPosition(coords);

	/////////////////////////////////////////////		

    vec3 n = normalize(normalCS);	    
 
	MaterialDesc mtl = MaterialDesc(diffuseColor, vec3(0.0), specularColor, vec3(0.0), shininess);
		
	vec3 scattered;
    vec3 reflected;
	spotLight(spotLights[spotLightId], mtl, n, eyeCS, scattered, reflected);
	
	vec4 color = vec4( scattered + reflected, 1.0);
	
    color = clamp( color, 0.0, 1.0 );
	color.a = 1.0;
	
	FragColor = color;

	/////////////////////////////////////////////	
}
