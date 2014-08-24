#include "FullScreenQuad.h"

#include "../Renderable.h"

FullScreenQuad::FullScreenQuad() {

	static float data[] = {
	-1.0f,  1.0f,	// top left
	-1.0f, -1.0f,	// bottom left
	 1.0f,  1.0f,	// top right
	 1.0f, -1.0f	// bottom right
	};
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
		
	glEnableVertexAttribArray(Renderable::v_positionLoc);	
	glVertexAttribPointer(Renderable::v_positionLoc, 2, GL_FLOAT, GL_FALSE, 0,  (const GLvoid*) 0);	
}

FullScreenQuad::~FullScreenQuad() {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1,&vao);
}

void FullScreenQuad::draw() {
	glBindVertexArray(vao);	
	//glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
	glBindVertexArray(0);
}
