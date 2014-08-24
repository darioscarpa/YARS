#ifndef DEFERRED_LIGHTING_INC
#define DEFERRED_LIGHTING_INC

//////////////////////////////////////////////////////////////////

vec3 calcDirectLight(const in vec2 coords) {	
	float zvalue = gbufGetDepth(coords);
	if (zvalue == 1.0) {
		discard; //FragColor = vec4(1.0, 0.0, 0.0, 1.0); return;
	}
		
	/////////////////////////////////////////////	
	vec3 diffuseColor = gbufGetDiffuseColor(coords);
	vec3 specularColor;
	float shininess;
	gbufGetSpecularColor(coords, specularColor, shininess);

	vec3 normalCS = gbufGetNormal(coords);

	vec3 eyeCS = -gbufGetPosition(coords);
	/////////////////////////////////////////////		

    vec3 n = normalize(normalCS);
    
    // Clear the light intensity accumulators
    vec3 Diffuse  = vec3(0.0);
    vec3 Specular = vec3(0.0);
	
	MaterialDesc mtl = MaterialDesc(diffuseColor, vec3(0.0), specularColor, vec3(0.0), shininess);
	
	/*MaterialDesc mtl;	
	mtl.diffuse   = diffuseColor; 
	mtl.ambient   = vec3(0.0);
	mtl.specular  = specularColor;
	mtl.emissive  = vec3(0.0);
	mtl.shininess = shininess; 
	*/
	
	calcAllDirectLighting(mtl, n, eyeCS, Diffuse, Specular);
	
	vec3 color;		
	color = vec3( Diffuse );
	color = clamp( color, 0.0, 1.0 );
	return color;
	//return color*vec3(0.5, 1.0, 0.5);
	//return vec3(0.0, 0.7, 0.0);
}

//////////////////////////////////////////////////////////////////

#endif