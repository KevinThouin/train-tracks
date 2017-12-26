#ifndef __DISPLAY_CMD_HPP__
#define __DISPLAY_CMD_HPP__

#include <queue>
#include <functional>

class AnimateMoveArrowInArrowBoxCommand;

#include "draw_gl.hpp"
#include "permutation.hpp"
#include "zero_handle.hpp"
#include "one_handle.hpp"
#include "command.hpp"
#include "arrow.hpp"

typedef std::pair<std::reference_wrapper<ArrowBox::ArrowInArrowBox>, int> ArrowInArrowBoxIndexed;
static inline ArrowInArrowBoxIndexed makeArrowInArrowBoxIndexed(ArrowBox::ArrowInArrowBox& arrow, int index) {
	return std::make_pair(std::reference_wrapper<ArrowBox::ArrowInArrowBox>(arrow), index);
}

class ArrowInArrowBoxIndexedCompare {
	bool operator()(ArrowInArrowBoxIndexed& a, ArrowInArrowBoxIndexed& b) {
		return a.second > b.second;
	}
};

static inline ArrowInArrowBoxIndexed makeArrowInArrowBoxIndexed(ArrowBox::ArrowInArrowBoxIndexedIterator val) {
	return makeArrowInArrowBoxIndexed(**val, val.getPos());
}


class AnimateMoveArrowInArrowBoxCommand : public Command<RendererGL&, float> {
private:
	struct MoveData {
		float m_p0x0; float m_p0y0; float m_p1x0; float m_p1y0; float m_t0x0; float m_t0y0; float m_t1x0; float m_t1y0;
		float m_p0x1; float m_p0y1; float m_p1x1; float m_p1y1; float m_t0x1; float m_t0y1; float m_t1x1; float m_t1y1;
		
		MoveData(float p0x0, float p0y0, float p1x0, float p1y0, float t0x0, float t0y0, float t1x0, float t1y0,
				float p0x1, float p0y1, float p1x1, float p1y1, float t0x1, float t0y1, float t1x1, float t1y1) :
			m_p0x0(p0x0), m_p0y0(p0y0), m_p1x0(p1x0), m_p1y0(p1y0), m_t0x0(t0x0), m_t0y0(t0y0), m_t1x0(t1x0), m_t1y0(t1y0),
			m_p0x1(p0x1), m_p0y1(p0y1), m_p1x1(p1x1), m_p1y1(p1y1), m_t0x1(t0x1), m_t0y1(t0y1), m_t1x1(t1x1), m_t1y1(t1y1) {}
	};
	
	std::vector<std::pair<Arrow::Renderer*, MoveData>> m_arrowsMoveData;
	ArrowBox::Renderer& m_arrowBox;
	bool m_isAbsolute;
	
	AnimateMoveArrowInArrowBoxCommand(ArrowBox::Renderer& arrowBox, bool isAbsolute) : m_arrowBox(arrowBox), m_isAbsolute(isAbsolute) {}
	
public:
	static AnimateMoveArrowInArrowBoxCommand* create(ArrowBox::Renderer& arrowBox, bool isAbsolute) {
		return new AnimateMoveArrowInArrowBoxCommand(arrowBox, isAbsolute);
	}
	
	void addArrow(Arrow::Renderer& arrow, float p0x0, float p0y0, float p1x0, float p1y0, float t0x0, float t0y0, float t1x0, float t1y0,
										  float p0x1, float p0y1, float p1x1, float p1y1, float t0x1, float t0y1, float t1x1, float t1y1);
	
	virtual void run(RendererGL& rendererGL, float t);
	
	virtual std::string name() const {return "AnimateMoveArrowInArrowBoxCommand";}
};

class AnimateMoveTrackLineArrowBox : public Command<RendererGL&, float> {
private:
	struct MoveData {
		float m_p0x0; float m_p0y0; float m_p1x0; float m_p1y0;
		float m_p0x1; float m_p0y1; float m_p1x1; float m_p1y1;
		
