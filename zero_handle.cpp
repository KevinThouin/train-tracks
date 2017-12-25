#include "zero_handle.hpp"
#include "color.h"

#include <cassert>
#include <algorithm>

std::pair<unsigned int, int> ZeroHandle::getIndexEdgeFromIndex(unsigned int index) const {
	int edge;
	if (index<m_numTrackFull) {
		edge = 0;
		index = m_fullHandle.getRightIndex(index);
	} else if (index<m_numTrackFull+m_numTrackVoid) {
		edge = 1;
		index -= m_numTrackFull;
	} else if (index<2*m_numTrackFull+m_numTrackVoid) {
		edge = 2;
		index -= (m_numTrackFull+m_numTrackVoid);
	} else {
		edge = 3;
		index = m_voidHandle.getRightIndex(index-2*m_numTrackFull-m_numTrackVoid);
	}
	return std::make_pair(index, edge);
}

unsigned int ZeroHandle::getMarkovIndex(unsigned int i, unsigned int j, int edge) const {
	if (edge==0) {
		i = m_fullHandle.getLeftIndex(i);
		j = m_fullHandle.getLeftIndex(j);
	} else if (edge==3) {
		i = m_voidHandle.getLeftIndex(i);
		j = m_voidHandle.getLeftIndex(j);
	}
	
	unsigned int l = (edge==1 || edge==3) ? m_numTrackVoid : m_numTrackFull;
	unsigned int ret = 2+i*l+j;
	if (edge==1)
		ret += m_numTrackFull*m_numTrackFull;
	else if (edge==2)
		ret += m_numTrackFull*m_numTrackFull + m_numTrackVoid*m_numTrackVoid;
	else if (edge==3)
		ret += 2*m_numTrackFull*m_numTrackFull + m_numTrackVoid*m_numTrackVoid;
	
	return ret;
}

unsigned int ZeroHandle::getMarkovIndex(unsigned int i, unsigned int j, const OneHandle& oneHandle, bool isPost) const {
	bool isVoidHandle = (&oneHandle==&m_voidHandle);
	assert((&oneHandle==&m_fullHandle) != isVoidHandle);
	int edge;
	if (!isVoidHandle && !isPost)
		edge = 0;
	else if (isVoidHandle && isPost)
		edge = 1;
	else if (!isVoidHandle && isPost)
		edge = 2;
	else
		edge = 3;
	
	return getMarkovIndex(i, j, edge);
}

unsigned int ZeroHandle::getCorrectedIndexFromEdge(unsigned int index, int edge) const {
	if (edge==0) {
		return m_fullHandle.getLeftIndex(index);
	} else if (edge==3) {
		return m_voidHandle.getLeftIndex(index);
	} else {
		return index;
	}
}

void ZeroHandle::buildMarkovPart(DeterministMarkov& markov, int edge) const {
	bool isVoidHandle = (edge==1 || edge==3);
	unsigned int n = (isVoidHandle) ? m_numTrackVoid : m_numTrackFull;
	unsigned int add;
	if (edge==0)
		add = 0;
	else if (edge==1)
		add = m_numTrackFull;
	else if (edge==2)
		add = m_numTrackFull+m_numTrackVoid;
	else
		add = 2*m_numTrackFull+m_numTrackVoid;
	
	for (unsigned int i=0; i<n; ++i) {
		std::pair<unsigned int, int> pI = getIndexEdgeFromIndex(m_pairing[getCorrectedIndexFromEdge(i, edge)+add]);
		unsigned int& I = pI.first;
		int& iEdge = pI.second;
		int iEdge0 = iEdge-edge;
		if (iEdge0<0) iEdge0+=4;
		
		for (unsigned int j=0; j<n; ++j) {
			std::pair<unsigned int, int> pJ = getIndexEdgeFromIndex(m_pairing[getCorrectedIndexFromEdge(j, edge)+add]);
			unsigned int& J = pJ.first;
			int& jEdge = pJ.second;
			
			unsigned int pIndex0 = getMarkovIndex(i, j, edge);
			if (iEdge==jEdge) {
				int iEdge1;
				if (iEdge==0) {
					iEdge1 = 2;
				} else if (iEdge==1) {
					iEdge1 = 3;
				} else if (iEdge==2) {
					iEdge1 = 0;
				} else {
					iEdge1 = 1;
				}
				unsigned int pIndex1 = getMarkovIndex(I, J, iEdge1);
				markov[pIndex0] = pIndex1;
			} else {
				jEdge -= edge;
				assert(iEdge0!=0 && jEdge!=0);
				if (jEdge<0) jEdge+=4;
				markov[pIndex0] = (iEdge0<jEdge) ? 0 : 1;
			}
		}
	}
}

ZeroHandle::ZeroHandle(unsigned int k, const FUMatrix& matrix, std::list<Arrow>&& voidArrows, std::list<Arrow>&& fullArrows,
		std::list<std::pair<unsigned int, unsigned int>>& voidArrowsData, std::list<ArrowBox::ArrowInArrowBox*>& voidArrowsPtr,
		std::list<std::pair<unsigned int, unsigned int>>& fullArrowsData, std::list<ArrowBox::ArrowInArrowBox*>& fullArrowsPtr) :
	m_numTrackVoid(matrix.size()/2-k), m_numTrackFull(k), m_pairing(matrix, k), m_voidHandle(std::move(voidArrows), m_numTrackVoid, voidArrowsData, voidArrowsPtr),
		m_fullHandle(std::move(fullArrows), m_numTrackFull, fullArrowsData, fullArrowsPtr) {
	updateMarkov();
}

void ZeroHandle::updateMarkov() {
	DeterministMarkov markov(2+2*(m_numTrackVoid*m_numTrackVoid+m_numTrackFull*m_numTrackFull));
	
	markov[0] = 0;
	markov[1] = 1;
	
	buildMarkovPart(markov, 2);
	buildMarkovPart(markov, 1);
	buildMarkovPart(markov, 0);
	buildMarkovPart(markov, 3);
	
	m_markov = std::move(DeterministMarkovPower(std::move(markov), 1+2*(m_numTrackVoid*m_numTrackVoid+m_numTrackFull*m_numTrackFull)));
}

bool ZeroHandle::trackPairEndsClockwise(std::pair<unsigned int, unsigned int> tracks, const OneHandle& oneHandle, bool isPost) const {
	unsigned int index = getMarkovIndex(tracks.first, tracks.second, oneHandle, isPost);
	return (m_markov.getMaxValue(index)==0);
}

bool ZeroHandle::trackPairEndsAntiClockwise(std::pair<unsigned int, unsigned int> tracks, const OneHandle& oneHandle, bool isPost) const {
	unsigned int index = getMarkovIndex(tracks.first, tracks.second, oneHandle, isPost);
	return (m_markov.getMaxValue(index)==1);
}

int  ZeroHandle::getArrowDepth(std::pair<unsigned int, unsigned int> tracks, const OneHandle& oneHandle, bool isPost) const {
	unsigned int index = getMarkovIndex(tracks.first, tracks.second, oneHandle, isPost);
	std::pair<unsigned int, bool> v = m_markov.getWeight(index);
	return (v.second) ? static_cast<int>(v.first) : -static_cast<int>(v.first);
}

unsigned int ZeroHandle::getDepth() const {
	return std::min(m_voidHandle.getMinimalDepth(*this), m_fullHandle.getMinimalDepth(*this));
}

