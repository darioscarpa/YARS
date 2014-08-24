#include "Scene.h"

#include "Sandbox.h"
#include "RenderWindow.h"


#include "Input.h"
#include "CameraNode.h"
#include "FpsCameraNode.h"
#include "Renderer.h"
#include "renderers/DoublePassBlendingDeferredRenderer.h"
#include "renderers/DeferredRenderer.h"

#include "renderers/SaAoRenderer.h"
#include "renderers/SaAoIlRenderer.h"

#include "renderers/shaders/sub/SamplingPatternShaderSub.h"


#include "scenes/SceneSponza.h"

#include "gui/Gui.h"

#include "util/vsGLInfoLib.h"
#include "util/string_utils.h"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <locale>

template <class charT, charT sep>
class punct_facet: public std::numpunct<charT> {
protected:
    charT do_decimal_point() const { return sep; }
};



std::unique_ptr<Scene> Scene::getEmptyScene() {
	Scene *s = new Scene();
	s->setLabel("default");
	s->setBackgroundColor(glm::vec3(1.0, 1.0, 1.0));

	s->setAmbientLightLevel(0.5);

	SceneNode *root = new SceneNode("root");
	s->setRootNode(root);

	FpsCameraNode *c = new FpsCameraNode("fpsCamera");
	c->setPosition(glm::vec3(0.0, 0.0, 10.0)); 
	s->addCameraNode(c);


	//Asset *cube = Sandbox::loadAsset("cube/cube.obj");
	Asset *cube = Sandbox::loadAsset("Rubik/Rubik's cube.obj");
	AssetNode *a = new AssetNode("cube", cube);
	a->setPosition(glm::vec3(0.0, 0.0, 0.0));
	a->setScaling(glm::vec3(0.01));
	a->attach(cube);
	s->addAssetNode(a);

	LightNode *l = new LightNode("pointlight");
	l->setPosition(glm::vec3(0.0, 10.0, 0.0));
	s->addLightNode(l);

	CameraPath *cp = new CameraPath();
	cp->addKnot(glm::vec3(0.0, 0.0, 10.0), c->getOrientation());
	cp->addKnot(glm::vec3(0.0, 0.0, 20.0), c->getOrientation());
	s->addCameraPath(cp);

	return std::unique_ptr<Scene>(s);

}


Scene::Scene() : 
		label("scene"),
		currNodeLabel(""),
		pointLightAsset("pointLightMarker", Sandbox::loadAsset("sphere/sphere.obj")) {	

	currNode = 0;
	
	//label = "scene";	
	//currNodeLabel = "";

	currCamera = 0;
	currAsset = 0;
	currLight = 0;

	activeCamera  = nullptr;
	activeAsset   = nullptr;
	activeLight   = nullptr;
	activeCamPath = nullptr;

	activeNode    = nullptr;
	rootNode      = nullptr;
	
	//pointLightAsset.attach(Sandbox::loadAsset("sphere/sphere.obj"));
	pointLightAsset.setScaling(glm::vec3(0.1));	
}

Scene::~Scene() {
	printf("Destroying scene: %s\n", label.c_str());

}

void Scene::setup() {
	
}

void Scene::onActivate() {
	reshape(RenderWindow::getWidth(), RenderWindow::getHeight());
}

////////////////////////////////////////////////////////////////////////////////
void Scene::reshape(int winW, int winH) {
	float ar = (float) winW / winH;
	for (auto c : cameras) {
		if (c->getAutoUpdateAspectRatio()) 
			c->setAspectRatio(ar);
	}	
}

////////////////////////////////////////////////////////////////////////////////
template<class vecType>
vecType * rotiterateNext(const std::vector<vecType*> &v, int *current) {
	(*current)++;
	if (*current == v.size()) *current = 0;
	return v.at(*current);
}

template<class vecType>
vecType * rotiteratePrev(const std::vector<vecType*> &v, int *current) {
	(*current)--;
	if (*current < 0) *current = v.size() - 1;
	return v.at(*current);
}

SceneNode * const  Scene::nextNode()  {	return rotiterateNext(nodes, &currNode); }
SceneNode * const  Scene::prevNode()  {	return rotiteratePrev(nodes, &currNode); }

