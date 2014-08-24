#ifndef HEMISPHERESAMPLING_H
#define HEMISPHERESAMPLING_H

#include "../util/inc_math.h"

#include <vector>

class HemisphereSampling {
public:
	enum KernelMode_t {
			HEMISPHERE_RANDOM,		
			HEMISPHERE_RANDOM_DISTANCEFALLOFF,
			HEMISPHERE_TRIANGLEHIERARCHY,
			HEMISPHERE_TRIANGLEHIERARCHY_INTERLEAVED
			/*,
			EMISPHERE_PYRAMID,
			EMISPHERE_PYRAMID_DISTANCEFALLOFF,
			EMISPHERE_PYRAMID_DSSO,
			EMISPHERE_PYRAMID_DSSO_DISTANCEFALLOFF
			*/
	};

	static void getTrianglesHierarchy(
		//in
		int n, bool allPointsOnEmishphereSurface, bool octantSampleInterleaving,
		//out
		std::vector<glm::vec3>& orderedSamplingVectors,
		std::vector<int>&   skipTable, 
		std::vector<float>& solidAngleTable, 
		std::vector<float>& sampleWeightTable
		);
	
	static void getRandom(
		//in
		int kernelSize, bool allPointsOnEmishphereSurface, bool distanceFalloff,
		//out
		std::vector<glm::vec3>& orderedSamplingVectors
		);
};


#endif