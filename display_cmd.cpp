#include <queue>
#include <vector>
#include <map>
#include <cmath>
#include <utility>
#include <tuple>
#include <cassert>

#include <pthread.h>

#include "display_cmd.hpp"
#include "command.hpp"
#include "zero_handle.hpp"
#include "one_handle.hpp"
#include "arrow.hpp"

static std::queue<Command<RendererGL&>*> displayQueue;
static std::queue<Command<RendererGL&, float, float&>*> subCommandQueue;
static pthread_mutex_t       displayMutex;
static ZeroHandleRenderer* zeroHandleCurrent = nullptr;

void AnimateMoveArrowInArrowBoxCommand::addArrow(Arrow::Renderer& arrow, float p0x0, float p0y0, float p1x0, float p1y0, float t0x0, float t0y0, float t1x0, float t1y0,
								float p0x1, float p0y1, float p1x1, float p1y1, float t0x1, float t0y1, float t1x1, float t1y1) {
	const float x = m_arrowBox.getBasex();
	const float y = m_arrowBox.getBasey();
	
	if (m_isAbsolute)
		m_arrowsMoveData.emplace_back(std::piecewise_construct, std::forward_as_tuple(&arrow), std::forward_as_tuple(x+p0x0, y-p0y0, x+p1x0, y-p1y0, t0x0, t0y0, t1x0, t1y0,
								x+p0x1, y-p0y1, x+p1x1, y-p1y1, t0x1, t0y1, t1x1, t1y1));
	else
		m_arrowsMoveData.emplace_back(std::piecewise_construct, std::forward_as_tuple(&arrow), std::forward_as_tuple(p0x0, p0y0, p1x0, p1y0, t0x0, t0y0, t1x0, t1y0,
								p0x1, p0y1, p1x1, p1y1, t0x1, t0y1, t1x1, t1y1));
}

void AnimateMoveArrowInArrowBoxCommand::run(RendererGL& rendererGL, float t) {
	const float x = m_arrowBox.getBasex();
	const float y = m_arrowBox.getBasey();
	float s = 1.0-t;
	
	for (size_t i=0; i<m_arrowsMoveData.size(); i++) {
		MoveData& md = m_arrowsMoveData[i].second;
		if (m_isAbsolute)
			m_arrowsMoveData[i].first->changePoints(s*md.m_p0x0+t*md.m_p0x1, s*md.m_p0y0+t*md.m_p0y1, s*md.m_p1x0+t*md.m_p1x1, s*md.m_p1y0+t*md.m_p1y1, 
								s*md.m_t0x0+t*md.m_t0x1, s*md.m_t0y0+t*md.m_t0y1, s*md.m_t1x0+t*md.m_t1x1, s*md.m_t1y0+t*md.m_t1y1);
		else
			m_arrowsMoveData[i].first->changePoints(x+(s*md.m_p0x0+t*md.m_p0x1), y-(s*md.m_p0y0+t*md.m_p0y1), x+(s*md.m_p1x0+t*md.m_p1x1), y-(s*md.m_p1y0+t*md.m_p1y1),
								s*md.m_t0x0+t*md.m_t0x1, s*md.m_t0y0+t*md.m_t0y1, s*md.m_t1x0+t*md.m_t1x1, s*md.m_t1y0+t*md.m_t1y1);
	}
}

void AnimateMoveTrackLineArrowBox::addTrack(TrackLine& track, float p0x0, float p0y0, float p1x0, float p1y0, float p0x1, float p0y1, float p1x1, float p1y1) {
	const float x = m_arrowBox.getBasex();
	const float y = m_arrowBox.getBasey();
	
	m_trackMoveData.emplace_back(std::piecewise_construct, std::forward_as_tuple(&track),
							   std::forward_as_tuple(x+p0x0, y-p0y0, x+p1x0, y-p1y0, x+p0x1, y-p0y1, x+p1x1, y-p1y1));
}

void AnimateMoveTrackLineArrowBox::run(RendererGL& rendererGL, float t) {
	float s = 1.0-t;
	
	for (unsigned int i=0; i<m_trackMoveData.size(); i++) {
		MoveData& md = m_trackMoveData[i].second;
		m_trackMoveData[i].first->changePoints(s*md.m_p0x0+t*md.m_p0x1, s*md.m_p0y0+t*md.m_p0y1, s*md.m_p1x0+t*md.m_p1x1, s*md.m_p1y0+t*md.m_p1y1);
	}
}

void AnimateMoveArrowThroughtArrowBox::run(RendererGL& rendererGL, float t) {
	float y0 = m_arrowBox.getBasey() - m_arrowBox.getRelativeYForTrack(m_begin);
	float y1 = m_arrowBox.getBasey() - m_arrowBox.getRelativeYForTrack(m_end);
	float t0y = (m_end > m_begin) ? -1.0 : 1.0;
	float s = t*m_endT + (1.0-t)*m_startT;
	
	float px = m_arrowBox.getBasex()+s*m_arrowBox.lenght();
	m_arrow.changePoints(px, y0, px, y1, 0.0, t0y, 0.0, -t0y);
}

void AnimateMoveTrackPermutationBox::addTrack(TrackBezier& track, float p0x0, float p0y0, float p1x0, float p1y0, float p0x1, float p0y1, float p1x1, float p1y1,
		Direction d0, Direction d1) {
	m_trackMoveData.emplace_back(std::piecewise_construct, std::forward_as_tuple(&track),
							   std::forward_as_tuple(p0x0, p0y0, p1x0, p1y0, p0x1, p0y1, p1x1, p1y1, d0, d1));
}

void AnimateMoveTrackPermutationBox::run(RendererGL& rendererGL, float t) {
	float s = 1.0-t;
	float x = m_permutationBox.getBasex();
	float y = m_permutationBox.getBasey();
	
	for (unsigned int i=0; i<m_trackMoveData.size(); i++) {
		MoveData& md = m_trackMoveData[i].second;
		m_trackMoveData[i].first->changePoints(x+(s*md.m_p0x0+t*md.m_p0x1), y-(s*md.m_p0y0+t*md.m_p0y1), x+(s*md.m_p1x0+t*md.m_p1x1),
											   y-(s*md.m_p1y0+t*md.m_p1y1), md.m_d0, md.m_d1);
	}
}

void AnimateMoveArrowPermutationBox::run(RendererGL& rendererGL, float t) {
	t = (1.0-t)*m_startPos + t*m_endPos;
	
	const TrackBezier& trackI(m_permutationBox.getTrack(m_rightI));
	const TrackBezier& trackJ(m_permutationBox.getTrack(m_rightJ));
	float p0x, p0y, p1x, p1y;
	trackI.getPoint(p0x, p0y, t);
	trackJ.getPoint(p1x, p1y, t);
	float t0y = (p1y < p0y) ? -1.0 : 1.0;
	
	m_arrow.changePoints(p0x, p0y, p1x, p1y, 0.0, t0y, 0.0, -t0y);
}

void AnimateInterpolateShowableBezierCurve::addCurve(ShowableBezier& curve, Bezier curve0, Bezier curve1) {
	m_curves.emplace_back(std::piecewise_construct, std::forward_as_tuple(&curve), std::forward_as_tuple(curve0, curve1));
}
	
void AnimateInterpolateShowableBezierCurve::run(RendererGL& rendererGL, float t) {
	float s = 1.0-t;
	
	for (size_t i=0; i<m_curves.size(); i++) {
		const float* c0 = m_curves[i].second.m_curve0.getControlPoints();
		const float* c1 = m_curves[i].second.m_curve1.getControlPoints();
		float c[4*2];
		for (int j=0; j<4*2; j++)
			c[j] = s*c0[j] + t*c1[j];
		
		m_curves[i].first->changePoints(c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]);
	}
}

void GenCrossingCommand::run(RendererGL& rendererGL, float t) {
	float s = 1.0-t;
	float xEnd = s*m_xBegin+t*m_xEnd;
	
	
	m_arrowBox.getCrossing().m_crossA.changePoints(m_xBegin, m_yI, xEnd, m_yJ, EAST, WEST);
	m_arrowBox.getCrossing().m_crossB.changePoints(m_xBegin, m_yJ, xEnd, m_yI, EAST, WEST);
	m_arrowBox.getCrossing().m_lineA.changePoints (xEnd, m_yI, m_arrowBox.endX(), m_yI);
	m_arrowBox.getCrossing().m_lineB.changePoints (xEnd, m_yJ, m_arrowBox.endX(), m_yJ);
	std::pair<std::reference_wrapper<TrackLine>, std::reference_wrapper<TrackLine>> tracks(m_arrowBox.getCrossingTracks());
	std::pair<float, float> tracksHeight(m_arrowBox.getCrossingTracksHeight());
	tracks.first.get().changePoints(m_arrowBox.getBasex(), m_arrowBox.getBasey()-tracksHeight.first, m_xBegin, m_arrowBox.getBasey()-tracksHeight.first);
	tracks.second.get().changePoints(m_arrowBox.getBasex(), m_arrowBox.getBasey()-tracksHeight.second, m_xBegin, m_arrowBox.getBasey()-tracksHeight.second);
}

void MoveCrossingCommand::run(RendererGL& rendererGL, float t) {
	float s = 1.0-t;
	
	float xBegin = s*m_crossingStart + t*m_crossingNewStart;
	float xEnd   = s*m_crossingEnd+t*m_crossingNewEnd;
	
	m_arrowBox.getCrossing().m_crossA.changePoints(xBegin, m_yI, xEnd, m_yJ, EAST, WEST);
	m_arrowBox.getCrossing().m_crossB.changePoints(xBegin, m_yJ, xEnd, m_yI, EAST, WEST);
	m_arrowBox.getCrossing().m_lineA.changePoints (xEnd, m_yI, m_arrowBox.endX(), m_yI);
	m_arrowBox.getCrossing().m_lineB.changePoints (xEnd, m_yJ, m_arrowBox.endX(), m_yJ);
	std::pair<std::reference_wrapper<TrackLine>, std::reference_wrapper<TrackLine>> tracks(m_arrowBox.getCrossingTracks());
	std::pair<float, float> tracksHeight(m_arrowBox.getCrossingTracksHeight());
	tracks.first.get().changePoints(m_arrowBox.getBasex(), m_arrowBox.getBasey()-tracksHeight.first, xBegin, m_arrowBox.getBasey()-tracksHeight.first);
	tracks.second.get().changePoints(m_arrowBox.getBasex(), m_arrowBox.getBasey()-tracksHeight.second, xBegin, m_arrowBox.getBasey()-tracksHeight.second);
}

void MoveCrossingCommandSlideArrow::run(RendererGL& rendererGL, float t) {
	MoveCrossingCommand::run(rendererGL, t);
	
	float s = 1.0-t;
	
	float x, y;
	if (m_followTrackA)
		m_arrowBox.getCrossing().m_crossA.getPoint(x, y, s);
	else
		m_arrowBox.getCrossing().m_crossB.getPoint(x, y, s);
	
	if (m_moveBegin)
		m_arrow->changePoints(x, y, x, m_arrowFixPoint, 0.0, m_arrowT0y, 0.0, -m_arrowT0y);
	else
		m_arrow->changePoints(x, m_arrowFixPoint, x, y, 0.0, m_arrowT0y, 0.0, -m_arrowT0y);
}

void MoveCrossingCommandMoveArrow::run(RendererGL& rendererGL, float t) {
	MoveCrossingCommand::run(rendererGL, t);
	
	float s = 1.0-t;
	m_arrow->changePoints(s*m_arrowXa+t*m_arrowXb, m_arrowY0, s*m_arrowXa+t*m_arrowXb, m_arrowY1, 0.0, m_arrowT0y, 0.0, -m_arrowT0y);
}

void MoveCrossingCommandInvertArrow::run(RendererGL& rendererGL, float t) {
	MoveCrossingCommand::run(rendererGL, t);
	
	float s = 1.0-t;
	float x0, y0;
	float x1, y1;
	
	m_arrowBox.getCrossing().m_crossA.getPoint(x0, y0, s);
	m_arrowBox.getCrossing().m_crossB.getPoint(x1, y1, s);
	if (!m_isUpArrow) {
		std::swap(x0, x1); std::swap(y0, y1);
	}
	float t0y = (y0 > y1) ? 1.0 : -1.0;
	
	m_arrow->changePoints(x0, y0, x1, y1, 0.0, t0y, 0.0, -t0y);
}

