#ifndef __ONE_HANDLE_HPP__
#define __ONE_HANDLE_HPP__

#include <vector>
#include <list>
#include <limits>
#include <functional>

#include "arrow.hpp"
#include "permutation.hpp"
#include "draw_gl.hpp"
#include "curves.hpp"
#include "surface.hpp"
#include "track.hpp"
#include "color.h"

class ZeroHandle;
class ZeroHandleRenderer;

class AnimateMoveTrackLineArrowBox;
class AnimateMoveTrackPermutationBox;
class AnimateMoveArrowInArrowBoxCommand;
class AsynchronousSubTaskCommand;
class GenCrossingCommand;
class MoveCrossingCommand;
class FadeCrossingCommand;
class UpdateArrowBoxLenghtCommand;
class PassArrowsThroughtZeroHandleCommand;
class OneHandle;
class OneHandleRenderer;

class PermutationBox {
public:
	class Renderer {
		std::vector<TrackBezier> m_tracks;
		Permutation m_inv_per;
		float m_basex, m_basey;
		int m_layer;
		OneHandleRenderer& m_oneHandle;
		
	public:
		Renderer(RendererGL& rendererGL, float basex, float basey, int layer, OneHandleRenderer& oneHandle);
		
		void changeBasePoints(float basex, float basey);
		void permute(const BiPermutation& permutation, AnimateMoveTrackPermutationBox* animPermutation, bool isAfter);
		void permute(std::pair<unsigned int, unsigned int> permutation, AnimateMoveTrackPermutationBox* animPermutation, bool isAfter);
		void refresh();
		TrackBezier& getTrack(unsigned int rightIndex) {assert(rightIndex<m_tracks.size()); return m_tracks[rightIndex];}
		const TrackBezier& getTrack(unsigned int rightIndex) const {assert(rightIndex<m_tracks.size()); return m_tracks[rightIndex];}
		unsigned int pre(unsigned int val) const {assert(val<m_inv_per.size()); return m_inv_per[val];}
		Bezier getBezier(unsigned int left, unsigned int right) const;
		
		float getBasex() const {return m_basex;}
		float getBasey() const {return m_basey;}
		float lenght()   const {return 0.5;}
		int getLayer() const {return m_layer;}
	};
	
private:
	BiPermutation m_permutation;
	
public:
	PermutationBox(unsigned int m_numberOfTrackss) : m_permutation(m_numberOfTrackss) {}
	
	void permute(const BiPermutation& permutation, bool isAfter);
	void permute(std::pair<unsigned int, unsigned int> permutation, bool isAfter);
	unsigned int pre (unsigned int val) const {return m_permutation.pre(val);}
	unsigned int post(unsigned int val) const {return m_permutation.post(val);}
	
	friend Renderer;
};

class ArrowBox {
public:
	class Renderer;
	typedef std::list<Arrow::Renderer>::iterator ArrowRendererInList;
	
	class ArrowInArrowBox {
		Arrow m_arrow;
		ArrowRendererInList m_arrowRendererInList;
#ifndef NDEBUG
		bool m_arrowRendererInListSet = false;
#endif
		
		void setArrowRendererInList(ArrowRendererInList arrowRendererInList) {
#ifndef NDEBUG
			m_arrowRendererInListSet = true;
#endif
			m_arrowRendererInList = arrowRendererInList;
		}
		
	public:
		ArrowInArrowBox() = delete;
		ArrowInArrowBox(Arrow arrow, ArrowRendererInList arrowRendererInList) : m_arrow(arrow), m_arrowRendererInList(arrowRendererInList) {}
		ArrowInArrowBox(const ArrowInArrowBox& other) = delete;
		ArrowInArrowBox(ArrowInArrowBox&& other) : m_arrow(std::move(other.m_arrow)) {
			m_arrowRendererInList = std::move(other.m_arrowRendererInList);
#ifndef NDEBUG
			m_arrowRendererInListSet = other.m_arrowRendererInListSet;
			other.m_arrowRendererInListSet = false;
#endif
		}
		
#ifndef NDEBUG
		bool arrowInListSet() const {return m_arrowRendererInListSet;}
#endif
		
