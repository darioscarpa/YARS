#include "DoublePassBlendingDeferredRenderer.h"


#include "../shaders/DRgeometryPassShader.h"
#include "../shaders/DRlightPassShader.h"
#include "../shaders/DRlightPassShaderPointLight.h"
#include "../shaders/DRlightPassShaderSpotLight.h"
#include "../shaders/DRlightPassShaderDirLight.h"
#include "../shaders/DRindirectLightPassShader.h"
#include "../shaders/DRdirectLightPassShader.h"
#include "../shaders/DRblendingPassShader.h"
#include "../shaders/DRsamplingTestPasssShader.h"
#include "../shaders/DRdiscDSSAOshader.h"

#include "../HemisphereSampling.h"
#include "../FullScreenQuad.h"
#include "../../Texture.h"
#include "../../Sandbox.h"

#include "../../util/vsGLInfoLib.h"

#include "../../util/inc_math.h"


const float DEFAULT_DISKDISPLACEMENT = 0.2f;
const float DEFAULT_DISTMAX = 1.5f;
const float DEFAULT_RADLENGTH = 0.3f;

const char* DoublePassBlendingDeferredRenderer::TRIANGLE_DIVS = "#TRIANGLE_DIVS#";

void DoublePassBlendingDeferredRenderer::setTriangleSideDivs(int divs) {
	std::ostringstream v; v << divs;
	shadersCompileConstants[TRIANGLE_DIVS] = v.str();
	reloadShaders();
}
int DoublePassBlendingDeferredRenderer::getTriangleSideDivs() const {
	auto it = shadersCompileConstants.find(TRIANGLE_DIVS);
	return std::stoi(it->second);
}

void DoublePassBlendingDeferredRenderer::setTechnique(Technique_t t) {
	technique = t;
	blendShader->setTechnique(t);
	switch(t) {
			case PYRAMID_BUCKETS:
				break;
			case SIMPLE_SUM:
				break;
			case TRIANGLE_HIERARCHICAL_SKIP:
				setHemisphereSamplingMode(HemisphereSampling::HEMISPHERE_TRIANGLEHIERARCHY);
				break;			
	}
}

void DoublePassBlendingDeferredRenderer::setUniformSamplingRadius(bool tf) {
	bs_emisphereAllPointsOnSurface = tf;
	//setupKernel(MAX_KERNEL_SIZE, tf, bs_octantInterleaving);
	setHemisphereSamplingMode(getHemisphereSamplingMode());
}

/*
void DoublePassBlendingDeferredRenderer::setSamplingOctantInterleaving(bool tf) {
	bs_octantInterleaving = tf;
	//setupKernel(MAX_KERNEL_SIZE, bs_uniformRadLength, tf);
	setSamplingMode(getHemisphereSamplingMode());
}
*/

void DoublePassBlendingDeferredRenderer::setupEmisphereKernel_random(int kernelSize, bool allPointsOnEmishphereSurface, bool distanceFalloff) {
	std::vector<glm::vec3> kernel;
	kernel.reserve(kernelSize);

	HemisphereSampling::getRandom(kernelSize, allPointsOnEmishphereSurface, distanceFalloff, kernel);

	blendShader->setSampleKernel(kernel);	
}

void DoublePassBlendingDeferredRenderer::setupEmisphereKernel_triangleHierarchy(int kernelSize, bool allPointsOnEmishphereSurface, bool octantSampleInterleaving) {	
	const int nsquare = kernelSize/4;
	const int n       = log(nsquare) / log(2);		

	std::vector<glm::vec3> orderedSamplingVectors(kernelSize);
	std::vector<int>   skipTable(kernelSize);
	std::vector<float> solidAngleTable(kernelSize);
	//std::vector<float> vectorLenTable(kernelSize);
	std::vector<float> sampleWeightTable(kernelSize);

	HemisphereSampling::getTrianglesHierarchy(n, allPointsOnEmishphereSurface, octantSampleInterleaving,
		orderedSamplingVectors, skipTable, solidAngleTable, sampleWeightTable);
	
	blendShader->setSampleKernel(orderedSamplingVectors);
	blendShader->setSampleKernelDetails(skipTable, solidAngleTable, sampleWeightTable);
	//printf("uKernelOffsets infodump - BEGIN **\n");
	//VSGLInfoLib::getUniformInfo(blendShader->getId(), "uKernelOffsets");
	//printf("uKernelOffsets infodump - END **\n");
	///////////////////////////////////////////////////////////////////////////	
}

