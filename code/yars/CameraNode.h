#ifndef CAMERANODE_H
#define CAMERANODE_H

#include "SceneNode.h"

const float DEFAULT_ASPECTRATIO = 4.0f/3.0f;
const float DEFAULT_FOV         = 60.0f;
const float DEFAULT_NEAR_CLIP   = 0.1f;
const float DEFAULT_FAR_CLIP    = 100.0f;

class CameraNode : public SceneNode {
public :
	CameraNode(std::string _label) :
		SceneNode(_label) {
		m_viewMatrix = glm::mat4(); // default constructor: identity matrix

		setNearClipDist(DEFAULT_NEAR_CLIP);
		setFarClipDist(DEFAULT_FAR_CLIP);

		setAspectRatio(DEFAULT_ASPECTRATIO);
		setAutoUpdateAspectRatio(true);
		
		setFOV(DEFAULT_FOV);
	}
	
	virtual glm::mat4 *getViewMatrix() {	
		updateViewMatrix();
		return &m_viewMatrix;
	}

	inline glm::mat4 *getProjectionMatrix() {	
		return &m_projectionMatrix;
	}
	inline glm::mat4 *getInvProjectionMatrix() {	
		return &m_invProjectionMatrix;
	}

	void setAutoUpdateAspectRatio(bool enabled) { autoUpdateAspectRatio = enabled; };
	void setAspectRatio(float ratio) { aspectRatio = ratio; updateProjectionMatrix();};
	void setFOV(float camfov)        { fov = camfov;        updateProjectionMatrix();};
	void setNearClipDist(float dist) { nearClip = dist;     updateProjectionMatrix();};
	void setFarClipDist(float dist)  { farClip = dist;      updateProjectionMatrix();};	

	inline bool  getAutoUpdateAspectRatio() const { return autoUpdateAspectRatio; };
	inline float getAspectRatio()  const { return aspectRatio;};
	inline float getFOV()          const { return fov;        };
	inline float getNearClipDist() const { return nearClip;   };
	inline float getFarClipDist()  const { return farClip ;   };

	/*virtual float *getViewMatrix() {	
		getModelMatrix();
		m_viewMatrix = glm::inverse(m_modelMatrix);						
		return glm::value_ptr(m_viewMatrix);
	}*/	

	/*void lookAt(glm::vec3 eye, glm::vec3 at, glm::vec3 up ) {
		m_viewMatrix = glm::lookAt(
								glm::vec3(4,3,-3), // Camera is at (4,3,-3), in World Space
								glm::vec3(0,0,0), // and looks at the origin
								glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
						   );
		m_modelMatrix = glm::inverse(m_viewMatrix);
	}*/

	/*virtual glm::vec3 getLookAtDirection() {
		glm::vec3 disp(0, 0, -1.0);
		glm::mat3 m = glm::toMat3(orientation);
		glm::vec3 r = m * disp;		
		return r;
	}*/

protected:
	glm::mat4  m_viewMatrix;
	glm::mat4  m_projectionMatrix;
	glm::mat4  m_invProjectionMatrix;

	bool  autoUpdateAspectRatio;
	float aspectRatio;
	float fov;
	float nearClip;
	float farClip;

	inline void updateViewMatrix() {
		getModelMatrix();
		m_viewMatrix = glm::inverse(m_modelMatrix);		
	}
	inline void updateProjectionMatrix() {
		m_projectionMatrix    = glm::perspective( glm::radians(fov), aspectRatio, nearClip, farClip);
		m_invProjectionMatrix = glm::inverse(m_projectionMatrix);
	}
};

#endif
