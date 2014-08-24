
#ifndef TEXTURE_H
#define	TEXTURE_H

// OPENGL
#include "util/inc_opengl.h"

// DevIL
#include <IL/il.h>

// STL
#include <string>

class Texture {
public:
	enum TextureClampMode_t {
		CLAMP_TO_EDGE, CLAMP_TO_BORDER, REPEAT
	};
	
	Texture(GLenum textureTarget, const std::string& filename) : m_textureTarget(textureTarget), m_filename(filename), m_hasMipmaps(false) {};
	//~Texture();

	bool load()					{ return loadFromFile(false); };
	bool loadAndBuildMipmaps()  { return loadFromFile(true);  };
    
	inline GLuint getId() const { return m_textureId;         };
	
	inline void bind(GLenum textureUnit) {
		glActiveTexture(textureUnit);
		glBindTexture(m_textureTarget, m_textureId);
	}

	static void setWrapMode(GLenum textureUnit, TextureClampMode_t);
	//static void setFilteringEnabled(GLenum textureUnit, bool enabled);
	
private:
    std::string m_filename;
    
	GLenum m_textureTarget;
    GLuint m_textureId;   
	bool   m_hasMipmaps;

	bool loadFromFile(bool buildMipmaps);	
};

#endif