void ZeroHandle::moveArrowThroughout(PassArrowsThroughtZeroHandle* cmd, ArrowBox& src, std::list<ArrowBox::ArrowInArrowBox*>::iterator arrow, int index,
			unsigned int beginI, unsigned int beginJ) {
	unsigned int add;
	
	if (&src==&m_fullHandle.getFirstArrowBox()) {
		add = 0;
	} else if (&src==&m_voidHandle.getSecondArrowBox()) {
		add = m_numTrackFull;
	} else if (&src==&m_fullHandle.getSecondArrowBox()) {
		add = m_numTrackFull+m_numTrackVoid;
	} else {
		add = 2*m_numTrackFull+m_numTrackVoid;
	}
	
	unsigned int endI = m_pairing[beginI+add];
	unsigned int endJ = m_pairing[beginJ+add];
	int endEdge;
	
	if (endI<m_numTrackFull) {
		assert(endJ<m_numTrackFull);
		endEdge = 0;
	} else if (endI<m_numTrackFull+m_numTrackVoid) {
		assert(endJ<m_numTrackFull+m_numTrackVoid && endJ>=m_numTrackFull);
		endEdge = 1;
		endI -= m_numTrackFull; endJ -= m_numTrackFull;
	} else if (endI<2*m_numTrackFull+m_numTrackVoid) {
		assert(endJ<2*m_numTrackFull+m_numTrackVoid && endJ>=m_numTrackFull+m_numTrackVoid);
		endEdge = 2;
		endI -= m_numTrackFull+m_numTrackVoid; endJ -= m_numTrackFull+m_numTrackVoid;
	} else {
		assert(endJ<2*m_numTrackFull+2*m_numTrackVoid && endJ>=2*m_numTrackFull+m_numTrackVoid);
		endEdge = 3;
		endI -= 2*m_numTrackFull+m_numTrackVoid; endJ -= 2*m_numTrackFull+m_numTrackVoid;
	}
	
	if (endEdge==0 || endEdge==2) {
		m_fullHandle.addArrowFromZeroHandle(cmd, src, arrow, index, beginI, beginJ, endI, endJ, endEdge==0);
	} else {
		m_voidHandle.addArrowFromZeroHandle(cmd, src, arrow, index, beginI, beginJ, endI, endJ, endEdge==3);
	}
}

void ZeroHandle::proposition28() {
	unsigned int depth = getDepth();
	printDepth();
		
	while (depth!=std::numeric_limits<unsigned int>::max()) {
		// Step 1
		m_voidHandle.transferArrowsToFirstArrowBox();
		m_voidHandle.lemma30(*this);
		m_fullHandle.transferArrowsToFirstArrowBox();
		m_fullHandle.lemma30(*this);
		updateMarkov();
		
		// Step 2
		m_voidHandle.removeDepthMArrows(*this, depth, true);
		m_fullHandle.removeDepthMArrows(*this, depth, true);
		
		// Step 3
		m_voidHandle.lemma30(*this);
		m_voidHandle.removeDepthMArrows(*this, depth, true);
		m_voidHandle.emptyArrowBoxThroughZeroHandle(*this, true);
		updateMarkov();
		m_fullHandle.lemma30(*this);
		m_fullHandle.removeDepthMArrows(*this, depth, true);
		m_fullHandle.emptyArrowBoxThroughZeroHandle(*this, true);
		updateMarkov();
		m_voidHandle.transferArrowsToFirstArrowBox();
		m_voidHandle.lemma30(*this);
		m_voidHandle.removeDepthMArrows(*this, depth, true);
		m_voidHandle.emptyArrowBoxThroughZeroHandle(*this, false);
		updateMarkov();
		m_fullHandle.transferArrowsToFirstArrowBox();
		m_fullHandle.lemma30(*this);
		m_fullHandle.removeDepthMArrows(*this, depth, true);
		m_fullHandle.emptyArrowBoxThroughZeroHandle(*this, false);
		updateMarkov();
		
		// Step 4
		m_voidHandle.removeDepthMArrows(*this, depth, false);
		m_fullHandle.removeDepthMArrows(*this, depth, false);
		
		unsigned int newDepth = getDepth();
		printDepth();
		assert(newDepth > depth);
		depth = newDepth;
	}
}

void ZeroHandle::printDepth() const {
	unsigned int depth = getDepth();
	if (depth==std::numeric_limits<unsigned int>::max())
		std::cout << "Depth: Infinity" << std::endl;
	else
		std::cout << "Depth: " << depth << std::endl;
}


void ZeroHandleRenderer::getLenghtPoints(unsigned int i, float& t0, float& t1, float& t2, float& t3, float& t4, float& t5, float& t6, float& t7, float& t8) {
	if (i < m_numTrackFull) {
		t0 = m_fullWestGapTracks[0].getCurve().getLenght();
		t1 = t0;
		t2 = t1;
		t3 = t2;
		t4 = t3;
		t5 = t4;
		t6 = t5;
		t7 = t6;
		t8 = t7;
	} else if (i < m_numTrackFull+m_numTrackVoid) {
		i-= m_numTrackFull;
		t0 =      m_voidEastGapTracks[0].getCurve().getLenght(); // NOTE: toutes les voies devraient avoir la meme longueur
		t1 = t0 + 1.0;
		t2 = t1 + m_rightVoidHandleTracks[0].getCurve().getLenght();
		t3 = t2 + 1.0;
		t4 = t3 + m_lowerVoidHandleTracks[0].getCurve().getLenght();
		t5 = t4 + 1.0;
		t6 = t5 + m_southTracks[0].getCurve().getLenght();
		t7 = t6;
		t8 = t7;
	} else if (i < 2*m_numTrackFull+m_numTrackVoid) {
		i -= (m_numTrackFull+m_numTrackVoid);
		t0 =      m_fullEastGapTracks[0].getCurve().getLenght();
		t1 = t0 + 1.0;
		t2 = t1 + m_rightFullHandleTracks[0].getCurve().getLenght();
		t3 = t2 + 1.0;
		t4 = t3 + m_lowerFullHandleTracks[0].getCurve().getLenght();
		t5 = t4 + 1.0;
		t6 = t5 + m_leftFullHandleTracks[0].getCurve().getLenght();
		t7 = t6 + 1.0;
		t8 = t7 + m_westTracks[0].getCurve().getLenght();
	} else {
		i -= (2*m_numTrackFull+m_numTrackVoid);
		t0 =      m_voidWestGapTracks[0].getCurve().getLenght();
		t1 = t0 + 1.0;
		t2 = t1 + m_northTracks[0].getCurve().getLenght();
		t3 = t2;
		t4 = t3;
		t5 = t4;
		t6 = t5;
		t7 = t6;
		t8 = t7;
	}
}