void FadeCrossingCommand::run(RendererGL& rendererGL, float t) {
	float s = 1.0-t;
	
	float xBegin = m_arrowBox.getBasex()+(s*m_crossingStart + t*m_crossingFadeX);
	float xEnd   = m_arrowBox.getBasex()+(s*m_crossingEnd + t*m_crossingFadeX);
	float y0a = s*m_yI + t*m_yJ;
	float y1a = s*m_yJ + t*m_yI;
	
	m_arrowBox.getCrossing().m_crossA.changePoints(xBegin, m_yI, xEnd, y1a, EAST, WEST);
	m_arrowBox.getCrossing().m_crossB.changePoints(xBegin, m_yJ, xEnd, y0a, EAST, WEST);
	m_arrowBox.getCrossing().m_lineA.changePoints (xEnd, y0a, m_arrowBox.endX(), y0a);
	m_arrowBox.getCrossing().m_lineB.changePoints (xEnd, y1a, m_arrowBox.endX(), y1a);
	std::pair<std::reference_wrapper<TrackLine>, std::reference_wrapper<TrackLine>> tracks(m_arrowBox.getCrossingTracks());
	std::pair<float, float> tracksHeight(m_arrowBox.getCrossingTracksHeight());
	tracks.first.get().changePoints(m_arrowBox.getBasex(), m_arrowBox.getBasey()-tracksHeight.first, xBegin, m_arrowBox.getBasey()-tracksHeight.first);
	tracks.second.get().changePoints(m_arrowBox.getBasex(), m_arrowBox.getBasey()-tracksHeight.second, xBegin, m_arrowBox.getBasey()-tracksHeight.second);
}

void MoveArrowThroughtZeroHandleCommand::run(RendererGL& rendererGL, float t) {
	m_zeroHandle.setArrowPosInZeroHandle(m_arrow, m_startI, m_endI, m_startJ, m_endJ, t);
}

void ArrowSetLayerCommand::run(RendererGL& rendererGL, float t) {
	m_arrow.setLayer(m_newLayer);
}


AsynchronousSubTaskCommand::~AsynchronousSubTaskCommand() {
	while (!m_cmd.empty()) {
		delete m_cmd.top().m_cmd;
		m_cmd.pop();
	}
	
	auto it=m_executingCmd.begin();
	while (it!=m_executingCmd.end()) {
		delete it->m_cmd;
		it = m_executingCmd.erase(it);
	}
}

void AsynchronousSubTaskCommand::run(RendererGL& rendererGL, float dt, float& pastTime) {
	m_t+=dt;
	pastTime = m_t-m_duration;
	
	if (pastTime>=0.0) {
		while (!m_cmd.empty()) {
			delete m_cmd.top().m_cmd;
			m_cmd.pop();
		}
		
		auto it=m_executingCmd.begin();
		while (it!=m_executingCmd.end()) {
			delete it->m_cmd;
			it = m_executingCmd.erase(it);
		}
	} else {
		while (!m_cmd.empty() && m_cmd.top().m_taskBegin <= m_t) {
			m_executingCmd.emplace_back(m_cmd.top().m_cmd, m_cmd.top().m_taskBegin, m_cmd.top().m_taskEnd);
			m_cmd.pop();
		}
		
		auto it=m_executingCmd.begin();
		while (true) {
			while (it!=m_executingCmd.end() && it->m_taskEnd <= m_t) {
				it->m_cmd->run(rendererGL, 1.0);
				delete it->m_cmd;
				it = m_executingCmd.erase(it);
			}
			if (it == m_executingCmd.end()) break;
			
			if (it->m_taskEnd-it->m_taskBegin >= 1e-5) {
				float p = (m_t - it->m_taskBegin)/(it->m_taskEnd - it->m_taskBegin);
				it->m_cmd->run(rendererGL, p);
			} else {
				it->m_cmd->run(rendererGL, 1.0);
			}
			++it;
		}
	}
}

void AsynchronousSubTaskCommand::addCommand(Command<RendererGL&, float>* cmd, float cmdBegin, float cmdEnd) {
	assert(cmdBegin <= cmdEnd);
	m_cmd.emplace(cmd, cmdBegin, cmdEnd);
	m_duration = std::max(m_duration, cmdEnd);
}

class SynchronousSubTaskCommand : public Command<RendererGL&, float, float&> {
	struct SynchronousSubTaskData {
		Command<RendererGL&, float>* m_cmd = nullptr;
		
		SynchronousSubTaskData(Command<RendererGL&, float>* cmd) : m_cmd(cmd) {}
		SynchronousSubTaskData(SynchronousSubTaskData&& other) : m_cmd(other.m_cmd) {other.m_cmd=nullptr;}
		~SynchronousSubTaskData() {delete m_cmd;}
		SynchronousSubTaskData& operator=(SynchronousSubTaskData&& other) {
			m_cmd = other.m_cmd;
			other.m_cmd = nullptr;
			return *this;
		}
	};
	
	std::vector<SynchronousSubTaskData> m_tasks;
	const float m_duration;
	float m_t = 0.0;
	
	SynchronousSubTaskCommand(float duration) : m_duration(duration) {}
	
public:
	static SynchronousSubTaskCommand* create(float duration) {return new SynchronousSubTaskCommand(duration);}
	
	virtual void run(RendererGL& rendererGL, float dt, float& pastTime);
	
	virtual std::string name() const {return "SynchronousSubTaskCommand";}
	
	void addCommand(Command<RendererGL&, float>* cmd) {
		m_tasks.emplace_back(cmd);
	}
};


class RefreshArrowBoxCommand : public Command<RendererGL&, float, float&> {
	ArrowBox::Renderer& m_arrowBox;
	bool m_refreshLenght, m_refreshTracks;
	
	RefreshArrowBoxCommand(ArrowBox::Renderer& arrowBox, bool refreshLenght, bool refreshTracks) : m_arrowBox(arrowBox), m_refreshLenght(refreshLenght),
			m_refreshTracks(refreshTracks) {}
	
public:
	static Command<RendererGL&, float, float&>* create(ArrowBox::Renderer& arrowBox, bool refreshLenght, bool refreshTracks=false) {
		return new RefreshArrowBoxCommand(arrowBox, refreshLenght, refreshTracks);
	}
	
	virtual void run(RendererGL& rendererGL, float dt, float& pastTime);
	
	virtual std::string name() const {return "RefreshArrowBoxCommand";}
};

class RefreshPermutationBoxCommand : public Command<RendererGL&, float, float&> {
	PermutationBox::Renderer& m_permutationBox;
	
	RefreshPermutationBoxCommand(PermutationBox::Renderer& permutationBox) : m_permutationBox(permutationBox) {}
	
public:
	static Command<RendererGL&, float, float&>* create(PermutationBox::Renderer& permutationBox) {
		return new RefreshPermutationBoxCommand(permutationBox);
	}
	
	virtual void run(RendererGL& rendererGL, float dt, float& pastTime);
	
	virtual std::string name() const {return "RefreshPermutationBoxCommand";}
};

class RemoveArrowCommand : public Command<RendererGL&, float, float&> {
	ArrowBox::Renderer& m_arrowBox;
	ArrowBox::ArrowInArrowBox& m_arrow;
	
	RemoveArrowCommand(ArrowBox::Renderer& arrowBox, ArrowBox::ArrowInArrowBox& arrow) : m_arrowBox(arrowBox), m_arrow(arrow) {}
	
public:
	static Command<RendererGL&, float, float&>* create(ArrowBox::Renderer& arrowBox, ArrowBox::ArrowInArrowBox& arrow) {
		return new RemoveArrowCommand(arrowBox, arrow);
	}
	
	virtual void run(RendererGL& rendererGL, float dt, float& pastTime);
	
	virtual std::string name() const {return "RemoveArrowCommand";}
};

class RemoveCrossingCommand : public Command<RendererGL&, float, float&> {
	ArrowBox::Renderer& m_arrowBox;
	
	RemoveCrossingCommand(ArrowBox::Renderer& arrowBox) : m_arrowBox(arrowBox) {}
	
public:
	static Command<RendererGL&, float, float&>* create(ArrowBox::Renderer& arrowBox) {
		return new RemoveCrossingCommand(arrowBox);
		
	}
	
	virtual void run(RendererGL& rendererGL, float dt, float& pastTime);
	
	virtual std::string name() const {return "RemoveCrossingCommand";}
};

class RemoveTrackCommand : public Command<RendererGL&, float, float&> {
	std::vector<Track*> m_curves;
	
public:
	static RemoveTrackCommand* create() {return new RemoveTrackCommand();}
	
	void addCurve(Track& curve) {
		m_curves.push_back(&curve);
	}
	
	virtual void run(RendererGL& rendererGL, float dt, float& pastTime);
	
	virtual std::string name() const {return "RemoveTrackCommand";}
};

class DeleteZeroHandleCommand : public Command<RendererGL&> {
	ZeroHandle& m_zeroHandle;
	
	DeleteZeroHandleCommand(ZeroHandle& zeroHandle) : m_zeroHandle(zeroHandle) {}
	
public:
	static Command<RendererGL&>* create(ZeroHandle& zeroHandle) {return new DeleteZeroHandleCommand(zeroHandle);}
	
	virtual void run(RendererGL& rendererGL);
	
	virtual std::string name() const {return "DeleteZeroHandleCommand";}
};

class DrawZeroHandleCommand : public Command<RendererGL&> {
private:
	const Pairing m_pairing;
	ZeroHandle& m_zeroHandle;
	std::list<std::pair<unsigned int, unsigned int>> m_voidArrowsData;
	std::list<ArrowBox::ArrowInArrowBox*> m_voidArrows;
	std::list<std::pair<unsigned int, unsigned int>> m_fullArrowsData;
	std::list<ArrowBox::ArrowInArrowBox*> m_fullArrows;
	
	DrawZeroHandleCommand(Pairing pairing, ZeroHandle& zeroHandle,
		std::list<std::pair<unsigned int, unsigned int>>&& voidArrowsData, std::list<ArrowBox::ArrowInArrowBox*>&& voidArrows,
		std::list<std::pair<unsigned int, unsigned int>>&& fullArrowsData, std::list<ArrowBox::ArrowInArrowBox*>&& fullArrows) : 
		m_pairing(pairing), m_zeroHandle(zeroHandle), m_voidArrowsData(std::move(voidArrowsData)), m_voidArrows(std::move(voidArrows)),
				m_fullArrowsData(std::move(fullArrowsData)), m_fullArrows(std::move(fullArrows)) {}
	
public:
	static Command<RendererGL&>* create(Pairing pairing, ZeroHandle& zeroHandle,
			std::list<std::pair<unsigned int, unsigned int>>&& voidArrowsData, std::list<ArrowBox::ArrowInArrowBox*>&& voidArrows,
			std::list<std::pair<unsigned int, unsigned int>>&& fullArrowsData, std::list<ArrowBox::ArrowInArrowBox*>&& fullArrows)
	{return new DrawZeroHandleCommand(pairing, zeroHandle, std::move(voidArrowsData), std::move(voidArrows),
				std::move(fullArrowsData), std::move(fullArrows));}
	
	virtual void run(RendererGL& rendererGL);
	
	virtual std::string name() const {return "DrawZeroHandleCommand";}
};

class PermuteArrowBoxCommand : public Command<RendererGL&> {
private:
	BiPermutation m_permutation;
	OneHandle& m_oneHandle;
	bool m_isFirstArrowBox;
	
	PermuteArrowBoxCommand(const BiPermutation& permutation, OneHandle& oneHandle, bool isFirstArrowBox) : m_permutation(permutation), m_oneHandle(oneHandle), 	
		m_isFirstArrowBox(isFirstArrowBox) {}
	
public:
	static Command<RendererGL&>* create(const BiPermutation& permutation, OneHandle& oneHandle, bool isFirstArrowBox) {
		return new PermuteArrowBoxCommand(permutation, oneHandle, isFirstArrowBox);
	}
	
	virtual void run(RendererGL& rendererGL);
	
	virtual std::string name() const {return "PermuteArrowBoxCommand";}
};

class MoveArrowInArrowBoxCommand : public Command<RendererGL&> {
private:
	ArrowBox& m_arrowBox;
	ArrowInArrowBoxIndexed m_movingArrow;
	ArrowInArrowBoxIndexed m_targetArrow;
	
	MoveArrowInArrowBoxCommand(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow) : 
			m_arrowBox(arrowBox), m_movingArrow(movingArrow), m_targetArrow(targetArrow) {assert(movingArrow.second!=targetArrow.second);}
	
public:
	static Command<RendererGL&>* create(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow)
		{return new MoveArrowInArrowBoxCommand(arrowBox, movingArrow, targetArrow);}
	
	virtual void run(RendererGL& rendererGL);
	
	virtual std::string name() const {return "MoveArrowInArrowBoxCommand";}
};

class GenArrowAfterMoveCrossingCommand : public Command<RendererGL&> {
private:
	ArrowBox& m_arrowBox;
	ArrowInArrowBoxIndexed m_movingArrow;
	ArrowInArrowBoxIndexed m_targetArrow;
	ArrowBox::ArrowInArrowBox& m_newArrow;
	unsigned int m_from, m_to;
	bool m_genAfter;
	
