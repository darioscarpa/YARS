#ifndef RANDOMGENERATORSHADERSUB_H
#define RANDOMGENERATORSHADERSUB_H

#include "../../../ShaderHandling.h"

class aRandomGeneratorShaderSub : public ShaderSubroutine {
public:

	enum OptionValues {
		none,
		noiseFunc,
		texture,
		func,
	};
	aRandomGeneratorShaderSub() 
		: ShaderSubroutine("getRandom") {
			addFunction("getRandomFixedValue");
			addFunction("getRandomByNoiseFunc");
			//addFunction("getRandomByTexture");
			//addFunction("getRandomByPseudoRandFunc");		
	}
};

#endif