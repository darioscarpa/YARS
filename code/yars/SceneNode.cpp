#include "SceneNode.h"

SceneNode::SceneNode(glm::vec3 _position, glm::quat _orientation, std::string _label) 
	: position(_position), orientation(_orientation) , label(_label) {	
	init();
}

SceneNode::SceneNode(std::string _label) {
	position = glm::vec3(0,0,0);
	orientation = glm::quat();	
	label = _label;
	init();
}

SceneNode::~SceneNode() {
	printf("sceneNode %s destructor called\n", label.c_str());
}

void SceneNode::init() {		
	scaling = glm::vec3(1,1,1);	
	dirtyModelMatrix = true;

	parentNode = nullptr;

	//attached = NULL;	
	//childNodes = new std::vector<SceneNode*>();

	printf("SceneNode %s %08x init() - children: %d\n", label.c_str(), this, childNodes.size());
}