		Arrow& getArrow() {return m_arrow;}
		ArrowRendererInList getArrowRendererInList() const {assert(m_arrowRendererInListSet); return m_arrowRendererInList;}
		
		friend Renderer;
	};
	
	class Renderer {
	public:
		struct Crossing {
			TrackBezier m_crossA;
			TrackBezier m_crossB;
			TrackLine m_lineA, m_lineB;
			
			Crossing(RendererGL& rendererGL, int layer) :
				m_crossA(rendererGL, 1e+10, 1e+10, 1e+10, 1e+10, trackColor.r, trackColor.g, trackColor.b, layer, EAST, WEST),
				m_crossB(rendererGL, 1e+10, 1e+10, 1e+10, 1e+10, trackColor.r, trackColor.g, trackColor.b, layer, EAST, WEST),
				m_lineA (rendererGL, 1e+10, 1e+10, 1e+10, 1e+10, trackColor.r, trackColor.g, trackColor.b, layer),
				m_lineB (rendererGL, 1e+10, 1e+10, 1e+10, 1e+10, trackColor.r, trackColor.g, trackColor.b, layer) {}
		};
		
	private:
		std::vector<TrackLine> m_tracks;
		float m_basex, m_basey, m_lenght;
		OneHandleRenderer& m_oneHandle;
		std::list<Arrow::Renderer> m_arrow;
		int m_layer;
		Crossing* m_crossing = nullptr;
		unsigned int m_crossingI, m_crossingJ;
		size_t m_crossingIndex;
		ArrowRendererInList m_crossingPrevArrow;
		
		void doMoveArrow(std::list<Arrow::Renderer>::iterator it, std::list<Arrow::Renderer>::iterator it1,
			int index, int n, float start, float end, ArrowBox::Renderer& src, AnimateMoveArrowInArrowBoxCommand* anim, bool sameBox, bool singleArrow);
		
	public:
		Renderer(RendererGL& rendererGL, ArrowBox& arrowBox, float basex, float basey, int layer, OneHandleRenderer& oneHandle, 
				 std::list<std::pair<unsigned int, unsigned int>>& arrowsData, std::list<ArrowBox::ArrowInArrowBox*>& arrows);
		Renderer(RendererGL& rendererGL, ArrowBox& arrowBox, float basex, float basey, int layer, OneHandleRenderer& oneHandle);
		
		ArrowRendererInList begin() {return m_arrow.begin();}
		ArrowRendererInList end()   {return m_arrow.end();}
		size_t size() const {return m_arrow.size();}
		OneHandleRenderer& getOneHandle() {return m_oneHandle;}
		
		void changeBasePoints(float basex, float basey, bool changeArrows);
		void permuteTracks(const Permutation& permutation, AnimateMoveTrackLineArrowBox* animArrowTracks, AnimateMoveArrowInArrowBoxCommand* animMoveArrow);
		void moveArrowsFake(int beginIndexOffset, int endIndexOffset, AnimateMoveArrowInArrowBoxCommand* anim);
		
		void moveArrow(ArrowBox::ArrowRendererInList it, int index, int n, AnimateMoveArrowInArrowBoxCommand* anim) {
			doMoveArrow(it, it, index, n, static_cast<float>(index), static_cast<float>(index+n), *this, anim, true, false);
		}
		
		void moveArrowToEnd(ArrowBox::ArrowRendererInList it, int index, Renderer& src, AnimateMoveArrowInArrowBoxCommand* anim) {
			doMoveArrow(it, it, index, m_arrow.size()-index-1, static_cast<float>(index), static_cast<float>(m_arrow.size())-0.5, src, anim, &src==this, false);
		}
		void moveArrowToEndFake(ArrowBox::ArrowRendererInList it, int index, Renderer& src, AnimateMoveArrowInArrowBoxCommand* anim) {
			doMoveArrow(it, it, index, 0, static_cast<float>(index), static_cast<float>(m_arrow.size()-1), src, anim, &src==this, true);
		}
		
