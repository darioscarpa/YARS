#include "SceneSponza.h"

#include "Sandbox.h"

#include "AnimatedAsset.h"
#include "Asset.h"
#include "AssetNode.h"

#include "CameraNode.h"
#include "FpsCameraNode.h"

#include "LightNode.h"

#include "Renderer.h"

#include <algorithm>

SceneSponza::SceneSponza() : Scene() {
	setLabel("sponza");

	bullets     = nullptr;
	bulletMesh  = nullptr;
	bulletLight = nullptr;
	fireLight   = nullptr;
}

void SceneSponza::setup() {
	
	setRootNode(new SceneNode("root"));
	setActiveNode(getRootNode());

	setBackgroundColor(glm::vec3(0.0f, 0.72f, 0.96f));
	setAmbientLightLevel(0.5);
	
	//float sponzaSF;
	//Asset *Asponza = Sandbox::loadAsset("cube/cube.obj");
	//Asset *Asponza = Sandbox::loadAsset("Rubik/Rubik's cube.obj");	
	//Asset *Asponza  = assetLoader->loadAsset("T:/src/models/sibenik/sibenik.obj");
	//Asset *Asponza  = assetLoader->loadAsset("T:/src/models/sponza/sponza_clean.obj");

	Asset *Ateapot = Sandbox::loadAsset("teapot/teapot.obj");	
	AssetNode *Nteapot = new AssetNode("sponza", Ateapot);
	Nteapot->setPosition(glm::vec3(-3.0, 0.0, 0.0));
	Nteapot->setScaling(glm::vec3(0.02, 0.02, 0.02));
	Nteapot->setOrientation(glm::quat(0.707229, 0, -0.706985, 0));
	addAssetNode(Nteapot);	

	Asset *Asponza = Sandbox::loadAsset("sponza_obj/sponza.obj");	
	AssetNode *Nsponza = new AssetNode("sponza", Asponza);
	Nsponza->setPosition(glm::vec3(0.0, 0.0, 0.0));
	Nsponza->setScaling(glm::vec3(0.01, 0.01, 0.01));
	addAssetNode(Nsponza);
	setActiveAsset(Nsponza);

	/*
	Asset *Acyl = Sandbox::loadAsset("cylinder/cylinder.obj");
	AssetNode *Ncyl = new AssetNode("cyl", Acyl);	
	Ncyl->setPosition(glm::vec3(0.5, 0.0, 0.0));
	Ncyl->setScaling(glm::vec3(0.1, 0.1, 0.1));
	addAssetNode(Ncyl);
	
	
	Asset *panchinaA = Sandbox::loadAsset("bench.obj");
	AssetNode *panchinaN = new AssetNode("panchina", panchinaA);
	panchinaN->setScaling(glm::vec3(0.1f));
    panchinaN->setPosition(glm::vec3(0.0f, 0.0f, 1.0f));
	addAssetNode(panchinaN);
	
	
	Asset *Mcube = Sandbox::loadAsset("Rubik/Rubik's cube.obj");
	AssetNode *Ncube = new AssetNode("cube", Mcube);
	Ncube->setPosition(glm::vec3(0.0, 0.0, 0.0));
	Ncube->setScaling(glm::vec3(0.01));
	addAssetNode(Ncube);

	AssetNode *Ncube2 = new AssetNode("cube2", Mcube);
	Ncube2->setPosition(glm::vec3(0.0, 0.2, 0.0));
	Ncube2->setScaling(glm::vec3(0.01));	
	addAssetNode(Ncube2);

	AssetNode *Ncube3 = new AssetNode("cube3", Mcube);
	Ncube3->setPosition(glm::vec3(0.0, 2.0, 0.0));
	addAssetNode(Ncube3);
	*/
	
	/*
	AnimatedAsset *bigguy = Sandbox::loadAnimatedAsset("Bigguy/bigguy_", ".obj", 59);
	AnimatedAssetNode *Nbigguy = new AnimatedAssetNode("bigGuy", bigguy);
	addAssetNode(Nbigguy);
	

	Asset *Msphere = Sandbox::loadAsset("lv_sphere.obj");
	*/

	CameraNode *cam1 = new CameraNode("cam1");
	//cam1->lookAt(glm::vec3(4,3,-3), glm::vec3(0,0,0), glm::vec3(0,1,0));
	cam1->setPosition(glm::vec3(8.28, 3.20, -0.11));
	cam1->setOrientation(glm::quat(0.11, 0.69, 0.12, -0.71));	

	CameraNode *plantCam = new CameraNode("plantCam");
	plantCam->setPosition(glm::vec3(2.16, 1.02, -0.13));
	plantCam->setOrientation(glm::quat(-0.09,  -0.98, -0.06, -0.19));
	//nPointLight->setPosition(2.16, 1.02, -0.13);

	CameraNode *plantCam2 = new CameraNode("plantCam");
	plantCam2->setPosition(glm::vec3(-2.16, 0.79, -2.03));
	plantCam2->setOrientation(glm::quat(-0.15, 0.57, 0.36, 0.73));

	CameraNode *modelCam = new CameraNode("cam2");
	//modelCam->lookAt(glm::vec3(-3.0f, 7.0f, -10.0f),
	modelCam->lookAt(glm::vec3(11.2f, 1.50f, -7.5f),
					glm::vec3(0.0f, -0.2f, 1.0f),
					glm::vec3(0.0, 1.0f, 0.0f));
	modelCam->setPosition(glm::vec3(5.15f, 5.38f, -0.68f));
	//modelCam->setOrientation();

	CameraNode *benchCam = new CameraNode("camPanchina");
	benchCam->lookAt(glm::vec3(0.27f, 1.21f, -1.36f),
					glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(0.0, 1.0f, 0.0f));
	addCameraNode(cam1);
	addCameraNode(plantCam);
	addCameraNode(plantCam2);
	addCameraNode(modelCam);
	addCameraNode(benchCam);
	
	FpsCameraNode* fps = new FpsCameraNode("fps");	
	addCameraNode(fps);
	setActiveCamera(fps);
		

	bullets = new std::vector<SceneNode*>();
	bulletMesh = Sandbox::loadAsset("sphere/sphere.obj"); 	

	LightNode *pl1 = new LightNode("pointLight1");
	pl1->setPosition(glm::vec3(0.0f, 0.0f, 4.0f));
	//pl1->setColor(glm::vec3(0.0f, 0.7f, 0.0f));
	pl1->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
	addLightNode(pl1);

	LightNode *pl2 = new LightNode("pointLight2");
	pl2->setPosition(glm::vec3(0.0f, 0.0f, 4.0f));
	pl2->setColor(glm::vec3(0.0f, 0.0f, 0.5f));
	addLightNode(pl2);

	//pl1->attach(Msphere);
	//addNode(pl1);
	//
	//pl2->attach(Msphere);
	//addNode(pl2);	

	bulletLight = new LightNode("pointLightBullet");
	bulletLight->setColor(glm::vec3(1.0f, 0.0f, 0.0f));
	bulletLight->setPosition(glm::vec3(-100.0, -100.0, -100.0));
	addLightNode(bulletLight);

	fireLight = new LightNode("fireLight1");
	fireLight->setPosition(glm::vec3(-12.5, 1.28, 3.97));
	fireLight->setColor(glm::vec3(1.0f, 120.0f/255.0f, 0.0f));
	addLightNode(fireLight);

	//fireLight->attach(Msphere);
	//addNode(fireLight);

}

