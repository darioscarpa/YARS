#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include "Sandbox.h"
#include "util/inc_glfw.h"

// STL
#include <string>
#include <sstream>
#include <exception>
#include <algorithm>


// wrap GLFW - single window, primary monitor - no fancy usage

class RenderWindow {
public:


	enum mouseBtn {
		LEFT   = GLFW_MOUSE_BUTTON_LEFT,
		RIGHT  = GLFW_MOUSE_BUTTON_RIGHT,
		MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE		
	};
	enum mouseBtnAction {
		MBTN_RELEASE = GLFW_RELEASE,
		MBTN_PRESS   = GLFW_PRESS
	};
	enum keyAction {
		KEY_RELEASE = GLFW_RELEASE,
		KEY_PRESS   = GLFW_PRESS
	};


	inline static void init() {
		if (!glfwInit()) 	{
			std::stringstream msg;
			msg << "Failed to itialize GLFW";
			throw std::exception(msg.str().c_str());
		}
	}

	inline static void destroy() {
		glfwTerminate();
	}

	inline static void swapBuffers() {
		glfwSwapBuffers();		
	}

	inline static double getTime() {
		return glfwGetTime();
	}

	inline static void getCursorPos(int *xpos, int *ypos) {
		glfwGetMousePos(xpos, ypos);
	}

	inline static bool shouldClose() {
		return !( (glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS) &&  (glfwGetWindowParam(GLFW_OPENED)) ); 
	}

	inline static void showFPSinTitleBar() {
		static char titleString[] = "fps: 0000.00";
		sprintf(titleString + 5, "%4.2f", Sandbox::getFPS());
		glfwSetWindowTitle(titleString);
	}
	
	inline static void setSize(int w, int h) { 
		width = w; 
		height = h;
		aspectRatio = static_cast<float>(width) / height; 
		glfwSetWindowSize(w, h);
	}
	inline static void setWidth(int w)       { setSize(w, height); }
	inline static void setHeight(int h)      { setSize(width, h);  }

	inline static int   getWidth()       { return width; };
	inline static int   getHeight()      { return height; };
	inline static float getAspectRatio() { return aspectRatio; };
	
	inline static void setVsync(bool enabled)  { vsync = enabled; 	glfwSwapInterval(enabled ? 1 : 0); }
	inline static bool isVsyncEnabled() { return vsync; };
	

	static void setCallbacks();
	static void create(int desiredWidth, int desiredHeight, bool fullscreen, int samples); 

	static const int OPENGL_VERSION_MAJOR = 4;
	static const int OPENGL_VERSION_MINOR = 2;
private:
	
	static int   width;
	static int   height;
	static float aspectRatio;

	static bool  fullscreen;
	static bool  vsync;

	/////////////////
	// GLFW 2.7 callbacks
	/////////////////
	static void GLFWCALL windowSizeCB(int width, int height) { Sandbox::cb_windowResized(width, height); }
	static void GLFWCALL mouseButtonCB(int btn, int action)  { Sandbox::cb_mouseButton(btn, action); }
	static void GLFWCALL mousePosCB(int mouseX, int mouseY)  { Sandbox::cb_mousePos(mouseX, mouseY); }
	static void GLFWCALL mouseWheelCB(int pos)               { Sandbox::cb_mouseWheel(pos); }
	static void GLFWCALL charCB(int c, int action)           { Sandbox::cb_char(c, action); }
	static void GLFWCALL keyCB(int key, int action)          { Sandbox::cb_key(key, action); }
	/////////////////
};

/////////////////
// GLFW 3 (work in progress)
/////////////////
/*
void windowSizeCB(GLFWwindow *w, int width, int height)     { Sandbox::cb_windowResized(width, height); }
void mouseButtonCB(GLFWwindow *w, int btn, int action)      { Sandbox::cb_mouseButton(btn, action); }
void mousePosCB(GLFWwindow *w, double mouseX, double mouseY){ Sandbox::cb_mousePos(mouseX, mouseY); }
void mouseWheelCB(GLFWwindow *w, double xoff, double yoff)  { Sandbox::cb_mouseWheel(xoff); }
void charCB(GLFWwindow *w, int glfwChar, int glfwAction)    { Sandbox::cb_char(glfwChar, glfwAction); }
void keyCB(GLFWwindow *w, int key, int scancode,
	                      int action, int mods)             { Sandbox::cb_key(key, action); }
/////////////////


class RenderWindow {
public:
	inline static void init() {
		if (!glfwInit()) 	{
			std::stringstream msg;
			msg << "Failed to itialize GLFW";
			throw std::exception(msg.str().c_str());
		}
	}

	inline static void terminate() {
		glfwTerminate(); 
	}

	inline static void swapBuffers() { 
		glfwSwapBuffers(window); 
		glfwPollEvents();
	}

	inline static int getTime() {
		return glfwGetTime();
	}

	inline static void getCursorPos(double *xpos, double *ypos) {
		glfwGetCursorPos(window, xpos, ypos);
	}

	inline static bool shouldClose() {
		return glfwWindowShouldClose(window);
	}
	
	inline static void showFPSinTitleBar() { 
		static char titleString[] = "fps: 0000.00";
		sprintf(titleString + 5, "%4.2f", Sandbox::getFPS());
		glfwSetWindowTitle(window, titleString);
	};

	inline static void setCallbacks() {

		glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

		// glfw callbacks ///////////////////////////////////////////////////	
		glfwSetKeyCallback(window, keyCB);
		glfwSetCharCallback(window, charCB);
		glfwSetCursorPosCallback(window, mousePosCB);
		glfwSetMouseButtonCallback(window, mouseButtonCB);
		glfwSetScrollCallback(window, mouseWheelCB);

		glfwSetWindowSizeCallback(window, windowSizeCB);
	}

	inline static void create(int w, int h, bool fullscreen, int samples) {
		glfwWindowHint(GLFW_SAMPLES, samples);

		// set OpenGL version/profile	
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// debugging?
#if _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode *videomode = glfwGetVideoMode(monitor);

		int win_width, win_height;
		if (fullscreen) {
			win_width = videomode->width;
			win_height = videomode->height;
		}
		else {
			win_width = std::min(DEFAULT_WIN_WIDTH, videomode->width);
			win_height = std::min(DEFAULT_WIN_HEIGHT, videomode->height);
			monitor = nullptr;
		}
		const int depth_bits = 32;
		const int stencil_bits = 32;

		window = glfwCreateWindow(w, h,
			"YARS - Yet Another Rendering Sandbox",
			monitor,
			nullptr);

		if (!window) {
			terminate();
			
			std::stringstream msg;  
			msg << "Failed to open GLFW window for OpenGL " << OPENGL_VERSION_MAJOR << "." << OPENGL_VERSION_MINOR;
			throw std::exception(msg.str().c_str());
		}
	}
	

	static const int DEFAULT_WIN_WIDTH = 1280;
	static const int DEFAULT_WIN_HEIGHT = 720;
	
	static const int OPENGL_VERSION_MAJOR = 4;
	static const int OPENGL_VERSION_MINOR = 2;


private:
	static GLFWwindow *window;	

};
*/

#endif