void DoublePassBlendingDeferredRenderer::setScreenSpaceRayOrigin() {	
	glm::vec2 newpos = Sandbox::getActiveScene()->getSpecialPoint();
	if (screenSpaceRayOrigin != newpos && blendShader->isInitialized()) {
		screenSpaceRayOrigin = newpos;
		blendShader->use();
		blendShader->setRayOriginPos(screenSpaceRayOrigin);
	}
}

void DoublePassBlendingDeferredRenderer::setScreenSpaceEmisphereCenter() {	
	glm::vec2 newpos = Sandbox::getActiveScene()->getEmisphereCenterPoint();
	if (screenSpaceEmisphereCenter != newpos && blendShader->isInitialized()) {
		screenSpaceEmisphereCenter = newpos;
		blendShader->use();
		//blendShader->setEmisphereCenterPos(screenSpaceEmisphereCenter);
		blendShader->setMousePos(screenSpaceEmisphereCenter);
	}
}

DoublePassBlendingDeferredRenderer::DoublePassBlendingDeferredRenderer() : gbuf(3, true) , dlBuf(2, false) {
	setLabel("2pass blending deferred renderer");

	// make cppcheck happy
	gpShader      = nullptr;
	dlpShader     = nullptr;
	blendShader   = nullptr;

	
	//activeShaderProgram = nullptr;
	quad          = nullptr;
	randomTexture = nullptr;
	showingDLBuffer = showingGBuffer = false;	
	bs_radLength = bs_diskDisplacement = bs_distMax = bs_samples = 0;
	bs_emisphereAllPointsOnSurface = true;
	//bs_octantInterleaving = false;

	glClearStencil(0x0);
}

