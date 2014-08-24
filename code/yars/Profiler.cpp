#include "Profiler.h"

#include "Sandbox.h"
#include "Scene.h"
#include "Renderer.h"
#include "RenderWindow.h"

#include <vector>
#include <algorithm>
#include <functional>
#include <numeric>
#include <stdio.h>
#include <fstream>
#include <string>

const std::string Profiler::logsFolder = "_profiling/";

template<class T>
void doCalcs(const std::vector<T>& v, double &mean, double &stdev, T &max, T &min) {
	T sum = std::accumulate(v.begin(), v.end(), 0.0);
	mean = sum / v.size();
	T sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
	stdev = std::sqrt(sq_sum / v.size() - mean * mean);
	max = *std::max_element(v.begin(), v.end());		
	min = *std::min_element(v.begin(), v.end());		
}

void Profiler::getResults() {
		if (m_frameCount > floor(m_duration*MAX_FPS)) {
			//TODO throw
			printf("set higher MAX_FPS in Profiler.h, vector resize might have impacted profiling\n");
		}
		
		std::vector<int> fpsBySec;
		float timesum = 0.0;
		int fpscount = 0;
		for (auto sec : m_frameTimes) {			
			if (timesum + sec > 1.0) {
				fpsBySec.push_back(fpscount);
				timesum = timesum + sec - 1.0;
				fpscount = 1;
			} else {
				timesum += sec;
				fpscount++;
			}
		}

		double mean, stdev;
		float max, min;
		doCalcs(m_frameTimes, mean, stdev, max, min);

		double fps_mean, fps_stdev;
		int fps_max, fps_min;
		doCalcs(fpsBySec, fps_mean, fps_stdev, fps_max, fps_min);

		printf("mean: %2.3lf sec \t %2.3lf\n", mean, fps_mean);
		printf("stdev: %2.3lf sec \t %2.3lf\n", stdev, fps_stdev);
		printf("max: %2.3f sec \t %3d\n", max, fps_max);
		printf("min: %2.3f sec \t %3d\n", min, fps_min);
		
		std::ofstream myfile;
		const std::string scenelabel(Sandbox::getActiveScene()->getLabel());
		const std::string rendererlabel(Sandbox::getActiveRenderer()->getLabel());

		std::string  filename = scenelabel + "_" + rendererlabel + "_" + std::to_string(RenderWindow::getWidth()) + "x" + std::to_string(RenderWindow::getHeight()) + ".csv";
		myfile.open("_profiling/" + filename, std::fstream::out);
		myfile << "scene;renderer;max;min;avg;stddev;maxfps;minfps;avgfps;stddevfps" << std::endl;
		myfile <<  scenelabel << ";" << rendererlabel << ";" 
			<< max << ";" <<min << ";" <<mean << ";" << stdev << ";"
			<< fps_max << ";" << fps_min << ";" << fps_mean << ";" << fps_stdev << ";" << std::endl;
		myfile << std::endl;
		
		// write frame per sec
		for (auto fps : fpsBySec) {
			//std::cout << fps << std::endl;
			myfile << fps << ";" << std::endl;
		}
		myfile << std::endl;

		// write sec per frame
		for (auto sec : m_frameTimes) {	
			myfile << sec << ";" << std::endl;
		}
		myfile << std::endl;

		myfile.close();
}

	
