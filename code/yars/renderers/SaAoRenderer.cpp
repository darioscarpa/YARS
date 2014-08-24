#include "SaAoRenderer.h"

#include "HemisphereSampling.h"
#include "FullScreenQuad.h"

#include "../Sandbox.h"

#include "../util/vsGLInfoLib.h"

#include "../util/inc_math.h"


const float DEFAULT_ANGLEBIAS = 0.5f;
const float DEFAULT_MAXDIST   = 1.0f;
const float DEFAULT_SAMPLINGRADIUS = 0.5f;
const float DEFAULT_NUMSAMPLES = 16;


const char* SaAoRenderer::TRIANGLE_DIVS = "#TRIANGLE_DIVS#";

void SaAoRenderer::setTriangleSideDivs(int divs) {
	std::ostringstream v; v << divs;
	shadersCompileConstants[TRIANGLE_DIVS] = v.str();
	reloadShaders();
}
int SaAoRenderer::getTriangleSideDivs() const {
	auto it = shadersCompileConstants.find(TRIANGLE_DIVS);
	return std::stoi(it->second);
}


void SaAoRenderer::setUniformSamplingRadius(bool tf) {
	bs_emisphereAllPointsOnSurface = tf;
	//setupKernel(MAX_KERNEL_SIZE, tf, bs_octantInterleaving);
	setHemisphereSamplingMode(getHemisphereSamplingMode());
}

void SaAoRenderer::setupEmisphereKernel_random(int kernelSize, bool allPointsOnEmishphereSurface, bool distanceFalloff) {
	std::vector<glm::vec3> kernel;
	kernel.reserve(kernelSize);

	HemisphereSampling::getRandom(kernelSize, allPointsOnEmishphereSurface, distanceFalloff, kernel);

	processingPassShader->use();
	processingPassShader->setSampleKernel(kernel);
}

void SaAoRenderer::setupEmisphereKernel_triangleHierarchy(int kernelSize, bool allPointsOnEmishphereSurface, bool octantSampleInterleaving) {
	const int nsquare = kernelSize/4;
	const int n       = log(nsquare) / log(2);

	std::vector<glm::vec3> orderedSamplingVectors(kernelSize);
	std::vector<int>   skipTable(kernelSize);
	std::vector<float> solidAngleTable(kernelSize);
	//std::vector<float> vectorLenTable(kernelSize);
	std::vector<float> sampleWeightTable(kernelSize);

	HemisphereSampling::getTrianglesHierarchy(n, allPointsOnEmishphereSurface, octantSampleInterleaving,
		orderedSamplingVectors, skipTable, solidAngleTable, sampleWeightTable);

	processingPassShader->use();
	processingPassShader->setSampleKernel(orderedSamplingVectors);
	processingPassShader->setSampleKernelDetails(skipTable, solidAngleTable, sampleWeightTable);
	//printf("uKernelOffsets infodump - BEGIN **\n");
	//VSGLInfoLib::getUniformInfo(blendShader->getId(), "uKernelOffsets");
	//printf("uKernelOffsets infodump - END **\n");
	///////////////////////////////////////////////////////////////////////////
}

void SaAoRenderer::setScreenSpaceRayOrigin() {
	glm::vec2 newpos = Sandbox::getActiveScene()->getSpecialPoint();
	if (screenSpaceRayOrigin != newpos && processingPassShader->isInitialized()) {
		screenSpaceRayOrigin = newpos;
		processingPassShader->use();
		processingPassShader->setRayOriginPos(screenSpaceRayOrigin);
	}
}

void SaAoRenderer::setScreenSpaceEmisphereCenter() {
	glm::vec2 newpos = Sandbox::getActiveScene()->getEmisphereCenterPoint();
	if (screenSpaceEmisphereCenter != newpos && processingPassShader->isInitialized()) {
		screenSpaceEmisphereCenter = newpos;
		processingPassShader->use();
		processingPassShader->setMousePos(screenSpaceEmisphereCenter);
	}
}