		void moveArrowToBegin(ArrowBox::ArrowRendererInList it, int index, Renderer& src, AnimateMoveArrowInArrowBoxCommand* anim) {
			doMoveArrow(it, it, index, -index, static_cast<float>(index), -0.5, src, anim, &src==this, false);
		}
		void moveArrowToBeginFake(ArrowBox::ArrowRendererInList it, int index, Renderer& src, AnimateMoveArrowInArrowBoxCommand* anim) {
			doMoveArrow(it, it, index, 0, static_cast<float>(index), -0.5, src, anim, &src==this, true);
		}
		
		void moveArrowFromEnd(ArrowBox::ArrowRendererInList it, int n, Renderer& src, AnimateMoveArrowInArrowBoxCommand* anim) {
			doMoveArrow(it, --m_arrow.end(), m_arrow.size(), -n, static_cast<float>(m_arrow.size())-0.5, static_cast<float>(m_arrow.size()-1-n),
					src, anim, &src==this, false);
		}
		void moveArrowFromEndFake(ArrowBox::ArrowRendererInList it, int n, Renderer& src, AnimateMoveArrowInArrowBoxCommand* anim) {
			doMoveArrow(it, m_arrow.end(), m_arrow.size(), 0, static_cast<float>(m_arrow.size()+n)+0.5, static_cast<float>(m_arrow.size()),
					src, anim, &src==this, true);
		}
		
		void moveArrowFromBegin(ArrowBox::ArrowRendererInList it, int n, Renderer& src, AnimateMoveArrowInArrowBoxCommand* anim) {
			doMoveArrow(it, m_arrow.begin(), 0, n, -0.5, static_cast<float>(n), src, anim, &src==this, false);
		}
		void moveArrowFromBeginFake(ArrowBox::ArrowRendererInList it, int n, Renderer& src, AnimateMoveArrowInArrowBoxCommand* anim) {
			doMoveArrow(it, m_arrow.begin(), 0, 0, -0.5, static_cast<float>(n), src, anim, &src==this, true);
		}
		void moveArrowFromBeginWithOthers(ArrowBox::ArrowRendererInList it, int n, Renderer& src, AnimateMoveArrowInArrowBoxCommand* anim) {
			doMoveArrow(it, m_arrow.end(), m_arrow.size(), n-m_arrow.size(), -0.5, static_cast<float>(n), src, anim, &src==this, false);
		}
		
		void spawnArrowAfterCrossingForward(RendererGL& rendererGL, ArrowBox::ArrowRendererInList it, ArrowInArrowBox& newArrow, int index,
				unsigned int from, unsigned int to, AnimateMoveArrowInArrowBoxCommand* anim0,
				AnimateMoveArrowInArrowBoxCommand* anim1, UpdateArrowBoxLenghtCommand* lenAnim1, bool spawnAfter);
		void spawnArrowAfterCrossingBackward(RendererGL& rendererGL, ArrowBox::ArrowRendererInList it, ArrowInArrowBox& newArrow, int index,
				unsigned int from, unsigned int to, AnimateMoveArrowInArrowBoxCommand* anim0,
				AnimateMoveArrowInArrowBoxCommand* anim1, UpdateArrowBoxLenghtCommand* lenAnim1, bool spawnAfter);
		void mergeArrows(RendererGL& rendererGL, ArrowBox::ArrowRendererInList it, int index,
				AnimateMoveArrowInArrowBoxCommand* anim0, UpdateArrowBoxLenghtCommand* lenAnim0,
				AnimateMoveArrowInArrowBoxCommand* anim1, UpdateArrowBoxLenghtCommand* lenAnim1, bool after);
		void removeArrow(RendererGL& rendererGL, ArrowBox::ArrowRendererInList it, int index,
				AnimateMoveArrowInArrowBoxCommand* anim0, UpdateArrowBoxLenghtCommand* lenAnim0);
		void genCrossing(RendererGL& rendererGL, ArrowBox::ArrowRendererInList arrow, int index, AnimateMoveArrowInArrowBoxCommand* anim,
				GenCrossingCommand*& genAnim, std::vector<MoveCrossingCommand*>& crossingMoveCommands,
				FadeCrossingCommand*& fadeAnim, unsigned int i, unsigned int j);
		void transferArrowsAsPermutationBoxSlide(ArrowBox::Renderer& target, const PermutationBox::Renderer& permutation,
				AsynchronousSubTaskCommand* cmd, float duration);
		
