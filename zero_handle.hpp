#ifndef __ZERO_HANDLE_HPP__
#define __ZERO_HANDLE_HPP__

#include <map>
#include <vector>
#include <list>

class ZeroHandle;
class ZeroHandleRenderer;

#include "permutation.hpp"
#include "track.hpp"
#include "curves.hpp"
#include "surface.hpp"
#include "c_cpp_bridge.h"
#include "draw_gl.hpp"
#include "one_handle.hpp"
#include "display_cmd.hpp"

class ZeroHandle {
private:
	class UnorderedIdempotentsPair {
		unsigned int m_idem0;
		unsigned int m_idem1;
		
	public:
		UnorderedIdempotentsPair(unsigned int idem0, unsigned int idem1) : m_idem0((idem0<idem1) ? idem0 : idem1), m_idem1((idem0<idem1) ? idem1 : idem0) {}
		
		bool operator<(const UnorderedIdempotentsPair& other) const {return (m_idem0==other.m_idem0) ? (m_idem1<other.m_idem1) : (m_idem0<other.m_idem0);}
	};
	
	const unsigned int m_numTrackVoid;
	const unsigned int m_numTrackFull;
	Pairing m_pairing;
	ZeroHandleRenderer* m_renderer = nullptr;
	OneHandle m_voidHandle;
	OneHandle m_fullHandle;
	DeterministMarkovPower m_markov;
	
	std::pair<unsigned int, int> getIndexEdgeFromIndex(unsigned int index) const;
	unsigned int getMarkovIndex(unsigned int i, unsigned int j, int edge) const;
	unsigned int getMarkovIndex(unsigned int i, unsigned int j, const OneHandle& oneHandle, bool isPost) const;
	unsigned int getCorrectedIndexFromEdge(unsigned int index, int edge) const;
	void buildMarkovPart(DeterministMarkov& markov, int edge) const;
	
public:
	ZeroHandle(unsigned int k, const FUMatrix& matrix, std::list<Arrow>&& voidArrows, std::list<Arrow>&& fullArrows,
			std::list<std::pair<unsigned int, unsigned int>>& voidArrowsData, std::list<ArrowBox::ArrowInArrowBox*>& voidArrowsPtr,
			std::list<std::pair<unsigned int, unsigned int>>& fullArrowsData, std::list<ArrowBox::ArrowInArrowBox*>& fullArrowsPtr);
	
	void updateMarkov();
	bool trackPairEndsClockwise(std::pair<unsigned int, unsigned int> tracks, const OneHandle& oneHandle, bool isPost) const;
	bool trackPairEndsAntiClockwise(std::pair<unsigned int, unsigned int> tracks, const OneHandle& oneHandle, bool isPost) const;
	int  getArrowDepth(std::pair<unsigned int, unsigned int> tracks, const OneHandle& oneHandle, bool isPost) const;
	unsigned int getDepth() const;
	void moveArrowThroughout(PassArrowsThroughtZeroHandleCommand* cmd, ArrowBox& src, std::list<ArrowBox::ArrowInArrowBox*>::iterator arrow, int index,
			unsigned int beginI, unsigned int beginJ);
	
	void proposition28();
	
	const Pairing& getPairing() {return m_pairing;}
	ZeroHandleRenderer& getRenderer() {assert(m_renderer!=nullptr); return *m_renderer;}
	
	OneHandle& getVoidHandle() {return m_voidHandle;}
	unsigned int getVoidSize() const {return m_numTrackVoid;}
	OneHandle& getFullHandle() {return m_fullHandle;}
	unsigned int getFullSize() const {return m_numTrackFull;}
	void printDepth() const;
	
	friend ZeroHandleRenderer;
};

class ZeroHandleRenderer {
private:
	OneHandleRenderer m_voidHandle;
	OneHandleRenderer m_fullHandle;
	float m_handleLenght;
	const unsigned int m_numTrackVoid;
	const unsigned int m_numTrackFull;
	
	std::map<ZeroHandle::UnorderedIdempotentsPair, TrackBezier> m_pairTrackMap;
	ShowableLine m_line[8];
	Quad m_quad;
	
	Quad m_southQuad;
	ShowableLine m_southBorder[2];
	Quad m_westQuad;
	ShowableLine m_westBorder[2];
	Quad m_northQuad;
	ShowableLine m_northBorder[2];
	
