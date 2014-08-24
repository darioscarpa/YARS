#ifndef DOUBLEPASSBLENDINGDEFERREDRENDERER_H
#define DOUBLEPASSBLENDINGDEFERREDRENDERER_H

#include "../gBuffer.h"
#include "../deferred/DlBuffer.h"

#include "../MrtBuffer.h"

#include "../../Renderer.h"
#include "../../ShaderHandling.h"

#include "../shaders/sub/AreaCalculatorShaderSub.h"
#include "../shaders/sub/RandomGeneratorShaderSub.h"
#include "../shaders/sub/SamplingPatternShaderSub.h"
#include "../shaders/sub/D2DaoShaderSub.h"
#include "../shaders/sub/D2DilShaderSub.h"

#include "../HemisphereSampling.h"

class DRgeometryPassShader;

class DRlightPassShader;
class DRlightPassShaderPointLight;
class DRlightPassShaderSpotLight;
class DRlightPassShaderDirLight;

class DRdirectLightPassShader;
class DRblendingPassShader;

class DRindirectLightPassShader;
class DRsamplingTestPassShader;

class DRdiscDSSAOshader;

class FullScreenQuad;
class Texture;


class DoublePassBlendingDeferredRenderer : public Renderer {
	public:	
		DoublePassBlendingDeferredRenderer();

		void reshape(int awinW, int awinH);
		void render(const Scene& scene);
		
		void init(int winW, int winH);
		virtual void reinit();
		
		void destroy();
		void reloadShaders();
		
		// emisphere sampling //////////////////////////////////////////////////////////////////////
		
		enum Technique_t {
			SIMPLE_SUM,
			TRIANGLE_HIERARCHICAL_SKIP,
			PYRAMID_BUCKETS
		};
			
		void setTechnique(Technique_t t);
		Technique_t getTechnique() const { return technique; }


		void setHemisphereSamplingMode(HemisphereSampling::KernelMode_t samplingMode);
		HemisphereSampling::KernelMode_t getHemisphereSamplingMode() const {
			return bs_emisphereSamplingMode;
		};
		void setupEmisphereKernel_random(int, bool, bool);
		void setupEmisphereKernel_triangleHierarchy(int, bool, bool);
		
		void setUniformSamplingRadius(bool tf);
		inline bool isUniformSamplingRadius() const { return bs_emisphereAllPointsOnSurface; };

		void setScreenSpaceRayOrigin();
		void setScreenSpaceEmisphereCenter();
		///////////////////////////////////////////////////////////////////////////////////////////
		
		// gbuffer
		void setShowingGBuffer(bool enabled);
		inline bool isShowingGBuffer() const { return showingGBuffer; }

		// dlbuffer
		void setShowingDLBuffer(bool enabled);
		inline bool isShowingDLBuffer() const { return showingDLBuffer; }

		// area debug
		void setShowingArea(bool enabled);
		inline bool isShowingArea() const { return showingArea; }

		// area debug
		void setShowingSampling(bool enabled);
		inline bool isShowingSampling() const { return showingSampling; }

		void setShowingSamplingDensity(bool enabled);
		inline bool isShowingSamplingDensity() const { return showingSamplingDensity; }

		void setShowingZoomedArea(bool enabled);
		inline bool isShowingZoomedArea() const { return showingZoomedArea; }
				
		////////////////////////////////////////////////////////////////////////////
		//inline int   getSamplingScopeId()    const { return samplingScopeId;     };
		//void         setSamplingScopeId(int id);
		////////////////////////////////////////////////////////////////////////////
		//inline RandomGeneratorShaderSub::OptionValues getRandomGenerationId() const { return randomGenerationId;  };
		//void   setRandomGenerationId(RandomGeneratorShaderSub::OptionValues id);
		////////////////////////////////////////////////////////////////////////////
		inline SamplingPatternShaderSub::OptionValues getSamplingPatternId() const { return samplingPatternId;  };
		void   setSamplingPatternId(SamplingPatternShaderSub::OptionValues id);
		////////////////////////////////////////////////////////////////////////////
		inline D2DaoShaderSub::OptionValues getAoComputationId() const { return aoComputationId;  };
		void   setAoComputationId(D2DaoShaderSub::OptionValues id);
		////////////////////////////////////////////////////////////////////////////
		inline D2DilShaderSub::OptionValues getIlComputationId() const { return ilComputationId;  };
		void   setIlComputationId(D2DilShaderSub::OptionValues id);
		////////////////////////////////////////////////////////////////////////////
		inline AreaCalculatorShaderSub::OptionValues getAreaComputationId() const { return areaComputationId;  };
		void   setAreaComputationId(AreaCalculatorShaderSub::OptionValues id);

