#ifndef __DRAW_GL_HPP__
#define __DRAW_GL_HPP__

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <map>
#include <limits>

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
		GLfloat pos[2];
		
		VertexData() = default;
		VertexData(GLfloat x, GLfloat y) : pos{x, y} {}
	};

	enum PrimitiveType {
		TRIANGLES = 0,
		LINES = 1
	};

	struct DisplayParam {
		int layer;
		PrimitiveType primitiveType;
		GLfloat color[3];
		
		DisplayParam() : layer(0), primitiveType(LINES), color{1.0, 1.0, 1.0} {}
		
		void setUniforms(const ProgramsGL::Program& program) const;
		
		bool operator<(const DisplayParam& other) const {
			if (layer!=other.layer)
				return layer < other.layer;
			
			if (primitiveType!=other.primitiveType)
				return primitiveType < other.primitiveType;
			
			for (size_t i=0; i<2; i++) {
				if (color[i]!=other.color[i])
					return color[i] < other.color[i];
			}
			
			return color[2] < other.color[2];
		}
	};


	template <typename VERTEX_DATA>
	class PrimitiveData {
	private:
		DisplayParam displayParam;
		DrawData<VERTEX_DATA>* drawData = nullptr;
		DrawLists<VERTEX_DATA>* drawLst = nullptr;
		size_t n_vert = 0;
		size_t location = std::numeric_limits<size_t>::max();
		bool empty = false;
		
		void getDrawData();
		
		void changeLocation(size_t new_location) {location = new_location;}
		size_t getLocation() {return location;}
		
	public:
		PrimitiveData(const PrimitiveData& other) = delete;
		PrimitiveData() : empty(true) {};
		
		PrimitiveData(PrimitiveData&& other) : displayParam(other.displayParam), drawData(other.drawData),
				drawLst(other.drawLst), n_vert(other.n_vert), location(other.location) {
			if (drawData!=nullptr) {
				assert(drawData->primitives[location] == &other);
				drawData->primitives[location] = this;
			}
			other.location = std::numeric_limits<size_t>::max();
			other.drawData = nullptr;
			other.drawLst  = nullptr;
			other.empty = true;
		}
		
		PrimitiveData(RendererGL& rendererGL, const DisplayParam& dispParam) : displayParam(dispParam), drawLst(&(rendererGL.getDrawLists<VERTEX_DATA>())) {}
		PrimitiveData(RendererGL& rendererGL, const DisplayParam& dispParam, size_t num_vertex, VERTEX_DATA* data) : 
				displayParam(dispParam), drawLst(&(rendererGL.getDrawLists<VERTEX_DATA>())), n_vert(num_vertex) {
			setDataWithSize(rendererGL, num_vertex, data);
		}
		
		PrimitiveData(RendererGL& rendererGL) : drawLst(&(rendererGL.getDrawLists<VERTEX_DATA>())) {}
		PrimitiveData(RendererGL& rendererGL, size_t num_vertex, VERTEX_DATA* data) : 
				drawLst(&(rendererGL.getDrawLists<VERTEX_DATA>())), n_vert(num_vertex) {
			setDataWithSize(rendererGL, num_vertex, data);
		}
		
		~PrimitiveData() {remove();}
		
		PrimitiveData& operator=(PrimitiveData&& other) {
			displayParam = other.displayParam;
			drawData     = other.drawData;
			drawLst      = other.drawLst;
			n_vert       = other.n_vert;
			location     = other.location;
			if (drawData!=nullptr) {
				assert(drawData->primitives[location] == &other);
				drawData->primitives[location] = this;
			}
			other.location = std::numeric_limits<size_t>::max();
			other.drawData = nullptr;
			other.drawLst  = nullptr;
			other.empty = true;
			
			return *this;
		}
		
		void setDataWithSize(RendererGL& rendererGL, size_t num_vertex, VERTEX_DATA* data) {
			assert(drawLst!=nullptr);
			
			if (drawData!=nullptr)
				drawData->removeElement(location);
			
			n_vert = num_vertex;
			drawData = &(drawLst->getDrawData(n_vert, displayParam));
			drawData->pushElement(data, *this);
		}
		
		void remove() {
			if (drawData!=nullptr) {
				drawData->removeElement(location);
				drawData = nullptr;
			}
		}
		
		const DisplayParam& getDisplayParam() {return displayParam;}
		
		void updateData(VERTEX_DATA* data) {if (drawData!=nullptr) drawData->changeElement(data, location);}
		
		void setColor(GLfloat red, GLfloat green, GLfloat blue) {
			displayParam.color[0]=red;
			displayParam.color[1]=green;
			displayParam.color[2]=blue;
			if (drawData!=nullptr)
				getDrawData();
		}
		
		void setLayer(int layer) {displayParam.layer = layer; if (drawData!=nullptr) getDrawData();}
		void setPrimitiveType(PrimitiveType type) {displayParam.primitiveType = type; if (drawData!=nullptr) getDrawData();}
		
		friend DrawData<VERTEX_DATA>;
	};

	
	
	
	
	
