#ifndef __DRAW_GL_HPP__
#define __DRAW_GL_HPP__

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <map>
#include <limits>
#include <tuple>
#include <algorithm>

#include <GL/glew.h>
#include <GL/gl.h>

#ifndef NDEBUG
 #include <pthread.h>
#endif

#include "prog_gl.hpp"

#ifndef NDEBUG
	extern pthread_t displayThread;
	
	#define CHECK_THREAD() do {assert(pthread_self()==displayThread);} while(0)
#endif

class RendererGL {
private:
	template <typename VERTEX_DATA>
	class DrawData;
	
	template <class VERTEX_DATA>
	class DrawLists;

public:
	struct VertexData {
		GLfloat m_pos[2];
		
		VertexData() = default;
		VertexData(GLfloat x, GLfloat y) : m_pos{x, y} {}
	};

	enum PrimitiveType {
		TRIANGLES = 0,
		LINES = 1
	};

	struct DisplayParam {
		int m_layer;
		PrimitiveType m_primitiveType;
		GLfloat m_color[3];
		
		DisplayParam() : m_layer(0), m_primitiveType(LINES), m_color{1.0, 1.0, 1.0} {}
		
		void setUniforms(const ProgramsGL::Program& program) const;
		
		bool operator<(const DisplayParam& other) const {
			return std::tie(m_layer, m_primitiveType, m_color[0], m_color[1], m_color[2]) 
					< std::tie(other.m_layer, other.m_primitiveType, other.m_color[0], other.m_color[1], other.m_color[2]);
		}
	};


	template <typename VERTEX_DATA>
	class PrimitiveData {
	private:
		DisplayParam m_displayParam;
		DrawData<VERTEX_DATA>* m_drawData = nullptr;
		DrawLists<VERTEX_DATA>* m_drawList = nullptr;
		size_t m_numberOfVertex = 0;
		size_t m_location = std::numeric_limits<size_t>::max();
		bool m_empty = false;
		
		void getDrawData();
		
		void changeLocation(size_t new_location) {m_location = new_location;}
		size_t getLocation() {return m_location;}
		
	public:
		PrimitiveData(const PrimitiveData& other) = delete;
		PrimitiveData() : m_empty(true) {};
		
		PrimitiveData(PrimitiveData&& other) : m_displayParam(other.m_displayParam), m_drawData(other.m_drawData),
				m_drawList(other.m_drawList), m_numberOfVertex(other.m_numberOfVertex), m_location(other.m_location) {
			if (m_drawData!=nullptr) {
				assert(m_drawData->m_primitives[m_location] == &other);
				m_drawData->m_primitives[m_location] = this;
			}
			other.m_location = std::numeric_limits<size_t>::max();
			other.m_drawData = nullptr;
			other.m_drawList  = nullptr;
			other.m_empty = true;
		}
		
		PrimitiveData(RendererGL& rendererGL, const DisplayParam& displayParam) : m_displayParam(displayParam), m_drawList(&(rendererGL.getDrawLists<VERTEX_DATA>())) {}
		PrimitiveData(RendererGL& rendererGL, const DisplayParam& displayParam, size_t numberOfVertex, VERTEX_DATA* data) : 
				m_displayParam(displayParam), m_drawList(&(rendererGL.getDrawLists<VERTEX_DATA>())), m_numberOfVertex(numberOfVertex) {
			setDataWithSize(rendererGL, numberOfVertex, data);
		}
		
		PrimitiveData(RendererGL& rendererGL) : m_drawList(&(rendererGL.getDrawLists<VERTEX_DATA>())) {}
		PrimitiveData(RendererGL& rendererGL, size_t numberOfVertex, VERTEX_DATA* data) : 
				m_drawList(&(rendererGL.getDrawLists<VERTEX_DATA>())), m_numberOfVertex(numberOfVertex) {
			setDataWithSize(rendererGL, numberOfVertex, data);
		}
		
		~PrimitiveData() {remove();}
		
