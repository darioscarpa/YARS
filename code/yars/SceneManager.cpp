#include "SceneManager.h"

#include "Sandbox.h"

#include "Scene.h"
#include "Asset.h"

#include "SceneNode.h"
#include "CameraNode.h"
#include "LightNode.h"
#include "AssetNode.h"

#include <string>

SceneManager::SceneManager() { }

Scene *SceneManager::getScenePtr(int id) {
	//if (id >= m_scenesMap.size()) return nullptr; //TODO
	auto& it = m_scenesMap.begin();
    std::advance(it, id);
	return (*it).second.get();
}

int SceneManager::getSceneId(Scene *ptr) {
	int id = 0;
	for ( auto& s : m_scenesMap ) {
		if ( s.second.get() == ptr ) return id;
		id++;
	}
}

void SceneManager::addScene(std::unique_ptr<Scene> sp) {
	Scene *s = sp.get();
	auto label = s->getLabel();
	
	while (true) {
		auto it = m_scenesMap.find(label);
		if (it == m_scenesMap.end()) {
			s->setLabel(label);
			m_scenesMap.insert(std::make_pair( label, std::move(sp) ));
			break;
		}
		label = label + "_";		
	}
}

void SceneManager::remScene(Scene *s) {
	m_scenesMap.erase(s->getLabel());
}

bool SceneManager::empty() const {
	return m_scenesMap.empty();
}

int SceneManager::numScenes() const {
	return m_scenesMap.size();
}

