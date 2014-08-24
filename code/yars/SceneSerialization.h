#ifndef SCENESERIALIZATION_H
#define SCENESERIALIZATION_H

// RAPIDJSON
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filestream.h>
#include <rapidjson/stringbuffer.h>	

// MATH
#include "util/inc_math.h"

// STL
#include <string>
#include <iostream>
#include <fstream>
#include <memory>

class Scene;
class SceneNode;
class CameraPath;


class SceneSerialization {
public:
	static std::unique_ptr<Scene> load(const std::string& filename);
	static void   save(const std::string& filename, const Scene& scene);

private:
	static const std::string scenesFolder;
	static const std::string fileExt;

	static std::string getSceneFilename(const std::string& filename);

	static Scene *SceneDeserialize(const std::string& jsonStr, Scene *s);
	static SceneNode * SceneNodeDeserialize(const std::string& json, Scene *scene, SceneNode *father);
	static CameraPath* CameraPathDeserialize(const std::string& json);
	static void SceneNodeSerialize(SceneNode *sn, rapidjson::PrettyWriter<rapidjson::FileStream>& writer);

	static void writeVec3(glm::vec3 v, rapidjson::PrettyWriter<rapidjson::FileStream>& writer);
	static void writeQuat(glm::quat q, rapidjson::PrettyWriter<rapidjson::FileStream>& writer);
};

#endif