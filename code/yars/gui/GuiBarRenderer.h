#ifndef GUIBARRENDERER_H
#define GUIBARRENDERER_H

#include "GuiBar.h"

#include "../Renderer.h"
#include "../renderers/deferred/DeferredRenderer.h"
#include "../renderers/deferred/DoublePassBlendingDeferredRenderer.h"
#include "../renderers/def_saao/SaAoRenderer.h"
#include "../renderers/def_saaoil/SaAoIlRenderer.h"
#include "../Sandbox.h"

#include "../renderers/shaders/sub/RandomGeneratorShaderSub.h"
#include "../renderers/shaders/sub/SamplingPatternShaderSub.h"
#include "../renderers/shaders/sub/D2DaoShaderSub.h"
#include "../renderers/shaders/sub/D2DilShaderSub.h"

#include <vector>
#include <memory>
#include <map>

#include <sstream>

namespace GuiBarRenderer_ns {	
	void TW_CALL btnCB_reloadShaders(void *clientData) {  
		Renderer *r = static_cast<Renderer*>(clientData);
		try {
			r->reloadShaders();
		} catch (std::exception &e) {
			printf(e.what());
		}		
	}
}

namespace GuiBarDeferredRenderer_ns {	
}

using namespace GuiBarRenderer_ns;
using namespace GuiBarDeferredRenderer_ns;

class GuiBarRenderer : public GuiBar {
public:
	GuiBarRenderer() : GuiBar("Current Renderer", 0){	} ;
	virtual ~GuiBarRenderer() {};

