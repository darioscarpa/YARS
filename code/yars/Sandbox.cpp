#include "Sandbox.h"

#include "Scene.h"
#include "Renderer.h"

#include "gui/gui.h"
#include "SceneSerialization.h"

#include "RenderWindow.h"

// DevIL
#include <IL/il.h>
#include <IL/ilut.h>

#include <iostream>
#include <streambuf>
#include <sstream>
#include <iomanip>

Profiler Sandbox::profiler;
AssetManager Sandbox::assetManager;
TextureManager Sandbox::textureManager;
SceneManager Sandbox::sceneManager;

std::vector<std::unique_ptr<Renderer>> Sandbox::renderers;

Renderer *Sandbox::currRenderer = nullptr;
Scene    *Sandbox::currScene    = nullptr;

Input Sandbox::input;

bool        Sandbox::screenshotMightOverwrite;
bool        Sandbox::screenshotScheduled; 
std::string Sandbox::screenshotFilename;
std::string Sandbox::screenshotExt;

float Sandbox::fps = 0.0;

const glm::vec2 Sandbox::getMousePos() {
	return glm::vec2(input.getMouseX(), RenderWindow::getHeight() - input.getMouseY()); 
};

void Sandbox::init( int winWidth, int winHeight ) {
	screenshotScheduled = false;
	Gui::init();	
}

void Sandbox::destroy() {
	Gui::destroy();	
}

void Sandbox::update(float sec) {
	currScene->update(&input, sec);	
}

void Sandbox::render() {
	currRenderer->render(*currScene);
	Gui::render();
	updateFPScount();
	//maybeSaveScreenshot();

	//GLvoid *imageData =  static_cast<GLvoid*>(malloc(winWidth()*winHeight()*32));
	//glReadPixels(0, 0, winWidth(), winHeight(), GL_RGBA, GL_UNSIGNED_BYTE, imageData);
	if (screenshotScheduled) {
		actuallySaveScreenshot();
		screenshotScheduled = false;
	}
}

void Sandbox::updateFPScount() {
	// local static values initialized on the first call
	static const double fpsUpdateInterval = 1.0; // in seconds
	static double t0         = RenderWindow::getTime();
	static int    frameCount = 0;             
 
	// time in secs since the program started
	double currentTime = RenderWindow::getTime();
  
	if ( (currentTime - t0) > fpsUpdateInterval ) {
		// calc fps (#frames/time since last calculation)
		fps = (double)frameCount / (currentTime - t0);	
 
		frameCount = 0;
		t0 = RenderWindow::getTime();
	} else { 
		frameCount++;
	}		
}

void Sandbox::addRenderer(std::unique_ptr<Renderer> r) {	
	renderers.push_back(std::move(r));
	if (renderers.size() == 1) setActiveRenderer(0);
	Gui::update();
}

void Sandbox::setActiveRenderer(Renderer *ptr) {
	//int pos = std::find(renderers.begin(), renderers.end(), ptr) - renderers.begin();

	int pos = std::find_if(renderers.begin(), renderers.end(), [&](std::unique_ptr<Renderer>& p) { return p.get() == ptr; })
		      - renderers.begin();

	setActiveRenderer(pos);
	Gui::update();
}
void Sandbox::setActiveRenderer(int id) {
	if (currRenderer != nullptr) {
		// term prev renderer
		currRenderer->destroy();
	}
	currRenderer = renderers.at(id).get();
	currRenderer->init(RenderWindow::getWidth(), RenderWindow::getHeight());
	Gui::update();
	Gui::updateRendererBars();
}

void Sandbox::loadScene(const std::string& filename) {
	std::unique_ptr<Scene> sp = SceneSerialization::load(filename); // todo return unique_ptr
	if (sp) {
		addScene(std::move(sp)); 
	} else {
		printf("error loading scene: %s\n", filename.c_str());
	}	
}

void Sandbox::saveCurrentSceneToFile(const std::string& filename) {	
	SceneSerialization::save(filename, *currScene);
}

void Sandbox::addScene(std::unique_ptr<Scene> s) {
	sceneManager.addScene(std::move(s));
	if (sceneManager.numScenes() == 1) setActiveScene(0);
	Gui::update();
}


void Sandbox::closeCurrentScene() {	
	if (sceneManager.numScenes() == 1) return;
	Scene *curr = getActiveScene();
	sceneManager.remScene(curr);
	setActiveScene(0);	
}

void Sandbox::setActiveScene(int id) {
	currScene = sceneManager.getScenePtr(id);
	currScene->onActivate();
	Gui::updateSceneBars();
}

void Sandbox::setActiveScene(Scene *s) {
	int id = sceneManager.getSceneId(s);
	setActiveScene(id);	
}

SceneManager* Sandbox::getSceneManager()  {
	return &sceneManager;	
}

