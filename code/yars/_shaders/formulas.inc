#ifndef FORMULAS_INC
#define FORMULAS_INC

//////////////////////////////////////////////////////////////////
// BUNNELL
//////////////////////////////////////////////////////////////////

// CG: float3 result = saturate(texCol0.rgb - Density*(texCol1.rgb));
// GLSL equivalent: vec3 result   = clamp(texCol0.rgb - Density*(texCol1.rgb), 0.0, 1.0);

// Bunnell code (GLSL conversion) -----------------------------------------------------------------
float bunnell_calcSolidAngle(
                     const in vec3 v,               // normalize( emitter.pos - receiver.pos )
                     const in float d2,             // distance between emitter and receiver, squared (must not be 0)
                     const in vec3 receiverNormal,
                     const in vec3 emitterNormal, 
                     const in float emitterArea     // emitter area / pi
                    ) {
    return ( 1 - inversesqrt( emitterArea/d2 + 1) )        * 
           clamp(     dot( emitterNormal,  v ), 0.0, 1.0 ) *
           clamp( 3 * dot( receiverNormal, v ), 0.0, 1.0 );
}

// element-to-element radiance transfer
float bunnell_calcFormFactor(
                 const in vec3 v,
				 const in float d2, 
				 const in vec3 receiverNormal,
				 const in vec3 emitterNormal,
				 const in float emitterArea
				 ) {
	return emitterArea * 
	       clamp( dot( emitterNormal,  v ), 0.0, 1.0 ) *
		   clamp( dot( receiverNormal, v ), 0.0, 1.0 ) / ( d2 + emitterArea );
}
// ------------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////
// SPATOLA
//////////////////////////////////////////////////////////////////

// disk to disk form factor (geometrical relation between emitter and receiver disk)
float formFactor(const in vec3 V, 
				 const in float dist,				// distance between disks
				 const in vec3 receiverNormal,		// receiver orientation
				 const in vec3 emitterNormal,		// emitter orientation
				 const in float emitterArea) {		// emitter area
	
	vec3 toEmitterDisk = normalize(V); // direction from receiver to emitter
	

	// angle between the two disks
	float normalDiff = clamp( dot(receiverNormal, emitterNormal), 0.0, 1.0);

	float dd;

	if ( normalDiff > 0.95f ) { // almost 90°
		dd = diskDisplacement; 
	} else {
		dd = 0.0f;
	}

	//return (emitterArea * saturate(dot(receiverNormal, toEmitterDisk) - dd)) / (3.14159265 * (dist*dist) + emitterArea);
	
	// (Aj * cos Ti) / ((pi * d^2) + A)
	return (emitterArea * clamp( dot(receiverNormal, toEmitterDisk) - dd, 0.0, 1.0) ) / (3.14159265 * (dist*dist) + emitterArea);
}

//////////////////////////////////////////////////////////////////

float dssoFormFactor(
                 const in vec3 V, 
				 const in float dist,				// distance between disks
				 const in vec3 receiverNormal,		// receiver orientation
				 const in vec3 emitterNormal,		// emitter orientation
				 const in float emitterArea) {		// emitter area
	
	vec3 toEmitterDisk = normalize(V); // direction from receiver to emitter
	

	// angle between the two disks
	float normalDiff = clamp( dot(receiverNormal, emitterNormal), 0.0, 1.0);

	float dd;

	if ( normalDiff > 0.95f ) { // almost 90°
		dd = diskDisplacement; 
	} else {
		dd = 0.0f;
	}

	//return (emitterArea * saturate(dot(receiverNormal, toEmitterDisk) - dd)) / (3.14159265 * (dist*dist) + emitterArea);
	
	// (Aj * cos Ti * cos Ri) / ((pi * d^2) + A)
	return (emitterArea *
	        clamp( dot(receiverNormal, toEmitterDisk) - dd, 0.0, 1.0) *
			clamp( dot(emitterNormal, toEmitterDisk)  - dd, 0.0, 1.0) 
		   ) / (dist*dist)*3.14;
}

float diskToPointFormFactor(
                 const in vec3  delta,               // vector between point and disk center
				 const in vec3  receiverNormal,		// receiver orientation
				 const in vec3  emitterNormal,		// emitter orientation
				 const in float emitterArea) {		// emitter area
	 
	 vec3 normalizedDelta = normalize(delta);
	 float ff = 
	   max(0.0, dot( normalizedDelta, -receiverNormal ))
	 * max(0.0, dot ( normalizedDelta, emitterNormal ))
	 / dot (delta, delta);
	
	return ff;
}
//////////////////////////////////////////////////////////////////