	Quad m_lowerVoidHandleQuad;
	ShowableLine m_lowerVoidHandleBorder[2];
	Quad m_lowerFullHandleQuad;
	ShowableLine m_lowerFullHandleBorder[2];
	
	Quad m_leftFullHandleQuad;
	ShowableLine m_leftFullHandleBorder[2];
	Quad m_rightFullHandleQuad;
	ShowableLine m_rightFullHandleBorder[2];
	Quad m_rightVoidHandleQuad;
	ShowableLine m_rightVoidHandleBorder[2];
	
	std::vector<TrackLine> m_southTracks;
	std::vector<TrackLine> m_westTracks;
	std::vector<TrackLine> m_northTracks;
	
	std::vector<TrackLine> m_lowerVoidHandleTracks;
	std::vector<TrackLine> m_lowerFullHandleTracks;
	
	std::vector<TrackLine> m_leftFullHandleTracks;
	std::vector<TrackLine> m_rightFullHandleTracks;
	std::vector<TrackLine> m_rightVoidHandleTracks;
	
	std::vector<TrackBezier> m_upperLeftCornerFullHandleTracks;
	std::vector<TrackBezier> m_lowerLeftCornerFullHandleTracks;
	std::vector<TrackBezier> m_lowerRightCornerFullHandleTracks;
	std::vector<TrackBezier> m_upperRightCornerFullHandleTracks;
	
	std::vector<TrackBezier> m_upperLeftCornerVoidHandleTracks;
	std::vector<TrackBezier> m_lowerLeftCornerVoidHandleTracks;
	std::vector<TrackBezier> m_lowerRightCornerVoidHandleTracks;
	std::vector<TrackBezier> m_upperRightCornerVoidHandleTracks;
	
	Quad m_voidEastGapQuad;
	ShowableLine m_voidEastGapBorder[2];
	Quad m_voidWestGapQuad;
	ShowableLine m_voidWestGapBorder[2];
	
	Quad m_fullEastGapQuad;
	ShowableLine m_fullEastGapBorder[2];
	Quad m_fullWestGapQuad;
	ShowableLine m_fullWestGapBorder[2];
	
	std::vector<TrackLine> m_voidEastGapTracks;
	std::vector<TrackLine> m_voidWestGapTracks;
	std::vector<TrackLine> m_fullEastGapTracks;
	std::vector<TrackLine> m_fullWestGapTracks;
	
	void getLenghtPoints(unsigned int i, float& t0, float& t1, float& t2, float& t3, float& t4, float& t5, float& t6, float& t7, float& t8);
	ShowableCurve& getInterpolatedTrack(float& newT, unsigned int trackStart, unsigned int trackEnd, float time, const float t[19]);
	
public:
	ZeroHandleRenderer(RendererGL& rendererGL, const Pairing& pairing, ZeroHandle& zeroHandle,
			std::list<std::pair<unsigned int, unsigned int>>& voidArrowsData, std::list<ArrowBox::ArrowInArrowBox*>& voidArrows,
			std::list<std::pair<unsigned int, unsigned int>>& fullArrowsData, std::list<ArrowBox::ArrowInArrowBox*>& fullArrows);
	
	void updateLenght();
	float lenght() const {return m_handleLenght;}
	OneHandleRenderer& getVoidHandle() {return m_voidHandle;}
	OneHandleRenderer& getFullHandle() {return m_fullHandle;}
	unsigned int getNumberOfTrackVoid() const {return m_numTrackVoid;}
	unsigned int getNumberOfTrackFull() const {return m_numTrackFull;}
	float getPathLenght(unsigned int trackStartI, unsigned int trackEndI, unsigned int trackStartJ, unsigned int trackEndJ);
	float getTAfterZeroHandle(unsigned int trackStartI, unsigned int trackEndI, unsigned int trackStartJ, unsigned int trackEndJ);
	void setArrowPosInZeroHandle(Arrow::Renderer& arrow, unsigned int trackStartI, unsigned int trackEndI,
		unsigned int trackStartJ, unsigned int trackEndJ, float t);
	unsigned int getIndexFromTrack(unsigned int track, int edge);
	int getEdgeFromArrowBox(ArrowBox::Renderer& arrowBox);
};

#endif // __ZERO_HANDLE_HPP__
