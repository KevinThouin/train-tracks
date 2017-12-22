#include <cassert>
#include <cmath>

#include "curves.hpp"

void Curve::getPoint(float& out_x, float& out_y, float t) const {
	out_x = getPosComponent(t, 0);
	out_y = getPosComponent(t, 1);
}

void Curve::getTangent(float& out_x, float& out_y, float t) const {
	out_x = getSpeedComponent(t, 0);
	out_y = getSpeedComponent(t, 1);
}

void Curve::getNormalToPoint(float& out_x, float& out_y, float t, float px, float py) const {
	out_y = getSpeedComponent(t, 0);
	out_x = -getSpeedComponent(t, 1);
	
	float norm = sqrt(out_x*out_x+out_y*out_y);
	out_x /= norm;
	out_y /= norm;
	
	px -= getPosComponent(t, 0);
	py -= getPosComponent(t, 1);
	if (out_x*px+out_y*py < 0.0) {
		out_x = -out_x;
		out_y = -out_y;
	}
}

void ShowableCurve::updateData() {
	float delta = 2.0/m_data.size();
	float newx, newy, oldx, oldy;
	float t=0.0;
	getCurve().getPoint(newx, newy, t);
	for (size_t i=0; i<m_data.size()/2; i++) {
		oldx = newx;
		oldy = newy;
		t+=delta;
		getCurve().getPoint(newx, newy, t);
		m_data[2*i]   = RendererGL::VertexData(oldx, oldy);
		m_data[2*i+1] = RendererGL::VertexData(newx, newy);
	}
}

float Line::getPosComponent(float t, size_t component) const {
	return m_data[1][component]*t + m_data[0][component]*(1-t);
}

float Line::getSpeedComponent(float t, size_t component) const {
	return m_data[1][component] - m_data[0][component];
}

float Line::getAccelerationComponent(float t, size_t component) const {
	return 0.0;
}

float Line::getLenght() {
	float dx = m_data[1][0]-m_data[0][0];
	float dy = m_data[1][1]-m_data[0][1];
	return sqrt(dx*dx+dy*dy);
}

Bezier::Bezier(float p0x, float p0y, float c0x, float c0y, float p1x, float p1y, float c1x, float c1y) {
	changePoints(p0x, p0y, c0x, c0y, p1x, p1y, c1x, c1y);
}

float Bezier::getLenght() {
	const int numStep = 100;
	const float dt = 1.0/numStep;
	
	if (!m_calculatedLenght) {
		m_lenght = 0.0;
		float old_x, old_y;
		float new_x, new_y;
		float dx, dy;
		float t = 0.0;
		getPoint(new_x, new_y, t);
		for (int i=0; i<numStep; i++) {
			t+=dt;
			old_x = new_x; old_y = new_y;
			getPoint(new_x, new_y, t);
			dx = new_x - old_x;
			dy = new_y - old_y;
			m_lenght += sqrt(dx*dx+dy*dy);
		}
		
		m_calculatedLenght = true;
	}
	
	return m_lenght;
}

float Bezier::getPosComponent(float t, size_t component) const {
	float c0 = (1.0-t)*(1.0-t)*(1.0-t);
	float c1 = 3*t*(1.0-t)*(1.0-t);
	float c2 = 3*t*t*(1.0-t);
	float c3 = t*t*t;
	
	return c0*controlPoints[0][component] + c1*controlPoints[1][component] + c2*controlPoints[2][component] + c3*controlPoints[3][component];
}

float Bezier::getSpeedComponent(float t, size_t component) const {
	float c0 = -3*(1.0-t)*(1.0-t);
	float c1 = 3*(1.0-t)*(1.0-3*t);
	float c2 = -3*t*(3*t-2.0);
	float c3 = 3*t*t;
	
	return c0*controlPoints[0][component] + c1*controlPoints[1][component] + c2*controlPoints[2][component] + c3*controlPoints[3][component];
}

float Bezier::getAccelerationComponent(float t, size_t component) const {
	float c0 = 6.0*(1.0-t);
	float c1 = 12-18*t;
	float c2 = 18*t+6;
	float c3 = 6.0;
	
	return c0*controlPoints[0][component] + c1*controlPoints[1][component] + c2*controlPoints[2][component] + c3*controlPoints[3][component];
}

void Bezier::changePoints(float p0x, float p0y, float c0x, float c0y, float p1x, float p1y, float c1x, float c1y) {
	controlPoints[0][0] = p0x;
	controlPoints[0][1] = p0y;
	controlPoints[1][0] = c0x;
	controlPoints[1][1] = c0y;
	controlPoints[2][0] = p1x;
	controlPoints[2][1] = p1y;
	controlPoints[3][0] = c1x;
	controlPoints[3][1] = c1y;
}

/* Utilise l'algorithme de bissection-Newton pour approximer l'intersection
 *  de deux courbes de Bezier dont les composantesx corrspondent */
float Bezier::getIntersectionXCoordMatch(const Bezier& other) const {
	const float EPSILON = 1e-5;
	const float GAMMA   = 0.75;
	float x0 = 0.0;
	float x1 = 1.0;
	
	assert((getPosComponent(x0, 1)-other.getPosComponent(x0, 1))*(getPosComponent(x1, 1)-other.getPosComponent(x1, 1)) < 0.0);
	
	const int sgn = ((getPosComponent(x0, 1)-other.getPosComponent(x0, 1)) - (getPosComponent(x1, 1)-other.getPosComponent(x1, 1)) < 0.0) ? 1.0 : -1.0;
	
	while (std::abs(x1-x0)>=EPSILON) {
		float dN = (other.getPosComponent(x1, 1)-getPosComponent(x1, 1))/(getSpeedComponent(x1, 1)-other.getSpeedComponent(x1, 1));
		float xp;
		if (((x0-x1)*dN > 0) && (dN/(x0-x1) < GAMMA))
			xp = x1 + dN;
		else
			xp = (x0+x1)/2;
		
		float f = getPosComponent(xp, 1)-other.getPosComponent(xp, 1);
		if (std::abs(f)<EPSILON) return xp;
		if (sgn*(x1-x0)*f <= 0.0) x0 = x1;
		x1 = xp;
	}
	
	return (x0+x1)/2;
}

Bezier Bezier::splitCurve(float t, bool keepBegin) {
	float px0, py0, tx0, ty0;
	float px, py, tx, ty;
	float px1, py1, tx1, ty1;
	float s = 1.0-t;
	
	getPoint(px0, py0, 0.0);
	getTangent(tx0, ty0, 0.0);
	getPoint(px, py, t);
	getTangent(tx, ty, t);
	getPoint(px1, py1, 1.0);
	getTangent(tx1, ty1, 1.0);
	
	Bezier ret;
	if (keepBegin) {
		ret = Bezier(px, py,   px+s*tx/3, py+s*ty/3,     px1-s*tx1/3, py1-s*ty1/3, px1, py1);
		changePoints(px0, py0, px0+t*tx0/3, py0+t*ty0/3, px-t*tx/3, py-t*ty/3, px, py);
	} else {
		ret = Bezier(px0, py0, px0+t*tx0/3, py0+t*ty0/3, px-t*tx/3, py-t*ty/3, px, py);
		changePoints(px, py,   px+s*tx/3, py+s*ty/3,     px1-s*tx1/3, py1-s*ty1/3, px1, py1);
	}
	
	return ret;
}
