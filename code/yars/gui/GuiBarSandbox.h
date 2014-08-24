#ifndef GUIBARSANDBOX_H
#define GUIBARSANDBOX_H

#include "GuiBar.h"

#include "../Sandbox.h"
#include "../RenderWindow.h"
#include "../Scene.h"
#include "../Renderer.h"

#include <vector>
#include <memory>

#include <sstream>

namespace GuiBarSandbox_ns {	
	void TW_CALL getCB_winWidth(void *value, void *clientData) {
		int *dest = static_cast<int*>(value);
		*dest = RenderWindow::getWidth();	
	}
	void TW_CALL getCB_winHeight(void *value, void *clientData) {
		int *dest = static_cast<int*>(value);
		*dest = RenderWindow::getHeight();
	}

	void TW_CALL setCB_winWidth(const void *value, void *clientData) {
		const int *v = static_cast<const int*>(value);
		RenderWindow::setWidth(*v);		
	}
	void TW_CALL setCB_winHeight(const void *value, void *clientData) {
		const int *v = static_cast<const int*>(value);
		RenderWindow::setHeight(*v);
	}

	void TW_CALL getCB_winAspectRatio(void *value, void *clientData) {
		float *dest = static_cast<float*>(value);
		*dest = RenderWindow::getAspectRatio();	
	}
	void TW_CALL getCB_fps(void *value, void *clientData) {
		float *dest = static_cast<float*>(value);
		*dest = Sandbox::getFPS();
	}
	
	void TW_CALL setCB_bVsync(const void *value, void *clientData) {
		const bool *v = static_cast<const bool*>(value);
		RenderWindow::setVsync(*v);
	}
	void TW_CALL getCB_bVsync(void *value, void *clientData) {
		bool *dest = static_cast<bool*>(value);
		*dest = RenderWindow::isVsyncEnabled();		
	}

	void TW_CALL btnCB_saveScreenshot(void *clientData) {     
		Sandbox::saveScreenshot("screenshot", "png");
	}


	void TW_CALL btnCB_sceneActivate(void *clientData) {     
		Scene *s = static_cast<Scene *>(clientData);
		Sandbox::setActiveScene(s);
	}
	void TW_CALL btnCB_sceneLoad(void *clientData) { 
		std::string* s = static_cast<std::string*>(clientData);
		Sandbox::loadScene(*s);	
	}
	void TW_CALL btnCB_sceneSave(void *clientData) { 
		std::string* s = static_cast<std::string*>(clientData);
		Sandbox::saveCurrentSceneToFile(*s);		
	}
	void TW_CALL btnCB_sceneClose(void *clientData) { 
		Sandbox::closeCurrentScene();
	}
	void TW_CALL btnCB_sceneNew(void *clientData) { 
		std::unique_ptr<Scene> newscene = Scene::getEmptyScene();
		Sandbox::addScene(std::move(newscene));
		Sandbox::setActiveScene(newscene.get());
	}
	void TW_CALL btnCB_rendererActivate(void *clientData) {     
		Renderer *r = static_cast<Renderer *>(clientData);
		Sandbox::setActiveRenderer(r);
	}
	void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString) {
	  // Copy the content of souceString handled by the AntTweakBar library to destinationClientString handled by your application
	  destinationClientString = sourceLibraryString;
	}

}

using namespace GuiBarSandbox_ns;

class GuiBarSandbox : public GuiBar {
public:
	GuiBarSandbox() : GuiBar("Sandbox",0) {} ;
	virtual ~GuiBarSandbox() {};

	void init() {
		TwRemoveAllVars(m_barHandle);

		TwAddVarCB(m_barHandle, "FPS",    TW_TYPE_FLOAT, nullptr,  getCB_fps,  nullptr, "");
		TwAddVarCB(m_barHandle, "vsync",  TW_TYPE_BOOLCPP, setCB_bVsync,  getCB_bVsync,  nullptr, "");

		TwAddButton(m_barHandle, "screenshot",  btnCB_saveScreenshot, nullptr, "");
	
		TwAddVarCB(m_barHandle, "winWidth",    TW_TYPE_INT32, setCB_winWidth,  getCB_winWidth,       nullptr, " group='window' ");
		TwAddVarCB(m_barHandle, "winHeight",   TW_TYPE_INT32, setCB_winHeight,  getCB_winHeight,      nullptr, " group='window' ");	
		TwAddVarCB(m_barHandle, "aspectRatio", TW_TYPE_FLOAT, nullptr,  getCB_winAspectRatio, nullptr, " group='window' ");	
			
		SceneManager *sm = Sandbox::getSceneManager();
		for ( int i = 0 ; i < sm->numScenes() ; i++ ) {
			Scene *s = sm->getScenePtr(i);		
			std::string label = s->getLabel().c_str();
			if (s == Sandbox::getActiveScene()) label = "> "+ label;
			TwAddButton(m_barHandle, label.c_str(), btnCB_sceneActivate, s, " group='scenes' ");
		}	

		for ( auto& r  : Sandbox::getRenderers() ) {
			std::string label = r.get()->getLabel();
			if (r.get() == Sandbox::getActiveRenderer()) label = "> " + label;
			TwAddButton(m_barHandle, label.c_str(), btnCB_rendererActivate, r.get(), " group='renderers' ");
		}


		TwCopyStdStringToClientFunc(CopyStdStringToClient);

		TwAddVarRW(m_barHandle,  "filename", TW_TYPE_STDSTRING, &sceneFilename, " group='save/load scene' ");
		TwAddButton(m_barHandle, "load",     btnCB_sceneLoad,   &sceneFilename, " group='save/load scene' ");
		TwAddButton(m_barHandle, "save",     btnCB_sceneSave,   &sceneFilename, " group='save/load scene' ");
		TwAddButton(m_barHandle, "close",    btnCB_sceneClose,   nullptr,       " group='save/load scene' ");
		TwAddButton(m_barHandle, "close",    btnCB_sceneClose,   nullptr,       " group='save/load scene' ");
		TwAddButton(m_barHandle, "new",      btnCB_sceneNew,     nullptr,       " group='save/load scene' ");
		
	};
private:
	std::string sceneFilename;
};

#endif