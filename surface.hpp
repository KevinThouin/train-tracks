#ifndef __SURFACE_HPP__
#define __SURFACE_HPP__

class Quad {
private:
	RendererGL::PrimitiveData<RendererGL::VertexData> m_primitiveData;
	RendererGL::VertexData m_data[6];
	
public:
	Quad() = default;
	Quad(Quad&& other) = default;
	Quad(RendererGL& rendererGL, float x1, float x2, float y1, float y2) 
			: m_primitiveData(rendererGL), m_data{RendererGL::VertexData(x1, y1), RendererGL::VertexData(x2, y1), RendererGL::VertexData(x2, y2), 				
				RendererGL::VertexData(x1, y1), RendererGL::VertexData(x2, y2), RendererGL::VertexData(x1, y2)}
	{m_primitiveData.setPrimitiveType(RendererGL::TRIANGLES);}
	
	Quad(RendererGL& rendererGL, float x1, float x2, float y1, float y2, const RendererGL::DisplayParam& displayParam) 
			: m_primitiveData(rendererGL, displayParam), m_data{RendererGL::VertexData(x1, y1), RendererGL::VertexData(x2, y1), RendererGL::VertexData(x2, y2), 				
				RendererGL::VertexData(x1, y1), RendererGL::VertexData(x2, y2), RendererGL::VertexData(x1, y2)}
	{m_primitiveData.setPrimitiveType(RendererGL::TRIANGLES);}
	
	Quad& operator=(Quad&& other) = default;
	
	void show(RendererGL& rendererGL) {m_primitiveData.setDataWithSize(rendererGL, 6, m_data);}
	void hide() {m_primitiveData.remove();}
	void setColor(float red, float green, float blue) {m_primitiveData.setColor(red, green, blue);}
	void setLayer(int layer) {m_primitiveData.setLayer(layer);}
	void changePoints(float x1, float x2, float y1, float y2) {
		m_data[0] = RendererGL::VertexData(x1, y1);
		m_data[1] = RendererGL::VertexData(x2, y1);
		m_data[2] = RendererGL::VertexData(x2, y2);
		m_data[3] = RendererGL::VertexData(x1, y1);
		m_data[4] = RendererGL::VertexData(x2, y2);
		m_data[5] = RendererGL::VertexData(x1, y2);
		m_primitiveData.updateData(m_data);
	}
	
};

#endif // __SURFACE_HPP__
