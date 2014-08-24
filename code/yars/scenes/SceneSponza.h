#ifndef SCENESPONZA_H
#define SCENESPONZA_H

#include "../Scene.h"

#include "../Asset.h"

#include "../util/Spline.h"

class SceneSponza : public Scene {
public:
		SceneSponza();
		void setup();	
		virtual void update(Input * input, float deltaTime);			
		
		void shootLightBullet();
		void updateLightBullets(float);

private:
	std::vector<SceneNode*>  *bullets;
	Asset *bulletMesh;
	//bool bullet_is_too_far(SceneNode *n);

	LightNode *bulletLight;
	LightNode *fireLight;
};

#endif