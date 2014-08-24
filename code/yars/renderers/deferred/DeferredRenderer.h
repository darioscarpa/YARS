#ifndef DEFERREDRENDERER_H
#define DEFERREDRENDERER_H

#include "../gBuffer.h"
#include "../MrtBuffer.h"

#include "../../Renderer.h"
#include "../../ShaderHandling.h"

#include "../shaders/sub/RandomGeneratorShaderSub.h"
#include "../shaders/sub/SamplingPatternShaderSub.h"
#include "../shaders/sub/D2DaoShaderSub.h"
#include "../shaders/sub/D2DilShaderSub.h"
#include "../shaders/sub/AreaCalculatorShaderSub.h"

#include "../HemisphereSampling.h"

class DRgeometryPassShader;

class DRlightPassShader;
class DRlightPassShaderPointLight;
class DRlightPassShaderSpotLight;
class DRlightPassShaderDirLight;

class DRindirectLightPassShader;
class DRsamplingTestPassShader;

class DRdiscDSSAOshader;

class FullScreenQuad;
class Texture;


class DeferredRenderer : public Renderer {
	public:	
		DeferredRenderer();

		void reshape(int awinW, int awinH);
		void render(const Scene& scene);
		
		void recursive_render_lights(glm::mat4 *vp, glm::mat4 currModelMatrix, SceneNode *n);
		
		void init(int winW, int winH);
		virtual void reinit();
		
		void destroy();
		void reloadShaders();

		/*inline void setActiveShaderProgram(ShaderProgram *sp) {
			if (activeShaderProgram != sp) {
				activeShaderProgram->deactivate();
				activeShaderProgram = sp;
				sp->use();			
			}
		};*/			

		// gbuffer
		void setShowingGBuffer(bool enabled);
		inline bool isShowingGBuffer() const { return showingGBuffer; }

		// AO stuff
		void setAO(bool enabled);
		inline bool isAOenabled() const  { return enabledAO; };		

		void setShowingOnlyAO(bool enabled);
		inline bool isShowingOnlyAO() const { return showingOnlyAO; }
				
		////////////////////////////////////////////////////////////////////////////
		void setDSSAOradLength(float val);
		void setDSSAOdiskDisplacement(float val);
		void setDSSAOdistMax(float val);
		void setDSSAOsamples(int samples);		

		inline float getDSSAOradLength()        const { return dssao_radLength;        };
		inline float getDSSAOdiskDisplacement() const { return dssao_diskDisplacement; };
		inline float getDSSAOdistMax()          const { return dssao_distMax;          };
		inline int   getDSSAOsamples()          const { return dssao_samples;          };
		////////////////////////////////////////////////////////////////////////////
		void setILradLength(float val);
		void setILdiskDisplacement(float val);
		void setILdistMax(float val);
		void setILsamples(int samples);	

		inline float getILradLength()        const { return il_radLength;        };
		inline float getILdiskDisplacement() const { return il_diskDisplacement; };
		inline float getILdistMax()          const { return il_distMax;          };
		inline int   getILsamples()          const { return il_samples;          };
		////////////////////////////////////////////////////////////////////////////
		inline int   getSamplingScopeId()    const { return samplingScopeId;     };
		void         setSamplingScopeId(int id);
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

		void setSTradLength(float val);
		void setSTdiskDisplacement(float val);
		void setSTdistMax(float val);
		void setSTsamples(int samples);	

		inline float getSTradLength()        const { return samplingScopeId ? dssao_radLength        : il_radLength;        };
		inline float getSTdiskDisplacement() const { return samplingScopeId ? dssao_diskDisplacement : il_diskDisplacement; };
		inline float getSTdistMax()          const { return samplingScopeId ? dssao_distMax          : il_distMax;          };
		inline int   getSTsamples()          const { return samplingScopeId ? dssao_samples          : il_samples;          };
		////////////////////////////////////////////////////////////////////////////
		void setDDOradLength(float val);
		void setDDOdiskDisplacement(float val);
		void setDDOdistMax(float val);
		void setDDOsamples(int samples);	

