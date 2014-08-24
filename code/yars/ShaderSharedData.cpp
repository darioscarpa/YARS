#include "ShaderSharedData.h"
#include "Renderable.h"

void ShaderSharedData::init() {
	printf("ShaderSharedData::init()\n");
	////////////////////////////////////////////////////////////////////////////
	glGenBuffers(1, &matricesUniBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUniBuffer);
		glBufferData(GL_UNIFORM_BUFFER, MatricesUniBufferSize, NULL, GL_DYNAMIC_DRAW);
		glBindBufferRange(GL_UNIFORM_BUFFER,  Renderable::ubo_matrices, matricesUniBuffer, 0, MatricesUniBufferSize);
	glBindBuffer(GL_UNIFORM_BUFFER,0);
 	////////////////////////////////////////////////////////////////////////////
	glGenBuffers(1, &lightsUniBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, lightsUniBuffer);
		glBufferData(GL_UNIFORM_BUFFER, LightsUniBufferSize, NULL, GL_DYNAMIC_DRAW);
		glBindBufferRange(GL_UNIFORM_BUFFER, Renderable::ubo_lights, lightsUniBuffer, 0, LightsUniBufferSize);
	glBindBuffer(GL_UNIFORM_BUFFER,0);
	////////////////////////////////////////////////////////////////////////////
	glGenBuffers(1, &scrSizeUniBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, scrSizeUniBuffer);
		glBufferData(GL_UNIFORM_BUFFER, ScrSizeUniBufferSize, NULL, GL_STATIC_DRAW); //GL_DYNAMIC_DRAW); // GL_STATIC_DRAW);
		glBindBufferRange(GL_UNIFORM_BUFFER, Renderable::ubo_deferred, scrSizeUniBuffer, 0, ScrSizeUniBufferSize);
	glBindBuffer(GL_UNIFORM_BUFFER,0);
	
}

void ShaderSharedData::destroy() {
	printf("ShaderSharedData::destroy()\n");
	glDeleteBuffers(1, &matricesUniBuffer);
	glDeleteBuffers(1, &lightsUniBuffer);
	glDeleteBuffers(1, &scrSizeUniBuffer);
}

void ShaderSharedData::setMVPMatrix(const glm::mat4 &mvpMatrix) {
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUniBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, MVPMatrixOffset, MatrixSize, &mvpMatrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ShaderSharedData::setModelViewMatrix(const glm::mat4 &mvMatrix) {
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(mvMatrix));
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUniBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, ModelViewMatrixOffset, MatrixSize, &mvMatrix[0][0]);
		glBufferSubData(GL_UNIFORM_BUFFER, NormalMatrixOffset, MatrixSize, &normalMatrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ShaderSharedData::setModelMatrix(const glm::mat4 &modelMatrix) {
	//Model = modelMatrix;
	//glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUniBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, ModelMatrixOffset,  MatrixSize, &modelMatrix[0][0]);
		//glBufferSubData(GL_UNIFORM_BUFFER, NormalMatrixOffset, MatrixSize, &normalMatrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ShaderSharedData::setProjectionMatrix(const glm::mat4 &projMatrix) {
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUniBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, ProjMatrixOffset, MatrixSize, &projMatrix[0][0]);		
	glBindBuffer(GL_UNIFORM_BUFFER,0);
}

void ShaderSharedData::setInvProjectionMatrix(const glm::mat4 &invProjMatrix) {
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUniBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, InvProjMatrixOffset, MatrixSize, &invProjMatrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER,0);
}


void ShaderSharedData::setViewMatrix(const glm::mat4 &viewMatrix) {
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUniBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, ViewMatrixOffset, MatrixSize, &viewMatrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER,0);
}


void ShaderSharedData::setLights(const float* numLights, const PointLight* pointLights, const SpotLight* spotLights, const DirLight* dirLights ) {
	glBindBuffer(GL_UNIFORM_BUFFER, lightsUniBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0,                numLightsUniBufferSize,   numLights);			
		glBufferSubData(GL_UNIFORM_BUFFER, PointLightOffset, PointLightsUniBufferSize, pointLights);
		glBufferSubData(GL_UNIFORM_BUFFER, SpotLightOffset,  SpotLightsUniBufferSize,  spotLights);
		glBufferSubData(GL_UNIFORM_BUFFER, DirLightOffset,   DirLightsUniBufferSize,   dirLights);
	glBindBuffer(GL_UNIFORM_BUFFER,0);	
}

void ShaderSharedData::setScreenSize(int width, int height) {
	float scrSizeBuf[4];
	scrSizeBuf[0] = static_cast<float>(width);
	scrSizeBuf[1] = static_cast<float>(height);
	scrSizeBuf[2] = 1.0f/width;
	scrSizeBuf[3] = 1.0f/height;
	printf("\n%d\n", scrSizeUniBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, scrSizeUniBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, ScrSizeUniBufferSize, scrSizeBuf);
	glBindBuffer(GL_UNIFORM_BUFFER,0);
}
		