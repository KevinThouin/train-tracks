#ifndef __CURVES_HPP__
#define __CURVES_HPP__

#include <cstdlib>
#include <list>
#include <vector>

#include "draw_gl.hpp"

class Curve {
protected:
	virtual float getPosComponent(float t, size_t component) const =0;
	virtual float getSpeedComponent(float t, size_t component) const =0;
	virtual float getAccelerationComponent(float t, size_t component) const =0;
	
public:
	virtual float getLenght() =0;
	void getPoint(float& out_x, float& out_y, float t) const;
	void getTangent(float& out_x, float& out_y, float t) const;
	void getNormalToPoint(float& out_x, float& out_y, float t, float px, float py) const;
};

class ShowableCurve {
protected:
	RendererGL::PrimitiveData<RendererGL::VertexData> m_primitiveData;
	std::vector<RendererGL::VertexData> m_data;
	
	void updateData();
	
public:
	ShowableCurve(size_t num_step) : m_primitiveData(), m_data(2*num_step) {}
	ShowableCurve(const ShowableCurve& other) = delete;
	ShowableCurve(ShowableCurve&& other) = default;
	
	ShowableCurve(RendererGL& rendererGL, size_t num_step) : m_primitiveData(rendererGL), m_data(2*num_step) {}
	
	ShowableCurve(RendererGL& rendererGL, const RendererGL::DisplayParam& displayParam, size_t num_step) : m_primitiveData(rendererGL, displayParam),
			m_data(2*num_step) {}
	
	ShowableCurve& operator=(ShowableCurve&& other) = default;
	ShowableCurve& operator=(const ShowableCurve& other) = delete;
	
	virtual Curve& getCurve() =0;
	virtual const Curve& getCurve() const =0;
	virtual void show(RendererGL& rendererGL) {m_primitiveData.setDataWithSize(rendererGL, m_data.size(), m_data.data());}
	virtual void hide() {m_primitiveData.remove();}
	void setColor(float red, float green, float blue) {m_primitiveData.setColor(red, green, blue);}
	void setLayer(int layer) {m_primitiveData.setLayer(layer);}
	float getLenght() {return getCurve().getLenght();}
	
	void getPoint(float& out_x, float& out_y, float t) const {getCurve().getPoint(out_x, out_y, t);}
	void getTangent(float& out_x, float& out_y, float t) const {getCurve().getTangent(out_x, out_y, t);}
	void getNormalToPoint(float& out_x, float& out_y, float t, float px, float py) const {getCurve().getNormalToPoint(out_x, out_y, t, px, py);}
};

template <class CURVE>
class ShowableCurveTemplate : public ShowableCurve {
protected:
	CURVE m_curve;
	void updateData();
	
public:
	ShowableCurveTemplate(size_t num_step) : ShowableCurve(num_step), m_curve() {}
	ShowableCurveTemplate(const ShowableCurveTemplate<CURVE>& other) = delete;
	ShowableCurveTemplate(ShowableCurveTemplate<CURVE>&& other) = default;
	
	template<class ... ARGS>
	ShowableCurveTemplate(RendererGL& rendererGL, size_t num_step, ARGS ... args) : ShowableCurve(rendererGL, num_step), m_curve(args...) {
		updateData();
		m_primitiveData.updateData(m_data.data());
	}
	
	template<class ... ARGS>
	ShowableCurveTemplate(RendererGL& rendererGL, const RendererGL::DisplayParam& displayParam, size_t num_step, ARGS ... args) : ShowableCurve(rendererGL, displayParam),
			m_curve(args...) {
		updateData();
		m_primitiveData.updateData(m_data.data());
	}
	
	ShowableCurveTemplate& operator=(ShowableCurveTemplate&& other) = default;
	ShowableCurveTemplate& operator=(const ShowableCurveTemplate& other) = delete;
	
	virtual Curve& getCurve() {return m_curve;}
	virtual const Curve& getCurve() const {return m_curve;}
	
	template <class ... ARGS>
	void changePoints(ARGS ... args) {
		m_curve.changePoints(args...);
		updateData();
		m_primitiveData.updateData(m_data.data());
	}
	
	void changePoints(const CURVE& curve) {
		m_curve = curve;
		updateData();
		m_primitiveData.updateData(m_data.data());
	}
};

template <class CURVE, size_t N>
class ShowableCurveFixVertices : public ShowableCurveTemplate<CURVE> {
public:
	ShowableCurveFixVertices() : ShowableCurveTemplate<CURVE>(N) {}
	
	template<class ... ARGS>
	ShowableCurveFixVertices(RendererGL& rendererGL, ARGS ... args) : ShowableCurveTemplate<CURVE>(rendererGL, N, args...) {}
	
	template<class ... ARGS>
	ShowableCurveFixVertices(RendererGL& rendererGL, const RendererGL::DisplayParam& displayParam, ARGS ... args) :
		ShowableCurveTemplate<CURVE>(rendererGL, displayParam, N, args...) {}
};

template <class CURVE>
class ShowableCurveVariableVertices : public ShowableCurveTemplate<CURVE> {
	static size_t m_num_step;
	static std::list<ShowableCurveVariableVertices<CURVE>*> m_listCurve;
	
