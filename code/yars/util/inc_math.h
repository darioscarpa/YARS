#ifndef INC_MATH
#define INC_MATH

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>


static glm::quat degreesToQuat(float headingDegrees, float pitchDegrees, float rollDegrees) {
		glm::quat rx = glm::angleAxis( glm::radians(pitchDegrees),   glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat ry = glm::angleAxis( glm::radians(headingDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat rz = glm::angleAxis( glm::radians(rollDegrees),    glm::vec3(0.0f, 0.0f, 1.0f));

		glm::quat rot = rz * ry * rx;
		return rot;	
}



#endif