#ifndef SANDBOX_H
#define SANDBOX_H

#include "AssetManager.h"
#include "SceneManager.h"
#include "TextureManager.h"

#include "Input.h"

#include "Profiler.h"


#include <vector>
#include <string>

//fwd dec
class Renderer;
class Scene;

class Sandbox {

public:
	static void init( int winWidth, int winHeight );
	static void destroy();
	static void update(float sec);
	static void render();
		
	static void          updateFPScount();
	inline static double getFPS() { return fps; }

	static void startProfiling(float duration) { profiler.reset(duration); }
	static void updateProfiling(float val)     { profiler.update(val); }
	static void stopProfiling()                { profiler.getResults(); }

	static void addRenderer(std::unique_ptr<Renderer>);
	static void setActiveRenderer(int id);
	static void setActiveRenderer(Renderer *ptr);
	static Renderer * const getActiveRenderer() { return currRenderer; }
	
	static void loadScene(const std::string& filename);
	static void addScene(std::unique_ptr<Scene> s);
	static void setActiveScene(int id);
	static void setActiveScene(Scene *s);
	static Scene * const getActiveScene() { return currScene; }
	static void saveCurrentSceneToFile(const std::string& filename);
	static void closeCurrentScene();

	static SceneManager* getSceneManager();

	static const std::vector<std::unique_ptr<Renderer>>& getRenderers() { return renderers; }


	static Asset* loadAsset(const std::string& filename);
	static AnimatedAsset* loadAnimatedAsset(const std::string& filenamePrefix, const std::string& filenameSuffix, const int frames);

	static void cb_windowResized( int width, int height );
	static void cb_key( int key, int action );
	static void cb_mouseButton( int btn, int action );
	static void cb_mousePos( int mouseX, int mouseY );
	static void cb_mouseWheel( int pos );
	static void cb_char( int c, int action );

	static void saveScreenshot(const std::string& filename, const std::string& ext, bool mightOverwrite = false);

	static const glm::vec2 getMousePos();

	static AssetManager assetManager;
	static SceneManager sceneManager;
	static TextureManager textureManager;
private:
	Sandbox();

	
	static std::vector<std::unique_ptr<Renderer>> renderers;

	static Renderer *currRenderer;
	static Scene    *currScene;

	static Input input;

	static Profiler profiler;

	static bool        screenshotMightOverwrite;
	static bool        screenshotScheduled;
	static std::string screenshotFilename;
	static std::string screenshotExt;
	static void actuallySaveScreenshot();

	static float fps;

};

#endif