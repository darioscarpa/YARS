#ifndef GUIBARLIGHTNODES_H
#define GUIBARLIGHTNODES_H

#include "GuiBar.h"

#include "../LightNode.h"

#include <vector>
#include <memory>

#include <sstream>

namespace GuiBarLightNodes_ns {
	///////////////////////////////////////////////////////////////////////////////
	void TW_CALL setCB_light_color(const void *value, void *clientData) {
		LightNode *ln = static_cast<LightNode*>(clientData);
		float *col = (float*) value;	
		ln->setColor(glm::vec3(col[0], col[1], col[2]));
	}

	void TW_CALL getCB_light_color(void *value, void *clientData) {
		LightNode *ln = static_cast<LightNode*>(clientData);
		glm::vec3 col = ln->getColor();	

		float *dest = (float*) value;
		dest[0] = col.r;
		dest[1] = col.g;
		dest[2] = col.b;
	}	
	///////////////////////////////////////////////////////////////////////////////	
}

using namespace GuiBarLightNodes_ns;

class GuiBarLightNodes : public GuiBar {
public:
	GuiBarLightNodes() : GuiBar("Light Nodes", 2) {} ;
	virtual ~GuiBarLightNodes() {};

	void init() {
		TwRemoveAllVars(m_barHandle);

		auto lightNodes = Sandbox::getActiveScene()->getLightNodes();
		for(int i=0; i < lightNodes.size(); ++i)  {

			LightNode *ln = lightNodes.at(i);
			std::string s = ln->getLabel();

			///
			std::ostringstream oss_lightid;
			oss_lightid << "L_" << i;
		
			std::string varname;
			std::string groupname = oss_lightid.str() + " " + s;
			std::string options = " group='" + groupname + "' ";
						
			///
			
			TwEnumVal lightMode[] =     {
				{ LightNode::POINT,       "point"     }, 
				{ LightNode::SPOT,        "spot"      },	
				{ LightNode::DIRECTIONAL, "directional"}
			};
			TwType lightModeType = TwDefineEnum("LightType", lightMode, 3); 

			varname = oss_lightid.str() + "/type";
			TwAddVarCB(m_barHandle, varname.c_str(), lightModeType, 
				//setCB_light_type, getCB_light_type, 
				GuiCB::setter<LightNode::LightType, LightNode, &LightNode::setType>,
				GuiCB::getter<LightNode::LightType, LightNode, &LightNode::getType>,
				ln, options.c_str());
			

			varname = oss_lightid.str() + "/color";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_COLOR3F,
				setCB_light_color, getCB_light_color,
				//GuiCB::setter<glm::vec3, LightNode, &LightNode::setColor>,
				//GuiCB::getter<glm::vec3, LightNode, &LightNode::getColor>,
				ln, options.c_str()); 


			options = " group='" + groupname + "' min=0 step=0.01";

			varname = oss_lightid.str() + "/spotExponent";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,
				//setCB_light_spotExponent, getCB_light_spotExponent,
				GuiCB::setter<float, LightNode, &LightNode::setSpotExponent>,
				GuiCB::getter<float, LightNode, &LightNode::getSpotExponent>,			
				ln, options.c_str()); 

			options = " group='" + groupname + "' min=0 max=90 step=1.0";
			varname = oss_lightid.str() + "/spotInnerCutoff";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	
				//setCB_light_spotInnerCutoff, getCB_light_spotInnerCutoff, 
				GuiCB::setter<float, LightNode, &LightNode::setSpotInnerCutoff>,
				GuiCB::getter<float, LightNode, &LightNode::getSpotInnerCutoff>,			
				ln, options.c_str()); 

			varname = oss_lightid.str() + "/spotOuterCutoff";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,
				//setCB_light_spotOuterCutoff, getCB_light_spotOuterCutoff,
				GuiCB::setter<float, LightNode, &LightNode::setSpotOuterCutoff>,
				GuiCB::getter<float, LightNode, &LightNode::getSpotOuterCutoff>,
				ln, options.c_str()); 
			
			options = " group='" + groupname + "' min=0 step=0.01";
			varname = oss_lightid.str() + "/radius";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,
				//setCB_light_radius,  getCB_light_radius,
				GuiCB::setter<float, LightNode, &LightNode::setRadius>,
				GuiCB::getter<float, LightNode, &LightNode::getRadius>,
				ln, options.c_str()); 

			varname = oss_lightid.str() + "/constAttenuation";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,
				//setCB_light_constantAttenuation,  getCB_light_constantAttenuation, 
				GuiCB::setter<float, LightNode, &LightNode::setConstantAttenuation>,
				GuiCB::getter<float, LightNode, &LightNode::getConstantAttenuation>,
				ln, options.c_str()); 

			varname = oss_lightid.str() + "/linearAttenuation";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT,	
				//setCB_light_linearAttenuation,  getCB_light_linearAttenuation,
				GuiCB::setter<float, LightNode, &LightNode::setLinearAttenuation>,
				GuiCB::getter<float, LightNode, &LightNode::getLinearAttenuation>,				
				ln, options.c_str()); 
			
			varname = oss_lightid.str() + "/quadraticAttenuation";
			TwAddVarCB(m_barHandle, varname.c_str(), TW_TYPE_FLOAT, 
				//setCB_light_quadraticAttenuation, getCB_light_quadraticAttenuation, 
				GuiCB::setter<float, LightNode, &LightNode::setQuadraticAttenuation>,
				GuiCB::getter<float, LightNode, &LightNode::getQuadraticAttenuation>,								
				ln, options.c_str());								
			///
		}
	};
private:
	
};

#endif