CameraNode * const Scene::nextCamera() { return rotiterateNext(cameras, &currCamera); }
CameraNode * const Scene::prevCamera() { return rotiteratePrev(cameras, &currCamera); }

LightNode * const  Scene::nextLight() {	return rotiterateNext(lights, &currLight); }
LightNode * const  Scene::prevLight() { return rotiteratePrev(lights, &currLight); }

AssetNode * const  Scene::nextAsset() {	return rotiterateNext(assets, &currAsset); }
AssetNode * const  Scene::prevAsset() { return rotiteratePrev(assets, &currAsset); }
////////////////////////////////////////////////////////////////////////////////

SceneNode *const Scene::getNodeByLabel(const std::string& name) {
	for (auto n : nodes) {
		if (n->getLabel() == name) return n;
	}
	return nullptr;
}

void Scene::remCameraPath(CameraPath *cp) {
	auto it = remove_if(camPaths.begin(), camPaths.end(), 
			  [cp](std::shared_ptr<CameraPath> campath) { return campath.get() == cp;});
	if (it != camPaths.end()) {		
		camPaths.erase(it, camPaths.end());		
		
		if (camPaths.empty()) 
			setActiveCamPath(nullptr);
		else {
			setActiveCamPath(camPaths[0].get());
		}
	}		
}
void Scene::addCameraPath(CameraPath *cp) {
	if (camPaths.empty()) setActiveCamPath(cp);

	std::string baselbl = cp->getLabel();
	if (baselbl == "") baselbl = "path" ;

	int i = 0;
	while (1) {
		i++;
		std::string lbl = baselbl + std::to_string(i);	
		auto it = find_if(camPaths.begin(), camPaths.end(), [lbl] (std::shared_ptr<CameraPath> x) { return x.get()->getLabel() == lbl; } );

		if (it == camPaths.end()) {
			cp->setLabel(lbl);
			camPaths.push_back(std::shared_ptr<CameraPath>(cp));
			break;
		}
		
		/*if (it != camPaths.end()) lbl = lbl + "_";

		if (cp->getLabel() == "" || it!=camPaths.end()) {
			cp->setLabel("path"+std::to_string(camPaths.size()));
		}*/
	} 	
}

void Scene::addCameraPathByCameras() {
	if (cameras.size()>1) {
		CameraPath *cp = new CameraPath();
		for (auto c : cameras) {
			cp->addKnot(c->getPosition(), c->getOrientation());
		}
		addCameraPath(cp);
	}
}

void Scene::makeUniqueLabel(SceneNode *n, const char *baseName) {
	int id = 1; 
	while ( n->getLabel() == "" 
		    || getNodeByLabel(n->getLabel()) != nullptr
		  ) {
		n->setLabel(baseName+std::to_string(id++));
	}	
}

void Scene::addLightNode(LightNode *l,   SceneNode *father) { 
	if (lights.empty()) setActiveLight(l);
	
	l->setArrayPos(lights.size());
	makeUniqueLabel(l, "Light");
	
	lights.push_back(l); 
	addNode(l, father);
}

void Scene::addCameraNode(CameraNode *c, SceneNode *father) {
	if (cameras.empty()) setActiveCamera(c);
	
	std::string type("Camera");
	if (dynamic_cast<FpsCameraNode*>(c))  type = "FpsCamera";
	makeUniqueLabel(c, type.c_str());

	cameras.push_back(c);
	addNode(c, father);
}

void Scene::addAssetNode(AssetNode *a,   SceneNode *father) { 
	if (assets.empty()) setActiveAsset(a);	
	
	makeUniqueLabel(a, "Asset");
	
	assets.push_back(a); 
	addNode(a, father); 
}

void Scene::addNode(SceneNode *n, SceneNode *father) {
	if (father == nullptr) father = rootNode;	
	printf("addNode - nodes->size %d %s (father: %s)\n",
			 nodes.size(),
			 n->getLabel().c_str(),
			 father->getLabel().c_str());
	 //printf("addNode - father: %s \n", father->getLabel().c_str());
	nodes.push_back(n);
	father->addChild(n);
}

