#ifndef __TRACK_HPP__
#define __TRACK_HPP__

#include "curves.hpp"
#include "draw_gl.hpp"

enum Direction {
	EAST  = 0,
	SOUTH = 1,
	WEST  = 2,
	NORTH = 3
};

void getUnitTangentFromDirection(Direction d, float& x, float& y);

class Track {
public:
	virtual ~Track() {}
	
	virtual ShowableCurve& getCurve() =0;
	virtual const ShowableCurve& getCurve() const =0;
	
	void getPoint(float& x, float& y, float t) const {getCurve().getPoint(x, y, t);}
};

class TrackBezier : public Track {
private:
	ShowableBezier curve;
	
public:
	TrackBezier(const TrackBezier& other) = delete;
	TrackBezier(TrackBezier&& other) : curve(std::move(other.curve)) {}
	TrackBezier() = default;
	
	TrackBezier(RendererGL& rendererGL, float p0x, float p0y, float p1x, float p1y, float r, float g, float b, int layer, Direction d0, Direction d1) :
		curve(rendererGL, p0x, p0y, (d0&0x1) ? p0x : 0.5*(p1x+p0x), (d0&0x1) ? 0.5*(p1y+p0y) : p0y, 
			  (d1&0x1) ? p1x : 0.5*(p0x+p1x), (d1&0x1) ? 0.5*(p0y+p1y) : p1y , p1x, p1y) {
		curve.setColor(r, g, b);
		curve.setLayer(layer);
		curve.show(rendererGL);
	}
	
	TrackBezier(RendererGL& rendererGL, float p0x, float p0y, float p1x, float p1y, float t0x, float t0y, float t1x, float t1y, float r, float g, float b, int layer) :
		curve(rendererGL, p0x, p0y, p0x+t0x, p0y+t0y, p1x+t1x, p1y+t1y, p1x, p1y) {
		curve.setColor(r, g, b);
		curve.setLayer(layer);
		curve.show(rendererGL);
	}
	
	TrackBezier(RendererGL& rendererGL, const Bezier& bezier, float r, float g, float b, int layer) : curve(rendererGL, bezier) {
		curve.setColor(r, g, b);
		curve.setLayer(layer);
		curve.show(rendererGL);
	}
	
	virtual ShowableCurve& getCurve() {return curve;}
	virtual const ShowableCurve& getCurve() const {return curve;}
	
	void changePoints(float p0x, float p0y, float p1x, float p1y, Direction d0, Direction d1) {
		curve.changePoints(p0x, p0y, (d0&0x1) ? p0x : 0.5*(p1x+p0x), (d0&0x1) ? 0.5*(p1y+p0y) : p0y,
						   (d1&0x1) ? p1x : 0.5*(p0x+p1x), (d1&0x1) ? 0.5*(p0y+p1y) : p1y , p1x, p1y);
	}
	
	static Bezier getBezier(float p0x, float p0y, float p1x, float p1y, Direction d0, Direction d1);
	
	void changePoints(float p0x, float p0y, float p1x, float p1y, float t0x, float t0y, float t1x, float t1y) {
		curve.changePoints(p0x, p0y, p0x+t0x, p0y+t0y, p1x+t1x, p1y+t1y, p1x, p1y);
	}
	
	static Bezier getBezier(float p0x, float p0y, float p1x, float p1y, float t0x, float t0y, float t1x, float t1y);
	
	TrackBezier& operator=(TrackBezier&& other) {
		curve = std::move(other.curve);
		return *this;
	}
};

class TrackLine : public Track {
private:
	ShowableLine line;
	
public:
	TrackLine(const TrackLine& other) = delete;
	TrackLine(TrackLine&& other) : line(std::move(other.line)) {}
	TrackLine() = default;
	
	TrackLine(RendererGL& rendererGL, float p0x, float p0y, float p1x, float p1y, float r, float g, float b, int layer) :
		line(rendererGL, p0x, p0y, p1x, p1y) {
		line.setColor(r, g, b);
		line.setLayer(layer);
		line.show(rendererGL);
	}
	
	void changePoints(float p0x, float p0y, float p1x, float p1y) {
		line.changePoints(p0x, p0y, p1x, p1y);
	}
	
	TrackLine& operator=(TrackLine&& other) {
		line = std::move(other.line);
		return *this;
	}
	
	virtual ShowableCurve& getCurve() {return line;}
	virtual const ShowableCurve& getCurve() const {return line;}
};

#endif // __TRACK_HPP__
