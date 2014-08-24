#ifndef RENDERER_H
#define RENDERER_H

#include "Asset.h"
#include "Scene.h"

#include "ShaderSharedData.h"

class Renderer {
	public:	
		Renderer();
		virtual ~Renderer();
		
		void setLabel(const std::string& l) { label = l; }
		const std::string& getLabel() { return label; }

		virtual void init(int awinW, int awinH) = 0;		
		virtual void reinit() { init(winW, winH); };
		virtual void reshape(int winW, int winH);
		virtual	void destroy() = 0;		
		virtual void reloadShaders() = 0;
				
		virtual void render(const Scene& scene) = 0;
		virtual void recursive_render(glm::mat4 *vp, glm::mat4 currModelMatrix, SceneNode *n);
		
		void renderSceneGeometry(const Scene& scene);

		inline void setBackgroundColor(const glm::vec3& bg) { backgroundColor = bg;	 }
		inline const glm::vec3& getBackgroundColor() const  { return backgroundColor; }

		const glm::mat4& const getModelMatrix() { return Model; };
		const glm::mat4&  const getViewMatrix()  { return *View;   };
				
		
		virtual void setLightingEnabled(bool enabled) { enabledLighting        = enabled; };
		virtual void setNormalMapping(bool enabled)   { enabledNormalMapping   = enabled; };
		virtual void setSpecularMapping(bool enabled) { enabledSpecularMapping = enabled; };
		virtual void setOpacityMapping(bool enabled)  { enabledOpacityMapping  = enabled; };
		virtual void setTextureMapping(bool enabled)  { enabledTextureMapping  = enabled; }; 

		inline bool  isNormalMappingEnabled()   const { return enabledNormalMapping; }
		inline bool  isSpecularMappingEnabled() const { return enabledSpecularMapping; }
		inline bool  isOpacityMappingEnabled()  const { return enabledOpacityMapping; }
		inline bool  isTextureMappingEnabled()  const { return enabledTextureMapping; }

		inline bool  isLightingEnabled()        const { return enabledLighting; }
				
		void setMVPMatrix(const glm::mat4& mvpMatrix);
		void setModelMatrix(const glm::mat4& modelMatrix);
		void setProjectionMatrix(const glm::mat4& projMatrix);
		void setInvProjectionMatrix(const glm::mat4 &invProjMatrix);
		void setViewMatrix(const glm::mat4& viewMatrix);
		void setModelViewMatrix(const glm::mat4& mvMatrix);	

		bool isMultisamplingEnabled() const;
		void setMultisampling(const bool enabled);
		int  multisamplingSamples() const;

		//void setTextureFiltering(bool enabled);
		//bool isTextureFilteringEnabled() const  { return enabledTextureFiltering; }
		
		int surfaceHeight() const { return winH; };
		int surfaceWidth()  const { return winW; };
		
	protected:
		int winH, winW;	

		ShaderSharedData ssd;

		// matrices
		glm::mat4 Model;
		glm::mat4 *View;
		glm::mat4 *Projection;
		glm::mat4 *InvProjection;
		
		//glm::mat4 ModelView;
		//glm::mat4 MVP;
		
		//bool mustUpdateProjectionMatrix; // update proj only on aspect ratio change

		float      numLights[4]; // 0: tot, 1: pl, 2: sl, 3; dl
		PointLight pointLights[POINT_LIGHTS_MAX];
		SpotLight  spotLights[SPOT_LIGHTS_MAX];
		DirLight   dirLights[DIR_LIGHTS_MAX];

		void setLights(const glm::mat4& viewMatrix, const Scene& scene);		
				
	private:
		Renderer(const Renderer&);
		Renderer& operator=(const Renderer&);

		glm::vec3 backgroundColor;		

		bool  enabledLighting;
		float ambientLightLevel;

		bool  enabledNormalMapping;
		bool  enabledSpecularMapping;
		bool  enabledOpacityMapping;
		bool  enabledTextureMapping;

		//bool  enabledTextureFiltering;

		std::string label;
};

#endif