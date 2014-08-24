#ifndef GUI_H
#define GUI_H

class Scene;
class Renderer;

//flag to completely strip out GUI (for profiling and debugging)
#define GUIENABLED_GUI_H 1

#ifdef GUIENABLED_GUI_H

// Ant Tweak Bar
#include <AntTweakBar.h>

// STL
#include <vector>
#include <memory>
#include <string>

class GuiBar;
class GuiBarAssetNodes;
class GuiBarCameraNodes;
class GuiBarLightNodes;
class GuiBarSandbox;
class GuiBarScene;
class GuiBarStatus;
class GuiBarRenderer;
class GuiBarCameraPaths;

namespace GuiCB {

	template<class T, class Towner, void (Towner::*method)(T)>
	void TW_CALL setter(const void *value, void *clientData) {	
		const T v = *static_cast<const T*>(value);
		(*static_cast<Towner*>(clientData).*method)(v);
	}

	template <class T, class Towner, T (Towner::*method)(void) const>	
	void TW_CALL getter(void *value, void *clientData) {
		T *dest = static_cast<T*>(value);
		*dest = (*( static_cast<Towner*>(clientData) ).*method)();
	}

	/*
	template<class glm::vec3, class Towner, void (Towner::*method)(glm::vec3)>
	void TW_CALL setter(const void *value, void *clientData) {	
		const float *farray = static_cast<const float*>(value);
		(*static_cast<Towner*>(clientData).*method)(glm::vec3(farray[0], farray[1], farray[2]));		
	}

	template <class glm::vec3, class Towner, glm::vec3 (Towner::*method)(void) const>	
	void TW_CALL getter(void *value, void *clientData) {
		glm::vec3 v = (static_cast<Towner*>(clientData).*method)();
		float *dest = static_cast<float*>(value);
		dest[0] = v.r;
		dest[1] = v.g;
		dest[2] = v.b;	
	}
	*/

}

class Gui {
public:	
	
	static void init();
	static void show();
	static void hide();
	static void changeBarVisibilityLevel();

	static bool isVisible() {
		return visibilityLevel > 0;
	}
	static int windowSize(int width, int height);

	inline static void destroy() {
		TwTerminate();
	}

	inline static void render() {
		TwDraw();
	}	

	static void setStatus(const std::string& s);
	static void update();
	static void updateSceneBars();
	static void updateRendererBars();	

	//static void setStatus(const std::string& ns);

	inline static int eventMouseButton(int btn, int action) {
		return TwEventMouseButtonGLFW(btn, action);		
	}

	inline static int eventMousePos( int mouseX, int mouseY ) {
		return TwEventMousePosGLFW(mouseX, mouseY);
	}

	inline static int eventMouseWheel( int pos ) {
		return TwEventMouseWheelGLFW(pos);
    }

	inline static int eventKey( int key, int action ) {
		return TwEventKeyGLFW(key, action);
		/*bool ret = TwEventKeyGLFW(glfwKey, glfwAction);
		printf("key CB %c handled by GUI: %d\n", glfwKey, ret);
		return ret;*/
    }

	inline static int eventChar(int c, int action ) {
		return TwEventCharGLFW(c, action);
		/*bool ret = TwEventCharGLFW(glfwChar, glfwAction);
		printf("charCB %c handled by GUI: %d\n", glfwChar, ret);
		return ret;*/
    }	
	
	inline static int keyTest(int c, int action ) {
		return TwKeyTest(c, action);
	}

	template <class T, class Towner>
	inline static void setter_callback(const void *value, void *clientData, void (Towner::*m)(T)) {
		const T v = *static_cast<const T*>(value);
		(*static_cast<Towner*>(clientData).*m)(v);
	}

	template <class T, class Towner>	
	inline static void getter_callback(void *value, void *clientData, T (Towner::*m)(void) const) {
		T *dest = static_cast<T*>(value);
		*dest = (*( static_cast<Towner*>(clientData) ).*m)();
	}
private:
	static const float Gui::DEFAULT_UPDATE_INTERVAL;
	static const int   Gui::NUM_VISIBILITY_LEVELS;
		
	static std::vector<std::shared_ptr<GuiBar>> guiBars;
	
	static std::shared_ptr<GuiBarStatus>      statusBar;
	static std::shared_ptr<GuiBarSandbox>     sandboxBar;
	static std::shared_ptr<GuiBarScene>		  sceneBar;
	static std::shared_ptr<GuiBarAssetNodes>  assetNodesBar;
	static std::shared_ptr<GuiBarCameraNodes> cameraNodesBar;
	static std::shared_ptr<GuiBarLightNodes>  lightNodesBar;
	static std::shared_ptr<GuiBarRenderer>    rendererBar;
	static std::shared_ptr<GuiBarCameraPaths> campathsBar;

	
	static int visibilityLevel;
	static void setBarVisibilityLevel(int lev);
	
	
};

#else

class Gui {
public:		
	inline static void setupSceneBars(Scene*) {}
	inline static void setupRendererBars(Renderer*) {}

	inline static void update() {};
	inline static void updateSceneBars() {};
	inline static void updateRendererBars() {};
	inline static void init() {};
	inline static void show() {};
	inline static void hide() {};
	inline static void changeBarVisibilityLevel() {};
	inline static bool isVisible() { return false; }	
	
	inline static void destroy() {};
	inline static void render() {};

	
	inline static int windowSize(int width, int height)      { return false; };
	inline static int eventMouseButton(int btn, int action)  { return false; };
	inline static int eventMousePos( int mouseX, int mouseY ){ return false; };
	inline static int eventMouseWheel( int pos )             { return false; };	
	inline static int eventKey( int key, int action )        { return false; };	
	inline static int eventChar(int c, int action )          { return false; };	
	inline static int keyTest(int c, int action )            { return false; };
			
	

};
#endif

#endif