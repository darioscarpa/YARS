#ifndef AREACALCULATORSHADERSUB_H
#define AREACALCULATORSHADERSUB_H

#include "../../../ShaderHandling.h"

class AreaCalculatorShaderSub : public ShaderSubroutine {
public:

	enum OptionValues {
		unitValue,
		triangleArea,
		inscribedCircleArea,
		circumscribedCircleArea,
		inscribedCircleRadius,
		circumscribedCircleRadius,
		inscribedCircleRadiusSquared,
		circumscribedCircleRadiusSquared,		
		SIZE
	};
	AreaCalculatorShaderSub() 
		: ShaderSubroutine("getArea") {
			addFunction("getAreaUnitValue");
			addFunction("getAreaTriangleArea");
			addFunction("getAreaInscribedCircleArea");
			addFunction("getAreaCircumscribedCircleArea");
			addFunction("getAreaInscribedCircleRadius");
			addFunction("getAreaCircumscribedCircleRadius");
			addFunction("getAreaInscribedCircleRadiusSquared");
			addFunction("getAreaCircumscribedCircleRadiusSquared");			
	}
};

#endif