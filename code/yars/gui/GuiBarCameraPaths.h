#ifndef GUIBARCAMERAPATHS_H
#define GUIBARCAMERAPATHS_H

#include "GuiBar.h"


#include "../Sandbox.h"

#include "../CameraNode.h"
#include "../CameraPath.h"

#include <vector>
#include <memory>

#include <sstream>

//class AssetNode;

namespace GuiBarCameraPaths_ns {
	class KnotCallbackData {
		public:
			KnotCallbackData(CameraPath *camp, int i) : cp(camp),pos(i) { }
			~KnotCallbackData() { }
		
			CameraPath *cp;
			int pos;
	};

	typedef  void  (CameraPath::*CameraPathFloatSet)(float val);
	typedef  float (CameraPath::*CameraPathFloatGet)(void) const;	

	void guiCamPathFloatCB_set(const void *value, void *clientData, CameraPathFloatSet m) {	
		(static_cast<CameraPath*>(clientData)->*m)(*(static_cast<const float*>(value)));		
	}
	void guiCamPathFloatCB_get(void *value, void *clientData, CameraPathFloatGet m) {
		float *dest = static_cast<float*>(value);
		*dest = (static_cast<CameraPath*>(clientData)->*m)();
	}
	

	void TW_CALL btnCB_campath_profile(void *clientData)  { 
		Gui::hide();
		Sandbox::getActiveScene()->startPlayingCameraPath();
	}
	void TW_CALL btnCB_campath_play(void *clientData)  { 
		Sandbox::getActiveScene()->startPlayingCameraPath();
	}
	void TW_CALL btnCB_campath_stop(void *clientData)  { 
		Sandbox::getActiveScene()->stopPlayingCameraPath();
	}
	
	void TW_CALL btnCB_campath_setActive(void *clientData) {
		CameraPath *cp = static_cast<CameraPath*>(clientData);
		Sandbox::getActiveScene()->setActiveCamPath(cp);
		Gui::updateSceneBars();
	} 
	
	void TW_CALL btnCB_campath_addKnot(void *clientData)  { 
		Scene *s = Sandbox::getActiveScene();
		CameraNode *cn = s->getActiveCamera();		
		s->getActiveCamPath()->addKnot(cn->getPosition(), cn->getOrientation());
		Gui::updateSceneBars();
	}
	void TW_CALL btnCB_campath_addKnotAfter(void *clientData)  { 
		Scene *s = Sandbox::getActiveScene();
		CameraNode *cn = s->getActiveCamera();			
		s->getActiveCamPath()->addKnotAfter(cn->getPosition(), cn->getOrientation());
		Gui::updateSceneBars();
	}
	void TW_CALL btnCB_campath_remKnot(void *clientData)  { 
		Scene *s = Sandbox::getActiveScene();
		CameraPath *cp = s->getActiveCamPath();
		int knotpos = cp->getActiveKnot();
		if (knotpos!=-1) {
			cp->remKnot(knotpos);
			Gui::updateSceneBars();
		}
	}
	void TW_CALL btnCB_campath_editKnot(void *clientData) {
		Scene *s = Sandbox::getActiveScene();
		CameraNode *cn = s->getActiveCamera();
		CameraPath *cp = s->getActiveCamPath();
		int knotpos = cp->getActiveKnot();
		if (knotpos!=-1) {
			cp->setKnot(cp->getActiveKnot(), cn->getPosition(), cn->getOrientation());
		}
	}

	void TW_CALL btnCB_campath_setActiveKnot(void *clientData) {
		KnotCallbackData *kcd = static_cast<KnotCallbackData*>(clientData);
			
		Scene *s = Sandbox::getActiveScene();		
		CameraPath *cp = kcd->cp;
		int knotpos = kcd->pos;

		s->setActiveCamPath(cp);		
		cp->setActiveKnot(knotpos);
		s->getActiveCamera()->setPosition(cp->getKnots()[knotpos].pos());
		s->getActiveCamera()->setOrientation(cp->getKnots()[knotpos].ori());
		Gui::updateSceneBars();
	}
	

	void TW_CALL setCB_campath_duration(const void *value, void *clientData)     { guiCamPathFloatCB_set(value, clientData, &CameraPath::setDuration);  }
	void TW_CALL getCB_campath_duration(void *value, void *clientData)           { guiCamPathFloatCB_get(value, clientData, &CameraPath::getDuration);  }
	
	//void TW_CALL setCB_campath_looping(const void *value, void *clientData)     { guiCamPathBoolCB_set(value, clientData, &CameraPath::setLooping);  }
	//void TW_CALL getCB_campath_looping(void *value, void *clientData)           { guiCamPathBoolCB_get(value, clientData, &CameraPath::isLooping);  }
	
