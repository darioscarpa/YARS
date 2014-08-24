#include "AnimatedAsset.h"

 AnimatedAsset::AnimatedAsset(const std::string& filenamePrefix, const std::string& filenameSuffix, const int frames) :	
	m_filePrefix(filenamePrefix),
	m_fileSuffix(filenameSuffix),
	m_frames(frames), 
	m_label(filenamePrefix + std::to_string(frames) + filenameSuffix), m_currentFrame(0),
    m_playingForward(true) {
		m_frameAssets.reserve(frames);		
}

void AnimatedAsset::render(Renderer *boundRenderer, bool withMaterials) {
	m_frameAssets[m_currentFrame]->render(boundRenderer, withMaterials);
}

void AnimatedAsset::setFrameAsset(const int frame, Asset* a) {
	m_frameAssets.insert(m_frameAssets.begin()+frame,a);
}