		PrimitiveData& operator=(PrimitiveData&& other) {
			m_displayParam   = other.m_displayParam;
			m_drawData       = other.m_drawData;
			m_drawList       = other.m_drawList;
			m_numberOfVertex = other.m_numberOfVertex;
			m_location       = other.m_location;
			if (m_drawData!=nullptr) {
				assert(m_drawData->m_primitives[m_location] == &other);
				m_drawData->m_primitives[m_location] = this;
			}
			other.m_location = std::numeric_limits<size_t>::max();
			other.m_drawData = nullptr;
			other.m_drawList  = nullptr;
			other.m_empty = true;
			
			return *this;
		}
		
		void setDataWithSize(RendererGL& rendererGL, size_t numberOfVertex, VERTEX_DATA* data) {
			assert(m_drawList!=nullptr);
			
			if (m_drawData!=nullptr)
				m_drawData->removeElement(m_location);
			
			m_numberOfVertex = numberOfVertex;
			m_drawData = &(m_drawList->getDrawData(m_numberOfVertex, m_displayParam));
			m_drawData->pushElement(data, *this);
		}
		
		void remove() {
			if (m_drawData!=nullptr) {
				m_drawData->removeElement(m_location);
				m_drawData = nullptr;
			}
		}
		
		const DisplayParam& getDisplayParam() {return m_displayParam;}
		
		void updateData(VERTEX_DATA* data) {if (m_drawData!=nullptr) m_drawData->changeElement(data, m_location);}
		
		void setColor(GLfloat red, GLfloat green, GLfloat blue) {
			m_displayParam.m_color[0] = red;
			m_displayParam.m_color[1] = green;
			m_displayParam.m_color[2] = blue;
			if (m_drawData!=nullptr)
				getDrawData();
		}
		
		void setLayer(int layer) {m_displayParam.m_layer = layer; if (m_drawData!=nullptr) getDrawData();}
		void setPrimitiveType(PrimitiveType type) {m_displayParam.m_primitiveType = type; if (m_drawData!=nullptr) getDrawData();}
		
		friend DrawData<VERTEX_DATA>;
	};

	
	
	
	
	
private:
	template <typename VERTEX_DATA>
	class DrawData {
	private:
		const size_t m_vertexPerPrimitive;
		size_t m_size = 0;
		size_t m_capacity = 1;
		GLenum m_mode;
		GLuint m_vao;
		GLuint m_vbo;
		const ProgramsGL::Program& m_program;
		VERTEX_DATA* m_data;
		std::vector<PrimitiveData<VERTEX_DATA>*> m_primitives;
		
		void bindVBO() {
			glBindVertexArray(m_vao);
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		}
		
		void unbindVBO() {
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		
		void loadAttributes();
		
	public:
		DrawData(GLenum mode, size_t vertexPerPrimitive, const ProgramsGL::Program& program);
		DrawData(const DrawData& other) = delete;
		~DrawData();
		
		void pushElement(const VERTEX_DATA* data, PrimitiveData<VERTEX_DATA>& element);
		void removeElement(size_t i);
		void changeElement(VERTEX_DATA* newElement, size_t i);
		const VERTEX_DATA* getElement(size_t i) {return m_data + i*m_vertexPerPrimitive;}
		bool empty() {return m_size==0;}
		
		void draw();
		
		friend PrimitiveData<VERTEX_DATA>;
	};

	template <class VERTEX_DATA>
	class DrawLists {
	private:
		const ProgramsGL::Program* m_program = nullptr;
		std::map<DisplayParam, std::map<size_t, DrawData<VERTEX_DATA>>> m_displayMap;
		
	public:
		DrawLists() = default;
		DrawLists(const ProgramsGL::Program& program) : m_program(&program) {}
		
		void draw(const ProgramsGL& programs, const RendererGL& rendererGL);
		
