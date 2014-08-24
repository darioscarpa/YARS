#ifndef D2DILSHADERSUB_H
#define D2DILSHADERSUB_H

#include "../../../ShaderHandling.h"

class D2DilShaderSub : public ShaderSubroutine {
public:

	enum OptionValues {
		bunnell,
		dssao,
		test
	};
	D2DilShaderSub() 
		: ShaderSubroutine("diskToDiskRadiance") {
			addFunction("d2dr_bunnell");
			addFunction("d2dr_dssao");
			addFunction("d2dr_test");
	}
};

#endif
