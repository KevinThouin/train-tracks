#ifndef __ARROW_HPP__
#define __ARROW_HPP__

#include <list>
#include <cassert>

#include "draw_gl.hpp"
#include "curves.hpp"

class Arrow {
public:
	class Renderer {
		Arrow& m_arrow;
		ShowableBezier m_arrowBody;
		ShowableLine m_tip[2];
		unsigned int m_begin, m_end;
		
	public:
		Renderer(RendererGL& rendererGL, Arrow& arrow, float p0x, float p0y, float p1x, float p1y, float t0x, float t0y, float t1x, float t1y, int layer,
				 unsigned int begin, unsigned int end);
		void changePoints(float p0x, float p0y, float p1x, float p1y, float t0x, float t0y, float t1x, float t1y);
		void setBeginEnd(unsigned int begin, unsigned int end) {
			m_begin = begin;
			m_end = end;
		}
		void setLayer(int newLayer);
		
		unsigned int begin()    const {return m_begin;}
		unsigned int end()      const {return m_end;}
		Arrow&       getArrow() const {return m_arrow;}
	};
	
private:
	unsigned int m_begin, m_end;
	
public:
	Arrow(unsigned int begin, unsigned int end) : m_begin(begin), m_end(end) {assert(m_begin!=m_end);}
	
	unsigned int begin()  const {return m_begin;}
	unsigned int end()    const {return m_end;}
	unsigned int lenght() const {return (m_begin>m_end) ? m_begin-m_end : m_end-m_begin;}
	bool         isUp()   const {return m_end<m_begin;}
	bool         isDown() const {return m_begin<m_end;}
	
	friend Renderer;
};

#endif // __ARROW_HPP__
