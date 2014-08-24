#ifndef PROFILER_H
#define	PROFILER_H

#include <vector>
#include <string>

class Profiler {
public:
	Profiler() {}

	Profiler(float duration) : m_duration(duration) {
		reset(m_duration);
	}

	void reset(float duration) { 
		m_duration = duration;
		reset();
		m_frameTimes.reserve(floor(m_duration*MAX_FPS)); 		
	}

	void reset() {
		m_frameCount = 0;
		m_frameTimes.clear();
	}

	inline void update(float frameRenderingTime) { 
		m_frameCount++;
		m_frameTimes.push_back(frameRenderingTime);
	}

	void getResults();
	
private:
	int m_duration;
	int m_frameCount;
	std::vector<float> m_frameTimes;
	static const int MAX_FPS = 300;
	static const std::string logsFolder;
};

#endif