#include "gui.h"

#ifdef GUIENABLED_GUI_H

#include "../Scene.h"
#include "../Renderer.h"
#include "../Sandbox.h"

#include "GuiBarStatus.h"
#include "GuiBarSandbox.h"
#include "GuiBarAssetNodes.h"
#include "GuiBarCameraNodes.h"
#include "GuiBarLightNodes.h"
#include "GuiBarScene.h"
#include "GuiBarRenderer.h"
#include "GuiBarCameraPaths.h"

std::vector< std::shared_ptr<GuiBar> > Gui::guiBars;

int Gui::visibilityLevel = 0;
const int Gui::NUM_VISIBILITY_LEVELS = 4;
const float Gui::DEFAULT_UPDATE_INTERVAL = 0.1f;	

std::shared_ptr<GuiBarSandbox>     Gui::sandboxBar;
std::shared_ptr<GuiBarStatus>      Gui::statusBar;
std::shared_ptr<GuiBarScene>       Gui::sceneBar;
std::shared_ptr<GuiBarAssetNodes>  Gui::assetNodesBar;
std::shared_ptr<GuiBarCameraNodes> Gui::cameraNodesBar;
std::shared_ptr<GuiBarLightNodes>  Gui::lightNodesBar;
std::shared_ptr<GuiBarRenderer>    Gui::rendererBar;
std::shared_ptr<GuiBarCameraPaths> Gui::campathsBar;

void Gui::setStatus(const std::string& ns) {
	statusBar.get()->setStatusString(ns);
	statusBar.get()->init();
}

void Gui::init() {
	TwInit(TW_OPENGL_CORE, NULL);
	TwDefine(" GLOBAL iconalign=horizontal ");
	TwDefine(" TW_HELP visible=false ");

	sandboxBar = std::shared_ptr<GuiBarSandbox>(new GuiBarSandbox());
	guiBars.push_back(sandboxBar);

	sceneBar = std::shared_ptr<GuiBarScene>(new GuiBarScene());
	guiBars.push_back(sceneBar);
	
	rendererBar = std::shared_ptr<GuiBarRenderer>(new GuiBarRenderer());
	guiBars.push_back(rendererBar);

	assetNodesBar = std::shared_ptr<GuiBarAssetNodes>(new GuiBarAssetNodes());
	guiBars.push_back(assetNodesBar);
	
	cameraNodesBar = std::shared_ptr<GuiBarCameraNodes>(new GuiBarCameraNodes());
	guiBars.push_back(cameraNodesBar);
	
	lightNodesBar = std::shared_ptr<GuiBarLightNodes>(new GuiBarLightNodes());
	guiBars.push_back(lightNodesBar);
	
	statusBar = std::shared_ptr<GuiBarStatus>(new GuiBarStatus());
	guiBars.push_back(statusBar);
	statusBar.get()->setStatusString("YARS initialized");
	
	campathsBar = std::shared_ptr<GuiBarCameraPaths>(new GuiBarCameraPaths());
	guiBars.push_back(campathsBar);
	
	for ( auto bar : guiBars ) {

		bar.get()->setUpdateInterval(DEFAULT_UPDATE_INTERVAL);
		bar.get()->setAlpha(200);
		bar.get()->setColor(20, 150, 255);
	}	
	
	show();	
}

void Gui::update() {
	sandboxBar.get()->init();
	statusBar.get()->init();		
}

void Gui::updateSceneBars() {
	sandboxBar->init();
	sceneBar->init();
	assetNodesBar->init();
	cameraNodesBar->init();
	lightNodesBar->init();	
	campathsBar->init();
}

void Gui::updateRendererBars() {
	rendererBar->init();
}

void Gui::setBarVisibilityLevel(int lev) {
	printf("setBarVisibilityLevel %d\n", lev);
	visibilityLevel = lev;	
	for ( auto bar : guiBars ) {
		bar.get()->setVisible( bar.get()->getVisibilityLevel() < visibilityLevel );
	}		
}

void Gui::changeBarVisibilityLevel() {	
	setBarVisibilityLevel((visibilityLevel + 1) % NUM_VISIBILITY_LEVELS);
}

void Gui::show() {
	Gui::setBarVisibilityLevel(NUM_VISIBILITY_LEVELS);
}
void Gui::hide() {
	Gui::setBarVisibilityLevel(0);	
}



int Gui::windowSize(int width, int height) {
	const int MARGIN_TOP    = 0;
	const int MARGIN_BOTTOM = 40;
	
	const int DEFAULT_BAR_WIDTH = 230;
		
	int halfWinHeight     = (RenderWindow::getHeight() - (MARGIN_TOP + MARGIN_BOTTOM))  / 2 ;
	int oneThirdWinHeight = (RenderWindow::getHeight() - (MARGIN_TOP + MARGIN_BOTTOM)) / 3  ;

	sandboxBar.get()->setSize(DEFAULT_BAR_WIDTH, halfWinHeight);
	sandboxBar.get()->setPosition(0, MARGIN_TOP);

	rendererBar.get()->setSize(DEFAULT_BAR_WIDTH, halfWinHeight);
	rendererBar.get()->setPosition(0, halfWinHeight+MARGIN_TOP);

	sceneBar.get()->setSize(DEFAULT_BAR_WIDTH, RenderWindow::getHeight());
	sceneBar.get()->setPosition(RenderWindow::getWidth()-DEFAULT_BAR_WIDTH, MARGIN_TOP);

	
	assetNodesBar.get()->setSize(DEFAULT_BAR_WIDTH*2, oneThirdWinHeight);
	assetNodesBar.get()->setPosition(RenderWindow::getWidth() - DEFAULT_BAR_WIDTH * 3, MARGIN_TOP);

	cameraNodesBar.get()->setSize(DEFAULT_BAR_WIDTH, oneThirdWinHeight);
	cameraNodesBar.get()->setPosition(RenderWindow::getWidth() - DEFAULT_BAR_WIDTH * 2, MARGIN_TOP + oneThirdWinHeight);
	
	lightNodesBar.get()->setSize(DEFAULT_BAR_WIDTH, oneThirdWinHeight);
	lightNodesBar.get()->setPosition(RenderWindow::getWidth() - DEFAULT_BAR_WIDTH * 2, MARGIN_TOP + oneThirdWinHeight * 2);

	campathsBar.get()->setSize(DEFAULT_BAR_WIDTH, oneThirdWinHeight*2);
	campathsBar.get()->setPosition(RenderWindow::getWidth() - DEFAULT_BAR_WIDTH * 3, MARGIN_TOP + oneThirdWinHeight);

	
	statusBar.get()->setSize(RenderWindow::getWidth() - DEFAULT_BAR_WIDTH * 2, MARGIN_BOTTOM);
	statusBar.get()->setPosition(DEFAULT_BAR_WIDTH, RenderWindow::getHeight() - MARGIN_BOTTOM);
	statusBar.get()->setValuesWidth(RenderWindow::getWidth() - DEFAULT_BAR_WIDTH);

	return TwWindowSize(width, height);
}





#endif