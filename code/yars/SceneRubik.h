#ifndef SCENERUBIK_H
#define SCENERUBIK_H

#include "Scene.h"

#include "Asset.h"

class SceneRubik : public Scene {
public:
		SceneRubik();
		void setup();	
		void update(Input * input, float dt);

private:
	   std::vector<AssetNode*> cubes;
	   std::vector<AssetNode*> rotcubes;
	   int   moveUpDownDir;
};

#endif
