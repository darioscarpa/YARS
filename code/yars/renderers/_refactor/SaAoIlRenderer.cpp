#include "SaAoIlRenderer.h"

#include "HemisphereSampling.h"
#include "FullScreenQuad.h"

#include "../Sandbox.h"

#include "../util/vsGLInfoLib.h"

#include "../util/inc_math.h"

SaAoIlRenderer::SaAoIlRenderer()  {
	setLabel("SaSSAO renderer (IL)");

	ilEnabled = true;
	ilMultiplier = 1.0;
}

void SaAoIlRenderer::init(int awinW, int awinH) {
	printf("initializing %s\n", getLabel().c_str());
	

	gBuffer.init(winW, winH, 0);
	gBuffer.printFramebufferInfo(GL_DRAW_FRAMEBUFFER);

	aoBuffer.init(winW, winH, SaAoIlGBuffer::GBUFFER_TEXTURE_AVAILABLE, MrtBufferType::colora_rgba16_f);
	aoBuffer.printFramebufferInfo(GL_DRAW_FRAMEBUFFER);
	
	geometryPassShader   = new SaAoIlGeometryPassShader();
	processingPassShader = new SaAoIlProcessingPassShader(shadersCompileConstants);
	filteringPassShader  = new SaAoIlFilteringPassShader();
		
	setIlMultiplier(1.0f);	
	setIlComputationId(D2DilShaderSub::dssao);
		
	setIlEnabled(true);
	
	
}



void SaAoIlRenderer::reloadShaders() {
	if (geometryPassShader) delete geometryPassShader;
	if (processingPassShader) delete processingPassShader;
	if (filteringPassShader) delete filteringPassShader;

	geometryPassShader   = new SaAoIlGeometryPassShader();
	processingPassShader = new SaAoIlProcessingPassShader(shadersCompileConstants);
	filteringPassShader  = new SaAoIlFilteringPassShader();

	
	setIlEnabled(isIlEnabled());
	
}

void SaAoIlRenderer::reinit() {

	bool _showingSampling  = showingSampling;
	bool _showingSamplingDensity = showingSamplingDensity;
	bool _showingArea      = showingArea;
	bool _showingGBuffer = showingGBuffer;
	bool _showingZoomedArea = showingZoomedArea;
	int  _bsSamples        = bs_samples;

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
	*/

	setBSmaxSamplingRadius(getBSmaxSamplingRadius());
	setBSdiskDisplacement(getBSdiskDisplacement());
	setBSdistMax(getBSdistMax());
	setBSnumSamples(getBSnumSamples());

	setAoMultiplier(getAoMultiplier());
	setIlMultiplier(getIlMultiplier());
	setAreaMultiplier(getAreaMultiplier());

	setHemisphereSamplingMode(getHemisphereSamplingMode());

	setAreaComputationId(getAreaComputationId());
	setSamplingPatternId(_samplingPatternId);

	setScreenSpaceRayOrigin();
	setScreenSpaceEmisphereCenter();

	setBlurEnabled(isBlurEnabled());
	setBlurKdepth(getBlurKdepth());
	setBlurKnormal(getBlurKnormal());
	setBlurKernelSize(getBlurKernelSize());
};

////////////////////////////////////////////////////////////////////////////


void SaAoIlRenderer::render(const Scene& scene) {
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
		//ppShader->setRayOriginPos(Sandbox::getMousePos());

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

void SaAoIlRenderer::showGBuffer() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gBuffer.bindForReading();


	// 2*2 grid
	GLsizei offsetH = (GLsizei)(winH / 2.0f);
	GLsizei offsetW = (GLsizei)(winW / 2.0f);

	GLsizei offsetW2 = offsetW * 2;;


	//up left
	gBuffer.setReadBuffer(SaAoIlGBuffer::GBUFFER_TEXTURE_TYPE_DIRECTLIGHT);
    glBlitFramebuffer(0, 0, winW, winH,
                    0, offsetH, offsetW, winH,
					GL_COLOR_BUFFER_BIT, GL_LINEAR);
	//down left
	gBuffer.setReadBuffer(SaAoIlGBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
    glBlitFramebuffer(0, 0, winW, winH,
                    0, 0, offsetW, offsetH,
					GL_COLOR_BUFFER_BIT, GL_LINEAR);

	//up mid
	gBuffer.setReadBuffer(SaAoIlGBuffer::GBUFFER_TEXTURE_TYPE_ALBEDO);
    glBlitFramebuffer(0, 0, winW, winH,
                    offsetW, offsetH, offsetW2, winH,
					GL_COLOR_BUFFER_BIT, GL_LINEAR);

	//up mid
	//gbuf.SetReadDepthBuffer();
    glBlitFramebuffer(0, 0, winW, winH,
                    offsetW, offsetH, offsetW2, winH,
					GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}



void SaAoIlRenderer::setIlEnabled(bool enabled){
	ilEnabled = enabled;
	if (filteringPassShader->isInitialized()) {	
		filteringPassShader->use(); 
		static_cast<SaAoIlFilteringPassShader*>(filteringPassShader)->setIlEnabled(enabled);
	}
}


void SaAoIlRenderer::setIlComputationId(D2DilShaderSub::OptionValues id) {
	ilComputationId = id;
	if (processingPassShader->isInitialized()) {
		processingPassShader->use();
		static_cast<SaAoIlProcessingPassShader*>(processingPassShader)->setIlCalculation(id);
	}
}

void SaAoIlRenderer::setIlMultiplier(float m) {
	ilMultiplier = m;
	if (processingPassShader->isInitialized()) {
		processingPassShader->use();
		static_cast<SaAoIlProcessingPassShader*>(processingPassShader)->setIlMultiplier(m);
	}
}
