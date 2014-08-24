#ifndef SAAOILRENDERER_H
#define SAAOILRENDERER_H

#include "SaAoRenderer.h"


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


class SaAoIlRenderer : public SaAoRenderer {
	public:
		SaAoIlRenderer();

		void reshape(int awinW, int awinH);
		void render(const Scene& scene);

		void init(int winW, int winH);
		virtual void reinit();

		void destroy();
		void reloadShaders();

		void setIlEnabled(bool enabled);
		inline bool isIlEnabled() const  { return ilEnabled; };

		inline D2DilShaderSub::OptionValues getIlComputationId() const { return ilComputationId;  };
		void   setIlComputationId(D2DilShaderSub::OptionValues id);

		void   setIlMultiplier(float m);
		inline float getIlMultiplier()   const { return ilMultiplier;   }

	private:
		float ilMultiplier;		
		bool ilEnabled;
		D2DilShaderSub::OptionValues            ilComputationId;
};

#endif