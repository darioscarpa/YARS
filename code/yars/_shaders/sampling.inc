#ifndef SAMPLING_INC
#define SAMPLING_INC

//////////////////////////////////////////////////////////////////

const int MAX_KERNEL_SIZE = 64;
uniform vec3  uKernelOffsets[MAX_KERNEL_SIZE];
uniform int   uKernelSkip[MAX_KERNEL_SIZE];
uniform float uKernelWeight[MAX_KERNEL_SIZE];
uniform float uKernelSolidAngle[MAX_KERNEL_SIZE];

vec2 samplesPos[MAX_KERNEL_SIZE]; // DANGER DANGER! global array of samples as texture coords ('cause used to sample the Gbuffer)
vec3 samplesPosCS[MAX_KERNEL_SIZE]; // DANGER DANGER! global array of samples as texture coords ('cause used to sample the Gbuffer)

//////////////////////////////////////////////////////////////////

#include random.inc
#include pyramid.inc
#include subroutinesConfig.inc


mat3 getOrthoBasis(const in vec2 coords, const in vec3 normal) {
	//Gram–Schmidt process to compute orthogonal basis
	
	//vec3 rvec = vec3(myrand(coords.xy), myrand(coords.yx), 1.0);

	//vec3 rvec      = vec3(0, -1, 1.0);
	vec3 rvec      = vec3(getRandom(coords), 1.0);

	//rvec = reflect(vec[i], randVec) * sampleRadius;
		

	//rvec = normalize(rvec);
	vec3 tangent   = normalize(rvec - normal * dot(rvec, normal));
	vec3 bitangent = cross(tangent, normal);

	// normal fills the z component 'cause the kernel has to be oriented along the z axis
	//mat3 kernelBasis = mat3(tangent, bitangent, normal);
	mat3 kernelBasis = mat3(bitangent, normal, tangent);
	return kernelBasis;
}

vec2 camSpaceToScreenSpaceTC(const in vec3 coordsCS) {
	vec4 offset = projMatrix * vec4(coordsCS, 1.0);
	offset.xy /= offset.w; // only need xy
	offset.xy = offset.xy * 0.5 + 0.5; // scale/bias to texcoords
	return offset.xy;	
}

vec3 calcCameraSpaceEmisphereSamplePos(const in int i, const in mat3 kernelBasis, const in vec3 originPos, const in float radLength) {
	// transform emisphere sampling vector to camera space
	vec3 samplePos = kernelBasis * uKernelOffsets[i] * radLength;
	samplePos = samplePos  + originPos;
	return samplePos;
}

vec2 calcScreenSpaceEmisphereSamplePos(const in int i, const in mat3 kernelBasis, const in vec3 originPos, const in float radLength) {

	// transform emisphere sampling vector to camera space
	vec3 samplePos = kernelBasis * uKernelOffsets[i] * radLength;
	samplePos = samplePos  + originPos;
		
	// project camera space sample position to screen space
	return camSpaceToScreenSpaceTC(samplePos);
}


float intPlane( const in vec3 rd ) {
	vec3 pl = -vec3(sign(rd.x), sign(rd.y), sign(rd.z));
	vec3 ro = vec3(0.0);
		
    float t = -(dot(pl.xyz, ro)+1.0) / (dot(pl.xyz, rd)+1e-5);
    return t;

	/*
	const float INFINITY = 1e10;

	float b = dot(pl.xyz, rd);
	if (abs(b) < 1e-5) {
        return INFINITY;
    }

	float a = -(dot(pl.xyz, ro)+1.0);

    float r = a / b;
    if (r < 0.0) {
        return INFINITY; 
	}
	return r;
	*/
}