void Scene::remNode(SceneNode *n, SceneNode *father) {
	if (father == nullptr) father = rootNode;	
	for(size_t i = 0; i < nodes.size(); ++i) {
         if(nodes.at(i) == n) {
              nodes.erase(nodes.begin() + i);
              break; 
         }
	}
	if ( CameraNode *nn = dynamic_cast<CameraNode*>(n) ) {
		for(size_t i = 0; i < cameras.size(); ++i) {
			if(cameras.at(i) == nn) {
				cameras.erase(cameras.begin() + i);
				break; 
			}
		}	
	} else if ( AssetNode *nn = dynamic_cast<AssetNode*>(n) ) {
		for(size_t i = 0; i < assets.size(); ++i) {
			if(assets.at(i) == nn) {
				assets.erase(assets.begin() + i);
				break; 
			}
		}
	} else if ( LightNode *nn = dynamic_cast<LightNode*>(n) ) {
		for(size_t i = 0; i < lights.size(); ++i) {
			if(lights.at(i) == nn) {
				lights.erase(lights.begin() + i);
				break; 
			}
		}
	}
	father->remChild(n);
	/*
	if (father == nullptr) father = rootNode;
	printf("remNode %s\n", n->getLabel().c_str());
	for (int i = 0; i < father->getChildNodes()->size(); i++) {
		if (father->getChildNodes()->at(i) == n) {
			for (int j = i; j < father->getChildNodes()->size()-1; j++) {
				father->getChildNodes()->at(j) = father->getChildNodes()->at(j+1);
			}
			father->getChildNodes()->pop_back();
			printf("father->getChildNodes()->pop_back()\n");
			//delete n;
			break;
		}				
	}
	for (int i = 0; i < nodes->size(); i++) {
		if (nodes->at(i) == n) {
			for (int j = i; j < nodes->size()-1; j++) {
				nodes->at(j) = nodes->at(j+1);
			}
			nodes->pop_back();
			printf("nodes->pop_back()\n");
			break;
		}
	}
	delete n;
	*/
	/*
			int idxToDelete = -1;			
			for (int i = 0; i < father->getChildNodes()->size(); i++) {
				if (father->getChildNodes()->at(i) == n) {
					idxToDelete = i;
				}				
			}
			if (idxToDelete == -1) {
				printf("remNode failure, node not found");
			} else {
				for (int i = idxToDelete; i < father->getChildNodes()->size()-1; i++) {
					father->getChildNodes()->at(i) = father->getChildNodes()->at(i+1);
				}
				father->getChildNodes()->pop_back();
				delete n;
			}*/
}

////////////////////////////////////////////////////////////////////////////////

void Scene::startPlayingCameraPath(bool loop) {	
	if (!activeCamPath->isPlayable()) {
		printf("non playable path\n");
		return;
	}
	float duration = activeCamPath->getDuration();
	Sandbox::startProfiling(duration);		
	camPathWalker.setPlaybackTime(duration);
	camPathWalker.setLooping(loop);
	camPathWalker.play();
}

void Scene::stopPlayingCameraPath() {
	camPathWalker.stop();
}


void Scene::updateAnimatedNodes(float dt) {
	/*for ( auto a : animatedAssets ) {
		a->nextFrame();	
	}*/

	for ( auto a : assets ) {
		if (auto aa  = dynamic_cast<AnimatedAssetNode*>(a)) 
			aa->update(dt);	
	}
}

void Scene::updateCameraPathWalking(const float dt) {
	if (!activeCamPath->isPlayable()) {
		camPathWalker.setPaused(true);
		printf("non playable path (not enough nodes)\n");		
	}
	Sandbox::updateProfiling(dt);
	
	camPathWalker.update(dt);
	if ( camPathWalker.isPlayingFinished() ) {
		Sandbox::stopProfiling();
		Gui::show();	
	} else {		
		float time = camPathWalker.getElapsedTime();
		//printf("elapsed: %f\n", time);
		
		glm::vec3 pos;
		glm::quat ori;
		//activeCamPath->getCamStatusAt(time, &pos, &ori);
		activeCamPath->getSplineCamStatusAt(time, &pos, &ori);
		
		getActiveCamera()->setPosition(pos);
		getActiveCamera()->setOrientation(ori);
	}	
}

