#ifndef SHADERSHAREDDATA_H
#define SHADERSHAREDDATA_H

// OPENGL
#include "util/inc_opengl.h"

// MATH
#include "util/inc_math.h"

const int MatrixSize             = sizeof(float) * 16;
const int MatricesUniBufferSize  = MatrixSize * 7;

const int ProjMatrixOffset       = 0;
const int ViewMatrixOffset       = MatrixSize * 1;
const int ModelMatrixOffset      = MatrixSize * 2;
const int ModelViewMatrixOffset  = MatrixSize * 3;
const int MVPMatrixOffset        = MatrixSize * 4;
const int InvProjMatrixOffset	 = MatrixSize * 5;
const int NormalMatrixOffset	 = MatrixSize * 6;

struct PointLight {	
	float color[4];
	float posCS[4];

	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
	float padding;
};

struct SpotLight {	
	float color[4];
	float posCS[3];
	float cosCutoffOuter;

	float dirCS[3];
	float cosCutoffInner;

	float spotExponent;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;	
};

struct DirLight {
   float color[4];
   float dirCS[4];
};

//must match shader defs
const int POINT_LIGHTS_MAX = 5;
const int SPOT_LIGHTS_MAX  = 5;
const int DIR_LIGHTS_MAX   = 2;

const int numLightsUniBufferSize   = sizeof(float)             * 4;
const int PointLightsUniBufferSize = sizeof(struct PointLight) * POINT_LIGHTS_MAX;
const int SpotLightsUniBufferSize  = sizeof(struct SpotLight)  * SPOT_LIGHTS_MAX;
const int DirLightsUniBufferSize   = sizeof(struct DirLight)   * DIR_LIGHTS_MAX;

const int PointLightOffset    = numLightsUniBufferSize;
const int SpotLightOffset     = PointLightOffset + PointLightsUniBufferSize;
const int DirLightOffset      = SpotLightOffset  + SpotLightsUniBufferSize;

const int LightsUniBufferSize = DirLightOffset   + DirLightsUniBufferSize;

const int ScrSizeUniBufferSize = sizeof(float) * 4;


class ShaderSharedData {
public:
		void init();
		void destroy();
		void setMVPMatrix(const glm::mat4& mvpMatrix);
		void setModelMatrix(const glm::mat4& modelMatrix);
		void setProjectionMatrix(const glm::mat4& projMatrix);
		void setInvProjectionMatrix(const glm::mat4 &invProjMatrix);
		void setViewMatrix(const glm::mat4& viewMatrix);
		void setModelViewMatrix(const glm::mat4& mvMatrix);	

		void setScreenSize(int width, int height);
		
		//void setLights(const PointLight* pointLights);
		void setLights(const float* numLights, const PointLight* pointLights, const SpotLight* spotLights, const DirLight* dirLights );

private:
	GLuint matricesUniBuffer;
	GLuint lightsUniBuffer;
	GLuint scrSizeUniBuffer;
};

#endif