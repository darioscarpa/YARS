#include "SceneRubik.h"

#include "../Sandbox.h"

#include "../FpsCameraNode.h"

SceneRubik::SceneRubik() : Scene(){	
	setLabel("rubik");
	setup();
}

void SceneRubik::setup() {
	setRootNode(new SceneNode("root"));
	setActiveNode(getRootNode());

	setBackgroundColor(glm::vec3(0.96f, 0.72f, 0.0f));

	Asset *Aplane  = Sandbox::loadAsset("whiteplane/whiteplane.obj");
	
	AssetNode *Nplane = new AssetNode("ground", Aplane);
	Nplane->setPosition(glm::vec3(0.0, 0.0, 0.0));
	addAssetNode(Nplane);

	AssetNode *NplaneBack = new AssetNode("back", Aplane);
	NplaneBack->setPosition(glm::vec3(0.0, 10.0, -10.0));
	NplaneBack->rotateVert(90.0f);
	
	addAssetNode(NplaneBack);

	Asset *Adragon = Sandbox::loadAsset("dragon/dragon.obj");
	AssetNode *Ndragon = new AssetNode("dragon", Adragon);
	Ndragon->setPosition(glm::vec3(0.0, 0.0, -5.0));
	addAssetNode(Ndragon);


	Asset *Arubik = Sandbox::loadAsset("Rubik/Rubik's Cube.obj");
	
	float basePos = -7.5f;
	float offset  = -3.0f;
	float rotoffset = 90.0f;
	for (int i = 0 ; i < 6; ++i) {
		AssetNode *Nrubik = new AssetNode("rubik"+std::to_string(i), Arubik);	
		Nrubik->setPosition(glm::vec3(basePos - i*offset, 0.0, 7.5));
		Nrubik->setScaling(glm::vec3(0.01, 0.01, 0.01));
	
		Nrubik->rotate( i < 4 ? rotoffset * i : 0.0, 
				        i == 4 ? 90.0f : ( i == 5 ? -90.f : 0.0),
						0.0
						); 						
	
		addAssetNode(Nrubik);
		cubes.push_back(Nrubik);
	}

	for (int i = 0 ; i < 6; ++i) {
		AssetNode *Nrubik = new AssetNode("rotrubik"+std::to_string(i), Arubik);	
		Nrubik->setPosition(glm::vec3(basePos - i*offset, 0.5, 4.5));
		Nrubik->setScaling(glm::vec3(0.01, 0.01, 0.01));
	
		Nrubik->rotate( i < 4 ? rotoffset * i : 0.0, 
				        i == 4 ? 90.0f : ( i == 5 ? -90.f : 0.0),
						0.0
						); 						
	
		addAssetNode(Nrubik);
		rotcubes.push_back(Nrubik);
	}

	Asset *Ateapot = Sandbox::loadAsset("teapot/teapot.obj");
	for (int j = 0 ; j < 2; ++j) {
		for (int i = 0 ; i < 12; ++i) {
			AssetNode *Nteapot = new AssetNode("teapot"+std::to_string(j)+"_"+std::to_string(i), Ateapot);
			Nteapot->setPosition(glm::vec3(basePos - i*offset*0.5, 0.5, -0.5+j*1.5));
			Nteapot->setScaling(glm::vec3(0.015, 0.015, 0.015));
	
			Nteapot->rotate( 45.0, 0.0, 0.0 );
	
			addAssetNode(Nteapot);		
		}
	}
	
		
	FpsCameraNode* fps = new FpsCameraNode("fpsCam");	
	fps->setPosition(glm::vec3(0.0, 5.0, 20.0));
	addCameraNode(fps);
	setActiveCamera(fps);		
	
	LightNode *pl1 = new LightNode("pointLight1");
	pl1->setType(LightNode::POINT);
	pl1->setPosition(glm::vec3(0.0f, 5.0f, 0.0f));
	pl1->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
	addLightNode(pl1);

	setActiveLight(pl1);	

	setAmbientLightLevel(0.3);

	moveUpDownDir = 1;
}


void SceneRubik::update(Input * input, float dt) {
	Scene::update(input, dt);

	const float moveUpDownOffset = dt*0.2;
	
	/*	
	glm::vec3 pos = cubes[0]->getPosition() + glm::vec3(0.0, moveUpDownDir * moveUpDownOffset, 0.0);
	float newy = pos.y;
	if (newy > 0.5 || newy < -0.5) moveUpDownDir = -moveUpDownDir;
	*/
	
	float newy = cubes[0]->getPosition().y;
	if (newy > 0.5 || newy < -0.5) moveUpDownDir = -moveUpDownDir;

	for (auto c : cubes) {
		//c->setPosition(glm::vec3(c->getPosition().x, newy, c->getPosition().z)); // + glm::vec3(0.0, newy, 0.0));
	
		c->setPosition(c->getPosition() + glm::vec3(0.0, moveUpDownDir * moveUpDownOffset, 0.0));
		//c->setPosition(pos);
		//c->moveY(moveUpDownDir * moveUpDownOffset);
		//if (c->getPosition().y > 0.5 || c->getPosition().y < -0.5) moveUpDownDir = -moveUpDownDir;		
	}

	for (auto c : rotcubes) {
		c->rotateWS(dt*10, 0.0, 0.0);
	}
	//updateLightBullets( (float) deltaTime );	
	//updateLightBullets( deltaTime/30.0f );	

	//	float yellow = 120.0f + (rand()%30);
	//	fireLight->setColor(glm::vec3(1.0f, yellow/255.0f, 0.0f));	
}