void Scene::setSpecialPoint() { screenSpacePoint = Sandbox::getMousePos(); }

void Scene::setEmisphereCenterPoint() { emisphereCenterPoint = Sandbox::getMousePos(); }




void writeBlenderScript(const CameraNode &cam) {	

	float fov = cam.getFOV();

	float pos_x = cam.getPosition().x;
	float pos_y = cam.getPosition().y;
	float pos_z = cam.getPosition().z;
			
	float rot_qw = cam.getOrientation().w;
	float rot_qx = cam.getOrientation().x;
	float rot_qy = cam.getOrientation().y;
	float rot_qz = cam.getOrientation().z;
	

	/*
	glm::mat3 yup2zup( 1.0, 0.0, 0.0,
		               0.0, 0.0, 1.0, 
					   0.0, 1.0, 0.0 );
	glm::quat zupori = cam.getOrientation() * glm::quat(yup2zup) ;
	glm::vec3 rotangleszup = glm::eulerAngles(zupori);
			
	float rot_x = rotangleszup.x;
	float rot_y = rotangleszup.y;
	float rot_z = rotangleszup.z;
	*/

	glm::vec3 rotangles = glm::eulerAngles(cam.getOrientation());
	float rot_x = rotangles.x;
	float rot_y = rotangles.y;
	float rot_z = rotangles.z;


	int width  = RenderWindow::getWidth();
	int height = RenderWindow::getHeight();

	int   tech = 0;
	int   samples = 16;
	float dist_max = 1.0;
	int   renderer = 0;
	int   samplingPattern = 0;

	int   emisphereSamplingTech = 0;
	float samplingRadius = 1.0;
	int triangleDivs = 0; 
	float aoMultiplier = 1;
	int areaCalc = 0;
	float angleBias = 0;

	float alchemy_k = 0;
	float alchemy_ro = 0;
	float alchemy_u = 0;
	if ( DoublePassBlendingDeferredRenderer *dpbdr = dynamic_cast<DoublePassBlendingDeferredRenderer*>(Sandbox::getActiveRenderer()) ) {
		renderer = 4;
		samplingPattern = dpbdr->getSamplingPatternId();
		if (samplingPattern == SamplingPatternShaderSub::OptionValues::emisphere) {
			emisphereSamplingTech = dpbdr->getHemisphereSamplingMode();
		}
		samples = dpbdr->getBSnumSamples();
		dist_max = dpbdr->getBSdistMax();
	} else if ( DeferredRenderer *dr = dynamic_cast<DeferredRenderer*>(Sandbox::getActiveRenderer()) ) {
		renderer = 3;
		samplingPattern = dr->getSamplingPatternId();
		if (samplingPattern == SamplingPatternShaderSub::OptionValues::emisphere) {
			emisphereSamplingTech = dr->getHemisphereSamplingMode();
		}
		samples = dr->getDSSAOsamples();
		dist_max = dr->getDSSAOdistMax();
	} else if ( SaAoRenderer *saaor = dynamic_cast<SaAoRenderer*>(Sandbox::getActiveRenderer()) ) {
		renderer = 1;
		samplingPattern = saaor->getSamplingPatternId();
		if (samplingPattern == SamplingPatternShaderSub::OptionValues::emisphere) {
			emisphereSamplingTech = saaor->getHemisphereSamplingMode();
		}
		tech = saaor->getTechnique();
		
		samples = saaor->getAoNumSamples();
		samplingRadius = saaor->getAoSamplingRadius();
		dist_max = saaor->getAoMaxDistance();

		triangleDivs = saaor->getTriangleSideDivs();
		aoMultiplier = saaor->getAoMultiplier();
		areaCalc = saaor->getAreaComputationId();
		angleBias = saaor->getAoAngleBias();

		alchemy_k  = saaor->getAlchemyK();
		alchemy_ro = saaor->getAlchemyRO();
		alchemy_u  = saaor->getAlchemyU();
	} else if ( SaAoIlRenderer *saaoilr = dynamic_cast<SaAoIlRenderer*>(Sandbox::getActiveRenderer()) ) {
		renderer = 2;
		samplingPattern = saaoilr->getSamplingPatternId();
		if (samplingPattern == SamplingPatternShaderSub::OptionValues::emisphere) {
			emisphereSamplingTech = saaoilr->getHemisphereSamplingMode();
		}
		tech = saaor->getTechnique();
		samples = saaoilr->getAoNumSamples();
		samplingRadius = saaoilr->getAoSamplingRadius();
		dist_max = saaoilr->getAoMaxDistance();
		triangleDivs = saaoilr->getTriangleSideDivs();
		aoMultiplier = saaoilr->getAoMultiplier();
		areaCalc = saaoilr->getAreaComputationId();
		angleBias = saaoilr->getAoAngleBias();
	}

	
	std::ostringstream oss;	
	oss << "_test/sponza" 
		<< std::fixed << std::setprecision(3)
		<< "(" << pos_x << "_" << pos_y << "_" << pos_z << ")"
		<< "(" << rot_x << "_" << rot_y << "_" << rot_z << ")" 
		<< width << "x" << height << "_" 
		<< "maxdist" << dist_max << "_" 
		<< "GL_"
		<< "r" << renderer << "_"
		<< "samples" << samples << "_" 		
		<< "s" << samplingPattern << "_"
		<< "e" << emisphereSamplingTech << "_"
		<< "samplingRadius" << samplingRadius << "_"
		<< "tDivs" << triangleDivs << "_"
		<< "aoMul" << aoMultiplier  << "_"
		<< "area"  << areaCalc  << "_"
		<< "abias" << angleBias << "_"
		<< "fps"   << Sandbox::getFPS() 
		;
		//std::setw( 3 ) << std::setfill( '0' )
		//<< std::fixed << std::setprecision(3)
		//<< dist_max << "maxdist";
	Sandbox::saveScreenshot(oss.str(), "png", true);	


	std::ostringstream csv;
	csv.imbue(std::locale(oss.getloc(), new punct_facet<char, ','>));
	csv << std::fixed << std::setprecision(4)
	//SSIM index	triangleDivs	numSamples	maxDistance	samplingRadius	aoMultiplier	angleBias	area
	//<< "r" << renderer << "_"
	//<< "s" << samplingPattern << "_"	
	<< ";"
	<< (tech==0?"saSSAO":"alchemy") << ";"
	<<  Sandbox::getFPS()  << ";"	
	<< dist_max << ";"
	<< samplingRadius << ";"	
	<< (samplingPattern==SamplingPatternShaderSub::flat?"flat":"emisphere") << ";"
	<< samples << ";" 
	 
	<< angleBias << ";"
	<< aoMultiplier  << ";"
	
	<< (tech==0?std::to_string(triangleDivs):"") << ";"	
	<< (tech==0?std::to_string(areaCalc):"")  << ";"

	<< (tech==1?std::to_string(alchemy_k):"") << ";"	
	<< (tech==1?std::to_string(alchemy_ro):"")  << ";"
	<< (tech==1?std::to_string(alchemy_u):"")  << ";"

	<< std::endl;
		
	std::string csvFilepath = oss.str() + ".png.csv";
	std::ofstream csvStream(csvFilepath, std::ios::out);
	if (csvStream.is_open()) {
		csvStream << csv.str();
		csvStream.close();
	} else {
		printf("Impossible to open %s. Check your paths!\n", csvFilepath.c_str());		
		throw new std::exception("file writing failed\n");		
	}

	
	std::cout << "FOV " << std::to_string(fov) << "\n";
	std::cout << "WIDTH " << std::to_string(width) << "\n";
	std::cout << "HEIGHT " << std::to_string(height) << "\n";

	std::cout << "SAMPLES " << std::to_string(samples) << "\n";
	std::cout << "DIST_MAX " << std::to_string(dist_max) << "\n";


	std::cout << "POS_X " << std::to_string(pos_x) << "\n";
	std::cout << "POS_Y " << std::to_string(pos_y) << "\n";
	std::cout << "POS_Z " << std::to_string(pos_z) << "\n";

	std::cout << "ROT_X " << std::to_string(rot_x) << "\n";
	std::cout << "ROT_Y " << std::to_string(rot_y) << "\n";
	std::cout << "ROT_Z " << std::to_string(rot_z) << "\n";
			
	std::cout << "ROT_QW " << std::to_string(rot_qw) << "\n";
	std::cout << "ROT_QX " << std::to_string(rot_qx) << "\n";
	std::cout << "ROT_QY " << std::to_string(rot_qy) << "\n";
	std::cout << "ROT_QZ " << std::to_string(rot_qz) << "\n";

	std::string code;	
	std::string skelFilepath = "_blender/setCamAndRenderSkel.py";
	std::ifstream skelStream(skelFilepath, std::ios::in);	

	if(skelStream.is_open()){
		std::string line = "";
		while(getline(skelStream, line)) {
			// do substitutions in line			
			line = StringUtils::strReplace(line, "FOV", std::to_string(fov));
			line = StringUtils::strReplace(line, "WIDTH", std::to_string(width));
			line = StringUtils::strReplace(line, "HEIGHT", std::to_string(height));
			
			line = StringUtils::strReplace(line, "SAMPLES", std::to_string(samples));
			line = StringUtils::strReplace(line, "DIST_MAX", std::to_string(dist_max));

			line = StringUtils::strReplace(line, "POS_X", std::to_string(pos_x));
			line = StringUtils::strReplace(line, "POS_Y", std::to_string(pos_y));
			line = StringUtils::strReplace(line, "POS_Z", std::to_string(pos_z));

			line = StringUtils::strReplace(line, "ROT_X", std::to_string(rot_x));
			line = StringUtils::strReplace(line, "ROT_Y", std::to_string(rot_y));
			line = StringUtils::strReplace(line, "ROT_Z", std::to_string(rot_z));

			line = StringUtils::strReplace(line, "ROT_QW", std::to_string(rot_qw));
			line = StringUtils::strReplace(line, "ROT_QX", std::to_string(rot_qx));
			line = StringUtils::strReplace(line, "ROT_QY", std::to_string(rot_qy));
			line = StringUtils::strReplace(line, "ROT_QZ", std::to_string(rot_qz));

			code += "\n" + line;
		}
		skelStream.close();		
	} else {
		printf("Impossible to open %s. Check your paths!\n", skelFilepath.c_str());		
		throw new std::exception("file loading failed\n");		
	}
	
	std::string scriptFilepath = "_blender/setCamAndRender.py";
	std::ofstream scriptStream(scriptFilepath, std::ios::out);
	if (scriptStream.is_open()) {
		scriptStream << code;
		scriptStream.close();
	} else {
		printf("Impossible to open %s. Check your paths!\n", scriptFilepath.c_str());		
		throw new std::exception("file writing failed\n");		
	}
}


