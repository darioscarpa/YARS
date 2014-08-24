#ifndef GUIBARASSETNODES_H
#define GUIBARASSETNODES_H

#include "GuiBar.h"

#include "../Scene.h"
#include "../Asset.h"
#include "../Renderable.h"
#include "../AssetNode.h"
#include "../Sandbox.h"

#include <sstream>
#include <vector>
#include <memory>

class AssetNode;

namespace GuiBarAssetNodes_ns {

	typedef  void  (Asset::*AssetFloatVec3Set)(glm::vec3 val);
	typedef  const glm::vec3 (Asset::*AssetFloatVec3Get)(void) const;


	class AssetChangeCallbackData {
		public:
			AssetChangeCallbackData(const std::string& file, AssetNode *n) : filename(file), node(n) { }
			~AssetChangeCallbackData() { }
		
			 std::string filename;
			 AssetNode   * const node;
	};

	void TW_CALL btnCB_asset_change(void *clientData) {  
		AssetChangeCallbackData *cbdata = static_cast<AssetChangeCallbackData*>(clientData);
		try {
			Asset *a = Sandbox::loadAsset(cbdata->filename);
			cbdata->node->attach(a);
			//cbdata->node->setScaling(glm::vec3(a->getScaleFactor()));

		} catch (std::exception &e) {
			printf(e.what());
		}		
	}
	void TW_CALL getCB_str_currentFilename(void *value, void *clientData)  {
		AssetNode *an = static_cast<AssetNode*>(clientData);
		std::string s = an->getAttached()->getLabel();
		std::string *destPtr = static_cast<std::string *>(value);
		TwCopyStdStringToLibrary(*destPtr, s);
	}

	void guiAssetFloatVec3elem_get(void *value, void *clientData, AssetFloatVec3Get m, int elemPos) {
		glm::vec3 val = (*(static_cast<Asset*>(clientData)).*m)();	
		float *dest = static_cast<float*>(value);
		*dest = val[elemPos];
	}

	void TW_CALL getCB_asset_sizeX(void *value, void *clientData) { guiAssetFloatVec3elem_get(value, clientData, &Asset::getBoundingBoxSize, 0); }
	void TW_CALL getCB_asset_sizeY(void *value, void *clientData) { guiAssetFloatVec3elem_get(value, clientData, &Asset::getBoundingBoxSize, 1); }
	void TW_CALL getCB_asset_sizeZ(void *value, void *clientData) { guiAssetFloatVec3elem_get(value, clientData, &Asset::getBoundingBoxSize, 2); }

}


class GuiBarAssetNodes : public GuiBar {
public:
	GuiBarAssetNodes() : GuiBar("Asset Nodes", 2) {} ;
	virtual ~GuiBarAssetNodes() {};

	void init() {
		TwRemoveAllVars(m_barHandle);

		assetChangeData.clear();
	
		auto assetNodes = Sandbox::getActiveScene()->getAssetNodes();
		for(int i=0; i < assetNodes.size(); ++i)  {

			AssetNode *an = assetNodes.at(i);
			//std::string s = an->getAttached()->getLabel();
			std::string s = an->getLabel();

			auto callbackDataPtr = std::shared_ptr<GuiBarAssetNodes_ns::AssetChangeCallbackData>(
				new GuiBarAssetNodes_ns::AssetChangeCallbackData(an->getAttached()->getLabel(), an) 
			);
			assetChangeData.push_back(callbackDataPtr);
				
			///
			std::ostringstream oss_assetid;
			oss_assetid << "A_" << i;
		
			std::string varname;
			std::string groupname = oss_assetid.str() + " " + s;
			std::string options = " group='" + groupname + "' ";
				
			///		
			varname = oss_assetid.str() + "/currentFile";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_STDSTRING, nullptr, GuiBarAssetNodes_ns::getCB_str_currentFilename, an , options.c_str()); 
				
			varname = oss_assetid.str() + "/newFile";
			TwAddVarRW(m_barHandle, varname.c_str(), TW_TYPE_STDSTRING, &(callbackDataPtr.get()->filename), options.c_str()); 
		
			varname = oss_assetid.str() + "/change";
			TwAddButton(m_barHandle, varname.c_str(), GuiBarAssetNodes_ns::btnCB_asset_change, assetChangeData[i].get(), options.c_str());

			varname = oss_assetid.str() + "/sizeX";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	nullptr,  GuiBarAssetNodes_ns::getCB_asset_sizeX, an->getAttached(), options.c_str()); 

			varname = oss_assetid.str() + "/sizeY";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	nullptr,  GuiBarAssetNodes_ns::getCB_asset_sizeY, an->getAttached(), options.c_str()); 

			varname = oss_assetid.str() + "/sizeZ";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	nullptr,  GuiBarAssetNodes_ns::getCB_asset_sizeZ, an->getAttached(), options.c_str()); 
		}
	}


private:
	std::vector< std::shared_ptr<GuiBarAssetNodes_ns::AssetChangeCallbackData> > assetChangeData;
};

#endif