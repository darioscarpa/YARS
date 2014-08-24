#ifndef LIGHTING_INC
#define LIGHTING_INC



//////////////////////////////////////////////////////////////////

void pointLight(const in PointLight pl, const in MaterialDesc surfaceMtl, const in vec3 normal, const in vec3 eyeCS, 
	            out vec3 scatteredLight, out vec3 reflectedLight) {
//void pointLight(in int i, in vec3 normal, in vec3 eye, in vec4 posCS) {
//void pointLight(in int j, in vec3 normal, in vec4 eyeCS) {
   float nDotVP;       // normal . light direction
   float nDotHV;       // normal . light half vector
   float pf;           // power factor
   float attenuation;  // computed attenuation factor
   float d;            // distance from surface to light source
   vec3  VP;           // direction from surface to light position
   vec3  halfVector;   // direction of maximum highlights

   // Compute vector from surface to light position
   //VP = vec3(pointLights[i].posCS - posCS);

   //PointLight pl = pointLights[j];

   vec3 posCS = -eyeCS;

   //VP = vertexIn.pointLightsDirCS[j];
   //VP = vec3(pl.posCS) - vec3(posCS);
   
   VP = pl.posCS - posCS;
  
   //vec3 eye = normalize(eyeCS.xyz);
   vec3 eye = normalize(eyeCS);

   // Compute distance between surface and light position
   d = length(VP);

   // Normalize the vector from surface to light position
   VP = normalize(VP);

   // Compute attenuation
   attenuation = 1.0 / 
      (pl.constantAttenuation        +
       pl.linearAttenuation    * d   +
       pl.quadraticAttenuation * d * d);
	      
   halfVector = normalize(VP + eye);

   nDotVP = max(0.0, dot(normal, VP));
   nDotHV = max(0.0, dot(normal, halfVector));

   if (nDotVP == 0.0) {
       pf = 0.0;
   } else {
       pf = pow(nDotHV, surfaceMtl.shininess);
   }
    
   scatteredLight = pl.color * surfaceMtl.diffuse  * nDotVP * attenuation;
   reflectedLight = pl.color * surfaceMtl.specular * pf     * attenuation;

   //scatteredLight = pl.color * surfaceMtl.diffuse * nDotVP;
   //reflectedLight = pl.color * surfaceMtl.specular * pf;   
}




void spotLight(const in SpotLight sl, const in MaterialDesc surfaceMtl, const in vec3 normal, const in vec3 eyeCS, 
	            out vec3 scatteredLight, out vec3 reflectedLight) {

	vec3 posCS     = -eyeCS;
	vec3 lightDir  = sl.posCS - posCS;
    
	float d        = length(lightDir);

    vec3 L = normalize(lightDir);
    vec3 D = normalize(sl.dirCS);

    float cos_cur_angle = dot(L, D);
	float cos_outer_cone_angle = sl.cosCutoffOuter; //sl.cosCutoffInner - 0.05;

    float cos_inner_cone_angle = sl.cosCutoffInner;

    float cos_inner_minus_outer_angle =  cos_inner_cone_angle - cos_outer_cone_angle;

    float spot = clamp((cos_cur_angle - cos_outer_cone_angle) / cos_inner_minus_outer_angle, 0.0, 1.0);
    
    vec3 N = normal;

	scatteredLight = vec3(0.0, 0.0, 0.0);
	reflectedLight = vec3(0.0, 0.0, 0.0);
    float lambertTerm = max( dot(N,L), 0.0 );
    if(lambertTerm > 0.0)  {
		// Compute attenuation
		float attenuation = 1.0 / 
			(sl.constantAttenuation       +
			sl.linearAttenuation    * d   +
			sl.quadraticAttenuation * d * d);

        scatteredLight = sl.color * surfaceMtl.diffuse *  lambertTerm * attenuation * spot;

        vec3 E = normalize(eyeCS);

		//proper calc
        //vec3 R = reflect(-L, N);
        //float specular = pow( max(dot(R, E), 0.0), surfaceMtl.shininess );

		//half vector
		vec3 halfVector = normalize(L + E);
		float specular = pow( max(dot(N, halfVector), 0.0), surfaceMtl.shininess );

        reflectedLight = sl.color * surfaceMtl.specular * specular * attenuation * spot;
    }    
}

void dirLight(const in DirLight dl, const in MaterialDesc surfaceMtl,
               const in vec3 normal, const in vec3 eyeCS, 
	            out vec3 scatteredLight, out vec3 reflectedLight) {
   float nDotVP;       // normal . light direction
   float nDotHV;       // normal . light half vector
   float pf;           // power factor
   vec3  halfVector;   // direction of maximum highlights

   vec3 eye = normalize(eyeCS);
   halfVector = normalize(dl.dirCS + eye);

   nDotVP = max(0.0, dot(normal, dl.dirCS));
   nDotHV = max(0.0, dot(normal, halfVector));

   if (nDotVP == 0.0) {
       pf = 0.0;
   } else {
       pf = pow(nDotHV, surfaceMtl.shininess);
   }
    
   
   scatteredLight = dl.color * surfaceMtl.diffuse  * nDotVP ;
   reflectedLight = dl.color * surfaceMtl.specular * pf     ;
   
   scatteredLight = surfaceMtl.diffuse;
   reflectedLight = surfaceMtl.specular;
   

   //scatteredLight = surfaceMtl.diffuse  * nDotVP  ;
   //reflectedLight = surfaceMtl.specular * pf ;
}



