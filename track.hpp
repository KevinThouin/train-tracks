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
	ShowableBezier m_curve;
	
public:
	TrackBezier(const TrackBezier& other) = delete;
	TrackBezier(TrackBezier&& other) : m_curve(std::move(other.m_curve)) {}
	TrackBezier() = default;
	
	TrackBezier(RendererGL& rendererGL, float p0x, float p0y, float p1x, float p1y, float r, float g, float b, int layer, Direction d0, Direction d1) :
		m_curve(rendererGL, p0x, p0y, (d0&0x1) ? p0x : 0.5*(p1x+p0x), (d0&0x1) ? 0.5*(p1y+p0y) : p0y, 
			  (d1&0x1) ? p1x : 0.5*(p0x+p1x), (d1&0x1) ? 0.5*(p0y+p1y) : p1y , p1x, p1y) {
		m_curve.setColor(r, g, b);
		m_curve.setLayer(layer);
		m_curve.show(rendererGL);
	}
	
	TrackBezier(RendererGL& rendererGL, float p0x, float p0y, float p1x, float p1y, float t0x, float t0y, float t1x, float t1y, float r, float g, float b, int layer) :
		m_curve(rendererGL, p0x, p0y, p0x+t0x, p0y+t0y, p1x+t1x, p1y+t1y, p1x, p1y) {
		m_curve.setColor(r, g, b);
		m_curve.setLayer(layer);
		m_curve.show(rendererGL);
	}
	
	TrackBezier(RendererGL& rendererGL, const Bezier& bezier, float r, float g, float b, int layer) : m_curve(rendererGL, bezier) {
		m_curve.setColor(r, g, b);
		m_curve.setLayer(layer);
		m_curve.show(rendererGL);
	}
	
	virtual ShowableCurve& getCurve() {return m_curve;}
	virtual const ShowableCurve& getCurve() const {return m_curve;}
	
	void changePoints(float p0x, float p0y, float p1x, float p1y, Direction d0, Direction d1) {
		m_curve.changePoints(p0x, p0y, (d0&0x1) ? p0x : 0.5*(p1x+p0x), (d0&0x1) ? 0.5*(p1y+p0y) : p0y,
						   (d1&0x1) ? p1x : 0.5*(p0x+p1x), (d1&0x1) ? 0.5*(p0y+p1y) : p1y , p1x, p1y);
	}
	
	static Bezier getBezier(float p0x, float p0y, float p1x, float p1y, Direction d0, Direction d1);
	
	void changePoints(float p0x, float p0y, float p1x, float p1y, float t0x, float t0y, float t1x, float t1y) {
		m_curve.changePoints(p0x, p0y, p0x+t0x, p0y+t0y, p1x+t1x, p1y+t1y, p1x, p1y);
	}
	
	static Bezier getBezier(float p0x, float p0y, float p1x, float p1y, float t0x, float t0y, float t1x, float t1y);
	
	TrackBezier& operator=(TrackBezier&& other) {
		m_curve = std::move(other.m_curve);
		return *this;
	}
};

class TrackLine : public Track {
private:
	ShowableLine m_line;
	
public:
	TrackLine(const TrackLine& other) = delete;
	TrackLine(TrackLine&& other) : m_line(std::move(other.m_line)) {}
	TrackLine() = default;
	
	TrackLine(RendererGL& rendererGL, float p0x, float p0y, float p1x, float p1y, float r, float g, float b, int layer) :
		m_line(rendererGL, p0x, p0y, p1x, p1y) {
		m_line.setColor(r, g, b);
		m_line.setLayer(layer);
		m_line.show(rendererGL);
	}
	
	void changePoints(float p0x, float p0y, float p1x, float p1y) {
		m_line.changePoints(p0x, p0y, p1x, p1y);
	}
	
	TrackLine& operator=(TrackLine&& other) {
		m_line = std::move(other.m_line);
		return *this;
	}
	
	virtual ShowableCurve& getCurve() {return m_line;}
	virtual const ShowableCurve& getCurve() const {return m_line;}
};

#endif // __TRACK_HPP__