		DrawData<VERTEX_DATA>& getDrawData(size_t numberOfVertex, const DisplayParam& displayParam);
	};
	
	
	
	
	
	
	
private:
	DrawLists<VertexData> m_drawLists;
	const ProgramsGL* m_programs = nullptr;
	GLfloat m_matrix[16];
	GLfloat m_scl         = 1.0;
	GLfloat m_trans[2]    = {0.0, 0.0};
	GLfloat m_sizeFactorx = 1.0;
	GLfloat m_sizeFactory = 1.0;
	
	template <typename VERTEX_DATA>
	DrawLists<VERTEX_DATA>& getDrawLists();
	
	void updateDisplayMatrix();
	
public:
	RendererGL() = default;
	RendererGL(const ProgramsGL& programs) : m_drawLists(programs.getProgram(ProgramsGL::DEFAULT_PROGRAM)), m_programs(&programs) {
#ifndef NDEBUG
		displayThread = pthread_self();
#endif
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				m_matrix[4*i+j] = (i==j) ? 1.0 : 0.0;
			}
		}
		updateDisplayMatrix();
	}
	
	void draw() {m_drawLists.draw(*m_programs, *this);}
	
	void translate(int dx, int dy);
	void scale(float factor);
	void resize(int x, int y);
	
	friend DrawLists<VertexData>;
	friend PrimitiveData<VertexData>;
};








template <typename VERTEX_DATA>
void RendererGL::PrimitiveData<VERTEX_DATA>::getDrawData() {
	CHECK_THREAD();
	assert(m_drawList!=nullptr);
	
	DrawData<VERTEX_DATA>* newDrawData = &(m_drawList->getDrawData(m_numberOfVertex, m_displayParam));
	if (newDrawData!=m_drawData) {
		size_t old_location = m_location;
		const VERTEX_DATA* data = m_drawData->getElement(m_location);
		m_drawData->removeElement(old_location);
		newDrawData->pushElement(data, *this);
		m_drawData = newDrawData;
	}
}


template <typename VERTEX_DATA>
RendererGL::DrawData<VERTEX_DATA>::DrawData(GLenum mode, size_t vertexPerPrimitive, const ProgramsGL::Program& program)
		: m_vertexPerPrimitive(vertexPerPrimitive), m_mode(mode), m_program(program) {
	CHECK_THREAD();
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	
	m_data = new VERTEX_DATA[m_vertexPerPrimitive];
	glBufferData(GL_ARRAY_BUFFER, m_capacity*m_vertexPerPrimitive*sizeof(VERTEX_DATA), m_data, GL_DYNAMIC_DRAW);
	
	loadAttributes();
	
	unbindVBO();
}

template <typename VERTEX_DATA>
RendererGL::DrawData<VERTEX_DATA>::~DrawData() {
	CHECK_THREAD();
	free(m_data);
	glDeleteVertexArrays (1, &m_vao);
	glDeleteBuffers (1, &m_vbo);
}

template <typename VERTEX_DATA>
void RendererGL::DrawData<VERTEX_DATA>::removeElement(size_t i) {
	CHECK_THREAD();
	assert(i<m_size);
	
	m_primitives[i]->changeLocation(std::numeric_limits<size_t>::max());
	m_size--;
	if (i!=m_size) {
		VERTEX_DATA* last_ptr = m_data + m_size*m_vertexPerPrimitive;
		VERTEX_DATA* ptr      = m_data + i*m_vertexPerPrimitive;
		std::copy_n(last_ptr, m_vertexPerPrimitive, ptr);
		
		m_primitives.back()->changeLocation(i);
		m_primitives[i] = m_primitives.back();
		
		bindVBO();
		glBufferSubData(GL_ARRAY_BUFFER, i*m_vertexPerPrimitive*sizeof(VERTEX_DATA), m_vertexPerPrimitive*sizeof(VERTEX_DATA), ptr);
		unbindVBO();
	}
	m_primitives.pop_back();
}

