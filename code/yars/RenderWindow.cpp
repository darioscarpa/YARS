#include "RenderWindow.h"

int   RenderWindow::width;
int   RenderWindow::height;
float RenderWindow::aspectRatio;

bool  RenderWindow::vsync;
bool  RenderWindow::fullscreen;

void RenderWindow::setCallbacks() {

	glfwEnable(GLFW_STICKY_KEYS);
	//glfwDisable( GLFW_SYSTEM_KEYS );

	// glfw callbacks ///////////////////////////////////////////////////	
	glfwSetKeyCallback(keyCB);
	glfwSetCharCallback(charCB);
	glfwSetMousePosCallback(mousePosCB);
	glfwSetMouseButtonCallback(mouseButtonCB);
	glfwSetMouseWheelCallback(mouseWheelCB);

	glfwSetWindowSizeCallback(windowSizeCB);
}

void RenderWindow::create(int desiredWidth, int desiredHeight,
	bool fullscreen, int samples) {
	// multisampling 
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, samples);

	// set OpenGL version/profile	
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, OPENGL_VERSION_MINOR);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// debugging?
#if _DEBUG
	glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	// open window/create OpenGL context
	GLFWvidmode videomode;
	glfwGetDesktopMode(&videomode);

	int win_width, win_height, win_mode;
	if (fullscreen) {
		win_width = videomode.Width;
		win_height = videomode.Height;
		win_mode = GLFW_FULLSCREEN;
	}
	else {
		win_width = std::min(desiredWidth, videomode.Width);
		win_height = std::min(desiredHeight, videomode.Height);
		win_mode = GLFW_WINDOW;
	}
	const int depth_bits = 32;
	const int stencil_bits = 32;
	if (!glfwOpenWindow(win_width, win_height,
		0, 0, 0, 0,      // r, g, b, a - color & alpha buffer bit depth
		depth_bits,   // depth buffer bit depth
		stencil_bits, // stencil buffer bit depth
		win_mode      // GLFW_WINDOW | GLFW_FULLSCREEN
		)) {
		
		destroy();

		std::stringstream msg;
		msg << "Failed to open GLFW window for OpenGL " << OPENGL_VERSION_MAJOR << "." << OPENGL_VERSION_MINOR << std::endl;
		throw std::exception(msg.str().c_str());
	}
}