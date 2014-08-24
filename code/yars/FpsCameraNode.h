#ifndef FPSCAMERANODE_H
#define FPSCAMERANODE_H

#include "CameraNode.h"

class FpsCameraNode : public CameraNode {
public :
	FpsCameraNode(std::string _label) :
		CameraNode(_label) {
			baseNode.setLabel(std::string(_label+"_base"));
		//baseNode = new SceneNode(std::string(_label+"_base"));//_label+std::string("base"));
	}
	~FpsCameraNode() {
		//delete baseNode;
	}
	void move(float dx, float dy, float dz) {		
		glm::vec3 disp(dx, dy, dz);
		glm::mat3 m  = glm::toMat3(orientation);
		glm::mat3 mp = glm::toMat3(baseNode.getOrientation());				
		glm::vec3 r =  mp * m *  disp;				
		baseNode.setPosition(baseNode.getPosition() + r);		
	}
	void rotate(float headingDegrees, float pitchDegrees, float rollDegrees) {		
//		baseNode->rotate(headingDegrees, 0, rollDegrees);
		baseNode.rotate(headingDegrees, 0, 0);
		CameraNode::rotate(0, pitchDegrees, 0);
		baseNode.rotate(0, 0, rollDegrees);
	}

	const glm::vec3 getPosition()    const { return baseNode.getPosition(); };
	const glm::quat getOrientation() const { return baseNode.getOrientation() * orientation; };


	void setPosition(glm::vec3 pos) {
		baseNode.setPosition(pos);
	}	
	void setOrientation(glm::quat ori) {

		glm::vec3 oriangles = glm::eulerAngles(ori);
		//printf("oriangles %3.2f %3.2f %3.2f\n", oriangles.x, oriangles.y, oriangles.z);
		glm::quat rx = glm::angleAxis(glm::radians(oriangles.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat ry = glm::angleAxis(glm::radians(oriangles.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat rz = glm::angleAxis(glm::radians(oriangles.z), glm::vec3(0.0f, 0.0f, 1.0f));

		//baseNode->setOrientation(ry*rz);
		baseNode.setOrientation(glm::quat());
		baseNode.rotate(oriangles.y, 0, 0);

		CameraNode::setOrientation(rx);
		baseNode.rotate(0, 0, oriangles.z);

		/*glm::mat3 m  = glm::toMat3(ori);
		glm::mat3 basem(m[0], glm::vec3(0), m[2]);
		glm::mat3 topm(glm::vec3(0.0), m[1], glm::vec3(0.0));
		glm::quat baseori = glm::toQuat(basem);
		glm::quat topori  = glm::toQuat(topm);
		baseNode->setOrientation(baseori);*/
		//CameraNode::setOrientation(topori);
	}
		
	//TODO
	//virtual void lookAt(glm::vec3 eye, glm::vec3 at, glm::vec3 up ) {
	//	baseNode->lookAt(eye, at, up);		
	//}

	virtual const glm::mat4& getModelMatrix() {
		const glm::mat4& parMM = baseNode.getModelMatrix();
		glm::mat4 r = glm::toMat4(orientation);
		m_modelMatrix = parMM *  r ;		

		//return m_modelMatrix;		

		glm::mat4 parentM = getParent() == nullptr ? glm::mat4(1.0) : getParent()->getModelMatrix();
		return parentM * m_modelMatrix;		
	}		
	
private:
	SceneNode baseNode;	
			
};

#endif