/*
int getIntersectingTriangleId(const in vec3 occlusorToPointCS,
                              const in mat3 kernelBasis, 
							  const in vec3 emisphereCenter,
							  const in vec3 occlusorPos,							  
							  out vec3 ptheta) {
							  
    	
	// invk transforms from camera space to emishphere space
	//mat3 invk = inverse(kernelBasis);
	mat3 invk = transpose(kernelBasis); 

	vec3 dir = occlusorPos-emisphereCenter;
	dir = invk*dir;
	
	int triangleId;
	float intersectionDist = intPlane(dir);	
	ptheta =  dir * intersectionDist;

	//if (true) {	
	if (ptheta.y > 0) { //0.01) {
		float x = TRIANGLE_DIVS * ptheta.x * sign(dir.x);
		float z = TRIANGLE_DIVS * ptheta.z * sign(dir.z);

		int xi = int(floor(x));
		int zi = int(floor(z));

		int etheta = zi * (2*TRIANGLE_DIVS - zi) + 2 * xi;
	
		float diag = (x-xi)+(z-zi);
		if (diag > 1.0) ++etheta;

		triangleId = etheta;
	} else {
		triangleId = -1;
		ptheta = vec3(0.0);
	}
	ptheta = emisphereCenter + kernelBasis * ptheta * radLength;
	return triangleId;
}

int getGlobalIntersectingTriangleId(const in vec3 occlusorToPointCS,
                              const in mat3 kernelBasis, 
							  const in vec3 emisphereCenter,
							  const in vec3 occlusorPos,							  
							  out vec3 ptheta) {

	mat3 invk = transpose(kernelBasis); 
	vec3 dir = occlusorPos-emisphereCenter;
	dir = invk*dir;

	//vec3 dir = vec3(1,0,1);
	//vec3 dir = vec3(-1,0,1);
	//vec3 dir = vec3(-1,0,-1);
	//vec3 dir = vec3(1,0,-1);
	
	int triId  = getIntersectingTriangleId(occlusorToPointCS, kernelBasis, 
					emisphereCenter, occlusorPos, ptheta);
	int offset;
    if (triId != -1) {
		//float sx = sign(dir.x);
		//float sz = sign(dir.z);
		//if (sx > 0) {
		//	if (sz > 0) offset = 0;
		//		else offset = 3*TRIANGLE_DIVS_SQUARED;
		//} else {
		//	if (sz > 0) offset = 1*TRIANGLE_DIVS_SQUARED;
		//	else offset = 2*TRIANGLE_DIVS_SQUARED;
		//}
		//triId += offset;		
		triId += getQuadrantOffsetByCoords(dir);
		
	}
	return triId;
}

*/


int getGlobalIntersectingTriangleId(const in vec3 occlusorToPointCS,
                              const in mat3 kernelBasis, 
							  const in vec3 emisphereCenter,
							  const in vec3 occlusorPos,							  
							  out vec3 ptheta) {	    	
	// invk transforms from camera space to emishphere space
	//mat3 invk = inverse(kernelBasis);
	mat3 invk = transpose(kernelBasis); 

	vec3 dir = occlusorPos-emisphereCenter;
	dir = invk*dir;
	
	int triangleId;
	float intersectionDist = intPlane(dir);	
	ptheta =  dir * intersectionDist;

	//if (true) {	
	if (ptheta.y > 0) { //0.01) {
		float x = TRIANGLE_DIVS * ptheta.x * sign(dir.x);
		float z = TRIANGLE_DIVS * ptheta.z * sign(dir.z);

		int xi = int(floor(x));
		int zi = int(floor(z));

		int etheta = zi * (2*TRIANGLE_DIVS - zi) + 2 * xi;
	
		float diag = (x-xi)+(z-zi);
		if (diag > 1.0) ++etheta;

		triangleId = etheta;

		triangleId += getQuadrantOffsetByCoords(dir);
	} else {
		triangleId = -1;
		ptheta = vec3(0.0);
	}
	ptheta = emisphereCenter + kernelBasis * ptheta * radLength;
	return triangleId;
}

//////////////////////////////////////////////////////////////////

#ifdef USE_SUBROUTINE_SAMPLESLOCATOR

subroutine void    samplesLocator(const in vec2  pixelCoordsTC,
                                  const in int   oneEightOfSamples,
							      const in float maxSamplingRadius,
							      const in vec3  receiverPos,
							      const in vec3  receiverNormal
							      );
subroutine uniform samplesLocator calcSamplesPos;

#endif

