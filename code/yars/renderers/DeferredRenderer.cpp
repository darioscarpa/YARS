#include "DeferredRenderer.h"

#include "shaders/DRgeometryPassShader.h"
#include "shaders/DRlightPassShader.h"
#include "shaders/DRlightPassShaderPointLight.h"
#include "shaders/DRlightPassShaderSpotLight.h"
#include "shaders/DRlightPassShaderDirLight.h"
#include "shaders/DRindirectLightPassShader.h"
#include "shaders/DRsamplingTestPasssShader.h"
#include "shaders/DRdiscDSSAOshader.h"

#include "FullScreenQuad.h"
#include "../Texture.h"
#include "../Sandbox.h"

#include "../util/vsGLInfoLib.h"

#include "../util/inc_math.h"

const float DEFAULT_DISKDISPLACEMENT = 0.2f;
const float DEFAULT_DISTMAX = 1.5f;
const float DEFAULT_RADLENGTH = 0.3f;

DeferredRenderer::DeferredRenderer() : gbuf(3, true) {
	setLabel("deferred renderer");

	// make cppcheck happy
	gpShader      = nullptr;
	lpShader      = nullptr;
	lpPointShader = nullptr;
	lpSpotShader  = nullptr;
	lpDirShader   = nullptr;
	ilpShader     = nullptr;
	ddoShader     = nullptr;

	//activeShaderProgram = nullptr;
	quad          = nullptr;
	randomTexture = nullptr;
	enabledAO = showingOnlyAO = showingGBuffer = false;	
	dssao_radLength = dssao_diskDisplacement = dssao_distMax = dssao_samples = 0;
	il_radLength = il_diskDisplacement = il_distMax = il_samples = 0;

	bs_emisphereAllPointsOnSurface = true;

	glClearStencil(0x0);
}

void DeferredRenderer::init(int awinW, int awinH) {
	winW = awinW;
	winH = awinH;
//	if (!gbuf.init(winW, winH)) {
//       printf("error initializing GBUFFER\n");
//   }
	gbuf.init(winW, winH);
	gbuf.printFramebufferInfo(GL_DRAW_FRAMEBUFFER);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE );
	//glEnable( GL_MULTISAMPLE );

	glEnable(GL_STENCIL_TEST);
	
	//activeShaderProgram = nullptr;

	shadersCompileConstants["#TRIANGLE_DIVS#"] = "2";

	ssd.init();
	
	gpShader      = new DRgeometryPassShader();
	lpShader      = new DRlightPassShader(shadersCompileConstants);
	lpPointShader = new DRlightPassShaderPointLight();
	lpSpotShader  = new DRlightPassShaderSpotLight();
	lpDirShader   = new DRlightPassShaderDirLight();
	ilpShader     = new DRindirectLightPassShader(shadersCompileConstants);
	stShader      = new DRsamplingTestPassShader(shadersCompileConstants);
	ddoShader     = new DRdiscDSSAOshader(shadersCompileConstants);
	
	setTextureMapping(true);
	setNormalMapping(true);
	setSpecularMapping(true);
	setOpacityMapping(true);
			
	setShowingGBuffer(false);	
		
	setDSSAOradLength(DEFAULT_RADLENGTH);
	setDSSAOdiskDisplacement(DEFAULT_DISKDISPLACEMENT);
	setDSSAOdistMax(DEFAULT_DISTMAX);
	setDSSAOsamples(16);

	setILradLength(DEFAULT_RADLENGTH);
	setILdiskDisplacement(DEFAULT_DISKDISPLACEMENT);
	setILdistMax(DEFAULT_DISTMAX);
	setILsamples(16);

	setSamplingScopeId(0);
	setHemisphereSamplingMode(HemisphereSampling::HEMISPHERE_RANDOM);
	setUniformSamplingRadius(false);

	//setRandomGenerationId(RandomGeneratorShaderSub::func);
	setSamplingPatternId(SamplingPatternShaderSub::dssao);
	
	setAreaComputationId(AreaCalculatorShaderSub::unitValue);
	setAoComputationId(D2DaoShaderSub::dssao);
	setIlComputationId(D2DilShaderSub::dssao);

	setDDOradLength(DEFAULT_RADLENGTH);
	setDDOdiskDisplacement(DEFAULT_DISKDISPLACEMENT);
	setDDOdistMax(DEFAULT_DISTMAX);
	setDDOsamples(16);

