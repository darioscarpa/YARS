#ifndef ASSETNODE_H
#define ASSETNODE_H

#include "SceneNode.h"

class AssetNode : public SceneNode {
public:

	AssetNode(std::string _label) : SceneNode(_label), m_attached(nullptr) { }
	
	AssetNode(std::string _label, Renderable *a) : SceneNode(_label)  {				
		attach(a);
	}

	virtual ~AssetNode() {};

	void attach(Renderable *r) { m_attached = r; };
	Renderable *getAttached() const { return m_attached; };
private:
	Renderable *m_attached;
};

#endif
