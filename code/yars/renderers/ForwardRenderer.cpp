#include "ForwardRenderer.h"

#include "shaders/ForwardShader.h"
#include "shaders/ForwardShaderFlat.h"
//#include "CameraNode.h"

#include "../util/vsGLInfoLib.h"

// MATH
#include "../util/inc_math.h" 

////////////////////////////////////////////////////////////////////////////

ForwardRenderer::ForwardRenderer() : fwdShader(nullptr), fwdShaderFlat(nullptr) {
	setLabel("forward renderer");
}	

void ForwardRenderer::init(int winW, int winH) {
	// Enable depth test
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	//glEnable(GL_POLYGON_SMOOTH);
    
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE );
	glEnable( GL_MULTISAMPLE );	

	const glm::vec3 DEFAULT_BACKGROUND_COLOR(0.0f, 0.0f, 0.0f);
	setBackgroundColor(DEFAULT_BACKGROUND_COLOR);
	glClearColor(DEFAULT_BACKGROUND_COLOR.r,
		         DEFAULT_BACKGROUND_COLOR.g,
				 DEFAULT_BACKGROUND_COLOR.b,
				 1.0f);

	////////////////////////////////////////////////////////////////////////////
	ssd.init();
	fwdShader = new ForwardShader();
//	fwdShader->use();
	fwdShaderFlat = new ForwardShaderFlat();
	////////////////////////////////////////////////////////////////////////////	
	VSGLInfoLib::getUniformsInfo(fwdShader->getId());
	VSGLInfoLib::getAttributesInfo(fwdShader->getId());
 	////////////////////////////////////////////////////////////////////////////
	


	//setTextureFiltering(true);
	
	setTextureMapping(true);
	setNormalMapping(true);
	setSpecularMapping(true);
	setOpacityMapping(true);

	setLightingEnabled(true);
	//setAmbientLightLevel(0.5);	
}

void ForwardRenderer::destroy() {
	delete fwdShader;
	delete fwdShaderFlat;
	ssd.destroy();
}

void ForwardRenderer::reloadShaders() {	
	if (fwdShader) delete fwdShader;
	fwdShader = new ForwardShader();
	fwdShader->use();

	if (fwdShaderFlat) delete fwdShaderFlat;
	fwdShaderFlat = new ForwardShaderFlat();

    //setTextureFiltering(isTextureFilteringEnabled());	
	//setAmbientLightLevel(getAmbientLightLevel());
	setLightingEnabled(isLightingEnabled());
	setNormalMapping(isNormalMappingEnabled());
	setSpecularMapping(isSpecularMappingEnabled());
	setOpacityMapping(isOpacityMappingEnabled());
	setTextureMapping(isTextureMappingEnabled());
}

////////////////////////////////////////////////////////////////////////////

void ForwardRenderer::render(const Scene& scene) {
	// update clear color if needed
	const glm::vec3& newbg = scene.getBackgroundColor();
	if ( newbg != getBackgroundColor() ) {
		glClearColor(newbg.r, newbg.g, newbg.b, 1.0f);
		setBackgroundColor(newbg);
	}

	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	fwdShader->use();
	fwdShader->setAmbientLightLevel(scene.getAmbientLightLevel());

	renderSceneGeometry(scene);	

	// render light sources as flat spheres
	fwdShaderFlat->use();	
	
	int cont_p = 0;
	int cont_s = 0;
	int cont_d = 0;

	glm::mat4 VP = *Projection * *View;

	for (int i = 0; i < scene.getLightNodes().size(); ++i ) {
		LightNode *ln = scene.getLightNodes().at(i);
		const glm::mat4& currModel = ln->getModelMatrix();
		setModelMatrix(currModel);

		glm::mat4 ModelView = *View * currModel;
		setModelViewMatrix(ModelView);

		glm::mat4 MVP = VP * currModel;
		setMVPMatrix(MVP);

		switch(ln->getType()) {
			case LightNode::POINT:
				fwdShaderFlat->setPointLightId(cont_p++);
				break;
			case LightNode::SPOT:
				fwdShaderFlat->setPointLightId(cont_s++); 
				break;
			case LightNode::DIRECTIONAL:
				fwdShaderFlat->setPointLightId(cont_d++); 
				break;
		}
		scene.getPointLightAsset()->getAttached()->render(this);		
	}
	

	// render lights
	//	fwdShader_flat->use();
	//	render_lights(&VP, M, scene->getLightNodes());
}

////////////////////////////////////////////////////////////////////////////

/*
	//fwdShader->setNumPointLights(scene->getLightNodes().size());
	printf("--------------------------------------------\n");
//	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "numLights");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "numPointLights");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "numSpotLights");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "numDirLights");
	printf("--------------------------------------------\n");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "pointLights[0].color");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "pointLights[0].posCS");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "pointLights[0].constantAttenuation");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "pointLights[0].linearAttenuation");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "pointLights[0].quadraticAttenuation");
	printf("--------------------------------------------\n");
    VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "spotLights[0].color");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "spotLights[0].posCS");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "spotLights[0].dirCS");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "spotLights[0].cutoff");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "spotLights[0].exponent");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "spotLights[0].constantAttenuation");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "spotLights[0].linearAttenuation");
	VSGLInfoLib::getUniformInBlockInfo(fwdShader->getId(), "Lights", "spotLights[0].quadraticAttenuation");
	*/

/*
void ForwardRenderer::setAmbientLightLevel(float level) {
	Renderer::setAmbientLightLevel(level);
	fwdShader->use();
	fwdShader->setAmbientLightLevel(level);	
}
*/

void ForwardRenderer::setLightingEnabled(bool enabled) {
	Renderer::setLightingEnabled(enabled);
	fwdShader->use();
	fwdShader->setLightingEnabled(enabled);
}

void ForwardRenderer::setNormalMapping(bool enabled) {
	Renderer::setNormalMapping(enabled);
	fwdShader->use();
	fwdShader->setNormalMapping(enabled);
}

void ForwardRenderer::setSpecularMapping(bool enabled) {
	Renderer::setSpecularMapping(enabled);
	fwdShader->use();
	fwdShader->setSpecularMapping(enabled);
}

void ForwardRenderer::setOpacityMapping(bool enabled) {
	Renderer::setOpacityMapping(enabled);
	fwdShader->use();
	fwdShader->setOpacityMapping(enabled);
}

void ForwardRenderer::setTextureMapping(bool enabled) {
	Renderer::setTextureMapping(enabled);
	fwdShader->use();
	fwdShader->setTextureMapping(enabled);
}

////////////////////////////////////////////////////////////////////////////