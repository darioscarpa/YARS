#include "HemisphereSampling.h"

#include <random>

class Triangle {
  public:
	 Triangle() {  v[0] = v[1] = v[2] = glm::vec3(0.0); centroid = glm::vec3(0.0);}

	 Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) {
		  v[0] = v0;
		  v[1] = v1;
		  v[2] = v2;

		  const float oneThird = 1.0/3.0;
		  centroid = glm::vec3( oneThird*(v0.x + v1.x + v2.x),  oneThird*(v0.y + v1.y + v2.y), oneThird*(v0.z + v1.z + v2.z));	
	  }
      glm::vec3 v[3];
	  glm::vec3 centroid;
};
/*
n: number of side subdivisions
n=2 -> 4 triangles,  2 sampling levels
n=3 -> 9 triangles
n=4 -> 16 triangles, 3 sampling levels
*/
std::vector<Triangle> getTrianglesForOctant(const int n) {
	glm::vec3 v0 = glm::vec3(1, 0, 0);
	glm::vec3 v1 = glm::vec3(0, 1, 0);
	glm::vec3 v2 = glm::vec3(0, 0, 1);
	

	glm::vec3 v0v1 = v1 - v0;
	glm::vec3 v0v2 = v2 - v0;
	glm::vec3 v1v2 = v2 - v1;
	
	glm::vec3 planenormal = glm::cross(v0v1, v0v2);
	printf("%2.3f %2.3f %2.3f \n", planenormal.x, planenormal.y, planenormal.z);
	float incr = 1.0/n;

	/*
	std::vector<float> subdiv;	
	for (int i = 0; i < n; ++i) {
		subdiv.push_back( i*incr );
	}
	
	std::vector<glm::vec2> v0v1_subdiv;
	std::vector<glm::vec2> v0v2_subdiv;
	std::vector<glm::vec2> v1v2_subdiv;
	
	for (int i = 1; i < n; ++i) {
		v0v1_subdiv.push_back( v0 + v0v1 * subdiv[i] );
		v0v2_subdiv.push_back( v0 + v0v2 * subdiv[i] );
		v1v2_subdiv.push_back( v1 + v1v2 * subdiv[i] );
	}

	std::vector<glm::vec2> lines;
	lines.push_back(v0v1);
	lines.push_back(v0v2);
	lines.push_back(v1v2);
	for (int i = 0; i < n-1; ++i) {
		//v0v1 parallels
		lines.push_back( v1v2_subdiv[i] - v0v2_subdiv[i] );

		//v0v2 parallels
		lines.push_back( v1v2_subdiv[i] - v0v1_subdiv[i] );

		//v1v2 parallels
		lines.push_back( v0v2_subdiv[i] - v0v1_subdiv[i] );
	}
	*/
	
	const int nsquare = n*n;
	std::vector<Triangle> triangles(nsquare);
	
	int triangles_per_row = n;
	
	int triangleId = 0;

	for (int i = 0; i < n; ++i) { // rows
		glm::vec3 rowBottomLeftOffset = v0 + v0v1 * (incr * i);
		glm::vec3 rowTopLeftOffset = v0 + v0v1 * (incr * (i+1));

		for (int j = 0; j < triangles_per_row;   ++j) { //triangles per row, pointing up (es: 0, 2, 4, 6)
			 
			glm::vec3 triv0 = rowBottomLeftOffset + v0v2 * (incr * j);
			glm::vec3 triv1 = rowTopLeftOffset + v0v2 * (incr*j);
			glm::vec3 triv2 = rowBottomLeftOffset + v0v2 * (incr * (j + 1));
				
			triangles[triangleId] = Triangle(triv0, triv1, triv2);
			triangleId += 2;
		}
		triangleId = triangleId -(2*triangles_per_row) + 1;
		for (int j = 0; j < triangles_per_row-1; ++j) { //triangles per row, pointing down (es: 1, 3, 5)

			glm::vec3 triv0 = triangles[triangleId-1].v[1];
			glm::vec3 triv1 = triangles[triangleId+1].v[1];
			glm::vec3 triv2 = triangles[triangleId-1].v[2];			
			
			triangles[triangleId] = Triangle(triv0, triv1, triv2);			
			triangleId += 2;
		}
		--triangles_per_row;		
	}
	
	for (int i = 0; i < triangles.size(); ++i) {
			auto t = triangles[i];
			printf("%d v0(%1.2f, %1.2f, %1.2f) v1(%1.2f, %1.2f, %1.2f) v2(%1.2f, %1.2f, %1.2f) c(%1.2f, %1.2f, %1.2f)\n\n", 
			  i,
			  t.v[0].x, t.v[0].y, t.v[0].z,
			  t.v[1].x, t.v[1].y, t.v[1].z,
			  t.v[2].x, t.v[2].y, t.v[2].z,
			  t.centroid.x, t.centroid.y, t.centroid.z);
	}

	return triangles;
}

