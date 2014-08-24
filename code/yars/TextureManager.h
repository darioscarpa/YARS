#ifndef TEXTUREMANAGER_H
#define	TEXTUREMANAGER_H

// STL
#include <string>
#include <map>
#include <memory>

class Texture;

class TextureManager {

public: 		
		TextureManager() {};
		
		//void addTexture(Texture *t);
		//void remTexture(Texture *t);

		std::shared_ptr<Texture> loadTexture(const std::string& filename);	
		void removeUnusedTextures();

		//Texture *getTextureByName(const std::string& filename);
		//Texture *getTextureById(GLuint id);
		//int      getTextureId(Texture *ptr);

		void	 setTextureFiltering(bool enabled);
		
		bool empty()       const;
		int  numTextures() const;
private:
		std::map<std::string, std::shared_ptr<Texture>> m_texturesMap;
};

#endif