void DoublePassBlendingDeferredRenderer::init(int awinW, int awinH) {
	winW = awinW;
	winH = awinH;
	/*if (!gbuf.init(winW, winH)) {
        printf("error initializing GBUFFER\n");
    }*/
	gbuf.init(winW, winH);
	gbuf.printFramebufferInfo(GL_DRAW_FRAMEBUFFER);

	/*if (!dlBuf.init(winW, winH, GBuffer::GBUFFER_TEXTURE_AVAILABLE + 1)) {
        printf("error initializing DLBUFFER\n");
    }*/
	dlBuf.init(winW, winH, GBuffer::GBUFFER_TEXTURE_AVAILABLE + 1);
	dlBuf.printFramebufferInfo(GL_DRAW_FRAMEBUFFER);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	//glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE );
	//glEnable( GL_MULTISAMPLE );

	glEnable(GL_STENCIL_TEST);
	
	//activeShaderProgram = nullptr;

	shadersCompileConstants[TRIANGLE_DIVS] = "2";
	ssd.init();
	
	gpShader      = new DRgeometryPassShader();
	dlpShader     = new DRdirectLightPassShader();	
	blendShader   = new DRblendingPassShader(shadersCompileConstants);
	
	screenSpaceRayOrigin = glm::vec2(0,0);
	screenSpaceEmisphereCenter = glm::vec2(300,300);

	setTextureMapping(true);
	setNormalMapping(true);
	setSpecularMapping(true);
	setOpacityMapping(true);
			
	setShowingGBuffer(false);	
	setShowingDLBuffer(false);
	setShowingArea(false);
	setShowingSampling(false);
	setShowingSamplingDensity(false);
	setShowingZoomedArea(false);
		
	setBSmaxSamplingRadius(DEFAULT_RADLENGTH);
	setBSdiskDisplacement(DEFAULT_DISKDISPLACEMENT);
	setBSdistMax(DEFAULT_DISTMAX);
	setBSnumSamples(16);

	setAoMultiplier(1.0f);
	setIlMultiplier(1.0f);
	setAreaMultiplier(1.0f);
	setSolidAngleMultiplier(1.0f);

	setHemisphereSamplingMode(HemisphereSampling::HEMISPHERE_RANDOM);
	setTechnique(PYRAMID_BUCKETS);

	//setRandomGenerationId(RandomGeneratorShaderSub::func);
	setSamplingPatternId(SamplingPatternShaderSub::emisphere);
	setAoComputationId(D2DaoShaderSub::dssao);
	setIlComputationId(D2DilShaderSub::dssao);
	//setAreaComputationId(AreaCalculatorShaderSub::unitValue); 
	setAreaComputationId(AreaCalculatorShaderSub::circumscribedCircleArea); 

//	setAmbientLightLevel(0.5);

	/*
	setAmbientPass(true);
	setDirectLightPass(true);
	setIndirectLightPass(true);
	setSamplingTestPass(true);
	setDiskDSSAOpass(false);
	*/
	
	setAlbedoEnabled(true);
	setAoEnabled(true);
	setIlEnabled(true);
	setAmbientEnabled(true);
	setDiffuseEnabled(true);
	setSpecularEnabled(true);
	
	
	// init full screen quad
	quad = new FullScreenQuad();

	// init texture for random sampling	
	randomTexture = new Texture(GL_TEXTURE_2D, "_textures/noise.png"); 
	randomTexture->load();
	randomTexture->bind(GL_TEXTURE0 + GBuffer::GBUFFER_TEXTURE_AVAILABLE);
	Texture::setWrapMode(GL_TEXTURE0 + GBuffer::GBUFFER_TEXTURE_AVAILABLE, Texture::REPEAT);
	VSGLInfoLib::getCurrentTextureInfo();
			
	reshape(awinW, awinH);	
}

void DoublePassBlendingDeferredRenderer::destroy() {	
	if (gpShader) delete gpShader;
		
	if (dlpShader) delete dlpShader;
	if (blendShader) delete blendShader;
	
	if (quad) delete quad;
	ssd.destroy();	
}

void DoublePassBlendingDeferredRenderer::reloadShaders() {	
	if (gpShader)   delete  gpShader;
	if (dlpShader)   delete dlpShader;
	if (blendShader) delete blendShader;

	gpShader      = new DRgeometryPassShader();
	dlpShader     = new DRdirectLightPassShader();
	blendShader   = new DRblendingPassShader(shadersCompileConstants);

	VSGLInfoLib::getUniformsInfo(blendShader->getId());

	screenSpaceRayOrigin = glm::vec2(0,0);
	screenSpaceEmisphereCenter = glm::vec2(0,0);
	
	setTextureMapping(isTextureMappingEnabled());
	setNormalMapping(isNormalMappingEnabled());
	setSpecularMapping(isSpecularMappingEnabled());
	setOpacityMapping(isOpacityMappingEnabled());
			
	setShowingGBuffer(isShowingGBuffer());	
	setShowingDLBuffer(isShowingDLBuffer());	
	setShowingArea(isShowingArea());
	setShowingSampling(isShowingSampling());
	setShowingSamplingDensity(isShowingSamplingDensity());
	setShowingZoomedArea(isShowingZoomedArea());

	setBSmaxSamplingRadius(getBSmaxSamplingRadius());
	setBSdiskDisplacement(getBSdiskDisplacement());
	setBSdistMax(getBSdistMax());
	setBSnumSamples(getBSnumSamples());

//	setSamplingScopeId(getSamplingScopeId());
	//setRandomGenerationId(getRandomGenerationId());
	setSamplingPatternId(getSamplingPatternId());

	setAoMultiplier(getAoMultiplier());
	setIlMultiplier(getIlMultiplier());
	setAreaMultiplier(getAreaMultiplier());
	setSolidAngleMultiplier(getSolidAngleMultiplier());

	//setupKernel(MAX_KERNEL_SIZE, bs_uniformRadLength, bs_octantInterleaving);
	setHemisphereSamplingMode(getHemisphereSamplingMode());
	setTechnique(getTechnique());

	setAlbedoEnabled(isAlbedoEnabled());
	setAoEnabled(isAoEnabled());
	setIlEnabled(isIlEnabled());
	setAmbientEnabled(isAmbientEnabled());
	setDiffuseEnabled(isDiffuseEnabled());
	setSpecularEnabled(isSpecularEnabled());

	setAoComputationId(getAoComputationId());
	setIlComputationId(getIlComputationId());
	setAreaComputationId(getAreaComputationId());
	
	//setAmbientLightLevel(getAmbientLightLevel());

	reshape(winW, winH);
}