		MoveData(float p0x0, float p0y0, float p1x0, float p1y0, float p0x1, float p0y1, float p1x1, float p1y1) :
			m_p0x0(p0x0), m_p0y0(p0y0), m_p1x0(p1x0), m_p1y0(p1y0), m_p0x1(p0x1), m_p0y1(p0y1), m_p1x1(p1x1), m_p1y1(p1y1) {}
	};
	
	std::vector<std::pair<TrackLine*, MoveData>> m_trackMoveData;
	ArrowBox::Renderer& m_arrowBox;
	
	AnimateMoveTrackLineArrowBox(ArrowBox::Renderer& arrowBox) : m_arrowBox(arrowBox) {}
	
public:
	static AnimateMoveTrackLineArrowBox* create(ArrowBox::Renderer& arrowBox) {return new AnimateMoveTrackLineArrowBox(arrowBox);}
	
	void addTrack(TrackLine& track, float p0x0, float p0y0, float p1x0, float p1y0, float p0x1, float p0y1, float p1x1, float p1y1);
	
	virtual void run(RendererGL& rendererGL, float t);
	
	virtual std::string name() const {return "AnimateMoveTrackLineArrowBox";}
};

class AnimateMoveArrowThroughtArrowBox : public Command<RendererGL&, float> {
private:
	const ArrowBox::Renderer& m_arrowBox;
	Arrow::Renderer& m_arrow;
	unsigned int m_begin, m_end;
	float m_startT, m_endT;
	
	AnimateMoveArrowThroughtArrowBox(const ArrowBox::Renderer& arrowBox, Arrow::Renderer& arrow, unsigned int begin, unsigned int end, float startT, float endT) :
			m_arrowBox(arrowBox), m_arrow(arrow), m_begin(begin), m_end(end), m_startT(startT), m_endT(endT) {}
	
public:
	static AnimateMoveArrowThroughtArrowBox* create(const ArrowBox::Renderer& arrowBox, Arrow::Renderer& arrow,
			unsigned int begin, unsigned int end, float startT, float endT) {
		return new AnimateMoveArrowThroughtArrowBox(arrowBox, arrow, begin, end, startT, endT);
	}
	
	virtual void run(RendererGL& rendererGL, float t);
	
	virtual std::string name() const {return "AnimateMoveArrowThroughtArrowBox";}
};

class AnimateMoveTrackPermutationBox : public Command<RendererGL&, float> {
private:
	struct MoveData {
		float m_p0x0; float m_p0y0; float m_p1x0; float m_p1y0;
		float m_p0x1; float m_p0y1; float m_p1x1; float m_p1y1;
		Direction m_d0; Direction m_d1;
		
		MoveData(float p0x0, float p0y0, float p1x0, float p1y0, float p0x1, float p0y1, float p1x1, float p1y1, Direction d0, Direction d1) :
			m_p0x0(p0x0), m_p0y0(p0y0), m_p1x0(p1x0), m_p1y0(p1y0), m_p0x1(p0x1), m_p0y1(p0y1), m_p1x1(p1x1), m_p1y1(p1y1), m_d0(d0), m_d1(d1) {}
	};
	
	std::vector<std::pair<TrackBezier*, MoveData>> m_trackMoveData;
	PermutationBox::Renderer& m_permutationBox;
	
	AnimateMoveTrackPermutationBox(PermutationBox::Renderer& permutationBox) : m_permutationBox(permutationBox) {}
	
public:
	static AnimateMoveTrackPermutationBox* create(PermutationBox::Renderer& permutationBox) {return new AnimateMoveTrackPermutationBox(permutationBox);}
	
	void addTrack(TrackBezier& track, float p0x0, float p0y0, float p1x0, float p1y0, float p0x1, float p0y1, float p1x1, float p1y1, Direction d0, Direction d1);
	
	virtual void run(RendererGL& rendererGL, float t);
	
	virtual std::string name() const {return "AnimateMoveTrackPermutationBox";}
};