void calcAllDirectLighting(const in MaterialDesc surfaceMtl, const in vec3 normal, const in vec3 eyeCS, 
	                       out vec3 scatteredLight, out vec3 reflectedLight) {
	vec3 scattered = vec3(0);
	vec3 reflected = vec3(0);

	scatteredLight = vec3(0);
	reflectedLight = vec3(0);

	for (int i = 0 ; i < numPointLights; ++i) {
		pointLight(pointLights[i], surfaceMtl, normal, eyeCS, scattered, reflected);
		scatteredLight += scattered;
		reflectedLight += reflected;

		//scatteredLight += vec3(1.0, 0.0, 0.0);
		//reflectedLight += vec3(1.0, 0.0, 0.0);
	}	

	for (int i = 0 ; i < numSpotLights; ++i) {
		spotLight(spotLights[i], surfaceMtl, normal, eyeCS, scattered, reflected);
		scatteredLight += scattered;
		reflectedLight += reflected;

		//scatteredLight += vec3(0.0, 1.0, 0.0);
		//reflectedLight += vec3(0.0, 1.0, 0.0);
	}		
	for (int i = 0 ; i < numDirLights; ++i) {
		dirLight(dirLights[i], surfaceMtl, normal, eyeCS, scattered, reflected);
		scatteredLight += scattered;
		reflectedLight += reflected;

		//scatteredLight += vec3(0.0, 0.0, 1.0);
		//reflectedLight += vec3(0.0, 0.0, 1.0);
	}
}


/*
void directionalLight(in int i, in vec3 normal) {
   float nDotVP;         // normal . light direction
   float nDotHV;         // normal . light half vector
   float pf;             // power factor

   nDotVP = max(0.0, dot(normal, normalize(vec3 (gl_LightSource[i].position))));
   nDotHV = max(0.0, dot(normal, vec3 (gl_LightSource[i].halfVector)));

   if (nDotVP == 0.0) {
       pf = 0.0;
   } else {
       pf = pow(nDotHV, gl_FrontMaterial.shininess);
   }
   Ambient  += gl_LightSource[i].ambient;
   Diffuse  += gl_LightSource[i].diffuse * nDotVP;
   Specular += gl_LightSource[i].specular * pf;
}

*/

	/*
	SpotLight sl;
	sl.color = vec3(1.0, 1.0, 1.0);
	sl.posCS = vec3(-0.04, -0.16, -0.6);
	sl.dirCS = vec3(0.04, -0.02, 0.99);

	sl.cosCutoffOuter = 0.951;	
	sl.cosCutoffInner = 0.965;

	sl.exponent = 0.0;
	sl.constantAttenuation = 1.0;
	sl.linearAttenuation = 0.02;
	sl.quadraticAttenuation  = 0.08;
	
	*/
	

/*

void xspotLight(in SpotLight sl, in MaterialDesc surfaceMtl, in vec3 normal, in vec3 eyeCS, 
	            out vec3 scatteredLight, out vec3 reflectedLight) {

   float nDotVP;       // normal . light direction
   float nDotHV;       // normal . light half vector
   float pf;           // power factor
   float attenuation;  // computed attenuation factor
   float d;            // distance from surface to light source
   vec3  VP;           // direction from surface to light position
   vec3  halfVector;   // direction of maximum highlights

   vec3 posCS = -eyeCS;
   
   VP = sl.posCS - posCS;
  
   vec3 eye = normalize(eyeCS);

   // Compute distance between surface and light position
   d = length(VP);

   // Normalize the vector from surface to light position
   VP = normalize(VP);

   // Compute attenuation
   attenuation = 1.0 / 
      (sl.constantAttenuation        +
       sl.linearAttenuation    * d   +
       sl.quadraticAttenuation * d * d);
	      
   float spotCos = dot(VP, sl.dirCS);
     
   if (spotCos < sl.cosCutoffInner) {
		attenuation = 0.0;
   } else {
	    attenuation *= pow(spotCos, sl.exponent);
   }
   
   halfVector = normalize(VP + eye);

   nDotVP = max(0.0, dot(normal, VP));
   nDotHV = max(0.0, dot(normal, halfVector));

   if (nDotVP == 0.0) {
       pf = 0.0;
   } else {
       pf = pow(nDotHV, surfaceMtl.shininess);
   }
    
   scatteredLight = sl.color * surfaceMtl.diffuse  * nDotVP * attenuation;
   reflectedLight = sl.color * surfaceMtl.specular * pf     * attenuation;    
}
*/
//////////////////////////////////////////////////////////////////
#endif
