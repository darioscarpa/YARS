#ifndef ASSETMANAGER_H
#define	ASSETMANAGER_H

// ASSIMP
#include <assimp/Importer.hpp>	

// MATH
#include "util/inc_math.h"

// STL
#include <map>

class Asset;
class AnimatedAsset;

class AssetManager {
public:
	AssetManager() { 
		importer.SetPropertyBool("GLOB_MEASURE_TIME", true);
	};
	Asset *getAssetByName(const std::string& filename);
	void unloadAsset(const std::string& filename);
		
	Asset         *loadAsset(const std::string& filename);		
	AnimatedAsset *loadAnimatedAsset(const std::string& filenamePrefix, const std::string& filenameSuffix, const int frames);

private:
	Assimp::Importer importer;   	
	std::map<std::string, Asset*> m_assetsMap; // "sponza.obj" -> ptrToLoadedAsset
	
	static const std::string assetsFolder;
};


#endif