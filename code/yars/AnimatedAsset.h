#ifndef ANIMATEDASSET_H
#define	ANIMATEDASSET_H

#include "Asset.h"

class AnimatedAsset : public Renderable {
	friend class AssetManager;
public:
	~AnimatedAsset();
	
	void init() {};
	void destroy() {};

	virtual void render(Renderer *boundRenderer, bool withMaterials = true);

	/*inline void nextFrame() {
		if (m_currentFrame < m_frames-1) {
			++m_currentFrame;
		} else { 
			m_currentFrame = 0; 
		}
	}*/

	inline void nextFrame() {
		if (m_playingForward) {
			if ( m_currentFrame < m_frames-1 ) {
				++m_currentFrame;
			} else {
				m_playingForward = false;
			}
		} else {
			if ( m_currentFrame > 0 ) {
				--m_currentFrame;
			} else {
				m_playingForward = true;
			}
		}		
	}
	inline int getNumFrames() const { return m_frames; }
	inline const std::string& getFilePrefix() const { return m_filePrefix; };
	inline const std::string& getFileSuffix() const { return m_fileSuffix; };

private:
	AnimatedAsset(const std::string& filenamePrefix, const std::string& filenameSuffix, const int frames);

	void setFrameAsset(const int frame, Asset* a);

	std::string m_label;

	std::vector<Asset*> m_frameAssets;
	int m_currentFrame;
	int m_frames;

	bool m_playingForward;

	std::string m_filePrefix;
	std::string m_fileSuffix;
};

#endif
