#include "TextureManager.h"

#include "Texture.h"

std::shared_ptr<Texture> TextureManager::loadTexture(const std::string& filename) {

	auto it = m_texturesMap.find(filename);

	if (it == m_texturesMap.end()) {
		std::shared_ptr<Texture> t = std::make_shared<Texture>(GL_TEXTURE_2D, filename);
		bool loadingOk = t.get()->loadAndBuildMipmaps();
		if (loadingOk) {
			m_texturesMap[filename] = t;
			return t;
		} else {
			throw new std::exception("texture load/mipmap build failed\n");
		}
	} else {
		return it->second;
	}
}

void TextureManager::removeUnusedTextures() {
	//for (auto t : m_texturesMap) {
	//	t.second.use_count
	//}
}