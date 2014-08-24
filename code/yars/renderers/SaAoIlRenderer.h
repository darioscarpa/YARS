/*
#ifndef SAAOILRENDERER_H
#define SAAOILRENDERER_H

#include "SaAoIlGbuffer.h"
#include "aoBuffer.h"

#include "MrtBuffer.h"

#include "../Renderer.h"
#include "../ShaderHandling.h"

#include "HemisphereSampling.h"

#include "shaders/SaAoIlGeometryPassShader.h"
#include "shaders/SaAoIlProcessingPassShader.h"
#include "shaders/SaAoIlFilteringPassShader.h"

class FullScreenQuad;
class Texture;


class SaAoIlRenderer : public Renderer {
	public:
		SaAoIlRenderer();

		void reshape(int awinW, int awinH);
		void render(const Scene& scene);

		void init(int winW, int winH);
		virtual void reinit();

		void destroy();
		void reloadShaders();

		// emisphere sampling //////////////////////////////////////////////////////////////////////
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

		void setGbColorBufferType(MrtBufferType::OptionValues t);
		MrtBufferType::OptionValues getGbColorBufferType() const;

		void setGbDepthBufferType(MrtBufferType::OptionValues t);
		MrtBufferType::OptionValues getGbDepthBufferType() const;

		void setAoColorBufferType(MrtBufferType::OptionValues t);
		MrtBufferType::OptionValues getAoColorBufferType() const;


		// gbuffer
		void setShowingAOBuffer(bool enabled);
		inline bool isShowingAOBuffer() const { return showingAOBuffer; }

		
		// area debug
		//void setShowingArea(bool enabled);
		//inline bool isShowingArea() const { return showingArea; }

		// area debug
		//void setShowingSampling(bool enabled);
		//inline bool isShowingSampling() const { return showingSampling; }

		//void setShowingSamplingDensity(bool enabled);
		//inline bool isShowingSamplingDensity() const { return showingSamplingDensity; }

		//void setShowingZoomedArea(bool enabled);
		//inline bool isShowingZoomedArea() const { return showingZoomedArea; }
		

		///////////////////////////////////////////////////////////////////////////////////////////
		// bilateral blur
		void setBlurKdepth(float kd);
		float getBlurKdepth() const { return blurKdepth; }

		void setBlurKnormal(float kn);
		float getBlurKnormal() const { return blurKnormal; }

		void setBlurKernelSize(int size);
		int getBlurKernelSize() const { return blurKernelSize; }

		void setBlurEnabled(bool enabled);
		bool isBlurEnabled() const { return blurEnabled; }
		
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

		void setDirectEnabled(bool enabled);
		inline bool isDirectEnabled() const  { return directLightEnabled; };

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

		void setTriangleSideDivs(int divs);
		int  getTriangleSideDivs() const;
	private:

		static const char* TRIANGLE_DIVS;

		SaAoIlGeometryPassShader        *gpShader;
		SaAoIlProcessingPassShader      *ppShader;
		SaAoIlFilteringPassShader       *bpShader;

		Shader::compConstMap_t			 shadersCompileConstants;

		FullScreenQuad *quad;
		Texture *randomTexture;

		//GdlBuffer gdlbuf;
		MrtBuffer gdlbuf;
		void showGBuffer();
		bool showingGBuffer;

		//AoBuffer aobuf;
		MrtBuffer ilaobuf;
		void showAOBuffer();
		bool showingAOBuffer;

		void showArea();
		bool showingArea;

		bool showingSampling;
		bool showingSamplingDensity;
		bool showingZoomedArea;

		float aoMultiplier;
		float ilMultiplier;
		float areaMultiplier;
		float solidAngleMultiplier;

		// blending debug
		bool albedoEnabled;
		bool aoEnabled;
		bool doEnabled;
		bool ilEnabled;
		bool ambientLightEnabled;
		bool directLightEnabled;

		// blur
		int    blurKernelSize;
		float  blurKdepth;
		float  blurKnormal;
		bool   blurEnabled;


		//emisphere sampling settings
		HemisphereSampling::KernelMode_t   bs_emisphereSamplingMode;
		bool  bs_emisphereAllPointsOnSurface; // points on emisphere surface or into emisphere

		glm::vec2 screenSpaceRayOrigin;
		glm::vec2 screenSpaceEmisphereCenter;

		float bs_radLength;
		float bs_diskDisplacement;
		float bs_distMax;
		int   bs_samples;

		/////////////
		SamplingPatternShaderSub::OptionValues  samplingPatternId;
		D2DaoShaderSub::OptionValues            aoComputationId;
		D2DilShaderSub::OptionValues            ilComputationId;
		AreaCalculatorShaderSub::OptionValues   areaComputationId;
};

#endif
*/