void HemisphereSampling::getTrianglesHierarchy(
		//in
		int n, bool allPointsOnEmishphereSurface, bool octantSampleInterleaving,
		//out
		std::vector<glm::vec3>& orderedSamplingVectors,
		std::vector<int>&   skipTable,
		std::vector<float>& solidAngleTable,
		std::vector<float>& sampleWeightTable
		) {
			
    const int nsquare = n*n;

	std::vector<Triangle> triangles = getTrianglesForOctant(n);
	
	std::vector<glm::vec3> samplingVectors;
	for (int oct = 0; oct < 4; ++oct) {
		// for each octant, rotate the sampling dir by 90° around the y axis to build the hemisphere
		for (int i = 0; i < triangles.size(); ++i) {
			auto t = triangles[i];
			glm::vec3 samplingDir = glm::normalize(t.centroid) ;
			
			switch(oct) {
				case 0: break;
				case 1: samplingDir *= glm::vec3( 1,  1, -1); break;
				case 2: samplingDir *= glm::vec3(-1,  1, -1); break;
				case 3: samplingDir *= glm::vec3(-1,  1,  1); break;
			}
			samplingVectors.push_back(glm::normalize(samplingDir));
		}	
	}
	/////////////////////////////////////////////////////////
	for (int i = 0; i < samplingVectors.size(); ++i) {
		auto sv = samplingVectors[i];
		printf("%2d (%1.2f, %1.2f, %1.2f) \n", 
			  i, sv.x, sv.y, sv.z );	
	}
	/////////////////////////////////////////////////////////
	const float PI = 3.14159265358979f;
	int ord[16]; int skip[16]; float solidangle[16]; float len[16]; float weight[16];

	switch(nsquare) {	
	/////////////////////////////////////////////////////////
		case 4:
			ord[0] = 1; skip[0] = 3;  solidangle[0] = PI/2;  len[0] = 1.0;         weight[0] = 1.0/4.0;

			ord[1] = 0;  skip[1] = 0;  solidangle[1] = PI/8;  len[1] = 1*(1.0/4.0); weight[1] = 1.0/8.0;
			ord[2] = 2;  skip[2] = 0;  solidangle[2] = PI/8;  len[2] = 2*(1.0/4.0); weight[2] = 1.0/8.0;
			ord[3] = 3;  skip[3] = 0;  solidangle[3] = PI/8;  len[3] = 3*(1.0/4.0); weight[3] = 1.0/8.0;
			
			break;
	/////////////////////////////////////////////////////////			
		//case 16:
		default:
			ord[0]  =  9; skip[ 0] = 15; solidangle[ 0] = PI/2;  len[ 0] = 1.0;         weight[ 0] = 1.0/4.0;

			ord[ 1] =  1; skip[ 1] = 3;  solidangle[ 1] = PI/8;  len[ 1] = 1*(1.0/3.0); weight[ 1] = 1.0/8.0;

			ord[ 2] =  0; skip[ 2] = 0;  solidangle[ 2] = PI/32; len[ 2] = 1*(1.0/3.0); weight[ 2] = 1.0/16.0;
			ord[ 3] =  2; skip[ 3] = 0;  solidangle[ 3] = PI/32; len[ 3] = 2*(1.0/3.0); weight[ 3] = 1.0/16.0;
			ord[ 4] =  7; skip[ 4] = 0;  solidangle[ 4] = PI/32; len[ 4] = 3*(1.0/3.0); weight[ 4] = 1.0/16.0;

			ord[ 5] =  5; skip[ 5] = 3;  solidangle[ 5] = PI/8;  len[ 5] = 2*(1.0/3.0); weight[ 5] = 1.0/8.0;

			ord[ 6] =  4; skip[ 6] = 0;  solidangle[ 6] = PI/32; len[ 6] = 3*(1.0/3.0); weight[ 6] = 1.0/16.0; 
			ord[ 7] =  6; skip[ 7] = 0;  solidangle[ 7] = PI/32; len[ 7] = 1*(1.0/3.0); weight[ 7] = 1.0/16.0;
			ord[ 8] = 11; skip[ 8] = 0;  solidangle[ 8] = PI/32; len[ 8] = 2*(1.0/3.0); weight[ 8] = 1.0/16.0;

			ord[ 9] = 13; skip[ 9] = 3;  solidangle[ 9] = PI/8;  len[ 9] = 3*(1.0/3.0); weight[ 9] = 1.0/8.0;

			ord[10] = 12; skip[10] = 0;  solidangle[10] = PI/32; len[10] = 2*(1.0/3.0); weight[10] = 1.0/16.0;
			ord[11] = 14; skip[11] = 0;  solidangle[11] = PI/32; len[11] = 3*(1.0/3.0); weight[11] = 1.0/16.0;
			ord[12] = 15; skip[12] = 0;  solidangle[12] = PI/32; len[12] = 1*(1.0/3.0); weight[12] = 1.0/16.0;

			ord[13] =  8; skip[13] = 0;  solidangle[13] = PI/32; len[13] = 1*(1.0/3.0); weight[13] = 1.0/16.0;
			ord[14] = 10; skip[14] = 0;  solidangle[14] = PI/32; len[14] = 2*(1.0/3.0); weight[14] = 1.0/16.0;
			ord[15] =  3; skip[15] = 0;  solidangle[15] = PI/32; len[15] = 3*(1.0/3.0); weight[15] = 1.0/16.0;

			break;
	/////////////////////////////////////////////////////////
	}

	if (octantSampleInterleaving) {
		for (int i = 0, j = 0; i < triangles.size() ; ++i, j+=4) {
			for (int oct = 0; oct < 4; ++oct) {			
				orderedSamplingVectors[j + oct] = samplingVectors[ ord[i] + oct*nsquare ];
				if (!allPointsOnEmishphereSurface) orderedSamplingVectors[j + oct] *= len[i];
				skipTable[j + oct]              = skip[i];
				solidAngleTable[j + oct]        = solidangle[i];
				sampleWeightTable[j + oct]      = weight[i];
				//vectorLenTable[j + oct]         = len[i];
			}
		}
	} else { 		
		for (int oct = 0; oct < 4; ++oct) {
			for (int i = 0; i < triangles.size(); ++i) {
				orderedSamplingVectors[i + oct*nsquare] = samplingVectors[ ord[i] + oct*nsquare ]; // * (allPointsOnEmishphereSurface ? 1.0f : len[i]);
				if (!allPointsOnEmishphereSurface) orderedSamplingVectors[i + oct*nsquare] *= len[i];
				skipTable[i + oct*nsquare]         = skip[i];
				solidAngleTable[i + oct*nsquare]   = solidangle[i];
				sampleWeightTable[i + oct*nsquare] = weight[i];
				//vectorLenTable[i + oct*nsquare]    = len[i];
			}
		}
	}
}
	
void HemisphereSampling::getRandom(
		//in
		int kernelSize, bool allPointsOnEmishphereSurface, bool distanceFalloff,
		//out
		std::vector<glm::vec3>& kernel
		) {
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<> dis_y(0, 1);
			std::uniform_real_distribution<> dis_xz(-1, 1);
 
			for (int i = 0; i < kernelSize; ++i) {
				//kernel[i] = glm::vec3( dis_xy(gen), dis_xy(gen), dis_z(gen) );
				kernel.push_back( glm::vec3( dis_xz(gen), dis_y(gen), dis_xz(gen) ));
				kernel[i] = glm::normalize(kernel[i]);

				if (!allPointsOnEmishphereSurface) {
					if (distanceFalloff) {
						float scale = static_cast<float>(i) / static_cast<float>(kernelSize);
						//  return v0+(v1-v0)*t;
						scale = 0.1f + (1.0f - 0.1) * (scale*scale); //sml::lerp(0.1f, 1.0f, scale * scale);

						kernel[i] *= scale;
					} else {
						kernel[i] *= dis_y(gen);
					}		
				}

				printf( "%03.3f %03.3f %03.3f \n", kernel[i].x, kernel[i].y, kernel[i].z);
			}
}