ShowableCurve& ZeroHandleRenderer::getInterpolatedTrack(float& newT, unsigned int trackStart, unsigned int trackEnd, float time, const float t[19]) {
	if (trackStart > trackEnd) {
		std::swap(trackStart, trackEnd);
		time = 1.0-time;
	}
	
	int startEdge, endEdge;
	unsigned int indexStart = trackStart; unsigned int indexEnd = trackEnd;
	if (trackStart < m_numTrackFull) {
		startEdge = 0;
	} else if (trackStart < m_numTrackFull+m_numTrackVoid) {
		startEdge = 1;
		trackStart -= m_numTrackFull;
	} else if (trackStart < 2*m_numTrackFull+m_numTrackVoid) {
		startEdge = 2;
		trackStart -= m_numTrackFull+m_numTrackVoid;
	} else {
		startEdge = 3;
		trackStart -= 2*m_numTrackFull+m_numTrackVoid;
	}
	
	if (trackEnd < m_numTrackFull) {
		endEdge = 0;
	} else if (trackEnd < m_numTrackFull+m_numTrackVoid) {
		endEdge = 1;
		trackEnd -= m_numTrackFull;
	} else if (trackEnd < 2*m_numTrackFull+m_numTrackVoid) {
		endEdge = 2;
		trackEnd -= m_numTrackFull+m_numTrackVoid;
	} else {
		endEdge = 3;
		trackEnd -= 2*m_numTrackFull+m_numTrackVoid;
	}
	
	ShowableCurve* track = nullptr;
	
	if (time<t[0]) {
		track = (startEdge==0) ? &m_fullEastGapTracks[trackStart].getCurve() : ((startEdge==1) ? &m_voidWestGapTracks[trackStart].getCurve() :
				((startEdge==2) ? &m_fullWestGapTracks[trackStart].getCurve() : &m_voidEastGapTracks[trackStart].getCurve()));
		newT = time/t[0];
	} else if (time<t[1]) {
		assert(startEdge!=0);
		track = (startEdge==1) ? &m_upperRightCornerVoidHandleTracks[trackStart].getCurve() :
				((startEdge==2) ? &m_upperRightCornerFullHandleTracks[trackStart].getCurve() : &m_upperLeftCornerVoidHandleTracks[trackStart].getCurve());
		newT = (time-t[0])/(t[1]-t[0]);
	} else if (time<t[2]) {
		assert(startEdge!=0);
		track = (startEdge==1) ? &m_rightVoidHandleTracks[trackStart].getCurve() :
				((startEdge==2) ? &m_rightFullHandleTracks[trackStart].getCurve() : &m_northTracks[trackStart].getCurve());
		newT = (time-t[1])/(t[2]-t[1]);
	} else if (time<t[3]) {
		assert(startEdge!=0 && startEdge!=3);
		track = (startEdge==1) ? &m_lowerRightCornerVoidHandleTracks[trackStart].getCurve() :
				&m_lowerRightCornerFullHandleTracks[trackStart].getCurve();
		newT = (time-t[2])/(t[3]-t[2]);
	} else if (time<t[4]) {
		assert(startEdge!=0 && startEdge!=3);
		track = (startEdge==1) ? &m_lowerVoidHandleTracks[trackStart].getCurve() :
				&m_lowerFullHandleTracks[trackStart].getCurve();
		newT = (time-t[3])/(t[4]-t[3]);
	} else if (time<t[5]) {
		assert(startEdge!=0 && startEdge!=3);
		track = (startEdge==1) ? &m_lowerLeftCornerVoidHandleTracks[trackStart].getCurve() :
				&m_lowerLeftCornerFullHandleTracks[trackStart].getCurve();
		newT = (time-t[4])/(t[5]-t[4]);
	} else if (time<t[6]) {
		assert(startEdge!=0 && startEdge!=3);
		track = (startEdge==1) ? &m_southTracks[trackStart].getCurve() :
				&m_leftFullHandleTracks[trackStart].getCurve();
		newT = (time-t[5])/(t[6]-t[5]);
	} else if (time<t[7]) {
		assert(startEdge==2);
		track = &m_upperLeftCornerFullHandleTracks[trackStart].getCurve();
		newT = (time-t[6])/(t[7]-t[6]);
	} else if (time<t[8]) {
		assert(startEdge==2);
		track = &m_westTracks[trackStart].getCurve();
		newT = (time-t[7])/(t[8]-t[7]);
	} else if (time<t[9]) {
		auto it = m_pairTrackMap.find(ZeroHandle::UnorderedIdempotentsPair(indexStart, indexEnd));
		assert(it!=m_pairTrackMap.end());
		track = &it->second.getCurve();
		newT = (time-t[8])/(t[9]-t[8]);
	} else { 
		time = 1.0-time;
		
		if (time<t[10]) {
			track = (endEdge==0) ? &m_fullEastGapTracks[trackEnd].getCurve() : ((endEdge==1) ? &m_voidWestGapTracks[trackEnd].getCurve() :
					((endEdge==2) ? &m_fullWestGapTracks[trackEnd].getCurve() : &m_voidEastGapTracks[trackEnd].getCurve()));
			newT = time/t[10];
		} else if (time<t[11]) {
			assert(endEdge!=0);
			track = (endEdge==1) ? &m_upperRightCornerVoidHandleTracks[trackEnd].getCurve() :
					((endEdge==2) ? &m_upperRightCornerFullHandleTracks[trackEnd].getCurve() : &m_upperLeftCornerVoidHandleTracks[trackEnd].getCurve());
			newT = (time-t[10])/(t[11]-t[10]);
		} else if (time<t[12]) {
			assert(endEdge!=0);
			track = (endEdge==1) ? &m_rightVoidHandleTracks[trackEnd].getCurve() :
					((endEdge==2) ? &m_rightFullHandleTracks[trackEnd].getCurve() : &m_northTracks[trackEnd].getCurve());
			newT = (time-t[11])/(t[12]-t[11]);
		} else if (time<t[13]) {
			assert(endEdge!=0 && endEdge!=3);
			track = (endEdge==1) ? &m_lowerRightCornerVoidHandleTracks[trackEnd].getCurve() :
					&m_lowerRightCornerFullHandleTracks[trackEnd].getCurve();
			newT = (time-t[12])/(t[13]-t[12]);
		} else if (time<t[14]) {
			assert(endEdge!=0 && endEdge!=3);
			track = (endEdge==1) ? &m_lowerVoidHandleTracks[trackEnd].getCurve() :
					&m_lowerFullHandleTracks[trackEnd].getCurve();
			newT = (time-t[13])/(t[14]-t[13]);
		} else if (time<t[15]) {
			assert(endEdge!=0 && endEdge!=3);
			track = (endEdge==1) ? &m_lowerLeftCornerVoidHandleTracks[trackEnd].getCurve() :
					&m_lowerLeftCornerFullHandleTracks[trackEnd].getCurve();
			newT = (time-t[14])/(t[15]-t[14]);
		} else if (time<t[16]) {
			assert(endEdge!=0 && endEdge!=3);
			track = (endEdge==1) ? &m_southTracks[trackEnd].getCurve() :
					&m_leftFullHandleTracks[trackEnd].getCurve();
			newT = (time-t[15])/(t[16]-t[15]);
		} else if (time<t[17]) {
			assert(endEdge==2);
			track = &m_upperLeftCornerFullHandleTracks[trackEnd].getCurve();
			newT = (time-t[16])/(t[17]-t[16]);
		} else {
			assert(endEdge==2);
			track = &m_westTracks[trackEnd].getCurve();
			newT = (time-t[17])/(t[18]-t[17]);
		}
	}
	
	assert(track!=nullptr);
	return *track;
}