SaAoRenderer::SaAoRenderer() : gBuffer(3, true), aoBuffer(1, false) {
	setLabel("SaSSAO renderer");

	// make cppcheck happy
	geometryPassShader   = nullptr;
	processingPassShader = nullptr;
	filteringPassShader  = nullptr;

	quad          = nullptr;

	showingGBuffer = false;
	showingAOBuffer  = false;

	aoSamplingRadius = aoAngleBias = aoMaxDistance = aoNumSamples = 0;
	bs_emisphereAllPointsOnSurface = true;

	alchemy_k = alchemy_ro = 1.0;
	alchemy_u = 0.0001;
}

void SaAoRenderer::initMrtBuffers(int awinW, int awinH) {
	gBuffer.init(winW, winH, 0);
	gBuffer.printFramebufferInfo(GL_DRAW_FRAMEBUFFER);

	aoBuffer.init(winW, winH, GBuffer::GBUFFER_TEXTURE_AVAILABLE, MrtBufferType::color_r8_b);
	aoBuffer.printFramebufferInfo(GL_DRAW_FRAMEBUFFER);
}

void SaAoRenderer::initShaderPrograms() {
	shadersCompileConstants["#TRIANGLE_DIVS#"] = "2";
	
	geometryPassShader   = new SaAoGeometryPassShader();
	processingPassShader = new SaAoProcessingPassShader(shadersCompileConstants);
	filteringPassShader  = new SaAoFilteringPassShader();
}

void SaAoRenderer::init(int awinW, int awinH) {
	printf("initializing %s\n", getLabel().c_str());

	winW = awinW;
	winH = awinH;

	initMrtBuffers(winW, winH);
	initShaderPrograms();
	ssd.init();

	// when depth test enabled, accept fragment if closer to the camera than the former one
	glDepthFunc(GL_LESS);

	//glEnable(GL_CULL_FACE);

	// disable blending
	glDisable(GL_BLEND);



	VSGLInfoLib::getCurrentTextureInfo();

	screenSpaceRayOrigin = glm::vec2(0,0);
	screenSpaceEmisphereCenter = glm::vec2(300,300);

	setTextureMapping(true);
	setNormalMapping(true);
	setSpecularMapping(true);
	setOpacityMapping(true);

	setShowingGBuffer(false);

/*	setShowingArea(false);
	setShowingSampling(false);
	setShowingSamplingDensity(false);
	setShowingZoomedArea(false);
*/

	setAoSamplingRadius(DEFAULT_SAMPLINGRADIUS);
	setAoAngleBias(DEFAULT_ANGLEBIAS);
	setAoMaxDistance(DEFAULT_MAXDIST);
	setAoNumSamples(DEFAULT_NUMSAMPLES);

	setAoMultiplier(1.0f);
	setAreaMultiplier(1.0f);
	setSolidAngleMultiplier(1.0f);

	setAlchemyK(1.0);
	setAlchemyRO(1.0);
	setAlchemyU(0.0001);

	setSamplingPatternId(SamplingPatternShaderSub::emisphere);
	setHemisphereSamplingMode(HemisphereSampling::HEMISPHERE_RANDOM);
	setUniformSamplingRadius(false);

	setTechnique(SaAoProcessingPassShader::SASSAO);

	
	setAoComputationId(D2DaoShaderSub::dssao);	
	setAreaComputationId(AreaCalculatorShaderSub::circumscribedCircleArea);


	setAlbedoEnabled(true);
	setAoEnabled(true);
	setAmbientEnabled(true);
	setDirectEnabled(true);

	setBlurEnabled(true);
	setBlurKdepth(10.0);
	setBlurKnormal(20.0);
	setBlurKernelSize(3);

	// init full screen quad
	quad = new FullScreenQuad();

	reshape(awinW, awinH);
}

void SaAoRenderer::destroy() {
	if (geometryPassShader) delete geometryPassShader;
	if (processingPassShader) delete processingPassShader;
	if (filteringPassShader) delete filteringPassShader;

	if (quad) delete quad;
	ssd.destroy();
}