		////////////////////////////////////////////////////////////////////////////
	/*	void DoublePassBlendingDeferredRenderer::setTriangleSideDivs(int divs) {
			std::ostringstream v; v << divs;
			shadersCompileConstants[TRIANGLE_DIVS] = v.str();
		}
		int DoublePassBlendingDeferredRenderer::getTriangleSideDivs() const {
			return std::stoi(shadersCompileConstants[TRIANGLE_DIVS]);
		}*/
		void setTriangleSideDivs(int divs);
		int getTriangleSideDivs() const;


		void setAoMultiplier(float m);
		void setIlMultiplier(float m);
		void setAreaMultiplier(float m);
		void setSolidAngleMultiplier(float m);

		inline float getAoMultiplier()   const { return aoMultiplier;   }
		inline float getIlMultiplier()   const { return ilMultiplier;   }
		inline float getAreaMultiplier() const { return areaMultiplier; }
		inline float getSolidAngleMultiplier() const { return solidAngleMultiplier; }

		// blending debug
		
		void setAlbedoEnabled(bool enabled);
		inline bool isAlbedoEnabled() const  { return albedoEnabled; };		

		void setAoEnabled(bool enabled);
		inline bool isAoEnabled() const  { return aoEnabled; };		

		void setIlEnabled(bool enabled);
		inline bool isIlEnabled() const  { return ilEnabled; };		

		void setAmbientEnabled(bool enabled);
		inline bool isAmbientEnabled() const  { return ambientLightEnabled; };		

		void setDiffuseEnabled(bool enabled);
		inline bool isDiffuseEnabled() const  { return diffuseLightEnabled; };		

		void setSpecularEnabled(bool enabled);
		inline bool isSpecularEnabled() const  { return specularLightEnabled; };

		////////////////////////////////////////////////////////////////////////////
		void setBSmaxSamplingRadius(float val);
		void setBSdiskDisplacement(float val);
		void setBSdistMax(float val);
		void setBSnumSamples(int samples);	

		inline float getBSmaxSamplingRadius()const { return bs_radLength;        };
		inline float getBSdiskDisplacement() const { return bs_diskDisplacement; };
		inline float getBSdistMax()          const { return bs_distMax;          };
		inline int   getBSnumSamples()       const { return bs_samples;          };
		////////////////////////////////////////////////////////////////////////////
				
		void setNormalMapping(bool enabled);
		void setSpecularMapping(bool enabled);
		void setOpacityMapping(bool enabled);
		void setTextureMapping(bool enabled);	

		inline void setIndirectLightPass(bool enabled) { indirectLightPassEnabled = enabled; };
		bool isIndirectLightPassEnabled() const        { return indirectLightPassEnabled; }

		inline void setSamplingTestPass(bool enabled)  { samplingTestPassEnabled  = enabled; };
		bool isSamplingTestPassEnabled() const         { return samplingTestPassEnabled; }		

	private:	
		static const char* TRIANGLE_DIVS;

		DRgeometryPassShader        *gpShader;
		DRdirectLightPassShader     *dlpShader;		
		DRblendingPassShader        *blendShader;

		Shader::compConstMap_t      shadersCompileConstants;

		FullScreenQuad *quad;
		Texture *randomTexture;	
				
		
		MrtBuffer gbuf;
		void showGBuffer();
		bool showingGBuffer;
		
		MrtBuffer dlBuf;
		void showDLBuffer();
		bool showingDLBuffer;

		void showArea();
		bool showingArea;

		bool showingSampling;
		bool showingSamplingDensity;
		bool showingZoomedArea;		

		bool ambientPassEnabled;		
		bool directLightPassEnabled;
		bool indirectLightPassEnabled;
		bool samplingTestPassEnabled;	
		
		float aoMultiplier;
		float ilMultiplier;
		float areaMultiplier;
		float solidAngleMultiplier;

		// blending debug
		bool albedoEnabled;
		bool aoEnabled;
		bool ilEnabled;
		bool ambientLightEnabled;
		bool diffuseLightEnabled;
		bool specularLightEnabled;
		
		
		Technique_t technique;
		//emisphere sampling settings		
		HemisphereSampling::KernelMode_t   bs_emisphereSamplingMode;
		bool  bs_emisphereAllPointsOnSurface; // points on emisphere surface or into emisphere
		//bool  bs_distanceFalloff;    //  sample density scaled with distance		

		glm::vec2 screenSpaceRayOrigin;
		glm::vec2 screenSpaceEmisphereCenter;

		float bs_radLength; 
		float bs_diskDisplacement;
		float bs_distMax;
		int   bs_samples;
			
		/////////////
		int   samplingScopeId; 
		//RandomGeneratorShaderSub::OptionValues  randomGenerationId;
		SamplingPatternShaderSub::OptionValues  samplingPatternId;
		D2DaoShaderSub::OptionValues            aoComputationId;
		D2DilShaderSub::OptionValues            ilComputationId;
		AreaCalculatorShaderSub::OptionValues   areaComputationId;

};

#endif