	void TW_CALL setCB_campath_looping(const void *value, void *clientData)  { 
		/*CameraPath *cp = Sandbox::getActiveScene()->getActiveCamPath();
		if (cp != nullptr) {
			cp->setLooping(*(static_cast<const bool*>(value)));				
		}*/		
		CameraPathWalker& cpw = Sandbox::getActiveScene()->getCameraPathWalker();
		cpw.setLooping(*(static_cast<const bool*>(value)));
	}
	void TW_CALL getCB_campath_looping(void *value, void *clientData) {
		const CameraPathWalker& cpw = Sandbox::getActiveScene()->getCameraPathWalker();
		bool val = cpw.isLooping();
		bool *dest = static_cast<bool*>(value);
		*dest = val;		
		/*CameraPath *cp = Sandbox::getActiveScene()->getActiveCamPath();
		if (cp != nullptr) {
			bool val = cp->isLooping();			
			bool *dest = static_cast<bool*>(value);
			*dest = val;		
		}*/
	}
	
	void TW_CALL setCB_campath_paused(const void *value, void *clientData)  { 
		CameraPathWalker& cpw = Sandbox::getActiveScene()->getCameraPathWalker();
		bool val = *(static_cast<const bool*>(value));
		cpw.setPaused(val);
		/*
		CameraPath *cp = Sandbox::getActiveScene()->getActiveCamPath();
		if (cp != nullptr) {
			cp->setPaused(*(static_cast<const bool*>(value)));				
		}
		*/
	}
	void TW_CALL getCB_campath_paused(void *value, void *clientData) {
		CameraPathWalker& cpw = Sandbox::getActiveScene()->getCameraPathWalker();
		bool val = cpw.isPaused();
		bool *dest = static_cast<bool*>(value);
		*dest = val;
		/*
		CameraPath *cp = Sandbox::getActiveScene()->getActiveCamPath();
		if (cp != nullptr) {
			bool val = cp->isPaused();			
			bool *dest = static_cast<bool*>(value);
			*dest = val;		
		}
		*/
	}
	

	void TW_CALL btnCB_addEmptyPath(void *clientData) {
		Sandbox::getActiveScene()->addCameraPath(new CameraPath()); 
		Gui::updateSceneBars();
	}
	
	void TW_CALL btnCB_addPathByCams(void *clientData) {
		Sandbox::getActiveScene()->addCameraPathByCameras(); 
		Gui::updateSceneBars();
	}
	
	void TW_CALL btnCB_remCurrPath(void *clientData) { 
		Sandbox::getActiveScene()->remCameraPath(Sandbox::getActiveScene()->getActiveCamPath());
		Gui::updateSceneBars();
	}	

}


class GuiBarCameraPaths : public GuiBar {
public:
	GuiBarCameraPaths() : GuiBar("Camera Paths", 2),loop(false) {} ;
	virtual ~GuiBarCameraPaths() {};