	void init() {
		Renderer *r = Sandbox::getActiveRenderer();
		TwRemoveAllVars(m_barHandle);

		TwAddButton(m_barHandle, "reload shaders", btnCB_reloadShaders, r, "");
		TwAddSeparator(m_barHandle, nullptr, "");

		TwAddVarCB(m_barHandle, "samples", TW_TYPE_INT32,  
			nullptr, 
			GuiCB::getter<int, Renderer, &Renderer::multisamplingSamples>,			
			r,  " group='basic'");
		TwAddVarCB(m_barHandle, "multisampling", TW_TYPE_BOOLCPP,
			GuiCB::setter<bool, Renderer, &Renderer::setMultisampling>,
			GuiCB::getter<bool, Renderer, &Renderer::isMultisamplingEnabled>,
			r, " group='basic'");
		//TwAddVarCB(m_barHandle, "texture filering", TW_TYPE_BOOLCPP, setCB_bTextureFiltering,  getCB_bTextureFiltering,   r, " group='basic'");

		TwAddVarCB(m_barHandle, "textures",  TW_TYPE_BOOLCPP,
			GuiCB::setter<bool, Renderer, &Renderer::setTextureMapping>,
			GuiCB::getter<bool, Renderer, &Renderer::isTextureMappingEnabled>,					
			r, " group='basic'");
		TwAddVarCB(m_barHandle, "normal maps",   TW_TYPE_BOOLCPP, 
			GuiCB::setter<bool, Renderer, &Renderer::setNormalMapping>,
			GuiCB::getter<bool, Renderer, &Renderer::isNormalMappingEnabled>,					
			r, " group='basic'");
		TwAddVarCB(m_barHandle, "specular maps", TW_TYPE_BOOLCPP,
			GuiCB::setter<bool, Renderer, &Renderer::setSpecularMapping>,
			GuiCB::getter<bool, Renderer, &Renderer::isSpecularMappingEnabled>,			
			r, " group='basic'");
		TwAddVarCB(m_barHandle, "opacity maps",  TW_TYPE_BOOLCPP,
			GuiCB::setter<bool, Renderer, &Renderer::setOpacityMapping>,
			GuiCB::getter<bool, Renderer, &Renderer::isOpacityMappingEnabled>,
			r, " group='basic'");

		/*
		TwAddVarCB(m_barHandle, "lights enabled", TW_TYPE_BOOLCPP, 
			//setCB_bLightingEnabled,   getCB_bLightingEnabled,   
			GuiCB::setter<bool, Renderer, &Renderer::setLightingEnabled>,
			GuiCB::getter<bool, Renderer, &Renderer::isLightingEnabled>,			
			r, " group='basic' ");
		*/

		enum    SamplingTestFocus { SAMPLING_AO, SAMPLING_IL};
		TwEnumVal modeEV[] =     {
			{ SAMPLING_AO,   "AO"     }, 
			{ SAMPLING_IL,   "IndirectLight"    }			
		};
		TwType modeType = TwDefineEnum("Mode", modeEV, 2); 

	/*
		TwEnumVal randomMode[] =     {
			{ RandomGeneratorShaderSub::OptionValues::none,       "none"     }, 
			{ RandomGeneratorShaderSub::OptionValues::noiseFunc,  "noise"    },	
			{ RandomGeneratorShaderSub::OptionValues::texture,    "texture"  },			
			{ RandomGeneratorShaderSub::OptionValues::func,       "func"     }					
		};
		TwType randomModeType = TwDefineEnum("RandomMode", randomMode, 4); 
	*/
	
		TwEnumVal techniqueMode[] =     {
			{ DoublePassBlendingDeferredRenderer::Technique_t::SIMPLE_SUM,      "simple_sum"      }, 
			{ DoublePassBlendingDeferredRenderer::Technique_t::PYRAMID_BUCKETS, "pyramid_buckets" }, 
			{ DoublePassBlendingDeferredRenderer::Technique_t::TRIANGLE_HIERARCHICAL_SKIP, "triangle_hiearchical_skip"  }			
		};
		TwType techniqueModeType = TwDefineEnum("TechniqueMode", techniqueMode, 3); 

		TwEnumVal emisphereSamplingMode[] =     {
			{ HemisphereSampling::HEMISPHERE_RANDOM,                 "random"     }, 
			{ HemisphereSampling::HEMISPHERE_RANDOM_DISTANCEFALLOFF, "random_distFalloff"     }, 
			{ HemisphereSampling::HEMISPHERE_TRIANGLEHIERARCHY,      "triangles"  },
			{ HemisphereSampling::HEMISPHERE_TRIANGLEHIERARCHY_INTERLEAVED, "triangles_interleaved"  }			
		};
		TwType emisphereSamplingModeType = TwDefineEnum("EmisphereSamplingMode", emisphereSamplingMode, 4); 

		TwEnumVal samplingMode[] =     {
			{ SamplingPatternShaderSub::OptionValues::dssao,     "dssao"     }, 
			{ SamplingPatternShaderSub::OptionValues::radial,    "radial"    },
			{ SamplingPatternShaderSub::OptionValues::emisphere, "emisphere" },	
			{ SamplingPatternShaderSub::OptionValues::flat, "flat" }	
		};
		TwType samplingModeType = TwDefineEnum("SamplingMode", samplingMode, 4); 


		TwEnumVal aoMode[] =     {
			{ D2DaoShaderSub::OptionValues::bunnell, "bunnell"  }, 
			{ D2DaoShaderSub::OptionValues::dssao,	 "dssao"    },
			{ D2DaoShaderSub::OptionValues::test,    "test"     }
		};
		TwType aoModeType = TwDefineEnum("aoMode", aoMode, 3); 

        TwEnumVal ilMode[] =     {
			{ D2DilShaderSub::OptionValues::bunnell, "bunnell"  }, 
			{ D2DilShaderSub::OptionValues::dssao,	 "dssao"    },
			{ D2DilShaderSub::OptionValues::test,    "test"     }
		};
		TwType ilModeType = TwDefineEnum("ilMode", ilMode, 3); 

		 TwEnumVal areaMode[] =     {
			{ AreaCalculatorShaderSub::OptionValues::unitValue,               "unitValue"           }, 
			{ AreaCalculatorShaderSub::OptionValues::triangleArea,		      "triangleArea"        },
			{ AreaCalculatorShaderSub::OptionValues::inscribedCircleArea,     "inscribedCircleArea" },
			{ AreaCalculatorShaderSub::OptionValues::circumscribedCircleArea, "circumscribedCircleArea" },
			{ AreaCalculatorShaderSub::OptionValues::inscribedCircleRadius, "inscribedCircleRadius" },
			{ AreaCalculatorShaderSub::OptionValues::circumscribedCircleRadius, "circumscribedCircleRadius" },
			{ AreaCalculatorShaderSub::OptionValues::inscribedCircleRadiusSquared, "inscribedCircleRadiusSquared" },
			{ AreaCalculatorShaderSub::OptionValues::circumscribedCircleRadiusSquared, "circumscribedCircleRadiusSquared" }		
		};
		TwType areaModeType = TwDefineEnum("areaMode", areaMode, AreaCalculatorShaderSub::OptionValues::SIZE); 


		TwEnumVal aotechniqueMode[] =     {
			{ SaAoProcessingPassShader::Technique_t::SASSAO,  "saSSAO" }, 
			{ SaAoProcessingPassShader::Technique_t::ALCHEMY, "alchemy" }			
		};
		TwType aotechniqueModeType = TwDefineEnum("aoTechniqueMode", aotechniqueMode, 2); 
		
		std::vector<TwEnumVal> mrtbufferMode;
		for (auto& t : MrtBuffer::getBufTypes()) {
			 TwEnumVal v;
			 v.Label = t.label.c_str();
			 v.Value = mrtbufferMode.size();			
			 mrtbufferMode.push_back(v);			 
		}	
		TwType mrtbufferModeType = TwDefineEnum("mrtbufferMode", mrtbufferMode.data(), mrtbufferMode.size());
		
		if (SaAoIlRenderer *dr = dynamic_cast<SaAoIlRenderer*>(r)) {
			TwAddVarCB(m_barHandle, "albedo", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoIlRenderer, &SaAoIlRenderer::setAlbedoEnabled>,
				GuiCB::getter<bool, SaAoIlRenderer, &SaAoIlRenderer::isAlbedoEnabled>,
				dr, " group='compositing' ");		
			TwAddVarCB(m_barHandle, "ambientLight", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoIlRenderer, &SaAoIlRenderer::setAmbientEnabled>,
				GuiCB::getter<bool, SaAoIlRenderer, &SaAoIlRenderer::isAmbientEnabled>,
				dr, " group='compositing' ");		
			TwAddVarCB(m_barHandle, "directLight", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoIlRenderer, &SaAoIlRenderer::setDirectEnabled>,
				GuiCB::getter<bool, SaAoIlRenderer, &SaAoIlRenderer::isDirectEnabled>,
				dr, " group='compositing' ");		
			TwAddVarCB(m_barHandle, "ambientOcclusion", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoIlRenderer, &SaAoIlRenderer::setAoEnabled>,
				GuiCB::getter<bool, SaAoIlRenderer, &SaAoIlRenderer::isAoEnabled>,
				dr, " group='compositing' ");

			TwAddVarCB(m_barHandle, "indirectLight", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoIlRenderer, &SaAoIlRenderer::setIlEnabled>,
				GuiCB::getter<bool, SaAoIlRenderer, &SaAoIlRenderer::isIlEnabled>,
				dr, " group='compositing' ");

			TwAddVarCB(m_barHandle, "filtering", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoIlRenderer, &SaAoIlRenderer::setBlurEnabled>,
				GuiCB::getter<bool, SaAoIlRenderer, &SaAoIlRenderer::isBlurEnabled>,
				dr, " group='compositing' ");
		
			TwAddVarCB(m_barHandle, "Gbuffer colorBufferType",  mrtbufferModeType,
				GuiCB::setter<MrtBufferType::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::setGbColorBufferType>,
				GuiCB::getter<MrtBufferType::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::getGbColorBufferType>,								
				dr, " group='MRT' ");
			TwAddVarCB(m_barHandle, "Gbuffer depthBufferType",  mrtbufferModeType,
				GuiCB::setter<MrtBufferType::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::setGbDepthBufferType>,
				GuiCB::getter<MrtBufferType::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::getGbDepthBufferType>,								
				dr, " group='MRT' ");
			TwAddVarCB(m_barHandle, "AObuffer colorBufferType",  mrtbufferModeType,
				GuiCB::setter<MrtBufferType::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::setAoColorBufferType>,
				GuiCB::getter<MrtBufferType::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::getAoColorBufferType>,								
				dr, " group='MRT' ");
			
			TwAddVarCB(m_barHandle, "showGbuffer",  TW_TYPE_BOOLCPP, 
				GuiCB::setter<bool, SaAoIlRenderer, &SaAoIlRenderer::setShowingGBuffer>,
				GuiCB::getter<bool, SaAoIlRenderer, &SaAoIlRenderer::isShowingGBuffer>,
				dr, " group='debug' ");
			TwAddVarCB(m_barHandle, "showAObuffer",  TW_TYPE_BOOLCPP, 
				GuiCB::setter<bool, SaAoIlRenderer, &SaAoIlRenderer::setShowingAOBuffer>,
				GuiCB::getter<bool, SaAoIlRenderer, &SaAoIlRenderer::isShowingAOBuffer>,
				dr, " group='debug' ");
			/*TwAddVarCB(m_barHandle, "showAreas", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoIlRenderer, &SaAoIlRenderer::setShowingArea>,
				GuiCB::getter<bool, SaAoIlRenderer, &SaAoIlRenderer::isShowingArea>,
				dr, " group='debug' ");		
			TwAddVarCB(m_barHandle, "showSamplingDensity", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoIlRenderer, &SaAoIlRenderer::setShowingSamplingDensity>,
				GuiCB::getter<bool, SaAoIlRenderer, &SaAoIlRenderer::isShowingSamplingDensity>,
				dr, " group='debug' ");		
			TwAddVarCB(m_barHandle, "showSampling", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoIlRenderer, &SaAoIlRenderer::setShowingSampling>,
				GuiCB::getter<bool, SaAoIlRenderer, &SaAoIlRenderer::isShowingSampling>,
				dr, " group='debug' ");
			TwAddVarCB(m_barHandle, "showZoomedArea", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoIlRenderer, &SaAoIlRenderer::setShowingZoomedArea>,
				GuiCB::getter<bool, SaAoIlRenderer, &SaAoIlRenderer::isShowingZoomedArea>,
				dr, " group='debug' ");	
				*/
			 
			TwAddVarCB(m_barHandle, "technique", aotechniqueModeType,
				GuiCB::setter<SaAoIlProcessingPassShader::Technique_t, SaAoIlRenderer, &SaAoIlRenderer::setTechnique>,
				GuiCB::getter<SaAoIlProcessingPassShader::Technique_t, SaAoIlRenderer, &SaAoIlRenderer::getTechnique>,
				dr, " group='AOtech' ");

			TwAddVarCB(m_barHandle, "numSamples", TW_TYPE_INT32,
				GuiCB::setter<int, SaAoIlRenderer, &SaAoIlRenderer::setAoNumSamples>,
				GuiCB::getter<int, SaAoIlRenderer, &SaAoIlRenderer::getAoNumSamples>,				
				dr, " group='sampling' min=8 max=64 step=8 ");
			TwAddVarCB(m_barHandle, "maxSamplingRadius", TW_TYPE_FLOAT,
				GuiCB::setter<float, SaAoIlRenderer, &SaAoIlRenderer::setAoSamplingRadius>,
				GuiCB::getter<float, SaAoIlRenderer, &SaAoIlRenderer::getAoSamplingRadius>,									
				dr, " group='sampling' min=0.01 max=100 step=0.01 ");
			TwAddVarCB(m_barHandle, "samplingPattern",  samplingModeType,
				GuiCB::setter<SamplingPatternShaderSub::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::setSamplingPatternId>,
				GuiCB::getter<SamplingPatternShaderSub::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::getSamplingPatternId>,								
				dr, " group='sampling' ");
			TwAddVarCB(m_barHandle, "emisphereSamplingPattern", emisphereSamplingModeType,
				GuiCB::setter<HemisphereSampling::KernelMode_t, SaAoIlRenderer, &SaAoIlRenderer::setHemisphereSamplingMode>,
				GuiCB::getter<HemisphereSampling::KernelMode_t, SaAoIlRenderer, &SaAoIlRenderer::getHemisphereSamplingMode>,
				dr, " group='emisphereSampling' ");			
			TwAddVarCB(m_barHandle, "uniformSamplingRadius", TW_TYPE_BOOLCPP,
				GuiCB::setter<bool, SaAoIlRenderer, &SaAoIlRenderer::setUniformSamplingRadius>,
				GuiCB::getter<bool, SaAoIlRenderer, &SaAoIlRenderer::isUniformSamplingRadius>,									
				dr, " group='emisphereSampling' ");
						
			
			TwAddVarCB(m_barHandle, "aoComputation",   aoModeType, 
				GuiCB::setter<D2DaoShaderSub::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::setAoComputationId>,
				GuiCB::getter<D2DaoShaderSub::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::getAoComputationId>,				
				dr, " group='AmbientOcclusion' label='aoComputation' ");
			TwAddVarCB(m_barHandle, "aoMultiplier", TW_TYPE_FLOAT, 
				GuiCB::setter<float, SaAoIlRenderer, &SaAoIlRenderer::setAoMultiplier>,
				GuiCB::getter<float, SaAoIlRenderer, &SaAoIlRenderer::getAoMultiplier>,
				dr, " group='AmbientOcclusion' min=0 max=100 step=0.01 ");

			TwAddVarCB(m_barHandle, "angleBias", TW_TYPE_FLOAT, 
				GuiCB::setter<float, SaAoIlRenderer, &SaAoIlRenderer::setAoAngleBias>,
				GuiCB::getter<float, SaAoIlRenderer, &SaAoIlRenderer::getAoAngleBias>,					
				dr, " group='AmbientOcclusion' min=0 max=1 step=0.01 ");
			TwAddVarCB(m_barHandle, "maxDistance",          TW_TYPE_FLOAT,
				GuiCB::setter<float, SaAoIlRenderer, &SaAoIlRenderer::setAoMaxDistance>,
				GuiCB::getter<float, SaAoIlRenderer, &SaAoIlRenderer::getAoMaxDistance>,	
				dr, " group='AmbientOcclusion' min=0 max=10 step=0.01 ");
			
			TwAddVarCB(m_barHandle, "ilComputation",   ilModeType,
				GuiCB::setter<D2DilShaderSub::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::setIlComputationId>,
				GuiCB::getter<D2DilShaderSub::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::getIlComputationId>,				
				dr, " group='IndirectLight' label='ilComputation' ");
			TwAddVarCB(m_barHandle, "ilMultiplier", TW_TYPE_FLOAT, 
				GuiCB::setter<float, SaAoIlRenderer, &SaAoIlRenderer::setIlMultiplier>,
				GuiCB::getter<float, SaAoIlRenderer, &SaAoIlRenderer::getIlMultiplier>,
				dr, " group='IndirectLight' min=0 max=100 step=0.01 ");	

			TwAddVarCB(m_barHandle, "areaComputation", areaModeType, 
				GuiCB::setter<AreaCalculatorShaderSub::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::setAreaComputationId>,
				GuiCB::getter<AreaCalculatorShaderSub::OptionValues, SaAoIlRenderer, &SaAoIlRenderer::getAreaComputationId>,
				dr, " group='saSSAO' ");
			TwAddVarCB(m_barHandle, "areaMultiplier", TW_TYPE_FLOAT, 
				GuiCB::setter<float, SaAoIlRenderer, &SaAoIlRenderer::setAreaMultiplier>,
				GuiCB::getter<float, SaAoIlRenderer, &SaAoIlRenderer::getAreaMultiplier>,
				dr, " group='saSSAO' min=0 max=100 step=0.01 ");
			TwAddVarCB(m_barHandle, "solidAngleMultiplier", TW_TYPE_FLOAT, 
				GuiCB::setter<float, SaAoIlRenderer, &SaAoIlRenderer::setSolidAngleMultiplier>,
				GuiCB::getter<float, SaAoIlRenderer, &SaAoIlRenderer::getSolidAngleMultiplier>,
				dr, " group='saSSAO' min=0 max=100 step=0.01 ");
			TwAddVarCB(m_barHandle, "triangleSideDivs", TW_TYPE_INT32,
				GuiCB::setter<int, SaAoIlRenderer, &SaAoIlRenderer::setTriangleSideDivs>,
				GuiCB::getter<int, SaAoIlRenderer, &SaAoIlRenderer::getTriangleSideDivs>,
				dr, " group='saSSAO' min=1 max=4 step=1");			

			TwAddVarCB(m_barHandle, "alchemy_k", TW_TYPE_FLOAT,				
				GuiCB::setter<float, SaAoIlRenderer, &SaAoIlRenderer::setAlchemyK>,
				GuiCB::getter<float, SaAoIlRenderer, &SaAoIlRenderer::getAlchemyK>,
				dr, " group='alchemy' min=0.01 max=10 step=0.01");	
			TwAddVarCB(m_barHandle, "alchemy_ro", TW_TYPE_FLOAT,				
				GuiCB::setter<float, SaAoIlRenderer, &SaAoIlRenderer::setAlchemyRO>,
				GuiCB::getter<float, SaAoIlRenderer, &SaAoIlRenderer::getAlchemyRO>,
				dr, " group='alchemy' min=0.01 max=10 step=0.01");	
			TwAddVarCB(m_barHandle, "alchemy_u", TW_TYPE_FLOAT,				
				GuiCB::setter<float, SaAoIlRenderer, &SaAoIlRenderer::setAlchemyU>,
				GuiCB::getter<float, SaAoIlRenderer, &SaAoIlRenderer::getAlchemyU>,
				dr, " group='alchemy' min=0.0001 max=1 step=0.0001");

			
			TwAddVarCB(m_barHandle, "kernelSize", TW_TYPE_INT32,				
				GuiCB::setter<int, SaAoIlRenderer, &SaAoIlRenderer::setBlurKernelSize>,
				GuiCB::getter<int, SaAoIlRenderer, &SaAoIlRenderer::getBlurKernelSize>,
				dr, " group='filteringParams' min=0 max=20");	
			TwAddVarCB(m_barHandle, "kdepth", TW_TYPE_FLOAT,				
				GuiCB::setter<float, SaAoIlRenderer, &SaAoIlRenderer::setBlurKdepth>,
				GuiCB::getter<float, SaAoIlRenderer, &SaAoIlRenderer::getBlurKdepth>,
				dr, " group='filteringParams' min=0.01 max=100 step=0.01");	
			TwAddVarCB(m_barHandle, "knormal", TW_TYPE_FLOAT,				
				GuiCB::setter<float, SaAoIlRenderer, &SaAoIlRenderer::setBlurKnormal>,
				GuiCB::getter<float, SaAoIlRenderer, &SaAoIlRenderer::getBlurKnormal>,
				dr, " group='filteringParams' min=0.01 max=100 step=0.01");	
						
		} else 
		if (SaAoRenderer *dr = dynamic_cast<SaAoRenderer*>(r)) {
			TwAddVarCB(m_barHandle, "albedo", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoRenderer, &SaAoRenderer::setAlbedoEnabled>,
				GuiCB::getter<bool, SaAoRenderer, &SaAoRenderer::isAlbedoEnabled>,
				dr, " group='compositing' ");		
			TwAddVarCB(m_barHandle, "ambientLight", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoRenderer, &SaAoRenderer::setAmbientEnabled>,
				GuiCB::getter<bool, SaAoRenderer, &SaAoRenderer::isAmbientEnabled>,
				dr, " group='compositing' ");		
			TwAddVarCB(m_barHandle, "directLight", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoRenderer, &SaAoRenderer::setDirectEnabled>,
				GuiCB::getter<bool, SaAoRenderer, &SaAoRenderer::isDirectEnabled>,
				dr, " group='compositing' ");		
			TwAddVarCB(m_barHandle, "ambientOcclusion", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoRenderer, &SaAoRenderer::setAoEnabled>,
				GuiCB::getter<bool, SaAoRenderer, &SaAoRenderer::isAoEnabled>,
				dr, " group='compositing' ");	
			TwAddVarCB(m_barHandle, "filtering", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoRenderer, &SaAoRenderer::setBlurEnabled>,
				GuiCB::getter<bool, SaAoRenderer, &SaAoRenderer::isBlurEnabled>,
				dr, " group='compositing' ");
		
			TwAddVarCB(m_barHandle, "Gbuffer colorBufferType",  mrtbufferModeType,
				GuiCB::setter<MrtBufferType::OptionValues, SaAoRenderer, &SaAoRenderer::setGbColorBufferType>,
				GuiCB::getter<MrtBufferType::OptionValues, SaAoRenderer, &SaAoRenderer::getGbColorBufferType>,								
				dr, " group='MRT' ");
			TwAddVarCB(m_barHandle, "Gbuffer depthBufferType",  mrtbufferModeType,
				GuiCB::setter<MrtBufferType::OptionValues, SaAoRenderer, &SaAoRenderer::setGbDepthBufferType>,
				GuiCB::getter<MrtBufferType::OptionValues, SaAoRenderer, &SaAoRenderer::getGbDepthBufferType>,								
				dr, " group='MRT' ");
			TwAddVarCB(m_barHandle, "AObuffer colorBufferType",  mrtbufferModeType,
				GuiCB::setter<MrtBufferType::OptionValues, SaAoRenderer, &SaAoRenderer::setAoColorBufferType>,
				GuiCB::getter<MrtBufferType::OptionValues, SaAoRenderer, &SaAoRenderer::getAoColorBufferType>,								
				dr, " group='MRT' ");
			
			TwAddVarCB(m_barHandle, "showGbuffer",  TW_TYPE_BOOLCPP, 
				GuiCB::setter<bool, SaAoRenderer, &SaAoRenderer::setShowingGBuffer>,
				GuiCB::getter<bool, SaAoRenderer, &SaAoRenderer::isShowingGBuffer>,
				dr, " group='debug' ");
			TwAddVarCB(m_barHandle, "showAObuffer",  TW_TYPE_BOOLCPP, 
				GuiCB::setter<bool, SaAoRenderer, &SaAoRenderer::setShowingAOBuffer>,
				GuiCB::getter<bool, SaAoRenderer, &SaAoRenderer::isShowingAOBuffer>,
				dr, " group='debug' ");
			/*TwAddVarCB(m_barHandle, "showAreas", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoRenderer, &SaAoRenderer::setShowingArea>,
				GuiCB::getter<bool, SaAoRenderer, &SaAoRenderer::isShowingArea>,
				dr, " group='debug' ");		
			TwAddVarCB(m_barHandle, "showSamplingDensity", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoRenderer, &SaAoRenderer::setShowingSamplingDensity>,
				GuiCB::getter<bool, SaAoRenderer, &SaAoRenderer::isShowingSamplingDensity>,
				dr, " group='debug' ");		
			TwAddVarCB(m_barHandle, "showSampling", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoRenderer, &SaAoRenderer::setShowingSampling>,
				GuiCB::getter<bool, SaAoRenderer, &SaAoRenderer::isShowingSampling>,
				dr, " group='debug' ");
			TwAddVarCB(m_barHandle, "showZoomedArea", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, SaAoRenderer, &SaAoRenderer::setShowingZoomedArea>,
				GuiCB::getter<bool, SaAoRenderer, &SaAoRenderer::isShowingZoomedArea>,
				dr, " group='debug' ");	
				*/
			 
			TwAddVarCB(m_barHandle, "technique", aotechniqueModeType,
				GuiCB::setter<SaAoProcessingPassShader::Technique_t, SaAoRenderer, &SaAoRenderer::setTechnique>,
				GuiCB::getter<SaAoProcessingPassShader::Technique_t, SaAoRenderer, &SaAoRenderer::getTechnique>,
				dr, " group='AOtech' ");

			TwAddVarCB(m_barHandle, "numSamples", TW_TYPE_INT32,
				GuiCB::setter<int, SaAoRenderer, &SaAoRenderer::setAoNumSamples>,
				GuiCB::getter<int, SaAoRenderer, &SaAoRenderer::getAoNumSamples>,				
				dr, " group='sampling' min=8 max=64 step=8 ");
			TwAddVarCB(m_barHandle, "maxSamplingRadius", TW_TYPE_FLOAT,
				GuiCB::setter<float, SaAoRenderer, &SaAoRenderer::setAoSamplingRadius>,
				GuiCB::getter<float, SaAoRenderer, &SaAoRenderer::getAoSamplingRadius>,									
				dr, " group='sampling' min=0.01 max=100 step=0.01 ");
			TwAddVarCB(m_barHandle, "samplingPattern",  samplingModeType,
				GuiCB::setter<SamplingPatternShaderSub::OptionValues, SaAoRenderer, &SaAoRenderer::setSamplingPatternId>,
				GuiCB::getter<SamplingPatternShaderSub::OptionValues, SaAoRenderer, &SaAoRenderer::getSamplingPatternId>,								
				dr, " group='sampling' ");
			TwAddVarCB(m_barHandle, "emisphereSamplingPattern", emisphereSamplingModeType,
				GuiCB::setter<HemisphereSampling::KernelMode_t, SaAoRenderer, &SaAoRenderer::setHemisphereSamplingMode>,
				GuiCB::getter<HemisphereSampling::KernelMode_t, SaAoRenderer, &SaAoRenderer::getHemisphereSamplingMode>,
				dr, " group='emisphereSampling' ");			
			TwAddVarCB(m_barHandle, "uniformSamplingRadius", TW_TYPE_BOOLCPP,
				GuiCB::setter<bool, SaAoRenderer, &SaAoRenderer::setUniformSamplingRadius>,
				GuiCB::getter<bool, SaAoRenderer, &SaAoRenderer::isUniformSamplingRadius>,									
				dr, " group='emisphereSampling' ");
						
			
			TwAddVarCB(m_barHandle, "aoComputation",   aoModeType, 
				GuiCB::setter<D2DaoShaderSub::OptionValues, SaAoRenderer, &SaAoRenderer::setAoComputationId>,
				GuiCB::getter<D2DaoShaderSub::OptionValues, SaAoRenderer, &SaAoRenderer::getAoComputationId>,				
				dr, " group='AmbientOcclusion' label='aoComputation' ");
			TwAddVarCB(m_barHandle, "aoMultiplier", TW_TYPE_FLOAT, 
				GuiCB::setter<float, SaAoRenderer, &SaAoRenderer::setAoMultiplier>,
				GuiCB::getter<float, SaAoRenderer, &SaAoRenderer::getAoMultiplier>,
				dr, " group='AmbientOcclusion' min=0 max=100 step=0.01 ");

			TwAddVarCB(m_barHandle, "angleBias", TW_TYPE_FLOAT, 
				GuiCB::setter<float, SaAoRenderer, &SaAoRenderer::setAoAngleBias>,
				GuiCB::getter<float, SaAoRenderer, &SaAoRenderer::getAoAngleBias>,					
				dr, " group='AmbientOcclusion' min=0 max=1 step=0.01 ");
			TwAddVarCB(m_barHandle, "maxDistance",          TW_TYPE_FLOAT,
				GuiCB::setter<float, SaAoRenderer, &SaAoRenderer::setAoMaxDistance>,
				GuiCB::getter<float, SaAoRenderer, &SaAoRenderer::getAoMaxDistance>,	
				dr, " group='AmbientOcclusion' min=0 max=10 step=0.01 ");
			
			TwAddVarCB(m_barHandle, "areaComputation", areaModeType, 
				GuiCB::setter<AreaCalculatorShaderSub::OptionValues, SaAoRenderer, &SaAoRenderer::setAreaComputationId>,
				GuiCB::getter<AreaCalculatorShaderSub::OptionValues, SaAoRenderer, &SaAoRenderer::getAreaComputationId>,
				dr, " group='saSSAO' ");
			TwAddVarCB(m_barHandle, "areaMultiplier", TW_TYPE_FLOAT, 
				GuiCB::setter<float, SaAoRenderer, &SaAoRenderer::setAreaMultiplier>,
				GuiCB::getter<float, SaAoRenderer, &SaAoRenderer::getAreaMultiplier>,
				dr, " group='saSSAO' min=0 max=100 step=0.01 ");
			TwAddVarCB(m_barHandle, "solidAngleMultiplier", TW_TYPE_FLOAT, 
				GuiCB::setter<float, SaAoRenderer, &SaAoRenderer::setSolidAngleMultiplier>,
				GuiCB::getter<float, SaAoRenderer, &SaAoRenderer::getSolidAngleMultiplier>,
				dr, " group='saSSAO' min=0 max=100 step=0.01 ");
			TwAddVarCB(m_barHandle, "triangleSideDivs", TW_TYPE_INT32,
				GuiCB::setter<int, SaAoRenderer, &SaAoRenderer::setTriangleSideDivs>,
				GuiCB::getter<int, SaAoRenderer, &SaAoRenderer::getTriangleSideDivs>,
				dr, " group='saSSAO' min=1 max=4 step=1");			

			TwAddVarCB(m_barHandle, "alchemy_k", TW_TYPE_FLOAT,				
				GuiCB::setter<float, SaAoRenderer, &SaAoRenderer::setAlchemyK>,
				GuiCB::getter<float, SaAoRenderer, &SaAoRenderer::getAlchemyK>,
				dr, " group='alchemy' min=0.01 max=10 step=0.01");	
			TwAddVarCB(m_barHandle, "alchemy_ro", TW_TYPE_FLOAT,				
				GuiCB::setter<float, SaAoRenderer, &SaAoRenderer::setAlchemyRO>,
				GuiCB::getter<float, SaAoRenderer, &SaAoRenderer::getAlchemyRO>,
				dr, " group='alchemy' min=0.01 max=10 step=0.01");	
			TwAddVarCB(m_barHandle, "alchemy_u", TW_TYPE_FLOAT,				
				GuiCB::setter<float, SaAoRenderer, &SaAoRenderer::setAlchemyU>,
				GuiCB::getter<float, SaAoRenderer, &SaAoRenderer::getAlchemyU>,
				dr, " group='alchemy' min=0.0001 max=1 step=0.0001");

			
			TwAddVarCB(m_barHandle, "kernelSize", TW_TYPE_INT32,				
				GuiCB::setter<int, SaAoRenderer, &SaAoRenderer::setBlurKernelSize>,
				GuiCB::getter<int, SaAoRenderer, &SaAoRenderer::getBlurKernelSize>,
				dr, " group='filteringParams' min=0 max=20");	
			TwAddVarCB(m_barHandle, "kdepth", TW_TYPE_FLOAT,				
				GuiCB::setter<float, SaAoRenderer, &SaAoRenderer::setBlurKdepth>,
				GuiCB::getter<float, SaAoRenderer, &SaAoRenderer::getBlurKdepth>,
				dr, " group='filteringParams' min=0.01 max=100 step=0.01");	
			TwAddVarCB(m_barHandle, "knormal", TW_TYPE_FLOAT,				
				GuiCB::setter<float, SaAoRenderer, &SaAoRenderer::setBlurKnormal>,
				GuiCB::getter<float, SaAoRenderer, &SaAoRenderer::getBlurKnormal>,
				dr, " group='filteringParams' min=0.01 max=100 step=0.01");	
			
					
		} else 
		if (DoublePassBlendingDeferredRenderer *dr = dynamic_cast<DoublePassBlendingDeferredRenderer*>(r)) {
			TwAddVarCB(m_barHandle, "showGBuffer",  TW_TYPE_BOOLCPP, 
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setShowingGBuffer>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isShowingGBuffer>,
				dr, " group='debug' ");
			TwAddVarCB(m_barHandle, "showDLBuffer", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setShowingDLBuffer>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isShowingDLBuffer>,				
				dr, " group='debug' ");		
			TwAddVarCB(m_barHandle, "showAreas", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setShowingArea>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isShowingArea>,
				dr, " group='debug' ");		
			TwAddVarCB(m_barHandle, "showSamplingDensity", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setShowingSamplingDensity>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isShowingSamplingDensity>,
				dr, " group='debug' ");		
			TwAddVarCB(m_barHandle, "showSampling", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setShowingSampling>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isShowingSampling>,
				dr, " group='debug' ");		
			TwAddVarCB(m_barHandle, "showZoomedArea", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setShowingZoomedArea>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isShowingZoomedArea>,
				dr, " group='debug' ");	
			 

			TwAddVarCB(m_barHandle, "albedo", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setAlbedoEnabled>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isAlbedoEnabled>,
				dr, " group='compositing' ");		
			TwAddVarCB(m_barHandle, "ambientLight", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setAmbientEnabled>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isAmbientEnabled>,
				dr, " group='compositing' ");		
			TwAddVarCB(m_barHandle, "diffuseLight", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setDiffuseEnabled>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isDiffuseEnabled>,
				dr, " group='compositing' ");		
			TwAddVarCB(m_barHandle, "specularLight", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setSpecularEnabled>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isSpecularEnabled>,
				dr, " group='compositing' ");		
			TwAddVarCB(m_barHandle, "ambientOcclusion", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setAoEnabled>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isAoEnabled>,
				dr, " group='compositing' ");		
			TwAddVarCB(m_barHandle, "indirectLight", TW_TYPE_BOOLCPP,				
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setIlEnabled>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isIlEnabled>,
				dr, " group='compositing' ");		


			TwAddVarCB(m_barHandle, "technique", techniqueModeType,
				GuiCB::setter<DoublePassBlendingDeferredRenderer::Technique_t, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setTechnique>,
				GuiCB::getter<DoublePassBlendingDeferredRenderer::Technique_t, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getTechnique>,
				dr, " group='' ");
			
			TwAddVarCB(m_barHandle, "triangleSideDivs", TW_TYPE_INT32,
				GuiCB::setter<int, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setTriangleSideDivs>,
				GuiCB::getter<int, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getTriangleSideDivs>,
				dr, " group='pyramidBuckets' min=2 max=4 step=1");


			TwAddVarCB(m_barHandle, "emisphereSamplingPattern", emisphereSamplingModeType,
				GuiCB::setter<HemisphereSampling::KernelMode_t, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setHemisphereSamplingMode>,
				GuiCB::getter<HemisphereSampling::KernelMode_t, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getHemisphereSamplingMode>,
				dr, " group='emisphereSampling' ");
			
			TwAddVarCB(m_barHandle, "uniformSamplingRadius", TW_TYPE_BOOLCPP,
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setUniformSamplingRadius>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isUniformSamplingRadius>,									
				dr, " group='emisphereSampling' ");
			TwAddVarCB(m_barHandle, "samplingPattern",  samplingModeType,
				GuiCB::setter<SamplingPatternShaderSub::OptionValues, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setSamplingPatternId>,
				GuiCB::getter<SamplingPatternShaderSub::OptionValues, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getSamplingPatternId>,								
				dr, " group='sampling' ");
/*
			TwAddVarCB(m_barHandle, "randomGeneration", randomModeType,  
				GuiCB::setter<RandomGeneratorShaderSub::OptionValues, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setRandomGenerationId>,
				GuiCB::getter<RandomGeneratorShaderSub::OptionValues, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getRandomGenerationId>,				
				dr, " group='sampling' ");
*/
			TwAddVarCB(m_barHandle, "numSamples", TW_TYPE_INT32,
				//setCB_blSamples,   getCB_blSamples, 
				GuiCB::setter<int, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setBSnumSamples>,
				GuiCB::getter<int, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getBSnumSamples>,				
				dr, " group='sampling' min=8 max=64 step=8 ");
			TwAddVarCB(m_barHandle, "maxSamplingRadius", TW_TYPE_FLOAT,
				//setCB_blRadLength, getCB_blRadLength, 
				GuiCB::setter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setBSmaxSamplingRadius>,
				GuiCB::getter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getBSmaxSamplingRadius>,									
				dr, " group='sampling' min=0.01 max=100 step=0.01 ");
			

			
			/*
			TwAddVarCB(m_barHandle, "octantInterleaving", TW_TYPE_BOOLCPP,
				GuiCB::setter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setSamplingOctantInterleaving>,
				GuiCB::getter<bool, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::isSamplingOctantInterleving>,									
				dr, " group='sampling' ");
				*/

			TwAddVarCB(m_barHandle, "minDisplacement", TW_TYPE_FLOAT, 
				//setCB_blDiskDisplacement, getCB_blDiskDisplacement, 
				GuiCB::setter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setBSdiskDisplacement>,
				GuiCB::getter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getBSdiskDisplacement>,					
				dr, " group='DiskParams' min=0 max=1 step=0.01 ");
			TwAddVarCB(m_barHandle, "distMax",          TW_TYPE_FLOAT,
				GuiCB::setter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setBSdistMax>,
				GuiCB::getter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getBSdistMax>,	
				//setCB_blDistMax,   getCB_blDistMax, 
				dr, " group='DiskParams' min=0 max=10 step=0.01 ");
			TwAddVarCB(m_barHandle, "areaComputation", areaModeType, 
				GuiCB::setter<AreaCalculatorShaderSub::OptionValues, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setAreaComputationId>,
				GuiCB::getter<AreaCalculatorShaderSub::OptionValues, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getAreaComputationId>,
				dr, " group='DiskParams' ");
			TwAddVarCB(m_barHandle, "areaMultiplier", TW_TYPE_FLOAT, 
				GuiCB::setter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setAreaMultiplier>,
				GuiCB::getter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getAreaMultiplier>,
				dr, " group='DiskParams' min=0 max=100 step=0.01 ");
			TwAddVarCB(m_barHandle, "solidAngleMultiplier", TW_TYPE_FLOAT, 
				GuiCB::setter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setSolidAngleMultiplier>,
				GuiCB::getter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getSolidAngleMultiplier>,
				dr, " group='DiskParams' min=0 max=100 step=0.01 ");

			

			TwAddVarCB(m_barHandle, "aoComputation",   aoModeType, 
				GuiCB::setter<D2DaoShaderSub::OptionValues, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setAoComputationId>,
				GuiCB::getter<D2DaoShaderSub::OptionValues, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getAoComputationId>,				
				dr, " group='AmbientOcclusion' label='aoComputation' ");
			TwAddVarCB(m_barHandle, "aoMultiplier", TW_TYPE_FLOAT, 
				GuiCB::setter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setAoMultiplier>,
				GuiCB::getter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getAoMultiplier>,
				dr, " group='AmbientOcclusion' min=0 max=100 step=0.01 ");

			TwAddVarCB(m_barHandle, "ilComputation",   ilModeType,
				GuiCB::setter<D2DilShaderSub::OptionValues, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setIlComputationId>,
				GuiCB::getter<D2DilShaderSub::OptionValues, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getIlComputationId>,				
				dr, " group='IndLight' label='ilComputation' ");
			TwAddVarCB(m_barHandle, "ilMultiplier", TW_TYPE_FLOAT, 
				GuiCB::setter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::setIlMultiplier>,
				GuiCB::getter<float, DoublePassBlendingDeferredRenderer, &DoublePassBlendingDeferredRenderer::getIlMultiplier>,
				dr, " group='IndLight' min=0 max=100 step=0.01 ");		
		}
		if (DeferredRenderer *dr = dynamic_cast<DeferredRenderer*>(r)) {
			//TwAddVarCB(m_barHandle, "randomGeneration", randomModeType,  
			//	GuiCB::setter<RandomGeneratorShaderSub::OptionValues, DeferredRenderer, &DeferredRenderer::setRandomGenerationId>,
			//	GuiCB::getter<RandomGeneratorShaderSub::OptionValues, DeferredRenderer, &DeferredRenderer::getRandomGenerationId>,
			//	dr, " ");
			
			
			TwAddVarCB(m_barHandle, "samplingPattern",  samplingModeType,
				GuiCB::setter<SamplingPatternShaderSub::OptionValues, DeferredRenderer, &DeferredRenderer::setSamplingPatternId>,
				GuiCB::getter<SamplingPatternShaderSub::OptionValues, DeferredRenderer, &DeferredRenderer::getSamplingPatternId>,				
				//setCB_samplingPattern,  getCB_samplingPattern, 
				dr, " ");

			TwAddVarCB(m_barHandle, "emisphereSamplingPattern", emisphereSamplingModeType,
				GuiCB::setter<HemisphereSampling::KernelMode_t, DeferredRenderer, &DeferredRenderer::setHemisphereSamplingMode>,
				GuiCB::getter<HemisphereSampling::KernelMode_t, DeferredRenderer, &DeferredRenderer::getHemisphereSamplingMode>,
				dr, " group='sampling' ");
			TwAddVarCB(m_barHandle, "uniformSamplingRadius", TW_TYPE_BOOLCPP,
				GuiCB::setter<bool, DeferredRenderer, &DeferredRenderer::setUniformSamplingRadius>,
				GuiCB::getter<bool, DeferredRenderer, &DeferredRenderer::isUniformSamplingRadius>,									
				dr, " group='sampling' ");

			TwAddVarCB(m_barHandle, "showGBuffer",  TW_TYPE_BOOLCPP, 
				GuiCB::setter<bool, DeferredRenderer, &DeferredRenderer::setShowingGBuffer>,
				GuiCB::getter<bool, DeferredRenderer, &DeferredRenderer::isShowingGBuffer>,
				//setCB_showGBuf,  getCB_showGBuf,
				dr, " label='showGBuffer' ");

			TwAddVarCB(m_barHandle, "samplingTest", modeType,   
				GuiCB::setter<int, DeferredRenderer, &DeferredRenderer::setSamplingScopeId>,
				GuiCB::getter<int, DeferredRenderer, &DeferredRenderer::getSamplingScopeId>,				
				//setCB_setSamplingScope,  getCB_setSamplingScope, 
				dr, " label='samplingTest' ");
			
			TwAddVarCB(m_barHandle, "ambientPass",       TW_TYPE_BOOLCPP, 
				GuiCB::setter<bool, DeferredRenderer, &DeferredRenderer::setAmbientPass>,
				GuiCB::getter<bool, DeferredRenderer, &DeferredRenderer::isAmbientPassEnabled>,
				dr, " group='PASS' ");
				//setCB_ambientPass,       getCB_ambientPass,       dr, " group='PASS' ");
			TwAddVarCB(m_barHandle, "directLightPass",   TW_TYPE_BOOLCPP, 
				GuiCB::setter<bool, DeferredRenderer, &DeferredRenderer::setDirectLightPass>,
				GuiCB::getter<bool, DeferredRenderer, &DeferredRenderer::isDirectLightPassEnabled>,
				//setCB_directLightPass,   getCB_directLightPass,
				dr, " group='PASS' ");
			TwAddVarCB(m_barHandle, "indirectLightPass", TW_TYPE_BOOLCPP, 
				GuiCB::setter<bool, DeferredRenderer, &DeferredRenderer::setIndirectLightPass>,
				GuiCB::getter<bool, DeferredRenderer, &DeferredRenderer::isIndirectLightPassEnabled>,
				//setCB_indirectLightPass, getCB_indirectLightPass,
				dr, " group='PASS' ");
			TwAddVarCB(m_barHandle, "discDSSAOpass",     TW_TYPE_BOOLCPP,  
				GuiCB::setter<bool, DeferredRenderer, &DeferredRenderer::setDiskDSSAOpass>,
				GuiCB::getter<bool, DeferredRenderer, &DeferredRenderer::isDiskDSSAOpassEnabled>,
				//setCB_discDSSAOpass,     getCB_discDSSAOpass,
				dr, " group='PASS' ");
			TwAddVarCB(m_barHandle, "samplingTestPass",  TW_TYPE_BOOLCPP, 
				GuiCB::setter<bool, DeferredRenderer, &DeferredRenderer::setSamplingTestPass>,
				GuiCB::getter<bool, DeferredRenderer, &DeferredRenderer::isSamplingTestPassEnabled>,
				//setCB_samplingTestPass,  getCB_samplingTestPass,  
				dr, " group='PASS' ");
	
			TwAddVarCB(m_barHandle, "aoComputation",   aoModeType, 
				GuiCB::setter<D2DaoShaderSub::OptionValues, DeferredRenderer, &DeferredRenderer::setAoComputationId>,
				GuiCB::getter<D2DaoShaderSub::OptionValues, DeferredRenderer, &DeferredRenderer::getAoComputationId>,				
				dr, " group='DSSAO' label='aoComputation' ");
			TwAddVarCB(m_barHandle, "aoSamples",   TW_TYPE_INT32, 
				GuiCB::setter<int, DeferredRenderer, &DeferredRenderer::setDSSAOsamples>,
				GuiCB::getter<int, DeferredRenderer, &DeferredRenderer::getDSSAOsamples>,
				//setCB_aoSamples, getCB_aoSamples,
				dr, " group='DSSAO' label='aoSamples' min=8 max=64 step=8 ");
	
			TwAddVarCB(m_barHandle, "aoEnabled",   TW_TYPE_BOOLCPP,   
				GuiCB::setter<bool, DeferredRenderer, &DeferredRenderer::setAO>,
				GuiCB::getter<bool, DeferredRenderer, &DeferredRenderer::isAOenabled>,
				//setCB_aoEnabled, getCB_aoEnabled, 
				dr, " group='DSSAO' label='aoEnabled' ");
			TwAddVarCB(m_barHandle, "aoOnly",      TW_TYPE_BOOLCPP,    
				GuiCB::setter<bool, DeferredRenderer, &DeferredRenderer::setShowingOnlyAO>,
				GuiCB::getter<bool, DeferredRenderer, &DeferredRenderer::isShowingOnlyAO>,				
				//setCB_aoOnly,    getCB_aoOnly, 
				dr, " group='DSSAO' label='aoOnly' ");

			TwAddVarCB(m_barHandle, "radLength",   TW_TYPE_FLOAT,   
				//setCB_radLength, getCB_radLength, 
				GuiCB::setter<float, DeferredRenderer, &DeferredRenderer::setDSSAOradLength>,
				GuiCB::getter<float, DeferredRenderer, &DeferredRenderer::getDSSAOradLength>,
				dr, " group='DSSAO' label='radLength' min=0.01 max=100 step=0.01 ");
						
			TwAddVarCB(m_barHandle, "diskDisplacement", TW_TYPE_FLOAT,
				//setCB_diskDisplacement, getCB_diskDisplacement, 
				GuiCB::setter<float, DeferredRenderer, &DeferredRenderer::setDSSAOdiskDisplacement>,
				GuiCB::getter<float, DeferredRenderer, &DeferredRenderer::getDSSAOdiskDisplacement>,
				dr, " group='DSSAO' label='diskDisplacement' min=0 max=1 step=0.01 ");
			TwAddVarCB(m_barHandle, "distMax",     TW_TYPE_FLOAT,     
				//setCB_distMax,   getCB_distMax, 
				GuiCB::setter<float, DeferredRenderer, &DeferredRenderer::setDSSAOdistMax>,
				GuiCB::getter<float, DeferredRenderer, &DeferredRenderer::getDSSAOdistMax>,				
				dr, " group='DSSAO' label='distMax' min=0 max=10 step=0.01 ");

			TwAddVarCB(m_barHandle, "ilComputation",   ilModeType,
				GuiCB::setter<D2DilShaderSub::OptionValues, DeferredRenderer, &DeferredRenderer::setIlComputationId>,
				GuiCB::getter<D2DilShaderSub::OptionValues, DeferredRenderer, &DeferredRenderer::getIlComputationId>,				
				//setCB_ilComputation, getCB_ilComputation,
				dr, " group='IndLight' label='ilComputation' ");

			TwAddVarCB(m_barHandle, "IL/samples",   TW_TYPE_INT32,        
				GuiCB::setter<int, DeferredRenderer, &DeferredRenderer::setILsamples>,
				GuiCB::getter<int, DeferredRenderer, &DeferredRenderer::getILsamples>,				
				//setCB_ilSamples,   getCB_ilSamples,
				dr, " group='IndLight' label='samples' min=8 max=64 step=8 ");
			
			TwAddVarCB(m_barHandle, "IL/radLength",   TW_TYPE_FLOAT,
				GuiCB::setter<float, DeferredRenderer, &DeferredRenderer::setILradLength>,
				GuiCB::getter<float, DeferredRenderer, &DeferredRenderer::getILradLength>,			
				//setCB_ilRadLength, getCB_ilRadLength,
				dr, " group='IndLight' label='radLength' min=0.01 max=100 step=0.01 ");

			TwAddVarCB(m_barHandle, "IL/diskDisplacement", TW_TYPE_FLOAT, 
				GuiCB::setter<float, DeferredRenderer, &DeferredRenderer::setILdiskDisplacement>,
				GuiCB::getter<float, DeferredRenderer, &DeferredRenderer::getILdiskDisplacement>,
				//setCB_ilDiskDisplacement, getCB_ilDiskDisplacement,
				dr, " group='IndLight' label='diskDisplacement' min=0 max=1 step=0.01 ");

			TwAddVarCB(m_barHandle, "IL/distMax",     TW_TYPE_FLOAT,
				GuiCB::setter<float, DeferredRenderer, &DeferredRenderer::setILdistMax>,
				GuiCB::getter<float, DeferredRenderer, &DeferredRenderer::getILdistMax>,
				//setCB_ilDistMax,   getCB_ilDistMax,
				dr, " group='IndLight' label='distMax' min=0 max=10 step=0.01 ");

			TwAddVarCB(m_barHandle, "DDO/samples", TW_TYPE_INT32,       
				GuiCB::setter<int, DeferredRenderer, &DeferredRenderer::setDDOsamples>,
				GuiCB::getter<int, DeferredRenderer, &DeferredRenderer::getDDOsamples>,
				//setCB_ddoSamples,   getCB_ddoSamples,
				dr, " group='DDO' label='samples' min=8 max=64 step=8 ");
			TwAddVarCB(m_barHandle, "DDO/radLength", TW_TYPE_FLOAT, 
				GuiCB::setter<float, DeferredRenderer, &DeferredRenderer::setDDOradLength>,
				GuiCB::getter<float, DeferredRenderer, &DeferredRenderer::getDDOradLength>,				
				//setCB_ddoRadLength, getCB_ddoRadLength, 
				dr, " group='DDO' label='radLength' min=0.01 max=10 step=0.01 ");
			TwAddVarCB(m_barHandle, "DDO/diskDisplacement", TW_TYPE_FLOAT,
				GuiCB::setter<float, DeferredRenderer, &DeferredRenderer::setDDOdiskDisplacement>,
				GuiCB::getter<float, DeferredRenderer, &DeferredRenderer::getDDOdiskDisplacement>,
				//setCB_ddoDiskDisplacement, getCB_ddoDiskDisplacement,
				dr, " group='DDO' label='diskDisplacement' min=0 max=1 step=0.01 ");
			TwAddVarCB(m_barHandle, "DDO/distMax", TW_TYPE_FLOAT,
				GuiCB::setter<float, DeferredRenderer, &DeferredRenderer::setDDOdistMax>,
				GuiCB::getter<float, DeferredRenderer, &DeferredRenderer::getDDOdistMax>,				
				//setCB_ddoDistMax,   getCB_ddoDistMax,
				dr, " group='DDO' label='distMax' min=0 max=10 step=0.01 ");
			
		}
	};
private:
	

};

#endif