#ifdef USE_SUBROUTINE_SAMPLESLOCATOR
subroutine (samplesLocator) 
#endif
void getRadialSamples(const in vec2  pixelCoordsTC, 
                      const in int   oneEightOfSamples,
					  const in float maxSamplingRadius,
					  const in vec3  receiverPos,
					  const in vec3  receiverNormal) {
	
	//vec3 position = gbufGetPosition(pixelCoordsTC);
	//float sampleRadius = maxSamplingRadius / -position.z;

	float sampleRadius = maxSamplingRadius / -receiverPos.z;
	

	const mat2 matrix45deg =  //{0.707106781, 0.707106781, -0.707106781, 0.707106781};
	  mat2(
			0.707106781, 0.707106781,
			-0.707106781, 0.707106781);

    const int directions = 4;
	const vec2 vec[4]    =  vec2[4]( vec2(1,0), vec2(-1,0), vec2(0,1), vec2(0,-1) );	//shared array

	int samplesPerDir = oneEightOfSamples * 2;
	int samples       = samplesPerDir     * directions;
	
	float increment = sampleRadius / samplesPerDir;
	for(int i = 0; i < directions; i++) {
		float offset = increment;
		vec2 rotdir = vec[i] * matrix45deg;
		
		for (int j = 0; j < samplesPerDir; j = j + 2) {
			samplesPos[i*samplesPerDir + j]     = (pixelCoordsTC + (vec[i] * offset)) ;
			samplesPos[i*samplesPerDir + j + 1] = (pixelCoordsTC + (rotdir * offset)) ;
			offset += increment;			
		}		
	}		
}

#ifdef USE_SUBROUTINE_SAMPLESLOCATOR
subroutine (samplesLocator) 
#endif
void getDssaoSamples(const in vec2  pixelCoordsTC,
                     const in int   oneEightOfSamples,
					 const in float maxSamplingRadius,
					 const in vec3  receiverPos,
					 const in vec3  receiverNormal) {
	
	vec2 randVec  = getRandom(pixelCoordsTC);
	
	//vec3 position = gbufGetPosition(pixelCoordsTC);	
	//float sampleRadius = maxSamplingRadius / -position.z;

	float sampleRadius = maxSamplingRadius / -receiverPos.z;

	const mat2 matrix45deg =  //{0.707106781, 0.707106781, -0.707106781, 0.707106781};
	  mat2(
			0.707106781, 0.707106781,
			-0.707106781, 0.707106781);

    const int directions = 4;
	const vec2 vec[4]    =  vec2[4]( vec2(1,0), vec2(-1,0), vec2(0,1), vec2(0,-1) );	//shared array
	
	int samplesPerDir = oneEightOfSamples * 2;
	int samples       = samplesPerDir     * directions;
	
	float increment = 1.0 / samples;
	
	
	for(int i = 0; i < directions; i++) {
		vec2 reflectedVec        = reflect(vec[i], randVec) * sampleRadius;
		vec2 reflectedRotatedVec = reflectedVec * matrix45deg;
	
		float offset = increment;
		for (int j = 0; j < samplesPerDir; j = j + 2) {
			samplesPos[i*samplesPerDir + j]      = pixelCoordsTC + (reflectedVec        * offset);
			offset += increment;
			samplesPos[i*samplesPerDir + j + 1]  = pixelCoordsTC + (reflectedRotatedVec * offset);			
			offset += increment;
		}
	}		
}


#ifdef USE_SUBROUTINE_SAMPLESLOCATOR
subroutine (samplesLocator) 
#endif
void getEmisphereSamples(const in vec2  pixelCoordsTC,
                         const in int   oneEightOfSamples,
					     const in float maxSamplingRadius,
					     const in vec3  receiverPosition,
					     const in vec3  receiverNormal) {
		
	///////////////////////////////////////////////////////////		
	mat3 kernelBasis = getOrthoBasis(pixelCoordsTC, receiverNormal);

	const int totsamples = oneEightOfSamples*8;
	for (int i = 0; i < totsamples; ++i) {		
		samplesPosCS[i] = calcCameraSpaceEmisphereSamplePos(i, kernelBasis, receiverPosition, radLength);
		samplesPos[i]   = camSpaceToScreenSpaceTC(samplesPosCS[i]);
	}
	///////////////////////////////////////////////////////////		
		
	/*
	//for (int i = 0; i < oneEightOfSamples*8; ++i) {
	for (int i = 0; i < MAX_KERNEL_SIZE; ++i) {	
	//	get sample position:		
		vec3 samplePos = kernelBasis * uKernelOffsets[i] * radLength;
		samplePos      = samplePos  + receiverPosition;		
		
	//	project sample position:
		samplesPos[i] = camSpaceToScreenSpaceTC(samplePos);
	}	
	*/
}



