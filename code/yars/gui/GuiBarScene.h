#ifndef GUIBARSCENE_H
#define GUIBARSCENE_H

#include "GuiBar.h"

#include "../Scene.h"
#include "../Sandbox.h"

#include <vector>
#include <memory>

#include <sstream>

namespace GuiBarScene_ns {
	class NewNodeCallbackData {
		public:
			NewNodeCallbackData(std::string * const newname,
				                std::string * const parentname,
								std::string * const nodeNewLabel,
								std::string * const nodeNewParent) 
				: newNodeName(newname), 
				  parentNodeName(parentname),
				  currentNodeNewLabel(nodeNewLabel),
				  currentNodeNewParentLabel(nodeNewParent)
			{ }
			~NewNodeCallbackData() { }
		
			std::string *newNodeName;
			std::string *parentNodeName;
			std::string *currentNodeNewLabel;
			std::string *currentNodeNewParentLabel;			 
	};

	typedef  void  (SceneNode::*SceneNodeFloatVec3Setter)(glm::vec3 val);
	typedef  const glm::vec3 (SceneNode::*SceneNodeFloatVec3Getter)(void) const;

	typedef  void  (SceneNode::*SceneNodeFloatQuatSetter)(glm::quat val);
	typedef  const glm::quat (SceneNode::*SceneNodeFloatQuatGetter)(void) const;

	void guiSceneNodeFloatVec3elem_get(void *value, void *clientData, SceneNodeFloatVec3Getter m, int elemPos) {
		glm::vec3 val = (*(static_cast<SceneNode*>(clientData)).*m)();	
		float *dest = (float*) value;
		*dest = val[elemPos];
	}
	void guiSceneNodeFloatVec3elem_set(const void *value, void *clientData, SceneNodeFloatVec3Setter ms, SceneNodeFloatVec3Getter mg, int elemPos) {
		glm::vec3 val =  (*(static_cast<SceneNode*>(clientData)).*mg)();
		val[elemPos] = *(static_cast<const float*>(value));
		(*(static_cast<SceneNode*>(clientData)).*ms)(val); 
	}

	void guiSceneNodeFloatQuatElem_get(void *value, void *clientData, SceneNodeFloatQuatGetter m, int elemPos) {
		glm::quat val = (*(static_cast<SceneNode*>(clientData)).*m)();	
		float *dest = static_cast<float*>(value);
		*dest = val[elemPos];
	}
	void guiSceneNodeFloatQuatElem_set(const void *value, void *clientData, SceneNodeFloatQuatSetter ms, SceneNodeFloatQuatGetter mg, int elemPos) {
		glm::quat val =  (*(static_cast<SceneNode*>(clientData)).*mg)();
		val[elemPos] = *(static_cast<const float*>(value));
		(*(static_cast<SceneNode*>(clientData)).*ms)(val); 
	}

