#ifndef ANIMATEDASSETNODE_H
#define ANIMATEDASSETNODE_H

#include "AssetNode.h"
#include "AnimatedAsset.h"

class AnimatedAssetNode : public AssetNode {
public:

	AnimatedAssetNode(std::string _label) 
		: AssetNode(_label), 
		 m_currFrameElapsedTime(0),
	     m_frameTime ( 1.0 / 30.0f ) {
	};
	
	AnimatedAssetNode(std::string _label, AnimatedAsset *a) :
		AssetNode(_label, a),
		m_aa(a),
		m_currFrameElapsedTime(0),
	    m_frameTime ( 1.0f / 30.0f ) {
	};

	virtual ~AnimatedAssetNode() {};

	void update(const float dt) {
		m_currFrameElapsedTime += dt;
		while (m_currFrameElapsedTime >= m_frameTime) {
			m_aa->nextFrame();
			m_currFrameElapsedTime -= m_frameTime;
		}
	}

	void setAnimationFrameRate(const float desiredFrameRate) {
		m_frameTime = 1.0f / desiredFrameRate;
	}

	float getAnimationFrameRate() const {
		return 1.0f / m_frameTime;
	}
	
private:
	AnimatedAsset *m_aa;
	float          m_currFrameElapsedTime;
	float          m_frameTime;	
};

#endif
