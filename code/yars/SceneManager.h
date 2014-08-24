#ifndef SCENEMANAGER_H
#define	SCENEMANAGER_H

// STL
#include <string>
#include <map>
#include <memory>

class Scene;
class SceneNode;

class SceneManager {

public: 		
		SceneManager();
		
		void addScene(std::unique_ptr<Scene> s);
		void remScene(Scene *s);

		bool empty()     const;
		int  numScenes() const;

		Scene *getScenePtr(int id);
		int    getSceneId(Scene *ptr);
private:
		std::map<std::string, std::unique_ptr<Scene>> m_scenesMap;
};

#endif