class AnimateMoveArrowPermutationBox : public Command<RendererGL&, float> {
private:
	const PermutationBox::Renderer& m_permutationBox;
	Arrow::Renderer& m_arrow;
	unsigned int m_rightI, m_rightJ;
	float m_startPos, m_endPos;
	
	AnimateMoveArrowPermutationBox(const PermutationBox::Renderer& permutationBox, Arrow::Renderer& arrow, unsigned int rightI, unsigned int rightJ,
		float startPos, float endPos) :
			m_permutationBox(permutationBox), m_arrow(arrow), m_rightI(rightI), m_rightJ(rightJ), m_startPos(startPos), m_endPos(endPos) {}
	
public:
	static AnimateMoveArrowPermutationBox* create(const PermutationBox::Renderer& permutationBox, Arrow::Renderer& arrow,
			unsigned int rightI, unsigned int rightJ, float startPos, float endPos) {
		return new AnimateMoveArrowPermutationBox(permutationBox, arrow, rightI, rightJ, startPos, endPos);
	}
	
	virtual void run(RendererGL& rendererGL, float t);
	
	virtual std::string name() const {return "AnimateMoveArrowPermutationBox";}
};

class AnimateInterpolateShowableBezierCurve : public Command<RendererGL&, float> {
private:
	struct MoveData {
		Bezier m_curve0;
		Bezier m_curve1;
		
		MoveData(Bezier curve0, Bezier curve1) : m_curve0(curve0), m_curve1(curve1) {}
	};
	
	std::vector<std::pair<ShowableBezier*, MoveData>> m_curves;
	
public:
	static AnimateInterpolateShowableBezierCurve* create() {return new AnimateInterpolateShowableBezierCurve();}
	
	void addCurve(ShowableBezier& curve, Bezier curve0, Bezier curve1);
	
	virtual void run(RendererGL& rendererGL, float t);
	
	virtual std::string name() const {return "AnimateInterpolateShowableBezierCurve";}
};



class UpdateArrowBoxLenghtCommand : public Command<RendererGL&, float> {
	ArrowBox::Renderer& m_arrowBox;
	float m_oldLenght = 0.0;
	float m_newLenght = 0.0;
	bool  m_updateArrows;
	
	UpdateArrowBoxLenghtCommand(ArrowBox::Renderer& arrowBox, bool updateArrows) : m_arrowBox(arrowBox), m_updateArrows(updateArrows) {}
	
public:
	static UpdateArrowBoxLenghtCommand* create(ArrowBox::Renderer& arrowBox, bool updateArrows=true) {return new UpdateArrowBoxLenghtCommand(arrowBox, updateArrows);}
	
	void setVariation(float oldLenght, float newLenght) {
		m_oldLenght = oldLenght;
		m_newLenght = newLenght;
	}
	
	virtual void run(RendererGL& rendererGL, float t) {
		m_arrowBox.setLenght(t*m_newLenght + (1.0-t)*m_oldLenght, m_updateArrows);
	}
	
	virtual std::string name() const {return "UpdateArrowBoxLenghtCommand";}
};

class GenCrossingCommand : public Command<RendererGL&, float> {
	ArrowBox::Renderer& m_arrowBox;
	float m_xBegin, m_xEnd;
	float m_yI, m_yJ;
	int m_layer;
	
	GenCrossingCommand(ArrowBox::Renderer& arrowBox, float xBegin, float xEnd, float yI, float yJ) :
			m_arrowBox(arrowBox), m_xBegin(arrowBox.getBasex()+xBegin), m_xEnd(arrowBox.getBasex()+xEnd), m_yI(arrowBox.getBasey()-yI), m_yJ(arrowBox.getBasey()-yJ) {}
	
public:
	static GenCrossingCommand* create(ArrowBox::Renderer& arrowBox, float xBegin, float xEnd, float yI, float yJ) {
		return new GenCrossingCommand(arrowBox, xBegin, xEnd, yI, yJ);
	}
	
	virtual void run(RendererGL& rendererGL, float t);
	