template <typename VERTEX_DATA>
void RendererGL::DrawData<VERTEX_DATA>::pushElement(const VERTEX_DATA* vert_data, PrimitiveData<VERTEX_DATA>& element) {
	CHECK_THREAD();
	
	bool resized = false;
	if (++m_size > m_capacity) {
		size_t old_capacity = m_capacity;
		VERTEX_DATA* old_data = m_data;
		
		m_capacity *= 2;
		m_data = new VERTEX_DATA[m_capacity*m_vertexPerPrimitive];
		std::copy(old_data, old_data + old_capacity*m_vertexPerPrimitive, m_data);
		delete [] old_data;
		resized = true;
	}
	
	VERTEX_DATA* ptr = m_data + (m_size-1)*m_vertexPerPrimitive;
	std::copy_n(vert_data, m_vertexPerPrimitive, ptr);
	m_primitives.push_back(&element);
	element.changeLocation(m_size-1);
	
	bindVBO();
	if (resized) {
		glBufferData(GL_ARRAY_BUFFER, m_capacity*m_vertexPerPrimitive*sizeof(VERTEX_DATA), m_data, GL_DYNAMIC_DRAW);
	}
	else {
		glBufferSubData(GL_ARRAY_BUFFER, (m_size-1)*m_vertexPerPrimitive*sizeof(VERTEX_DATA), m_vertexPerPrimitive*sizeof(VERTEX_DATA), vert_data);
	}
	
	unbindVBO();
}

template <typename VERTEX_DATA>
void RendererGL::DrawData<VERTEX_DATA>::changeElement(VERTEX_DATA* new_element, size_t i) {
	CHECK_THREAD();
	assert(i<m_size);
	
	VERTEX_DATA* ptr = m_data + i*m_vertexPerPrimitive;
	std::copy_n(new_element, m_vertexPerPrimitive, ptr);
	bindVBO();
	glBufferSubData(GL_ARRAY_BUFFER, i*m_vertexPerPrimitive*sizeof(VERTEX_DATA), m_vertexPerPrimitive*sizeof(VERTEX_DATA), ptr);
	unbindVBO();
}

template <typename VERTEX_DATA>
void RendererGL::DrawData<VERTEX_DATA>::draw() {
	CHECK_THREAD();
	glBindVertexArray(m_vao);
	glDrawArrays(m_mode, 0, m_size*m_vertexPerPrimitive);
	glBindVertexArray(0);
}





template <typename VERTEX_DATA>
void RendererGL::DrawLists<VERTEX_DATA>::draw(const ProgramsGL& programs, const RendererGL& rendererGL) {
	CHECK_THREAD();
	
	m_program->use();
	glUniformMatrix4fv(m_program->getUniform(ProgramsGL::MATRIX), 1, GL_FALSE, rendererGL.m_matrix);
	
	for (auto it0 = m_displayMap.begin(); it0!=m_displayMap.end();) {
		std::map<size_t, DrawData<VERTEX_DATA>>& map = it0->second;
		if (!map.empty()) {
			it0->first.setUniforms(*m_program);
			
			for (auto it1 = map.begin(); it1!=map.end();) {
				DrawData<VERTEX_DATA>& drawData = it1->second;
				if (!drawData.empty()) {
					drawData.draw();
					it1++;
				}
				else {
					it1 = map.erase(it1);
				}
			}
			
			if (map.empty()) {
				it0 = m_displayMap.erase(it0);
			}
			else
				it0++;
		}
		else
			it0 = m_displayMap.erase(it0);
	}
	m_program->unuse();
}

template <typename VERTEX_DATA>
RendererGL::DrawData<VERTEX_DATA>& RendererGL::DrawLists<VERTEX_DATA>::getDrawData(size_t num_vertex, const DisplayParam& displayParam) {
	CHECK_THREAD();
	
	std::map<size_t, DrawData<VERTEX_DATA>>& map = m_displayMap[displayParam];
	if (map.find(num_vertex) != map.end())
		return map.at(num_vertex);
	else {
		GLenum mode = (displayParam.m_primitiveType==LINES) ? GL_LINES : GL_TRIANGLES;
		return map.emplace(std::piecewise_construct, std::forward_as_tuple(num_vertex), std::forward_as_tuple(mode, num_vertex, *m_program)).first->second;
	}
}

#endif // __DRAW_GL_HPP__