	void guiSceneNodeFloatVec3byQuat_get(void *value, void *clientData, SceneNodeFloatQuatGetter m, int elemPos) {
		glm::quat val = (*(static_cast<SceneNode*>(clientData)).*m)();
		glm::vec3 rotangles = glm::eulerAngles(val);			
		float *dest = static_cast<float*>(value);
		*dest = rotangles[elemPos];
	}
	void guiSceneNodeFloatVec3byQuat_set(const void *value, void *clientData,
		                                 SceneNodeFloatQuatSetter ms, SceneNodeFloatQuatGetter mg, int elemPos) {
		glm::quat val =  (*(static_cast<SceneNode*>(clientData)).*mg)();
		glm::vec3 rotangles = glm::eulerAngles(val);			
		printf("%.2f %.2f %.2f -> ", rotangles.x, rotangles.y, rotangles.z);
		rotangles[elemPos] = *(static_cast<const float*>(value));
		//glm::quat newval(rotangles);
		printf("%.2f %.2f %.2f \n", rotangles.x, rotangles.y, rotangles.z);
		glm::quat rx = glm::angleAxis(glm::radians(rotangles.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat ry = glm::angleAxis(glm::radians(rotangles.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat rz = glm::angleAxis(glm::radians(rotangles.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::quat rot = rz * ry * rx;
		//glm::quat newval = degreesToQuat(rotangles.x, rotangles.y, rotangles.z);
		(*(static_cast<SceneNode*>(clientData)).*ms)(rot);
	}
	////////////////////////////////////////////////////////////////////////
	void TW_CALL setCB_bgColor(const void *value, void *clientData) {
		const float *col = static_cast<const float*>(value); 
		static_cast<Scene*>(clientData)->setBackgroundColor(glm::vec3(col[0], col[1], col[2]));
	}
	void TW_CALL getCB_bgColor(void *value, void *clientData) {
		glm::vec3 bgcol = static_cast<Scene*>(clientData)->getBackgroundColor();

		float *dest = static_cast<float*>(value);
		dest[0] = bgcol.r;
		dest[1] = bgcol.g;
		dest[2] = bgcol.b;	
	}
	////////////////////////////////////////////////////////////////////////
	void TW_CALL setCB_currNode_ori(const void *value, void *clientData) {
		const float *v = static_cast<const float*>(value);
		static_cast<Scene*>(clientData)->getActiveNode()->setOrientation(glm::quat(v[0], v[1], v[2], v[3]));
	}
	void TW_CALL getCB_currNode_ori(void *value, void *clientData) {
		glm::quat ori = static_cast<Scene*>(clientData)->getActiveNode()->getOrientation();	

		float *dest = static_cast<float*>(value);
		dest[0] = ori.x;
		dest[1] = ori.y;
		dest[2] = ori.z;
		dest[3] = ori.w;
	}	
	void TW_CALL getCB_currNode_dir(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
	
		glm::quat ori  = sn->getOrientation();	
		glm::mat3 orim = glm::toMat3(ori);
		glm::vec3 dir  =  -orim[2];

		float *dest = static_cast<float*>(value);
		dest[0] = dir.x;
		dest[1] = dir.y;
		dest[2] = dir.z;	
	}
	////////////////////////////////////////////////////////////////////////
	void TW_CALL getCB_currNode_posX(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3elem_get(value, sn, &SceneNode::getPosition, 0);
	}
	void TW_CALL getCB_currNode_posY(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3elem_get(value, sn, &SceneNode::getPosition, 1);
	}
	void TW_CALL getCB_currNode_posZ(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3elem_get(value, sn, &SceneNode::getPosition, 2);
	}

	void TW_CALL setCB_currNode_posX(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3elem_set(value, sn, &SceneNode::setPosition, &SceneNode::getPosition, 0);
	}
	void TW_CALL setCB_currNode_posY(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3elem_set(value, sn, &SceneNode::setPosition, &SceneNode::getPosition, 1);
	}
	void TW_CALL setCB_currNode_posZ(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3elem_set(value, sn, &SceneNode::setPosition, &SceneNode::getPosition, 2);
	}
	////////////////////////////////////////////////////////////////////////
	void TW_CALL getCB_currNode_oriX(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatQuatElem_get(value, sn, &SceneNode::getOrientation, 0);
	}
	void TW_CALL getCB_currNode_oriY(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatQuatElem_get(value, sn, &SceneNode::getOrientation, 1);
	}
	void TW_CALL getCB_currNode_oriZ(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatQuatElem_get(value, sn, &SceneNode::getOrientation, 2);
	}
	void TW_CALL getCB_currNode_oriW(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatQuatElem_get(value, sn, &SceneNode::getOrientation, 3);
	}

	void TW_CALL setCB_currNode_oriX(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatQuatElem_set(value, sn, &SceneNode::setOrientation, &SceneNode::getOrientation, 0);
	}
	void TW_CALL setCB_currNode_oriY(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatQuatElem_set(value, sn, &SceneNode::setOrientation, &SceneNode::getOrientation, 1);
	}
	void TW_CALL setCB_currNode_oriZ(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatQuatElem_set(value, sn, &SceneNode::setOrientation, &SceneNode::getOrientation, 2);
	}
	void TW_CALL setCB_currNode_oriW(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatQuatElem_set(value, sn, &SceneNode::setOrientation, &SceneNode::getOrientation, 3);
	}
	void TW_CALL getCB_currNode_oriXangle(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3byQuat_get(value, sn, &SceneNode::getOrientation, 0);
	}
	void TW_CALL getCB_currNode_oriYangle(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3byQuat_get(value, sn, &SceneNode::getOrientation, 1);
	}
	void TW_CALL getCB_currNode_oriZangle(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3byQuat_get(value, sn, &SceneNode::getOrientation, 2);
	}

	void TW_CALL setCB_currNode_oriXangle(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3byQuat_set(value, sn, &SceneNode::setOrientation, &SceneNode::getOrientation, 0);
	}
	void TW_CALL setCB_currNode_oriYangle(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3byQuat_set(value, sn, &SceneNode::setOrientation, &SceneNode::getOrientation, 1);
	}
	void TW_CALL setCB_currNode_oriZangle(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3byQuat_set(value, sn, &SceneNode::setOrientation, &SceneNode::getOrientation, 2);
	}	
	////////////////////////////////////////////////////////////////////////
	void TW_CALL getCB_currNode_scale(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3elem_get(value, sn, &SceneNode::getScaling, 0);
	}
	void TW_CALL getCB_currNode_scaleX(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3elem_get(value, sn, &SceneNode::getScaling, 0);
	}
	void TW_CALL getCB_currNode_scaleY(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3elem_get(value, sn, &SceneNode::getScaling, 1);
	}
	void TW_CALL getCB_currNode_scaleZ(void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3elem_get(value, sn, &SceneNode::getScaling, 2);
	}
	void TW_CALL setCB_currNode_scale(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		glm::vec3 newscale(*(static_cast<const float*>(value)));
		sn->setScaling(newscale);
	}
	void TW_CALL setCB_currNode_scaleX(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3elem_set(value, sn, &SceneNode::setScaling, &SceneNode::getScaling, 0);
	}
	void TW_CALL setCB_currNode_scaleY(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3elem_set(value, sn, &SceneNode::setScaling, &SceneNode::getScaling, 1);
	}
	void TW_CALL setCB_currNode_scaleZ(const void *value, void *clientData) {
		SceneNode *sn = static_cast<Scene*>(clientData)->getActiveNode();
		guiSceneNodeFloatVec3elem_set(value, sn, &SceneNode::setScaling, &SceneNode::getScaling, 2);
	}
	////////////////////////////////////////////////////////////////////////
	/*void TW_CALL getCB_currCameraLabel(void *value, void *clientData)  {
		Scene *scene = Sandbox::getActiveScene();
		SceneNode *sn = scene->getActiveCamera();
		std::string s = sn->getLabel();
		std::string *destPtr = static_cast<std::string *>(value);
		TwCopyStdStringToLibrary(*destPtr, s);
	}*/
	void TW_CALL getCB_currNodeLabel(void *value, void *clientData)  {
		Scene *scene = Sandbox::getActiveScene();
		SceneNode *sn = scene->getActiveNode();
		std::string s = sn->getLabel();
		std::string *destPtr = static_cast<std::string *>(value);
		TwCopyStdStringToLibrary(*destPtr, s);
	}
	void TW_CALL btnCB_setCurrNodeLabel(void *clientData)  {
		Scene *scene = Sandbox::getActiveScene();
		SceneNode *sn = scene->getActiveNode();

		auto nncd = static_cast<NewNodeCallbackData*>(clientData);
		sn->setLabel(*nncd->currentNodeNewLabel);
		*nncd->currentNodeNewLabel = "";
		Gui::updateSceneBars();
	}
	void TW_CALL btnCB_setCurrNodeParent(void *clientData)  {
		Scene *scene = Sandbox::getActiveScene();
		SceneNode *sn = scene->getActiveNode();

		auto nncd = static_cast<NewNodeCallbackData*>(clientData);
		
		std::string *lbl = nncd->currentNodeNewParentLabel;
		if (SceneNode *node = Sandbox::getActiveScene()->getNodeByLabel(*lbl)) {
			sn->setParent(node);
			*nncd->currentNodeNewParentLabel = "";
			Gui::updateSceneBars();
		}		
	}

	void TW_CALL getCB_currNodeParentLabel(void *value, void *clientData)  {
		Scene *scene = Sandbox::getActiveScene();
		SceneNode *sn = scene->getActiveNode()->getParent();
		std::string s;
		if (sn != nullptr) {
			s = sn->getLabel();
		} else {
			s = "";
		}
		std::string *destPtr = static_cast<std::string *>(value);
		TwCopyStdStringToLibrary(*destPtr, s);
	}
	
	void TW_CALL btnCB_addAssetNode(void *clientData) { 		
		auto nncd = static_cast<NewNodeCallbackData*>(clientData);
		Asset *a = Sandbox::loadAsset("Rubik/Rubik's cube.obj");
		AssetNode *nn = new AssetNode(*nncd->newNodeName, a);
		
		Scene *s = Sandbox::getActiveScene();	
		nn->setPosition(s->getActiveCamera()->getPosition());
		nn->setOrientation(s->getActiveCamera()->getOrientation());
		nn->setScaling(glm::vec3(0.01));

		SceneNode *father = s->getNodeByLabel(*nncd->parentNodeName);
		s->addAssetNode(nn, father);
		
		Gui::updateSceneBars();
	}
	void TW_CALL btnCB_addCameraNode(void *clientData) { 
		auto nncd = static_cast<NewNodeCallbackData*>(clientData);		
		CameraNode *nn = new CameraNode(*nncd->newNodeName);

		Scene *s = Sandbox::getActiveScene();		
		nn->setPosition(s->getActiveCamera()->getPosition());
		nn->setOrientation(s->getActiveCamera()->getOrientation());
		nn->setAspectRatio(s->getActiveCamera()->getAspectRatio());

		SceneNode *father = s->getNodeByLabel(*nncd->parentNodeName);
		s->addCameraNode(nn, father);
		
		Gui::updateSceneBars();
	}
	void TW_CALL btnCB_addFpsCameraNode(void *clientData) { 
		auto nncd = static_cast<NewNodeCallbackData*>(clientData);		
		FpsCameraNode *nn = new FpsCameraNode(*nncd->newNodeName);

		Scene *s = Sandbox::getActiveScene();		
		nn->setPosition(s->getActiveCamera()->getPosition());
		nn->setOrientation(s->getActiveCamera()->getOrientation());
		nn->setAspectRatio(s->getActiveCamera()->getAspectRatio());

		SceneNode *father = s->getNodeByLabel(*nncd->parentNodeName);
		s->addCameraNode(nn, father);
		
		Gui::updateSceneBars();
	}
	void TW_CALL btnCB_addLightNode(void *clientData) { 
		auto nncd = static_cast<NewNodeCallbackData*>(clientData);		
		LightNode *nn = new LightNode(*nncd->newNodeName);
		
		Scene *s = Sandbox::getActiveScene();
		nn->setPosition(s->getActiveCamera()->getPosition());
		nn->setOrientation(s->getActiveCamera()->getOrientation());

		SceneNode *father = s->getNodeByLabel(*nncd->parentNodeName);
		s->addLightNode(nn, father);

		Gui::updateSceneBars();
	}

	void TW_CALL btnCB_setActiveNode(void *clientData) { 
		SceneNode *sn = static_cast<SceneNode*>(clientData);
		Scene *s = Sandbox::getActiveScene();
		s->setActiveNode(sn);	
		Gui::updateSceneBars();
	}	

	void TW_CALL btnCB_remCurrentNode(void *clientData) { 
		//SceneNode *sn = static_cast<SceneNode*>(clientData);
		Scene *s = Sandbox::getActiveScene();
		SceneNode *sn = s->getActiveNode();

		//SceneNode *sn = static_cast<SceneNode*>(clientData);
		//Scene *s = Sandbox::getActiveScene();
		s->remNode(sn, sn->getParent());

	   //SceneNodeCallbackData *cbd =  static_cast<SceneNodeCallbackData*>(clientData);
	   //cbd.scene->setActiveNode(cbd.sceneNode);
	}	
}

using namespace GuiBarScene_ns;

class GuiBarScene : public GuiBar {
public:
	GuiBarScene() :
				GuiBar("Current Scene", 1), 
				newNodeCallbackData(new NewNodeCallbackData(&strNewNodeName, &strNewNodeParentName, &strCurrentNodeNewLabel, &strCurrentNodeNewParentLabel )) {
	} ;
	virtual ~GuiBarScene() {};

	void init() {
		Scene *s = Sandbox::getActiveScene();
		TwRemoveAllVars(m_barHandle);

		TwAddVarCB(m_barHandle, "bgColor", TW_TYPE_COLOR3F, setCB_bgColor, getCB_bgColor, s, " label='Background color' ");

		TwAddVarCB(m_barHandle, "ambient light level",  TW_TYPE_FLOAT,  
			//setCB_fAmbientLightLevel, getCB_fAmbientLightLevel,
			GuiCB::setter<float, Scene, &Scene::setAmbientLightLevel>,
			GuiCB::getter<float, Scene, &Scene::getAmbientLightLevel>,				
			s, " min=0 max=1 step=0.01 ");

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////

		TwAddButton(m_barHandle, "remove selected node", btnCB_remCurrentNode, s, " group='add/rem nodes' ");
		TwAddSeparator(m_barHandle, nullptr, " group='add/rem nodes' ");
		TwAddVarRW(m_barHandle,  "new node label",TW_TYPE_STDSTRING, &strNewNodeName,       " group='add/rem nodes' ");	
		TwAddVarRW(m_barHandle,  "parent label",  TW_TYPE_STDSTRING, &strNewNodeParentName, " group='add/rem nodes' ");	
		//TwAddButton(bar, "add new node", NULL, NULL, "  group='add/rem nodes' ");
		TwAddButton(m_barHandle, "add new AssetNode here",  btnCB_addAssetNode,  newNodeCallbackData.get(), " group='add/rem nodes' ");
		TwAddButton(m_barHandle, "add new LightNode here",  btnCB_addLightNode,  newNodeCallbackData.get(), " group='add/rem nodes' ");
		TwAddButton(m_barHandle, "add new CameraNode here", btnCB_addCameraNode, newNodeCallbackData.get(), " group='add/rem nodes' ");
		TwAddButton(m_barHandle, "add new FpsCameraNode here", btnCB_addFpsCameraNode, newNodeCallbackData.get(), " group='add/rem nodes' ");
		

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////

		TwAddSeparator(m_barHandle, nullptr, "");
		for ( auto a : s->getAssetNodes() ) {
			std::string prefix;
			if ( a == Sandbox::getActiveScene()->getActiveNode() ) prefix += "*";
			if ( a == Sandbox::getActiveScene()->getActiveAsset() ) prefix += ">"; 
			std::string s = prefix + " " + a->getLabel().c_str();			
			TwAddButton(m_barHandle, s.c_str(),  btnCB_setActiveNode, a, " group='AssetNodes' ");
		}
		for ( auto c : s->getCameraNodes() ) {
			std::string prefix;			
			if ( c == Sandbox::getActiveScene()->getActiveNode() ) prefix += "*";
			if ( c == Sandbox::getActiveScene()->getActiveCamera() ) prefix += ">"; 
			std::string s = prefix + " " + c->getLabel().c_str();			
			TwAddButton(m_barHandle, s.c_str(),  btnCB_setActiveNode, c, " group='CameraNodes' ");		
		}
		for ( auto l : s->getLightNodes() ) {
			std::string prefix;			
			if ( l == Sandbox::getActiveScene()->getActiveNode() ) prefix += "*";
			if ( l == Sandbox::getActiveScene()->getActiveLight() ) prefix += ">"; 
			std::string s = prefix + " " + l->getLabel().c_str();			
			TwAddButton(m_barHandle, s.c_str(),  btnCB_setActiveNode, l, " group='LightNodes' ");		
		}		

		TwAddSeparator(m_barHandle, nullptr, "");
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////

		
		TwAddVarCB(m_barHandle, "label",  TW_TYPE_STDSTRING, nullptr, getCB_currNodeLabel,       s, "group='currentNode'");
		TwAddVarCB(m_barHandle, "parent", TW_TYPE_STDSTRING, nullptr, getCB_currNodeParentLabel, s, "group='currentNode'");
		TwAddVarCB(m_barHandle, "orientation", TW_TYPE_QUAT4F, setCB_currNode_ori, getCB_currNode_ori, s, "group='currentNode'");
		TwAddVarCB(m_barHandle, "direction", TW_TYPE_DIR3F,  nullptr,              getCB_currNode_dir, s, "group='currentNode'");

		TwAddVarCB(m_barHandle, "posX",   TW_TYPE_FLOAT,	setCB_currNode_posX,   getCB_currNode_posX, s, "group='currentNode' step=0.1"); 
		TwAddVarCB(m_barHandle, "posY",   TW_TYPE_FLOAT,	setCB_currNode_posY,   getCB_currNode_posY, s, "group='currentNode' step=0.1"); 
		TwAddVarCB(m_barHandle, "posZ",   TW_TYPE_FLOAT,	setCB_currNode_posZ,   getCB_currNode_posZ, s, "group='currentNode' step=0.1"); 
	
		TwAddVarCB(m_barHandle, "scaleX", TW_TYPE_FLOAT,	setCB_currNode_scaleX, getCB_currNode_scaleX, s, "group='currentNode' step=0.01"); 
		TwAddVarCB(m_barHandle, "scaleY", TW_TYPE_FLOAT,	setCB_currNode_scaleY, getCB_currNode_scaleY, s, "group='currentNode' step=0.01"); 
		TwAddVarCB(m_barHandle, "scaleZ", TW_TYPE_FLOAT,	setCB_currNode_scaleZ, getCB_currNode_scaleZ, s, "group='currentNode' step=0.01"); 
	
		TwAddVarCB(m_barHandle, "oriQuatX", TW_TYPE_FLOAT,	setCB_currNode_oriX, getCB_currNode_oriX, s, "group='currentNode' step=0.01"); 
		TwAddVarCB(m_barHandle, "oriQuatY", TW_TYPE_FLOAT,	setCB_currNode_oriY, getCB_currNode_oriY, s, "group='currentNode' step=0.01"); 
		TwAddVarCB(m_barHandle, "oriQuatZ", TW_TYPE_FLOAT,	setCB_currNode_oriZ, getCB_currNode_oriZ, s, "group='currentNode' step=0.01"); 
		TwAddVarCB(m_barHandle, "oriQuatW", TW_TYPE_FLOAT,	setCB_currNode_oriW, getCB_currNode_oriW, s, "group='currentNode' step=0.01"); 
	
		TwAddVarCB(m_barHandle, "oriAngleX", TW_TYPE_FLOAT,	setCB_currNode_oriXangle, getCB_currNode_oriXangle, s, "group='currentNode' step=0.1"); 
		TwAddVarCB(m_barHandle, "oriAngleY", TW_TYPE_FLOAT,	setCB_currNode_oriYangle, getCB_currNode_oriYangle, s, "group='currentNode' step=0.1"); 
		TwAddVarCB(m_barHandle, "oriAngleZ", TW_TYPE_FLOAT,	setCB_currNode_oriZangle, getCB_currNode_oriZangle, s, "group='currentNode' step=0.1"); 		
	
		//TwAddVarCB(m_barHandle, "scaleUniformly", TW_TYPE_FLOAT,	setCB_currNode_scale, getCB_currNode_scale, s, "group='currentNode' min=0.01 max=100 step=0.01"); 

		TwAddVarRW(m_barHandle,  "new label",        TW_TYPE_STDSTRING,      &strCurrentNodeNewLabel,   " group='set label'");
		TwAddButton(m_barHandle, "apply new label",  btnCB_setCurrNodeLabel, newNodeCallbackData.get(), " group='set label'");

		TwAddVarRW(m_barHandle,  "new parent",        TW_TYPE_STDSTRING,      &strCurrentNodeNewParentLabel,   " group='set parent'");
		TwAddButton(m_barHandle, "apply new parent",  btnCB_setCurrNodeParent, newNodeCallbackData.get(), " group='set parent'");

	};
private:
	std::string strNewNodeName;
	std::string strNewNodeParentName;

	std::string strCurrentNodeNewLabel;
	std::string strCurrentNodeNewParentLabel;

	std::shared_ptr<NewNodeCallbackData> newNodeCallbackData;

};

#endif