	GenArrowAfterMoveCrossingCommand(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow,
		ArrowBox::ArrowInArrowBox& newArrow, unsigned int from, unsigned int to, bool genAfter) :
			m_arrowBox(arrowBox), m_movingArrow(movingArrow), m_targetArrow(targetArrow), m_newArrow(newArrow), m_from(from), m_to(to), m_genAfter(genAfter)
	{assert(movingArrow.second != targetArrow.second);}

public:
	static Command<RendererGL&>* create(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow,
		ArrowBox::ArrowInArrowBox& newArrow, unsigned int from, unsigned int to, bool genAfter) {
		return new GenArrowAfterMoveCrossingCommand(arrowBox, movingArrow, targetArrow, newArrow, from, to, genAfter);
	}
	
	virtual void run(RendererGL& rendererGL);
	
	virtual std::string name() const {return "GenArrowAfterMoveCrossingCommand";}
};

class MoveMergeArrowsCommand : public Command<RendererGL&> {
private:
	ArrowBox& m_arrowBox;
	ArrowInArrowBoxIndexed m_movingArrow;
	ArrowInArrowBoxIndexed m_targetArrow;
	
	MoveMergeArrowsCommand(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow) :
		m_arrowBox(arrowBox), m_movingArrow(movingArrow), m_targetArrow(targetArrow) {assert(targetArrow.second!=movingArrow.second);}
	
public:
	static Command<RendererGL&>* create(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow) {
		return new MoveMergeArrowsCommand(arrowBox, movingArrow, targetArrow);
	}
	
	virtual void run(RendererGL& rendererGL);
	
	virtual std::string name() const {return "MoveMergeArrowsCommand";}
};

class RemoveArrowFromArrowBoxCommand : public Command<RendererGL&> {
	ArrowBox& m_arrowBox;
	ArrowInArrowBoxIndexed m_arrow;
	
	RemoveArrowFromArrowBoxCommand(ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow) : m_arrowBox(arrowBox), m_arrow(arrow) {}
	
public:
	static Command<RendererGL&>* create(ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow) {return new RemoveArrowFromArrowBoxCommand(arrowBox, arrow);}
	
	virtual void run(RendererGL& rendererGL);
	
	virtual std::string name() const {return "RemoveArrowFromArrowBoxCommand";}
};

class MoveArrowGenCrossingCommand : public Command<RendererGL&> {
private:
	ArrowBox& m_arrowBox;
	ArrowInArrowBoxIndexed m_movingArrow;
	ArrowInArrowBoxIndexed m_targetArrow;
	unsigned int m_crossingI, m_crossingJ;
	
	MoveArrowGenCrossingCommand(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow,
		unsigned int crossingI, unsigned int crossingJ) :
			m_arrowBox(arrowBox), m_movingArrow(movingArrow), m_targetArrow(targetArrow), m_crossingI(crossingI), m_crossingJ(crossingJ)
		{assert(targetArrow.second!=movingArrow.second);}
	
public:
	static Command<RendererGL&>* create(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow,
			unsigned int crossingI, unsigned int crossingJ) {
		return new MoveArrowGenCrossingCommand(arrowBox, movingArrow, targetArrow, crossingI, crossingJ);
	}
	
	virtual void run(RendererGL& rendererGL);
	
	virtual std::string name() const {return "MoveArrowGenCrossingCommand";}
};

class MoveArrowToFirstArrowBoxCommand : public Command<RendererGL&> {
	OneHandle& m_oneHandle;
	
	MoveArrowToFirstArrowBoxCommand(OneHandle& oneHandle) : m_oneHandle(oneHandle) {}
	
public:
	static Command<RendererGL&>* create(OneHandle& oneHandle) {
		return new MoveArrowToFirstArrowBoxCommand(oneHandle);
	}
	
	virtual void run(RendererGL& rendererGL);
	
	virtual std::string name() const {return "MoveArrowToFirstArrowBoxCommand";}
};

class MoveArrowToOtherArrowBoxCommand : public Command<RendererGL&> {
	OneHandle& m_oneHandle;
	ArrowBox&  m_arrowBox;
	ArrowInArrowBoxIndexed m_arrow;
	unsigned int m_targetI, m_targetJ;
	
	MoveArrowToOtherArrowBoxCommand(OneHandle& oneHandle, ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow, unsigned int targetI, unsigned int targetJ) :
		m_oneHandle(oneHandle), m_arrowBox(arrowBox), m_arrow(arrow), m_targetI(targetI), m_targetJ(targetJ) {}
	
public:
	static Command<RendererGL&>* create(OneHandle& oneHandle, ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow, unsigned int targetI, unsigned int targetJ) {
		return new MoveArrowToOtherArrowBoxCommand(oneHandle, arrowBox, arrow, targetI, targetJ);
	}
	
	virtual void run(RendererGL& rendererGL);
	
	virtual std::string name() const {return "MoveArrowToOtherArrowBoxCommand";}
};

class MoveArrowToOtherArrowBoxResolveCrossingCommand : public Command<RendererGL&> {
	OneHandle& m_oneHandle;
	ArrowBox&  m_arrowBox;
	ArrowInArrowBoxIndexed m_arrow;
	ArrowBox::ArrowInArrowBox& m_newArrow;
	unsigned int m_targetI, m_targetJ;
	
	MoveArrowToOtherArrowBoxResolveCrossingCommand(OneHandle& oneHandle, ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow, ArrowBox::ArrowInArrowBox& newArrow,
			unsigned int targetI, unsigned int targetJ) :
		m_oneHandle(oneHandle), m_arrowBox(arrowBox), m_arrow(arrow), m_newArrow(newArrow), m_targetI(targetI), m_targetJ(targetJ) {}
	
public:
	static Command<RendererGL&>* create(OneHandle& oneHandle, ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow, ArrowBox::ArrowInArrowBox& newArrow,
			unsigned int targetI, unsigned int targetJ) {
		return new MoveArrowToOtherArrowBoxResolveCrossingCommand(oneHandle, arrowBox, arrow, newArrow, targetI, targetJ);
	}
	
	virtual void run(RendererGL& rendererGL);
	
	virtual std::string name() const {return "MoveArrowToOtherArrowBoxResolveCrossingCommand";}
};







void MoveArrowsAcrossZeroHandle::moveArrowThroughtOneHandle(AsynchronousSubTaskCommand* cmd, float start, float end, unsigned int rightI, unsigned int rightJ,
		Arrow::Renderer& arrow, OneHandleRenderer& oneHandle, float& time) {
	assert(start>=0.0 && start<=1.0); assert(end>=0.0 && end<=1.0);
	float prePerT = oneHandle.getPrePermutation().lenght()/oneHandle.lenght();
	float arw0T   = prePerT + oneHandle.getFirstArrowBox().lenght()/oneHandle.lenght();
	float perT    = arw0T   + oneHandle.getPermutation().lenght()/oneHandle.lenght();
	float arw1T   = perT    + oneHandle.getSecondArrowBox().lenght()/oneHandle.lenght();
	
	while (std::abs(start-end)>=1e-5) {
		if (start<prePerT) {
			float tStart = start/prePerT;
			float tEnd   = (end>=prePerT) ? 1.0 : end/prePerT;
			unsigned int i = oneHandle.getPermutation().pre(oneHandle.getPostPermutation().pre(rightI));
			unsigned int j = oneHandle.getPermutation().pre(oneHandle.getPostPermutation().pre(rightJ));
			AnimateMoveArrowPermutationBox* anim = AnimateMoveArrowPermutationBox::create(oneHandle.getPrePermutation(), arrow, i, j, tStart, tEnd);
			float dTime = std::abs(tEnd-tStart)*oneHandle.getPrePermutation().lenght()/ArrowBox::Renderer::arrowSeparation();
			cmd->addCommand(anim, time, time+dTime); time+=dTime;
			start = (end>prePerT) ? prePerT : end;
		} else if (start<arw0T) {
			float tStart = (start-prePerT)/(arw0T-prePerT);
			float tEnd   = (end>=arw0T) ? 1.0 : ((end<=prePerT) ? 0.0 : (end-prePerT)/(arw0T-prePerT));
			unsigned int i = oneHandle.getPermutation().pre(oneHandle.getPostPermutation().pre(rightI));
			unsigned int j = oneHandle.getPermutation().pre(oneHandle.getPostPermutation().pre(rightJ));
			AnimateMoveArrowThroughtArrowBox* anim = AnimateMoveArrowThroughtArrowBox::create(oneHandle.getFirstArrowBox(), arrow, i, j, tStart, tEnd);
			float dTime = std::abs(tEnd-tStart)*oneHandle.getFirstArrowBox().lenght()/ArrowBox::Renderer::arrowSeparation();
			cmd->addCommand(anim, time, time+dTime); time+=dTime;
			start = (end>arw0T) ? arw0T : ((end<=prePerT) ? prePerT - 1e-5 : end);
		} else if (start<perT) {
			float tStart = (start-arw0T)/(perT-arw0T);
			float tEnd   = (end>=perT) ? 1.0 : ((end<=arw0T) ? 0.0 : (end-arw0T)/(perT-arw0T));
			unsigned int i = oneHandle.getPostPermutation().pre(rightI);
			unsigned int j = oneHandle.getPostPermutation().pre(rightJ);
			AnimateMoveArrowPermutationBox* anim = AnimateMoveArrowPermutationBox::create(oneHandle.getPermutation(), arrow, i, j, tStart, tEnd);
			float dTime = std::abs(tEnd-tStart)*oneHandle.getPermutation().lenght()/ArrowBox::Renderer::arrowSeparation();
			cmd->addCommand(anim, time, time+dTime); time+=dTime;
			start = (end>perT) ? perT : ((end<=arw0T) ? arw0T - 1e-5 : end);
		} else if (start<arw1T) {
			float tStart = (start-perT)/(arw1T-perT);
			float tEnd   = (end>=arw1T) ? 1.0 : ((end<=perT) ? 0.0 : (end-perT)/(arw1T-perT));
			unsigned int i = oneHandle.getPostPermutation().pre(rightI);
			unsigned int j = oneHandle.getPostPermutation().pre(rightJ);
			AnimateMoveArrowThroughtArrowBox* anim = AnimateMoveArrowThroughtArrowBox::create(oneHandle.getSecondArrowBox(), arrow, i, j, tStart, tEnd);
			float dTime = std::abs(tEnd-tStart)*oneHandle.getFirstArrowBox().lenght()/ArrowBox::Renderer::arrowSeparation();
			cmd->addCommand(anim, time, time+dTime); time+=dTime;
			start = (end>arw1T) ? arw1T : ((end<=perT) ? perT - 1e-5 : end);
		} else {
			float tStart = (start-arw1T)/(1.0-arw1T);
			float tEnd   = (end<=arw1T) ? 0.0 : (end-arw1T)/(1.0-arw1T);
			AnimateMoveArrowPermutationBox* anim = AnimateMoveArrowPermutationBox::create(oneHandle.getPostPermutation(), arrow, rightI, rightJ, tStart, tEnd);
			float dTime = std::abs(tEnd-tStart)*oneHandle.getPostPermutation().lenght()/ArrowBox::Renderer::arrowSeparation();
			cmd->addCommand(anim, time, time+dTime); time+=dTime;
			start = (end<=arw1T) ? arw1T - 1e-5 : end;
		}
	}
}