void SaAoRenderer::reloadShaders() {
	if (geometryPassShader) delete geometryPassShader;
	if (processingPassShader) delete processingPassShader;
	if (filteringPassShader) delete filteringPassShader;

	/*
	geometryPassShader = new SaAoGeometryPassShader();
	processingPassShader = new SaAoProcessingPassShader(shadersCompileConstants);
	filteringPassShader = new SaAoFilteringPassShader();
	*/

	initShaderPrograms();

	VSGLInfoLib::getUniformsInfo(processingPassShader->getId());

	screenSpaceRayOrigin = glm::vec2(0,0);
	screenSpaceEmisphereCenter = glm::vec2(0,0);

	setTextureMapping(isTextureMappingEnabled());
	setNormalMapping(isNormalMappingEnabled());
	setSpecularMapping(isSpecularMappingEnabled());
	setOpacityMapping(isOpacityMappingEnabled());

	setShowingGBuffer(isShowingGBuffer());

	/*
	setShowingArea(isShowingArea());
	setShowingSampling(isShowingSampling());
	setShowingSamplingDensity(isShowingSamplingDensity());
	setShowingZoomedArea(isShowingZoomedArea());
	*/

	setAoSamplingRadius(getAoSamplingRadius());
	setAoAngleBias(getAoAngleBias());
	setAoMaxDistance(getAoMaxDistance());
	setAoNumSamples(getAoNumSamples());

	setSamplingPatternId(getSamplingPatternId());

	setAoMultiplier(getAoMultiplier());
	setAreaMultiplier(getAreaMultiplier());
	setSolidAngleMultiplier(getSolidAngleMultiplier());

	setHemisphereSamplingMode(getHemisphereSamplingMode());

	setAlbedoEnabled(isAlbedoEnabled());
	setAoEnabled(isAoEnabled());
	
	setAmbientEnabled(isAmbientEnabled());
	setDirectEnabled(isDirectEnabled());

	setAoComputationId(getAoComputationId());	
	setAreaComputationId(getAreaComputationId());

	setBlurEnabled(isBlurEnabled());
	setBlurKdepth(getBlurKdepth());
	setBlurKnormal(getBlurKnormal());
	setBlurKernelSize(getBlurKernelSize());

	setAlchemyK(getAlchemyK());
	setAlchemyRO(getAlchemyRO());
	setAlchemyU(getAlchemyU());

	setTechnique(getTechnique());

	reshape(winW, winH);
}

void SaAoRenderer::reinit() {

	bool _showingSampling  = showingSampling;
	bool _showingSamplingDensity = showingSamplingDensity;
	bool _showingArea      = showingArea;
	bool _showingGBuffer = showingGBuffer;
	bool _showingZoomedArea = showingZoomedArea;
	
	SamplingPatternShaderSub::OptionValues _samplingPatternId  = getSamplingPatternId();

	screenSpaceRayOrigin = glm::vec2(0,0);
	screenSpaceEmisphereCenter = glm::vec2(0,0);

	Renderer::reinit();

	setShowingGBuffer(_showingGBuffer);

	/*
	setShowingArea(_showingArea);
	setShowingSampling(_showingSampling);
	setShowingSamplingDensity(_showingSamplingDensity);
	setShowingZoomedArea(_showingZoomedArea);

	
	setScreenSpaceRayOrigin();
	setScreenSpaceEmisphereCenter();
	*/

	setAoMaxDistance(getAoMaxDistance());
	setAoAngleBias(getAoAngleBias());
	setAoNumSamples(getAoNumSamples());
	setAoMultiplier(getAoMultiplier());
	
	setAreaComputationId(getAreaComputationId());	
	setAreaMultiplier(getAreaMultiplier());

	setSamplingPatternId(_samplingPatternId);
	setHemisphereSamplingMode(getHemisphereSamplingMode());
	
	setBlurEnabled(isBlurEnabled());
	setBlurKdepth(getBlurKdepth());
	setBlurKnormal(getBlurKnormal());
	setBlurKernelSize(getBlurKernelSize());
};