#ifdef USE_SUBROUTINE_SAMPLESLOCATOR
subroutine (samplesLocator) 
#endif
void getFlatSamples(const in vec2 pixelCoordsTC,
                    const in int oneEightOfSamples,
					const in float maxSamplingRadius,
					const in vec3 receiverPosition,
					const in vec3 receiverNormal) {

	mat3 kernelBasis = getOrthoBasis(pixelCoordsTC, vec3(0,0,1));	

	/*
	const int totsamples = oneEightOfSamples*8;
	for (int i = 0; i < totsamples; ++i) {		
		samplesPosCS[i] = calcCameraSpaceEmisphereSamplePos(i, kernelBasis, receiverPosition, radLength);
		samplesPos[i]   = camSpaceToScreenSpaceTC(samplesPosCS[i]);
	}
	*/
	

	const mat2 matrix45deg =  //{0.707106781, 0.707106781, -0.707106781, 0.707106781};
	  mat2(
			0.707106781, 0.707106781,
			-0.707106781, 0.707106781);

    const int directions = 4;
	const vec2 vec[4]    =  vec2[4]( vec2(1,0), vec2(-1,0), vec2(0,1), vec2(0,-1) );	//shared array

	int samplesPerDir = oneEightOfSamples * 2;
	int samples       = samplesPerDir     * directions;
	
	float increment = maxSamplingRadius / (samplesPerDir-1);
	for(int i = 0; i < directions; i++) {
		float offset = increment;
		vec2 rotdir = vec[i] * matrix45deg;
		
		for (int j = 0; j < samplesPerDir; j = j + 2) {
			vec2 off1 = vec[i] * offset;
			vec2 off2 = rotdir * offset;
			samplesPos[i*samplesPerDir + j]     = camSpaceToScreenSpaceTC( receiverPosition + kernelBasis * vec3(off1.x, 0.0, off1.y) );  //(pixelCoordsTC + (vec[i] * offset)) ;
			samplesPos[i*samplesPerDir + j + 1] = camSpaceToScreenSpaceTC( receiverPosition + kernelBasis * vec3(off2.x, 0.0, off2.y) );  //(pixelCoordsTC + (rotdir * offset)) ;
			offset += increment;			
		}		
	}	
	
}


#ifndef USE_SUBROUTINE_SAMPLESLOCATOR

uniform int calcSamplesPosSelector;

void calcSamplesPos(const in vec2 pixelCoordsTC, const in int oneEightOfSamples, const in float maxSamplingRadius, const in vec3 receiverPos, const in vec3 receiverNormal) {
	switch(calcSamplesPosSelector) {
		case 0: getDssaoSamples(pixelCoordsTC, oneEightOfSamples, maxSamplingRadius, receiverPos, receiverNormal);
				break;		
		case 1: getRadialSamples(pixelCoordsTC, oneEightOfSamples, maxSamplingRadius, receiverPos, receiverNormal);
		        break;		
		case 2: getEmisphereSamples(pixelCoordsTC, oneEightOfSamples, maxSamplingRadius, receiverPos, receiverNormal);
		        break;		
		case 3: getFlatSamples(pixelCoordsTC, oneEightOfSamples, maxSamplingRadius, receiverPos, receiverNormal);
		        break;		
	}
}




#endif


#endif // SAMPLING_INC