	typename std::list<ShowableCurveVariableVertices<CURVE>*>::iterator m_it = m_listCurve.end();
	bool shown = false;
	
public:
	ShowableCurveVariableVertices() : ShowableCurveTemplate<CURVE>(m_num_step) {}
	ShowableCurveVariableVertices(ShowableCurveVariableVertices<CURVE>&& other) : ShowableCurveTemplate<CURVE>(std::move(other)), m_it(other.m_it) {
		other.m_it = m_listCurve.end();
	}
	
	template<class ... ARGS>
	ShowableCurveVariableVertices(RendererGL& rendererGL, ARGS ... args) : ShowableCurveTemplate<CURVE>(rendererGL, m_num_step, args...) {
		m_listCurve.push_back(this);
		m_it = --m_listCurve.end();
	}
	
	template<class ... ARGS>
	ShowableCurveVariableVertices(RendererGL& rendererGL, const RendererGL::DisplayParam& displayParam, ARGS ... args) :
			ShowableCurveTemplate<CURVE>(rendererGL, displayParam, m_num_step, args...) {
		m_listCurve.push_back(this);
		m_it = --m_listCurve.end();
	}
	
	~ShowableCurveVariableVertices() {if (m_it!=m_listCurve.end()) m_listCurve.erase(m_it);}
	
	ShowableCurveVariableVertices<CURVE>& operator=(ShowableCurveVariableVertices<CURVE>&& other) {
		m_it = other.m_it;
		other.m_it = m_listCurve.end();
		ShowableCurveTemplate<CURVE>::operator=(std::move(other));
		return *this;
	}
	
	virtual void show(RendererGL& rendererGL) {shown = true; ShowableCurveTemplate<CURVE>::show(rendererGL);}
	virtual void hide() {shown = false; ShowableCurveTemplate<CURVE>::hide();}
	
	static void changeNumStep(RendererGL& rendererGL, size_t new_num_step);
};

class Line : public Curve {
private:
	float m_data[2][2];
	
protected:
	virtual float getPosComponent(float t, size_t component) const;
	virtual float getSpeedComponent(float t, size_t component) const;
	virtual float getAccelerationComponent(float t, size_t component) const;
	
public:
	Line() = default;
	Line(float p0x, float p0y, float p1x, float p1y) 
			: m_data{{p0x, p0y}, {p1x, p1y}} {}
	
	virtual float getLenght();
	
	void changePoints(float p0x, float p0y, float p1x, float p1y) {
		m_data[0][0] = p0x;
		m_data[0][1] = p0y;
		m_data[1][0] = p1x;
		m_data[1][1] = p1y;
	}
};

class Bezier : public Curve {
private:
	float m_controlPoints[4][2];
	float m_lenght;
	bool m_calculatedLenght = false;
	
	void updateData();
	virtual float getPosComponent(float t, size_t component) const;
	virtual float getSpeedComponent(float t, size_t component) const;
	virtual float getAccelerationComponent(float t, size_t component) const;
	
public:
	Bezier() = default;
	Bezier(float p0x, float p0y, float c0x, float c0y, float p1x, float p1y, float c1x, float c1y);
	
	virtual float getLenght();
	
	void changePoints(float p0x, float p0y, float c0x, float c0y, float p1x, float p1y, float c1x, float c1y);
	float getIntersectionXCoordMatch(const Bezier& other) const;
	Bezier splitCurve(float t, bool keepBegin);
	const float* getControlPoints() const {return &m_controlPoints[0][0];}
};

template <class CURVE>
void ShowableCurveTemplate<CURVE>::updateData() {
	float delta = 2.0/m_data.size();
	float newx, newy, oldx, oldy;
	float t=0.0;
	m_curve.getPoint(newx, newy, t);
	for (size_t i=0; i<m_data.size()/2; i++) {
		oldx = newx;
		oldy = newy;
		t+=delta;
		m_curve.getPoint(newx, newy, t);
		m_data[2*i]   = RendererGL::VertexData(oldx, oldy);
		m_data[2*i+1] = RendererGL::VertexData(newx, newy);
	}
}

template <class CURVE>
size_t ShowableCurveVariableVertices<CURVE>::m_num_step = 10;

template <class CURVE>
std::list<ShowableCurveVariableVertices<CURVE>*> ShowableCurveVariableVertices<CURVE>::m_listCurve;

template <class CURVE>
void ShowableCurveVariableVertices<CURVE>::changeNumStep(RendererGL& rendererGL, size_t new_num_step) {
		assert(new_num_step>0);
	
	m_num_step = new_num_step;
	for (auto it = m_listCurve.begin(); it!=m_listCurve.end(); it++) {
		ShowableCurveVariableVertices<CURVE>& curve = **it;
		curve.m_data.resize(2*m_num_step);
		curve.updateData();
		if (curve.shown)
			curve.primitiveData.setDataWithSize(rendererGL, 2*m_num_step, curve.data);
	}
}

typedef ShowableCurveVariableVertices<Bezier> ShowableBezier;
typedef ShowableCurveFixVertices<Line, 1> ShowableLine;

#endif // __CURVES_HPP__