#ifndef SAAOILRENDERER_H
#define SAAOILRENDERER_H

#include "SaAoIlGbuffer.h"
#include "aoBuffer.h"

#include "SaAoRenderer.h"

#include "MrtBuffer.h"

#include "../Renderer.h"
#include "../ShaderHandling.h"

#include "HemisphereSampling.h"

#include "shaders/SaAoIlGeometryPassShader.h"
#include "shaders/SaAoIlProcessingPassShader.h"
#include "shaders/SaAoIlFilteringPassShader.h"

class FullScreenQuad;

class SaAoIlRenderer : public Renderer {
	public:
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		

		SaAoIlRenderer();

		void reshape(int awinW, int awinH);
		void render(const Scene& scene);

		void init(int winW, int winH);
		virtual void reinit();

		void destroy();
		void reloadShaders();

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

		//void initMrtBuffers(int awinW, int awinH);
		//void initShaderPrograms();
		
		// hemisphere sampling ////////////////////////////////////////////////////////////////////
		void setHemisphereSamplingMode(HemisphereSampling::KernelMode_t samplingMode);
		HemisphereSampling::KernelMode_t getHemisphereSamplingMode() const {
			return bs_emisphereSamplingMode;
		};
		void setupEmisphereKernel_random(int, bool, bool);
		void setupEmisphereKernel_triangleHierarchy(int, bool, bool);

		void setUniformSamplingRadius(bool tf);
		inline bool isUniformSamplingRadius() const { return bs_emisphereAllPointsOnSurface; };
		///////////////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////////////////
		// gBuffer
		void setShowingGBuffer(bool enabled);
		inline bool isShowingGBuffer() const { return showingGBuffer; }

		void setGbColorBufferType(MrtBufferType::OptionValues t);
		MrtBufferType::OptionValues getGbColorBufferType() const;

		void setGbDepthBufferType(MrtBufferType::OptionValues t);
		MrtBufferType::OptionValues getGbDepthBufferType() const;

		///////////////////////////////////////////////////////////////////////////////////////////
		// aoBuffer
		void setShowingAOBuffer(bool enabled);
		inline bool isShowingAOBuffer() const { return showingAOBuffer; }

		void setAoColorBufferType(MrtBufferType::OptionValues t);
		MrtBufferType::OptionValues getAoColorBufferType() const;
		///////////////////////////////////////////////////////////////////////////////////////////

		/*
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
		*/
				
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		// geometry pass settings
		void setNormalMapping(bool enabled);
		void setSpecularMapping(bool enabled);
		void setOpacityMapping(bool enabled);
		void setTextureMapping(bool enabled);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		// processing pass settings		
		
		void setScreenSpaceRayOrigin();
		void setScreenSpaceEmisphereCenter();
		
		void setTechnique(SaAoIlProcessingPassShader::Technique_t t);
		SaAoIlProcessingPassShader::Technique_t getTechnique() const { return m_technique; };				
				
		inline SamplingPatternShaderSub::OptionValues getSamplingPatternId() const { return samplingPatternId;  };
		void   setSamplingPatternId(SamplingPatternShaderSub::OptionValues id);
		
		inline D2DaoShaderSub::OptionValues getAoComputationId() const { return aoComputationId;  };
		void   setAoComputationId(D2DaoShaderSub::OptionValues id);		
		
		inline D2DilShaderSub::OptionValues getIlComputationId() const { return ilComputationId;  };
		void   setIlComputationId(D2DilShaderSub::OptionValues id);		
		
		////////////////////////////////////////////////////////////////////////////
		
		////////////////////////////////////////////////////////////////////////////
		// generic AO params
		void setAoMultiplier(float m);
		void setIlMultiplier(float m);
		void setAoSamplingRadius(float val);
		void setAoAngleBias(float val);
		void setAoMaxDistance(float val);
		void setAoNumSamples(int samples);

		inline float getAoMultiplier()     const { return aoMultiplier;   }
		inline float getIlMultiplier()   const   { return ilMultiplier;   }
		inline float getAoSamplingRadius() const { return aoSamplingRadius; };
		inline float getAoAngleBias()      const { return aoAngleBias;      };
		inline float getAoMaxDistance()    const { return aoMaxDistance;    };
		inline int   getAoNumSamples()     const { return aoNumSamples;     };