void MoveArrowsAcrossZeroHandle::moveArrowThroughtZeroHandle(AsynchronousSubTaskCommand* cmd, float start, float end, unsigned int startI, unsigned int endI,
		unsigned int startJ, unsigned int endJ, Arrow::Renderer& arrow, float& time) {
	ZeroHandleRenderer& zeroHandle = m_zeroHandle.getRenderer();
	float dTime = 0.0;
	int layer = 0;
	bool isVoidHandle = true;
	if ((endI<zeroHandle.getNumberOfTrackFull() ||
		(endI>=zeroHandle.getNumberOfTrackFull()+zeroHandle.getNumberOfTrackVoid() &&
		 endI<2*zeroHandle.getNumberOfTrackFull()+zeroHandle.getNumberOfTrackVoid()))) {
		layer = 2;
		isVoidHandle = false;
	}
	
	if      (startI < zeroHandle.getNumberOfTrackFull())
		dTime += 0.5*(zeroHandle.lenght()-zeroHandle.getFullHandle().lenght())/ArrowBox::Renderer::arrowSeparation();
	else if (startI < zeroHandle.getNumberOfTrackFull()+zeroHandle.getNumberOfTrackVoid())
		dTime += (0.5*(zeroHandle.lenght()-zeroHandle.getVoidHandle().lenght()) + 4.0 + zeroHandle.lenght())/ArrowBox::Renderer::arrowSeparation();
	else if (startI < 2*zeroHandle.getNumberOfTrackFull()+zeroHandle.getNumberOfTrackVoid())
		dTime += (0.5*(zeroHandle.lenght()-zeroHandle.getFullHandle().lenght()) + 6.0 + zeroHandle.lenght())/ArrowBox::Renderer::arrowSeparation();
	else
		dTime += (0.5*(zeroHandle.lenght()-zeroHandle.getVoidHandle().lenght())+1.0)/ArrowBox::Renderer::arrowSeparation();
	
	if      (endI < zeroHandle.getNumberOfTrackFull())
		dTime += 0.5*(zeroHandle.lenght()-zeroHandle.getFullHandle().lenght())/ArrowBox::Renderer::arrowSeparation();
	else if (endI < zeroHandle.getNumberOfTrackFull()+zeroHandle.getNumberOfTrackVoid())
		dTime += (0.5*(zeroHandle.lenght()-zeroHandle.getVoidHandle().lenght()) + 4.0 + zeroHandle.lenght())/ArrowBox::Renderer::arrowSeparation();
	else if (endI < 2*zeroHandle.getNumberOfTrackFull()+zeroHandle.getNumberOfTrackVoid())
		dTime += (0.5*(zeroHandle.lenght()-zeroHandle.getFullHandle().lenght()) + 6.0 + zeroHandle.lenght())/ArrowBox::Renderer::arrowSeparation();
	else
		dTime += (0.5*(zeroHandle.lenght()-zeroHandle.getVoidHandle().lenght())+1.0)/ArrowBox::Renderer::arrowSeparation();
	
	dTime *= std::abs(end-start);
	if (!isVoidHandle) {
		dTime += 3.0/ArrowBox::Renderer::arrowSeparation();
	}
	Command<RendererGL&, float>* anim = MoveArrowThroughtZeroHandleCommand::create(arrow, zeroHandle, startI, endI, startJ, endJ, start, end);
	Command<RendererGL&, float>* layerChange = ArrowSetLayerCommand::create(arrow, layer);
	cmd->addCommand(layerChange, time+dTime/2, time+dTime/2);
	cmd->addCommand(anim, time, time+dTime); time+=dTime;
}

float MoveArrowsAcrossZeroHandle::getPosInArrowBox(ArrowInArrowBoxIndexed arrow, ArrowBox::Renderer& arrowBox) {
	OneHandleRenderer& oneHandle = arrowBox.getOneHandle();
	float ret = oneHandle.getPrePermutation().lenght();
	if (&arrowBox == &oneHandle.getSecondArrowBox()) {
		ret += oneHandle.getFirstArrowBox().lenght();
		ret += oneHandle.getPermutation().lenght();
	}
	ret += arrow.second*ArrowBox::Renderer::arrowSeparation() + ArrowBox::Renderer::arrowSeparation()/2;
	ret /= oneHandle.lenght();
	
	return ret;
}

void MoveArrowsAcrossZeroHandle::getStartEndIJ(unsigned int& startI, unsigned int& endI, unsigned int& startJ, unsigned int& endJ,
		IterationData& current, IterationData& prev) {
	ZeroHandleRenderer& zeroHandle = m_zeroHandle.getRenderer();
	startI = (prev.m_moveRight) ? prev.m_rightI : prev.getLeftI();
	endI   = (prev.m_moveRight) ? prev.m_rightJ : prev.getLeftJ();
	startJ = (current.m_moveRight) ? current.getLeftI() : current.m_rightI;
	endJ   = (current.m_moveRight) ? current.getLeftJ() : current.m_rightJ;
	
	if (prev.m_moveRight && prev.m_isVoidArrow) {
		startI += zeroHandle.getNumberOfTrackVoid();
		endI   += zeroHandle.getNumberOfTrackVoid();
	} else if (prev.m_moveRight && !prev.m_isVoidArrow) {
		startI += zeroHandle.getNumberOfTrackVoid() + zeroHandle.getNumberOfTrackFull();
		endI   += zeroHandle.getNumberOfTrackVoid() + zeroHandle.getNumberOfTrackFull();
	} else if (!prev.m_moveRight && prev.m_isVoidArrow) {
		startI += zeroHandle.getNumberOfTrackVoid() + 2*zeroHandle.getNumberOfTrackFull();
		endI   += zeroHandle.getNumberOfTrackVoid() + 2*zeroHandle.getNumberOfTrackFull();
	}
	
	if (!current.m_moveRight && current.m_isVoidArrow) {
		startJ += zeroHandle.getNumberOfTrackVoid();
		endJ   += zeroHandle.getNumberOfTrackVoid();
	} else if (!current.m_moveRight && !current.m_isVoidArrow) {
		startJ += zeroHandle.getNumberOfTrackVoid() + zeroHandle.getNumberOfTrackFull();
		endJ   += zeroHandle.getNumberOfTrackVoid() + zeroHandle.getNumberOfTrackFull();
	} else if (current.m_moveRight && current.m_isVoidArrow) {
		startJ += zeroHandle.getNumberOfTrackVoid() + 2*zeroHandle.getNumberOfTrackFull();
		endJ   += zeroHandle.getNumberOfTrackVoid() + 2*zeroHandle.getNumberOfTrackFull();
	}
}

void MoveArrowsAcrossZeroHandle::moveArrow(AsynchronousSubTaskCommand* cmd, MoveData& moveData, float time, unsigned int index, size_t iterationIndex, float t) {
	assert(!moveData.m_iterationData.empty());
	
	bool isLeadingArrow = true;
	if (moveData.m_iterationData.size()>1) {
		IterationData& iterationData = moveData.m_iterationData[0];
		float startPos = getPosInArrowBox(moveData.m_arrow, moveData.m_arrowBox->getRenderer());
		float endPos   = moveData.m_iterationData[0].m_moveRight ? 1.0 : 0.0;
		Arrow::Renderer& arrow = *moveData.m_arrow.first.get().getArrowRendererInList();
		
		if (!m_arrows.empty() && m_arrows.top().m_iterationStartIndex == iterationIndex) {
			isLeadingArrow = false;
			MoveData moveData(std::move(m_arrows.top())); m_arrows.pop();
			float pos = getPosInArrowBox(moveData.m_arrow, moveData.m_arrowBox->getRenderer());
			float moveTime = time+std::abs(pos-startPos)*iterationData.m_oneHandle.getRenderer().lenght()/ArrowBox::Renderer::arrowSeparation()-1.0;
			moveArrow(cmd, moveData, moveTime, index+1, iterationIndex, 0.0);
		}
		
		moveArrowThroughtOneHandle(cmd, startPos, endPos, moveData.m_iterationData[0].m_rightI, moveData.m_iterationData[0].m_rightJ,
			arrow, iterationData.m_oneHandle.getRenderer(), time);
		
		for (size_t i=1; i<moveData.m_iterationData.size()-1; i++) {
			IterationData& prev = moveData.m_iterationData[i-1];
			IterationData& current = moveData.m_iterationData[i];
			
			unsigned int startI, endI, startJ, endJ;
			getStartEndIJ(startI, endI, startJ, endJ, current, prev);
			moveArrowThroughtZeroHandle(cmd, 0.0, 1.0, startI, startJ, endI, endJ, arrow, time);
			
			++iterationIndex;
			float startPos = current.m_moveRight ? 0.0 : 1.0;
			float endPos   = current.m_moveRight ? 1.0 : 0.0;
			if (isLeadingArrow && !m_arrows.empty() && m_arrows.top().m_iterationStartIndex == iterationIndex) {
				isLeadingArrow = false;
				MoveData moveData(std::move(m_arrows.top())); m_arrows.pop();
				float pos = getPosInArrowBox(moveData.m_arrow, moveData.m_arrowBox->getRenderer());
				float moveTime = time+std::abs(pos-startPos)*iterationData.m_oneHandle.getRenderer().lenght()/ArrowBox::Renderer::arrowSeparation()-1.0;
				moveArrow(cmd, moveData, moveTime, index+1, iterationIndex, 0.0);
			}
			moveArrowThroughtOneHandle(cmd, startPos, endPos, current.m_rightI, current.m_rightJ, arrow, current.m_oneHandle.getRenderer(), time);
		}
		
		IterationData& prev = moveData.m_iterationData[moveData.m_iterationData.size()-2];
		IterationData& current = moveData.m_iterationData[moveData.m_iterationData.size()-1];
		unsigned int startI, endI, startJ, endJ;
		getStartEndIJ(startI, endI, startJ, endJ, current, prev);
		float pos = m_zeroHandle.getRenderer().getTAfterZeroHandle(startI, startJ, endI, endJ);
		float pathLenght = m_zeroHandle.getRenderer().getPathLenght(startI, startJ, endI, endJ);
		pos += (0.5+index)*ArrowBox::Renderer::arrowSeparation()/pathLenght;
		moveArrowThroughtZeroHandle(cmd, 0.0, std::min(1.0f, pos), startI, startJ, endI, endJ, arrow, time);
		
		float pos1 = pos-1.0; pos1 *= pathLenght/current.m_oneHandle.getRenderer().lenght();
		if (pos1>0.0) {
			assert(pos1<1.0);
			++iterationIndex;
			startPos = current.m_moveRight ? 0.0 : 1.0;
			endPos   = current.m_moveRight ? 0.0+pos1 : 1.0-pos1;
			if (isLeadingArrow && !m_arrows.empty() && m_arrows.top().m_iterationStartIndex == iterationIndex) {
				isLeadingArrow = false;
				MoveData moveData(std::move(m_arrows.top())); m_arrows.pop();
				float pos = getPosInArrowBox(moveData.m_arrow, moveData.m_arrowBox->getRenderer());
				float moveTime = time+std::abs(pos-startPos)*iterationData.m_oneHandle.getRenderer().lenght()/ArrowBox::Renderer::arrowSeparation()-1.0;
				moveArrow(cmd, moveData, moveTime, index+1, iterationIndex, pos1*pathLenght);
			}
			moveArrowThroughtOneHandle(cmd, startPos, endPos, current.m_rightI, current.m_rightJ, arrow, current.m_oneHandle.getRenderer(), time);
		}
		
		if (!moveData.m_shouldRemoveArrow) {
			if (pos1>0.0) {
				moveArrowThroughtOneHandle(cmd, endPos, startPos, current.m_rightI, current.m_rightJ, arrow, current.m_oneHandle.getRenderer(), time);
			}
			moveArrowThroughtZeroHandle(cmd, std::min(1.0f, pos), 0.0, startI, startJ, endI, endJ, arrow, time);
			
			if (moveData.m_iterationData.size()>2) {
				size_t i = moveData.m_iterationData.size()-1;
				do {
					--i;
					IterationData& prev = moveData.m_iterationData[i];
					IterationData& current = moveData.m_iterationData[i+1];
					
					float startPos = current.m_moveRight ? 1.0 : 0.0;
					float endPos   = current.m_moveRight ? 0.0 : 1.0;
					moveArrowThroughtOneHandle(cmd, startPos, endPos, current.m_rightI, current.m_rightJ, arrow, current.m_oneHandle.getRenderer(), time);
					
					unsigned int startI, endI, startJ, endJ;
					getStartEndIJ(startI, endI, startJ, endJ, current, prev);
					moveArrowThroughtZeroHandle(cmd, 1.0, 0.0, startI, startJ, endI, endJ, arrow, time);
				} while (i>0);
			}
			
			float startPos = getPosInArrowBox(moveData.m_arrow, moveData.m_arrowBox->getRenderer());
			float endPos   = moveData.m_iterationData[0].m_moveRight ? 1.0 : 0.0;
			Arrow::Renderer& arrow = *moveData.m_arrow.first.get().getArrowRendererInList();
			moveArrowThroughtOneHandle(cmd, endPos, startPos, moveData.m_iterationData[0].m_rightI, moveData.m_iterationData[0].m_rightJ,
				arrow, current.m_oneHandle.getRenderer(), time);
		} else {
			// TODO: put arrow far away (after other commands)
		}
	} else { // Il n'y a qu'une seule iteration, il faut simplement deplacer la fleche dans le 1-handle
		float f = t + ArrowBox::Renderer::arrowSeparation()/moveData.m_iterationData[0].m_oneHandle.getRenderer().lenght();
		if (f>0) {
			float startPos = getPosInArrowBox(moveData.m_arrow, moveData.m_arrowBox->getRenderer());
			float endPos   = moveData.m_iterationData[0].m_moveRight ? 0.0+f : 1.0-f;
			Arrow::Renderer& arrow = *moveData.m_arrow.first.get().getArrowRendererInList();
			
			if (!m_arrows.empty() && m_arrows.top().m_iterationStartIndex == iterationIndex) {
				isLeadingArrow = false;
				MoveData moveData(std::move(m_arrows.top())); m_arrows.pop();
				float pos = getPosInArrowBox(moveData.m_arrow, moveData.m_arrowBox->getRenderer());
				float moveTime = time+std::abs(pos-startPos)*moveData.m_iterationData[0].m_oneHandle.getRenderer().lenght()/ArrowBox::Renderer::arrowSeparation()-1.0;
				moveArrow(cmd, moveData, moveTime, index+1, iterationIndex, f);
			}
			
			moveArrowThroughtOneHandle(cmd, startPos, endPos, moveData.m_iterationData[0].m_rightI, moveData.m_iterationData[0].m_rightJ,
				arrow, moveData.m_arrowBox->getRenderer().getOneHandle(), time);
		}
	}
}