	virtual std::string name() const {return "GenCrossingCommand";}
};

class MoveCrossingCommand : public Command<RendererGL&, float> {
protected:
	ArrowBox::Renderer& m_arrowBox;
private:
	float m_crossingStart, m_crossingEnd, m_crossingNewStart, m_crossingNewEnd;
	float m_yI, m_yJ;
	
protected:
	MoveCrossingCommand(ArrowBox::Renderer& arrowBox, float crossingStart, float crossingEnd, float crossingNewStart, float crossingNewEnd, float yI, float yJ) :
		m_arrowBox(arrowBox), m_crossingStart(arrowBox.getBasex()+crossingStart), m_crossingEnd(arrowBox.getBasex()+crossingEnd),
			m_crossingNewStart(arrowBox.getBasex()+crossingNewStart), m_crossingNewEnd(arrowBox.getBasex()+crossingNewEnd),
			m_yI(arrowBox.getBasey()-yI), m_yJ(arrowBox.getBasey()-yJ) {}
	
public:
	virtual void run(RendererGL& rendererGL, float t);
};

class MoveCrossingCommandSlideArrow : public MoveCrossingCommand {
	Arrow::Renderer* m_arrow;
	float m_arrowFixPoint;
	float m_arrowT0y;
	bool m_moveBegin;
	bool m_followTrackA;
	
	MoveCrossingCommandSlideArrow(ArrowBox::Renderer& arrowBox, float crossingStart, float crossingEnd, float crossingNewStart, float crossingNewEnd,
		float yI, float yJ, Arrow::Renderer* arrow, float arrowFixPoint, float arrowT0y, bool moveBegin, bool followTrackA) :
			MoveCrossingCommand(arrowBox, crossingStart, crossingEnd, crossingNewStart, crossingNewEnd, yI, yJ), m_arrow(arrow),
			m_arrowFixPoint(arrowBox.getBasey()-arrowFixPoint), m_arrowT0y(arrowT0y), m_moveBegin(moveBegin), m_followTrackA(followTrackA) {}
	
public:
	static MoveCrossingCommandSlideArrow* create(ArrowBox::Renderer& renderer, float crossingStart, float crossingEnd, float crossingNewStart,  float crossingNewEnd,
		float yI, float yJ, Arrow::Renderer* arrow, float arrowFixPoint, float arrowT0y, bool moveBegin, bool followTrackA) {
			return new MoveCrossingCommandSlideArrow(renderer, crossingStart, crossingEnd, crossingNewStart, crossingNewEnd, yI, yJ, arrow,
					arrowFixPoint, arrowT0y, moveBegin, followTrackA);
	}
	
	virtual void run(RendererGL& rendererGL, float t);
	
	virtual std::string name() const {return "MoveCrossingCommandSlideArrow";}
};

class MoveCrossingCommandMoveArrow : public MoveCrossingCommand {
	Arrow::Renderer* m_arrow;
	float m_arrowXa, m_arrowXb, m_arrowY0, m_arrowY1, m_arrowT0y;
	
	MoveCrossingCommandMoveArrow(ArrowBox::Renderer& arrowBox, float crossingStart, float crossingEnd, float crossingNewStart, float crossingNewEnd,
		float yI, float yJ, Arrow::Renderer* arrow, float arrowXa, float arrowXb, float arrowY0, float arrowY1, float arrowT0y) :
			MoveCrossingCommand(arrowBox, crossingStart, crossingEnd, crossingNewStart, crossingNewEnd, yI, yJ), m_arrow(arrow),
			m_arrowXa(arrowBox.getBasex()+arrowXa), m_arrowXb(arrowBox.getBasex()+arrowXb), m_arrowY0(arrowBox.getBasey()-arrowY0),
			m_arrowY1(arrowBox.getBasey()-arrowY1), m_arrowT0y(arrowT0y) {}
	
public:
	static MoveCrossingCommandMoveArrow* create(ArrowBox::Renderer& arrowBox, float crossingStart, float crossingEnd, float crossingNewStart, float crossingNewEnd,
			float yI, float yJ, Arrow::Renderer* arrow, float arrowXa, float arrowXb, float arrowY0, float arrowY1, float arrowT0y) {
		return new MoveCrossingCommandMoveArrow(arrowBox, crossingStart, crossingEnd, crossingNewStart, crossingNewEnd, yI, yJ, arrow,
				arrowXa, arrowXb, arrowY0, arrowY1, arrowT0y);
	}
	
