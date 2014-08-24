#ifndef GUIBARCAMERANODES_H
#define GUIBARCAMERANODES_H

#include "GuiBar.h"

#include "../CameraNode.h"

#include <vector>
#include <memory>

#include <sstream>

//class AssetNode;

namespace GuiBarCameraNodes_ns {
	typedef  void  (CameraNode::*CameraFloatVec3Set)(glm::vec3 val);
	typedef  const glm::vec3 (CameraNode::*CameraFloatVec3Get)(void) const;

	void guiCamFloatVec3elem_get(void *value, void *clientData, CameraFloatVec3Get m, int elemPos) {
		glm::vec3 val = (*(static_cast<CameraNode*>(clientData)).*m)();	
		float *dest = static_cast<float*>(value);
		*dest = val[elemPos];
	}
	///////////////////////////////////////////////////////////////////////////////
	// cam pos
	void TW_CALL getCB_cam_posX(void *value, void *clientData) { guiCamFloatVec3elem_get(value, clientData, &CameraNode::getPosition, 0); }
	void TW_CALL getCB_cam_posY(void *value, void *clientData) { guiCamFloatVec3elem_get(value, clientData, &CameraNode::getPosition, 1); }
	void TW_CALL getCB_cam_posZ(void *value, void *clientData) { guiCamFloatVec3elem_get(value, clientData, &CameraNode::getPosition, 2); }

	// cam ori
	void TW_CALL getCB_cam_ori(void *value, void *clientData)  {
		glm::quat ori = static_cast<CameraNode*>(clientData)->getOrientation(); 

		float *dest = static_cast<float*>(value);
		dest[0] = ori.x;
		dest[1] = ori.y;
		dest[2] = ori.z;
		dest[3] = ori.w;
	}

	void TW_CALL getCB_cam_dir(void *value, void *clientData) {
		glm::quat ori = static_cast<CameraNode*>(clientData)->getOrientation(); 
		glm::mat3 orim = glm::toMat3(ori);
		glm::vec3 dir =  -orim[2];

		float *dest = static_cast<float*>(value);
		dest[0] = dir.x;
		dest[1] = dir.y;
		dest[2] = dir.z;	
	}
}


class GuiBarCameraNodes : public GuiBar {
public:
	GuiBarCameraNodes() : GuiBar("Camera Nodes", 2) {} ;
	virtual ~GuiBarCameraNodes() {};

	void init() {
		TwRemoveAllVars(m_barHandle);

		auto cameraNodes = Sandbox::getActiveScene()->getCameraNodes();

		for(int i=0; i < cameraNodes.size(); ++i)  {

			CameraNode *cn = cameraNodes.at(i);
			std::string s = cn->getLabel();

			///
			std::ostringstream oss_cameraid;
			oss_cameraid << "C_" << i;
		
			std::string varname;
			std::string groupname = oss_cameraid.str() + " " + s;
			
			std::string options = " group='" + groupname + "' ";
						
			///
			varname = oss_cameraid.str() + "/posX";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	nullptr,  GuiBarCameraNodes_ns::getCB_cam_posX, cn, options.c_str()); 

			varname = oss_cameraid.str() + "/posY";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	nullptr,  GuiBarCameraNodes_ns::getCB_cam_posY, cn, options.c_str()); 

			varname = oss_cameraid.str() + "/posZ";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	nullptr,  GuiBarCameraNodes_ns::getCB_cam_posZ, cn, options.c_str()); 
			
			varname = oss_cameraid.str() + "/orientation";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_QUAT4F, nullptr, GuiBarCameraNodes_ns::getCB_cam_ori, cn, options.c_str());
			
			varname = oss_cameraid.str() + "/lookAtDir";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_DIR3F, nullptr, GuiBarCameraNodes_ns::getCB_cam_dir, cn, options.c_str());
						
			options = " group='" + groupname + "' min=0 max=200 step=0.1";
			varname = oss_cameraid.str() + "/farClip";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	
				//GuiBarCameraNodes_ns::setCB_cam_farClip, GuiBarCameraNodes_ns::getCB_cam_farClip,
				GuiCB::setter<float, CameraNode, &CameraNode::setFarClipDist>,
				GuiCB::getter<float, CameraNode, &CameraNode::getFarClipDist>,
				cn, options.c_str()); 

			varname = oss_cameraid.str() + "/nearClip";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,
				//GuiBarCameraNodes_ns::setCB_cam_nearClip, GuiBarCameraNodes_ns::getCB_cam_nearClip, 
				GuiCB::setter<float, CameraNode, &CameraNode::setNearClipDist>,
				GuiCB::getter<float, CameraNode, &CameraNode::getNearClipDist>,				
				cn, options.c_str()); 

			varname = oss_cameraid.str() + "/FOV";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT, 
				//GuiBarCameraNodes_ns::setCB_cam_fov, GuiBarCameraNodes_ns::getCB_cam_fov,			
				GuiCB::setter<float, CameraNode, &CameraNode::setFOV>,
				GuiCB::getter<float, CameraNode, &CameraNode::getFOV>,
				cn, options.c_str()); 
			
			
			options = " group='" + groupname + "' min=0 max=3 step=0.01";
			varname = oss_cameraid.str() + "/aspectRatio";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT, 
				//GuiBarCameraNodes_ns::setCB_cam_aspectRatio, GuiBarCameraNodes_ns::getCB_cam_aspectRatio, 
				GuiCB::setter<float, CameraNode, &CameraNode::setAspectRatio>,
				GuiCB::getter<float, CameraNode, &CameraNode::getAspectRatio>,				
				cn, options.c_str()); 

			options = " group='" + groupname + "' ";
			varname = oss_cameraid.str() + "/autoUpdateAspectRatio";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_BOOLCPP, 
				//GuiBarCameraNodes_ns::setCB_cam_autoUpdateAspectRatio, GuiBarCameraNodes_ns::getCB_cam_autoUpdateAspectRatio, 
				GuiCB::setter<bool, CameraNode, &CameraNode::setAutoUpdateAspectRatio>,
				GuiCB::getter<bool, CameraNode, &CameraNode::getAutoUpdateAspectRatio>,
				cn, options.c_str()); 

			///
		}
	};
private:
	//std::vector< std::shared_ptr<GuiBarAssetNodes_ns::AssetChangeCallbackData> > assetChangeData;
};

#endif