#ifndef FULLSCREENQUAD_H
#define	FULLSCREENQUAD_H

// OPENGL
#include "../util/inc_opengl.h"

class FullScreenQuad {
public:
	FullScreenQuad();	
	~FullScreenQuad();
	
	void draw();
private:
	GLuint vao;
	GLuint vbo;
};

#endif