		////////////////////////////////////////////////////////////////////////////
		// saSSAO params
		void setTriangleSideDivs(int divs);
		int  getTriangleSideDivs() const;	

		inline AreaCalculatorShaderSub::OptionValues getAreaComputationId() const { return areaComputationId;  };
		void   setAreaComputationId(AreaCalculatorShaderSub::OptionValues id);
				
		void setAreaMultiplier(float m);
		inline float getAreaMultiplier()       const { return areaMultiplier; }
		
		void setSolidAngleMultiplier(float m);		
		inline float getSolidAngleMultiplier() const { return solidAngleMultiplier; }		
		
		////////////////////////////////////////////////////////////////////////////
		// Alchemy params
		void setAlchemyK(float val);
		void setAlchemyRO(float val);
		void setAlchemyU(float val);

		inline float getAlchemyK()  const { return alchemy_k;   };
		inline float getAlchemyRO() const { return alchemy_ro; };
		inline float getAlchemyU()  const { return alchemy_u; };
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		// filtering/compositing pass settings		
		
		////////////////////////////////////////////////////////////////////////////
		// compositing debug
		void setAlbedoEnabled(bool enabled);
		inline bool isAlbedoEnabled() const  { return albedoEnabled; };

		void setAoEnabled(bool enabled);
		inline bool isAoEnabled() const  { return aoEnabled; };

		void setIlEnabled(bool enabled);
		inline bool isIlEnabled() const  { return ilEnabled; };

		void setAmbientEnabled(bool enabled);
		inline bool isAmbientEnabled() const  { return ambientLightEnabled; };

		void setDirectEnabled(bool enabled);
		inline bool isDirectEnabled() const  { return directLightEnabled; };
				
		void setBlurEnabled(bool enabled);
		inline bool isBlurEnabled() const { return blurEnabled; }

		////////////////////////////////////////////////////////////////////////////
		// bilateral filtering
		void setBlurKdepth(float kd);
		float getBlurKdepth() const { return blurKdepth; }

		void setBlurKnormal(float kn);
		float getBlurKnormal() const { return blurKnormal; }

		void setBlurKernelSize(int size);
		int getBlurKernelSize() const { return blurKernelSize; }

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		


	protected :
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		// shaders stuff
		static const char* TRIANGLE_DIVS;
		Shader::compConstMap_t      shadersCompileConstants;

		
		SaAoIlGeometryPassShader        *geometryPassShader;
		SaAoIlProcessingPassShader      *processingPassShader;
		SaAoIlFilteringPassShader       *filteringPassShader;
		

		/*
		GfxShaderProgram *geometryPassShader;
		GfxShaderProgram *processingPassShader;
		GfxShaderProgram *filteringPassShader;
		*/
				
		SaAoIlProcessingPassShader::Technique_t m_technique;
	
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		// MRT buffers
		MrtBuffer gBuffer;
		MrtBuffer aoBuffer;
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		FullScreenQuad *quad;	
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		// shared AO params
		int   aoNumSamples;
		float aoSamplingRadius;
		float aoAngleBias;
		float aoMaxDistance;					
		float aoMultiplier;	
		float ilMultiplier;

		D2DaoShaderSub::OptionValues            aoComputationId;		
		D2DilShaderSub::OptionValues            ilComputationId;
		SamplingPatternShaderSub::OptionValues  samplingPatternId;		

		//emisphere sampling settings
		HemisphereSampling::KernelMode_t   bs_emisphereSamplingMode;
		bool  bs_emisphereAllPointsOnSurface; // points on emisphere surface or into emisphere
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		// alchemy params
		float alchemy_k;
		float alchemy_ro;
		float alchemy_u;
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		// saSSAO
		AreaCalculatorShaderSub::OptionValues   areaComputationId;		
		float areaMultiplier;
		float solidAngleMultiplier;		

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		// filtering params
		bool   blurEnabled;
		int    blurKernelSize;
		float  blurKdepth;
		float  blurKnormal;
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		// debug visualizations
		bool showingSampling;
		bool showingSamplingDensity;
		bool showingZoomedArea;
		glm::vec2 screenSpaceRayOrigin;
		glm::vec2 screenSpaceEmisphereCenter;
		void showGBuffer();
		bool showingGBuffer;
		void showAOBuffer();
		bool showingAOBuffer;
		void showArea();
		bool showingArea;
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		// compositing debug
		bool albedoEnabled;
		bool aoEnabled;
		bool ambientLightEnabled;
		bool directLightEnabled;
		bool ilEnabled;
};

#endif