		inline float getDDOradLength()        const { return ddo_radLength;        };
		inline float getDDOdiskDisplacement() const { return ddo_diskDisplacement; };
		inline float getDDOdistMax()          const { return ddo_distMax;          };
		inline int   getDDOsamples()          const { return ddo_samples;          };
		////////////////////////////////////////////////////////////////////////////

		// overloaded basic stuff
		//void setAmbientLightLevel(float level);
		//void setLightingEnabled(bool enabled);		
		
		void setNormalMapping(bool enabled);
		void setSpecularMapping(bool enabled);
		void setOpacityMapping(bool enabled);
		void setTextureMapping(bool enabled);	

		inline void setAmbientPass(bool enabled)       { ambientPassEnabled       = enabled; };
		bool isAmbientPassEnabled() const              { return ambientPassEnabled; }

		inline void setDirectLightPass(bool enabled)   { directLightPassEnabled   = enabled; };
		bool isDirectLightPassEnabled() const          { return directLightPassEnabled; }

		inline void setIndirectLightPass(bool enabled) { indirectLightPassEnabled = enabled; };
		bool isIndirectLightPassEnabled() const        { return indirectLightPassEnabled; }

		inline void setSamplingTestPass(bool enabled)  { samplingTestPassEnabled  = enabled; };
		bool isSamplingTestPassEnabled() const         { return samplingTestPassEnabled; }
		
		inline void setDiskDSSAOpass(bool enabled)     { diskDSSAOpassEnabled  = enabled; };
		bool isDiskDSSAOpassEnabled() const            { return diskDSSAOpassEnabled; }
		
		////////////
		void setHemisphereSamplingMode(HemisphereSampling::KernelMode_t samplingMode);
		HemisphereSampling::KernelMode_t getHemisphereSamplingMode() const {
			return bs_emisphereSamplingMode;
		};
		void setupEmisphereKernel_random(int, bool, bool);
		void setupEmisphereKernel_triangleHierarchy(int, bool, bool);
		
		void setUniformSamplingRadius(bool tf);
		inline bool isUniformSamplingRadius() const { return bs_emisphereAllPointsOnSurface; };

	private:	
		DRgeometryPassShader        *gpShader;
		DRlightPassShader           *lpShader;
		
		DRlightPassShaderPointLight *lpPointShader;
		DRlightPassShaderSpotLight  *lpSpotShader;
		DRlightPassShaderDirLight   *lpDirShader;
		
		DRindirectLightPassShader   *ilpShader;
		DRsamplingTestPassShader    *stShader;

		DRdiscDSSAOshader           *ddoShader;

		Shader::compConstMap_t      shadersCompileConstants;

		FullScreenQuad *quad;
		Texture *randomTexture;	
				
		//GBuffer gbuf;
		MrtBuffer gbuf;

		void showGBuffer();
		bool showingGBuffer;		

		bool ambientPassEnabled;		
		bool directLightPassEnabled;
		bool indirectLightPassEnabled;
		bool samplingTestPassEnabled;	
		bool diskDSSAOpassEnabled;


		//ShaderProgram *activeShaderProgram;

		// AO
		bool  enabledAO;
		bool  showingOnlyAO;
		// DSSAO settings
		float dssao_radLength; 
		float dssao_diskDisplacement;
		float dssao_distMax;
		int   dssao_samples;	

		// IL settings
		float il_radLength; 
		float il_diskDisplacement;
		float il_distMax;
		int   il_samples;	
			
		/////////////
		int   samplingScopeId; 
		//RandomGeneratorShaderSub::OptionValues  randomGenerationId;
		SamplingPatternShaderSub::OptionValues  samplingPatternId;
		D2DaoShaderSub::OptionValues            aoComputationId;
		D2DilShaderSub::OptionValues            ilComputationId;
		AreaCalculatorShaderSub::OptionValues   areaComputationId;

		//emisphere sampling settings		
		HemisphereSampling::KernelMode_t   bs_emisphereSamplingMode;
		bool  bs_emisphereAllPointsOnSurface; // points on emisphere surface or into emisphere
		//bool  bs_distanceFalloff;    //  sample density scaled with distance		


		/////////////
		// disk DSSAO
		float ddo_radLength; 
		float ddo_diskDisplacement;
		float ddo_distMax;
		int   ddo_samples;	

};

#endif