		void removeArrow(ArrowInArrowBox& arrow) {m_arrow.erase(arrow.getArrowRendererInList()); delete &arrow;}
		void refreshArrows();
		void refreshLenght();
		void refreshTracks();
		std::pair<ArrowRendererInList, int> pushBackArrow(RendererGL& rendererGL, ArrowInArrowBox& newArrow,
				unsigned int from, unsigned int to, bool refresh=true);
		std::pair<ArrowRendererInList, int> pushFrontArrow(RendererGL& rendererGL, ArrowInArrowBox& newArrow,
				unsigned int from, unsigned int to, bool refresh=true);
		ArrowRendererInList transferArrowToBack(Renderer& target, ArrowRendererInList arrow, unsigned int targetI, unsigned int targetJ);
		
		bool empty() const {return m_lenght==0.0;}
		float lenght() const {return m_lenght;}
		void setLenght(float newLenght, bool updateArrows);
		float endX() const {return m_basex+m_lenght;}
		static float arrowSeparation() {return 0.055;}
		float getBasex() const {return m_basex;}
		float getBasey() const {return m_basey;}
		float getRelativeYForTrack(unsigned int index) const;
		
		Crossing& getCrossing() {
			assert(m_crossing!=nullptr);
			return *m_crossing;
		}
		
		std::pair<unsigned int, unsigned int> getCrossingTracksIndexes() const {
			assert(m_crossing!=nullptr);
			return std::make_pair(m_crossingI, m_crossingJ);
		}
		
		std::pair<std::reference_wrapper<TrackLine>, std::reference_wrapper<TrackLine>> getCrossingTracks() {
			assert(m_crossing!=nullptr);
			return std::make_pair(std::reference_wrapper<TrackLine>(m_tracks[m_crossingI]), std::reference_wrapper<TrackLine>(m_tracks[m_crossingJ]));
		}
		
		std::pair<float, float> getCrossingTracksHeight() const;
		
		void deleteCrossing() {
			delete m_crossing;
			m_crossing = nullptr;
		}
		
		void createCrossing(RendererGL& rendererGL, unsigned int i, unsigned int j, size_t crossingIndex, ArrowRendererInList prevArrow, int layer);
	};
	
	class ArrowInArrowBoxIndexedIterator {
		std::list<ArrowInArrowBox*>::iterator m_it;
		size_t m_index;
		
		ArrowInArrowBoxIndexedIterator(std::list<ArrowInArrowBox*>::iterator it, size_t index) : m_it(it), m_index(index) {}
		
	public:
		ArrowInArrowBoxIndexedIterator() : m_index(std::numeric_limits<size_t>::max()) {}
		
		static ArrowInArrowBoxIndexedIterator fromBeginOfBox(ArrowBox& arrowBox);
		static ArrowInArrowBoxIndexedIterator fromEndOfBox  (ArrowBox& arrowBox);
		
		void setIndex(size_t newIndex) {m_index = newIndex;}
		void incIndex() {m_index++;}
		ArrowInArrowBoxIndexedIterator& operator++();
		ArrowInArrowBoxIndexedIterator  operator++(int) {
			ArrowInArrowBoxIndexedIterator tmp(*this);
			operator++();
			return tmp;
		}
		
		void decIndex() {assert(m_index!=0); m_index--;}
		ArrowInArrowBoxIndexedIterator& operator--();
		ArrowInArrowBoxIndexedIterator  operator--(int) {
			ArrowInArrowBoxIndexedIterator tmp(*this);
			operator--();
			return tmp;
		}
		bool operator==(const ArrowInArrowBoxIndexedIterator& other) const;
		bool operator!=(const ArrowInArrowBoxIndexedIterator& other) const {return !operator==(other);}
		bool operator< (const ArrowInArrowBoxIndexedIterator& other) const {return m_index<other.m_index;}
		bool operator> (const ArrowInArrowBoxIndexedIterator& other) const {return other<(*this);}
		bool operator<=(const ArrowInArrowBoxIndexedIterator& other) const {return !(*this>other);}
		bool operator>=(const ArrowInArrowBoxIndexedIterator& other) const {return !(*this<other);}
		ArrowInArrowBox* operator*() {return *m_it;}
		std::list<ArrowInArrowBox*>::iterator getIterator() const {return m_it;}
		size_t getPos() const {return m_index;}
		bool checkValidity(ArrowBox& arrowBox) {return m_it==arrowBox.get(m_index);}
		
