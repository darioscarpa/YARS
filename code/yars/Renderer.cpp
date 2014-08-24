#include "Renderer.h"

#include "SceneNode.h"
#include "CameraNode.h"
#include "LightNode.h"
#include "Texture.h"

#include "util/vsGLInfoLib.h"

// MATH
#include "util/inc_math.h"

Renderer::Renderer() {	
	// make cppcheck happy
	winH = winW = 0;
	View = Projection = InvProjection = nullptr;
	ambientLightLevel = 0.0f;
	//enabledLighting =
	enabledNormalMapping = enabledSpecularMapping = enabledOpacityMapping = enabledTextureMapping = false; //= enabledTextureFiltering
		 

	for (auto l : pointLights) {
		l.constantAttenuation  = 0.0f;
		l.linearAttenuation    = 0.0f;
		l.quadraticAttenuation = 0.0f;	
	}
}

Renderer::~Renderer() {
	std::cout << "destroying renderer: " << label << std::endl;
}

void Renderer::setModelMatrix(const glm::mat4 &modelMatrix) {
	Model = modelMatrix;
	ssd.setModelMatrix(modelMatrix);	
}

void Renderer::setViewMatrix(const glm::mat4 &viewMatrix) {
	ssd.setViewMatrix(viewMatrix);	
}

void Renderer::setProjectionMatrix(const glm::mat4 &projMatrix) {
	ssd.setProjectionMatrix(projMatrix);	
}

void Renderer::setInvProjectionMatrix(const glm::mat4 &invProjMatrix) {
	ssd.setInvProjectionMatrix(invProjMatrix);	
}

void Renderer::setModelViewMatrix(const glm::mat4 &mvMatrix) {
	ssd.setModelViewMatrix(mvMatrix);	
}

void Renderer::setMVPMatrix(const glm::mat4 &mvpMatrix) {
	ssd.setMVPMatrix(mvpMatrix);	
}

void Renderer::reshape(int awinW, int awinH) {
	winH = awinH; winW = awinW;
	glViewport(0, 0, (GLsizei)winW, (GLsizei)winH);
	
	/*Projection    = glm::perspective(FOV, (float)winW/winH, NEAR_CLIP, FAR_CLIP);
	InvProjection = glm::inverse(Projection);
	setProjectionMatrix(Projection, InvProjection);*/
	//mustUpdateProjectionMatrix = true;
}


void Renderer::renderSceneGeometry(const Scene& scene){
	////////////////////////////////////////////////////////////////////////
	// update view matrix
	View  = scene.getActiveCamera()->getViewMatrix();
	setViewMatrix(*View);

	// update proj matrix if needed
	//if (mustUpdateProjectionMatrix) {
	Projection =  scene.getActiveCamera()->getProjectionMatrix();	
	setProjectionMatrix(*Projection);
	InvProjection = scene.getActiveCamera()->getInvProjectionMatrix();
	setInvProjectionMatrix(*InvProjection);
	
	// update lights 
	setLights(*View, scene);

	// calc VP matrix
	glm::mat4 VP = *Projection * *View;

	// init M matrix
	glm::mat4 M  = glm::mat4(1.0);
	
	for ( auto a : scene.getAssetNodes() ) {
		const glm::mat4& currModel = a->getModelMatrix();
		setModelMatrix(currModel);

		glm::mat4 ModelView = *View * currModel;
		setModelViewMatrix(ModelView);

		glm::mat4 MVP = VP * currModel;
		setMVPMatrix(MVP);

		a->getAttached()->render(this);
	}
}

void Renderer::recursive_render(glm::mat4 *vp,
								glm::mat4 currModel,
								SceneNode *n) {
	//AssetNode *an = dynamic_cast<AssetNode*>(n);
	//printf("recursive_render node %08X\n", n);
	if (AssetNode *an = dynamic_cast<AssetNode*>(n) ) {
			glm::mat4 Model = n->getModelMatrix();
			currModel = currModel * Model;
			setModelMatrix(currModel);

			glm::mat4 ModelView = *View * currModel;
			setModelViewMatrix(ModelView);

			glm::mat4 MVP = *vp * currModel;
			setMVPMatrix(MVP);

			an->getAttached()->render(this);
	}
	//printf("children: %d\n", n->getChildNodes()->size());

	for(std::vector<SceneNode*>::const_iterator it = n->getChildNodes().begin();
		it != n->getChildNodes().end();
		++it) {
			SceneNode *child = (*it);
			recursive_render(vp, currModel, child);
	}
}

