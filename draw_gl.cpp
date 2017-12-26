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
	glUniform3f(program.getUniform(ProgramsGL::COLOR), m_color[0], m_color[1], m_color[2]);
}

template <>
void RendererGL::DrawData<RendererGL::VertexData>::loadAttributes() {
	glEnableVertexAttribArray(m_program.getAttribute(ProgramsGL::POSITION));
	glVertexAttribPointer(ProgramsGL::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0);
}

template <>
RendererGL::DrawLists<RendererGL::VertexData>& RendererGL::getDrawLists() {return m_drawLists;}

void RendererGL::updateDisplayMatrix() {
	m_matrix[0*4+0] = m_scl*m_sizeFactorx;
	m_matrix[1*4+1] = m_scl*m_sizeFactory;
	m_matrix[3*4+0] = m_matrix[0*4+0]*m_trans[0];
	m_matrix[3*4+1] = m_matrix[1*4+1]*m_trans[1];
}

void RendererGL::translate(int dx, int dy) {
	m_trans[0]+=2*dx/(pixelFactor*m_scl);
	m_trans[1]+=2*dy/(pixelFactor*m_scl);
	updateDisplayMatrix();
}

void RendererGL::scale(float factor) {
	m_scl*=factor;
	if (m_scl > maxScale)
		m_scl = maxScale;
	else if (m_scl < minScale)
		m_scl = minScale;
	updateDisplayMatrix();
}

void RendererGL::resize(int x, int y) {
	m_sizeFactorx = pixelFactor/x;
	m_sizeFactory = pixelFactor/y;
	updateDisplayMatrix();
}
