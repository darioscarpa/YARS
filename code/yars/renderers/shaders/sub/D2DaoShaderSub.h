#ifndef D2DAOSHADERSUB_H
#define D2DAOSHADERSUB_H

#include "../../../ShaderHandling.h"

class D2DaoShaderSub : public ShaderSubroutine {
public:

	enum OptionValues {
		bunnell,
		dssao,
		test		
	};
	D2DaoShaderSub() 
		: ShaderSubroutine("diskToDiskOcclusion") {
			addFunction("d2do_bunnell");
			addFunction("d2do_dssao");
			addFunction("d2do_test");
	}
};

#endif