/*


// orig : ray origin
// dir  : ray direction
// vertices : triangle vertices
float intersectTriangle(vec3 orig, vec3 dir, vec3 vertices[3]) {
    const float INFINITY = 1e10;

    vec3 u, v, n;  // triangle vectors
    vec3 w0, w;    // ray vectors
    float r, a, b; // params to calc ray-plane intersect

    // get triangle edge vectors and plane normal
    u = vertices[1] - vertices[0];
    v = vertices[2] - vertices[0];
    n = cross(u, v);

    w0 = orig - vertices[0];
    a = -dot(n, w0);
    b =  dot(n, dir);
    if (abs(b) < 1e-5) {
        // ray is parallel to triangle plane, and thus can never intersect.
        return INFINITY;
    }

    // get intersect point of ray with triangle plane
    r = a / b;
    if (r < 0.0)
        return INFINITY; // ray goes away from triangle.

    vec3 I = orig + r * dir; // I is the intersection point
    float uu, uv, vv, wu, wv, D;

    uu = dot(u, u);
    uv = dot(u, v);
    vv = dot(v, v);
    w = I - vertices[0];
    wu = dot(w, u);
    wv = dot(w, v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    float s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)
        return INFINITY;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)
        return INFINITY;

    return (r > 1e-5) ? r : INFINITY;
}

int allInPosOctant_getIntersectingTriangleId(in vec3 occlusorToPointCS,
                              in mat3 kernelBasis, 
							  in vec3 emisphereCenter,
							  in vec3 occlusorPos,
							  out vec3 ptheta) {

	const int n = TRIANGLE_DIVS;

	// invk transforms from camera space to emishphere space
	//mat3 invk = inverse(kernelBasis);
	mat3 invk = transpose(kernelBasis); 

	vec3 dir = occlusorPos-emisphereCenter;
	dir = invk*dir;

	vec3 dirPosOct = vec3( dir.x * sign(dir.x) , dir.y * sign(dir.y), dir.z * sign(dir.z) );

	vec3 triverts[3];
	triverts[1] = vec3(1, 0, 0);
	triverts[0] = vec3(0, 1, 0);
	triverts[2] = vec3(0, 0, 1);

	const float INFINITY = 1e10;

	int triangleId = -1;

	float intersectionDist = intersectTriangle(vec3(0), dirPosOct, triverts);
	ptheta = vec3(0);
	if (intersectionDist != INFINITY) {	
	
		//vec3
		ptheta =  dirPosOct * intersectionDist;
		float x = n * ptheta.x;
		float z = n * ptheta.z;
		
		int xi = int(floor(x));
		int zi = int(floor(z));

		int etheta = zi * (2*n - zi) + 2 * xi;
	
		float diag = (x-xi)+(z-zi);
		if (diag > 1.0) ++etheta;

		triangleId = etheta;
	} 

	ptheta = vec3( ptheta.x * sign(dir.x) , ptheta.y * sign(dir.y), ptheta.z * sign(dir.z) );

	ptheta = emisphereCenter + kernelBasis * ptheta * radLength;
	return triangleId;
}

//nota : errore perché ribalto cambiando i segni invece di ruotare il vettore nell'altro ottante
int old_getIntersectingTriangleId(in vec3 occlusorToPointCS,
                              in mat3 kernelBasis, 
							  in vec3 emisphereCenter,
							  in vec3 occlusorPos,							  
							  out vec3 ptheta) {
							  
	const int n = TRIANGLE_DIVS;

	// invk transforms from camera space to emishphere space
	//mat3 invk = inverse(kernelBasis);
	mat3 invk = transpose(kernelBasis); 

	vec3 dir = occlusorPos-emisphereCenter;
	dir = invk*dir;

	vec3 triverts[3];
	triverts[0] = vec3(sign(dir.x), 0, 0);
	triverts[1] = vec3(0, 1, 0)          ;
	triverts[2] = vec3(0, 0, sign(dir.z));

	const float INFINITY = 1e10;

	int triangleId = -1;

	float intersectionDist = intersectTriangle(vec3(0), dir, triverts);
	ptheta = vec3(0);
	if (intersectionDist != INFINITY) {	
	
		//vec3
		ptheta =  dir * intersectionDist;
		float x = n * ptheta.x * sign(dir.x);
		float z = n * ptheta.z * sign(dir.z);

		//vec3 triptheta = invk*ptheta;
		//float x = n * triptheta.x;
		//float z = n * triptheta.z;
				
		int xi = int(floor(x));
		int zi = int(floor(z));

		int etheta = zi * (2*n - zi) + 2 * xi;
	
		float diag = (x-xi)+(z-zi);
		if (diag > 1.0) ++etheta;

		triangleId = etheta;
	} 
	ptheta = emisphereCenter + kernelBasis * ptheta * radLength;
	return triangleId;
}

*/


