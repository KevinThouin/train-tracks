#ifndef __ARROW_HPP__
#define __ARROW_HPP__

#include <list>
#include <cassert>

#include "draw_gl.hpp"
#include "curves.hpp"

class Arrow {
public:
	class Renderer {
		Arrow& arw;
		ShowableBezier arrowBody;
		ShowableLine tip[2];
		unsigned int i, j;
		
	public:
		Renderer(RendererGL& rendererGL, Arrow& arrow, float p0x, float p0y, float p1x, float p1y, float t0x, float t0y, float t1x, float t1y, int layer,
				 unsigned int begin, unsigned int end);
		void changePoints(float p0x, float p0y, float p1x, float p1y, float t0x, float t0y, float t1x, float t1y);
		void setBeginEnd(unsigned int begin, unsigned int end) {
			i = begin;
			j = end;
		}
		void setLayer(int newLayer);
		
		unsigned int begin()    const {return i;}
		unsigned int end()      const {return j;}
		Arrow&       getArrow() const {return arw;}
	};
	
private:
	unsigned int i, j;
	
public:
	Arrow(unsigned int begin, unsigned int end) : i(begin), j(end) {assert(i!=j);}
	
	unsigned int begin()  const {return i;}
	unsigned int end()    const {return j;}
	unsigned int lenght() const {return i>j ? i-j : j-i;}
	bool         isUp()   const {return j<i;}
	bool         isDown() const {return i<j;}
	
	friend Renderer;
};

#endif // __ARROW_HPP__