	virtual void run(RendererGL& rendererGL, float t);
	
	virtual std::string name() const {return "MoveCrossingCommandMoveArrow";}
};

class MoveCrossingCommandInvertArrow : public MoveCrossingCommand {
	Arrow::Renderer* m_arrow;
	bool m_isUpArrow;
	
	MoveCrossingCommandInvertArrow(ArrowBox::Renderer& arrowBox, float crossingStart, float crossingEnd, float crossingNewStart, float crossingNewEnd,
		float yI, float yJ, Arrow::Renderer* arrow, bool isUpArrow) :
			MoveCrossingCommand(arrowBox, crossingStart, crossingEnd, crossingNewStart, crossingNewEnd, yI, yJ), m_arrow(arrow), m_isUpArrow(isUpArrow) {}
	
public:
	static MoveCrossingCommandInvertArrow* create(ArrowBox::Renderer& arrowBox, float crossingStart, float crossingEnd, float crossingNewStart, float crossingNewEnd,
			float yI, float yJ, Arrow::Renderer* arrow, bool isUpArrow) {
		return new MoveCrossingCommandInvertArrow(arrowBox, crossingStart, crossingEnd, crossingNewStart, crossingNewEnd, yI, yJ, arrow, isUpArrow);
	}
	
	virtual void run(RendererGL& rendererGL, float t);
	
	virtual std::string name() const {return "MoveCrossingCommandInvertArrow";}
};

class FadeCrossingCommand : public Command<RendererGL&, float> {
	ArrowBox::Renderer& m_arrowBox;
	float m_crossingStart, m_crossingEnd, m_crossingFadeX;
	float m_yI, m_yJ;
	
	FadeCrossingCommand(ArrowBox::Renderer& arrowBox, float crossingStart, float crossingEnd, float crossingFadeX, float yI, float yJ) :
		m_arrowBox(arrowBox), m_crossingStart(crossingStart), m_crossingEnd(crossingEnd), m_crossingFadeX(crossingFadeX),
				m_yI(arrowBox.getBasey()-yI), m_yJ(arrowBox.getBasey()-yJ) {}
	
public:
	static FadeCrossingCommand* create(ArrowBox::Renderer& arrowBox, float crossingStart, float crossingEnd, float crossingFadeX, float yI, float yJ) {
		return new FadeCrossingCommand(arrowBox, crossingStart, crossingEnd, crossingFadeX, yI, yJ);
	}
	
	virtual void run(RendererGL& rendererGL, float t);
	
	virtual std::string name() const {return "FadeCrossingCommand";}
};

class MoveArrowThroughtZeroHandleCommand : public Command<RendererGL&, float> {
	Arrow::Renderer& m_arrow;
	ZeroHandleRenderer& m_zeroHandle;
	unsigned int m_startI, m_endI;
	unsigned int m_startJ, m_endJ;
	float m_startTime, m_endTime;
	
	MoveArrowThroughtZeroHandleCommand(Arrow::Renderer& arrow, ZeroHandleRenderer& zeroHandle, unsigned int startI, unsigned int endI,
			unsigned int startJ, unsigned int endJ, float startTime, float endTime) : 
		m_arrow(arrow), m_zeroHandle(zeroHandle), m_startI(startI), m_endI(endI), m_startJ(startJ), m_endJ(endJ), m_startTime(startTime), m_endTime(endTime) {}
	
public:
	static Command<RendererGL&, float>* create(Arrow::Renderer& arrow, ZeroHandleRenderer& zeroHandle, unsigned int startI, unsigned int endI,
			unsigned int startJ, unsigned int endJ, float startTime, float endTime) {
		return new MoveArrowThroughtZeroHandleCommand(arrow, zeroHandle, startI, endI, startJ, endJ, startTime, endTime);
	}
	