//	setAmbientLightLevel(0.5);

	setAmbientPass(true);
	setDirectLightPass(true);
	setIndirectLightPass(true);
	setSamplingTestPass(true);
	setDiskDSSAOpass(false);
	
	setAO(false);
	setShowingOnlyAO(false);
	
	// init full screen quad
	quad = new FullScreenQuad();

	// init texture for random sampling	
	randomTexture = new Texture(GL_TEXTURE_2D, "_textures/noise.png"); //randomTexture200x150.jpg");
//	randomTexture = new Texture(GL_TEXTURE_2D, "textures/random.tga"); //randomTexture200x150.jpg");
	randomTexture->load();

	/*randomTexture->bind(GL_TEXTURE0 + Renderable::tu_random);
	Texture::setWrapMode(GL_TEXTURE0 + Renderable::tu_random, Texture::REPEAT);
	Texture::setFilteringEnabled(GL_TEXTURE0 + Renderable::tu_random, false);*/

		
	randomTexture->bind(GL_TEXTURE0 + GBuffer::GBUFFER_TEXTURE_AVAILABLE);
	Texture::setWrapMode(GL_TEXTURE0 + GBuffer::GBUFFER_TEXTURE_AVAILABLE, Texture::REPEAT);
	//Texture::setFilteringEnabled(GL_TEXTURE0 + GBuffer::GBUFFER_TEXTURE_AVAILABLE, false);
	
	VSGLInfoLib::getCurrentTextureInfo();
	
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	*/
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 		
			
	reshape(awinW, awinH);	
}

void DeferredRenderer::destroy() {	
	if (gpShader) delete gpShader;
	if (lpShader) delete lpShader;
	
	if (lpPointShader) delete lpPointShader;
	if (lpSpotShader)  delete lpSpotShader;
	if (lpDirShader)   delete lpDirShader;

	if (ilpShader) delete ilpShader;
	if (ddoShader) delete ddoShader;

	if (quad) delete quad;
	ssd.destroy();
	//randomTexture->unload
}

void DeferredRenderer::reloadShaders() {	
	if (gpShader) delete gpShader;
	if (lpShader) delete lpShader;
	
	if (lpPointShader) delete lpPointShader;
	if (lpSpotShader)  delete lpSpotShader;
	if (lpDirShader)   delete lpDirShader;

	if (ilpShader) delete ilpShader;
	if (ddoShader) delete ddoShader;

	gpShader      = new DRgeometryPassShader();
	lpShader      = new DRlightPassShader(shadersCompileConstants);
	lpPointShader = new DRlightPassShaderPointLight();
	lpSpotShader  = new DRlightPassShaderSpotLight();
	lpDirShader   = new DRlightPassShaderDirLight();

	ilpShader     = new DRindirectLightPassShader(shadersCompileConstants);
	stShader      = new DRsamplingTestPassShader(shadersCompileConstants);
	ddoShader     = new DRdiscDSSAOshader(shadersCompileConstants);

	setTextureMapping(isTextureMappingEnabled());
	setNormalMapping(isNormalMappingEnabled());
	setSpecularMapping(isSpecularMappingEnabled());
	setOpacityMapping(isOpacityMappingEnabled());
			
	setShowingGBuffer(isShowingGBuffer());	
	setShowingOnlyAO(isShowingOnlyAO());
	setAO(isAOenabled());

	setDSSAOradLength(getDSSAOradLength());
	setDSSAOdiskDisplacement(getDSSAOdiskDisplacement());
	setDSSAOdistMax(getDSSAOdistMax());
	setDSSAOsamples(getDSSAOsamples());

	setILradLength(getILradLength());
	setILdiskDisplacement(getILdiskDisplacement());
	setILdistMax(getILdistMax());
	setILsamples(getILsamples());

	setSamplingScopeId(getSamplingScopeId());
	//setRandomGenerationId(getRandomGenerationId());
	setSamplingPatternId(getSamplingPatternId());

	setAoComputationId(getAoComputationId());
	setIlComputationId(getIlComputationId());

	setDDOradLength(getDDOradLength());
	setDDOdiskDisplacement(getDDOdiskDisplacement());
	setDDOdistMax(getDDOdistMax());
	setDDOsamples(getDDOsamples());

//	setAmbientLightLevel(getAmbientLightLevel());

	setAmbientPass(isAmbientPassEnabled());
	setDirectLightPass(isDirectLightPassEnabled());
	setIndirectLightPass(isIndirectLightPassEnabled());
	setSamplingTestPass(isSamplingTestPassEnabled());


	reshape(winW, winH);
}

