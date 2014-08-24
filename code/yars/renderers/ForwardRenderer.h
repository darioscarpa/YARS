#ifndef FORWARDRENDERER_H
#define FORWARDRENDERER_H

#include "../Renderer.h"

class ForwardShader;
class ForwardShaderFlat;
class SceneNode;

class ForwardRenderer : public Renderer {
	public:		
		ForwardRenderer();

		void init(int winW, int winH);	
		void destroy();		
		void reloadShaders();
		
		void render(const Scene& scene);
		

		//void setAmbientLightLevel(float level);
		void setLightingEnabled(bool enabled);

		void setNormalMapping(bool enabled);
		void setSpecularMapping(bool enabled);
		void setOpacityMapping(bool enabled);
		void setTextureMapping(bool enabled);
	private:
		ForwardShader     *fwdShader;
		ForwardShaderFlat *fwdShaderFlat;
};

#endif