		ArrowInArrowBoxIndexedIterator eraseFromArrowBox(ArrowBox& arrowBox);
		ArrowInArrowBoxIndexedIterator insertInArrowBox(ArrowInArrowBox* arrow, ArrowBox& arrowBox);
		void transfer(ArrowInArrowBoxIndexedIterator& other, ArrowBox& arrowBox);
	};
	
private:
	std::list<ArrowInArrowBox*> m_arrows;
	unsigned int m_numberOfTracks;
	Renderer* m_renderer = nullptr;
	
	void lemma29MovePastArrow(ArrowInArrowBox& movingArrow, unsigned int& movingIndex, ArrowInArrowBoxIndexedIterator& arrow,
			ArrowInArrowBoxIndexedIterator* last, ArrowInArrowBoxIndexedIterator& end);
	void doLemma29(ArrowInArrowBoxIndexedIterator& begin, ArrowInArrowBoxIndexedIterator checkStart,
					unsigned int k, bool shouldGoDeeper, bool strictK, ArrowInArrowBoxIndexedIterator& end, ArrowInArrowBoxIndexedIterator* last=nullptr);
	void lemma29RemoveSameArrows(ArrowInArrowBoxIndexedIterator& begin, ArrowInArrowBoxIndexedIterator& end);
	
public:
	ArrowBox(std::list<Arrow>&& arrows, unsigned int numberOfTracks) : m_numberOfTracks(numberOfTracks) {
		for (auto it=arrows.begin(); it!=arrows.end(); it++) {
			m_arrows.push_back(new ArrowInArrowBox(*it, ArrowRendererInList()));
		}
	}
	ArrowBox(unsigned int numberOfTracks) : m_numberOfTracks(numberOfTracks) {}
	
	Renderer& getRenderer() {assert(m_renderer!=nullptr); return *m_renderer;}
	
	std::list<ArrowInArrowBox*>::iterator get(size_t index) {auto it = m_arrows.begin(); for(size_t i=0; i<index; i++) it++; return it;}
	void removeDepthMArrows(const OneHandle& oneHandle, const ZeroHandle& zeroHandle, int m, bool to);
	unsigned int getMinimalDepth(const OneHandle& oneHandle, const ZeroHandle& zeroHandle) const;
	ArrowInArrowBoxIndexedIterator removeArrow(ArrowInArrowBoxIndexedIterator arrow);
	void moveArrowsThroughoutZeroHandle(ZeroHandle& zeroHandle, OneHandle& oneHandle, bool fromEnd);
	
	ArrowInArrowBox& pushArrowRet(Arrow arrow) {
		ArrowInArrowBox* arrowInArrowBox = new ArrowInArrowBox(arrow, ArrowRendererInList());
		return **(m_arrows.insert(m_arrows.end(), arrowInArrowBox));
	}
	
	size_t size() const {return m_arrows.size();}
	bool empty() const {return m_arrows.empty();}
	
	std::list<ArrowInArrowBox*>::iterator begin() {return m_arrows.begin();}
	std::list<ArrowInArrowBox*>::iterator end()   {return m_arrows.end();}
	
	void pushBackArrow(Arrow arrow) {
		ArrowInArrowBox* arrowInArrowBox = new ArrowInArrowBox(arrow, ArrowRendererInList());
		m_arrows.push_back(arrowInArrowBox);
	}
	
	void pushFrontArrow(Arrow arrow) {
		ArrowInArrowBox* arrowInArrowBox = new ArrowInArrowBox(arrow, ArrowRendererInList());
		m_arrows.push_front(arrowInArrowBox);
	}
	
	Arrow popBackArrow() {
		Arrow ret(m_arrows.back()->getArrow());
		m_arrows.pop_back();
		return ret;
	}
	
