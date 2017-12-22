#include <cmath>

#include "arrow.hpp"
#include "color.h"

Arrow::Renderer::Renderer(RendererGL& rendererGL, Arrow& arrow, float p0x, float p0y, float p1x, float p1y, float t0x, float t0y, float t1x, float t1y, int layer,
				 unsigned int begin, unsigned int end) : arw(arrow),i(begin), j(end)
{
	float dx = p1x-p0x;
	float dy = p1y-p0y;
	float div = t1x*t0y-t1y*t0x;
	
	if (std::abs(div)>=1e-3) {
		float t1 = (dy*t0x-dx*t0y)/div;
		float t0 = (dy*t1x-dx*t1y)/div;
		arrowBody = ShowableBezier(rendererGL, p0x, p0y, p0x+0.666666666667*t0*t0x, p0y+0.666666666667*t0*t0y,
								   p1x+0.666666666667*t1*t1x, p1y+0.666666666667*t1*t1y, p1x, p1y);
	} else {
		arrowBody = ShowableBezier(rendererGL, p0x, p0y, p0x+0.333333333333*dx,     p0y+0.333333333333*dy,
								   p1x-0.333333333333*dx,     p1y-0.333333333333*dy,     p1x, p1y);
	}
	
	float tipFactor;
	float dst = dx*dx+dy*dy;
	if (dst<0.05*0.05) {
		tipFactor = sqrt(dst)/2;
	}
	else
		tipFactor = 0.05/2;
	
	float f = t1x*dx+t1y*dy;
	if (f<0) tipFactor = -tipFactor;
	f = sqrt(t1x*t1x+t1y+t1y);
	if (std::abs(f) >= 1e-3) {
		t1x/=f; t1y/=f;
	}
	
	tip[0] = ShowableLine(rendererGL, p1x, p1y, p1x+tipFactor*(t1x-t1y), p1y+tipFactor*(t1y+t1x));
	tip[1] = ShowableLine(rendererGL, p1x, p1y, p1x+tipFactor*(t1x+t1y), p1y+tipFactor*(t1y-t1x));
	
	arrowBody.setColor(arrowColor.r, arrowColor.g, arrowColor.b);
	arrowBody.setLayer(layer+1);
	arrowBody.show(rendererGL);
	
	for (int i=0; i<2; i++) {
		tip[i].setColor(arrowColor.r, arrowColor.g, arrowColor.b);
		tip[i].setLayer(layer+1);
		tip[i].show(rendererGL);
	}
}

void Arrow::Renderer::changePoints(float p0x, float p0y, float p1x, float p1y, float t0x, float t0y, float t1x, float t1y) {
	float dx = p1x-p0x;
	float dy = p1y-p0y;
	float div = t1x*t0y-t1y*t0x;
	
	if (std::abs(div)>=1e-3) {
		float t1 = (dy*t0x-dx*t0y)/div;
		float t0 = (dy*t1x-dx*t1y)/div;
		arrowBody.changePoints(p0x, p0y, p0x+0.666666666667*t0*t0x, p0y+0.666666666667*t0*t0y, p1x-0.666666666667*t1*t1x, p1y-0.666666666667*t1*t1y, p1x, p1y);
	} else {
		arrowBody.changePoints(p0x, p0y, p0x+0.333333333333*dx,     p0y+0.333333333333*dy,     p1x-0.333333333333*dx,     p1y-0.333333333333*dy,     p1x, p1y);
	}
	
	
	float tipFactor;
	float dst = dx*dx+dy*dy;
	if (dst<0.05*0.05) {
		tipFactor = sqrt(dst)/2;
	}
	else
		tipFactor = 0.05/2;
	
	float f = t1x*dx+t1y*dy;
	if (f>0) tipFactor = -tipFactor;
	f = sqrt(t1x*t1x+t1y*t1y);
	if (f >= 1e-3) {
		t1x/=f; t1y/=f;
	}
	
	tip[0].changePoints(p1x, p1y, p1x+tipFactor*(t1x-t1y), p1y+tipFactor*(t1y+t1x));
	tip[1].changePoints(p1x, p1y, p1x+tipFactor*(t1x+t1y), p1y+tipFactor*(t1y-t1x));
}

void Arrow::Renderer::setLayer(int newLayer) {
	arrowBody.setLayer(newLayer+1);
	tip[0].setLayer(newLayer+1);
	tip[1].setLayer(newLayer+1);
}