	void run(RendererGL& rendererGL, float t);
	
	virtual std::string name() const {return "MoveArrowThroughtZeroHandleCommand";}
};

class ArrowSetLayerCommand : public Command<RendererGL&, float> {
	Arrow::Renderer& m_arrow;
	int m_newLayer;
	
	ArrowSetLayerCommand(Arrow::Renderer& arrow, int newLayer) : m_arrow(arrow), m_newLayer(newLayer) {}
	
public:
	static Command<RendererGL&, float>* create(Arrow::Renderer& arrow, int newLayer) {return new ArrowSetLayerCommand(arrow, newLayer);}
	
	void run(RendererGL& rendererGL, float t);
	
	virtual std::string name() const {return "ArrowSetLayerCommand";}
};

class AsynchronousSubTaskCommand : public Command<RendererGL&, float, float&> {
	struct AsynchronousSubTaskData {
		Command<RendererGL&, float>* m_cmd;
		float m_taskBegin;
		float m_taskEnd;
		
		AsynchronousSubTaskData(Command<RendererGL&, float>* cmd, float taskBegin, float taskEnd) : m_cmd(cmd), m_taskBegin(taskBegin), m_taskEnd(taskEnd) {}
		AsynchronousSubTaskData(AsynchronousSubTaskData&& other) : m_cmd(other.m_cmd), m_taskBegin(other.m_taskBegin), m_taskEnd(other.m_taskEnd) {other.m_cmd = nullptr;}
		
		bool operator< (const AsynchronousSubTaskData& other) const {return m_taskBegin < other.m_taskBegin;}
		bool operator> (const AsynchronousSubTaskData& other) const {return m_taskBegin > other.m_taskBegin;}
		bool operator<=(const AsynchronousSubTaskData& other) const {return !(*this > other);}
		bool operator>=(const AsynchronousSubTaskData& other) const {return !(*this < other);}
		
		AsynchronousSubTaskData& operator=(AsynchronousSubTaskData&& other) {
			m_cmd = other.m_cmd; m_taskBegin = other.m_taskBegin; m_taskEnd = other.m_taskEnd;
			other.m_cmd = nullptr;
			return *this;
		}
	};
	
	std::priority_queue<AsynchronousSubTaskData, std::vector<AsynchronousSubTaskData>, std::greater<AsynchronousSubTaskData>> m_cmd;
	std::list<AsynchronousSubTaskData> m_executingCmd;
	float m_duration = 0.0;
	float m_t;
	
	AsynchronousSubTaskCommand() = default;
	
public:
	~AsynchronousSubTaskCommand();
	
	static AsynchronousSubTaskCommand* create() {return new AsynchronousSubTaskCommand();}
	
	virtual void run(RendererGL& rendererGL, float dt, float& pastTime);
	
	virtual std::string name() const {return "AsynchronousSubTaskCommand";}
	
	void addCommand(Command<RendererGL&, float>* cmd, float cmdBegin, float cmdEnd);
};

class PassArrowsThroughtZeroHandleCommand : public Command<RendererGL&> {
	struct MoveData {
		ArrowInArrowBoxIndexed m_arrow;
		ArrowBox& m_targetArrowBox;
		unsigned int m_beginI, m_beginJ;
		unsigned int m_endI, m_endJ;
		unsigned int m_targetI, m_targetJ;
		
		MoveData(ArrowInArrowBoxIndexed arrow, ArrowBox& targetArrowBox, unsigned int beginI, unsigned int beginJ, unsigned int endI, unsigned int endJ,
				 unsigned int targetI, unsigned int targetJ) :
			m_arrow(arrow), m_targetArrowBox(targetArrowBox), m_beginI(beginI), m_beginJ(beginJ), m_endI(endI), m_endJ(endJ), m_targetI(targetI), m_targetJ(targetJ) {}
	};
	