float solidAngle_simple( const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea, 
		                 const in vec3 receiverPosition, const in vec3 receiverNormal) {

	vec3 V = receiverPosition - emitterPosition;
	float Vlen = length(V);
	float distSquared = Vlen*Vlen;
	V = normalize(V);
	vec3 N = normalize(emitterNormal);

	
	float coveredSolidAngle = clamp( dot( N,  V ), 0.0, 1.0) * emitterArea / distSquared;
	
	//float coveredSolidAngle =  emitterArea / distSquared;

	/*
	vec3 V = emitterPosition - receiverPosition;
	float d2 = dot(V, V) + 1e-16;
	V *= inversesqrt(d2);
	float coveredSolidAngle = clamp( dot( -N,  V ), 0.0, 1.0) * emitterArea / d2;
	*/	

	//return 0.03;
	return coveredSolidAngle;
}


float solidAngle_falloff(const in float t) {
	float u = 0.1 * radLength;
	float falloff = u * t * pow(max(u, t) , -2);
	return falloff;
}

/*
float solidAngle_approx( const in float cosrtheta, const in float dist, const in vec3 recToEmi, const in vec3 recToEmiNorm ) {
	float distSquared = dist*dist + 1e-16;
	
	float coveredSolidAngle = (dotp * emitterArea) / distSquared;
	float falloff = solidAngle_falloff(Vlen);
	coveredSolidAngle *= falloff;
	
	return coveredSolidAngle;
}
float solidAngle_approx2( const in float cosrtheta, const in float dist, const in vec3 recToEmi, const in vec3 recToEmiNorm ) {
	float distSquared = dist*dist + 1e-16;
	
	float dotp = abs(dot( receiverNormal,  V ));
	float dotp2 = clamp(dot( receiverNormal,  V ), 0.0, 1.0) ;
		
	//const float angleBias = 0.30;
	//if (dotp < angleBias) return 0.0;

	if (dotp < diskDisplacement) return 0;
	
	
	//float coveredSolidAngle = (dotp * emitterArea) / distSquared;
	float coveredSolidAngle = (dotp * dotp2 * emitterArea) / distSquared;

	float falloff = solidAngle_falloff(Vlen);
	coveredSolidAngle *= falloff;
	
	return coveredSolidAngle;
}
*/
float solidAngle_approx( const in vec3 emitterPosition,  const in vec3 emitterNormal,  const in float emitterArea, 
		                 const in vec3 receiverPosition, const in vec3 receiverNormal) {

	vec3 V = emitterPosition - receiverPosition;
	float Vlen = length(V);
	float distSquared = Vlen*Vlen + 1e-16;
	V = normalize(V);
	//receiverNormal = normalize(receiverNormal);

	float dotp = clamp(dot( receiverNormal,  V ), 0.0, 1.0) ;
		
	const float angleBias = 0.30;
	if (dotp < angleBias) return 0.0;
	
	float coveredSolidAngle = (dotp * emitterArea) / distSquared;
	
	float falloff = solidAngle_falloff(Vlen);
	coveredSolidAngle *= falloff;
	
	return coveredSolidAngle;
}

float solidAngle_approx2( const in vec3 emitterPosition,  const in vec3 emitterNormal, const in float emitterArea, 
		                  const in vec3 receiverPosition,  const in vec3 receiverNormal) {
	vec3 V = emitterPosition - receiverPosition;
	float Vlen = length(V);
	float distSquared = Vlen*Vlen + 1e-16;
	V = normalize(V);
	//receiverNormal = normalize(receiverNormal);

	//float dotp = clamp(dot( receiverNormal,  V ), 0.0, 1.0) ;
	//float dotp = clamp(dot( emitterNormal,  -V ), 0.0, 1.0) ;
	
	float dotp = abs(dot( receiverNormal,  V ));
	float dotp2 = clamp(dot( receiverNormal,  V ), 0.0, 1.0) ;
		
	//const float angleBias = 0.30;
	//if (dotp < angleBias) return 0.0;

	if (dotp < diskDisplacement) return 0;
	
	
	//float coveredSolidAngle = (dotp * emitterArea) / distSquared;
	float coveredSolidAngle = (dotp * dotp2 * emitterArea) / distSquared;

	float falloff = solidAngle_falloff(Vlen);
	coveredSolidAngle *= falloff;
	
	return coveredSolidAngle;
}

float alchemy( const in vec3 emitterPosition, const in vec3 receiverPosition, const in vec3 receiverNormal) {

	vec3 V = emitterPosition - receiverPosition;
	float dotp = max( 0, dot(V, receiverNormal) + receiverPosition.z * 0.0001);
	float dist2 = dot(V,V);
	return dotp/dist2;
}


float formFactor_d2d( const in vec3 emitterPosition, const in vec3 emitterNormal, const in float emitterArea,
				 	  const in vec3 receiverPosition, const in vec3 receiverNormal, const in float receiverArea) {
	vec3 v = receiverPosition - emitterPosition;
	float vlen = length(v);
	float vlen2 = vlen * vlen;
	return emitterArea * 
	       clamp( dot( emitterNormal,  v ), 0.0, 1.0 ) *
		   clamp( dot( receiverNormal, -v ), 0.0, 1.0 ) / ( 3.14159265 * vlen2 + emitterArea );
}

#endif