void MoveArrowsAcrossZeroHandle::addArrow(ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow, std::vector<IterationData>&& iterationData,
		 size_t iterationStartIndex, bool shouldRemoveArrow) {
	assert(!iterationData.empty());
	m_arrows.emplace(arrowBox, arrow, std::move(iterationData), iterationStartIndex, shouldRemoveArrow);
}

void MoveArrowsAcrossZeroHandle::run(RendererGL& rendererGL) {
	assert(m_arrows.empty() || m_arrows.top().m_iterationStartIndex == 0);
	assert(!m_arrows.top().m_iterationData.empty());
	
	AsynchronousSubTaskCommand* cmd = AsynchronousSubTaskCommand::create();
	MoveData moveData(std::move(m_arrows.top())); m_arrows.pop();
	moveArrow(cmd, moveData, 0.0, 0, 0, 0.0);
	subCommandQueue.push(cmd);
	subCommandQueue.push(RefreshArrowBoxCommand::create(m_zeroHandle.getRenderer().getVoidHandle().getFirstArrowBox(), false, true));
	subCommandQueue.push(RefreshArrowBoxCommand::create(m_zeroHandle.getRenderer().getVoidHandle().getSecondArrowBox(), false, true));
	subCommandQueue.push(RefreshArrowBoxCommand::create(m_zeroHandle.getRenderer().getFullHandle().getFirstArrowBox(), false, true));
	subCommandQueue.push(RefreshArrowBoxCommand::create(m_zeroHandle.getRenderer().getFullHandle().getSecondArrowBox(), false, true));
}

class PushArrowCommand : public Command<RendererGL&> {
	ArrowBox& m_arrowBox;
	ArrowBox::ArrowInArrowBox& m_newArrow;
	unsigned int m_from, m_to;
	
	PushArrowCommand(ArrowBox& arrowBox, ArrowBox::ArrowInArrowBox& newArrow, unsigned int from, unsigned int to) :
		m_arrowBox(arrowBox), m_newArrow(newArrow), m_from(from), m_to(to) {}
	
public:
	static Command<RendererGL&>* create(ArrowBox& arrowBox, ArrowBox::ArrowInArrowBox& newArrow, unsigned int from, unsigned int to) {
		return new PushArrowCommand(arrowBox, newArrow, from, to);
	}
	
	virtual void run(RendererGL& rendererGL) {
		m_arrowBox.getRenderer().pushBackArrow(rendererGL, m_newArrow, m_from, m_to);
	}
	
	virtual std::string name() const {return "PushArrowCommand";}
};









void SynchronousSubTaskCommand::run(RendererGL& rendererGL, float dt, float& pastTime) {
	m_t+=dt;
	pastTime = m_t-m_duration;
	
	if (pastTime < 0.0){
		float p = m_t/m_duration;
		for (auto it=m_tasks.begin(); it!=m_tasks.end(); it++)
			it->m_cmd->run(rendererGL, p);
	}
}

void RefreshArrowBoxCommand::run(RendererGL& rendererGL, float dt, float& pastTime) {
	m_arrowBox.refreshArrows();
	if (m_refreshLenght) m_arrowBox.refreshLenght();
	if (m_refreshTracks) m_arrowBox.refreshTracks();
	pastTime = 0.0;
}

void RefreshPermutationBoxCommand::run(RendererGL& rendererGL, float dt, float& pastTime) {
	m_permutationBox.refresh();
	pastTime = 0.0;
}

void RemoveArrowCommand::run(RendererGL& rendererGL, float dt, float& pastTime) {
	m_arrowBox.removeArrow(m_arrow);
	pastTime = 0.0;
}

void RemoveCrossingCommand::run(RendererGL& rendererGL, float dt, float& pastTime) {
	m_arrowBox.deleteCrossing();
	pastTime = 0.0;
}

void RemoveTrackCommand::run(RendererGL& rendererGL, float dt, float& pastTime) {
	for (size_t i=0; i<m_curves.size(); i++)
		delete m_curves[i];
	
	m_curves.clear();
	pastTime=0.0;
}

std::pair<int, bool> pushMoveSubcommand(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow) {
	int n = targetArrow.second - movingArrow.second;
	
	bool forward = false;
	if (n>0) {
		n--; forward = true;
	} else n++;
	if (n != 0) {
		SynchronousSubTaskCommand* cmd = SynchronousSubTaskCommand::create(std::abs(n)/3.0);
		AnimateMoveArrowInArrowBoxCommand* anim = AnimateMoveArrowInArrowBoxCommand::create(arrowBox.getRenderer(), false);
		cmd->addCommand(anim);
		arrowBox.getRenderer().moveArrow(movingArrow.first.get().getArrowRendererInList(), movingArrow.second, n, anim);
		subCommandQueue.push(cmd);
	}
	
	return std::make_pair(n, forward);
}

void DeleteZeroHandleCommand::run(RendererGL& rendererGL) {
	ZeroHandleRenderer& zeroHandle = m_zeroHandle.getRenderer();
	assert(&m_zeroHandle.getRenderer() == zeroHandleCurrent);
	delete &zeroHandle;
	delete &m_zeroHandle;
	zeroHandleCurrent = nullptr;
}

void DrawZeroHandleCommand::run(RendererGL& rendererGL) {
	assert(zeroHandleCurrent == nullptr);
	zeroHandleCurrent = new ZeroHandleRenderer(rendererGL, m_pairing, m_zeroHandle, m_voidArrowsData, m_voidArrows, m_fullArrowsData, m_fullArrows);
}

void PermuteArrowBoxCommand::run(RendererGL& rendererGL) {
	OneHandleRenderer& oneHandle = m_oneHandle.getRenderer();
	ArrowBox::Renderer& arrowBox = (m_isFirstArrowBox) ? oneHandle.getFirstArrowBox() : oneHandle.getSecondArrowBox();
	PermutationBox::Renderer& permutation = oneHandle.getPermutation();
	PermutationBox::Renderer& sidePermutation = (m_isFirstArrowBox) ? oneHandle.getPrePermutation() : oneHandle.getPostPermutation();
	
	SynchronousSubTaskCommand* cmd = SynchronousSubTaskCommand::create(10.0/3.0);
	AnimateMoveTrackLineArrowBox*      animArrowTrack     = AnimateMoveTrackLineArrowBox::create(arrowBox);
	AnimateMoveArrowInArrowBoxCommand* animArrows         = AnimateMoveArrowInArrowBoxCommand::create(arrowBox, true);
	AnimateMoveTrackPermutationBox*    animPrePermutation = AnimateMoveTrackPermutationBox::create(sidePermutation);
	AnimateMoveTrackPermutationBox*    animPermutation    = AnimateMoveTrackPermutationBox::create(permutation);
	cmd->addCommand(animArrowTrack); cmd->addCommand(animArrows); cmd->addCommand(animPrePermutation); cmd->addCommand(animPermutation);
	
	arrowBox.permuteTracks(m_permutation.getPermutation(), animArrowTrack, animArrows);
	if (!m_isFirstArrowBox) m_permutation.inverse();
	sidePermutation.permute(m_permutation, animPrePermutation, m_isFirstArrowBox);
	m_permutation.inverse();
	permutation.permute(m_permutation, animPermutation, !m_isFirstArrowBox);
	if (m_isFirstArrowBox) m_permutation.inverse();
	
	subCommandQueue.push(cmd);
	subCommandQueue.push(RefreshPermutationBoxCommand::create(sidePermutation));
	subCommandQueue.push(RefreshArrowBoxCommand::create(arrowBox, false, true));
	subCommandQueue.push(RefreshPermutationBoxCommand::create(permutation));
}

void MoveArrowInArrowBoxCommand::run(RendererGL& rendererGL) {
	ArrowBox::Renderer& arrowBox = m_arrowBox.getRenderer();
	int n = m_targetArrow.second - m_movingArrow.second;
	
	SynchronousSubTaskCommand* cmd = SynchronousSubTaskCommand::create(std::abs(n)/3.0);
	AnimateMoveArrowInArrowBoxCommand* anim = AnimateMoveArrowInArrowBoxCommand::create(arrowBox, false);
	cmd->addCommand(anim);
	
	arrowBox.moveArrow(m_movingArrow.first.get().getArrowRendererInList(), m_movingArrow.second, n, anim);
	subCommandQueue.push(cmd);
	subCommandQueue.push(RefreshArrowBoxCommand::create(arrowBox, false));
}

void GenArrowAfterMoveCrossingCommand::run(RendererGL& rendererGL) {
	ArrowBox::Renderer& arrowBox = m_arrowBox.getRenderer();
	
	std::pair<int, bool> moveData = pushMoveSubcommand(m_arrowBox, m_movingArrow, m_targetArrow);
	bool& forward(moveData.second);
	
	SynchronousSubTaskCommand*         cmd0     = SynchronousSubTaskCommand::create(0.5/3.0);
	AnimateMoveArrowInArrowBoxCommand* anim0    = AnimateMoveArrowInArrowBoxCommand::create(arrowBox, false);
	cmd0->addCommand(anim0);
	
	SynchronousSubTaskCommand*         cmd1     = SynchronousSubTaskCommand::create(1.5/3.0);
	AnimateMoveArrowInArrowBoxCommand* anim1    = AnimateMoveArrowInArrowBoxCommand::create(arrowBox, false);
	UpdateArrowBoxLenghtCommand*       lenAnim1 = UpdateArrowBoxLenghtCommand::create(arrowBox);
	cmd1->addCommand(lenAnim1); cmd1->addCommand(anim1);
	
	if (forward)
		arrowBox.spawnArrowAfterCrossingForward (rendererGL, m_targetArrow.first.get().getArrowRendererInList(), m_newArrow, m_targetArrow.second,
				m_from, m_to, anim0, anim1, lenAnim1, m_genAfter);
	else
		arrowBox.spawnArrowAfterCrossingBackward(rendererGL, m_targetArrow.first.get().getArrowRendererInList(), m_newArrow, m_targetArrow.second,
				m_from, m_to, anim0, anim1, lenAnim1, m_genAfter);
	
	subCommandQueue.push(cmd0);
	subCommandQueue.push(cmd1);
	subCommandQueue.push(RefreshArrowBoxCommand::create(arrowBox, true));
}

void MoveMergeArrowsCommand::run(RendererGL& rendererGL) {
	ArrowBox::Renderer& arrowBox = m_arrowBox.getRenderer();
	std::pair<int, bool> moveData = pushMoveSubcommand(m_arrowBox, m_movingArrow, m_targetArrow);
	bool& forward(moveData.second);
	
	SynchronousSubTaskCommand*         cmd0     = SynchronousSubTaskCommand::create(0.5/3.0);
	AnimateMoveArrowInArrowBoxCommand* anim0    = AnimateMoveArrowInArrowBoxCommand::create(arrowBox, false);
	UpdateArrowBoxLenghtCommand*       lenAnim0 = UpdateArrowBoxLenghtCommand::create(arrowBox);
	cmd0->addCommand(lenAnim0); cmd0->addCommand(anim0);
	
	SynchronousSubTaskCommand*         cmd1     = SynchronousSubTaskCommand::create(1.5/3.0);
	AnimateMoveArrowInArrowBoxCommand* anim1    = AnimateMoveArrowInArrowBoxCommand::create(arrowBox, false);
	UpdateArrowBoxLenghtCommand*       lenAnim1 = UpdateArrowBoxLenghtCommand::create(arrowBox);
	cmd1->addCommand(lenAnim1); cmd1->addCommand(anim1);
	
	arrowBox.mergeArrows(rendererGL, m_targetArrow.first.get().getArrowRendererInList(), m_targetArrow.second, anim0, lenAnim0, anim1, lenAnim1, forward);
	
	subCommandQueue.push(cmd0);
	subCommandQueue.push(RemoveArrowCommand::create(arrowBox, m_movingArrow.first));
	subCommandQueue.push(RemoveArrowCommand::create(arrowBox, m_targetArrow.first));
	subCommandQueue.push(cmd1);
	subCommandQueue.push(RefreshArrowBoxCommand::create(arrowBox, true));
}