ZeroHandleRenderer::ZeroHandleRenderer(RendererGL& rendererGL, const Pairing& pairing, ZeroHandle& zeroHandle,
				std::list<std::pair<unsigned int, unsigned int>>& voidArrowsData, std::list<ArrowBox::ArrowInArrowBox*>& voidArrows,
				std::list<std::pair<unsigned int, unsigned int>>& fullArrowsData, std::list<ArrowBox::ArrowInArrowBox*>& fullArrows) :
	m_voidHandle(rendererGL, zeroHandle.m_voidHandle, voidArrowsData, voidArrows, 0.5, 1.5, 0, *this),
	m_fullHandle(rendererGL, zeroHandle.m_fullHandle, fullArrowsData, fullArrows, 0.5, 0.5, 2, *this),
	m_handleLenght(std::max(m_voidHandle.lenght(), m_fullHandle.lenght()-1.0f)),
	m_numTrackVoid(zeroHandle.m_numTrackVoid), m_numTrackFull(zeroHandle.m_numTrackFull),
	m_line{ShowableLine(rendererGL, 0.5, 0.5, 0.5, 0.5-0.5/(m_numTrackFull+1)), ShowableLine(rendererGL, 0.5, -0.5, 0.5, -0.5+0.5/(m_numTrackFull+1)),
		 ShowableLine(rendererGL, 0.5, -0.5, 0.5-0.5/(m_numTrackVoid+1), -0.5), ShowableLine(rendererGL, -0.5, -0.5, -0.5+0.5/(m_numTrackVoid+1), -0.5),
		 ShowableLine(rendererGL, -0.5, -0.5, -0.5, -0.5+0.5/(m_numTrackFull+1)), ShowableLine(rendererGL, -0.5, 0.5, -0.5, 0.5-0.5/(m_numTrackFull+1)),
		 ShowableLine(rendererGL, -0.5, 0.5, -0.5+0.5/(m_numTrackVoid+1), 0.5), ShowableLine(rendererGL, 0.5, 0.5, 0.5-0.5/(m_numTrackVoid+1), 0.5)
	},
	m_quad(rendererGL, -0.5, 0.5, -0.5, 0.5),
	
	m_southQuad(rendererGL, -0.5+0.5/(m_numTrackVoid+1), 0.5-0.5/(m_numTrackVoid+1), -0.5-0.5/(m_numTrackVoid+1), -0.5),
	m_southBorder{ShowableLine(rendererGL, 0.5-0.5/(m_numTrackVoid+1), -0.5, 0.5-0.5/(m_numTrackVoid+1), -0.5-0.5/(m_numTrackVoid+1)), 
		ShowableLine(rendererGL, -0.5+0.5/(m_numTrackVoid+1), -0.5, -0.5+0.5/(m_numTrackVoid+1), -1.5+0.5/(m_numTrackVoid+1))},
		
	m_westQuad(rendererGL, -0.5-0.5/(m_numTrackFull+1), -0.5, -0.5+0.5/(m_numTrackFull+1), 0.5-0.5/(m_numTrackFull+1)),
	m_westBorder{ShowableLine(rendererGL, -0.5, -0.5+0.5/(m_numTrackFull+1), -0.5-0.5/(m_numTrackFull+1), -0.5+0.5/(m_numTrackFull+1)), 
		ShowableLine(rendererGL, -0.5, 0.5-0.5/(m_numTrackFull+1), -1.5+0.5/(m_numTrackFull+1), 0.5-0.5/(m_numTrackFull+1))},
		
	m_northQuad(rendererGL, -0.5+0.5/(m_numTrackVoid+1), 0.5-0.5/(m_numTrackVoid+1), 0.5, 1.5-0.5/(m_numTrackVoid+1)),
	m_northBorder{ShowableLine(rendererGL, -0.5+0.5/(m_numTrackVoid+1), 0.5, -0.5+0.5/(m_numTrackVoid+1), 1.5-0.5/(m_numTrackVoid+1)),
		ShowableLine(rendererGL, 0.5-0.5/(m_numTrackVoid+1), 0.5, 0.5-0.5/(m_numTrackVoid+1), 0.5+0.5/(m_numTrackVoid+1))},
	
	m_lowerVoidHandleQuad(rendererGL, -0.5+0.5/(m_numTrackVoid+1), m_handleLenght+1.5-0.5/(m_numTrackVoid+1),
						-1.5+0.5/(m_numTrackVoid+1), -0.5-0.5/(m_numTrackVoid+1)),
	m_lowerVoidHandleBorder{ShowableLine(rendererGL, -0.5+0.5/(m_numTrackVoid+1), -1.5+0.5/(m_numTrackVoid+1),
						m_handleLenght+1.5-0.5/(m_numTrackVoid+1), -1.5+0.5/(m_numTrackVoid+1)),
				ShowableLine(rendererGL, 0.5-0.5/(m_numTrackVoid+1), -0.5-0.5/(m_numTrackVoid+1),
						m_handleLenght+0.5+0.5/(m_numTrackVoid+1), -0.5-0.5/(m_numTrackVoid+1))},
	m_lowerFullHandleQuad(rendererGL, -1.5+0.5/(m_numTrackFull+1), m_handleLenght+2.5-0.5/(m_numTrackFull+1),
						-2.5+0.5/(m_numTrackFull+1), -1.5-0.5/(m_numTrackFull+1)),
	m_lowerFullHandleBorder{ShowableLine(rendererGL, -1.5+0.5/(m_numTrackFull+1), -2.5+0.5/(m_numTrackFull+1),
						m_handleLenght+2.5-0.5/(m_numTrackFull+1), -2.5+0.5/(m_numTrackFull+1)),
				ShowableLine(rendererGL, -0.5-0.5/(m_numTrackFull+1), -1.5-0.5/(m_numTrackFull+1),
						m_handleLenght+1.5+0.5/(m_numTrackFull+1), -1.5-0.5/(m_numTrackFull+1))},
	
	m_leftFullHandleQuad(rendererGL, -1.5+0.5/(m_numTrackFull+1), -0.5-0.5/(m_numTrackFull+1),
					   -1.5-0.5/(m_numTrackFull+1), 0.5-0.5/(m_numTrackFull+1)),
	m_leftFullHandleBorder{ShowableLine(rendererGL, -1.5+0.5/(m_numTrackFull+1), 0.5-0.5/(m_numTrackFull+1),
						-1.5+0.5/(m_numTrackFull+1), -2.5+0.5/(m_numTrackFull+1)), 
		ShowableLine(rendererGL, -0.5-0.5/(m_numTrackFull+1), -0.5+0.5/(m_numTrackFull+1),
						-0.5-0.5/(m_numTrackFull+1), -1.5-0.5/(m_numTrackFull+1))},
	m_rightFullHandleQuad(rendererGL, m_handleLenght+1.5+0.5/(m_numTrackFull+1), m_handleLenght+2.5-0.5/(m_numTrackFull+1),
						-1.5-0.5/(m_numTrackFull+1), 0.5-0.5/(m_numTrackFull+1)),
	m_rightFullHandleBorder{ShowableLine(rendererGL, m_handleLenght+1.5+0.5/(m_numTrackFull+1), -1.5-0.5/(m_numTrackFull+1),
						m_handleLenght+1.5+0.5/(m_numTrackFull+1), -0.5+0.5/(m_numTrackFull+1)), 
		ShowableLine(rendererGL, m_handleLenght+2.5-0.5/(m_numTrackFull+1), -2.5+0.5/(m_numTrackFull+1),
						m_handleLenght+2.5-0.5/(m_numTrackFull+1), 0.5-0.5/(m_numTrackFull+1))},
	m_rightVoidHandleQuad(rendererGL, m_handleLenght+0.5+0.5/(m_numTrackVoid+1), m_handleLenght+1.5-0.5/(m_numTrackVoid+1),
						-0.5-0.5/(m_numTrackVoid+1), 1.5-0.5/(m_numTrackVoid+1)),
	m_rightVoidHandleBorder{ShowableLine(rendererGL, m_handleLenght+0.5+0.5/(m_numTrackVoid+1), -0.5-0.5/(m_numTrackVoid+1),
						m_handleLenght+0.5+0.5/(m_numTrackVoid+1), 0.5+0.5/(m_numTrackVoid+1)), 
		ShowableLine(rendererGL, m_handleLenght+1.5-0.5/(m_numTrackVoid+1), -1.5+0.5/(m_numTrackVoid+1),
						m_handleLenght+1.5-0.5/(m_numTrackVoid+1), 1.5-0.5/(m_numTrackVoid+1))},
	
	m_voidEastGapQuad(rendererGL, 0.5-0.5/(m_numTrackVoid+1), 0.5-0.5*(m_voidHandle.lenght()-m_handleLenght),
					0.5+0.5/(m_numTrackVoid+1), 1.5-0.5/(m_numTrackVoid+1)),
	m_voidEastGapBorder{ShowableLine(rendererGL, 0.5-0.5/(m_numTrackVoid+1), 0.5+0.5/(m_numTrackVoid+1),
					0.5-0.5*(m_voidHandle.lenght()-m_handleLenght), 0.5+0.5/(m_numTrackVoid+1)),
		ShowableLine(rendererGL, -0.5+0.5/(m_numTrackVoid+1), 1.5-0.5/(m_numTrackVoid+1),
					0.5-0.5*(m_voidHandle.lenght()-m_handleLenght), 1.5-0.5/(m_numTrackVoid+1))},
	m_voidWestGapQuad(rendererGL, 0.5+m_voidHandle.lenght()-0.5*(m_voidHandle.lenght()-m_handleLenght), 0.5+m_handleLenght+0.5/(m_numTrackVoid+1),
					0.5+0.5/(m_numTrackVoid+1), 1.5-0.5/(m_numTrackVoid+1)),
	m_voidWestGapBorder{ShowableLine(rendererGL, 0.5+m_voidHandle.lenght()-0.5*(m_voidHandle.lenght()-m_handleLenght), 0.5+0.5/(m_numTrackVoid+1), 
					0.5+m_handleLenght+0.5/(m_numTrackVoid+1), 0.5+0.5/(m_numTrackVoid+1)),
		ShowableLine(rendererGL, 0.5+m_voidHandle.lenght()-0.5*(m_voidHandle.lenght()-m_handleLenght), 1.5-0.5/(m_numTrackVoid+1),
					1.5+m_handleLenght-0.5/(m_numTrackVoid+1), 1.5-0.5/(m_numTrackVoid+1))},
	
	m_fullEastGapQuad(rendererGL, 0.5, 1.0-0.5*(m_fullHandle.lenght()-m_handleLenght),
					-0.5+0.5/(m_numTrackFull+1), 0.5-0.5/(m_numTrackFull+1)),
	m_fullEastGapBorder{ShowableLine(rendererGL, 0.5, -0.5+0.5/(m_numTrackFull+1),
					1.0-0.5*(m_fullHandle.lenght()-m_handleLenght), -0.5+0.5/(m_numTrackFull+1)),
		ShowableLine(rendererGL, 0.5, 0.5-0.5/(m_numTrackFull+1),
					1.0-0.5*(m_fullHandle.lenght()-m_handleLenght), 0.5-0.5/(m_numTrackFull+1))},
	m_fullWestGapQuad(rendererGL, 1.0+m_fullHandle.lenght()-0.5*(m_fullHandle.lenght()-m_handleLenght), 1.5+m_handleLenght+0.5/(m_numTrackFull+1),
					-0.5+0.5/(m_numTrackFull+1), 0.5-0.5/(m_numTrackFull+1)),
	m_fullWestGapBorder{ShowableLine(rendererGL, 1.0+m_fullHandle.lenght()-0.5*(m_fullHandle.lenght()-m_handleLenght), -0.5+0.5/(m_numTrackFull+1), 
					1.5+m_handleLenght+0.5/(m_numTrackFull+1), -0.5+0.5/(m_numTrackFull+1)),
		ShowableLine(rendererGL, 1.0+m_fullHandle.lenght()-0.5*(m_fullHandle.lenght()-m_handleLenght), 0.5-0.5/(m_numTrackFull+1),
					2.5+m_handleLenght-0.5/(m_numTrackFull+1), 0.5-0.5/(m_numTrackFull+1))}
{
	assert(pairing.size() == 2*(m_numTrackVoid+m_numTrackFull));
	
	zeroHandle.m_renderer = this;
	for (unsigned int i=0; i<pairing.size(); i++) {
		unsigned int j = pairing[i];
		
		std::pair<std::map<ZeroHandle::UnorderedIdempotentsPair, TrackBezier>::iterator, bool> res = m_pairTrackMap.insert(
				std::make_pair(ZeroHandle::UnorderedIdempotentsPair(i, j), TrackBezier()));
		if (res.second) {
			float p0x, p0y, p1x, p1y;
			Direction d0, d1;
			
			if (i>j) std::swap(i, j);
			
			if (i < m_numTrackFull) {
				d0  = EAST;
				p0x = 0.5;
				p0y = 0.5 - (1.0*(i+1)) / (m_numTrackFull+1);
			} else if (i < m_numTrackFull + m_numTrackVoid) {
				d0  = SOUTH;
				p0x = (1.0*(i-m_numTrackFull+1)) / (m_numTrackVoid+1) - 0.5;
				p0y = -0.5;
			} else if (i < 2*m_numTrackFull + m_numTrackVoid) {
				d0  = WEST;
				p0x = -0.5;
				p0y = 0.5 - (1.0*(i-m_numTrackFull-m_numTrackVoid+1)) / (m_numTrackFull+1);
			} else {
				d0  = NORTH;
				p0x = (1.0*(i-2*m_numTrackFull-m_numTrackVoid+1)) / (m_numTrackVoid+1) - 0.5;
				p0y = 0.5;
			}
			
			if (j < m_numTrackFull) {
				d1  = EAST;
				p1x = 0.5;
				p1y = 0.5 - (1.0*(j+1)) / (m_numTrackFull+1);
			} else if (j < m_numTrackFull + m_numTrackVoid) {
				d1  = SOUTH;
				p1x = (1.0*(j-m_numTrackFull+1)) / (m_numTrackVoid+1) - 0.5;
				p1y = -0.5;
			} else if (j < 2*m_numTrackFull + m_numTrackVoid) {
				d1  = WEST;
				p1x = -0.5;
				p1y = 0.5 - (1.0*(j-m_numTrackFull-m_numTrackVoid+1)) / (m_numTrackFull+1);
			} else {
				d1  = NORTH;
				p1x = (1.0*(j-2*m_numTrackFull-m_numTrackVoid+1)) / (m_numTrackVoid+1) - 0.5;
				p1y = 0.5;
			}
			
			if (d0!=d1)
				res.first->second = std::move(TrackBezier(rendererGL, p0x, p0y, p1x, p1y, trackColor.r, trackColor.g, trackColor.b, 0, d0, d1));
			else {
				float tx, ty;
				getUnitTangentFromDirection(d0, tx, ty);
				tx /= 2*(m_numTrackVoid+1); tx = -tx;
				ty /= 2*(m_numTrackFull+1); ty = -ty;
				res.first->second = std::move(TrackBezier(rendererGL, p0x, p0y, p1x, p1y, tx, ty, tx, ty, trackColor.r, trackColor.g, trackColor.b, 0));
			}
		}
	}
	
	m_quad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_quad.show(rendererGL);
	for (int i=0; i<8; i++) {
		m_line[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_line[i].show(rendererGL);
	}
	
	m_southQuad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_southQuad.show(rendererGL);
	for (int i=0; i<2; i++) {
		m_southBorder[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_southBorder[i].show(rendererGL);
	}
	
	m_westQuad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_westQuad.show(rendererGL);
	for (int i=0; i<2; i++) {
		m_westBorder[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_westBorder[i].show(rendererGL);
	}
	
	m_northQuad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_northQuad.show(rendererGL);
	for (int i=0; i<2; i++) {
		m_northBorder[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_northBorder[i].show(rendererGL);
	}
	
	m_lowerVoidHandleQuad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_lowerVoidHandleQuad.show(rendererGL);
	for (int i=0; i<2; i++) {
		m_lowerVoidHandleBorder[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_lowerVoidHandleBorder[i].show(rendererGL);
	}
	
	m_lowerFullHandleQuad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_lowerFullHandleQuad.show(rendererGL);
	for (int i=0; i<2; i++) {
		m_lowerFullHandleBorder[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_lowerFullHandleBorder[i].show(rendererGL);
	}
	
	m_leftFullHandleQuad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_leftFullHandleQuad.show(rendererGL);
	for (int i=0; i<2; i++) {
		m_leftFullHandleBorder[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_leftFullHandleBorder[i].show(rendererGL);
	}
	
	m_rightFullHandleQuad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_rightFullHandleQuad.show(rendererGL);
	for (int i=0; i<2; i++) {
		m_rightFullHandleBorder[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_rightFullHandleBorder[i].show(rendererGL);
	}
	
	m_rightVoidHandleQuad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_rightVoidHandleQuad.show(rendererGL);
	for (int i=0; i<2; i++) {
		m_rightVoidHandleBorder[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_rightVoidHandleBorder[i].show(rendererGL);
	}
	
	m_westTracks.reserve(m_numTrackFull);
	m_upperLeftCornerFullHandleTracks.reserve(m_numTrackFull);
	m_leftFullHandleTracks.reserve(m_numTrackFull);
	m_lowerLeftCornerFullHandleTracks.reserve(m_numTrackFull);
	m_lowerFullHandleTracks.reserve(m_numTrackFull);
	m_lowerRightCornerFullHandleTracks.reserve(m_numTrackFull);
	m_rightFullHandleTracks.reserve(m_numTrackFull);
	m_upperRightCornerFullHandleTracks.reserve(m_numTrackFull);
	m_fullEastGapTracks.reserve(m_numTrackFull);
	m_fullWestGapTracks.reserve(m_numTrackFull);
	for (unsigned int j=0; j<m_numTrackFull; j++) {
		const unsigned int i=j+1;
		const float delta = 1.0/(m_numTrackFull+1);
		m_westTracks.emplace_back(rendererGL, -0.5-0.5*delta, 0.5-i*delta, -0.5, 0.5-i*delta, trackColor.r, trackColor.g, trackColor.b, 0);
		m_upperLeftCornerFullHandleTracks.emplace_back(rendererGL, -1.5+i*delta, -0.5+0.5*delta, -0.5-0.5*delta, 0.5-i*delta,
								trackColor.r, trackColor.g, trackColor.b, 0, NORTH, WEST);
		m_leftFullHandleTracks.emplace_back(rendererGL, -1.5+i*delta, -1.5-0.5*delta, -1.5+i*delta, -0.5+0.5*delta, trackColor.r, trackColor.g, trackColor.b, 0);
		m_lowerLeftCornerFullHandleTracks.emplace_back(rendererGL, -0.5-0.5*delta, -2.5+i*delta, -1.5+i*delta, -1.5-0.5*delta,
								trackColor.r, trackColor.g, trackColor.b, 0, WEST, SOUTH);
		m_lowerFullHandleTracks.emplace_back(rendererGL, m_handleLenght+1.5+0.5*delta, -2.5+i*delta, -0.5-0.5*delta, -2.5+i*delta,
								trackColor.r, trackColor.g, trackColor.b, 0);
		m_lowerRightCornerFullHandleTracks.emplace_back(rendererGL, m_handleLenght+2.5-i*delta, -1.5-0.5*delta, m_handleLenght+1.5+0.5*delta, -2.5+i*delta,
								trackColor.r, trackColor.g, trackColor.b, 0, SOUTH, EAST);
		m_rightFullHandleTracks.emplace_back(rendererGL, m_handleLenght+2.5-i*delta, -0.5+0.5*delta, m_handleLenght+2.5-i*delta, -1.5-0.5*delta,
								trackColor.r, trackColor.g, trackColor.b, 0);
		m_upperRightCornerFullHandleTracks.emplace_back(rendererGL, m_handleLenght+1.5+0.5*delta, 0.5-i*delta, m_handleLenght+2.5-i*delta, -0.5+0.5*delta,
								trackColor.r, trackColor.g, trackColor.b, 0, WEST, NORTH);
		
		m_fullEastGapTracks.emplace_back(rendererGL, 1.0-0.5*(m_fullHandle.lenght()-m_handleLenght), 0.5-i*delta, 0.5, 0.5-i*delta,
								trackColor.r, trackColor.g, trackColor.b, 2);
		m_fullWestGapTracks.emplace_back(rendererGL, 1.0+m_fullHandle.lenght()-0.5*(m_fullHandle.lenght()-m_handleLenght), 0.5-i*delta,
								1.5+m_handleLenght+0.5*delta, 0.5-i*delta, trackColor.r, trackColor.g, trackColor.b, 2);
	}
	
	m_southTracks.reserve(m_numTrackVoid);
	m_lowerLeftCornerVoidHandleTracks.reserve(m_numTrackVoid);
	m_lowerVoidHandleTracks.reserve(m_numTrackVoid);
	m_lowerRightCornerVoidHandleTracks.reserve(m_numTrackVoid);
	m_rightVoidHandleTracks.reserve(m_numTrackVoid);
	m_upperRightCornerVoidHandleTracks.reserve(m_numTrackVoid);
	m_upperLeftCornerVoidHandleTracks.reserve(m_numTrackVoid);
	m_northTracks.reserve(m_numTrackVoid);
	m_voidEastGapTracks.reserve(m_numTrackVoid);
	m_voidWestGapTracks.reserve(m_numTrackVoid);
	for (unsigned int j=0; j<m_numTrackVoid; j++) {
		const unsigned int i=j+1;
		const float delta = 1.0/(m_numTrackVoid+1);
		m_southTracks.emplace_back(rendererGL, -0.5+i*delta, -0.5-0.5*delta, -0.5+i*delta, -0.5, trackColor.r, trackColor.g, trackColor.b, 0);
		m_lowerLeftCornerVoidHandleTracks.emplace_back(rendererGL, 0.5-0.5*delta, -1.5+i*delta, -0.5+i*delta, -0.5-0.5*delta,
								trackColor.r, trackColor.g, trackColor.b, 0, WEST, SOUTH);
		m_lowerVoidHandleTracks.emplace_back(rendererGL, m_handleLenght+0.5+0.5*delta, -1.5+i*delta, 0.5-0.5*delta, -1.5+i*delta,
								trackColor.r, trackColor.g, trackColor.b, 0);
		m_lowerRightCornerVoidHandleTracks.emplace_back(rendererGL, m_handleLenght+1.5-i*delta, -0.5-0.5*delta, m_handleLenght+0.5+0.5*delta, -1.5+i*delta,
								trackColor.r, trackColor.g, trackColor.b, 0, SOUTH, EAST);
		m_rightVoidHandleTracks.emplace_back(rendererGL, m_handleLenght+1.5-i*delta, 0.5+0.5*delta, m_handleLenght+1.5-i*delta, -0.5-0.5*delta,
								trackColor.r, trackColor.g, trackColor.b, 0);
		m_upperRightCornerVoidHandleTracks.emplace_back(rendererGL, m_handleLenght+0.5+0.5*delta, 1.5-i*delta, m_handleLenght+1.5-i*delta, 0.5+0.5*delta,
								trackColor.r, trackColor.g, trackColor.b, 0, WEST, NORTH);
		m_upperLeftCornerVoidHandleTracks.emplace_back(rendererGL, 0.5-0.5*delta, 1.5-i*delta, -0.5+i*delta, 0.5+0.5*delta,
								trackColor.r, trackColor.g, trackColor.b, 0, WEST, NORTH);
		m_northTracks.emplace_back(rendererGL, -0.5+i*delta, 0.5+0.5*delta, -0.5+i*delta, 0.5, trackColor.r, trackColor.g, trackColor.b, 0);
		
		m_voidEastGapTracks.emplace_back(rendererGL, 0.5-0.5*delta, 1.5-i*delta, 0.5-0.5*(m_voidHandle.lenght()-m_handleLenght), 1.5-i*delta,
								trackColor.r, trackColor.g, trackColor.b, 0);
		m_voidWestGapTracks.emplace_back(rendererGL, 0.5+m_handleLenght+0.5*delta, 1.5-i*delta,
								0.5+m_voidHandle.lenght()-0.5*(m_voidHandle.lenght()-m_handleLenght), 1.5-i*delta, trackColor.r, trackColor.g, trackColor.b, 0);
	}
	
	m_voidEastGapQuad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_voidEastGapQuad.show(rendererGL);
	m_voidWestGapQuad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_voidWestGapQuad.show(rendererGL);
	for (int i=0; i<2; i++) {
		m_voidEastGapBorder[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_voidEastGapBorder[i].show(rendererGL);
		m_voidWestGapBorder[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_voidWestGapBorder[i].show(rendererGL);
	}
	
	m_fullEastGapQuad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_fullEastGapQuad.setLayer(2);
	m_fullEastGapQuad.show(rendererGL);
	m_fullWestGapQuad.setColor(handleColor.r, handleColor.g, handleColor.b);
	m_fullWestGapQuad.setLayer(2);
	m_fullWestGapQuad.show(rendererGL);
	for (int i=0; i<2; i++) {
		m_fullEastGapBorder[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_fullEastGapBorder[i].setLayer(2);
		m_fullEastGapBorder[i].show(rendererGL);
		m_fullWestGapBorder[i].setColor(borderColor.r, borderColor.g, borderColor.b);
		m_fullWestGapBorder[i].setLayer(2);
		m_fullWestGapBorder[i].show(rendererGL);
	}
	
	m_voidHandle.changeBasePoints(0.5-0.5*(m_voidHandle.lenght()-m_handleLenght), 1.5);
	m_fullHandle.changeBasePoints(1.0-0.5*(m_fullHandle.lenght()-m_handleLenght), 0.5);
}

void ZeroHandleRenderer::updateLenght() {
	m_handleLenght = std::max(m_voidHandle.lenght(), m_fullHandle.lenght()-1.0f);
	
	const float deltaFull = 1.0/(m_westTracks.size()+1);
	const float diffFull  = m_handleLenght-m_fullHandle.lenght();
	const float deltaVoid = 1.0/(m_southTracks.size()+1);
	const float diffVoid  = m_handleLenght-m_voidHandle.lenght();
	
	m_lowerVoidHandleQuad.changePoints(-0.5+0.5*deltaVoid, m_handleLenght+1.5-0.5*deltaVoid, -1.5+0.5*deltaVoid, -0.5-0.5*deltaVoid);
	m_lowerVoidHandleBorder[0].changePoints(-0.5+0.5*deltaVoid, -1.5+0.5*deltaVoid, m_handleLenght+1.5-0.5*deltaVoid, -1.5+0.5*deltaVoid);
	m_lowerVoidHandleBorder[1].changePoints(0.5-0.5*deltaVoid, -0.5-0.5*deltaVoid, m_handleLenght+0.5+0.5*deltaVoid, -0.5-0.5*deltaVoid);
	m_lowerFullHandleQuad.changePoints(-1.5+0.5*deltaFull, m_handleLenght+2.5-0.5*deltaFull, -2.5+0.5*deltaFull, -1.5-0.5*deltaFull);
	m_lowerFullHandleBorder[0].changePoints(-1.5+0.5*deltaFull, -2.5+0.5*deltaFull, m_handleLenght+2.5-0.5*deltaFull, -2.5+0.5*deltaFull);
	m_lowerFullHandleBorder[1].changePoints(-0.5-0.5*deltaFull, -1.5-0.5*deltaFull, m_handleLenght+1.5+0.5*deltaFull, -1.5-0.5*deltaFull);
	
	m_rightFullHandleQuad.changePoints(m_handleLenght+1.5+0.5*deltaFull, m_handleLenght+2.5-0.5*deltaFull, -1.5-0.5*deltaFull, 0.5-0.5*deltaFull);
	m_rightFullHandleBorder[0].changePoints(m_handleLenght+1.5+0.5*deltaFull, -1.5-0.5*deltaFull, m_handleLenght+1.5+0.5*deltaFull, -0.5+0.5*deltaFull);
	m_rightFullHandleBorder[1].changePoints(m_handleLenght+2.5-0.5*deltaFull, -2.5+0.5*deltaFull, m_handleLenght+2.5-0.5*deltaFull, 0.5-0.5*deltaFull);
	m_rightVoidHandleQuad.changePoints(m_handleLenght+0.5+0.5*deltaVoid, m_handleLenght+1.5-0.5*deltaVoid, -0.5-0.5*deltaVoid, 1.5-0.5*deltaVoid);
	m_rightVoidHandleBorder[0].changePoints(m_handleLenght+0.5+0.5*deltaVoid, -0.5-0.5*deltaVoid, m_handleLenght+0.5+0.5*deltaVoid, 0.5+0.5*deltaVoid); 
	m_rightVoidHandleBorder[1].changePoints(m_handleLenght+1.5-0.5*deltaVoid, -1.5+0.5*deltaVoid, m_handleLenght+1.5-0.5*deltaVoid, 1.5-0.5*deltaVoid);
	
	for (unsigned int j=0; j<m_westTracks.size(); j++) {
		const unsigned int i=j+1;
		m_lowerFullHandleTracks[j].changePoints(m_handleLenght+1.5+0.5*deltaFull, -2.5+i*deltaFull, -0.5-0.5*deltaFull, -2.5+i*deltaFull);
		m_lowerRightCornerFullHandleTracks[j].changePoints(m_handleLenght+2.5-i*deltaFull, -1.5-0.5*deltaFull,
								m_handleLenght+1.5+0.5*deltaFull, -2.5+i*deltaFull, SOUTH, EAST);
		m_rightFullHandleTracks[j].changePoints(m_handleLenght+2.5-i*deltaFull, -0.5+0.5*deltaFull, m_handleLenght+2.5-i*deltaFull, -1.5-0.5*deltaFull);
		m_upperRightCornerFullHandleTracks[j].changePoints(m_handleLenght+1.5+0.5*deltaFull, 0.5-i*deltaFull,
								m_handleLenght+2.5-i*deltaFull, -0.5+0.5*deltaFull, WEST, NORTH);
		
		m_fullEastGapTracks[j].changePoints(1.0+0.5*diffFull, 0.5-i*deltaFull, 0.5, 0.5-i*deltaFull);
		m_fullWestGapTracks[j].changePoints(1.0+m_fullHandle.lenght()+0.5*diffFull, 0.5-i*deltaFull, 1.5+m_handleLenght+0.5*deltaFull, 0.5-i*deltaFull);
	}
	
	for (unsigned int j=0; j<m_southTracks.size(); j++) {
		const unsigned int i=j+1;
		m_lowerVoidHandleTracks[j].changePoints(m_handleLenght+0.5+0.5*deltaVoid, -1.5+i*deltaVoid, 0.5-0.5*deltaVoid, -1.5+i*deltaVoid);
		m_lowerRightCornerVoidHandleTracks[j].changePoints(m_handleLenght+1.5-i*deltaVoid, -0.5-0.5*deltaVoid,
								m_handleLenght+0.5+0.5*deltaVoid, -1.5+i*deltaVoid, SOUTH, EAST);
		m_rightVoidHandleTracks[j].changePoints(m_handleLenght+1.5-i*deltaVoid, 0.5+0.5*deltaVoid, m_handleLenght+1.5-i*deltaVoid, -0.5-0.5*deltaVoid);
		m_upperRightCornerVoidHandleTracks[j].changePoints(m_handleLenght+0.5+0.5*deltaVoid, 1.5-i*deltaVoid,
								m_handleLenght+1.5-i*deltaVoid, 0.5+0.5*deltaVoid, WEST, NORTH);
		
		m_voidEastGapTracks[j].changePoints(0.5+0.5*diffVoid, 1.5-i*deltaVoid, 0.5-0.5*deltaVoid, 1.5-i*deltaVoid);
		m_voidWestGapTracks[j].changePoints(0.5+m_voidHandle.lenght()+0.5*diffVoid, 1.5-i*deltaVoid, 0.5+m_handleLenght+0.5*deltaVoid, 1.5-i*deltaVoid);
	}
	
	m_voidEastGapQuad.changePoints(0.5-0.5*deltaVoid, 0.5+0.5*diffVoid, 0.5+0.5*deltaVoid, 1.5-0.5*deltaVoid);
	m_voidEastGapBorder[0].changePoints(0.5-0.5*deltaVoid, 0.5+0.5*deltaVoid, 0.5+0.5*diffVoid, 0.5+0.5*deltaVoid);
	m_voidEastGapBorder[1].changePoints(-0.5+0.5*deltaVoid, 1.5-0.5*deltaVoid, 0.5+0.5*diffVoid, 1.5-0.5*deltaVoid);
	m_voidWestGapQuad.changePoints(0.5+m_voidHandle.lenght()+0.5*diffVoid, 0.5+m_handleLenght+0.5*deltaVoid, 0.5+0.5*deltaVoid, 1.5-0.5*deltaVoid);
	m_voidWestGapBorder[0].changePoints(0.5+m_voidHandle.lenght()+0.5*diffVoid, 0.5+0.5*deltaVoid, 0.5+m_handleLenght+0.5*deltaVoid, 0.5+0.5*deltaVoid);
	m_voidWestGapBorder[1].changePoints(0.5+m_voidHandle.lenght()+0.5*diffVoid, 1.5-0.5*deltaVoid, 1.5+m_handleLenght-0.5*deltaVoid, 1.5-0.5*deltaVoid);
	
	m_fullEastGapQuad.changePoints(0.5, 1.0+0.5*diffFull, -0.5+0.5*deltaFull, 0.5-0.5*deltaFull);
	m_fullEastGapBorder[0].changePoints(0.5, -0.5+0.5*deltaFull, 1.0+0.5*diffFull, -0.5+0.5*deltaFull);
	m_fullEastGapBorder[1].changePoints(0.5, 0.5-0.5*deltaFull, 1.0+0.5*diffFull, 0.5-0.5*deltaFull);
	m_fullWestGapQuad.changePoints(1.0+m_fullHandle.lenght()+0.5*diffFull, 1.5+m_handleLenght+0.5*deltaFull, -0.5+0.5*deltaFull, 0.5-0.5*deltaFull);
	m_fullWestGapBorder[0].changePoints(1.0+m_fullHandle.lenght()+0.5*diffFull, -0.5+0.5*deltaFull, 1.5+m_handleLenght+0.5*deltaFull, -0.5+0.5*deltaFull);
	m_fullWestGapBorder[1].changePoints(1.0+m_fullHandle.lenght()+0.5*diffFull, 0.5-0.5*deltaFull, 2.5+m_handleLenght-0.5*deltaFull, 0.5-0.5*deltaFull);
	
	m_voidHandle.changeBasePoints(0.5+0.5*diffVoid, 1.5);
	m_fullHandle.changeBasePoints(1.0+0.5*diffFull, 0.5);
}

float ZeroHandleRenderer::getPathLenght(unsigned int trackStartI, unsigned int trackEndI, unsigned int trackStartJ, unsigned int trackEndJ) {
	float tI0, tI1;
	getLenghtPoints(trackStartI, tI0, tI0, tI0, tI0, tI0, tI0, tI0, tI0, tI0);
	tI0 += 1.0;
	getLenghtPoints(trackEndI, tI1, tI1, tI1, tI1, tI1, tI1, tI1, tI1, tI1);
	float sumI = tI0+tI1;
	
	float tJ0, tJ1;
	getLenghtPoints(trackStartJ, tJ0, tJ0, tJ0, tJ0, tJ0, tJ0, tJ0, tJ0, tJ0);
	tJ0 += 1.0;
	getLenghtPoints(trackEndJ, tJ1, tJ1, tJ1, tJ1, tJ1, tJ1, tJ1, tJ1, tJ1);
	float sumJ = tJ0+tJ1;
	
	return 0.5*(sumI+sumJ);
}

float ZeroHandleRenderer::getTAfterZeroHandle(unsigned int trackStartI, unsigned int trackEndI, unsigned int trackStartJ, unsigned int trackEndJ) {
	float tI, tZeroHandleI;
	getLenghtPoints(trackStartI, tZeroHandleI, tZeroHandleI, tZeroHandleI, tZeroHandleI, tZeroHandleI, tZeroHandleI, tZeroHandleI, tZeroHandleI, tZeroHandleI);
	tZeroHandleI += 1.0;
	getLenghtPoints(trackEndI, tI, tI, tI, tI, tI, tI, tI, tI, tI);
	tZeroHandleI /= tZeroHandleI+tI;
	
	float tJ, tZeroHandleJ;
	getLenghtPoints(trackStartJ, tZeroHandleJ, tZeroHandleJ, tZeroHandleJ, tZeroHandleJ, tZeroHandleJ, tZeroHandleJ, tZeroHandleJ, tZeroHandleJ, tZeroHandleJ);
	tZeroHandleJ += 1.0;
	getLenghtPoints(trackEndJ, tJ, tJ, tJ, tJ, tJ, tJ, tJ, tJ, tJ);
	tZeroHandleJ /= tZeroHandleJ+tJ;
	
	if ((trackStartI>trackEndI) == (trackStartJ>trackEndJ))
		return 0.5*(tZeroHandleI+tZeroHandleJ);
	else
		return 0.5*(tZeroHandleI+1.0-tZeroHandleJ);
}

void ZeroHandleRenderer::setArrowPosInZeroHandle(Arrow::Renderer& arrow, unsigned int trackStartI, unsigned int trackEndI,
		unsigned int trackStartJ, unsigned int trackEndJ, float t) {
	float timeI = t; float timeJ = t;
	if (trackStartI > trackEndI) {
		std::swap(trackStartI, trackEndI);
		timeI = 1.0-t;
	}
	if (trackStartJ > trackEndJ) {
		std::swap(trackStartJ, trackEndJ);
		timeJ = 1.0-t;
	}
	
	float tI[19];
	getLenghtPoints(trackStartI, tI[0], tI[1], tI[2], tI[3], tI[4], tI[5], tI[6], tI[7], tI[8]);
	tI[9] = tI[8]+1.0;
	getLenghtPoints(trackEndI, tI[10], tI[11], tI[12], tI[13], tI[14], tI[15], tI[16], tI[17], tI[18]);
	float sum = tI[9]+tI[18];
	tI[0]/=sum; tI[1]/=sum; tI[2]/=sum; tI[3]/=sum; tI[4]/=sum; tI[5]/=sum; tI[6]/=sum; tI[7]/=sum; tI[8]/=sum; tI[9]/=sum;
	tI[10]/=sum; tI[11]/=sum; tI[12]/=sum; tI[13]/=sum; tI[14]/=sum; tI[15]/=sum; tI[16]/=sum; tI[17]/=sum; tI[18]/=sum;
	
	float tJ[19];
	getLenghtPoints(trackStartJ, tJ[0], tJ[1], tJ[2], tJ[3], tJ[4], tJ[5], tJ[6], tJ[7], tJ[8]);
	tJ[9] = tJ[8]+1.0;
	getLenghtPoints(trackEndJ, tJ[10], tJ[11], tJ[12], tJ[13], tJ[14], tJ[15], tJ[16], tJ[17], tJ[18]);
	sum = tJ[9]+tJ[18];
	tJ[0]/=sum; tJ[1]/=sum; tJ[2]/=sum; tJ[3]/=sum; tJ[4]/=sum; tJ[5]/=sum; tJ[6]/=sum; tJ[7]/=sum; tJ[8]/=sum; tJ[9]/=sum;
	tJ[10]/=sum; tJ[11]/=sum; tJ[12]/=sum; tJ[13]/=sum; tJ[14]/=sum; tJ[15]/=sum; tJ[16]/=sum; tJ[17]/=sum; tJ[18]/=sum;
	
	if (timeI==timeJ) {
		for (int i=0; i<19; i++) {
			tI[i] = tJ[i] = 0.5*(tI[i]+tJ[i]);
		}
	} else {
		for (int i=0; i<9; i++) {
			float f = 0.5*(tI[i]+1.0-tJ[10+i]);
			tI[i] = f;
			tJ[10+i] = 1.0-f;
		}
		float f = 0.5*(tI[9]+1.0-tJ[9]);
		tI[9] = f; tJ[9] = 1.0-f;
		for (int i=10; i<19; i++) {
			float f = 0.5*(tI[i]+1.0-tJ[i-10]);
			tI[i] = f;
			tJ[10+i] = 1.0-f;
		}
	}
	
	ShowableCurve& trackI = getInterpolatedTrack(timeI, trackStartI, trackEndI, timeI, tI);
	ShowableCurve& trackJ = getInterpolatedTrack(timeJ, trackStartJ, trackEndJ, timeJ, tJ);
	
	float px0, py0, px1, py1, tx0, ty0, tx1, ty1;
	trackI.getPoint(px0, py0, timeI);
	trackJ.getPoint(px1, py1, timeJ);
	trackI.getNormalToPoint(tx0, ty0, timeI, px1, py1);
	trackJ.getNormalToPoint(tx1, ty1, timeJ, px0, py0);
	arrow.changePoints(px0, py0, px1, py1, tx0, ty0, tx1, ty1);
}

unsigned int ZeroHandleRenderer::getIndexFromTrack(unsigned int track, int edge) {
	if (edge==0) {
		assert(track<m_numTrackFull);
		return track;
	} else if (edge==1) {
		assert(track<m_numTrackVoid);
		return track + m_numTrackFull;
	} else if (edge==2) {
		assert(track<m_numTrackFull);
		return track + (m_numTrackFull+m_numTrackVoid);
	} else {
		assert(track<m_numTrackVoid);
		return track + (2*m_numTrackFull+m_numTrackVoid);
	}
}

int ZeroHandleRenderer::getEdgeFromArrowBox(ArrowBox::Renderer& arrowBox) {
	if (&arrowBox==&(m_fullHandle.getFirstArrowBox()))
		return 0;
	else if (&arrowBox==&(m_voidHandle.getSecondArrowBox()))
		return 1;
	else if (&arrowBox==&(m_fullHandle.getSecondArrowBox()))
		return 2;
	else {
		assert(&arrowBox==&(m_voidHandle.getFirstArrowBox()));
		return 3;		
	}
}