	Arrow popFrontArrow() {
		Arrow ret(m_arrows.front()->getArrow());
		m_arrows.pop_front();
		return ret;
	}
	
	ArrowInArrowBox& back() {return *(m_arrows.back());}
	
	void transferToFrontArrow(std::list<ArrowInArrowBox*>::iterator it, ArrowBox& src) {
		m_arrows.splice(m_arrows.begin(), src.m_arrows, it);
	}
	void transferToBackArrow(std::list<ArrowInArrowBox*>::iterator it, ArrowBox& src) {
		m_arrows.splice(m_arrows.end(), src.m_arrows, it);
	}
	
	void removeArrowGenCrossing(ArrowInArrowBoxIndexedIterator arrow, ArrowInArrowBoxIndexedIterator& crossingPos, PermutationBox& permutationBox);
	void lemma29(ArrowInArrowBoxIndexedIterator& begin, ArrowInArrowBoxIndexedIterator& end, ArrowInArrowBoxIndexedIterator* checkStart=nullptr);
	void permuteTracks(const Permutation& permutation);
	void fillArrowsRendererLists(std::list<std::pair<unsigned int, unsigned int>>& arrowsData, std::list<ArrowInArrowBox*>& arrows);
	
	friend ArrowInArrowBoxIndexedIterator;
};

class OneHandleRenderer {
	unsigned int m_numberOfTracks;
	PermutationBox::Renderer m_prePermutation;
	ArrowBox::Renderer m_arrows0;
	PermutationBox::Renderer m_permutation;
	ArrowBox::Renderer m_arrows1;
	PermutationBox::Renderer m_postPermutation;
	float m_lenght;
	Quad m_quad;
	ShowableLine m_border[2];
	float m_basex, m_basey;
	ZeroHandleRenderer& m_zeroHandle;
	
public:
	OneHandleRenderer(RendererGL& rendererGL, OneHandle& oneHandle, std::list<std::pair<unsigned int, unsigned int>>& arrowsData,
					  std::list<ArrowBox::ArrowInArrowBox*>& arrows, float basex, float basey, int layer, ZeroHandleRenderer& zeroHandle);
	
	PermutationBox::Renderer& getPrePermutation()  {return m_prePermutation;}
	ArrowBox::Renderer&       getFirstArrowBox()   {return m_arrows0;}
	PermutationBox::Renderer& getPermutation()     {return m_permutation;}
	ArrowBox::Renderer&       getSecondArrowBox()  {return m_arrows1;}
	PermutationBox::Renderer& getPostPermutation() {return m_postPermutation;}
	
	void changeBasePoints(float basex, float basey);
	void refresh(bool updateArrows);
	float lenght() const {return m_lenght;}
	unsigned int numberOfTracks() const {return m_numberOfTracks;}
	void updateLenght(bool updateArrows);
	ZeroHandleRenderer& getZeroHandle() {return m_zeroHandle;}
	
	void transferArrowsToFirstArrowBox(AsynchronousSubTaskCommand* cmd, float duration) {
		m_arrows1.transferArrowsAsPermutationBoxSlide(m_arrows0, m_permutation, cmd, duration);
	}
};

class OneHandle {
private:
	class StrandOrderCompareAntiClockwise {
		const ArrowBox& m_arrowBox;
		const OneHandle& m_oneHandle;
		const ZeroHandle& m_zeroHandle;
		
	public:
		StrandOrderCompareAntiClockwise(const ArrowBox& arrowBox, const OneHandle& oneHandle, const ZeroHandle& zeroHandle) : m_arrowBox(arrowBox), m_oneHandle(oneHandle),
				m_zeroHandle(zeroHandle) {}
		
		// Retourne si i < j
		bool operator() (unsigned int i, unsigned int j);
	};
	
	class StrandOrderCompareClockwise {
		const ArrowBox& m_arrowBox;
		const OneHandle& m_oneHandle;
		const ZeroHandle& m_zeroHandle;
		