	ArrowBox& m_arrowBox;
	std::vector<MoveData> m_moveData;
	
	PassArrowsThroughtZeroHandleCommand(ArrowBox& arrowBox) : m_arrowBox(arrowBox) {}
public:
	static PassArrowsThroughtZeroHandleCommand* create(ArrowBox& arrowBox) {return new PassArrowsThroughtZeroHandleCommand(arrowBox);}
	
	virtual void run(RendererGL& rendererGL);
	
	void addArrow(ArrowInArrowBoxIndexed arrow, ArrowBox& targetArrowBox, unsigned int beginI, unsigned int beginJ, unsigned int endI, unsigned int endJ,
				 unsigned int targetI, unsigned int targetJ) {
		m_moveData.emplace_back(arrow, targetArrowBox, beginI, beginJ, endI, endJ, targetI, targetJ);
	}
	
	virtual std::string name() const {return "PassArrowsThroughtZeroHandle";}
};

class MoveArrowsAcrossZeroHandle : public Command<RendererGL&> {
public:
	struct IterationData {
		unsigned int m_rightI, m_rightJ;
		bool m_isVoidArrow, m_moveRight;
		OneHandle& m_oneHandle;
		
		IterationData() = delete;
		IterationData(unsigned int rightI, unsigned int rightJ, bool isVoidArrow, bool moveRight, OneHandle& oneHandle) : m_rightI(rightI), m_rightJ(rightJ),
				m_isVoidArrow(isVoidArrow), m_moveRight(moveRight), m_oneHandle(oneHandle) {}
		
		unsigned int getLeftI() const {
			OneHandleRenderer& oneHandle = m_oneHandle.getRenderer();
			return oneHandle.getPermutation().pre(oneHandle.getPermutation().pre(oneHandle.getPostPermutation().pre(m_rightI)));
		}
		
		unsigned int getLeftJ() const {
			OneHandleRenderer& oneHandle = m_oneHandle.getRenderer();
			return oneHandle.getPermutation().pre(oneHandle.getPermutation().pre(oneHandle.getPostPermutation().pre(m_rightJ)));
		}
	};
	
private:
	struct MoveData {
		ArrowBox* m_arrowBox;
		ArrowInArrowBoxIndexed m_arrow;
		mutable std::vector<IterationData> m_iterationData;
		size_t m_iterationStartIndex;
		bool m_shouldRemoveArrow;
		
		MoveData(ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow, std::vector<IterationData>&& iterationData,
				 size_t iterationStartIndex, bool shouldRemoveArrow) : 
			m_arrowBox(&arrowBox), m_arrow(arrow), m_iterationData(std::move(iterationData)),
			m_iterationStartIndex(iterationStartIndex), m_shouldRemoveArrow(shouldRemoveArrow) {assert(!m_iterationData.empty());}
			
		MoveData(const MoveData&& other) : m_arrowBox(other.m_arrowBox), m_arrow(other.m_arrow), m_iterationData(std::move(other.m_iterationData)),
			m_iterationStartIndex(other.m_iterationStartIndex), m_shouldRemoveArrow(other.m_shouldRemoveArrow) {}
		
		MoveData& operator=(const MoveData&& other) {
			m_arrowBox = other.m_arrowBox;
			m_arrow = other.m_arrow;
			m_iterationData = std::move(other.m_iterationData);
			m_iterationStartIndex = other.m_iterationStartIndex;
			return *this;
		}
		
		bool operator<(const MoveData& other) const {return m_iterationStartIndex==other.m_iterationStartIndex ? m_arrow.second<other.m_arrow.second : 
			m_iterationStartIndex<other.m_iterationStartIndex;}
		bool operator>(const MoveData& other) const {return other.operator<(*this);}
		bool operator<=(const MoveData& other) const {return !this->operator>(other);}
		bool operator>=(const MoveData& other) const {return !this->operator<(other);}
	};
	