void RemoveArrowFromArrowBoxCommand::run(RendererGL& rendererGL) {
	ArrowBox::Renderer& arrowBox = m_arrowBox.getRenderer();
	ArrowBox::ArrowRendererInList arrowInList = m_arrow.first.get().getArrowRendererInList();
	int& index = m_arrow.second;
	
	SynchronousSubTaskCommand* cmd0 = SynchronousSubTaskCommand::create(1.0/3.0);
	AnimateMoveArrowInArrowBoxCommand* anim0    = AnimateMoveArrowInArrowBoxCommand::create(arrowBox, false);
	UpdateArrowBoxLenghtCommand*       lenAnim0 = UpdateArrowBoxLenghtCommand::create(arrowBox);
	cmd0->addCommand(lenAnim0); cmd0->addCommand(anim0);
	
	arrowBox.removeArrow(rendererGL, arrowInList, index, anim0, lenAnim0);
	
	subCommandQueue.push(RemoveArrowCommand::create(arrowBox, m_arrow.first));
	subCommandQueue.push(cmd0);
	subCommandQueue.push(RefreshArrowBoxCommand::create(arrowBox, true));
}

void MoveArrowGenCrossingCommand::run(RendererGL& rendererGL) {
	ArrowBox::Renderer& arrowBox = m_arrowBox.getRenderer();
	assert(&arrowBox == &(arrowBox.getOneHandle().getFirstArrowBox()));
	
	std::pair<int, bool> moveData = pushMoveSubcommand(m_arrowBox, m_movingArrow, m_targetArrow);
#ifndef NDEBUG
	bool& forward(moveData.second);
	assert(forward);
#endif
	
	SynchronousSubTaskCommand*         cmd0  = SynchronousSubTaskCommand::create(1.0/3.0);
	AnimateMoveArrowInArrowBoxCommand* anim0 = AnimateMoveArrowInArrowBoxCommand::create(arrowBox, false);
	cmd0->addCommand(anim0);
	
	SynchronousSubTaskCommand* cmd1 = SynchronousSubTaskCommand::create(1.0/3.0);
	GenCrossingCommand*        anim1 = nullptr;
	
	std::vector<MoveCrossingCommand*> anims2;
	FadeCrossingCommand* anim3 = nullptr;
	arrowBox.genCrossing(rendererGL, m_targetArrow.first.get().getArrowRendererInList(), m_targetArrow.second,
			anim0, anim1, anims2, anim3, m_crossingI, m_crossingJ);
	cmd1->addCommand(anim1);
	
	subCommandQueue.push(cmd0);
	subCommandQueue.push(RemoveArrowCommand::create(arrowBox, m_movingArrow.first));
	subCommandQueue.push(cmd1);
	
	for (size_t i=0; i<anims2.size(); i++) {
		SynchronousSubTaskCommand* cmd = SynchronousSubTaskCommand::create(1.0/3.0);
		cmd->addCommand(anims2[i]);
		subCommandQueue.push(cmd);
	}
	
	SynchronousSubTaskCommand*      cmd3  = SynchronousSubTaskCommand::create(1.0/3.0);
	UpdateArrowBoxLenghtCommand*    animLen3 = UpdateArrowBoxLenghtCommand::create(arrowBox);
	animLen3->setVariation(arrowBox.lenght(), arrowBox.lenght()-ArrowBox::Renderer::arrowSeparation());
	AnimateMoveTrackPermutationBox* animPer3 = AnimateMoveTrackPermutationBox::create(arrowBox.getOneHandle().getPermutation());
	arrowBox.getOneHandle().getPermutation().permute(std::make_pair(m_crossingI, m_crossingJ), animPer3, false);
	cmd3->addCommand(animLen3); cmd3->addCommand(animPer3); cmd3->addCommand(anim3);
	subCommandQueue.push(cmd3);
	subCommandQueue.push(RemoveCrossingCommand::create(arrowBox));
	subCommandQueue.push(RefreshArrowBoxCommand::create(arrowBox, true));
}

void MoveArrowToFirstArrowBoxCommand::run(RendererGL& rendererGL) {
	OneHandleRenderer& oneHandle = m_oneHandle.getRenderer();
	ArrowBox::Renderer& firstBox = oneHandle.getFirstArrowBox();
	ArrowBox::Renderer& secondBox = oneHandle.getSecondArrowBox();
	
	AsynchronousSubTaskCommand* cmd0 = AsynchronousSubTaskCommand::create();
	float arrowBox0Lenght = firstBox.lenght();
	float arrowBox1Lenght = secondBox.lenght();
	float duration = arrowBox1Lenght/(3.0*ArrowBox::Renderer::arrowSeparation());
	UpdateArrowBoxLenghtCommand* animLen0 = UpdateArrowBoxLenghtCommand::create(firstBox, false);
	animLen0->setVariation(arrowBox0Lenght, arrowBox0Lenght+arrowBox1Lenght);
	UpdateArrowBoxLenghtCommand* animLen1 = UpdateArrowBoxLenghtCommand::create(secondBox, false);
	animLen1->setVariation(arrowBox1Lenght, 0.0);
	cmd0->addCommand(animLen0, 0.0, duration); cmd0->addCommand(animLen1, 0.0, duration);
	
	oneHandle.transferArrowsToFirstArrowBox(cmd0, duration);
	subCommandQueue.push(cmd0);
	subCommandQueue.push(RefreshArrowBoxCommand::create(firstBox, true));
	subCommandQueue.push(RefreshArrowBoxCommand::create(secondBox, true));
}

void MoveArrowToOtherArrowBoxCommand::run(RendererGL& rendererGL) {
	OneHandleRenderer&  oneHandle = m_oneHandle.getRenderer();
	ArrowBox::Renderer& box = m_arrowBox.getRenderer();
	
	const bool isFirstArrowBox = (&box == &(oneHandle.getFirstArrowBox()));
	assert((&box == &(oneHandle.getSecondArrowBox())) || isFirstArrowBox);
	
	ArrowBox::Renderer& otherBox = (isFirstArrowBox) ? oneHandle.getSecondArrowBox() : oneHandle.getFirstArrowBox();
	PermutationBox::Renderer& permutation = oneHandle.getPermutation();
	ArrowBox::ArrowRendererInList arrowRen = m_arrow.first.get().getArrowRendererInList();
	const unsigned int arrowBegin = arrowRen->begin();
	const unsigned int arrowEnd   = arrowRen->end();
	const int& arrowPos = m_arrow.second;
	
	const int n = (isFirstArrowBox) ? box.size() - arrowPos - 1 : arrowPos;
	const unsigned int& leftI  = (isFirstArrowBox) ? arrowBegin : m_targetI;
	const unsigned int& leftJ  = (isFirstArrowBox) ? arrowEnd   : m_targetJ;
	const float permutationAnimStart = (isFirstArrowBox) ? 0.0 : 1.0;
	
	SynchronousSubTaskCommand* cmd0 = SynchronousSubTaskCommand::create((n + 0.5)/3.0);
	AnimateMoveArrowInArrowBoxCommand* anim0 = AnimateMoveArrowInArrowBoxCommand::create(box, false);
	cmd0->addCommand(anim0);
	
	if (isFirstArrowBox)
		box.moveArrowToEnd(arrowRen, arrowPos, box, anim0);
	else
		box.moveArrowToBegin(arrowRen, arrowPos, box, anim0);
	
	SynchronousSubTaskCommand* cmd1 = SynchronousSubTaskCommand::create(permutation.lenght()/ArrowBox::Renderer::arrowSeparation() - 1.0);
	AnimateMoveArrowPermutationBox* anim1 = AnimateMoveArrowPermutationBox::create(permutation, *arrowRen, leftI, leftJ, permutationAnimStart, 1.0-permutationAnimStart);
	UpdateArrowBoxLenghtCommand* animLen0 = UpdateArrowBoxLenghtCommand::create(box, false);
	animLen0->setVariation(box.lenght(), box.lenght()-ArrowBox::Renderer::arrowSeparation());
	UpdateArrowBoxLenghtCommand* animLen1 = UpdateArrowBoxLenghtCommand::create(otherBox, false);
	animLen1->setVariation(otherBox.lenght(), otherBox.lenght()+ArrowBox::Renderer::arrowSeparation());
	cmd1->addCommand(anim1); cmd1->addCommand(animLen0); cmd1->addCommand(animLen1);
	
	SynchronousSubTaskCommand* cmd2 = SynchronousSubTaskCommand::create(0.5/3.0);
	AnimateMoveArrowInArrowBoxCommand* anim2 = AnimateMoveArrowInArrowBoxCommand::create(otherBox, false);
	cmd2->addCommand(anim2);
	arrowRen->setBeginEnd(m_targetI, m_targetJ);
	
	subCommandQueue.push(cmd0);
	subCommandQueue.push(cmd1);
	subCommandQueue.push(cmd2);
	subCommandQueue.push(RefreshArrowBoxCommand::create(box, true));
	subCommandQueue.push(RefreshArrowBoxCommand::create(otherBox, true));
	
	if (isFirstArrowBox)
		otherBox.moveArrowFromBegin(arrowRen, 0, box, anim2);
	else
		otherBox.moveArrowFromEnd(arrowRen, 0, box, anim2);
}

void MoveArrowToOtherArrowBoxResolveCrossingCommand::run(RendererGL& rendererGL) {		
	OneHandleRenderer&  oneHandle = m_oneHandle.getRenderer();
	ArrowBox::Renderer& box = m_arrowBox.getRenderer();
	
	const bool isFirstArrowBox = (&box == &(oneHandle.getFirstArrowBox()));
	assert((&box == &(oneHandle.getSecondArrowBox())) || isFirstArrowBox);
	
	ArrowBox::Renderer& otherBox = (isFirstArrowBox) ? oneHandle.getSecondArrowBox() : oneHandle.getFirstArrowBox();
	ArrowBox::ArrowRendererInList arrowRen = m_arrow.first.get().getArrowRendererInList();
	PermutationBox::Renderer& permutation = oneHandle.getPermutation();
	const unsigned int arrowBegin = arrowRen->begin();
	const unsigned int arrowEnd   = arrowRen->end();
	const int& arrowPos = m_arrow.second;
	
	const int n = (isFirstArrowBox) ? box.size() - arrowPos - 1 : arrowPos;
	const unsigned int& leftI  = (isFirstArrowBox) ? arrowBegin : m_targetI;
	const unsigned int& leftJ  = (isFirstArrowBox) ? arrowEnd   : m_targetJ;
	const unsigned int& rightI = (isFirstArrowBox) ? m_targetI  : arrowBegin;
	const unsigned int& rightJ = (isFirstArrowBox) ? m_targetJ  : arrowEnd;
	const float permutationAnimStart = (isFirstArrowBox) ? 0.0 : 1.0;
	
	SynchronousSubTaskCommand* cmd0 = SynchronousSubTaskCommand::create((n + 0.5)/3.0);
	AnimateMoveArrowInArrowBoxCommand* anim0 = AnimateMoveArrowInArrowBoxCommand::create(box, false);
	cmd0->addCommand(anim0);
	
	ShowableCurve& trackI(permutation.getTrack(leftI).getCurve());
	ShowableCurve& trackJ(permutation.getTrack(leftJ).getCurve());
	const float t = static_cast<Bezier&>(trackI.getCurve()).getIntersectionXCoordMatch(static_cast<Bezier&>(trackJ.getCurve()));
	SynchronousSubTaskCommand* cmd1 = SynchronousSubTaskCommand::create(t*permutation.lenght()/ArrowBox::Renderer::arrowSeparation());
	AnimateMoveArrowPermutationBox* anim1 = AnimateMoveArrowPermutationBox::create(permutation, *arrowRen, leftI, leftJ, permutationAnimStart, t);
	cmd1->addCommand(anim1);
	
	Bezier bezier00a(static_cast<Bezier&>(trackI.getCurve()));
	Bezier bezier10a(static_cast<Bezier&>(trackJ.getCurve()));
	Bezier bezier01a(bezier00a.splitCurve(t, true));
	Bezier bezier11a(bezier10a.splitCurve(t, true));
	
	Bezier bezier00b(permutation.getBezier(leftI, rightI));
	Bezier bezier10b(permutation.getBezier(leftJ, rightJ));
	Bezier bezier01b(bezier00b.splitCurve(t, true));
	Bezier bezier11b(bezier10b.splitCurve(t, true));
	
	TrackBezier* trackI0 = new TrackBezier(rendererGL, 1e+10, 1e+10, 1e+10, 1e+10, trackColor.r, trackColor.g, trackColor.b, permutation.getLayer(), EAST, WEST);
	TrackBezier* trackJ0 = new TrackBezier(rendererGL, 1e+10, 1e+10, 1e+10, 1e+10, trackColor.r, trackColor.g, trackColor.b, permutation.getLayer(), EAST, WEST);
	
	SynchronousSubTaskCommand* cmd2 = SynchronousSubTaskCommand::create(2.0/3.0);
	AnimateInterpolateShowableBezierCurve* animCurve = AnimateInterpolateShowableBezierCurve::create();
	animCurve->addCurve(static_cast<ShowableBezier&>(trackI),  bezier00a, bezier00b);
	animCurve->addCurve(static_cast<ShowableBezier&>(trackI0->getCurve()), bezier01a, bezier11b);
	animCurve->addCurve(static_cast<ShowableBezier&>(trackJ),  bezier10a, bezier10b);
	animCurve->addCurve(static_cast<ShowableBezier&>(trackJ0->getCurve()), bezier11a, bezier01b);
	
	std::pair<ArrowBox::ArrowRendererInList, int> p;
	if (isFirstArrowBox) {
		box.moveArrowToEnd(arrowRen, arrowPos, box, anim0);
		p = box.pushBackArrow(rendererGL, m_newArrow, arrowEnd, arrowBegin, false);
	} else {
		box.moveArrowToBegin(arrowRen, arrowPos, box, anim0);
		p = box.pushFrontArrow(rendererGL, m_newArrow, arrowEnd, arrowBegin, false);
	}
	ArrowBox::ArrowRendererInList& newArrowRen = p.first;
	
	AnimateMoveArrowPermutationBox* animPer0 = AnimateMoveArrowPermutationBox::create(permutation, *arrowRen, leftI, leftJ, 1.0, 1.0);
	AnimateMoveArrowPermutationBox* animPer1 = AnimateMoveArrowPermutationBox::create(permutation, *newArrowRen, leftJ, leftI, 1.0, 1.0);
	cmd2->addCommand(animCurve); cmd2->addCommand(animPer0); cmd2->addCommand(animPer1);
	permutation.permute(std::make_pair(rightJ, rightI), nullptr, true);
	
	RemoveTrackCommand* cmd3 = RemoveTrackCommand::create();
	cmd3->addCurve(*trackI0); cmd3->addCurve(*trackJ0);
	
	float f = std::max(t, 1.0f-t);
	SynchronousSubTaskCommand* cmd4 = SynchronousSubTaskCommand::create(f*permutation.lenght()/ArrowBox::Renderer::arrowSeparation());
	AnimateMoveArrowPermutationBox* anim2 = AnimateMoveArrowPermutationBox::create(permutation, *arrowRen, leftI, leftJ, t, 1.0-permutationAnimStart);
	AnimateMoveArrowPermutationBox* anim3 = AnimateMoveArrowPermutationBox::create(permutation, *newArrowRen, leftJ, leftI, t, permutationAnimStart);
	cmd4->addCommand(anim2); cmd4->addCommand(anim3);
	
	SynchronousSubTaskCommand* cmd5 = SynchronousSubTaskCommand::create(1.0/3.0);
	AnimateMoveArrowInArrowBoxCommand* anim4 = AnimateMoveArrowInArrowBoxCommand::create(otherBox, false);
	AnimateMoveArrowInArrowBoxCommand* anim5 = AnimateMoveArrowInArrowBoxCommand::create(box, false);
	UpdateArrowBoxLenghtCommand* animLen1 = UpdateArrowBoxLenghtCommand::create(otherBox);
	animLen1->setVariation(otherBox.lenght(), otherBox.lenght()+ArrowBox::Renderer::arrowSeparation());
	cmd5->addCommand(animLen1); cmd5->addCommand(anim4); cmd5->addCommand(anim5);
	arrowRen->setBeginEnd(m_targetI, m_targetJ);
	
	if (isFirstArrowBox) {
		otherBox.moveArrowFromBeginWithOthers(arrowRen, 0, box, anim4);
		box.moveArrowFromEnd(newArrowRen, 0, box, anim5);
	} else {
		otherBox.moveArrowFromEnd(arrowRen, 0, box, anim4);
		box.moveArrowFromBeginWithOthers(newArrowRen, 0, box, anim5);
	}
	
	subCommandQueue.push(cmd0);
	subCommandQueue.push(cmd1);
	subCommandQueue.push(cmd2);
	subCommandQueue.push(cmd3);
	subCommandQueue.push(RefreshPermutationBoxCommand::create(permutation));
	subCommandQueue.push(cmd4);
	subCommandQueue.push(cmd5);
	subCommandQueue.push(RefreshArrowBoxCommand::create(box, true));
	subCommandQueue.push(RefreshArrowBoxCommand::create(otherBox, true));
}