void SaAoRenderer::setTechnique(SaAoProcessingPassShader::Technique_t t) {
	m_technique = t;
	processingPassShader->use();
	processingPassShader->setTechnique(t);	
	if (t==SaAoProcessingPassShader::SASSAO) {
		setSamplingPatternId(SamplingPatternShaderSub::emisphere);
	} else {
		setSamplingPatternId(SamplingPatternShaderSub::flat);
	}
}
////////////////////////////////////////////////////////////////////////////

void SaAoRenderer::setGbColorBufferType(MrtBufferType::OptionValues t) {
	gBuffer.setColorBufferType(t);
}

MrtBufferType::OptionValues SaAoRenderer::getGbColorBufferType() const {
	return gBuffer.getColorBufferType();
}

void SaAoRenderer::setGbDepthBufferType(MrtBufferType::OptionValues t) {
	gBuffer.setDepthBufferType(t);
}

MrtBufferType::OptionValues SaAoRenderer::getGbDepthBufferType() const {
	return gBuffer.getDepthBufferType();
}
void SaAoRenderer::setAoColorBufferType(MrtBufferType::OptionValues t) {
	aoBuffer.setColorBufferType(t);
}

MrtBufferType::OptionValues SaAoRenderer::getAoColorBufferType() const {
	return aoBuffer.getColorBufferType();
}


void SaAoRenderer::reshape(int awinW, int awinH) {
	Renderer::reshape(awinW, awinH);
	//if (awinW == 0) awinW++; 	if (awinH == 0) awinH++;

	gBuffer.resize(awinW, awinH);
	aoBuffer.resize(awinW, awinH);

	ssd.setScreenSize(awinW, awinH);
}

