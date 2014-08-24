#include "Sandbox.h"

#include "renderers/deferred/DeferredRenderer.h"
#include "renderers/forward/ForwardRenderer.h"
#include "renderers/deferred/DoublePassBlendingDeferredRenderer.h"
#include "renderers/def_saao/SaAoRenderer.h"
#include "renderers/def_saaoil/SaAoIlRenderer.h"

#include "scenes/SceneSponza.h"
#include "scenes/SceneRubik.h"

// OPENGL
#include "util/inc_opengl.h"

// GLM
#include "util/inc_math.h"

// I/O
#include "util/io_utils.h"

// GLFW wrapper
#include "RenderWindow.h"

// STL
#include <iostream>

using std::cout;
using std::endl;
void APIENTRY debugCB(GLenum source,
                      GLenum type,
                      GLuint id,
                      GLenum severity,
                      GLsizei length,
                      const GLchar* message,
				      void* userParam) {
 
    cout << "---------------------opengl-callback-start------------" << endl;
    cout << "message: "<< message << endl;
    cout << "type: ";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        cout << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        cout << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        cout << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        cout << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        cout << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        cout << "OTHER";
        break;
    }
    cout << endl;
 
    cout << "id: " << id << endl;
	cout << "severity: ";
    switch (severity){
    case GL_DEBUG_SEVERITY_LOW:
        cout << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        cout << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        cout << "HIGH";
        break;
    }
    cout << endl;
    cout << "---------------------opengl-callback-end--------------" << endl;
	//system("pause");
	//Sandbox::getActiveRenderer()->reloadShaders();
}

int main( void ) {
	
	const int DEFAULT_WIN_WIDTH  = 1280;
	const int DEFAULT_WIN_HEIGHT =  720;

	const int OPENGL_VERSION_MAJOR = 4;
	const int OPENGL_VERSION_MINOR = 2;

	const int DEFAULT_MULTISAMPLING_SAMPLES = 16;

	//TODO get by cmdline params
	bool multisampling = false;
	int  multisampling_samples = DEFAULT_MULTISAMPLING_SAMPLES;	
	bool fullscreen = false;
	int  win_width  = DEFAULT_WIN_WIDTH;
	int  win_height = DEFAULT_WIN_HEIGHT;
		

	// create window     //////////////////////////////////////////////////	
	try {
		RenderWindow::init();	
		multisampling_samples = multisampling ? multisampling_samples : 0;
		RenderWindow::create(win_width, win_height, fullscreen, multisampling_samples);
		RenderWindow::setVsync(false);
	}
	catch (std::exception& e) {
		std::cerr << e.what();
		IoUtils::blockUntilNewline();
		return -1;
	}
	
	// init GLEW       //////////////////////////////////////////////////	
	glewExperimental = true; // needed for core profile
	if ( glewInit() != GLEW_OK ) {
		std::cerr << "Failed to initialize GLEW\n";
		IoUtils::blockUntilNewline();
		return -1;
	}
	
	/////////////////////////////////////////////////////////////////////	

	Sandbox::init(win_width, win_height);
	
	/////////////////////////////////////////////////////////////////////	
	// init renderers
	Sandbox::addRenderer(std::unique_ptr<ForwardRenderer>(new ForwardRenderer()));
	Sandbox::addRenderer(std::unique_ptr<DoublePassBlendingDeferredRenderer>(new DoublePassBlendingDeferredRenderer()));
	//Sandbox::addRenderer(std::unique_ptr<DeferredRenderer>(new DeferredRenderer()));
	Sandbox::addRenderer(std::unique_ptr<SaAoRenderer>(new SaAoRenderer()));
	Sandbox::addRenderer(std::unique_ptr<SaAoIlRenderer>(new SaAoIlRenderer()));

	/////////////////////////////////////////////////////////////////////	
	// init scenes

	// example: init by json file
	// Sandbox::loadScene("sponza.json");
	// Sandbox::loadScene("sibenik.json");

	// example: init by code (custom logic overrides etc)
	// SceneSponza scenespo;
	// scenespo.setup();
	// Sandbox::addScene(&scenespo);
	
	// SceneRubik sceneRub;
	// sceneRub.setup();
	// Sandbox::addScene(&sceneRub);

	// default empty scene
	Sandbox::addScene(Scene::getEmptyScene());
		
	Sandbox::addScene(std::make_unique<SceneRubik>());
	/////////////////////////////////////////////////////////////////////	
	// init input/window callbacks
	RenderWindow::setCallbacks();
	/////////////////////////////////////////////////////////////////////	

	// debug callbacks //////////////////////////////////////////////////
	//glDebugMessageCallback(debugCB, nullptr);
#if _DEBUG
    if(glDebugMessageCallback){
        cout << "Registering OpenGL debug callback " << endl;
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debugCB, nullptr);
        GLuint unusedIds = 0;
        glDebugMessageControl(GL_DONT_CARE,
            GL_DONT_CARE,
            GL_DONT_CARE,
            0,
            &unusedIds,
            true);
    } else {
        cout << "glDebugMessageCallback not available" << endl;
	}
#endif
		
	// rendering loop   /////////////////////////////////////////////////	
	double lastTime = RenderWindow::getTime();
	while (!RenderWindow::shouldClose()) {	
		double currentTime = RenderWindow::getTime();
		double deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		
		Sandbox::update(deltaTime);
		Sandbox::render();
		
		RenderWindow::swapBuffers();
		RenderWindow::showFPSinTitleBar();		
	}	
	/////////////////////////////////////////////////////////////////////	

	// cleanup          /////////////////////////////////////////////////		
	Sandbox::destroy();
	RenderWindow::destroy();
	/////////////////////////////////////////////////////////////////////	
	return 0;
}