void DoublePassBlendingDeferredRenderer::reinit() {
	
	bool _showingGBuffer  = showingGBuffer;
	bool _showingDLBuffer = showingDLBuffer;
	int  _bsSamples       = bs_samples;

//	RandomGeneratorShaderSub::OptionValues _randomGenerationId = getRandomGenerationId();
	SamplingPatternShaderSub::OptionValues _samplingPatternId  = getSamplingPatternId();
	
	screenSpaceRayOrigin = glm::vec2(0,0);
	screenSpaceEmisphereCenter = glm::vec2(0,0);

	//float _ambLevel = getAmbientLightLevel();	
	Renderer::reinit(); 
	//setAmbientLightLevel(_ambLevel);

	setShowingGBuffer(_showingGBuffer);
	setShowingGBuffer(_showingDLBuffer);
	
	setBSmaxSamplingRadius(getBSmaxSamplingRadius());
	setBSdiskDisplacement(getBSdiskDisplacement());
	setBSdistMax(getBSdistMax());
	setBSnumSamples(getBSnumSamples());

	setAoMultiplier(getAoMultiplier());
	setIlMultiplier(getIlMultiplier());
	setAreaMultiplier(getAreaMultiplier());
	
	setTechnique(getTechnique());
	setHemisphereSamplingMode(getHemisphereSamplingMode());

	setAreaComputationId(getAreaComputationId());
	//setRandomGenerationId(_randomGenerationId);
	setSamplingPatternId(_samplingPatternId);

	setScreenSpaceRayOrigin();
	setScreenSpaceEmisphereCenter();
};

////////////////////////////////////////////////////////////////////////////



void DoublePassBlendingDeferredRenderer::reshape(int awinW, int awinH) {
	Renderer::reshape(awinW, awinH);
	//if (awinW == 0) awinW++; 	if (awinH == 0) awinH++;
	gbuf.destroy();
	gbuf.init(awinW, awinH);

	dlBuf.destroy();
	dlBuf.init(awinW, awinH, GBuffer::GBUFFER_TEXTURE_AVAILABLE + 1);

	ssd.setScreenSize(awinW, awinH);
}

