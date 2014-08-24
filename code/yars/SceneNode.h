#ifndef SCENENODE_H
#define SCENENODE_H

#include "Renderable.h"

#include "util/inc_math.h"


// STL
#include <vector>

// fwd dec
// class Renderer;

class SceneNode {

public:
	SceneNode(glm::vec3 _position, glm::quat _orientation, std::string label = "");
	SceneNode(std::string label = "");

	virtual ~SceneNode();

	std::string const getLabel() const { return label; }
	void setLabel(const std::string & s) { label = s; }

	////////////////////////////////////////////////////////////////////////////////
	virtual const glm::vec3 getPosition()    const { return position   ; };
	virtual const glm::quat getOrientation() const { return orientation; };
	virtual const glm::vec3 getScaling()     const { return scaling    ; };

	virtual void setPosition(glm::vec3 pos) {
			position = pos;
			recursive_setDirtyModelMatrix();
	}	
	virtual void setOrientation(glm::quat ori) {
			orientation = ori;
			recursive_setDirtyModelMatrix();
	}
	void setScaling(glm::vec3 s) {
			scaling = s;
			recursive_setDirtyModelMatrix();
	}
	////////////////////////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////////////////////////
	
	/*virtual void lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up ) {		
		glm::mat4 viewMatrix = glm::lookAt(	eye, center, up  );
		m_modelMatrix = glm::inverse(viewMatrix);
				
		setPosition(eye);
		glm::mat3 ori = glm::mat3(m_modelMatrix);
		orientation = glm::quat(ori);
	}*/
	virtual void lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up ) {		
		glm::mat4 viewMatrix  = glm::lookAt( eye, center, up );
		
		glm::mat4 modelMatrix = glm::inverse(viewMatrix);		
		glm::mat3 ori = glm::mat3(modelMatrix);

		setPosition(eye);
		setOrientation(glm::quat(ori));
	}
		
	virtual void move(float dx, float dy, float dz) {
		glm::vec3 disp(dx, dy, dz);
		glm::mat3 m = glm::toMat3(orientation);
		glm::vec3 r = m * disp;

		SceneNode::setPosition(position + r);
		//position = position + r;	

		//recursive_setDirtyModelMatrix();
	}
		
	void rotateWS(float headingDegrees, float pitchDegrees, float rollDegrees) {		
		glm::quat rot = degreesToQuat(headingDegrees, pitchDegrees, rollDegrees);
		
		SceneNode::setOrientation( rot * orientation );
		//orientation   = rot * orientation;

		//recursive_setDirtyModelMatrix();
	}
	
	virtual void rotate(float headingDegrees, float pitchDegrees, float rollDegrees) {
		glm::quat rot = degreesToQuat(headingDegrees, pitchDegrees, rollDegrees);
		
		SceneNode::setOrientation( orientation * rot );
		//orientation = orientation * rot;

		//recursive_setDirtyModelMatrix();
	}
	
	virtual const glm::mat4& getModelMatrix() {
		if (dirtyModelMatrix) {
			glm::mat4 t = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 r = glm::toMat4(orientation);
			glm::mat4 s = glm::scale(glm::mat4(1.0f), scaling);
		
			//m_modelMatrix = t * r * s;		
			glm::mat4 parentM = getParent() == nullptr ? glm::mat4(1.0) : getParent()->getModelMatrix();
			m_modelMatrix = parentM * t * r * s;
		}
		return m_modelMatrix;		
	}

	///////////////////////////////////////////
	void moveForward(float offset)   { move(0,0,-offset); }
	void moveBackwards(float offset) { move(0,0, offset); }
	
	void moveUp(float offset)        { move(0, offset,0); }
	void moveDown(float offset)      { move(0,-offset,0); }	
	
	void moveLeft(float offset)      { move(-offset,0,0); }
	void moveRight(float offset)     { move( offset,0,0); }

	void moveX(float offset)         { move(offset,0,0); }
	void moveY(float offset)         { move(0,offset,0); }
	void moveZ(float offset)         { move(0,0,offset); }
	
	void yaw(float offset)		     { rotate(offset, 0, 0); }
	void pitch(float offset)	     { rotate(0, offset, 0); }
	void roll(float offset)		     { rotate(0, 0, offset); }

	void rotateLeft(float offset)    { rotate(offset, 0,0); }
	void rotateRight(float offset)   { rotate(-offset,0,0); }
	
	void rotateHoriz(float offset)   { rotate(offset,0,0); }
	void rotateVert(float offset)    { rotate(0,offset,0); }
	///////////////////////////////////////////
	
	void setParent(SceneNode *parent) { parentNode = parent; }
	SceneNode *getParent()            { return parentNode; }
	
	void addChild(SceneNode *child) {
		printf("addChild %s to node %s (#children: %d)\n",
			child->getLabel().c_str(),
			this->getLabel().c_str(),
			childNodes.size());
		childNodes.push_back(child);
		child->setParent(this);
	}
	void remChild(SceneNode *child) {
		 for(size_t i = 0; i < childNodes.size(); ++i) {
            if(childNodes.at(i) == child) {
                childNodes.erase(childNodes.begin() + i);
                break; 
            }
        }
	}
	const std::vector<SceneNode*>& getChildNodes() { return childNodes; }
	///////////////////////////////////////////
protected:
	glm::mat4 m_modelMatrix;
	
	glm::vec3  position;	
	glm::vec3  scaling;
	glm::quat  orientation;
	
private: 
	bool dirtyModelMatrix;	

	std::string label;
			
	SceneNode *parentNode;
	std::vector<SceneNode*> childNodes;	

	// ----------------------------------------

	void init();	
	void recursive_setDirtyModelMatrix() {
		dirtyModelMatrix = true;
		for ( auto c : childNodes ) {
			c->recursive_setDirtyModelMatrix();
		}
	};

};


#endif
