#version 420

#include ubo_matrices.inc
#include ubo_materials.inc
#include ubo_lights.inc
#include ubo_deferred.inc
#include lighting.inc

#include gbuffer.inc

/////////////////////////////////////////////////////
// input

// all input is taken by texture sampling

/////////////////////////////////////////////////////
// output

layout(location = 0) out vec4 ScatteredFragColor;
layout(location = 1) out vec4 ReflectedFragColor;

/////////////////////////////////////////////////////

void xmain() {
	ScatteredFragColor = vec4(1.0, 0.0, 0.0, 1.0);
	ReflectedFragColor = vec4(0.0, 1.0, 0.0, 1.0);
}


void main() {	
	/////////////////////////////////////////////
	
	vec2 coords = vec2(gl_FragCoord.x * invScreenSize.x, gl_FragCoord.y * invScreenSize.y);

	/////////////////////////////////////////////
	
	vec3 diffuseColor = gbufGetDiffuseColor(coords);

	/*vec3 specularColor = vec3(0);
	float shininess = 0.0;
	gbufGetSpecularColor(coords, specularColor, shininess);*/

	vec3 specularColor = gbufGetSpecularColorX(coords);
	float shininess = gbufGetSpecularShininess(coords);

	vec3 normalCS = gbufGetNormal(coords);

	vec3 eyeCS = -gbufGetPosition(coords);

	/////////////////////////////////////////////

    vec3 n = normalize(normalCS);	    
  
	MaterialDesc mtl = MaterialDesc(diffuseColor, vec3(0.0), specularColor, vec3(0.0), shininess);
	/*mtl.diffuse   = diffuseColor;
	mtl.ambient   = vec3(0.0);
	mtl.specular  = specularColor;
	mtl.emissive  = vec3(0.0);
	mtl.shininess = shininess; 	*/

	/////////////////////////////////////////////

	vec3 scattered = vec3(0);
    vec3 reflected = vec3(0);
	calcAllDirectLighting(mtl, n, eyeCS, scattered, reflected);	

	/////////////////////////////////////////////

	ScatteredFragColor = vec4(scattered, 0.0);
	ReflectedFragColor = vec4(reflected, 0.0);

	//ScatteredFragColor.rgb = scattered;
	//ReflectedFragColor.rg = vec2(0.0);
	
	//ScatteredFragColor = vec4(mtl.diffuse, 1.0);
	//ReflectedFragColor = vec4(mtl.specular, 1.0);

}