Asset *Sandbox::loadAsset(const std::string& filename) {
	/*Asset *ret = nullptr;
	try {
		ret = assetManager.loadAsset(filename);
	} catch (std::exception& e) {
		std::cerr << e.what();
	}
	return ret;
	*/
	return assetManager.loadAsset(filename);	
}
AnimatedAsset *Sandbox::loadAnimatedAsset(const std::string& filenamePrefix, const std::string& filenameSuffix, const int frames) {
	return assetManager.loadAnimatedAsset(filenamePrefix, filenameSuffix, frames);
}

// GLFW CALLBACKS
void Sandbox::cb_windowResized(int width, int height) {
	RenderWindow::setSize(width, height);		
	Gui::windowSize(width, height);
	currScene->reshape(width, height);
	currRenderer->reshape(width, height);	
}
	
int lastkey = 0;
void Sandbox::cb_key(int key, int action) {
	//printf("--\nsandbox::cb_key\n");
	//if ( Gui::eventChar(key, action) ) return;
	//if ( Gui::keyTest(key, action) ) return;
	/*if ( Gui::eventChar(key, action) ) {
		lastkey = key;
		return; // text input into gui
	}
	if (lastkey == key) {
		lastkey = 0;
		return;
	}*/
	if (  !Gui::keyTest(key, action)  && !Gui::eventKey(key, action) ) {
		//printf("gui didn't handle keypress/release\n");
		if (action == RenderWindow::keyAction::KEY_PRESS) {
			if (key >= '1' && key <='9') {
				int sceneid = key - '1';
				if (sceneid < sceneManager.numScenes())
					setActiveScene(sceneid);
			} else if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F8) {
				int rendererid = key - GLFW_KEY_F1;
				if (rendererid < renderers.size())
					setActiveRenderer(rendererid);
			} else if (key == GLFW_KEY_F9) {
				saveScreenshot("screenshot", "png");				
				//screenshotScheduled = true;
			} else {

				switch(key) {
					case GLFW_KEY_TAB:
						Gui::isVisible() ? Gui::hide() : Gui::show();
						break;

					case 'M':
						currRenderer->reinit();
						break;											

					case 92: // \ 
						//Gui::isSomeVisible() ? Gui::hideSome() : Gui::showSome();						
						Gui::changeBarVisibilityLevel();
						break;

					default:
						input.keyCB(key, action);
				}			
			}
		} else {
			input.keyCB(key, action);
		}       
    }
}

void Sandbox::cb_mouseButton( int button, int action ) {
	if( !Gui::eventMouseButton(button, action) )  {  
		input.mouseButtonCB(button, action);
    }
}
void Sandbox::cb_mousePos( int mouseX, int mouseY ) {
	if( !Gui::eventMousePos(mouseX, mouseY) )  {   
		input.mousePosCB(mouseX, mouseY);
    }
}
void Sandbox::cb_mouseWheel( int pos ) {
	if( !Gui::eventMouseWheel(pos) ) {
		input.mouseWheelCB(pos);
    }
}
void Sandbox::cb_char( int c, int action ) {
	//printf("sandbox::cb_char\n");	
    if( !Gui::eventChar(c, action) ) { 
		// nothing for now
    }
}


void Sandbox::actuallySaveScreenshot() {
	static const std::string SCREENSHOT_DIR = "_screenshots/";
	
	std::string filePath;

	if (!screenshotMightOverwrite) {
		int num = 0;	
		do {
			std::ostringstream ss;
			ss << SCREENSHOT_DIR << screenshotFilename << std::setw( 3 ) << std::setfill( '0' ) << num << "." << screenshotExt;

			filePath = ss.str();
			std::ifstream ifile(filePath);
			if (!ifile) break;
			num++;
		} while (true);
	} else {
		filePath = screenshotFilename + "." + screenshotExt;
	}


	const char* fn = filePath.c_str();
	const wchar_t* screenshotFile = reinterpret_cast<const wchar_t*>(fn);


	ILuint imageID = ilGenImage();
    ilBindImage(imageID);
    ilutGLScreen();
    ilEnable(IL_FILE_OVERWRITE);

	if (!ilSaveImage(screenshotFile)) {
	//if (!ilSave(IL_PNG, screenshotFile)) {
	//if (!ilSaveImage(L"screenshot.bmp")) { //screenshotFile)) {
			printf("Couldn't save screenshot Image: %s\n", screenshotFile);
			ILenum Error;
			while ((Error = ilGetError()) != IL_NO_ERROR) { 
				//printf("%d: %s/n", Error, iluErrorString(Error)); 
				printf("%d: \n", Error); 
			}
	}
    ilDeleteImage(imageID);
    printf("Screenshot saved to: %s\n", screenshotFile);
}
void Sandbox::saveScreenshot(const std::string& filename, const std::string& ext, bool mightOverwrite) {
	screenshotMightOverwrite = mightOverwrite;
	screenshotScheduled = true;
	screenshotFilename = filename;
	screenshotExt      = ext;
}