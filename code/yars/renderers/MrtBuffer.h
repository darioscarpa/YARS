#ifndef MRTBUFFER_H
#define MRTBUFFER_H

// OPENGL
#include "../util/inc_opengl.h"

// STL
#include <vector>
#include <map>
#include <string>

class MrtBufferType {
public:
	enum OptionValues {
		color_r8_b,
		color_r16_f,
		color_r32_f,		
		colora_rgba16_f,
		colora_rgba32_f,
		depthStencil_24_8_uint,
		depth16_f,
		depth24_f,
		depth32_f,		
		SIZE
	};

	std::string label;
	GLint  internalFormat;
	GLenum format;
	GLenum type;
	MrtBufferType(const std::string& lbl, GLint cbif, GLenum cbf, GLenum cbt) : label(lbl), internalFormat(cbif), format(cbf), type(cbt) {};
	MrtBufferType() {};
};

class MrtBuffer {
public:	
	MrtBuffer(int numColorBufs, bool withDepth, bool withDepthStencil = false) : m_numBuffers(numColorBufs), m_hasDepth(withDepth), m_hasDepthStencil(withDepthStencil) {
		m_fbo = 0;
		m_depthTexture = 0;
    
		m_textures.resize(m_numBuffers);		
	};
	~MrtBuffer() {
		destroy();
	};

	void init(unsigned int WindowWidth, unsigned int WindowHeight, int firstTexOffset = 0,
			  MrtBufferType::OptionValues colorBufFmt = MrtBufferType::colora_rgba16_f, 
			  MrtBufferType::OptionValues depthBufFmt = MrtBufferType::depth24_f);
		      //GLint colorBufInternalFormat = GL_RGBA16F, GLenum colorBufFormat = GL_RGBA, GLenum colorBufType = GL_FLOAT,
			  //GLint depthBufInternalFormat = GL_DEPTH24_STENCIL8, GLenum depthBufFormat = GL_DEPTH_STENCIL, GLenum depthBufType = GL_UNSIGNED_INT_24_8); //TODO throws exception
	void destroy();

	inline void unbind()         { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); };

	inline void bindForWriting() { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo); };
   
	inline void bindForReading() { glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo); };
	inline void bindForReadingAsTextures() {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		for (int i = 0; i < m_numBuffers; ++i) {
			//printf("textureUnit %d -> texture %d\n",  m_startTextureUnit + i, m_textures[i]);
			glActiveTexture(GL_TEXTURE0 + m_startTextureUnit + i);	
			glBindTexture(GL_TEXTURE_2D, m_textures[i]);
		}

		if (m_hasDepthStencil || m_hasDepth) {		
			glActiveTexture(GL_TEXTURE0 + m_numBuffers);
			glBindTexture(GL_TEXTURE_2D, m_depthTexture);
		}
	}

	inline void bindColorBufferToTexUnit(int colorBuffer, int texUnit) { 
		glActiveTexture(GL_TEXTURE0 + texUnit);	
		glBindTexture(GL_TEXTURE_2D, m_textures[colorBuffer]);
	}
	inline void bindDepthStencilBufferToTexUnit(int texUnit) {		
		glActiveTexture(GL_TEXTURE0 + texUnit);
		glBindTexture(GL_TEXTURE_2D, m_depthTexture);
	}

	inline void setReadBuffer(int colorAttachment) { glReadBuffer(GL_COLOR_ATTACHMENT0 + colorAttachment); }	

	void printFramebufferInfo(GLenum target);

	static void printMaxColorAttachments() { 
		int res;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &res);
		printf("Max Color Attachments: %d\n", res);
	}
			
	static const std::vector<MrtBufferType>& getBufTypes() { return bufTypePresets; }		

	inline void setColorBufferType(MrtBufferType::OptionValues t) { m_colorBufType = t; reinit(); }
	inline void setDepthBufferType(MrtBufferType::OptionValues t) { m_depthBufType = t; reinit(); }
	
	inline MrtBufferType::OptionValues getColorBufferType() const { return m_colorBufType; }
	inline MrtBufferType::OptionValues getDepthBufferType() const { return m_depthBufType; }

	inline void resize(int winWidth, int winHeight) { m_winWidth = winWidth; m_winHeight = winHeight; reinit(); }

	inline void reinit() { 
		destroy();
		init(m_winWidth, m_winHeight, m_startTextureUnit, m_colorBufType, m_depthBufType);	
	}

	

private:
	
	static std::vector<MrtBufferType> createBufTypePresets() {
		std::vector<MrtBufferType> v;
		v.resize(MrtBufferType::OptionValues::SIZE);
		
		v[MrtBufferType::color_r8_b] = MrtBufferType("r8_b", GL_R8, GL_RED, GL_UNSIGNED_BYTE);
		v[MrtBufferType::color_r16_f] = MrtBufferType("r16_f", GL_R16F, GL_RED, GL_HALF_FLOAT);
		v[MrtBufferType::color_r32_f] = MrtBufferType("r32_f", GL_R32F, GL_RED, GL_FLOAT);
		
		v[MrtBufferType::colora_rgba16_f] = MrtBufferType("rgba16_f", GL_RGBA16F, GL_RGBA, GL_FLOAT);
		v[MrtBufferType::colora_rgba32_f] = MrtBufferType("rgba32_f", GL_RGBA32F, GL_RGBA, GL_FLOAT);
		v[MrtBufferType::depthStencil_24_8_uint] = MrtBufferType("depthStencil_24_8_uint", GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
		v[MrtBufferType::depth32_f] = MrtBufferType("depth32_f", GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT);		
		v[MrtBufferType::depth24_f] = MrtBufferType("depth24_f", GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT);
		v[MrtBufferType::depth16_f] = MrtBufferType("depth16_f", GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_FLOAT);		
		
		return v;
	}
	static const std::vector<MrtBufferType> bufTypePresets;
		
	/*
	static std::map<std::string, MrtBufferType> createBufTypePresets() {
		std::map<std::string, MrtBufferType> m;
		m.insert(std::make_pair("colora_rgba16_f", MrtBufferType(GL_RGBA16F, GL_RGBA, GL_FLOAT)));
		m.insert(std::make_pair("colora_rgba32_f", MrtBufferType(GL_RGBA32F, GL_RGBA, GL_FLOAT)));
		m.insert(std::make_pair("depthStencil_24_8_uint", MrtBufferType(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8)));
		m.insert(std::make_pair("depthStencil_24_8_uint", MrtBufferType(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8)));		
		return m;
	}
	static const std::map<std::string, MrtBufferType> bufTypePresets;
	*/

	bool m_hasDepth;
	bool m_hasDepthStencil;
	
	int  m_numBuffers;
	int  m_startTextureUnit;

	// OGL handles
	GLuint m_fbo;
    std::vector<GLuint> m_textures;
    GLuint m_depthTexture;

	//MrtBufferType m_colorBufType;
	//MrtBufferType m_depthBufType;

	MrtBufferType::OptionValues m_colorBufType;
	MrtBufferType::OptionValues m_depthBufType;
	
	int m_winWidth;
	int m_winHeight;


};

#endif

