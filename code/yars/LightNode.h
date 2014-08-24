#ifndef LIGHTNODE_H
#define LIGHTNODE_H

#include "SceneNode.h"

class LightNode : public SceneNode {

public :

	enum LightType {
		POINT,			 // node orientation not used
		DIRECTIONAL,     // node position not used
		SPOT             // both orientation and position used
	};

	LightNode(std::string _label) :	SceneNode(_label) {
		setType(POINT); 
		
		setAttenuation(1.0f, 0.02f , 0.08f);
		setSpotProperties(15.0f, 18.0f, 0.0f);

		color = glm::vec3(1.0f, 1.0f, 1.0f);
		adjustGeometry();
	}

	virtual ~LightNode() {};

	void setArrayPos(unsigned int i) {
		arrayPos = i;
	}
	unsigned int getArrayPos() const { return arrayPos; }

	void setType(LightType t) {
		type = t;
		adjustGeometry();
	}
	LightType getType() const { return type; }

	void setColor(glm::vec3 c) {
		color = c;
		adjustGeometry();
	}

	void setAttenuation(float constant, float linear, float quadratic) {
		constantAttenuation  = constant;
		linearAttenuation    = linear;
		quadraticAttenuation = quadratic;
		adjustGeometry();
	}

	void setSpotProperties(float innercutoff, float outercutoff, float exponent) {
		spotExponent = exponent;
		spotInnerCutoff = innercutoff;
		spotOuterCutoff = outercutoff;
	}

	inline void  setSpotExponent(float f)     { 
		spotExponent = f; 
	};
	inline void  setSpotInnerCutoff(float f)  {
		spotInnerCutoff = f; 
		if (spotInnerCutoff > spotOuterCutoff) {
			setSpotOuterCutoff(spotInnerCutoff);
		}
	};
	inline void  setSpotOuterCutoff(float f)  { 
		spotOuterCutoff = f;   
		if (spotOuterCutoff < spotInnerCutoff) {
			setSpotInnerCutoff(spotOuterCutoff);
		}
	};

	inline float getSpotExponent()         const { return spotExponent;     }
	inline float getSpotInnerCutoff()      const { return spotInnerCutoff;  }
	inline float getSpotOuterCutoff()      const { return spotOuterCutoff;  }


	inline void setConstantAttenuation(float a)  { constantAttenuation  = a; adjustGeometry();};
	inline void setLinearAttenuation(float a)    { linearAttenuation    = a; adjustGeometry();};
	inline void setQuadraticAttenuation(float a) { quadraticAttenuation = a; adjustGeometry();};

	
	inline float getConstantAttenuation()  const { return constantAttenuation;  }
	inline float getLinearAttenuation()    const { return linearAttenuation;    }
	inline float getQuadraticAttenuation() const { return quadraticAttenuation; }
	inline const glm::vec3& getColor()     const { return color;                }

	inline float getRadius() const  { return radius; }
	inline void  setRadius(float r) { radius = r; }

private:

	unsigned int arrayPos;
	LightType type;
	glm::vec3 color;
	
	float radius;

	float spotExponent;
	float spotInnerCutoff;
	float spotOuterCutoff;

	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;	

	void computeRadius() {
		static const float thr = 0.1f;

		//-----------------------------------------------------
		//float max = color.r;
		//if (color.g > max) max = color.g;
		//if (color.b > max) max = color.b;
		
		//float cmax = max * intensity;
   		//float scale = 8.0f * sqrtf(cmax) + 1.0f;		
		//-----------------------------------------------------
		
		// 1/(c + ld + qd^2) = thr  -> solve for d
		// thr(c + ld + qd^2) = 1 
		// thr(c + ld + qd^2) - 1  = 0
		

		// x12 = -b +- sqrt(b^2 - 4ac)
		float a = quadraticAttenuation * thr;
		float b = linearAttenuation    * thr;
		float c = constantAttenuation  * thr - 1.0f;

		float delta = sqrt(b*b - 4.0f*a*c);
		float x1 = (-b + delta) / (2*a);
		float x2 = (-b - delta) / (2*a);

		radius = x1>x2? x1: x2;
	}

	void adjustGeometry() {
		computeRadius();
		static const float scaleMult = 0.001f;		
		setScaling(glm::vec3(radius*scaleMult));
	}
};

#endif