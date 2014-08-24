#include "AssetManager.h"
#include "Asset.h"
#include "AnimatedAsset.h"
#include "util/io_utils.h"

// STL
#include <iostream>
#include <sstream>
#include <iomanip>

const std::string AssetManager::assetsFolder = "_models/";

Asset *AssetManager::loadAsset(const std::string& filename) {
	Asset *ret; 

	std::map<std::string, Asset*>::iterator it;
	it = m_assetsMap.find(filename);

	if (it == m_assetsMap.end()) {
		try {
			ret = new Asset(importer, assetsFolder + filename);
			ret->setLabel(filename);
			m_assetsMap.insert(std::make_pair(filename, ret));
		} catch (std::exception& e) {			
			std::cerr << e.what();
			IoUtils::blockUntilNewline();
		}
	} else {
		ret = (*it).second;
	}	
	
	/*
	std::cout << "---" << std::endl;
	for (auto& kv : m_assetsMap) {
	   std::cout << kv.first << " has value " << kv.second << std::endl;
	}	
	std::cout << "---" << std::endl;
	*/

	return ret;
}

AnimatedAsset *AssetManager::loadAnimatedAsset(const std::string& filenamePrefix, const std::string& filenameSuffix, const int frames) {
	//number of digits
	int digits = (int) log10 ((double) frames) + 1 ;
	//int digits = std::to_string(frames).length();

	AnimatedAsset *aa = new AnimatedAsset(filenamePrefix, filenameSuffix, frames);
	for (int i = 0; i < frames; ++i) {
		std::ostringstream ss;
		ss << filenamePrefix << std::setw(digits) << std::setfill('0') << i << filenameSuffix;
		Asset *a = loadAsset(ss.str());
		aa->setFrameAsset(i, a);
	}
	return aa;
}