	void init() {
		TwRemoveAllVars(m_barHandle);

		knotChangeData.clear();

		std::string options = "";


		auto cameraPaths = Sandbox::getActiveScene()->getCameraPaths();
		for( auto cp : cameraPaths)  {
			std::string lbl = cp->getLabel();

			if (Sandbox::getActiveScene()->getActiveCamPath() == cp.get())  {
				lbl = "> " + lbl;
			}				
			TwAddButton(m_barHandle, lbl.c_str(),  GuiBarCameraPaths_ns::btnCB_campath_setActive,  cp.get(),  options.c_str());					
		}
		
		if ( cameraPaths.size() > 0 ) {
			TwAddSeparator(m_barHandle, nullptr, "");

			TwAddButton(m_barHandle, "play path (profiling)",        GuiBarCameraPaths_ns::btnCB_campath_profile,  nullptr,  options.c_str());
			TwAddButton(m_barHandle, "play",        GuiBarCameraPaths_ns::btnCB_campath_play,  nullptr,  options.c_str());
			TwAddButton(m_barHandle, "stop",        GuiBarCameraPaths_ns::btnCB_campath_stop,  nullptr,  options.c_str());
			//TwAddButton(m_barHandle, "pause",       GuiBarCameraPaths_ns::btnCB_campath_pause,  nullptr,  options.c_str());
			//TwAddVarRW(m_barHandle, "loop", TW_TYPE_BOOLCPP, &loop, options.c_str()); 
			TwAddVarCB(m_barHandle, "paused", TW_TYPE_BOOLCPP, GuiBarCameraPaths_ns::setCB_campath_paused, GuiBarCameraPaths_ns::getCB_campath_paused , nullptr, options.c_str()); 
			TwAddVarCB(m_barHandle, "loop", TW_TYPE_BOOLCPP, GuiBarCameraPaths_ns::setCB_campath_looping, GuiBarCameraPaths_ns::getCB_campath_looping , nullptr, options.c_str()); 
		}

		TwAddSeparator(m_barHandle, nullptr, "");

		TwAddButton(m_barHandle, "add empty path",        GuiBarCameraPaths_ns::btnCB_addEmptyPath,  nullptr,  options.c_str());
		TwAddButton(m_barHandle, "add path from cameras", GuiBarCameraPaths_ns::btnCB_addPathByCams, nullptr,  options.c_str());

		if ( cameraPaths.size() > 0 ) {
			TwAddButton(m_barHandle, "rem current path",      GuiBarCameraPaths_ns::btnCB_remCurrPath,   nullptr,  options.c_str());
		
			TwAddSeparator(m_barHandle, nullptr, "");

			TwAddButton(m_barHandle, "add new knot", GuiBarCameraPaths_ns::btnCB_campath_addKnot,  nullptr,  options.c_str());
			TwAddButton(m_barHandle, "add new knot after selected", GuiBarCameraPaths_ns::btnCB_campath_addKnotAfter, nullptr,  options.c_str());
			TwAddButton(m_barHandle, "rem selected knot",           GuiBarCameraPaths_ns::btnCB_campath_remKnot,  nullptr,  options.c_str());
			TwAddButton(m_barHandle, "set selected knot by camera", GuiBarCameraPaths_ns::btnCB_campath_editKnot, nullptr,  options.c_str());			
		
		}

		TwAddSeparator(m_barHandle, nullptr, "");
		
		for(int i=0; i < cameraPaths.size(); ++i)  {

			CameraPath *cp = cameraPaths.at(i).get();
			std::string s = cp->getLabel();

			///
			std::ostringstream oss_cpid;
			oss_cpid << "CP_" << i;
		
			std::string varname;
			std::string options;
			std::string groupname = oss_cpid.str() + " " + s;
									
			varname = oss_cpid.str() + "/duration";
			options = " group='" + groupname + "' min=0.1 max=60 step=0.1 ";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	GuiBarCameraPaths_ns::setCB_campath_duration, GuiBarCameraPaths_ns::getCB_campath_duration, cp, options.c_str()); 

			options = " group='" + groupname + "' ";			
			
			int knot_i = 0;
			for (auto k : cp->getKnots()) {
				std::string lbl = oss_cpid.str() + "/knot" + std::to_string(knot_i);
				if (Sandbox::getActiveScene()->getActiveCamPath() == cp && knot_i == cp->getActiveKnot()) lbl = "> " + lbl;

				auto callbackDataPtr = std::shared_ptr<GuiBarCameraPaths_ns::KnotCallbackData>(
					new GuiBarCameraPaths_ns::KnotCallbackData(cp, knot_i) 
				);
				knotChangeData.push_back(callbackDataPtr);

				TwAddButton(m_barHandle, lbl.c_str(), GuiBarCameraPaths_ns::btnCB_campath_setActiveKnot,  callbackDataPtr.get(),  options.c_str());
				knot_i++;
			}
			///
			/*
			varname = oss_cpid.str() + "/posX";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	nullptr,  GuiBarCameraNodes_ns::getCB_cam_posX, cn, options.c_str()); 

			varname = oss_cpid.str() + "/posY";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	nullptr,  GuiBarCameraNodes_ns::getCB_cam_posY, cn, options.c_str()); 

			varname = oss_cpid.str() + "/posZ";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	nullptr,  GuiBarCameraNodes_ns::getCB_cam_posZ, cn, options.c_str()); 
			
			varname = oss_cpid.str() + "/orientation";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_QUAT4F, nullptr, GuiBarCameraNodes_ns::getCB_cam_ori, cn, options.c_str());
			
			varname = oss_cpid.str() + "/lookAtDir";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_DIR3F, nullptr, GuiBarCameraNodes_ns::getCB_cam_dir, cn, options.c_str());
						
			options = " group='" + groupname + "' min=0 max=200 step=0.1";
			varname = oss_cpid.str() + "/farClip";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	GuiBarCameraNodes_ns::setCB_cam_farClip, GuiBarCameraNodes_ns::getCB_cam_farClip, cn, options.c_str()); 

			varname = oss_cpid.str() + "/nearClip";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT, GuiBarCameraNodes_ns::setCB_cam_nearClip, GuiBarCameraNodes_ns::getCB_cam_nearClip, cn, options.c_str()); 

			varname = oss_cpid.str() + "/FOV";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT, GuiBarCameraNodes_ns::setCB_cam_fov, GuiBarCameraNodes_ns::getCB_cam_fov, cn, options.c_str()); 
			
			
			options = " group='" + groupname + "' min=0 max=3 step=0.01";
			varname = oss_cpid.str() + "/aspectRatio";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT, GuiBarCameraNodes_ns::setCB_cam_aspectRatio, GuiBarCameraNodes_ns::getCB_cam_aspectRatio, cn, options.c_str()); 

			options = " group='" + groupname + "' ";
			varname = oss_cpid.str() + "/autoUpdateAspectRatio";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_BOOLCPP, GuiBarCameraNodes_ns::setCB_cam_autoUpdateAspectRatio, GuiBarCameraNodes_ns::getCB_cam_autoUpdateAspectRatio, cn, options.c_str()); 
			*/

			///
		}
	};
private:
	//std::vector< std::shared_ptr<GuiBarAssetNodes_ns::AssetChangeCallbackData> > assetChangeData;
	std::vector< std::shared_ptr<GuiBarCameraPaths_ns::KnotCallbackData> > knotChangeData;
	bool loop;
};

#endif