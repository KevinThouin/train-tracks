#include <cassert>
#include <limits>

#include "one_handle.hpp"
#include "display_cmd.hpp"

ArrowBox::Renderer::Renderer(RendererGL& rendererGL, ArrowBox& arrowBox, float basex, float basey, int layer, OneHandleRenderer& oneHandle,
	std::list<std::pair<unsigned int, unsigned int>>& arrowsData, std::list<ArrowBox::ArrowInArrowBox*>& arrows) :
	m_basex(basex), m_basey(basey), m_lenght(arrows.size()*0.055), m_oneHandle(oneHandle), m_layer(layer)
{
	assert(arrowsData.size() == arrows.size());
	
	m_tracks.reserve(m_oneHandle.numberOfTracks());
	for (unsigned int i=0; i<m_oneHandle.numberOfTracks(); i++) {
		m_tracks.emplace_back(rendererGL, m_basex, m_basey-(1.0*(i+1))/(m_oneHandle.numberOfTracks()+1), m_basex+m_lenght,
				m_basey-(1.0*(i+1))/(m_oneHandle.numberOfTracks()+1), trackColor.r, trackColor.g, trackColor.b, layer);
	}
	
	int i=0;
	auto it1 = arrows.begin();
	for(auto it0=arrowsData.begin(); it0!=arrowsData.end(); it0++, it1++, i++) {
		assert(it0->first!=it0->second);
		assert(it0->first<m_oneHandle.numberOfTracks() && it0->second<m_oneHandle.numberOfTracks());
		
		const float delta = 1/(m_oneHandle.numberOfTracks()+1);
		float px  = m_basex+i*0.055+0.0275;
		float p0y = m_basey - (it0->first+1)*delta;
		float p1y = m_basey - (it0->second+1)*delta;
		float t0y = (it0->second > it0->first) ? -1.0 : 1.0;
		
		m_arrow.emplace_back(rendererGL, (*it1)->getArrow(), px, p0y, px, p1y, 0.0, t0y, 0.0, -t0y, m_layer, it0->first, it0->second);
		(*it1)->setArrowRendererInList(--m_arrow.end());
	}
	
	arrowBox.m_renderer = this;
}

ArrowBox::Renderer::Renderer(RendererGL& rendererGL, ArrowBox& arrowBox, float basex, float basey, int layer, OneHandleRenderer& oneHandle) :
	m_basex(basex), m_basey(basey), m_lenght(0.0), m_oneHandle(oneHandle), m_layer(layer)
{
	m_tracks.reserve(m_oneHandle.numberOfTracks());
	for (unsigned int i=0; i<m_oneHandle.numberOfTracks(); i++) {
		m_tracks.emplace_back(rendererGL, m_basex, m_basey-(1.0*(i+1))/(m_oneHandle.numberOfTracks()+1),
				m_basex+m_lenght, m_basey-(1.0*(i+1))/(m_oneHandle.numberOfTracks()+1), trackColor.r, trackColor.g, trackColor.b, layer);
	}
	
	arrowBox.m_renderer = this;
}

void ArrowBox::Renderer::changeBasePoints(float basex, float basey, bool updateArrows) {
	m_basex = basex; m_basey = basey;
	if (updateArrows) {
		refreshArrows();
	}
	refreshTracks();
}