void PassArrowsThroughtZeroHandleCommand::run(RendererGL& rendererGL) {
	AsynchronousSubTaskCommand* cmd = AsynchronousSubTaskCommand::create();
	
	ArrowBox::Renderer& arrowBox = m_arrowBox.getRenderer();
	OneHandleRenderer& oneHandle = arrowBox.getOneHandle();
	ZeroHandleRenderer& zeroHandle = oneHandle.getZeroHandle();
	const bool isFirstArrowBox = (&arrowBox == &(oneHandle.getFirstArrowBox()));
	assert((&arrowBox == &(oneHandle.getSecondArrowBox())) || isFirstArrowBox);
	
	size_t size = arrowBox.size();
	size_t firstVoidArrowBoxNum = 0;
	size_t secondVoidArrowBoxNum = 0;
	size_t firstFullArrowBoxNum = 0;
	size_t secondFullArrowBoxNum = 0;
	for (size_t i=0; i<m_moveData.size(); ++i) {
		int edge = zeroHandle.getEdgeFromArrowBox(m_moveData[i].m_targetArrowBox.getRenderer());
		if (edge==0)
			++firstFullArrowBoxNum;
		else if (edge==1)
			++secondVoidArrowBoxNum;
		else if (edge==2)
			++secondFullArrowBoxNum;
		else
			++firstVoidArrowBoxNum;
	}
	
	if (&arrowBox!=&(zeroHandle.getVoidHandle().getFirstArrowBox()) && firstVoidArrowBoxNum>0) {
		ArrowBox::Renderer& updateBox = zeroHandle.getVoidHandle().getFirstArrowBox();
		UpdateArrowBoxLenghtCommand* lenAnim = UpdateArrowBoxLenghtCommand::create(updateBox, true);
		AnimateMoveArrowInArrowBoxCommand* anim = AnimateMoveArrowInArrowBoxCommand::create(updateBox, false);
		updateBox.moveArrowsFake(0, firstVoidArrowBoxNum, anim);
		lenAnim->setVariation(updateBox.lenght(), updateBox.lenght()+firstVoidArrowBoxNum*ArrowBox::Renderer::arrowSeparation());
		cmd->addCommand(lenAnim, 0.0, 1.0*firstVoidArrowBoxNum);
		cmd->addCommand(anim, 1e-5, 1.0*firstVoidArrowBoxNum);
	}
	
	if (&arrowBox!=&(zeroHandle.getVoidHandle().getSecondArrowBox()) && secondVoidArrowBoxNum>0) {
		ArrowBox::Renderer& updateBox = zeroHandle.getVoidHandle().getSecondArrowBox();
		UpdateArrowBoxLenghtCommand* lenAnim = UpdateArrowBoxLenghtCommand::create(updateBox, true);
		lenAnim->setVariation(updateBox.lenght(), updateBox.lenght()+secondVoidArrowBoxNum*ArrowBox::Renderer::arrowSeparation());
		cmd->addCommand(lenAnim, 0.0, 1.0*secondVoidArrowBoxNum);
	}
	
	if (&arrowBox!=&(zeroHandle.getFullHandle().getFirstArrowBox()) && firstFullArrowBoxNum>0) {
		ArrowBox::Renderer& updateBox = zeroHandle.getFullHandle().getFirstArrowBox();
		UpdateArrowBoxLenghtCommand* lenAnim = UpdateArrowBoxLenghtCommand::create(updateBox, true);
		AnimateMoveArrowInArrowBoxCommand* anim = AnimateMoveArrowInArrowBoxCommand::create(updateBox, false);
		updateBox.moveArrowsFake(0, firstFullArrowBoxNum, anim);
		lenAnim->setVariation(updateBox.lenght(), updateBox.lenght()+firstFullArrowBoxNum*ArrowBox::Renderer::arrowSeparation());
		cmd->addCommand(lenAnim, 0.0, 1.0*firstFullArrowBoxNum);
		cmd->addCommand(anim, 1e-5, 1.0*firstFullArrowBoxNum);
	}
	
	if (&arrowBox!=&(zeroHandle.getFullHandle().getSecondArrowBox()) && secondFullArrowBoxNum>0) {
		ArrowBox::Renderer& updateBox = zeroHandle.getFullHandle().getSecondArrowBox();
		UpdateArrowBoxLenghtCommand* lenAnim = UpdateArrowBoxLenghtCommand::create(updateBox, true);
		lenAnim->setVariation(updateBox.lenght(), updateBox.lenght()+secondFullArrowBoxNum*ArrowBox::Renderer::arrowSeparation());
		cmd->addCommand(lenAnim, 0.0, 1.0*secondFullArrowBoxNum);
	}
	
	UpdateArrowBoxLenghtCommand* lenAnim = UpdateArrowBoxLenghtCommand::create(arrowBox, true);
	lenAnim->setVariation(arrowBox.lenght(), arrowBox.lenght()-ArrowBox::Renderer::arrowSeparation()*m_moveData.size());
	cmd->addCommand(lenAnim, 0.0, 1.0*m_moveData.size());
	
	if (isFirstArrowBox) {
		for (size_t i=0; i<m_moveData.size(); ++i) {
			MoveData& moveData = m_moveData[i];
			ArrowBox::ArrowRendererInList it = moveData.m_arrow.first.get().getArrowRendererInList();
			
			ArrowBox::Renderer& targetArrowBox = moveData.m_targetArrowBox.getRenderer();
			OneHandleRenderer& targetOneHandle = targetArrowBox.getOneHandle();
			const bool targetIsFirstArrowBox = (&targetArrowBox == &(targetOneHandle.getFirstArrowBox()));
			assert((&targetArrowBox == &(targetOneHandle.getSecondArrowBox())) || targetIsFirstArrowBox);
			
			AnimateMoveArrowInArrowBoxCommand* anim0 = AnimateMoveArrowInArrowBoxCommand::create(arrowBox, false);
			arrowBox.moveArrowToBeginFake(it, moveData.m_arrow.second, arrowBox, anim0);
			AnimateMoveArrowPermutationBox* anim1 = AnimateMoveArrowPermutationBox::create(oneHandle.getPrePermutation(), *it,
					moveData.m_beginI, moveData.m_beginJ, 1.0, 0.0);
			
			it->setBeginEnd(moveData.m_targetI, moveData.m_targetJ);
			
			int arrowBoxEdge = zeroHandle.getEdgeFromArrowBox(arrowBox);
			int targetArrowBoxEdge = zeroHandle.getEdgeFromArrowBox(targetArrowBox);
			unsigned int beginI = zeroHandle.getIndexFromTrack(moveData.m_beginI, arrowBoxEdge);
			unsigned int beginJ = zeroHandle.getIndexFromTrack(moveData.m_beginJ, arrowBoxEdge);
			unsigned int endI   = zeroHandle.getIndexFromTrack(moveData.m_endI,   targetArrowBoxEdge);
			unsigned int endJ   = zeroHandle.getIndexFromTrack(moveData.m_endJ,   targetArrowBoxEdge);
			int layer = (targetArrowBoxEdge==0 || targetArrowBoxEdge==2) ? 2 : 0;
			Command<RendererGL&, float>* layerChangeCmd = ArrowSetLayerCommand::create(*it, layer);
			Command<RendererGL&, float>* anim2 = MoveArrowThroughtZeroHandleCommand::create(*it, zeroHandle, beginI, endI, beginJ, endJ, 0.0, 1.0);
			
			
			AnimateMoveArrowPermutationBox* anim3;
			AnimateMoveArrowInArrowBoxCommand* anim4;
			float targetPerLenght;
			if (targetIsFirstArrowBox) {
				targetPerLenght = targetOneHandle.getPrePermutation().lenght();
				anim3 = AnimateMoveArrowPermutationBox::create(targetOneHandle.getPrePermutation(), *it, moveData.m_endI, moveData.m_endJ, 0.0, 1.0);
				anim4 = AnimateMoveArrowInArrowBoxCommand::create(targetArrowBox, false);
				
				if (&targetOneHandle==&(zeroHandle.getVoidHandle())) {
					targetArrowBox.moveArrowFromBeginFake(it, --firstVoidArrowBoxNum, arrowBox, anim4);
				} else {
					assert(&targetOneHandle==&(zeroHandle.getFullHandle()));
					targetArrowBox.moveArrowFromBeginFake(it, --firstFullArrowBoxNum, arrowBox, anim4);
				}
			} else {
				targetPerLenght = targetOneHandle.getPostPermutation().lenght();
				anim3 = AnimateMoveArrowPermutationBox::create(targetOneHandle.getPostPermutation(), *it, moveData.m_targetI, moveData.m_targetJ, 1.0, 0.0);
				anim4 = AnimateMoveArrowInArrowBoxCommand::create(targetArrowBox, false);
				
				if (&targetOneHandle==&(zeroHandle.getVoidHandle())) {
					targetArrowBox.moveArrowFromEndFake(it, --secondVoidArrowBoxNum, arrowBox, anim4);
				} else {
					assert(&targetOneHandle==&(zeroHandle.getFullHandle()));
					targetArrowBox.moveArrowFromEndFake(it, --secondFullArrowBoxNum, arrowBox, anim4);
				}
			}
			
			float t0 = moveData.m_arrow.second+0.5;
			float t1 = t0 + oneHandle.getPrePermutation().lenght()/ArrowBox::Renderer::arrowSeparation();
			float t2 = t1 + zeroHandle.getPathLenght(beginI, endI, beginJ, endJ)/ArrowBox::Renderer::arrowSeparation();
			float t3 = t2 + targetPerLenght/ArrowBox::Renderer::arrowSeparation();
			cmd->addCommand(anim0, 1e-5, t0); // Doit etre apres la modification de la taille des anses
			cmd->addCommand(anim1, t0, t1);
			cmd->addCommand(anim2, t1, t2);
			cmd->addCommand(layerChangeCmd, (t1+t2)/2, (t1+t2)/2);
			cmd->addCommand(anim3, t2, t3);
			cmd->addCommand(anim4, t3, t3+0.5);
		}
	} else {
		for (size_t i=0; i<m_moveData.size(); ++i) {
			MoveData& moveData = m_moveData[i];
			ArrowBox::ArrowRendererInList it = moveData.m_arrow.first.get().getArrowRendererInList();
			ArrowBox::Renderer& targetArrowBox = moveData.m_targetArrowBox.getRenderer();
			assert(&targetArrowBox!=&arrowBox);
			
			AnimateMoveArrowInArrowBoxCommand* anim0 = AnimateMoveArrowInArrowBoxCommand::create(arrowBox, false);
			arrowBox.moveArrowToEndFake(it, moveData.m_arrow.second, arrowBox, anim0);
			AnimateMoveArrowPermutationBox* anim1 = AnimateMoveArrowPermutationBox::create(oneHandle.getPostPermutation(), *it,
					it->begin(), it->end(), 0.0, 1.0);
			
			it->setBeginEnd(moveData.m_targetI, moveData.m_targetJ);
			
			int arrowBoxEdge = zeroHandle.getEdgeFromArrowBox(arrowBox);
			int targetArrowBoxEdge = zeroHandle.getEdgeFromArrowBox(targetArrowBox);
			unsigned int beginI = zeroHandle.getIndexFromTrack(moveData.m_beginI, arrowBoxEdge);
			unsigned int beginJ = zeroHandle.getIndexFromTrack(moveData.m_beginJ, arrowBoxEdge);
			unsigned int endI   = zeroHandle.getIndexFromTrack(moveData.m_endI,   targetArrowBoxEdge);
			unsigned int endJ   = zeroHandle.getIndexFromTrack(moveData.m_endJ,   targetArrowBoxEdge);
			int layer = (targetArrowBoxEdge==0 || targetArrowBoxEdge==2) ? 2 : 0;
			Command<RendererGL&, float>* layerChangeCmd = ArrowSetLayerCommand::create(*it, layer);
			Command<RendererGL&, float>* anim2 = MoveArrowThroughtZeroHandleCommand::create(*it, zeroHandle, beginI, endI, beginJ, endJ, 0.0, 1.0);
			
			OneHandleRenderer& targetOneHandle = targetArrowBox.getOneHandle();
			const bool targetIsFirstArrowBox = (&targetArrowBox == &(targetOneHandle.getFirstArrowBox()));
			assert((&targetArrowBox == &(targetOneHandle.getSecondArrowBox())) || targetIsFirstArrowBox);
			
			float targetPerLenght;
			AnimateMoveArrowPermutationBox* anim3;
			AnimateMoveArrowInArrowBoxCommand* anim4;
			if (targetIsFirstArrowBox) {
				targetPerLenght = targetOneHandle.getPrePermutation().lenght();
				anim3 = AnimateMoveArrowPermutationBox::create(targetOneHandle.getPrePermutation(), *it, moveData.m_endI, moveData.m_endJ, 0.0, 1.0);
				anim4 = AnimateMoveArrowInArrowBoxCommand::create(targetArrowBox, false);
				
				if (&targetOneHandle==&(zeroHandle.getVoidHandle())) {
					targetArrowBox.moveArrowFromBeginFake(it, --firstVoidArrowBoxNum, arrowBox, anim4);
				} else {
					assert(&targetOneHandle==&(zeroHandle.getFullHandle()));
					targetArrowBox.moveArrowFromBeginFake(it, --firstFullArrowBoxNum, arrowBox, anim4);
				}
			} else {
				targetPerLenght = targetOneHandle.getPostPermutation().lenght();
				anim3 = AnimateMoveArrowPermutationBox::create(targetOneHandle.getPostPermutation(), *it, moveData.m_targetI, moveData.m_targetJ, 1.0, 0.0);
				anim4 = AnimateMoveArrowInArrowBoxCommand::create(targetArrowBox, false);
				
				if (&targetOneHandle==&(zeroHandle.getVoidHandle())) {
					targetArrowBox.moveArrowFromEndFake(it, --secondVoidArrowBoxNum, arrowBox, anim4);
				} else {
					assert(&targetOneHandle==&(zeroHandle.getFullHandle()));
					targetArrowBox.moveArrowFromEndFake(it, --secondFullArrowBoxNum, arrowBox, anim4);
				}
			}
			
			float t0 = size-moveData.m_arrow.second-0.5;
			float t1 = t0 + oneHandle.getPrePermutation().lenght()/ArrowBox::Renderer::arrowSeparation();
			float t2 = t1 + zeroHandle.getPathLenght(beginI, endI, beginJ, endJ)/ArrowBox::Renderer::arrowSeparation();
			float t3 = t2 + targetPerLenght/ArrowBox::Renderer::arrowSeparation();
			cmd->addCommand(anim0, 1e-5, t0); // Doit etre apres la modification de la taille des anses
			cmd->addCommand(anim1, t0, t1);
			cmd->addCommand(anim2, t1, t2);
			cmd->addCommand(layerChangeCmd, (t1+t2)/2, (t1+t2)/2);
			cmd->addCommand(anim3, t2, t3);
			cmd->addCommand(anim4, t3, t3+0.5);
		}
	}
	
	subCommandQueue.push(cmd);
	subCommandQueue.push(RefreshArrowBoxCommand::create(zeroHandle.getFullHandle().getFirstArrowBox(),  true));
	subCommandQueue.push(RefreshArrowBoxCommand::create(zeroHandle.getVoidHandle().getSecondArrowBox(), true));
	subCommandQueue.push(RefreshArrowBoxCommand::create(zeroHandle.getFullHandle().getSecondArrowBox(), true));
	subCommandQueue.push(RefreshArrowBoxCommand::create(zeroHandle.getFullHandle().getFirstArrowBox(),  true));
}