	public:
		StrandOrderCompareClockwise(const ArrowBox& arrowBox, const OneHandle& oneHandle, const ZeroHandle& zeroHandle) : m_arrowBox(arrowBox), m_oneHandle(oneHandle),
				m_zeroHandle(zeroHandle) {}
		
		// Retourne si i < j
		bool operator() (unsigned int i, unsigned int j);
	};
	
	PermutationBox m_prePermutation;
	ArrowBox m_arrows0;
	PermutationBox m_permutation;
	ArrowBox m_arrows1;
	PermutationBox m_postPermutation;
	unsigned int m_numberOfTracks;
	OneHandleRenderer* m_renderer;
	
	void doLemma30(ArrowBox::ArrowInArrowBoxIndexedIterator begin, ArrowBox::ArrowInArrowBoxIndexedIterator& end);
	void sortArrowBoxesStrands(ZeroHandle& zeroHandle);
	
public:
	OneHandle(std::list<Arrow>&& arrows, unsigned int numberOfTracks, std::list<std::pair<unsigned int, unsigned int>>& arrowsData,
		std::list<ArrowBox::ArrowInArrowBox*>& arrowsPtr) : 
			m_prePermutation(numberOfTracks), m_arrows0(std::move(arrows), numberOfTracks), m_permutation(numberOfTracks), m_arrows1(numberOfTracks),
			m_postPermutation(numberOfTracks), m_numberOfTracks(numberOfTracks) {
		m_arrows0.fillArrowsRendererLists(arrowsData, arrowsPtr);
	}
	
	ArrowBox& getFirstArrowBox() {return m_arrows0;}
	PermutationBox& getPermutation() {return m_permutation;}
	ArrowBox& getSecondArrowBox() {return m_arrows1;}
	OneHandleRenderer& getRenderer() {return *m_renderer;}
	unsigned int getRightIndex(unsigned int val) const {return m_postPermutation.post(m_permutation.post(m_prePermutation.post(val)));}
	unsigned int getLeftIndex(unsigned int val) const {return m_prePermutation.pre(m_permutation.pre(m_postPermutation.pre(val)));}
	bool tracksEndsClockwise(std::pair<unsigned int, unsigned int> tracks, const ZeroHandle& zeroHandle, const ArrowBox& arrowBox, bool to) const;
	bool tracksEndsAntiClockwise(std::pair<unsigned int, unsigned int> tracks, const ZeroHandle& zeroHandle, const ArrowBox& arrowBox, bool to) const;
	
	void permute(BiPermutation& permutation, bool isFirstArrowBox);
	void transferArrowsToFirstArrowBox();
	void transferArrowToSecondArrowBox(ArrowBox::ArrowInArrowBoxIndexedIterator it, ArrowBox::ArrowInArrowBoxIndexedIterator& end);
	void transferArrowToSecondArrowBoxResolveCrossing(ArrowBox::ArrowInArrowBoxIndexedIterator it, ArrowBox::ArrowInArrowBoxIndexedIterator& end);
	void lemma30(ZeroHandle& zeroHandle);
	int getArrowDepth(const ArrowBox& arrowBox, const ZeroHandle& zeroHandle, std::pair<unsigned int, unsigned int> tracks, bool to) const;
	void removeDepthMArrows(const ZeroHandle& zeroHandle, int m, bool to);
	unsigned int getMinimalDepth(const ZeroHandle& zeroHandle) const;
	void addArrowFromZeroHandle(PassArrowsThroughtZeroHandleCommand* cmd, ArrowBox& src, std::list<ArrowBox::ArrowInArrowBox*>::iterator arrow, int index,
			unsigned int beginI, unsigned int beginJ, unsigned int endI, unsigned int endJ, bool fromLeft);
	void removeArrowToZeroHandle(ZeroHandle& zeroHandle, PassArrowsThroughtZeroHandleCommand* cmd, ArrowBox& src, std::list<ArrowBox::ArrowInArrowBox*>::iterator arrow, int index);
	void emptyArrowBoxThroughZeroHandle(ZeroHandle& zeroHandle, bool isFirstArrowBox);
	
	unsigned int numberOfTracks() const {return m_numberOfTracks;}
	
	friend OneHandleRenderer;
};

#endif // __ONE_HANDLE_HPP__
