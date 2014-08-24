#ifndef CAMERAPATH_H
#define CAMERAPATH_H

#include "util/Spline.h"

// MATH
#include "util/inc_math.h"

// STL
#include <vector>

class CameraPath {
	class Knot {
	public:Knot() {};
		Knot(glm::vec3 p, glm::quat o) : m_pos(p), m_ori(o) {};
		inline const glm::vec3& pos() const { return m_pos; };
		inline const glm::quat& ori() const { return m_ori; };
	private:
		glm::vec3 m_pos;
		glm::quat m_ori;
	};
		
public:
	CameraPath() : 
		pathWalkDuration(10.0f),
		interpStepDuration(-1),
		activeKnot(-1),
		label("path"),		
		playable(false)	
	{};
	//~CameraPath() { printf("yay!\n"); }

	////////////////////////////////
	void setLabel(const std::string& l) { label = l; }
	const std::string& getLabel()       { return label; }
	////////////////////////////////
	inline const std::vector<Knot>& getKnots() const {
		return knots; 
	}
	inline int length() const { 
		return knots.size(); 
	}
	////////////////////////////////
	inline int getActiveKnot() const { 
		return activeKnot;
	}

	inline void setActiveKnot(int i) { 
		activeKnot = i; 
	}	
	////////////////////////////////
	void addKnot(glm::vec3 p, glm::quat o) {
		knots.push_back(Knot(p, o));
		setActiveKnot(knots.size()-1); 

		updatePath();
	}
	void addKnotAfter(glm::vec3 p, glm::quat o) {
		knots.insert(knots.begin() + (activeKnot + 1), Knot(p, o));
		setActiveKnot(activeKnot+1); 
		
		updatePath();		
	}
	void setKnot(int knotPos, glm::vec3 p, glm::quat o) {
		assert(knotPos >= 0 && knotPos < knots.size());
		knots[knotPos] = Knot(p, o); 

		updatePath();
	}
	void remKnot(int knotPos) { 
		assert(knotPos >= 0 && knotPos < knots.size());
		knots.erase(knots.begin() + knotPos); 
		if (getActiveKnot() == knotPos) setActiveKnot(knotPos-1);

		updatePath();
	}
	////////////////////////////////
	inline void setDuration(float sec) { 
		pathWalkDuration = sec; 
		updatePath();
	}
	inline float getDuration() const {
		return pathWalkDuration; 
	}
	////////////////////////////////	
	inline bool isPlayable() const { 
		return playable; 
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////

	inline void getCamStatusAt(float elapsedTime, glm::vec3 *pos, glm::quat *ori) {
		int step = floor(elapsedTime / interpStepDuration);
		float localInterpStep = (elapsedTime - step * interpStepDuration)/interpStepDuration;
		//printf("elapsed %3.2f duration %3.2f localint %1.3f step %d\n", elapsedTime, interpStepDuration, localInterpStep, step);
		
		glm::vec3 v1 = knots.at(step).pos();
		glm::vec3 v2 = knots.at(step+1).pos();		
		*pos = glm::mix(v1, v2, localInterpStep);
		
		glm::quat quat1 = knots.at(step).ori();
		glm::quat quat2 = knots.at(step+1).ori();		
		*ori = glm::mix(quat1, quat2, localInterpStep);
	}

	inline void getSplineCamStatusAt(float elapsedTime, glm::vec3 *pos, glm::quat *ori) {
		*pos = spline[elapsedTime];
		
		int step = floor(elapsedTime / interpStepDuration);
		float localInterpStep = (elapsedTime - step * interpStepDuration)/interpStepDuration;
		glm::quat quat1 = knots.at(step).ori();
		glm::quat quat2 = knots.at(step+1).ori();		
		*ori = glm::mix(quat1, quat2, localInterpStep);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
private:
	void updatePath() {
		if (knots.size()<2) { playable = false; return; }
		
		playable = true;
		interpStepDuration = pathWalkDuration / (knots.size()-1);
		float interpTime = 0.0f;

		std::vector<float>     times;
		std::vector<glm::vec3> points;		
				
		for (auto k : knots) {			
			points.push_back(k.pos());			
			times.push_back(interpTime);
			interpTime += interpStepDuration;
		}
		spline = Spline3d(times, points);		
	};

	std::string label;

	typedef Spline<float, glm::vec3> Spline3d;
	Spline3d spline;	
	
	std::vector<Knot> knots;
	int               activeKnot;	

	float pathWalkDuration;
	float interpStepDuration;
	bool  playable;
	
	
};

#endif