	ZeroHandle& m_zeroHandle;
	std::priority_queue<MoveData, std::vector<MoveData>, std::greater<MoveData>> m_arrows;
	
	MoveArrowsAcrossZeroHandle(ZeroHandle& zeroHandle) : m_zeroHandle(zeroHandle) {}
	void moveArrowThroughtOneHandle(AsynchronousSubTaskCommand* cmd, float start, float end, unsigned int rightI, unsigned int rightJ,
			Arrow::Renderer& arrow, OneHandleRenderer& oneHandle, float& time);
	void moveArrowThroughtZeroHandle(AsynchronousSubTaskCommand* cmd, float start, float end, unsigned int startI, unsigned int endI,
			unsigned int startJ, unsigned int endJ, Arrow::Renderer& arrow, float& time);
	float getPosInArrowBox(ArrowInArrowBoxIndexed arrow, ArrowBox::Renderer& arrowBox);
	void getStartEndIJ(unsigned int& startI, unsigned int& endI, unsigned int& startJ, unsigned int& endJ, IterationData& current, IterationData& prev);
	
public:
	static MoveArrowsAcrossZeroHandle* create(ZeroHandle& zeroHandle) {return new MoveArrowsAcrossZeroHandle(zeroHandle);}
	void moveArrow(AsynchronousSubTaskCommand* cmd, MoveData& moveData, float time, unsigned int index, size_t iterationIndex, float t);
	void addArrow(ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow, std::vector<IterationData>&& iterationData,
				 size_t iterationStartIndex, bool shouldRemoveArrow);
	
	virtual void run(RendererGL& rendererGL);
	
	virtual std::string name() const {return "MoveArrowsAcrossZeroHandle";}
};

void initDisplayCmd();
void destroyDisplayCmd();
void processCommand(RendererGL& rendererGL, float dt);
void postDrawZeroHandle(Pairing handlePairing, ZeroHandle& zeroHandle,
		std::list<std::pair<unsigned int, unsigned int>>&& voidArrowsData, std::list<ArrowBox::ArrowInArrowBox*>&& voidArrows,
		std::list<std::pair<unsigned int, unsigned int>>&& fullArrowsData, std::list<ArrowBox::ArrowInArrowBox*>&& fullArrows);
void postDeleteZeroHandle(ZeroHandle& zeroHandle);
void postPermuteArrowBoxCommand(const BiPermutation& permutation, OneHandle& oneHandle, bool isFirstArrowBox);
void postMoveArrowInArrowBox(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow);
void postGenArrowAfterMoveCrossing(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow,
		ArrowBox::ArrowInArrowBox& newArrow, unsigned int from, unsigned int to, bool genAfter);
void postMoveMergeArrowsCommand(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArw);
void postRemoveArrowFromArrowBoxCommand(ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow);
void postMoveArrowGenCrossingCommand(ArrowBox& arrowBox, ArrowInArrowBoxIndexed movingArrow, ArrowInArrowBoxIndexed targetArrow,
		unsigned int crossingI, unsigned int crossingJ);
void postMoveArrowToFirstArrowBoxCommand(OneHandle& oneHandle);
void postMoveArrowToOtherArrowBoxCommand(OneHandle& oneHandle, ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow, unsigned int targetI, unsigned int targetJ);
void postMoveArrowToOtherArrowBoxResolveCrossingCommand(OneHandle& oneHandle, ArrowBox& arrowBox, ArrowInArrowBoxIndexed arrow, ArrowBox::ArrowInArrowBox& newArrow,
		unsigned int targetI, unsigned int targetJ);
void postPassArrowsThroughtZeroHandleCommand(PassArrowsThroughtZeroHandleCommand* cmd);
void postMoveArrowsAcrossZeroHandle(MoveArrowsAcrossZeroHandle* cmd);
void postPushArrowCommand(ArrowBox& arrowBox, ArrowBox::ArrowInArrowBox& newArrow, unsigned int from, unsigned int to);

#endif /* __DISPLAY_CMD_HPP__ */
