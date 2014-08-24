#ifndef SAMPLINGPATTERNSHADERSUB_H
#define SAMPLINGPATTERNSHADERSUB_H

#include "../../../ShaderHandling.h"

class SamplingPatternShaderSub : public ShaderSubroutine {
public:
	enum OptionValues {
		dssao,
		radial,
		emisphere,
		flat
	};
	SamplingPatternShaderSub() 
		: ShaderSubroutine("calcSamplesPos") {
			addFunction("getDssaoSamples");
			addFunction("getRadialSamples");
			addFunction("getEmisphereSamples");
			addFunction("getFlatSamples");
	}
};

#endif