void initDisplayCmd() {
	pthread_mutex_init(&displayMutex, NULL);
}


void destroyDisplayCmd() {
	pthread_mutex_destroy(&displayMutex);
	while (!displayQueue.empty()) {
		displayQueue.front()->clear();
		displayQueue.pop();
	}
	
	while (!subCommandQueue.empty()) {
		subCommandQueue.front()->clear();
		subCommandQueue.pop();
	}
	
	delete zeroHandleCurrent;
}

void processDisplayCommand(RendererGL& rendererGL, float dt) {
	Command<RendererGL&, float, float&>* subTask;
	float pastTime;
	while (!subCommandQueue.empty()) {
		subTask = subCommandQueue.front();
		subTask->run(rendererGL, dt, pastTime);
		if (pastTime<0.0) return;
		subTask->clear();
		subCommandQueue.pop();
		dt-=pastTime;
	}
	
	Command<RendererGL&>* cmd = nullptr;
	
	pthread_mutex_lock(&displayMutex);
	if (!displayQueue.empty()) {
		cmd = displayQueue.front();
		displayQueue.pop();
	}
	pthread_mutex_unlock(&displayMutex);
	
	if (cmd!=nullptr) {
		cmd->run(rendererGL);
		cmd->clear();
	}
}

void postDrawZeroHandle(Pairing handlePairing, ZeroHandle& zeroHandle,
		std::list<std::pair<unsigned int, unsigned int>>&& voidArrowsData, std::list<ArrowBox::ArrowInArrowBox*>&& voidArrows,
		std::list<std::pair<unsigned int, unsigned int>>&& fullArrowsData, std::list<ArrowBox::ArrowInArrowBox*>&& fullArrows) {
	pthread_mutex_lock(&displayMutex);
		displayQueue.push(DrawZeroHandleCommand::create(handlePairing, zeroHandle, std::move(voidArrowsData), std::move(voidArrows),
				std::move(fullArrowsData), std::move(fullArrows)));
	pthread_mutex_unlock(&displayMutex);
}

void postDeleteZeroHandle(ZeroHandle& zeroHandle) {
	pthread_mutex_lock(&displayMutex);
		displayQueue.push(DeleteZeroHandleCommand::create(zeroHandle));
	pthread_mutex_unlock(&displayMutex);
}

void postPermuteArrowBoxCommand(const BiPermutation& permutation, OneHandle& oneHandle, bool isFirstArrowBox) {
	pthread_mutex_lock(&displayMutex);
		displayQueue.push(PermuteArrowBoxCommand::create(permutation, oneHandle, isFirstArrowBox));
	pthread_mutex_unlock(&displayMutex);
}

void postMoveArrowInArrowBox(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow) {
	if (movingArrow.second != targetArrow.second) {
		pthread_mutex_lock(&displayMutex);
			displayQueue.push(MoveArrowInArrowBoxCommand::create(arrowBox, movingArrow, targetArrow));
		pthread_mutex_unlock(&displayMutex);
	}
}

void postGenArrowAfterMoveCrossing(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow,
		ArrowBox::ArrowInArrowBox& newArrow, unsigned int from, unsigned int to, bool genAfter) {
	pthread_mutex_lock(&displayMutex);
		displayQueue.push(GenArrowAfterMoveCrossingCommand::create(arrowBox, movingArrow, targetArrow, newArrow, from, to, genAfter));
	pthread_mutex_unlock(&displayMutex);
}

void postMoveMergeArrowsCommand(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow) {
	pthread_mutex_lock(&displayMutex);
		displayQueue.push(MoveMergeArrowsCommand::create(arrowBox, movingArrow, targetArrow));
	pthread_mutex_unlock(&displayMutex);
}

void postRemoveArrowFromArrowBoxCommand(ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow) {
	pthread_mutex_lock(&displayMutex);
		displayQueue.push(RemoveArrowFromArrowBoxCommand::create(arrowBox, arrow));
	pthread_mutex_unlock(&displayMutex);
}

void postMoveArrowGenCrossingCommand(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow,
		unsigned int crossingI, unsigned int crossingJ) {
	pthread_mutex_lock(&displayMutex);
		displayQueue.push(MoveArrowGenCrossingCommand::create(arrowBox, movingArrow, targetArrow, crossingI, crossingJ));
	pthread_mutex_unlock(&displayMutex);
}

void postMoveArrowToFirstArrowBoxCommand(OneHandle& oneHandle) {
	pthread_mutex_lock(&displayMutex);
		displayQueue.push(MoveArrowToFirstArrowBoxCommand::create(oneHandle));
	pthread_mutex_unlock(&displayMutex);
}

void postMoveArrowToOtherArrowBoxCommand(OneHandle& oneHandle, ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow, unsigned int targetI, unsigned int targetJ) {
	pthread_mutex_lock(&displayMutex);
		displayQueue.push(MoveArrowToOtherArrowBoxCommand::create(oneHandle, arrowBox, arrow, targetI, targetJ));
	pthread_mutex_unlock(&displayMutex);
}

void postMoveArrowToOtherArrowBoxResolveCrossingCommand(OneHandle& oneHandle, ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow, ArrowBox::ArrowInArrowBox& newArrow,
		unsigned int targetI, unsigned int targetJ) {
	pthread_mutex_lock(&displayMutex);
		displayQueue.push(MoveArrowToOtherArrowBoxResolveCrossingCommand::create(oneHandle, arrowBox, arrow, newArrow, targetI, targetJ));
	pthread_mutex_unlock(&displayMutex);
}

void postPassArrowsThroughtZeroHandleCommand(PassArrowsThroughtZeroHandleCommand* cmd) {
	pthread_mutex_lock(&displayMutex);
		displayQueue.push(cmd);
	pthread_mutex_unlock(&displayMutex);
}

void postMoveArrowsAcrossZeroHandle(MoveArrowsAcrossZeroHandle* cmd) {
	pthread_mutex_lock(&displayMutex);
		displayQueue.push(cmd);
	pthread_mutex_unlock(&displayMutex);
}

void postPushArrowCommand(ArrowBox& arrowBox, ArrowBox::ArrowInArrowBox& newArrow, unsigned int from, unsigned int to) {
	pthread_mutex_lock(&displayMutex);
		displayQueue.push(PushArrowCommand::create(arrowBox, newArrow, from, to));
	pthread_mutex_unlock(&displayMutex);
}
