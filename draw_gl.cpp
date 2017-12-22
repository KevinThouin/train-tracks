#include <cstdlib>
#include <cstring>
#include <cassert>

#include <GL/glew.h>

#include "draw_gl.hpp"

#ifndef NDEBUG
	pthread_t displayThread;
#endif

static constexpr float pixelFactor = 600.0;
static constexpr float maxScale    = 1000.0;
static constexpr float minScale    = 0.001;

void RendererGL::DisplayParam::setUniforms(const ProgramsGL::Program& program) const {
	glUniform3f(program.getUniform(ProgramsGL::COLOR), color[0], color[1], color[2]);
}

template <>
void RendererGL::DrawData<RendererGL::VertexData>::loadAttributes() {
	glEnableVertexAttribArray(prog.getAttribute(ProgramsGL::POSITION));
	glVertexAttribPointer (ProgramsGL::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof (VertexData), 0);
}

template <>
RendererGL::DrawLists<RendererGL::VertexData>& RendererGL::getDrawLists() {return drawLists;}

void RendererGL::updateDisplayMatrix() {
	matrix[0*4+0] = scl*sizeFactorx;
	matrix[1*4+1] = scl*sizeFactory;
	matrix[3*4+0] = matrix[0*4+0]*trans[0];
	matrix[3*4+1] = matrix[1*4+1]*trans[1];
}

void RendererGL::translate(int dx, int dy) {
	trans[0]+=2*dx/(pixelFactor*scl);
	trans[1]+=2*dy/(pixelFactor*scl);
	updateDisplayMatrix();
}

void RendererGL::scale(float factor) {
	scl*=factor;
	if (scl > maxScale)
		scl = maxScale;
	else if (scl < minScale)
		scl = minScale;
	updateDisplayMatrix();
}

void RendererGL::resize(int x, int y) {
	sizeFactorx = pixelFactor/x;
	sizeFactory = pixelFactor/y;
	updateDisplayMatrix();
}