private:
	template <typename VERTEX_DATA>
	class DrawData {
	private:
		const size_t ver_per_pri;
		size_t size_total = 0;
		size_t capacity = 1;
		GLenum m;
		GLuint vao;
		GLuint vbo;
		const ProgramsGL::Program& prog;
		VERTEX_DATA* data;
		std::vector<PrimitiveData<VERTEX_DATA>*> primitives;
		
		void bindVBO() {
			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
		}
		
		void unbindVBO() {
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		
		void loadAttributes();
		
	public:
		DrawData(GLenum mode, size_t vertex_per_primitive, const ProgramsGL::Program& program);
		DrawData(const DrawData& drawData) = delete;
		~DrawData();
		
		void pushElement(const VERTEX_DATA* data, PrimitiveData<VERTEX_DATA>& element);
		void removeElement(size_t i);
		void changeElement(VERTEX_DATA* new_element, size_t i);
		const VERTEX_DATA* getElement(size_t i) {return data + i*ver_per_pri;}
		bool empty() {return size_total==0;}
		
		void draw();
		
		friend PrimitiveData<VERTEX_DATA>;
	};

	template <class VERTEX_DATA>
	class DrawLists {
	private:
		const ProgramsGL::Program* prog = nullptr;
		std::map<DisplayParam, std::map<size_t, DrawData<VERTEX_DATA>>> displayMap;
		
	public:
		DrawLists() = default;
		DrawLists(const ProgramsGL::Program& program) : prog(&program) {}
		
		void draw(const ProgramsGL& programs, const RendererGL& rendererGL);
		
		DrawData<VERTEX_DATA>& getDrawData(size_t num_vertex, const DisplayParam& displayParam);
	};
	
	
	
	
	
	
	
private:
	DrawLists<VertexData> drawLists;
	const ProgramsGL* programs = nullptr;
	GLfloat matrix[16];
	GLfloat scl         = 1.0;
	GLfloat trans[2]    = {0.0, 0.0};
	GLfloat sizeFactorx = 1.0;
	GLfloat sizeFactory = 1.0;
	
	template <typename VERTEX_DATA>
	DrawLists<VERTEX_DATA>& getDrawLists();
	
	void updateDisplayMatrix();
	
public:
	RendererGL() = default;
	RendererGL(const ProgramsGL& progs) : drawLists(progs.getProgram(ProgramsGL::DEFAULT_PROGRAM)), programs(&progs) {
#ifndef NDEBUG
		displayThread = pthread_self();
#endif
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				matrix[4*i+j] = (i==j) ? 1.0 : 0.0;
			}
		}
		updateDisplayMatrix();
	}
	
	void draw() {drawLists.draw(*programs, *this);}
	
	void translate(int dx, int dy);
	void scale(float factor);
	void resize(int x, int y);
	
	friend DrawLists<VertexData>;
	friend PrimitiveData<VertexData>;
};








template <typename VERTEX_DATA>
void RendererGL::PrimitiveData<VERTEX_DATA>::getDrawData() {
	CHECK_THREAD();
	assert(drawLst!=nullptr);
	
	DrawData<VERTEX_DATA>* newDrawData = &(drawLst->getDrawData(n_vert, displayParam));
	if (newDrawData!=drawData) {
		size_t old_location = location;
		const VERTEX_DATA* data = drawData->getElement(location);
		drawData->removeElement(old_location);
		newDrawData->pushElement(data, *this);
		drawData = newDrawData;
	}
}


template <typename VERTEX_DATA>
RendererGL::DrawData<VERTEX_DATA>::DrawData(GLenum mode, size_t vertex_per_primitive, const ProgramsGL::Program& program)
		: ver_per_pri(vertex_per_primitive), m(mode), prog(program) {
	CHECK_THREAD();
	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);
	
	glGenBuffers (1, &vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	
	data = static_cast<VERTEX_DATA*>(malloc(ver_per_pri*sizeof(VERTEX_DATA)));
	glBufferData(GL_ARRAY_BUFFER, capacity*ver_per_pri*sizeof(VERTEX_DATA), data, GL_DYNAMIC_DRAW);
	
	loadAttributes();
	
	unbindVBO();
}

template <typename VERTEX_DATA>
RendererGL::DrawData<VERTEX_DATA>::~DrawData() {
	CHECK_THREAD();
	free(data);
	glDeleteVertexArrays (1, &vao);
	glDeleteBuffers (1, &vbo);
}