void DeferredRenderer::reinit() {
	bool _aoOnly         = showingOnlyAO;
	bool _aoEnabled      = enabledAO;
	bool _showingGBuffer = showingGBuffer;
	int  _aoSamples      = dssao_samples;

//	RandomGeneratorShaderSub::OptionValues _randomGenerationId = getRandomGenerationId();
	SamplingPatternShaderSub::OptionValues _samplingPatternId  = getSamplingPatternId();
	
//	float _ambLevel = getAmbientLightLevel();	
	Renderer::reinit(); 
//	setAmbientLightLevel(_ambLevel);

	setAO(_aoEnabled);
	setShowingOnlyAO(_aoOnly);
	setShowingGBuffer(_showingGBuffer);
	setDSSAOsamples(_aoSamples);	

	//setRandomGenerationId(_randomGenerationId);
	setSamplingPatternId(_samplingPatternId);
};

////////////////////////////////////////////////////////////////////////////

void DeferredRenderer::reshape(int awinW, int awinH) {
	Renderer::reshape(awinW, awinH);
	//if (awinW == 0) awinW++; 	if (awinH == 0) awinH++;
	gbuf.destroy();
	gbuf.init(awinW, awinH);

	ssd.setScreenSize(awinW, awinH);
}

void DeferredRenderer::render(const Scene& scene) {
	///////////////////////////////////////////////////////////////////////////
	// 1st pass: write to Gbuffer	
	gpShader->use();
	gbuf.bindForWriting();		

	glDepthMask(GL_TRUE);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	renderSceneGeometry(scene);
	////////////////	
	//View  = glm::make_mat4(scene->getActiveCamera()->getViewMatrix());
	/*View  = scene->getActiveCamera()->getViewMatrix();
	setViewMatrix(*View);	
	
	//if (mustUpdateProjectionMatrix) {	
	Projection =  scene->getActiveCamera()->getProjectionMatrix();	
	setProjectionMatrix(*Projection);
	InvProjection = scene->getActiveCamera()->getInvProjectionMatrix();
	setInvProjectionMatrix(*InvProjection);
	//}
	//TODO Projection =  scene->getActiveCamera()->getProjectionMatrix();

	setLights(*View, scene);
	
	glm::mat4 VP = *Projection * *View;
	glm::mat4 M  = glm::mat4(1.0);

	//recursive_render(&VP, M, scene->getRootNode());

	for (auto a : scene->getAssetNodes() ) {
		const glm::mat4& currModel = a->getModelMatrix();
		setModelMatrix(currModel);

		glm::mat4 ModelView = *View * currModel;
		setModelViewMatrix(ModelView);

		glm::mat4 MVP = VP * currModel;
		setMVPMatrix(MVP);

		a->getAttached()->render(this);
	}
	*/

	////////////////
	gbuf.unbind();

	///////////////////////////////////////////////////////////////////////////
	// 2nd pass: deferred shading
	glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	/////////////////	
	if (showingGBuffer) {
		showGBuffer();
	} else {
	/*	glEnable(GL_BLEND);
   		glBlendEquation(GL_FUNC_ADD);
   		glBlendFunc(GL_ONE, GL_ONE);
	*/	
		//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		
		gbuf.bindForReadingAsTextures();
	
		//glDisable(GL_CULL_FACE);

		
		const glm::vec3& bg = scene.getBackgroundColor();
		setBackgroundColor(bg);
		glClearColor(bg.r, bg.g, bg.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	

		/////////////////////////////////////////////////////////////////////////////////
		glm::mat4 VP = *Projection * *View;
		glm::mat4 M  = glm::mat4(1.0);
		
		randomTexture->bind(GL_TEXTURE0 + GBuffer::GBUFFER_TEXTURE_AVAILABLE);			
		/////////////////////////////////////////////////////////////////////////////////
		if (isAmbientPassEnabled()) {
			lpShader->use();	
			lpShader->setAmbientLightLevel(scene.getAmbientLightLevel());
			quad->draw();
		}

		/*
		VSGLInfoLib::getUniformInBlockInfo(lpShader->getId(), "Lights", "pointLights[0].color");
		VSGLInfoLib::getUniformInBlockInfo(lpShader->getId(), "Lights", "pointLights[0].posCS");
		VSGLInfoLib::getUniformInBlockInfo(lpShader->getId(), "Lights", "pointLights[0].constantAttenuation");
		VSGLInfoLib::getUniformInBlockInfo(lpShader->getId(), "Lights", "pointLights[0].linearAttenuation");
		VSGLInfoLib::getUniformInBlockInfo(lpShader->getId(), "Lights", "pointLights[0].quadraticAttenuation");
		*/		

		glEnable(GL_BLEND);
   		glBlendEquation(GL_FUNC_ADD);
   		glBlendFunc(GL_ONE, GL_ONE);
	
		if (isDirectLightPassEnabled()) {
			//lpPointShader->use();
			/*VSGLInfoLib::getUniformInBlockInfo(lpPointShader->getId(), "Lights", "pointLights[0].color");
			VSGLInfoLib::getUniformInBlockInfo(lpPointShader->getId(), "Lights", "pointLights[0].posCS");
			VSGLInfoLib::getUniformInBlockInfo(lpPointShader->getId(), "Lights", "pointLights[0].constantAttenuation");
			VSGLInfoLib::getUniformInBlockInfo(lpPointShader->getId(), "Lights", "pointLights[0].linearAttenuation");
			VSGLInfoLib::getUniformInBlockInfo(lpPointShader->getId(), "Lights", "pointLights[0].quadraticAttenuation");
			VSGLInfoLib::getUniformInfo(lpPointShader->getId(), "pointLightId");*/

			/*
			for ( int i = 0; i < scene->getLightNodes().size(); ++i ) {
				lpPointShader->setPointLightId(i);
				quad->draw();
			}
			*/

			lpPointShader->use();			
			int cont_point = 0;
			for (auto l : scene.getLightNodes()) {
				if ( l->getType() == LightNode::POINT) {
					lpPointShader->setPointLightId(cont_point);
					quad->draw();
					++cont_point;
				}
			}
						
			lpSpotShader->use();			
			int cont_spot = 0;
			for (auto l : scene.getLightNodes()) {			
				if ( l->getType() == LightNode::SPOT) {
					lpSpotShader->setSpotLightId(cont_spot);
					quad->draw();
					++cont_spot;
				}
			}

			lpDirShader->use();			
			int cont_dir = 0;
			for (auto l : scene.getLightNodes()) {			
				if ( l->getType() == LightNode::DIRECTIONAL) {
					lpDirShader->setDirLightId(cont_spot);
					quad->draw();
					++cont_dir;
				}
			}
						
			/*		
			VSGLInfoLib::getUniformInBlockInfo(lpShader->getId(), "Matrices", "projMatrix");
			VSGLInfoLib::getUniformInBlockInfo(lpShader->getId(), "Matrices", "invProjMatrix");
			VSGLInfoLib::getUniformInBlockInfo(gpShader->getId(), "Matrices", "projMatrix");
			VSGLInfoLib::getUniformInBlockInfo(gpShader->getId(), "Matrices", "invProjMatrix");
			*/
		}

		if (isIndirectLightPassEnabled()) {
			ilpShader->use();			
			ilpShader->setMousePos( Sandbox::getMousePos() );
			quad->draw();
		}

		if (isDiskDSSAOpassEnabled()) {
			ddoShader->use();
			ddoShader->setMousePos( Sandbox::getMousePos() );
			quad->draw();
		}
			
		if (isSamplingTestPassEnabled()) {
			glDisable(GL_BLEND);
			stShader->use();			
			stShader->setMousePos( Sandbox::getMousePos() );
			quad->draw();
		}
		/////////////////////////////////////////////////////////////////////////////////		
	}
	//mrtShader->unbind();
	//gbuffer->restore();
}


void DeferredRenderer::recursive_render_lights(glm::mat4 *vp,
								glm::mat4 currModel,
								SceneNode *n) {
	//Renderable *r = n->getAttached();
	//LightNode *ln = dynamic_cast<LightNode*>(n);
	if ( LightNode *ln = dynamic_cast<LightNode*>(n) ) {
			glm::mat4 Model = n->getModelMatrix();
			currModel = currModel * Model;
			setModelMatrix(currModel);

			glm::mat4 ModelView = *View * currModel;
			setModelViewMatrix(ModelView);

			glm::mat4 MVP = *vp * currModel;
			setMVPMatrix(MVP);

			lpPointShader->setPointLightId(ln->getArrayPos());
			
			quad->draw();
			//r->render(this, false);
	}
	for(std::vector<SceneNode*>::const_iterator it = n->getChildNodes().begin();
		it != n->getChildNodes().end();
		++it) {
			SceneNode *child = (*it);
			recursive_render_lights(vp, currModel, child);
	}
}

void DeferredRenderer::setShowingGBuffer(bool enabled) {
	showingGBuffer = enabled;
}


void DeferredRenderer::showGBuffer() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gbuf.bindForReading();	
	
    //GLsizei HalfWidth = (GLsizei)(winW / 2.0f);
    //GLsizei HalfHeight = (GLsizei)(winH / 2.0f);

	// 3*2 grid
	/*
	GLsizei offsetH = (GLsizei)(winH / 2.0f);
	GLsizei offsetW = (GLsizei)(winW / 3.0f);

	GLsizei offsetW2 = offsetW * 2;;
	*/

	// 2*2 grid
	GLsizei offsetH = (GLsizei)(winH / 2.0f);
	GLsizei offsetW = (GLsizei)(winW / 2.0f);

	GLsizei offsetW2 = offsetW * 2;;


	//up left
    gbuf.setReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);

    glBlitFramebuffer(0, 0, winW, winH, 
                    0, offsetH, offsetW, winH, 
					GL_COLOR_BUFFER_BIT, GL_LINEAR);
	//down left
    gbuf.setReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_SPECULAR);
	glBlitFramebuffer(0, 0, winW, winH,
                    0, 0, offsetW, offsetH,
					GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	//up mid
    //gbuf.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
    /*glBlitFramebuffer(0, 0, winW, winH, 
                    offsetW, offsetH, offsetW2, winH,
					GL_COLOR_BUFFER_BIT, GL_LINEAR);*/

	//up mid
	//gbuf.SetReadDepthBuffer();
    glBlitFramebuffer(0, 0, winW, winH, 
                    offsetW, offsetH, offsetW2, winH,
					GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	
	//down mid
    gbuf.setReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
    glBlitFramebuffer(0, 0, winW, winH, 
                    offsetW, 0, offsetW2, offsetH, 
					GL_COLOR_BUFFER_BIT, GL_LINEAR);
    
	/*
	//up right
	gbuf.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXOORD);
    glBlitFramebuffer(0, 0, winW, winH, 
                    offsetW2, offsetH, winW, winH,
					GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	//down right
	//gbuf.SetReadDepthBuffer();
    glBlitFramebuffer(0, 0, winW, winH, 
                    offsetW2, 0, winW, offsetH, 
					GL_DEPTH_BUFFER_BIT, GL_NEAREST);	
					*/
	
	/*GLenum errCode = glGetError();
	const GLubyte *errString;
	if (errCode != GL_NO_ERROR) {
	    errString = gluErrorString(errCode);
		fprintf (stderr, "OpenGL Error: %s\n", errString);
	
		GLint buf, s;
		glGetIntegerv(GL_SAMPLE_BUFFERS, &buf);
		printf("number of sample buffers is %d\n", buf);
		glGetIntegerv(GL_SAMPLES, &s);
		printf("number of samples is %d\n", s);
	}*/	
}


/////////////////////
/*void DeferredRenderer::setLightingEnabled(bool enabled) {
	Renderer::setLightingEnabled(enabled); 
//	lpPointShader->use();
//	lpPointShader->setPointLightEnabled(enabled);	
}
*/

/*
void DeferredRenderer::setAmbientLightLevel(float level) {
	Renderer::setAmbientLightLevel(level);	
	lpShader->use();
	lpShader->setAmbientLightLevel(level);	
}
*/
/////////////////////
void DeferredRenderer::setAO(bool enabled) {
	enabledAO = enabled;
	lpShader->use();	
	lpShader->setAmbientOcclusion(enabled);	
}

void DeferredRenderer::setShowingOnlyAO(bool enabled) {
	showingOnlyAO = enabled;
	lpShader->use();	
	lpShader->setShowingOnlyAO(enabled);	
}

void DeferredRenderer::setDSSAOradLength(float val) {	
	dssao_radLength = val;
	lpShader->use();
	lpShader->setDSSAOradLength(val);	
		
	if (!samplingScopeId) setSTradLength(val);
}

void DeferredRenderer::setDSSAOdiskDisplacement(float val) {	
	dssao_diskDisplacement = val;
	lpShader->use();
	lpShader->setDSSAOdiskDisplacement(val);	
	
	if (!samplingScopeId) setSTdiskDisplacement(val);
}

void DeferredRenderer::setDSSAOdistMax(float val) {	
	dssao_distMax = val;
	lpShader->use();
	lpShader->setDSSAOdistMax(val);	

	if (!samplingScopeId) setSTdistMax(val);
}

void DeferredRenderer::setDSSAOsamples(int samples) {
	dssao_samples = samples;
	lpShader->use();
	lpShader->setDSSAOsamples(samples/8);	

	if (!samplingScopeId) setSTsamples(samples);
}


/////////////////////
void DeferredRenderer::setILradLength(float val) {	
	il_radLength = val;
	
	ilpShader->use();
	ilpShader->setILradLength(val);

	if (samplingScopeId) setSTradLength(val);
}

void DeferredRenderer::setILdiskDisplacement(float val) {	
	il_diskDisplacement = val;
	ilpShader->use();
	ilpShader->setILdiskDisplacement(val);	

	if (samplingScopeId) setSTdiskDisplacement(val);
}

void DeferredRenderer::setILdistMax(float val) {	
	il_distMax = val;
	ilpShader->use();
	ilpShader->setILdistMax(val);	

	if (samplingScopeId) setSTdistMax(val);
}

void DeferredRenderer::setILsamples(int samples) {
	il_samples = samples;
	
	ilpShader->use();
	ilpShader->setILsamples(samples/8);	

	if (samplingScopeId) setSTsamples(samples);
}
/////////////////////

void DeferredRenderer::setDDOradLength(float val) {	
	ddo_radLength = val;
	
	ddoShader->use();
	ddoShader->setMaxSamplingRadius(val);

	//if (samplingScopeId) setSTradLength(val);
}

void DeferredRenderer::setDDOdiskDisplacement(float val) {	
	ddo_diskDisplacement = val;
	ddoShader->use();
	ddoShader->setMaxDiskDisplacement(val);	

	//if (samplingScopeId) setSTdiskDisplacement(val);
}

void DeferredRenderer::setDDOdistMax(float val) {	
	ddo_distMax = val;
	ddoShader->use();
	ddoShader->setMaxDiskDistance(val);	

	//if (samplingScopeId) setSTdistMax(val);
}

void DeferredRenderer::setDDOsamples(int samples) {
	ddo_samples = samples;
	
	ddoShader->use();
	ddoShader->setNumSamples(samples/8);	

	//if (samplingScopeId) setSTsamples(samples);
}
/////////////////////
/*
void DeferredRenderer::setRandomGenerationId(RandomGeneratorShaderSub::OptionValues id) {
	//if (randomGenerationId == id ) return;
	randomGenerationId = id;
	
	if ( lpShader->isInitialized() ) {
		lpShader->use();
		lpShader->setRandomGenerator(id);
	}
	if (ilpShader->isInitialized()) {
		ilpShader->use();
		ilpShader->setRandomGenerator(id);
	}	
	if (stShader->isInitialized()) {
		stShader->use();
		stShader->setRandomGenerator(id);
	}	
}
*/



void DeferredRenderer::setUniformSamplingRadius(bool tf) {
	bs_emisphereAllPointsOnSurface = tf;
	//setupKernel(MAX_KERNEL_SIZE, tf, bs_octantInterleaving);
	setHemisphereSamplingMode(getHemisphereSamplingMode());
}

void DeferredRenderer::setupEmisphereKernel_random(int kernelSize, bool allPointsOnEmishphereSurface, bool distanceFalloff) {
	std::vector<glm::vec3> kernel;
	kernel.reserve(kernelSize);

	HemisphereSampling::getRandom(kernelSize, allPointsOnEmishphereSurface, distanceFalloff, kernel);

	lpShader->setSampleKernel(kernel);	
	ilpShader->setSampleKernel(kernel);	
	stShader->setSampleKernel(kernel);	
}

void DeferredRenderer::setupEmisphereKernel_triangleHierarchy(int kernelSize, bool allPointsOnEmishphereSurface, bool octantSampleInterleaving) {	
	const int nsquare = kernelSize/4;
	const int n       = log(nsquare) / log(2);		

	std::vector<glm::vec3> orderedSamplingVectors(kernelSize);
	std::vector<int>   skipTable(kernelSize);
	std::vector<float> solidAngleTable(kernelSize);
	//std::vector<float> vectorLenTable(kernelSize);
	std::vector<float> sampleWeightTable(kernelSize);

	HemisphereSampling::getTrianglesHierarchy(n, allPointsOnEmishphereSurface, octantSampleInterleaving,
		orderedSamplingVectors, skipTable, solidAngleTable, sampleWeightTable);
	
	lpShader->setSampleKernel(orderedSamplingVectors);
	lpShader->setSampleKernelDetails(skipTable, solidAngleTable, sampleWeightTable);

	ilpShader->setSampleKernel(orderedSamplingVectors);	
	ilpShader->setSampleKernelDetails(skipTable, solidAngleTable, sampleWeightTable);

	stShader->setSampleKernel(orderedSamplingVectors);	
	stShader->setSampleKernelDetails(skipTable, solidAngleTable, sampleWeightTable);
	//printf("uKernelOffsets infodump - BEGIN **\n");
	//VSGLInfoLib::getUniformInfo(blendShader->getId(), "uKernelOffsets");
	//printf("uKernelOffsets infodump - END **\n");
	///////////////////////////////////////////////////////////////////////////	
}
void DeferredRenderer::setHemisphereSamplingMode(HemisphereSampling::KernelMode_t samplingMode) {
	bs_emisphereSamplingMode = samplingMode;
	
	lpShader->setEmisphereSamplingTechnique(samplingMode);
	ilpShader->setEmisphereSamplingTechnique(samplingMode);
	stShader->setEmisphereSamplingTechnique(samplingMode);

	int bs_samples = dssao_samples;
	switch (samplingMode) {
		case HemisphereSampling::HEMISPHERE_RANDOM: 				
		//case HemisphereSampling::HEMISPHERE_PYRAMID:
		//case HemisphereSampling::HEMISPHERE_PYRAMID_DSSO:
			setupEmisphereKernel_random(bs_samples, bs_emisphereAllPointsOnSurface, false);
			break;
		case HemisphereSampling::HEMISPHERE_RANDOM_DISTANCEFALLOFF: 				
		//case HemisphereSampling::HEMISPHERE_PYRAMID_DISTANCEFALLOFF:
		//case HemisphereSampling::HEMISPHERE_PYRAMID_DSSO_DISTANCEFALLOFF:
			setupEmisphereKernel_random(bs_samples, bs_emisphereAllPointsOnSurface, true);
			break;
		case HemisphereSampling::HEMISPHERE_TRIANGLEHIERARCHY:
			setupEmisphereKernel_triangleHierarchy(bs_samples == 64 ? 64 : 16, bs_emisphereAllPointsOnSurface, false);
			break;
		case HemisphereSampling::HEMISPHERE_TRIANGLEHIERARCHY_INTERLEAVED:
			int numSamples = bs_samples == 64 ? 64 : 16;
			setupEmisphereKernel_triangleHierarchy(bs_samples == 64 ? 64 : 16, bs_emisphereAllPointsOnSurface, true);
			break;			

	} 
};


void DeferredRenderer::setSamplingPatternId(SamplingPatternShaderSub::OptionValues id) {
	//if (samplingPatternId == id ) return;
	samplingPatternId = id;
	
	if ( lpShader->isInitialized() ) {
		lpShader->use();
		lpShader->setSamplingPattern(id);

		/*if (id==SamplingPatternShaderSub::emisphere) {
			setupKernel(
		}*/
	}
	if (ilpShader->isInitialized()) {
		ilpShader->use();
		ilpShader->setSamplingPattern(id);
	}	
	if (stShader->isInitialized()) {
		stShader->use();
		stShader->setSamplingPattern(id);
	}
	/*
	if (ddoShader->isInitialized()) {
		ddoShader->use();
	}*/
}

void DeferredRenderer::setAoComputationId(D2DaoShaderSub::OptionValues id) {
	aoComputationId = id;
	if (lpShader->isInitialized()) {
		lpShader->use();
		lpShader->setAoCalculation(id);
	}
}

void DeferredRenderer::setIlComputationId(D2DilShaderSub::OptionValues id) {
	ilComputationId = id;
	if (ilpShader->isInitialized()) {
		ilpShader->use();
		ilpShader->setIlCalculation(id);
	}
}

void DeferredRenderer::setAreaComputationId(AreaCalculatorShaderSub::OptionValues id) {
	areaComputationId = id;
	if (gpShader->isInitialized()) {
		gpShader->use();
		gpShader->setAreaCalculation(id);
	}
}


/////////////////////
void DeferredRenderer::setSamplingScopeId(int id) {
	if (samplingScopeId != id) {
		samplingScopeId = id;
		stShader->use();
		if (id == 0) {
			stShader->setSamples(getDSSAOsamples()/8);	
			stShader->setRadLength(getDSSAOradLength());
			stShader->setDistMax(getDSSAOdistMax());
			stShader->setDiskDisplacement(getDSSAOdiskDisplacement());
		} else { // == 1
			stShader->setSamples(getILsamples()/8);	
			stShader->setRadLength(getILradLength());
			stShader->setDistMax(getILdistMax());
			stShader->setDiskDisplacement(getILdiskDisplacement());
		}
	}
}

void DeferredRenderer::setSTradLength(float val) {	
	stShader->use();
	stShader->setRadLength(val);
}

void DeferredRenderer::setSTdiskDisplacement(float val) {	
	stShader->use();
	stShader->setDiskDisplacement(val);	
}

void DeferredRenderer::setSTdistMax(float val) {	
	stShader->use();
	stShader->setDistMax(val);	
}

void DeferredRenderer::setSTsamples(int samples) {
	stShader->use();
	stShader->setSamples(samples/8);	
}
/////////////////////
void DeferredRenderer::setNormalMapping(bool enabled) {
	Renderer::setNormalMapping(enabled);
	gpShader->use();
	gpShader->setNormalMapping(enabled);
}

void DeferredRenderer::setSpecularMapping(bool enabled) {
	Renderer::setSpecularMapping(enabled);
	gpShader->use();
	gpShader->setSpecularMapping(enabled);	
}

void DeferredRenderer::setOpacityMapping(bool enabled) {
	Renderer::setOpacityMapping(enabled);
	gpShader->use();
	gpShader->setOpacityMapping(enabled);	
}

void DeferredRenderer::setTextureMapping(bool enabled) {
	Renderer::setTextureMapping(enabled);
	gpShader->use();
	gpShader->setTextureMapping(enabled);
}

///////////////////////////////////////////////////////////////////////////

void printBufInfo() {
	printf("-------------------\n");
	GLint buf, s;
	glGetIntegerv(GL_SAMPLE_BUFFERS, &buf);	
	printf("number of sample buffers is %d\n", buf);
	glGetIntegerv(GL_SAMPLES, &s);
	printf("number of samples is %d\n", s);
	printf("-------------------\n");
}