void DoublePassBlendingDeferredRenderer::render(const Scene& scene) {
	setScreenSpaceRayOrigin();
	setScreenSpaceEmisphereCenter();
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
	
	////////////////	
	
	renderSceneGeometry(scene);	

	////////////////

	gbuf.unbind();

	// DIRECT LIGHTING PASS //
	gbuf.bindForReadingAsTextures();
	dlBuf.bindForWriting();	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	///
	//M  = glm::mat4(1.0);
	//setModelViewMatrix(*View * M);
	//glm::mat4 MVP = VP * M;
	//setMVPMatrix(MVP);
	///
		
	dlpShader->use();	
	
	/*
	VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "numPointLights");
	VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "numSpotLights");
	VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "numDirLights");

	printf("--------------------------------------------\n");
	printf("\npointLights[0].color"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "pointLights[0].color");
	printf("\npointLights[0].posCS"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "pointLights[0].posCS");
	printf("\npointLights[0].constantAttenuation"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "pointLights[0].constantAttenuation");
	printf("\npointLights[0].linearAttenuation"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "pointLights[0].linearAttenuation");
	printf("\npointLights[0].quadraticAttenuation"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "pointLights[0].quadraticAttenuation");
	printf("--------------------------------------------\n");
    printf("\nspotLights[0].color"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "spotLights[0].color");
	printf("\nspotLights[0].posCS"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "spotLights[0].posCS");
	printf("\nspotLights[0].dirCS"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "spotLights[0].dirCS");
	printf("\nspotLights[0].cosCutoffOuter"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "spotLights[0].cosCutoffOuter");
	printf("\nspotLights[0].cosCutoffInner"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "spotLights[0].cosCutoffInner");
	printf("\nspotLights[0].exponent"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "spotLights[0].exponent");
	printf("\nspotLights[0].constantAttenuation"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "spotLights[0].constantAttenuation");
	printf("\nspotLights[0].linearAttenuation"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "spotLights[0].linearAttenuation");
	printf("\nspotLights[0].quadraticAttenuation"); VSGLInfoLib::getUniformInBlockInfo(dlpShader->getId(), "Lights", "spotLights[0].quadraticAttenuation");
	printf("--------------------------------------------\n");
    */

	quad->draw();
	dlBuf.unbind();

	if (showingGBuffer) {
		showGBuffer();
	} else if (showingDLBuffer) {
		showDLBuffer();
	} /* else if (showingArea) {

	} */ else {
		glDisable(GL_DEPTH_TEST);
		const glm::vec3& bg = scene.getBackgroundColor();
		setBackgroundColor(bg);
		glClearColor(bg.r, bg.g, bg.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	
		gbuf.bindForReadingAsTextures();
		dlBuf.bindForReadingAsTextures();
		blendShader->use();
		blendShader->setAmbientLightLevel(scene.getAmbientLightLevel());
		//blendShader->setMousePos( Sandbox::getMousePos() );
		blendShader->setRayOriginPos(Sandbox::getMousePos());

		quad->draw();
	}
	//////////////////////////	
}


void DoublePassBlendingDeferredRenderer::setShowingGBuffer(bool enabled) {
	showingGBuffer = enabled;
	if (enabled) {
		setShowingDLBuffer(false);
		setShowingArea(false);
	}
}

void DoublePassBlendingDeferredRenderer::setShowingDLBuffer(bool enabled) {
	showingDLBuffer = enabled;	
	if (enabled) {
		setShowingGBuffer(false);
		setShowingArea(false);
	}	
}

void DoublePassBlendingDeferredRenderer::setShowingArea(bool enabled) {
	showingArea = enabled;
	blendShader->use();
	blendShader->setShowAreas(enabled);
	if (enabled) {
		setShowingGBuffer(false);
		setShowingDLBuffer(false);		
	}	
}

void DoublePassBlendingDeferredRenderer::setShowingSampling(bool enabled) {
	showingSampling = enabled;
	blendShader->use();
	blendShader->setShowSampling(enabled);	
}

void DoublePassBlendingDeferredRenderer::setShowingSamplingDensity(bool enabled) {
	showingSamplingDensity = enabled;
	blendShader->use();
	blendShader->setShowSamplingDensity(enabled);	
}

void DoublePassBlendingDeferredRenderer::setShowingZoomedArea(bool enabled) {
	showingZoomedArea = enabled;
	blendShader->use();
	blendShader->setShowZoomedArea(enabled);	
}


void DoublePassBlendingDeferredRenderer::showGBuffer() {
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

void DoublePassBlendingDeferredRenderer::showDLBuffer() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	dlBuf.bindForReading();

	// 2*2 grid
	GLsizei offsetH = (GLsizei)(winH / 2.0f);
	GLsizei offsetW = (GLsizei)(winW / 2.0f);

	GLsizei offsetW2 = offsetW * 2;;


	//up left
	dlBuf.setReadBuffer(DlBuffer::DLBUFFER_TEXTURE_TYPE_DIFFUSE);
    glBlitFramebuffer(0, 0, winW, winH, 
                    0, offsetH, offsetW, winH, 
					GL_COLOR_BUFFER_BIT, GL_LINEAR);
	//down left
	dlBuf.setReadBuffer(DlBuffer::DLBUFFER_TEXTURE_TYPE_SPECULAR);
    glBlitFramebuffer(0, 0, winW, winH,
                    0, 0, offsetW, offsetH,
					GL_COLOR_BUFFER_BIT, GL_LINEAR);    
	
}


void DoublePassBlendingDeferredRenderer::setAlbedoEnabled(bool enabled){
	albedoEnabled = enabled;
	blendShader->use(); blendShader->setAlbedoEnabled(enabled);	
	//setEnabled<&albedoEnabled, blendShader, &DRblendingPassShader::setAlbedoEnabled>(enabled); 
}
void DoublePassBlendingDeferredRenderer::setAoEnabled(bool enabled){
	aoEnabled = enabled;
	blendShader->use(); blendShader->setAoEnabled(enabled);
}
void DoublePassBlendingDeferredRenderer::setIlEnabled(bool enabled){
	ilEnabled = enabled;
	blendShader->use(); blendShader->setIlEnabled(enabled);
}
void DoublePassBlendingDeferredRenderer::setAmbientEnabled(bool enabled){
	ambientLightEnabled = enabled;
	blendShader->use(); blendShader->setAmbientEnabled(enabled);
}
void DoublePassBlendingDeferredRenderer::setDiffuseEnabled(bool enabled){
	diffuseLightEnabled = enabled;
	blendShader->use(); blendShader->setDiffuseEnabled(enabled);
}
void DoublePassBlendingDeferredRenderer::setSpecularEnabled(bool enabled){
	specularLightEnabled = enabled;
	blendShader->use(); blendShader->setSpecularEnabled(enabled);
}

/////////////////////
void DoublePassBlendingDeferredRenderer::setBSmaxSamplingRadius(float val) {	
	bs_radLength = val;
	
	blendShader->use();
	blendShader->setMaxSamplingRadius(val);

	//if (samplingScopeId) setSTradLength(val);
}

void DoublePassBlendingDeferredRenderer::setBSdiskDisplacement(float val) {	
	bs_diskDisplacement = val;
	blendShader->use();
	blendShader->setDiskDisplacement(val);	

	//if (samplingScopeId) setSTdiskDisplacement(val);
}

void DoublePassBlendingDeferredRenderer::setBSdistMax(float val) {	
	bs_distMax = val;
	blendShader->use();
	blendShader->setDistMax(val);	

	//if (samplingScopeId) setSTdistMax(val);
}

void DoublePassBlendingDeferredRenderer::setBSnumSamples(int samples) {
	bs_samples = samples;
	
	blendShader->use();
	blendShader->setNumSamples(samples/8);	
	setHemisphereSamplingMode(getHemisphereSamplingMode());
	//if (samplingScopeId) setBSsamples(samples);
}

/////////////////////

void DoublePassBlendingDeferredRenderer::setHemisphereSamplingMode(HemisphereSampling::KernelMode_t samplingMode) {
	bs_emisphereSamplingMode = samplingMode;
	
	 		
	switch (samplingMode) {
		case HemisphereSampling::HEMISPHERE_RANDOM: 				
//		case HemisphereSampling::HEMISPHERE_PYRAMID:
			setupEmisphereKernel_random(bs_samples, bs_emisphereAllPointsOnSurface, false);
			break;
		case HemisphereSampling::HEMISPHERE_RANDOM_DISTANCEFALLOFF: 				
//		case HemisphereSampling::HEMISPHERE_PYRAMID_DISTANCEFALLOFF:
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


/////////////////////
/*
void DoublePassBlendingDeferredRenderer::setRandomGenerationId(RandomGeneratorShaderSub::OptionValues id) {
	randomGenerationId = id;
	if (blendShader->isInitialized()) {
		blendShader->use();
		blendShader->setRandomGenerator(id);
	}
}
*/

void DoublePassBlendingDeferredRenderer::setSamplingPatternId(SamplingPatternShaderSub::OptionValues id) {
	samplingPatternId = id;	
	if (blendShader->isInitialized()) {
		blendShader->use();
		blendShader->setSamplingPattern(id);
	}	
}

void DoublePassBlendingDeferredRenderer::setAoComputationId(D2DaoShaderSub::OptionValues id) {
	aoComputationId = id;	
	if (blendShader->isInitialized()) {
		blendShader->use();
		blendShader->setAoCalculation(id);
	}
}

void DoublePassBlendingDeferredRenderer::setIlComputationId(D2DilShaderSub::OptionValues id) {
	ilComputationId = id;
	if (blendShader->isInitialized()) {
		blendShader->use();
		blendShader->setIlCalculation(id);
	}
}

void DoublePassBlendingDeferredRenderer::setAreaComputationId(AreaCalculatorShaderSub::OptionValues id) {
	areaComputationId = id;
	if (gpShader->isInitialized()) {
		gpShader->use();
		gpShader->setAreaCalculation(id);
	}
}


void DoublePassBlendingDeferredRenderer::setNormalMapping(bool enabled) {
	Renderer::setNormalMapping(enabled);
	gpShader->use();
	gpShader->setNormalMapping(enabled);
}

void DoublePassBlendingDeferredRenderer::setSpecularMapping(bool enabled) {
	Renderer::setSpecularMapping(enabled);
	gpShader->use();
	gpShader->setSpecularMapping(enabled);	
}

void DoublePassBlendingDeferredRenderer::setOpacityMapping(bool enabled) {
	Renderer::setOpacityMapping(enabled);
	gpShader->use();
	gpShader->setOpacityMapping(enabled);	
}

void DoublePassBlendingDeferredRenderer::setTextureMapping(bool enabled) {
	Renderer::setTextureMapping(enabled);
	gpShader->use();
	gpShader->setTextureMapping(enabled);
}

void DoublePassBlendingDeferredRenderer::setAoMultiplier(float m) {
	aoMultiplier = m;
	blendShader->use();
	blendShader->setAoMultiplier(m);
}
		
void DoublePassBlendingDeferredRenderer::setIlMultiplier(float m) {
	ilMultiplier = m;
	blendShader->use();
	blendShader->setIlMultiplier(m);
}
void DoublePassBlendingDeferredRenderer::setAreaMultiplier(float m) {
	areaMultiplier = m;
	blendShader->use();
	blendShader->setAreaMultiplier(m);
}

void DoublePassBlendingDeferredRenderer::setSolidAngleMultiplier(float m) {
	solidAngleMultiplier = m;
	blendShader->use();
	blendShader->setSolidAngleMultiplier(m);
}
///////////////////////////////////////////////////////////////////////////

/*
void printBufInfo() {
	printf("-------------------\n");
	GLint buf, s;
	glGetIntegerv(GL_SAMPLE_BUFFERS, &buf);	
	printf("number of sample buffers is %d\n", buf);
	glGetIntegerv(GL_SAMPLES, &s);
	printf("number of samples is %d\n", s);
	printf("-------------------\n");
}

*/