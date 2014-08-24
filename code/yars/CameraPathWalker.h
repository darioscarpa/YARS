#ifndef CAMERAPATHWALKER_H
#define CAMERAPATHWALKER_H

#include "util/Spline.h"

// MATH
#include "util/inc_math.h"

// STL
#include <vector>

class CameraPathWalker {
	
		
public:
	CameraPathWalker() :
		playing(false),
		looping(false),
		paused(false),
		elapsedTime(0.0f),	
		totalWalkTime(0.0f),
		walkComplete(false)
		{};
	//~CameraPath() { printf("yay!\n"); }

	void play() {
		if (isPaused()) {
			printf("resume play\n");
			setPaused(false);
		} else {
			printf("play\n");
			playing = true;
			elapsedTime = 0;
			walkComplete = false;
		}		
	}

	inline bool isPlaying() const { 
		return playing;
	}

	inline void setPaused(bool tf)  { 
		paused = tf;
	}
	inline bool isPaused() const {
		return paused;
	}
	
	void stop() {
		printf("stop\n");
		setPaused(false);
		playing = false;
	}

	inline void setLooping(bool enabled) {
		printf("setlooping %d\n", enabled);
		looping = enabled;
	}
	inline bool isLooping() const {
		return looping;
	}

	void update(float dt) {
		assert(playing && !paused);		
		elapsedTime += dt;
		//printf("elapsed: %f\n", elapsedTime);
		if (elapsedTime >= totalWalkTime) {
			if (looping) {
				elapsedTime = 0; 
			} else {
				walkComplete = true;
				stop();
			}
		}
	}

	inline float getElapsedTime() const {
		return elapsedTime;
	}

	inline bool isPlayingFinished() const {
		return walkComplete;
	}

	inline void setPlaybackTime(float time) {
		totalWalkTime = time;
	}
	inline float getPlaybackTime() const {
		return totalWalkTime;
	}
private:

	bool looping;
	bool paused;
	bool playing;

	float elapsedTime;
	float totalWalkTime;
	bool  walkComplete;	
};

#endif