void ArrowBox::Renderer::moveArrowsFake(int beginIndexOffset, int endIndexOffset, AnimateMoveArrowInArrowBoxCommand* anim) {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	int index = 0;
	for (auto it=m_arrow.begin(); it!=m_arrow.end(); ++it) {
		float t0y = (it->end() > it->begin()) ? -1.0 : 1.0;
		float p0y = (it->begin()+1)*delta;
		float p1y = (it->end()+1)*delta;
		
		float px0 = (index+beginIndexOffset)*0.055+0.0275;
		float px1 = (index+endIndexOffset)*0.055+0.0275;
		anim->addArrow(*it, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
		++index;
	}
}

void ArrowBox::Renderer::permuteTracks(const Permutation& permutation, AnimateMoveTrackLineArrowBox* animArrowTracks, AnimateMoveArrowInArrowBoxCommand* animMoveArrow) {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	int index = 0;
	for (auto it=m_arrow.begin(); it!=m_arrow.end(); ++it) {
		float t0y0 = (it->end() > it->begin()) ? -1.0 : 1.0;
		float p0y0 = (it->begin()+1)*delta;
		float p1y0 = (it->end()+1)*delta;
		
		it->setBeginEnd(permutation[it->begin()], permutation[it->end()]);
		float t0y1 = (it->end() > it->begin()) ? -1.0 : 1.0;
		float p0y1 = (it->begin()+1)*delta;
		float p1y1 = (it->end()+1)*delta;
		
		float px = index*0.055+0.0275;
		animMoveArrow->addArrow(*it, px, p0y0, px, p1y0, 0.0, t0y0, 0.0, -t0y0, px, p0y1, px, p1y1, 0.0, t0y1, 0.0, -t0y1);
		index++;
	}
	
	for (size_t i=0; i<m_tracks.size(); i++) {
		float y0 = (1.0*(i+1))/(m_oneHandle.numberOfTracks()+1);
		float y1 = (1.0*(permutation[i]+1))/(m_oneHandle.numberOfTracks()+1);
		animArrowTracks->addTrack(m_tracks[i], 0.0, y0, m_lenght, y0, 0.0, y1, m_lenght, y1);
	}
}

void ArrowBox::Renderer::doMoveArrow(std::list<Arrow::Renderer>::iterator it, std::list<Arrow::Renderer>::iterator it1,
		int index, int n, float start, float end, ArrowBox::Renderer& src, AnimateMoveArrowInArrowBoxCommand* anim, bool sameBox, bool singleArrow) {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	if (anim!=nullptr) {
		float t0y = (it->end() > it->begin()) ? -1.0 : 1.0;
		float p0y = (it->begin()+1)*delta;
		float p1y = (it->end()+1)*delta;
		
		float px0 = start*0.055+0.0275;
		float px1 = end*0.055+0.0275;
		anim->addArrow(*it, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
	}
	
	if (!singleArrow) {
		if (n>=0) {
			if (sameBox)
				it1++;
			else
				assert(it1==m_arrow.begin());
			
			while (n>0) {
				assert(it1!=m_arrow.end());
				
				index++;
				if (anim!=nullptr) {
					float t0y = (it1->end() > it1->begin()) ? -1.0 : 1.0;
					float p0y = (it1->begin()+1)*delta;
					float p1y = (it1->end()+1)*delta;
					
					float px0 = index*0.055+0.0275;
					float px1 = index*0.055-0.0275;
					anim->addArrow(*it1, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
				}
				it1++;
				n--;
			}
			m_arrow.splice(it1, src.m_arrow, it);
		} else {
			while (n<0) {
				assert(it1!=m_arrow.begin());
				it1--;
				index--;
				if (anim!=nullptr) {
					float t0y = (it1->end() > it1->begin()) ? -1.0 : 1.0;
					float p0y = (it1->begin()+1)*delta;
					float p1y = (it1->end()+1)*delta;
					
					float px0 = index*0.055+0.0275;
					float px1 = index*0.055+0.0825;
					anim->addArrow(*it1, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
				}
				n++;
			}
			m_arrow.splice(it1, src.m_arrow, it);
		}
	} else {
		assert(n==0);
		m_arrow.splice(it1, src.m_arrow, it);
	}
}

void ArrowBox::Renderer::spawnArrowAfterCrossingForward(RendererGL& rendererGL, ArrowBox::ArrowRendererInList it1, ArrowInArrowBox& newArrow, int index,
				unsigned int from, unsigned int to, AnimateMoveArrowInArrowBoxCommand* anim0,
				AnimateMoveArrowInArrowBoxCommand* anim1, UpdateArrowBoxLenghtCommand* lenAnim1, bool spawnAfter) {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	if (lenAnim1!=nullptr) {
		lenAnim1->setVariation(m_arrow.size()*0.055, m_arrow.size()*0.055+0.055);
	}
	
	auto it = it1;
	it--;
	if (anim0!=nullptr) {
		float t0y = (it->end() > it->begin()) ? -1.0 : 1.0;
		float p0y = (it->begin()+1)*delta;
		float p1y = (it->end()+1)*delta;
		
		float px0 = index*0.055-0.0275;
		float px1 = index*0.055;
		anim0->addArrow(*it, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
		
		t0y = (it1->end() > it1->begin()) ? -1.0 : 1.0;
		p0y = (it1->begin()+1)*delta;
		p1y = (it1->end()+1)*delta;
		
		px0 = index*0.055+0.0275;
		px1 = index*0.055;
		anim0->addArrow(*it1, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
	}
	
	if (anim1!=nullptr) {
		float t0y = (it1->end() > it1->begin()) ? -1.0 : 1.0;
		float p0y = (it1->begin()+1)*delta;
		float p1y = (it1->end()+1)*delta;
		
		float px0 = index*0.055;
		float px1 = index*0.055-0.0275;
		anim1->addArrow(*it1, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);	
	}
	
	m_arrow.splice(it, m_arrow, it1);
	if (!spawnAfter) {
		it1 = m_arrow.emplace(it, rendererGL, newArrow.getArrow(), 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, m_layer, from, to);
		newArrow.setArrowRendererInList(it1);
		
		if (anim1!=nullptr) {
			float t0y = (it->end() > it->begin()) ? -1.0 : 1.0;
			float p0y = (it->begin()+1)*delta;
			float p1y = (it->end()+1)*delta;
			
			float px0 = index*0.055;
			float px1 = (index+1)*0.055+0.0275;
			anim1->addArrow(*it, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
			
			t0y = (it1->end() > it1->begin()) ? -1.0 : 1.0;
			p0y = (it1->begin()+1)*delta;
			p1y = (it1->end()+1)*delta;
			
			px0 = index*0.055;
			px1 = index*0.055+0.0275;
			anim1->addArrow(*it1, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
		}
		
		it1 = it;
	} else {
		it1 = it;
		it1++;
		it1 = m_arrow.emplace(it1, rendererGL, newArrow.getArrow(), 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, m_layer, from, to);
		newArrow.setArrowRendererInList(it1);
		
		if (anim1!=nullptr) {
			float t0y = (it->end() > it->begin()) ? -1.0 : 1.0;
			float p0y = (it->begin()+1)*delta;
			float p1y = (it->end()+1)*delta;
			
			float px0 = index*0.055;
			float px1 = index*0.055+0.0275;
			anim1->addArrow(*it, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
			
			t0y = (it1->end() > it1->begin()) ? -1.0 : 1.0;
			p0y = (it1->begin()+1)*delta;
			p1y = (it1->end()+1)*delta;
			
			px0 = index*0.055;
			px1 = (index+1)*0.055+0.0275;
			anim1->addArrow(*it1, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
		}
	}
	
	if (anim1!=nullptr) {
		++index;
		while (++it1 != m_arrow.end()) {
			++index;
			float t0y = (it1->end() > it1->begin()) ? -1.0 : 1.0;
			float p0y = (it1->begin()+1)*delta;
			float p1y = (it1->end()+1)*delta;
			
			float px0 = index*0.055-0.0275;
			float px1 = index*0.055+0.0275;
			anim1->addArrow(*it1, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
		}
	}
}

void ArrowBox::Renderer::spawnArrowAfterCrossingBackward(RendererGL& rendererGL, ArrowBox::ArrowRendererInList it1, ArrowInArrowBox& newArrow, int index,
				unsigned int from, unsigned int to, AnimateMoveArrowInArrowBoxCommand* anim0,
				AnimateMoveArrowInArrowBoxCommand* anim1, UpdateArrowBoxLenghtCommand* lenAnim1, bool spawnAfter) {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	if (lenAnim1!=nullptr) {
		lenAnim1->setVariation(m_arrow.size()*0.055, m_arrow.size()*0.055+0.055);
	}
	
	auto it = it1;
	it++;
	if (anim0!=nullptr) {
		float t0y = (it->end() > it->begin()) ? -1.0 : 1.0;
		float p0y = (it->begin()+1)*delta;
		float p1y = (it->end()+1)*delta;
		
		float px0 = (index+1)*0.055+0.0275;
		float px1 = (index+1)*0.055;
		anim0->addArrow(*it, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
		
		t0y = (it1->end() > it1->begin()) ? -1.0 : 1.0;
		p0y = (it1->begin()+1)*delta;
		p1y = (it1->end()+1)*delta;
		
		px0 = index*0.055+0.0275;
		px1 = (index+1)*0.055;
		anim0->addArrow(*it1, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
	}
	
	if (anim1!=nullptr) {
		float t0y = (it1->end() > it1->begin()) ? -1.0 : 1.0;
		float p0y = (it1->begin()+1)*delta;
		float p1y = (it1->end()+1)*delta;
		
		float px0 = (index+1)*0.055;
		float px1 = (index+2)*0.055+0.0275;
		anim1->addArrow(*it1, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);	
	}
	
	m_arrow.splice(it1, m_arrow, it);
	if (!spawnAfter) {
		it1 = m_arrow.emplace(it1, rendererGL, newArrow.getArrow(), 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, m_layer, from, to);
		newArrow.setArrowRendererInList(it1);
		
		if (anim1!=nullptr) {
			float t0y = (it->end() > it->begin()) ? -1.0 : 1.0;
			float p0y = (it->begin()+1)*delta;
			float p1y = (it->end()+1)*delta;
			
			float px0 = (index+1)*0.055;
			float px1 = index*0.055+0.0275;
			anim1->addArrow(*it, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
			
			t0y = (it1->end() > it1->begin()) ? -1.0 : 1.0;
			p0y = (it1->begin()+1)*delta;
			p1y = (it1->end()+1)*delta;
			
			px0 = (index+1)*0.055;
			px1 = (index+1)*0.055+0.0275;
			anim1->addArrow(*it1, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
		}
		
		it1++;
	} else {
		it1 = m_arrow.emplace(it, rendererGL, newArrow.getArrow(), 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, 1e+10, m_layer, from, to);
		newArrow.setArrowRendererInList(it1);
		
		if (anim1!=nullptr) {
			float t0y = (it->end() > it->begin()) ? -1.0 : 1.0;
			float p0y = (it->begin()+1)*delta;
			float p1y = (it->end()+1)*delta;
			
			float px0 = (index+1)*0.055;
			float px1 = (index+1)*0.055+0.0275;
			anim1->addArrow(*it, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
			
			t0y = (it1->end() > it1->begin()) ? -1.0 : 1.0;
			p0y = (it1->begin()+1)*delta;
			p1y = (it1->end()+1)*delta;
			
			px0 = (index+1)*0.055;
			px1 = index*0.055+0.0275;
			anim1->addArrow(*it1, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
		}
		it1 = ++it;
	}
	
	if (anim1!=nullptr) {
		++index;
		while (++it1 != m_arrow.end()) {
			++index;
			float t0y = (it1->end() > it1->begin()) ? -1.0 : 1.0;
			float p0y = (it1->begin()+1)*delta;
			float p1y = (it1->end()+1)*delta;
			
			float px0 = index*0.055+0.0275;
			float px1 = (index+1)*0.055+0.0275;
			anim1->addArrow(*it1, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
		}
	}
}

void ArrowBox::Renderer::mergeArrows(RendererGL& rendererGL, ArrowBox::ArrowRendererInList it, int index,
		AnimateMoveArrowInArrowBoxCommand* anim0, UpdateArrowBoxLenghtCommand* lenAnim0,
		AnimateMoveArrowInArrowBoxCommand* anim1, UpdateArrowBoxLenghtCommand* lenAnim1, bool after)
{
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	if (lenAnim0!=nullptr) {
		lenAnim0->setVariation(m_arrow.size()*0.055, m_arrow.size()*0.055-0.0275);
	}
	if (lenAnim1!=nullptr) {
		lenAnim1->setVariation(m_arrow.size()*0.055-0.0275, (m_arrow.size()-2)*0.055);
	}
	
	std::list<Arrow::Renderer>::iterator it0;
	if (!after) { 
		it0 = it;
		it0++;
		index++;
	} else {
		it0 = it;
		it--;
	}
	
	if (anim0!=nullptr) {
		float t0y = (it->end() > it->begin()) ? -1.0 : 1.0;
		float p0y = (it->begin()+1)*delta;
		float p1y = (it->end()+1)*delta;
		
		float px0 = index*0.055-0.0275;
		float px1 = index*0.055;
		anim0->addArrow(*it, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
		
		t0y = (it0->end() > it0->begin()) ? -1.0 : 1.0;
		p0y = (it0->begin()+1)*delta;
		p1y = (it0->end()+1)*delta;
		
		px0 = index*0.055+0.0275;
		px1 = index*0.055;
		anim0->addArrow(*it0, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
	}
	
	if (anim1!=nullptr || anim0!=nullptr) {
		while (++it0 != m_arrow.end()) {
			++index;
			float t0y = (it0->end() > it0->begin()) ? -1.0 : 1.0;
			float p0y = (it0->begin()+1)*delta;
			float p1y = (it0->end()+1)*delta;
			
			if (anim1!=nullptr) {
				float px0 = index*0.055;
				float px1 = (index-1)*0.055-0.0275;
				anim1->addArrow(*it0, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
			}
			if (anim0!=nullptr) {
				float px0 = index*0.055+0.0275;
				float px1 = index*0.055;
				anim0->addArrow(*it0, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
			}
		}
	}
}

void ArrowBox::Renderer::removeArrow(RendererGL& rendererGL, ArrowBox::ArrowRendererInList it, int index,
		AnimateMoveArrowInArrowBoxCommand* anim0, UpdateArrowBoxLenghtCommand* lenAnim0) {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	if (lenAnim0!=nullptr) {
		lenAnim0->setVariation(m_arrow.size()*0.055, m_arrow.size()*0.055-0.055);
	}
	
	if (anim0!=nullptr) {
		while (++it != m_arrow.end()) {
			++index;
			float t0y = (it->end() > it->begin()) ? -1.0 : 1.0;
			float p0y = (it->begin()+1)*delta;
			float p1y = (it->end()+1)*delta;
			
			if (anim0!=nullptr) {
				float px0 = index*0.055+0.0275;
				float px1 = index*0.055-0.0275;
				anim0->addArrow(*it, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
			}
		}
	}
}

void ArrowBox::Renderer::genCrossing(RendererGL& rendererGL, ArrowBox::ArrowRendererInList arrow, int index, AnimateMoveArrowInArrowBoxCommand* anim,
				GenCrossingCommand*& genAnim, std::vector<MoveCrossingCommand*>& crossingMoveCommands,
				FadeCrossingCommand*& fadeAnim, unsigned int i, unsigned int j) {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	ArrowBox::ArrowRendererInList& it(arrow);
	
	if (anim!=nullptr) {
		float t0y = (it->end() > it->begin()) ? -1.0 : 1.0;
		float p0y = (it->begin()+1)*delta;
		float p1y = (it->end()+1)*delta;
		
		float px0 = index*0.055+0.0275;
		float px1 = index*0.055-0.0275;
		anim->addArrow(*it, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
	}
	createCrossing(rendererGL, i, j, index, --arrow, m_layer);
	genAnim = GenCrossingCommand::create(*this, index*0.055-0.0275, index*0.055+0.0825, (i+1)*delta, (j+1)*delta);
	++arrow;
	
	while (++arrow!=m_arrow.end()) {
		Arrow::Renderer* arr = &(*arrow);
		float t0y = (it->end() > it->begin()) ? -1.0 : 1.0;
		float y0 = (it->begin()+1)*delta;
		float y1 = (it->end()+1)*delta;
		
		float crossingStart    = index*0.055-0.0275;
		float crossingEnd      = index*0.055+0.0825;
		float crossingNewStart = (index+1)*0.055-0.0275;
		ArrowBox::ArrowRendererInList nextArrow(arrow); ++nextArrow;
		float crossingNewEnd   = (nextArrow==m_arrow.end()) ? (index+1)*0.055+0.055 : (index+1)*0.055+0.0825;
		float yI = (m_crossingI+1)*delta;
		float yJ = (m_crossingJ+1)*delta;
		
		if (arrow->begin() == m_crossingI && arrow->end() == m_crossingJ) {
			arrow->setBeginEnd(arrow->end(), arrow->begin());
			crossingMoveCommands.push_back(MoveCrossingCommandInvertArrow::create(*this, crossingStart, crossingEnd, crossingNewStart, crossingNewEnd,
					yI, yJ, arr, false));
		} else if (arrow->begin() == m_crossingJ && arrow->end() == m_crossingI) {
			arrow->setBeginEnd(arrow->end(), arrow->begin());
			crossingMoveCommands.push_back(MoveCrossingCommandInvertArrow::create(*this, crossingStart, crossingEnd, crossingNewStart, crossingNewEnd,
					yI, yJ, arr, true));
		} else if (arrow->begin() == m_crossingI) {
			arrow->setBeginEnd(m_crossingJ, arrow->end());
			crossingMoveCommands.push_back(MoveCrossingCommandSlideArrow::create(*this, crossingStart, crossingEnd, crossingNewStart, crossingNewEnd,
					yI, yJ, arr, y1, t0y, true, false));
		} else if (arrow->begin() == m_crossingJ) {
			arrow->setBeginEnd(m_crossingI, arrow->end());
			crossingMoveCommands.push_back(MoveCrossingCommandSlideArrow::create(*this, crossingStart, crossingEnd, crossingNewStart, crossingNewEnd,
					yI, yJ, arr, y1, t0y, true, true));
		} else if (arrow->end() == m_crossingI) {
			arrow->setBeginEnd(arrow->begin(), m_crossingJ);
			crossingMoveCommands.push_back(MoveCrossingCommandSlideArrow::create(*this, crossingStart, crossingEnd, crossingNewStart, crossingNewEnd,
					yI, yJ, arr, y0, t0y, false, false));
		} else if (arrow->end() == m_crossingJ) {
			arrow->setBeginEnd(arrow->begin(), m_crossingI);
			crossingMoveCommands.push_back(MoveCrossingCommandSlideArrow::create(*this, crossingStart, crossingEnd, crossingNewStart, crossingNewEnd,
					yI, yJ, arr, y0, t0y, false, true));
		} else {
			crossingMoveCommands.push_back(MoveCrossingCommandMoveArrow::create(*this, crossingStart, crossingEnd, crossingNewStart, crossingNewEnd,
					yI, yJ, arr, crossingEnd, crossingNewStart, y0, y1, t0y));
		}
		
		index++;
	}
	
	fadeAnim = FadeCrossingCommand::create(*this, index*0.055-0.0275, index*0.055+0.055, index*0.055, (m_crossingI+1)*delta, (m_crossingJ+1)*delta);
}

void ArrowBox::Renderer::transferArrowsAsPermutationBoxSlide(ArrowBox::Renderer& target, const PermutationBox::Renderer& permutation,
		AsynchronousSubTaskCommand* cmd, float duration) {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	const float dt = duration/m_arrow.size();
	const float perTravelDuration = (permutation.lenght()/0.055)*dt;
	
	int k=0;
	auto it = m_arrow.begin();
	while (it!=m_arrow.end()) {
		float begin = dt/2 + k*dt;
		unsigned int i = permutation.pre(it->begin());
		unsigned int j = permutation.pre(it->end());
		AnimateMoveArrowPermutationBox* anim0 = AnimateMoveArrowPermutationBox::create(permutation, *it, i, j, 1.0, 0.0);
		AnimateMoveArrowInArrowBoxCommand* anim1 = AnimateMoveArrowInArrowBoxCommand::create(*this, true);
		
		float t0y = (j > i) ? -1.0 : 1.0;
		float p0y = (i+1)*delta;
		float p1y = (j+1)*delta;
		
		float px0 = std::min(k*0.055f+0.0275f, m_lenght-permutation.lenght());
		float px1 = k*0.055+0.0275-permutation.lenght();
		anim1->addArrow(*it, px0, p0y, px0, p1y, 0.0, t0y, 0.0, -t0y, px1, p0y, px1, p1y, 0.0, t0y, 0.0, -t0y);
		cmd->addCommand(anim0, begin, begin + perTravelDuration);
		cmd->addCommand(anim1, begin + perTravelDuration, begin + perTravelDuration + ((px0-px1)/0.055)*dt);
		it = transferArrowToBack(target, it, i, j);
		k++;
	}
}

void ArrowBox::Renderer::refreshArrows() {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	int i=0;
	for(auto it1=m_arrow.begin(); it1!=m_arrow.end(); it1++, i++) {
		float px  = m_basex+i*0.055+0.0275;
		float p0y = m_basey - (it1->begin()+1)*delta;
		float p1y = m_basey - (it1->end()+1)*delta;
		float t0y = (it1->end() > it1->begin()) ? -1.0 : 1.0;
		it1->changePoints(px, p0y, px, p1y, 0.0, t0y, 0.0, -t0y);
	}
}

void ArrowBox::Renderer::refreshLenght() {
	setLenght(m_arrow.size()*0.055, true);
}

void ArrowBox::Renderer::refreshTracks() {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	for (unsigned int i=0; i<m_tracks.size(); i++) {
		m_tracks[i].changePoints(m_basex, m_basey-(1.0*(i+1))*delta,m_basex+m_lenght, m_basey-(1.0*(i+1))*delta);
	}
}

std::pair<ArrowBox::ArrowRendererInList, int> ArrowBox::Renderer::pushBackArrow(RendererGL& rendererGL, ArrowInArrowBox& newArrow,
		unsigned int from, unsigned int to, bool refresh) {
	if (refresh) {
		const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
		float px  = m_basex+m_arrow.size()*0.055+0.0275;
		float p0y = m_basey - (from+1)*delta;
		float p1y = m_basey - (to+1)*delta;
		float t0y = (to > from) ? -1.0 : 1.0;
		m_arrow.emplace_back(rendererGL, newArrow.getArrow(), px, p0y, px, p1y, 0.0, t0y, 0.0, -t0y, m_layer, from, to);
		setLenght(m_arrow.size()*0.055, true);
	} else
		m_arrow.emplace_back(rendererGL, newArrow.getArrow(), 1e+10, 1e+10, 1e+10, 1e+10, 0.0, 0.0, 0.0, 0.0, m_layer, from, to);
	
	newArrow.setArrowRendererInList(--m_arrow.end());
	return std::make_pair(--m_arrow.end(), m_arrow.size()-1);
}

std::pair<ArrowBox::ArrowRendererInList, int> ArrowBox::Renderer::pushFrontArrow(RendererGL& rendererGL, ArrowInArrowBox& newArrow,
		unsigned int from, unsigned int to, bool refresh) {
	if (refresh) {
		const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
		float px  =m_basex+m_arrow.size()*0.055+0.0275;
		float p0y = m_basey - (from+1)*delta;
		float p1y = m_basey - (to+1)*delta;
		float t0y = (to > from) ? -1.0 : 1.0;
		m_arrow.emplace_front(rendererGL, newArrow.getArrow(), px, p0y, px, p1y, 0.0, t0y, 0.0, -t0y, m_layer, from, to);
		setLenght(m_arrow.size()*0.055, true);
	} else
		m_arrow.emplace_front(rendererGL, newArrow.getArrow(), 1e+10, 1e+10, 1e+10, 1e+10, 0.0, 0.0, 0.0, 0.0, m_layer, from, to);
	
	newArrow.setArrowRendererInList(m_arrow.begin());
	return std::make_pair(m_arrow.begin(), 0);
}

ArrowBox::ArrowRendererInList ArrowBox::Renderer::transferArrowToBack(Renderer& target, ArrowRendererInList arrow, unsigned int targetI, unsigned int targetJ) {
	ArrowRendererInList ret(arrow); ++ret;
	arrow->setBeginEnd(targetI, targetJ);
	target.m_arrow.splice(target.m_arrow.end(), m_arrow, arrow);
	return ret;
}

void ArrowBox::Renderer::setLenght(float newLenght, bool updateArrows) {
	m_lenght = newLenght;
	m_oneHandle.updateLenght(updateArrows);
}

float ArrowBox::Renderer::getRelativeYForTrack(unsigned int index) const {
	return (1.0*(index+1))/(m_oneHandle.numberOfTracks()+1);
}

std::pair<float, float> ArrowBox::Renderer::getCrossingTracksHeight() const {
	assert(m_crossing!=nullptr);
	return std::make_pair(getRelativeYForTrack(m_crossingI), getRelativeYForTrack(m_crossingJ));
}

void ArrowBox::Renderer::createCrossing(RendererGL& rendererGL, unsigned int i, unsigned int j, size_t crossingIndex, ArrowRendererInList prevArrow, int layer) {
	assert(m_crossing==nullptr);
	m_crossing = new Crossing(rendererGL, layer);
	m_crossingI = i; m_crossingJ = j;
	m_crossingIndex = crossingIndex;
	m_crossingPrevArrow = prevArrow;
}

ArrowBox::ArrowInArrowBoxIndexedIterator ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(ArrowBox& arrowBox) {
	return ArrowBox::ArrowInArrowBoxIndexedIterator(arrowBox.m_arrows.begin(), 0);
}

ArrowBox::ArrowInArrowBoxIndexedIterator ArrowBox::ArrowInArrowBoxIndexedIterator::fromEndOfBox(ArrowBox& arrowBox) {
	return ArrowBox::ArrowInArrowBoxIndexedIterator(arrowBox.m_arrows.end(), arrowBox.m_arrows.size());
}

ArrowBox::ArrowInArrowBoxIndexedIterator& ArrowBox::ArrowInArrowBoxIndexedIterator::operator++() {
	++m_it;
	++m_index;
	return *this;
}

ArrowBox::ArrowInArrowBoxIndexedIterator& ArrowBox::ArrowInArrowBoxIndexedIterator::operator--() {
	assert(m_index);
	--m_it;
	--m_index;
	return *this;
}

bool ArrowBox::ArrowInArrowBoxIndexedIterator::operator==(const ArrowBox::ArrowInArrowBoxIndexedIterator& other) const {
	bool ret = m_it==other.m_it;
	if (ret) assert(m_index==other.m_index);
	return ret;
}

ArrowBox::ArrowInArrowBoxIndexedIterator ArrowBox::ArrowInArrowBoxIndexedIterator::eraseFromArrowBox(ArrowBox& arrowBox) {
	return ArrowInArrowBoxIndexedIterator(arrowBox.m_arrows.erase(m_it), m_index);
}

ArrowBox::ArrowInArrowBoxIndexedIterator ArrowBox::ArrowInArrowBoxIndexedIterator::insertInArrowBox(ArrowBox::ArrowInArrowBox* arrow, ArrowBox& arrowBox) {
	auto it0 = arrowBox.m_arrows.insert(m_it, arrow);
	return ArrowBox::ArrowInArrowBoxIndexedIterator(it0, m_index++);
}

void ArrowBox::ArrowInArrowBoxIndexedIterator::transfer(ArrowBox::ArrowInArrowBoxIndexedIterator& other, ArrowBox& arrowBox) {
	if (*this==other) {assert(false);}
	arrowBox.m_arrows.splice(other.m_it, arrowBox.m_arrows, m_it);
	assert(other.m_index > m_index);
	m_index = other.m_index-1;
}

void ArrowBox::removeDepthMArrows(const OneHandle& oneHandle, const ZeroHandle& zeroHandle, int m, bool to) {
	assert(m>0);
	
	ArrowInArrowBoxIndexedIterator it(ArrowInArrowBoxIndexedIterator::fromBeginOfBox(*this));
	while (it!=ArrowInArrowBoxIndexedIterator::fromEndOfBox(*this)) {
		const Arrow& arrow = (*it)->getArrow();
		int depth = oneHandle.getArrowDepth(*this, zeroHandle, std::make_pair(arrow.begin(), arrow.end()), to);
		assert(std::abs(depth)>=m && depth!=(-m));
		
		if (depth==m)
			it = removeArrow(it);
		else
			++it;
	}
}

unsigned int ArrowBox::getMinimalDepth(const OneHandle& oneHandle, const ZeroHandle& zeroHandle) const {
	unsigned int depth = std::numeric_limits<unsigned int>::max();
	for (auto it=m_arrows.begin(); it!=m_arrows.end(); ++it) {
		const Arrow& arrow = (*it)->getArrow();
		int depthTo   = oneHandle.getArrowDepth(*this, zeroHandle, std::make_pair(arrow.begin(), arrow.end()), true);
		int depthFrom = oneHandle.getArrowDepth(*this, zeroHandle, std::make_pair(arrow.begin(), arrow.end()), false);
		
		if (depthTo!=0) {
			assert(depthFrom!=0);
			depth = std::min(depth, static_cast<unsigned int>(std::abs(depthTo)));
			depth = std::min(depth, static_cast<unsigned int>(std::abs(depthFrom)));
		} else {
			assert(depthFrom==0);
		}
	}
	
	return depth;
}

ArrowBox::ArrowInArrowBoxIndexedIterator ArrowBox::removeArrow(ArrowInArrowBoxIndexedIterator arrow) {
	postRemoveArrowFromArrowBoxCommand(*this, makeArrowInArrowBoxIndexed(arrow));
	return arrow.eraseFromArrowBox(*this);
}

void ArrowBox::lemma29MovePastArrow(ArrowInArrowBox& movingArrow, unsigned int& movingIndex, ArrowInArrowBoxIndexedIterator& arrow,
		ArrowInArrowBoxIndexedIterator* last, ArrowInArrowBoxIndexedIterator& end) {
	unsigned int moving_begin = movingArrow.getArrow().begin();
	unsigned int moving_end   = movingArrow.getArrow().end();
	unsigned int arrow_begin = (*arrow)->getArrow().begin();
	unsigned int arrow_end   = (*arrow)->getArrow().end();
	
	unsigned int create_begin;
	unsigned int create_end;
	bool shouldCreateArrow = false;
	
	if (moving_end==arrow_begin) {
		shouldCreateArrow = true;
		create_begin = moving_begin;
		create_end   = arrow_end;
	} else if (arrow_end==moving_begin) {
		shouldCreateArrow = true;
		create_begin = arrow_begin;
		create_end   = moving_end;
	}
	
	if (shouldCreateArrow) {
		ArrowInArrowBoxIndexedIterator* newLast = (last==nullptr) ? nullptr : (*last>arrow) ? last : nullptr;
		ArrowInArrowBoxIndexedIterator it1(arrow);
		++it1;
		it1 = it1.insertInArrowBox(new ArrowInArrowBox(Arrow(create_begin, create_end), ArrowRendererInList()), *this);
		postGenArrowAfterMoveCrossing(*this, makeArrowInArrowBoxIndexed(movingArrow, movingIndex), makeArrowInArrowBoxIndexed(arrow), **it1,
				create_begin, create_end, true);
		
		movingIndex = arrow.getPos();
		arrow = it1;
		if (newLast!=nullptr) newLast->incIndex();
		end.incIndex();
		doLemma29(arrow, ++it1, 0, false, false, end, newLast);
		if (newLast!=nullptr) {
			newLast->decIndex();
			lemma29RemoveSameArrows(++(*newLast), end);
			--(*newLast);
		} else {
			lemma29RemoveSameArrows(arrow, end);
		}
		--arrow;
	}
}

void ArrowBox::doLemma29(ArrowInArrowBoxIndexedIterator& begin, ArrowInArrowBoxIndexedIterator checkStart,
					unsigned int k, bool shouldGoDeeper, bool strictK, ArrowInArrowBoxIndexedIterator& end, ArrowInArrowBoxIndexedIterator* last) {
	if (k>=m_numberOfTracks) return;
	
	ArrowInArrowBoxIndexedIterator& it(checkStart);
	ArrowInArrowBoxIndexedIterator candidate;
	unsigned int max_lenght = 0;
	
#ifndef NDEBUG
	bool isUp = (*begin)->getArrow().isUp();
#endif
	
	do {
		--it;
		assert((*it)->getArrow().isUp()==isUp);
		
		unsigned int lenght = (*it)->getArrow().lenght();
		if (lenght > max_lenght && ((strictK) ? lenght==m_numberOfTracks-k : lenght <= m_numberOfTracks-k)) {
			max_lenght = lenght;
			candidate = it;
			if (max_lenght==m_numberOfTracks-k) break;
		}
	} while (it!=begin);
	
	if (max_lenght) {  // Si on a trouver une fleche a bouger
		k = m_numberOfTracks-max_lenght;
		it = candidate;
		
		ArrowInArrowBox& candidateArrow = **candidate;
		unsigned int candidateIndex = candidate.getPos();
		while (++it != end && (*it)->getArrow().lenght()<max_lenght) {
			lemma29MovePastArrow(candidateArrow, candidateIndex, it, last, end);
		}
		
		ArrowInArrowBoxIndexedIterator newCheckBegin(candidate);
		if (it==end || candidateArrow.getArrow().begin()!=(*it)->getArrow().begin() || 
				candidateArrow.getArrow().end()!=(*it)->getArrow().end()) {
			postMoveArrowInArrowBox(*this, makeArrowInArrowBoxIndexed(candidateArrow, candidateIndex), makeArrowInArrowBoxIndexed(--it));
			it++;
			
			if (begin==candidate) { // Si candidate==begin, begin doit pointer a la fleche suivante
				++begin;
				if (begin!=it) begin.decIndex();
				else --begin;
			}
			
			// candidate==newCheckBegin, donc newCheckBegin doit pointer a la fleche suivante
			++newCheckBegin;
			if (newCheckBegin!=it) newCheckBegin.decIndex();
			else --newCheckBegin;
			
			candidate.transfer(it, *this);
		} else {
			postMoveMergeArrowsCommand(*this, makeArrowInArrowBoxIndexed(candidateArrow, candidateIndex), makeArrowInArrowBoxIndexed(it));
			
			if (begin==candidate) {
				++begin;
				if (begin==it) {
					++begin;
					begin.decIndex();
				}
				begin.decIndex();
			}
			
			++newCheckBegin;
			if (newCheckBegin==it) {
				newCheckBegin++;
				newCheckBegin.decIndex();
			}
			newCheckBegin.decIndex();
			
			it.eraseFromArrowBox(*this);
			candidate.eraseFromArrowBox(*this);
			end.decIndex(); end.decIndex();
		}
		
		if (shouldGoDeeper) {
			if (newCheckBegin!=begin) {
				doLemma29(begin, newCheckBegin, k, true, true, end);
			} else {
				doLemma29(begin, end, k+1, true, false, end);
			}
		}
	} else if (strictK && shouldGoDeeper) {
		doLemma29(begin,end, k+1, true, false, end);
	}
}

void ArrowBox::lemma29RemoveSameArrows(ArrowInArrowBoxIndexedIterator& begin, ArrowInArrowBoxIndexedIterator& end) {
	if (begin==end) return;
	
	ArrowInArrowBoxIndexedIterator itTarget(begin);
	unsigned int beginLenght = (*begin)->getArrow().lenght();
	
	while (++itTarget!=end && (*itTarget)->getArrow().lenght()==beginLenght);
	ArrowInArrowBoxIndexedIterator last(itTarget); --last;
	
	while (--itTarget!=begin) {
		unsigned int itTargetBegin = (*itTarget)->getArrow().begin();
		unsigned int itTargetEnd   = (*itTarget)->getArrow().end();
		ArrowBox::ArrowInArrowBoxIndexedIterator it(itTarget);
		
		do {
			--it;
			if ((*it)->getArrow().begin()==itTargetBegin && (*it)->getArrow().end()==itTargetEnd) {
				ArrowInArrowBoxIndexedIterator it0(it);
				ArrowInArrowBox& itArrow = **it;
				unsigned int itIndex = it.getPos();
				
				while (++it0 != itTarget) {
					lemma29MovePastArrow(itArrow, itIndex, it0, &last, end);
				}
				
				postMoveMergeArrowsCommand(*this, makeArrowInArrowBoxIndexed(itArrow, itIndex), makeArrowInArrowBoxIndexed(itTarget));
				if (it==begin) {
					++begin;
					if (begin==itTarget) {
						++begin;
						begin.decIndex();
					}
					begin.decIndex();
				}
				
				++last;
				it0 = itTarget; ++it0; it0.decIndex(); it0.decIndex();
				itTarget.eraseFromArrowBox(*this);
				it.eraseFromArrowBox(*this);
				itTarget = it0;
				last.decIndex(); last.decIndex();
				end.decIndex(); end.decIndex();
				
				if (begin==itTarget) {
					if (begin==last)
						lemma29RemoveSameArrows(begin, end);
					else
						lemma29RemoveSameArrows(last, end);
					
					return;
				} else {
					--last;
					break;
				}
			}
		} while (it!=begin);
	}
	
	lemma29RemoveSameArrows(++last, end);
}

void ArrowBox::lemma29(ArrowInArrowBoxIndexedIterator& begin, ArrowInArrowBoxIndexedIterator& end, ArrowInArrowBoxIndexedIterator* checkStart) {
	if (begin!=end) {
		if (checkStart==nullptr)
			doLemma29(begin, end, 0, true, false, end);
		else
			doLemma29(begin, *checkStart, 0, true, false, end);
		
		// On verifie si le lemme a fonctionne
#ifndef NDEBUG
		ArrowInArrowBoxIndexedIterator it(begin);
		bool isUp = (*it)->getArrow().isUp();
		unsigned int lenght = (*it)->getArrow().lenght();
		while (++it != end) {
			assert((*it)->getArrow().isUp() == isUp);
			assert((*it)->getArrow().lenght() >= lenght);
			lenght = (*it)->getArrow().lenght();
		}
#endif
		
		lemma29RemoveSameArrows(begin, end);
		
		// On verifie si le lemme a fonctionne
#ifndef NDEBUG
		it = begin;
		isUp = (*it)->getArrow().isUp();
		lenght = (*it)->getArrow().lenght();
		while (++it != end) {
			assert((*it)->getArrow().isUp() == isUp);
			assert((*it)->getArrow().lenght() >= lenght);
			lenght = (*it)->getArrow().lenght();
		}
#endif
	}
}

void ArrowBox::permuteTracks(const Permutation& permutation) {
	for (auto it=m_arrows.begin(); it!=m_arrows.end(); ++it) {
		Arrow& arrow = (*it)->getArrow();
		arrow = Arrow(permutation[arrow.begin()], permutation[arrow.end()]);
	}
}

void ArrowBox::fillArrowsRendererLists(std::list<std::pair<unsigned int, unsigned int>>& arrowsData, std::list<ArrowInArrowBox*>& arrows) {
	arrows = m_arrows;
	for (auto it=m_arrows.begin(); it!=m_arrows.end(); it++) {
		arrowsData.push_back(std::make_pair((*it)->getArrow().begin(), (*it)->getArrow().end()));
	}
}

void ArrowBox::moveArrowsThroughoutZeroHandle(ZeroHandle& zeroHandle, OneHandle& oneHandle, bool fromEnd) {
	if (!m_arrows.empty()) {
		PassArrowsThroughtZeroHandleCommand* cmd = PassArrowsThroughtZeroHandleCommand::create(*this);
		
		if (!fromEnd) {
			int i=0;
			while (!m_arrows.empty()) {
				oneHandle.removeArrowToZeroHandle(zeroHandle, cmd, *this, m_arrows.begin(), i++);
			}
		} else {
			while (!m_arrows.empty()) {
				oneHandle.removeArrowToZeroHandle(zeroHandle, cmd, *this, --m_arrows.end(), m_arrows.size()-1);
			}
		}
		
		postPassArrowsThroughtZeroHandleCommand(cmd);
	}
}

void ArrowBox::removeArrowGenCrossing(ArrowInArrowBoxIndexedIterator arrow, ArrowInArrowBoxIndexedIterator& crossingPos, PermutationBox& permutationBox) {
	assert(crossingPos.getPos() > arrow.getPos());
	
	unsigned int begin = (*arrow)->getArrow().begin();
	unsigned int end   = (*arrow)->getArrow().end();
	
	assert((*crossingPos)->getArrow().end() == begin);
	assert((*crossingPos)->getArrow().begin() == end);
	
	arrow.eraseFromArrowBox(*this);
	ArrowInArrowBoxIndexedIterator it(crossingPos);
	it.decIndex();
	while (++it != ArrowInArrowBoxIndexedIterator::fromEndOfBox(*this)) {
		if (((*it)->getArrow().begin() == begin && (*it)->getArrow().end() == end) ||
				((*it)->getArrow().begin() == end && (*it)->getArrow().end() == begin))
			(*it)->getArrow() = Arrow((*it)->getArrow().end(), (*it)->getArrow().begin());
		else if ((*it)->getArrow().begin() == begin)
			(*it)->getArrow() = Arrow(end, (*it)->getArrow().end());
		else if ((*it)->getArrow().begin() == end)
			(*it)->getArrow() = Arrow(begin, (*it)->getArrow().end());
		else if ((*it)->getArrow().end() == begin)
			(*it)->getArrow() = Arrow((*it)->getArrow().begin(), end);
		else if ((*it)->getArrow().end() == end)
			(*it)->getArrow() = Arrow((*it)->getArrow().begin(), begin);
	}
	
	permutationBox.permute(std::make_pair(begin, end), false);
	
	postMoveArrowGenCrossingCommand(*this, makeArrowInArrowBoxIndexed(arrow), makeArrowInArrowBoxIndexed(crossingPos), begin, end);
	crossingPos.decIndex();
}

PermutationBox::Renderer::Renderer(RendererGL& rendererGL, float basex, float basey, int layer, OneHandleRenderer& oneHandle) :
	m_inv_per(Permutation::Identity(oneHandle.numberOfTracks())), m_basex(basex), m_basey(basey), m_layer(layer), m_oneHandle(oneHandle)
{
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	for (unsigned int i=0; i<m_oneHandle.numberOfTracks(); i++) {
		m_tracks.emplace_back(rendererGL, m_basex, m_basey-(1.0*(i+1))*delta, m_basex+lenght(),
							m_basey-(1.0*(i+1))*delta, trackColor.r, trackColor.g, trackColor.b, layer, EAST, WEST);
	}
}

void PermutationBox::Renderer::changeBasePoints(float basex, float basey) {
	m_basex = basex; m_basey = basey;
	refresh();
}

void PermutationBox::Renderer::permute(const BiPermutation& permutation, AnimateMoveTrackPermutationBox* animPermutation, bool isAfter) {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	Permutation new_inv_per(Permutation::Identity(m_inv_per.size()));
	
	if (isAfter) {
		new_inv_per = m_inv_per*permutation.getInversePermutation();
		
		if (animPermutation!=nullptr) {
			for (unsigned int j=0; j<m_tracks.size(); j++) {
				unsigned int i = m_inv_per[j];
				unsigned int k = permutation.post(j);
				float p1y1 = (1.0*(k+1))*delta;
				float p1y0 = (1.0*(j+1))*delta;
				float p0y  = (1.0*(i+1))*delta;
				
				animPermutation->addTrack(m_tracks[i], 0.0, p0y, lenght(), p1y0, 0.0, p0y, lenght(), p1y1, EAST, WEST);
			}
		}
	} else {
		new_inv_per = permutation.getInversePermutation()*m_inv_per;
		
		if (animPermutation!=nullptr) {
			for (unsigned int i=0; i<m_tracks.size(); i++) {
				unsigned int j = m_inv_per[i];
				unsigned int k = new_inv_per[i];
				float p0y1 = (1.0*(k+1))*delta;
				float p0y0 = (1.0*(j+1))*delta;
				float p1y  = (1.0*(i+1))*delta;
				
				animPermutation->addTrack(m_tracks[i], 0.0, p0y0, lenght(), p1y, 0.0, p0y1, lenght(), p1y, EAST, WEST);
			}
		}
	}
	m_inv_per = new_inv_per;
}

void PermutationBox::Renderer::permute(std::pair<unsigned int, unsigned int> permutation, AnimateMoveTrackPermutationBox* animPermutation, bool isAfter) {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	if (isAfter) {
		if (animPermutation!=nullptr) {
			unsigned int i = m_inv_per[permutation.first];
			unsigned int k = permutation.second;
			float p1y1 = (1.0*(k+1))*delta;
			float p1y0 = (1.0*(permutation.first+1))*delta;
			float p0y  = (1.0*(i+1))*delta;
			animPermutation->addTrack(m_tracks[i], 0.0, p0y, lenght(), p1y0, 0.0, p0y, lenght(), p1y1, EAST, WEST);
			
			i = m_inv_per[permutation.second];
			k = permutation.first;
			p1y1 = (1.0*(k+1))*delta;
			p1y0 = (1.0*(permutation.second+1))*delta;
			p0y  = (1.0*(i+1))*delta;
			animPermutation->addTrack(m_tracks[i], 0.0, p0y, lenght(), p1y0, 0.0, p0y, lenght(), p1y1, EAST, WEST);
		}
		
		m_inv_per *= permutation;
	} else {
		Permutation new_inv_per(Permutation::Identity(m_inv_per.size()));
		
		new_inv_per*=permutation;
		new_inv_per = new_inv_per*m_inv_per;
		
		if (animPermutation!=nullptr) {
			for (unsigned int i=0; i<m_tracks.size(); i++) {
				unsigned int j = m_inv_per[i];
				unsigned int k = new_inv_per[i];
				float p0y1 = (1.0*(k+1))*delta;
				float p0y0 = (1.0*(j+1))*delta;
				float p1y  = (1.0*(i+1))*delta;
				
				animPermutation->addTrack(m_tracks[i], 0.0, p0y0, lenght(), p1y, 0.0, p0y1, lenght(), p1y, EAST, WEST);
			}
		}
		
		m_inv_per = new_inv_per;
	}
}

void PermutationBox::Renderer::refresh() {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	
	for (unsigned int i=0; i<m_oneHandle.numberOfTracks(); i++) {
		unsigned int j = m_inv_per[i];
		m_tracks[j].changePoints(m_basex, m_basey-(1.0*(j+1))*delta, m_basex+lenght(), m_basey-(1.0*(i+1))*delta, EAST, WEST);
	}
}

Bezier PermutationBox::Renderer::getBezier(unsigned int left, unsigned int right) const {
	const float delta = 1.0/(m_oneHandle.numberOfTracks()+1);
	return TrackBezier::getBezier(m_basex, m_basey-(1.0*(left+1))*delta, m_basex+lenght(), m_basey-(1.0*(right+1))*delta, EAST, WEST);
}

void PermutationBox::permute(const BiPermutation& permutation, bool isAfter) {
	if (isAfter)
		m_permutation = m_permutation + permutation;
	else
		m_permutation = permutation + m_permutation;
}

void PermutationBox::permute(std::pair<unsigned int, unsigned int> permutation, bool isAfter) {
	if (isAfter)
		m_permutation.postAdd(permutation);
	else
		m_permutation.preAdd(permutation);
}

OneHandleRenderer::OneHandleRenderer(RendererGL& rendererGL, OneHandle& oneHandle, std::list<std::pair<unsigned int, unsigned int>>& arrowsData,
	std::list<ArrowBox::ArrowInArrowBox*>& arrows, float basex, float basey, int layer, ZeroHandleRenderer& zeroHandle) :
		m_numberOfTracks(oneHandle.m_numberOfTracks),
		m_prePermutation(rendererGL, basex, basey, layer, *this),
		m_arrows0(rendererGL, oneHandle.m_arrows0, basex+m_prePermutation.lenght(), basey, layer, *this, arrowsData, arrows),
		m_permutation(rendererGL, basex+m_prePermutation.lenght()+m_arrows0.lenght(), basey, layer, *this),
		m_arrows1(rendererGL, oneHandle.m_arrows1, basex+m_prePermutation.lenght()+m_arrows0.lenght()+m_permutation.lenght(), basey, layer, *this),
		m_postPermutation(rendererGL, basex+m_prePermutation.lenght()+m_arrows0.lenght()+m_permutation.lenght()+m_arrows1.lenght(), basey, layer, *this),
		m_lenght(m_prePermutation.lenght()+m_arrows0.lenght()+m_permutation.lenght()+m_arrows1.lenght()+m_postPermutation.lenght()),
		m_quad(rendererGL, basex, basex+m_lenght, basey-0.5/(m_numberOfTracks+1), basey-1.0+0.5/(m_numberOfTracks+1)),
		m_border{ShowableLine(rendererGL, basex, basey-0.5/(m_numberOfTracks+1), basex+m_lenght, basey-0.5/(m_numberOfTracks+1)),
			ShowableLine(rendererGL, basex, basey-1.0+0.5/(m_numberOfTracks+1), basex+m_lenght, basey-1.0+0.5/(m_numberOfTracks+1))},
		m_basex(basex),
		m_basey(basey),
		m_zeroHandle(zeroHandle)
{
	oneHandle.m_renderer = this;
	m_quad.setLayer(layer);
	m_quad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_quad.show(rendererGL);
	
	for (int i=0; i<2; i++) {
		m_border[i].setLayer(layer);
		m_border[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_border[i].show(rendererGL);
	}
}

void OneHandleRenderer::changeBasePoints(float basex, float basey) {
	m_basex = basex; m_basey = basey;
	refresh(true);
}

void OneHandleRenderer::refresh(bool updateArrows) {
	m_quad.changePoints(m_basex, m_basex+m_lenght, m_basey-0.5/(m_numberOfTracks+1), m_basey-1.0+0.5/(m_numberOfTracks+1));
	m_border[0].changePoints(m_basex, m_basey-0.5/(m_numberOfTracks+1),m_basex+m_lenght, m_basey-0.5/(m_numberOfTracks+1));
	m_border[1].changePoints(m_basex, m_basey-1.0+0.5/(m_numberOfTracks+1),m_basex+m_lenght, m_basey-1.0+0.5/(m_numberOfTracks+1));
	m_prePermutation.changeBasePoints(m_basex, m_basey);
	m_arrows0.changeBasePoints(m_basex+m_prePermutation.lenght(), m_basey, updateArrows);
	m_permutation.changeBasePoints(m_basex+m_prePermutation.lenght()+m_arrows0.lenght(), m_basey);
	m_arrows1.changeBasePoints(m_basex+m_prePermutation.lenght()+m_arrows0.lenght()+m_permutation.lenght(), m_basey, updateArrows);
	m_postPermutation.changeBasePoints(m_basex+m_prePermutation.lenght()+m_arrows0.lenght()+m_permutation.lenght()+m_arrows1.lenght(), m_basey);
}

void OneHandleRenderer::updateLenght(bool updateArrows) {
	m_lenght = m_prePermutation.lenght()+m_arrows0.lenght()+m_permutation.lenght()+m_arrows1.lenght()+m_postPermutation.lenght();
	if (updateArrows)
		m_zeroHandle.updateLenght();
	else
		refresh(false);
}



bool OneHandle::StrandOrderCompareAntiClockwise::operator() (unsigned int i, unsigned int j) {
	return m_oneHandle.tracksEndsAntiClockwise(std::make_pair(i, j), m_zeroHandle, m_arrowBox, true);
}

bool OneHandle::StrandOrderCompareClockwise::operator() (unsigned int i, unsigned int j) {
	return m_oneHandle.tracksEndsClockwise(std::make_pair(i, j), m_zeroHandle, m_arrowBox, true);
}


bool OneHandle::tracksEndsClockwise(std::pair<unsigned int, unsigned int> tracks, const ZeroHandle& zeroHandle, const ArrowBox& arrowBox, bool to) const {
	bool isFirstArrowBox = (&arrowBox==&m_arrows0);
	assert((&arrowBox==&m_arrows1) != isFirstArrowBox);
	bool ret;
	
	if (isFirstArrowBox) {
		tracks = std::make_pair(m_postPermutation.post(m_permutation.post(tracks.first)), m_postPermutation.post(m_permutation.post(tracks.second)));
		ret = zeroHandle.trackPairEndsClockwise(tracks, *this, !to);
	} else {
		tracks = std::make_pair(m_postPermutation.post(tracks.first), m_postPermutation.post(tracks.second));
		ret = zeroHandle.trackPairEndsClockwise(tracks, *this, to);
	}
	
	return ret;
}

bool OneHandle::tracksEndsAntiClockwise(std::pair<unsigned int, unsigned int> tracks, const ZeroHandle& zeroHandle, const ArrowBox& arrowBox, bool to) const {
	bool isFirstArrowBox = (&arrowBox==&m_arrows0);
	assert((&arrowBox==&m_arrows1) != isFirstArrowBox);
	bool ret;
	
	if (isFirstArrowBox) {
		tracks = std::make_pair(m_postPermutation.post(m_permutation.post(tracks.first)), m_postPermutation.post(m_permutation.post(tracks.second)));
		ret = zeroHandle.trackPairEndsAntiClockwise(tracks, *this, !to);
	} else {
		tracks = std::make_pair(m_postPermutation.post(tracks.first), m_postPermutation.post(tracks.second));
		ret = zeroHandle.trackPairEndsAntiClockwise(tracks, *this, to);
	}
	
	return ret;
}

void OneHandle::permute(BiPermutation& permutation, bool isFirstArrowBox) {
	assert(permutation.size() == m_numberOfTracks);
	
	PermutationBox& sidePermutation = (isFirstArrowBox) ? m_prePermutation : m_postPermutation;
	ArrowBox& arrowBox = (isFirstArrowBox) ? m_arrows0 : m_arrows1;
	arrowBox.permuteTracks(permutation.getPermutation());
	
	if (!isFirstArrowBox) permutation.inverse();
	sidePermutation.permute(permutation, isFirstArrowBox);
	permutation.inverse();
	m_permutation.permute(permutation, !isFirstArrowBox);
	if (isFirstArrowBox) permutation.inverse();
	
	postPermuteArrowBoxCommand(permutation, *this, isFirstArrowBox);
}

void OneHandle::sortArrowBoxesStrands(ZeroHandle& zeroHandle) {
	StrandOrderCompareAntiClockwise compare0(m_arrows0, *this, zeroHandle);
	StrandOrderCompareClockwise     compare1(m_arrows1, *this, zeroHandle);
	
	Permutation per0(Permutation::Identity(m_numberOfTracks));
	per0.sort(compare0);
	assert(per0.is_sorted(compare0));
	if (!per0.isIdentity()) {
		BiPermutation permutation0(std::move(per0), true);
		permute(permutation0, true);
	}
	
	Permutation per1(Permutation::Identity(m_numberOfTracks));
	per1.sort(compare1);
	assert(per1.is_sorted(compare1));
	if (!per1.isIdentity()) {
		BiPermutation permutation1(std::move(per1), true);
		permute(permutation1, false);
	}
	
	for (size_t i=0; i<m_numberOfTracks; i++) {
		for (size_t j=0; j<i; j++) {
			assert(!compare0(i, j));
		}
	}
	
	for (size_t i=0; i<m_numberOfTracks; i++) {
		for (size_t j=0; j<i; j++) {
			assert(!compare1(i, j));
		}
	}
}

void OneHandle::transferArrowsToFirstArrowBox() {
	if (!m_arrows1.empty()) {
		while (!m_arrows1.empty()) {
			Arrow& arrow = (*(ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(m_arrows1)))->getArrow();
			unsigned int targetI = m_permutation.pre(arrow.begin());
			unsigned int targetJ = m_permutation.pre(arrow.end());
			m_arrows0.transferToBackArrow(ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(m_arrows1).getIterator(), m_arrows1);
			arrow = Arrow(targetI, targetJ);
		}
		
		postMoveArrowToFirstArrowBoxCommand(*this);
	}
}

void OneHandle::transferArrowToSecondArrowBox(ArrowBox::ArrowInArrowBoxIndexedIterator it, ArrowBox::ArrowInArrowBoxIndexedIterator& end) {
	Arrow& arrow = (*it)->getArrow();
	unsigned int targetI = m_permutation.post(arrow.begin());
	unsigned int targetJ = m_permutation.post(arrow.end());
	m_arrows1.transferToFrontArrow(it.getIterator(), m_arrows0);
	end.incIndex();
	arrow = Arrow(targetI, targetJ);
	assert(arrow.isDown());
	
	postMoveArrowToOtherArrowBoxCommand(*this, m_arrows0, makeArrowInArrowBoxIndexed(it), targetI, targetJ);
}

void OneHandle::transferArrowToSecondArrowBoxResolveCrossing(ArrowBox::ArrowInArrowBoxIndexedIterator it, ArrowBox::ArrowInArrowBoxIndexedIterator& end) {
	Arrow& arrow = (*it)->getArrow();
	unsigned int targetI = m_permutation.post(arrow.begin());
	unsigned int targetJ = m_permutation.post(arrow.end());
	
	m_arrows1.transferToFrontArrow(it.getIterator(), m_arrows0);
	m_arrows0.pushBackArrow(Arrow(arrow.end(), arrow.begin()));
	m_permutation.permute(std::make_pair(arrow.begin(), arrow.end()), false);
	end.incIndex();
	arrow = Arrow(targetJ, targetI);
	assert(arrow.isDown());
	
	postMoveArrowToOtherArrowBoxResolveCrossingCommand(*this, m_arrows0, makeArrowInArrowBoxIndexed(it), m_arrows0.back(), targetJ, targetI);
}

void OneHandle::doLemma30(ArrowBox::ArrowInArrowBoxIndexedIterator begin, ArrowBox::ArrowInArrowBoxIndexedIterator& end) {
	while (begin!=ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(m_arrows0) && (*(--begin))->getArrow().isUp());
	if ((*begin)->getArrow().isUp()) return;
	
	ArrowBox::ArrowInArrowBoxIndexedIterator candidate(begin);
	ArrowBox::ArrowInArrowBoxIndexedIterator it(candidate);
	bool shouldDoTransfer = true;
	unsigned int candidateI = (*candidate)->getArrow().begin();
	unsigned int candidateJ = (*candidate)->getArrow().end();
	
	ArrowBox::ArrowInArrowBoxIndexedIterator tIt(ArrowBox::ArrowInArrowBoxIndexedIterator::fromEndOfBox(m_arrows0));
	m_arrows0.lemma29(++it, tIt);
	it=candidate;
	while (shouldDoTransfer && (++it)!=ArrowBox::ArrowInArrowBoxIndexedIterator::fromEndOfBox(m_arrows0)) {
		assert((*candidate)->getArrow().isDown());
		assert((*it)->getArrow().isUp());
		unsigned int itI = (*it)->getArrow().begin();
		unsigned int itJ = (*it)->getArrow().end();
		
		unsigned int createI;
		unsigned int createJ;
		bool shouldCreateArrow = false;
		
		if (candidateI==itJ && candidateJ==itI) {
			if (begin==candidate) {
				++begin;
				begin.decIndex();
			}
			m_arrows0.removeArrowGenCrossing(candidate, it, m_permutation);
			shouldDoTransfer = false;
		} else if (candidateI == itJ) {
			createI = itI;
			createJ = candidateJ;
			shouldCreateArrow = true;
		} else if (candidateJ == itI) {
			createI = candidateI;
			createJ = itJ;
			shouldCreateArrow = true;
		}
		
		if (shouldCreateArrow) {
			ArrowBox::ArrowInArrowBoxIndexedIterator it0(it); ++it0;
			ArrowBox::ArrowInArrowBoxIndexedIterator newArrow = it0.insertInArrowBox(new ArrowBox::ArrowInArrowBox(Arrow(createI, createJ),
					ArrowBox::ArrowRendererInList()), m_arrows0);
			postGenArrowAfterMoveCrossing(m_arrows0, makeArrowInArrowBoxIndexed(candidate), makeArrowInArrowBoxIndexed(it), **newArrow, createI, createJ, false);
			if (candidate==begin) {
				++begin; begin.decIndex();
			}
			candidate.transfer(it0, m_arrows0);
			newArrow.decIndex();
			it = candidate;
			
			if ((*newArrow)->getArrow().isDown()) {
				begin = it0;
				shouldDoTransfer = false;
			}
		}
	}
	
	if (shouldDoTransfer) {
		bool candidateIsBegin = (candidate==begin);
		if (candidateIsBegin) ++begin;
		
		assert(candidateI < candidateJ);
		if (m_permutation.post(candidateI) < m_permutation.post(candidateJ)) {
			transferArrowToSecondArrowBox(candidate, end);
			if (candidateIsBegin) begin.decIndex();
		} else {
			bool beginIsEnd = (begin == ArrowBox::ArrowInArrowBoxIndexedIterator::fromEndOfBox(m_arrows0));
			transferArrowToSecondArrowBoxResolveCrossing(candidate, end);
			if (candidateIsBegin) {
				if (beginIsEnd) --begin;
				else begin.decIndex();
			}
		}
		
		candidate = ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(m_arrows1);
		it = candidate; ++it;
		m_arrows1.lemma29(candidate, end, &it);
	}
	
	if (!m_arrows0.empty()) {
		doLemma30(begin, end);
	}
}

void OneHandle::lemma30(ZeroHandle& zeroHandle) {
	sortArrowBoxesStrands(zeroHandle);
	ArrowBox::ArrowInArrowBoxIndexedIterator end(ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(m_arrows1));
	
	if (!m_arrows0.empty()) {
		doLemma30(ArrowBox::ArrowInArrowBoxIndexedIterator::fromEndOfBox(m_arrows0), end);
		ArrowBox::ArrowInArrowBoxIndexedIterator it(ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(m_arrows0));
		ArrowBox::ArrowInArrowBoxIndexedIterator it0(ArrowBox::ArrowInArrowBoxIndexedIterator::fromEndOfBox(m_arrows0));
		m_arrows0.lemma29(it, it0);
	}
	
	// On verifie si le lemme a fonctionne
#ifndef NDEBUG
	ArrowBox::ArrowInArrowBoxIndexedIterator it(ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(m_arrows0));
	while (it!=ArrowBox::ArrowInArrowBoxIndexedIterator::fromEndOfBox(m_arrows0)) {
		assert((*it)->getArrow().isUp());
		++it;
	}
	
	it = ArrowBox::ArrowInArrowBoxIndexedIterator::fromBeginOfBox(m_arrows1);
	while (it!=end) {
		assert((*it)->getArrow().isDown());
		++it;
	}
#endif
}

int OneHandle::getArrowDepth(const ArrowBox& arrowBox, const ZeroHandle& zeroHandle, std::pair<unsigned int, unsigned int> tracks, bool to) const {
	bool isFirstArrowBox = (&arrowBox==&m_arrows0);
	assert((&arrowBox==&m_arrows1) != isFirstArrowBox);
	int ret;
	
	if (isFirstArrowBox) {
		tracks = std::make_pair(m_postPermutation.post(m_permutation.post(tracks.first)), m_postPermutation.post(m_permutation.post(tracks.second)));
		ret = zeroHandle.getArrowDepth(tracks, *this, !to);
	} else {
		tracks = std::make_pair(m_postPermutation.post(tracks.first), m_postPermutation.post(tracks.second));
		ret = zeroHandle.getArrowDepth(tracks, *this, to);
	}
	
	return ret;
}

void OneHandle::removeDepthMArrows(const ZeroHandle& zeroHandle, int m, bool to) {
	m_arrows0.removeDepthMArrows(*this, zeroHandle, m, to);
	m_arrows1.removeDepthMArrows(*this, zeroHandle, m, to);
}

unsigned int OneHandle::getMinimalDepth(const ZeroHandle& zeroHandle) const {
	return std::min(m_arrows0.getMinimalDepth(*this, zeroHandle), m_arrows1.getMinimalDepth(*this, zeroHandle));
}

void OneHandle::addArrowFromZeroHandle(PassArrowsThroughtZeroHandleCommand* cmd, ArrowBox& src, std::list<ArrowBox::ArrowInArrowBox*>::iterator arrow, int index,
			unsigned int beginI, unsigned int beginJ, unsigned int endI, unsigned int endJ, bool fromLeft) {
	if (fromLeft) {
		unsigned int targetI = m_prePermutation.post(endI);
		unsigned int targetJ = m_prePermutation.post(endJ);
		
		cmd->addArrow(makeArrowInArrowBoxIndexed(**arrow, index), m_arrows0, beginI, beginJ, endI, endJ, targetI, targetJ);
		(*arrow)->getArrow() = Arrow(targetI, targetJ);
		m_arrows0.transferToFrontArrow(arrow, src);
	} else {
		unsigned int targetI = m_postPermutation.pre(endI);
		unsigned int targetJ = m_postPermutation.pre(endJ);
		
		cmd->addArrow(makeArrowInArrowBoxIndexed(**arrow, index), m_arrows1, beginI, beginJ, endI, endJ, targetI, targetJ);
		(*arrow)->getArrow() = Arrow(targetI, targetJ);
		m_arrows1.transferToBackArrow(arrow, src);
	}
}

void OneHandle::removeArrowToZeroHandle(ZeroHandle& zeroHandle, PassArrowsThroughtZeroHandleCommand* cmd, ArrowBox& src,
		std::list<ArrowBox::ArrowInArrowBox*>::iterator arrow, int index) {
	bool isFirstArrowBox = (&src==&m_arrows0);
	assert((&src==&m_arrows1) != isFirstArrowBox);
	
	unsigned int beginI;
	unsigned int beginJ;
	if (isFirstArrowBox) {
		beginI = m_prePermutation.pre((*arrow)->getArrow().begin());
		beginJ = m_prePermutation.pre((*arrow)->getArrow().end());
	} else {
		beginI = m_postPermutation.post((*arrow)->getArrow().begin());
		beginJ = m_postPermutation.post((*arrow)->getArrow().end());
	}
	
	zeroHandle.moveArrowThroughout(cmd, src, arrow, index, beginI, beginJ);
}

void OneHandle::emptyArrowBoxThroughZeroHandle(ZeroHandle& zeroHandle, bool isFirstArrowBox) {
	if (isFirstArrowBox)
		m_arrows0.moveArrowsThroughoutZeroHandle(zeroHandle, *this, false);
	else
		m_arrows1.moveArrowsThroughoutZeroHandle(zeroHandle, *this, true);
}