void SaAoRenderer::render(const Scene& scene) {
	//setScreenSpaceRayOrigin();
	//setScreenSpaceEmisphereCenter();
	///////////////////////////////////////////////////////////////////////////
	// 1st pass: write to Gbuffer

	gBuffer.bindForWriting();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

	geometryPassShader->use();
	renderSceneGeometry(scene);
	gBuffer.unbind();
	///////////////////////////////////////////////////////////////////////////

	if (showingGBuffer) {
		showGBuffer();
	} else {
		///////////////////////////////////////////////////////////////////////////
		// 2nd pass: write AoBuffer

		glDisable(GL_DEPTH_TEST);

		////////////////
		gBuffer.bindForReadingAsTextures();
		aoBuffer.bindForWriting();

		glClearColor(0.0, 0.0, 0.0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		processingPassShader->use();
		//processingPassShader->setRayOriginPos(Sandbox::getMousePos());

		quad->draw();
		aoBuffer.unbind();
		///////////////////////////////////////////////////////////////////////////

		if (showingAOBuffer) {
			showAOBuffer();
		} else {
			///////////////////////////////////////////////////////////////////////////
			// 3rd pass: write framebuffer

			// clear bg with scene background color
			const glm::vec3& bg = scene.getBackgroundColor();
			setBackgroundColor(bg);
			glClearColor(bg.r, bg.g, bg.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			aoBuffer.bindForReadingAsTextures();
			filteringPassShader->use();
			filteringPassShader->setAmbientLightLevel(scene.getAmbientLightLevel());
			quad->draw();
			///////////////////////////////////////////////////////////////////////////
		}
	}
}

void SaAoRenderer::setShowingGBuffer(bool enabled) {
	showingGBuffer = enabled;
	showingAOBuffer = false;
}

void SaAoRenderer::showGBuffer() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gBuffer.bindForReading();


	// 2*2 grid
	GLsizei offsetH = (GLsizei)(winH / 2.0f);
	GLsizei offsetW = (GLsizei)(winW / 2.0f);

	GLsizei offsetW2 = offsetW * 2;;


	//up left
	gBuffer.setReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
    glBlitFramebuffer(0, 0, winW, winH,
                    0, offsetH, offsetW, winH,
					GL_COLOR_BUFFER_BIT, GL_LINEAR);
	//down left
	gBuffer.setReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
    glBlitFramebuffer(0, 0, winW, winH,
                    0, 0, offsetW, offsetH,
					GL_COLOR_BUFFER_BIT, GL_LINEAR);

	//up mid
	gBuffer.setReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_SPECULAR);
    glBlitFramebuffer(0, 0, winW, winH,
                    offsetW, offsetH, offsetW2, winH,
					GL_COLOR_BUFFER_BIT, GL_LINEAR);

	//up mid
	//gbuf.SetReadDepthBuffer();
    glBlitFramebuffer(0, 0, winW, winH,
                    offsetW, offsetH, offsetW2, winH,
					GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void SaAoRenderer::setShowingAOBuffer(bool enabled) {
	showingAOBuffer = enabled;
	showingGBuffer = false;
}

void SaAoRenderer::showAOBuffer() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	aoBuffer.bindForReading();

	//full screen blit
	aoBuffer.setReadBuffer(AoBuffer::AOBUFFER_TEXTURE_TYPE_AO);
    glBlitFramebuffer(0, 0, winW, winH,
		0, 0, winW, winH,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);
}


/*
void SaAoRenderer::setShowingArea(bool enabled) {
	showingArea = enabled;
	processingPassShader->use();
	processingPassShader->setShowAreas(enabled);
	if (enabled) {
		setShowingGDLBuffer(false);
	}
}

void SaAoRenderer::setShowingSampling(bool enabled) {
	showingSampling = enabled;
	processingPassShader->use();
	processingPassShader->setShowSampling(enabled);
}

void SaAoRenderer::setShowingSamplingDensity(bool enabled) {
	showingSamplingDensity = enabled;
	processingPassShader->use();
	processingPassShader->setShowSamplingDensity(enabled);
}

void SaAoRenderer::setShowingZoomedArea(bool enabled) {
	showingZoomedArea = enabled;
	processingPassShader->use();
	processingPassShader->setShowZoomedArea(enabled);
}

*/

void SaAoRenderer::setAlbedoEnabled(bool enabled){
	albedoEnabled = enabled;
	filteringPassShader->use(); filteringPassShader->setAlbedoEnabled(enabled);
}
void SaAoRenderer::setAoEnabled(bool enabled){
	aoEnabled = enabled;
	filteringPassShader->use(); filteringPassShader->setAoEnabled(enabled);
}

void SaAoRenderer::setAmbientEnabled(bool enabled){
	ambientLightEnabled = enabled;
	filteringPassShader->use(); filteringPassShader->setAmbientEnabled(enabled);
}
void SaAoRenderer::setDirectEnabled(bool enabled){
	directLightEnabled = enabled;
	filteringPassShader->use(); filteringPassShader->setDirectEnabled(enabled);
}

/////////////////////
void SaAoRenderer::setAoSamplingRadius(float val) {
	aoSamplingRadius = val;

	processingPassShader->use();
	processingPassShader->setMaxSamplingRadius(val);	
}

void SaAoRenderer::setAoAngleBias(float val) {
	aoAngleBias = val;
	processingPassShader->use();
	processingPassShader->setDiskDisplacement(val);	
}

void SaAoRenderer::setAoMaxDistance(float val) {
	aoMaxDistance = val;
	processingPassShader->use();
	processingPassShader->setDistMax(val);
}

void SaAoRenderer::setAoNumSamples(int samples) {
	aoNumSamples = samples;

	processingPassShader->use();
	processingPassShader->setNumSamples(samples/8);
	setHemisphereSamplingMode(getHemisphereSamplingMode());	
}

/////////////////////

void SaAoRenderer::setAlchemyRO(float val) {
	alchemy_ro = val;
	processingPassShader->use();
	processingPassShader->setAlchemyRO(val);	
}
void SaAoRenderer::setAlchemyK(float val) {
	alchemy_k = val;
	processingPassShader->use();
	processingPassShader->setAlchemyK(val);	
}

void SaAoRenderer::setAlchemyU(float val) {
	alchemy_u = val;
	processingPassShader->use();
	processingPassShader->setAlchemyU(val);	
}

void SaAoRenderer::setHemisphereSamplingMode(HemisphereSampling::KernelMode_t samplingMode) {
	bs_emisphereSamplingMode = samplingMode;

	switch (samplingMode) {
		case HemisphereSampling::HEMISPHERE_RANDOM:
		//case HemisphereSampling::HEMISPHERE_PYRAMID:
			setupEmisphereKernel_random(aoNumSamples, bs_emisphereAllPointsOnSurface, false);
			break;
		case HemisphereSampling::HEMISPHERE_RANDOM_DISTANCEFALLOFF:
		//case HemisphereSampling::HEMISPHERE_PYRAMID_DISTANCEFALLOFF:
			setupEmisphereKernel_random(aoNumSamples, bs_emisphereAllPointsOnSurface, true);
			break;
		case HemisphereSampling::HEMISPHERE_TRIANGLEHIERARCHY:
			setupEmisphereKernel_triangleHierarchy(aoNumSamples == 64 ? 64 : 16, bs_emisphereAllPointsOnSurface, false);
			break;
		case HemisphereSampling::HEMISPHERE_TRIANGLEHIERARCHY_INTERLEAVED:
			int numSamples = aoNumSamples == 64 ? 64 : 16;
			setupEmisphereKernel_triangleHierarchy(numSamples == 64 ? 64 : 16, bs_emisphereAllPointsOnSurface, true);
			break;
	}
};
/////////////////////

void SaAoRenderer::setSamplingPatternId(SamplingPatternShaderSub::OptionValues id) {
	samplingPatternId = id;
	if (processingPassShader->isInitialized()) {
		processingPassShader->use();
		processingPassShader->setSamplingPattern(id);
	}
}

void SaAoRenderer::setAoComputationId(D2DaoShaderSub::OptionValues id) {
	aoComputationId = id;
	if (processingPassShader->isInitialized()) {
		processingPassShader->use();
		processingPassShader->setAoCalculation(id);
	}
}

void SaAoRenderer::setAreaComputationId(AreaCalculatorShaderSub::OptionValues id) {
	areaComputationId = id;
	if (geometryPassShader->isInitialized()) {
		geometryPassShader->use();
		geometryPassShader->setAreaCalculation(id);
	}
}

void SaAoRenderer::setNormalMapping(bool enabled) {
	Renderer::setNormalMapping(enabled);
	geometryPassShader->use();
	geometryPassShader->setNormalMapping(enabled);
}
void SaAoRenderer::setSpecularMapping(bool enabled) {
	Renderer::setSpecularMapping(enabled);
	geometryPassShader->use();
	geometryPassShader->setSpecularMapping(enabled);
}
void SaAoRenderer::setOpacityMapping(bool enabled) {
	Renderer::setOpacityMapping(enabled);
	geometryPassShader->use();
	geometryPassShader->setOpacityMapping(enabled);
}
void SaAoRenderer::setTextureMapping(bool enabled) {
	Renderer::setTextureMapping(enabled);
	geometryPassShader->use();
	geometryPassShader->setTextureMapping(enabled);
}
void SaAoRenderer::setAoMultiplier(float m) {
	aoMultiplier = m;
	processingPassShader->use();
	processingPassShader->setAoMultiplier(m);
}

void SaAoRenderer::setAreaMultiplier(float m) {
	areaMultiplier = m;
	processingPassShader->use();
	processingPassShader->setAreaMultiplier(m);
}
void SaAoRenderer::setSolidAngleMultiplier(float m) {
	solidAngleMultiplier = m;
	processingPassShader->use();
	processingPassShader->setSolidAngleMultiplier(m);
}
///////////////////////////////////////////////////////////////////////////

void SaAoRenderer::setBlurKernelSize(int s) {
	blurKernelSize = s;
	filteringPassShader->use();
	filteringPassShader->setKernelSize(s);
}

void SaAoRenderer::setBlurKnormal(float kn) {
	blurKnormal = kn;
	filteringPassShader->use();
	filteringPassShader->setKnormal(kn);
}

void SaAoRenderer::setBlurKdepth(float kd) {
	blurKdepth = kd;
	filteringPassShader->use();
	filteringPassShader->setKnormal(kd);
}

void SaAoRenderer::setBlurEnabled(bool enabled) {
	blurEnabled = enabled;
	filteringPassShader->use();
	filteringPassShader->setBlurEnabled(enabled);
}