void SceneSponza::shootLightBullet() {
	printf("shooting!\n");
	AssetNode *nBullet = new AssetNode("lightBullet", bulletMesh);

	CameraNode *cc = getActiveCamera();

	nBullet->setPosition(cc->getPosition());
	nBullet->setOrientation(cc->getOrientation());
	nBullet->setScaling(glm::vec3(0.05, 0.05, 0.05));
	
	addAssetNode(nBullet);
	//addNode(nBullet);
	bullets->push_back(nBullet);

	bulletLight->setPosition(cc->getPosition());
	bulletLight->setOrientation(cc->getOrientation());			
}


//bool SceneSponza::bullet_is_too_far(SceneNode *n)  {
//   glm::vec3 campos = getActiveCamera()->getPosition();
//   static const float MAXBULLETDIST = 100.0f;
//   return (glm::distance(n->getPosition(),campos) > MAXBULLETDIST) ;       
// }

void SceneSponza::updateLightBullets(float dist) {
	SceneNode *deletable = NULL;
	int deletablePos;
	glm::vec3 campos = getActiveCamera()->getPosition();
	static const float MAXBULLETDIST = 100.0f;

	//bullets->erase( std::remove_if(bullets->begin(), bullets->end(), bullet_is_too_far),
//						bullets->end() ); 
	
	for(std::vector<SceneNode*>::const_iterator it = bullets->begin();
		it != bullets->end();
		++it) {
			SceneNode *b = (*it);
			b->moveForward(dist);
		
			bulletLight->setPosition(b->getPosition());			

			if (glm::distance(b->getPosition(),campos) > MAXBULLETDIST) {
				deletable = b;
				deletablePos = it - bullets->begin();
				printf("found deletable node at %d: %s\n", deletablePos, deletable->getLabel().c_str());
			}
	}
	if (deletable) {
		for (int i = deletablePos; i < bullets->size()-1; i++) {
			bullets->at(i) = bullets->at(i+1);
		}
		bullets->pop_back();
		remNode(deletable);		
	}

	//remNode
	//bullets->erase(deletable);
}


void SceneSponza::update(Input * input, float deltaTime) {
	Scene::update(input, deltaTime);
	updateLightBullets( deltaTime );	
	//updateLightBullets( deltaTime/30.0f );	

	//float yellow = 120.0f + (rand()%30);
	//fireLight->setColor(glm::vec3(1.0f, yellow/255.0f, 0.0f));	
}