bool rotCamera;
int prevFrameMouseX;
int prevFrameMouseY;
void Scene::update(Input *input, float dt) {

	if (  activeCamPath != nullptr && camPathWalker.isPlaying() && !camPathWalker.isPaused() ) updateCameraPathWalking(dt);
	updateAnimatedNodes(dt);

	int mx = input->getMouseX();
	int my = input->getMouseY();

	int mxDelta = prevFrameMouseX-mx;
	int myDelta = prevFrameMouseY-my;
	
	prevFrameMouseX = mx;
	prevFrameMouseY = my;

	float cameraSpeed = 2.0;
	float amount = dt*cameraSpeed;

	//move speed and rotation speed
	float mSpeed = amount*3;
	float rSpeed = amount*20;

	float mouseSensivity = 1;
	float mouseHor = mouseSensivity*(mxDelta);
	float mouseVer = mouseSensivity*(myDelta);
		
	if (input->keyDown('W')) activeCamera->moveForward(mSpeed);
	if (input->keyDown('S')) activeCamera->moveBackwards(mSpeed);
	if (input->keyDown('A')) activeCamera->moveLeft(mSpeed);
	if (input->keyDown('D')) activeCamera->moveRight(mSpeed);

	if (input->keyDown('Q')) activeCamera->rotateLeft(rSpeed);
	if (input->keyDown('E')) activeCamera->rotateRight(rSpeed);	
	
	if (rotCamera) {
		if (input->keyDown('R')) activeCamera->rotate(-rSpeed, 0, 0);
		if (input->keyDown('F')) activeCamera->rotate( rSpeed, 0, 0); 
		if (input->keyDown('T')) activeCamera->rotate(0, -rSpeed, 0);
		if (input->keyDown('G')) activeCamera->rotate(0,  rSpeed, 0); 
		if (input->keyDown('Y')) activeCamera->rotate(0, 0, -rSpeed);
		if (input->keyDown('H')) activeCamera->rotate(0, 0,  rSpeed); 
	} else {
		if (input->keyDown('R')) activeNode->rotate(-rSpeed, 0, 0);
		if (input->keyDown('F')) activeNode->rotate( rSpeed, 0, 0); 
		if (input->keyDown('T')) activeNode->rotate(0, -rSpeed, 0);
		if (input->keyDown('G')) activeNode->rotate(0,  rSpeed, 0); 
		if (input->keyDown('Y')) activeNode->rotate(0, 0, -rSpeed);
		if (input->keyDown('H')) activeNode->rotate(0, 0,  rSpeed); 
	}
	
	if (input->keyDown('I')) activeNode->move(0, 0, -mSpeed);
	if (input->keyDown('K')) activeNode->move(0, 0,  mSpeed);
	if (input->keyDown('J')) activeNode->move(-mSpeed, 0, 0);
	if (input->keyDown('L')) activeNode->move( mSpeed, 0, 0);
	if (input->keyDown('U')) activeNode->move(0,  mSpeed, 0);
	if (input->keyDown('O')) activeNode->move(0, -mSpeed, 0);
	
	if (input->mouseWheelScroll()!= 0) {
		activeCamera->moveForward(input->mouseWheelScroll()*mSpeed);
	}
			
	if (input->mouseDown(Input::BTN_LEFT)) {
		float hor = mouseHor*rSpeed;
		float ver = mouseVer*rSpeed;		
		activeCamera->rotate(hor,ver,0);
		//activeCamera->rotate(hor,0,0);
		//activeCamera->rotate(0,ver,0);
	}
	if (input->mouseDown(Input::BTN_RIGHT)) {
		float hor = mouseHor*mSpeed;
		float ver = mouseVer*mSpeed;
		activeCamera->move(-hor, ver, 0);
		//activeCamera->moveX(-hor);
		//activeCamera->moveY(ver);				
	}
	if (input->mouseDown(Input::BTN_MIDDLE)) {
		float hor = mouseHor*rSpeed;
		activeCamera->roll(hor);		
	}
	
	int key = input->getLastKeyPressed();
	
	//static bool normalMapping   = false;
	//static bool specularMapping = false;
	//static bool alphaMapping    = false;
	//static bool showingGBuffer  = false;
	switch(key) {		
		case 27: //case 'q':
			exit (0);
			break;		
		case 39: //'
			setActiveCamera(prevCamera());
			break;
		case 204: //ì
			setActiveCamera(nextCamera());
			break;		
		case 200: //è
			setActiveAsset(prevAsset());
			break;
		case 43: // +
			setActiveAsset(nextAsset());
			break;
		case 192: // à
			setActiveLight(nextLight());
			break;
		case 217: // ù
			setActiveLight(prevLight());
			break;
		case 295: // backspace
			// currentNode = currentCamera
			setActiveNode(getActiveCamera());
			break;
		case 294: // enter
			// currentNode = currentLight
			setActiveNode(getActiveLight());
			break;
		case 288: // r shift
			// currentNode = currentAsset
			setActiveNode(getActiveAsset());
			break;
		

		case 'C':
			setActiveCamera(nextCamera());
			break;
		case 'X':
			setActiveNode(nextNode());
			printf("active node: %s pos: %.2f %.2f %.2f\n", activeNode->getLabel().c_str(), 
				    activeNode->getPosition().x,
				    activeNode->getPosition().y,
				    activeNode->getPosition().z);
			break;
		case 'Z':
			//rotCamera = !rotCamera;
			writeBlenderScript(*activeCamera);
			printf("cam pos: %.2f %.2f %.2f\n",
				    activeCamera->getPosition().x,
				    activeCamera->getPosition().y,
				    activeCamera->getPosition().z);
			
			if (SceneNode *p = activeCamera->getParent()) 
			printf("cam pos: %.2f %.2f %.2f\n",
				    p->getPosition().x,
				    p->getPosition().y,
				    p->getPosition().z);

			printf("cam ori: w=%.2f x=%.2f y=%.2f z=%.2f\n",
					activeCamera->getOrientation().w,
				    activeCamera->getOrientation().x,
				    activeCamera->getOrientation().y,
					activeCamera->getOrientation().z);
			{

			
			glm::mat3 yup2zup( 1.0, 0.0, 0.0,
				               0.0, 0.0, 1.0, 
							   0.0, 1.0, 0.0 );

			//glm::quat zupori = glm::quat(yup2zup) * activeCamera->getOrientation();
			glm::quat zupori = activeCamera->getOrientation() * glm::quat(yup2zup) ;

			
			glm::vec3 rotangles = glm::eulerAngles(activeCamera->getOrientation());
			printf("cam ori angles %.2f %.2f %.2f\n",
					rotangles.x, rotangles.y, rotangles.z);
			
			printf("cam ori: w=%.2f x=%.2f y=%.2f z=%.2f\n",
					zupori.w,
				    zupori.x,
				    zupori.y,
					zupori.z);
			glm::vec3 rotangleszup = glm::eulerAngles(zupori);
			printf("cam ori angles %.2f %.2f %.2f\n",
					rotangleszup.x, rotangleszup.y, rotangleszup.z);
			}
			printf("-----\n");
/*			printf("light pos: %.2f %.2f %.2f\n",
				    pointLightNode->getPosition().x,
				    pointLightNode->getPosition().y,
				    pointLightNode->getPosition().z);*/
			if (SceneSponza *ss = dynamic_cast<SceneSponza*>(this)) {
				ss->shootLightBullet();			
			}						
			//VSGLInfoLib::getUniformInBlockInfo(renderer->getCurrentProgramId() , "Lights", "pointLights[0].posCS");

			break;
		case 'B':			
			if (SceneSponza *ss = dynamic_cast<SceneSponza*>(this)) {
				ss->shootLightBullet();			
			}						

		case 'V':
			{
			SceneNode *pl = static_cast<SceneNode*>( lights.at(0) );
			setActiveNode(pl);
			pl->setPosition(activeCamera->getPosition());
			pl->setOrientation(activeCamera->getOrientation());
						
			break;

			}
		case 32: //spacebar			
			setSpecialPoint();
			setEmisphereCenterPoint();

			break;
		
		/*case 'M':
			renderer->reinit();
			break;
			
		case 'N':			
			normalMapping = !normalMapping;
			renderer->setNormalMapping(normalMapping);
			printf("normalMapping: %d\n", normalMapping);
			break;
		case 'B':			
			specularMapping = !specularMapping;
			renderer->setSpecularMapping(specularMapping);
			printf("specularMapping: %d\n", specularMapping);
			break;
		case ',':			
			alphaMapping = !alphaMapping;
			renderer->setOpacityMapping(alphaMapping);
			printf("alphaMapping: %d\n", alphaMapping);
			break;
			
		case 'G':						
			if (DeferredRenderer *r = dynamic_cast<DeferredRenderer*>(renderer)) {
				showingGBuffer = !showingGBuffer;
				r->setShowingGBuffer(showingGBuffer);
				printf("showingGBuffer: %d\n", showingGBuffer);
			} else {
				printf("not using deferred rendering\n");
			}			
			break;
			*/
	//	case 'm':
	//		msaa = 1 - msaa;
	//		break;
		default:
			if (key!=0) printf("unmapped key pressed %c %d\n", key, key);
			break;
	}	
	
}

///////////////////////////////////////////////////////////////////////////////