void Renderer::setLights(const glm::mat4& viewMatrix, const Scene& scene) {

	auto lights = scene.getLightNodes();	
	int  maxL = lights.size();
	//if (maxPL > POINT_LIGHTS_MAX) maxPL = POINT_LIGHTS_MAX;	

	int cont_point = 0;
	int cont_spot  = 0;
	int cont_dir   = 0;

	// update pointLights array and send it to ssd
	for (int i = 0; i < maxL; i++) {
		//printf("setting light %d\n", i);
		LightNode *ln = lights.at(i); 

		glm::vec3 lightPos        = ln->getPosition();
		//glm::vec3 lightDir      = glm::vec3(1.0, 0.0, 0.0);//glm::vec3(glm::toMat3(pln->getOrientation())[2]); 
		glm::mat3 lightOriM       = glm::toMat3(ln->getOrientation());
		glm::vec3 lightDir        = glm::vec3(lightOriM[2]); 
		glm::vec4 posCameraSpace  = viewMatrix * glm::vec4( lightPos, 1.0 );
		glm::vec4 dirCameraSpace  = viewMatrix * glm::vec4( lightDir, 0.0 );
		//glm::vec4 pointLightPosCameraSpace = pln->getModelMatrix() * viewMatrix * glm::vec4( pointLightPos, 1.0 );
	
		switch(ln->getType()) {
			case LightNode::POINT:
				assert(cont_point < POINT_LIGHTS_MAX);
				pointLights[cont_point].constantAttenuation  = ln->getConstantAttenuation();
				pointLights[cont_point].linearAttenuation    = ln->getLinearAttenuation();
				pointLights[cont_point].quadraticAttenuation = ln->getQuadraticAttenuation();		
				memcpy(&(pointLights[cont_point].color), glm::value_ptr(ln->getColor()), 3*sizeof(float));
				memcpy(&(pointLights[cont_point].posCS), glm::value_ptr(posCameraSpace), 3*sizeof(float));
				++cont_point;
				break;
			case LightNode::SPOT:
				assert(cont_spot < SPOT_LIGHTS_MAX);
				spotLights[cont_spot].constantAttenuation  = ln->getConstantAttenuation();
				spotLights[cont_spot].linearAttenuation    = ln->getLinearAttenuation();
				spotLights[cont_spot].quadraticAttenuation = ln->getQuadraticAttenuation();
				spotLights[cont_spot].cosCutoffOuter   = glm::cos(glm::radians(ln->getSpotOuterCutoff()));
				spotLights[cont_spot].cosCutoffInner   = glm::cos(glm::radians(ln->getSpotInnerCutoff()));
				spotLights[cont_spot].spotExponent  = ln->getSpotExponent();

				memcpy(&(spotLights[cont_spot].color), glm::value_ptr(ln->getColor()), 3*sizeof(float));
				memcpy(&(spotLights[cont_spot].posCS), glm::value_ptr(posCameraSpace), 3*sizeof(float));
				memcpy(&(spotLights[cont_spot].dirCS), glm::value_ptr(dirCameraSpace), 3*sizeof(float));
				++cont_spot;
				break;
			case LightNode::DIRECTIONAL:
				assert(cont_dir < DIR_LIGHTS_MAX);
				memcpy(&(dirLights[cont_dir].color), glm::value_ptr(ln->getColor()), 3*sizeof(float));
				memcpy(&(dirLights[cont_dir].dirCS), glm::value_ptr(dirCameraSpace), 3*sizeof(float));
				++cont_dir;
				break;
		}
				
		/*PointLight &pl = pointLights[i];
		printf("light infos: pos:%.1f %.1f %.1f - color:%.1f %.1f %.1f - a:%.1f %.1f %.1f - int:%.1f\n",
			pl.posCS[0], pl.posCS[1], pl.posCS[2], 
			pl.color[0], pl.color[1], pl.color[2],
			pl.constantAttenuation, pl.linearAttenuation, pl.quadraticAttenuation,			
			pl.intensity);	*/
	}
	numLights[0] = maxL;
	numLights[1] = cont_point;
	numLights[2] = cont_spot;
	numLights[3] = cont_dir;

	ssd.setLights(numLights, pointLights, spotLights, dirLights);		
}

int Renderer::multisamplingSamples() const {
	GLint s;
	glGetIntegerv(GL_SAMPLES, &s);
	return s;
}
bool Renderer::isMultisamplingEnabled() const {
	GLboolean enabled = glIsEnabled(GL_MULTISAMPLE);
	return enabled;
}

void Renderer::setMultisampling(const bool enabled) {
	if (enabled) {
		glEnable(GL_MULTISAMPLE);
	} else {
		glDisable(GL_MULTISAMPLE);
	}
}

/*
void Renderer::setTextureFiltering(bool enabled) {
	enabledTextureFiltering = enabled;
	Texture::setFilteringEnabled(GL_TEXTURE0 + Renderable::tu_diffuse,     enabled);
	Texture::setFilteringEnabled(GL_TEXTURE0 + Renderable::tu_normalMap,   enabled);
	Texture::setFilteringEnabled(GL_TEXTURE0 + Renderable::tu_specularMap, enabled);
	Texture::setFilteringEnabled(GL_TEXTURE0 + Renderable::tu_opacityMap,  enabled);
}
*/