template <typename VERTEX_DATA>
void RendererGL::DrawData<VERTEX_DATA>::removeElement(size_t i) {
	CHECK_THREAD();
	assert(size_total!=0);
	assert(i<size_total);
	
	assert(primitives[i]->getLocation() == i);
	assert(primitives.back()->getLocation() == size_total-1);
	
	primitives[i]->changeLocation(std::numeric_limits<size_t>::max());
	size_total--;
	if (i!=size_total) {
		VERTEX_DATA* last_ptr = data + size_total*ver_per_pri;
		VERTEX_DATA* ptr      = data + i*ver_per_pri;
		memcpy(ptr, last_ptr, ver_per_pri*sizeof(VERTEX_DATA));
		
		primitives.back()->changeLocation(i);
		primitives[i] = primitives.back();
		
		bindVBO();
		glBufferSubData(GL_ARRAY_BUFFER, i*ver_per_pri*sizeof(VERTEX_DATA), ver_per_pri*sizeof(VERTEX_DATA), ptr);
		unbindVBO();
	}
	primitives.pop_back();
}

template <typename VERTEX_DATA>
void RendererGL::DrawData<VERTEX_DATA>::pushElement(const VERTEX_DATA* vert_data, PrimitiveData<VERTEX_DATA>& element) {
	CHECK_THREAD();
	
	bool resized = false;
	if (++size_total > capacity) {
		capacity *= 2;
		data = static_cast<VERTEX_DATA*>(realloc(data, capacity*ver_per_pri*sizeof(VERTEX_DATA)));
		resized = true;
	}
	
	assert(size_total!=0);
	VERTEX_DATA* ptr = data + (size_total-1)*ver_per_pri;
	memcpy(ptr, vert_data, ver_per_pri*sizeof(VERTEX_DATA));
	primitives.push_back(&element);
	element.changeLocation(size_total-1);
	
	bindVBO();
	if (resized) {
		glBufferData(GL_ARRAY_BUFFER, capacity*ver_per_pri*sizeof(VERTEX_DATA), data, GL_DYNAMIC_DRAW);
	}
	else {
		glBufferSubData(GL_ARRAY_BUFFER, (size_total-1)*ver_per_pri*sizeof(VERTEX_DATA), ver_per_pri*sizeof(VERTEX_DATA), vert_data);
	}
	
	unbindVBO();
}

template <typename VERTEX_DATA>
void RendererGL::DrawData<VERTEX_DATA>::changeElement(VERTEX_DATA* new_element, size_t i) {
	CHECK_THREAD();
	assert(i<size_total);
	
	VERTEX_DATA* ptr = data + i*ver_per_pri;
	memcpy(ptr, new_element, ver_per_pri*sizeof(VERTEX_DATA));
	bindVBO();
	glBufferSubData(GL_ARRAY_BUFFER, i*ver_per_pri*sizeof(VERTEX_DATA), ver_per_pri*sizeof(VERTEX_DATA), ptr);
	unbindVBO();
}

template <typename VERTEX_DATA>
void RendererGL::DrawData<VERTEX_DATA>::draw() {
	CHECK_THREAD();
	glBindVertexArray(vao);
	glDrawArrays(m, 0, size_total*ver_per_pri);
	glBindVertexArray(0);
}





template <typename VERTEX_DATA>
void RendererGL::DrawLists<VERTEX_DATA>::draw(const ProgramsGL& programs, const RendererGL& rendererGL) {
	CHECK_THREAD();
	
	prog->use();
	glUniformMatrix4fv(prog->getUniform(ProgramsGL::MATRIX), 1, GL_FALSE, rendererGL.matrix);
	
	for (auto it0 = displayMap.begin(); it0!=displayMap.end();) {
		std::map<size_t, DrawData<VERTEX_DATA>>& map = it0->second;
		if (!map.empty()) {
			it0->first.setUniforms(*prog);
			
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
				it0 = displayMap.erase(it0);
			}
			else
				it0++;
		}
		else
			it0 = displayMap.erase(it0);
	}
	prog->unuse();
}

template <typename VERTEX_DATA>
RendererGL::DrawData<VERTEX_DATA>& RendererGL::DrawLists<VERTEX_DATA>::getDrawData(size_t num_vertex, const DisplayParam& displayParam) {
	CHECK_THREAD();
	
	std::map<size_t, DrawData<VERTEX_DATA>>& map = displayMap[displayParam];
	if (map.find(num_vertex) != map.end())
		return map.at(num_vertex);
	else {
		GLenum mode = (displayParam.primitiveType==LINES) ? GL_LINES : GL_TRIANGLES;
		return map.emplace(std::piecewise_construct, std::forward_as_tuple(num_vertex), std::forward_as_tuple(mode, num_vertex, *prog)).first->second;
	}
}

#endif // __DRAW_GL_HPP__
