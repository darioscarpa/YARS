#include "SceneSerialization.h"

#include "SceneNode.h"
#include "Scene.h"
#include "Asset.h"
#include "AssetNode.h"
#include "CameraNode.h"
#include "CameraPath.h"
#include "FpsCameraNode.h"
#include "LightNode.h"
#include "Sandbox.h"

using namespace rapidjson;

const std::string SceneSerialization::scenesFolder = "_scenes/";
const std::string SceneSerialization::fileExt      = ".json";

// first level
static const char * const JSON_ambientLightLevel = "ambientLightLevel";
static const char * const JSON_backgroundColor   = "backgroundColor";
static const char * const JSON_cameraPaths       = "cameraPaths";
static const char * const JSON_nodes             = "nodes";

static const char * const JSON_camPathDuration = "duration";
static const char * const JSON_camPathKnots    = "knots";
static const char * const JSON_camPathKnotPos  = "pos";
static const char * const JSON_camPathKnotOri  = "ori";

static const char * const JSON_label           = "label";
static const char * const JSON_type            = "type";
static const char * const JSON_type_assetNode  = "AssetNode";
static const char * const JSON_type_animatedAssetNode = "AnimatedAssetNode";
static const char * const JSON_type_cameraNode = "CameraNode";
static const char * const JSON_type_fpscameraNode = "FpsCameraNode";
static const char * const JSON_type_lightNode  = "LightNode";
static const char * const JSON_type_sceneNode  = "SceneNode";

static const char * const JSON_position     = "position";
static const char * const JSON_orientation  = "orientation";
static const char * const JSON_scaling      = "scaling";
static const char * const JSON_children     = "children";

//assets
static const char * const JSON_assetFilename = "assetFilename";

static const char * const JSON_animatedAssetFilenamePrefix = "animatedAssetFilenamePrefix";
static const char * const JSON_animatedAssetFilenameSuffix = "animatedAssetFilenameSuffix";
static const char * const JSON_animatedAssetNumFrames = "animatedAssetNumFrames";
static const char * const JSON_animatedAssetFrameRate = "animatedAssetFrameRate";

//cameras
static const char * const JSON_fov          = "fov";
static const char * const JSON_aspectRatio  = "aspectRatio";
static const char * const JSON_nearClip     = "nearClip";
static const char * const JSON_farClip      = "farClip";

//lights
static const char * const JSON_color           = "color";
static const char * const JSON_lightType       = "lightType";
static const char * const JSON_spotExponent    = "spotExponent";
static const char * const JSON_spotInnerCutoff = "spotInnerCutoff";
static const char * const JSON_spotOuterCutoff = "spotOuterCutoff";
static const char * const JSON_constantAttenuation  = "constantAttenuation";
static const char * const JSON_linearAttenuation    = "linearAttenuation";
static const char * const JSON_quadraticAttenuation = "quadraticAttenuation";


std::string SceneSerialization::getSceneFilename(const std::string& filename) {
	std::string filePath = scenesFolder + filename;
	if ( (filePath.substr(filePath.length()-5,5)) != fileExt) {
		filePath += fileExt;
	}
	return filePath;
}

std::unique_ptr<Scene> SceneSerialization::load(const std::string& filename) {
	std::string filePath(getSceneFilename(filename));
	std::ifstream jsonStream(filePath, std::ios::in);
	
	std::string jsonStr;	
	if(jsonStream.is_open()){
		std::string line = "";
		while(getline(jsonStream, line)) {
			jsonStr += "\n" + line;
		}
		jsonStream.close();

		Scene *s = new Scene();
		s->setLabel(filename);
		SceneDeserialize(jsonStr, s);

		return std::unique_ptr<Scene>(s);		
	} else {
		// TODO throw
		printf("Impossible to open %s. Check your paths!\n", filename.c_str());
		return nullptr;		
	}	
}

void SceneSerialization::writeVec3(glm::vec3 v, PrettyWriter<FileStream>& writer) {
	writer.StartArray();
		writer.Double(v.x);
		writer.Double(v.y);
		writer.Double(v.z);	
	writer.EndArray();	
}
void SceneSerialization::writeQuat(glm::quat q, PrettyWriter<FileStream>& writer) {
	writer.StartArray();
		writer.Double(q.w);
		writer.Double(q.x);
		writer.Double(q.y);
		writer.Double(q.z);	
	writer.EndArray();	
}

void SceneSerialization::save(const std::string& filename, const Scene& scene) {
	std::string filePath(getSceneFilename(filename));
	
	FILE *f = fopen(filePath.c_str(), "w");
	FileStream s(f);	
	//FileStream s(stdout);

	PrettyWriter<FileStream> writer(s);		// Can also use Writer for condensed formatting

	writer.StartObject();
		glm::vec3 color = scene.getBackgroundColor();
		writer.String(JSON_backgroundColor);
		writeVec3(color, writer);

		writer.String(JSON_ambientLightLevel);
		writer.Double(scene.getAmbientLightLevel());
		
		writer.String(JSON_cameraPaths);
		writer.StartArray();

		for (auto cp : scene.getCameraPaths()) {

			writer.StartObject();
			writer.String(JSON_camPathDuration);
			writer.Double(cp->getDuration());

			writer.String(JSON_camPathKnots);
			writer.StartArray();
			for (auto knot : cp->getKnots()) {
				
				writer.StartObject();

				glm::vec3 knotpos = knot.pos();

				//writeVec3(knotpos, writer);
				
				writer.String(JSON_camPathKnotPos);
				writeVec3(knotpos, writer);
				
				glm::quat knotori = knot.ori();
				
				writer.String(JSON_camPathKnotOri);
				writeQuat(knotori, writer);
				
				writer.EndObject();
			}
			writer.EndArray();
			writer.EndObject();
		}
		writer.EndArray();	

		writer.String(JSON_nodes);
		SceneNodeSerialize(scene.getRootNode(), writer);

	writer.EndObject();

	/*
	writer.StartArray();
	for (std::vector<SceneNode*>::const_iterator nodeItr = scene->getNodes()->begin();
		nodeItr != scene->getNodes()->end();
		++nodeItr) {
			//nodeItr->Serialize(writer);
			SceneNodeSerializer(*nodeItr, writer);
	}
	writer.EndArray();
	*/

	//SceneNodeSerialize(scene.getRootNode(), writer);
	fclose(f);
}

CameraPath* SceneSerialization::CameraPathDeserialize(const std::string& jsonStr) {
	Document document;	// Default template parameter uses UTF8 and MemoryPoolAllocator.

	document.Parse<0>(jsonStr.c_str());

	if (document.HasParseError()) {
		printf("CameraPathDeserialize parse error\n");
		return nullptr;
	} else {
		CameraPath *cp = new CameraPath();
		
		double duration = document[JSON_camPathDuration].GetDouble();
		cp->setDuration(duration);

		const Value& knots = document[JSON_camPathKnots];
		assert(knots.IsArray());		
		for (Value::ConstValueIterator itr = knots.Begin(); itr != knots.End(); ++itr) {
				const Value& knot = *itr;

				const Value& knotpos = knot[JSON_camPathKnotPos];
				glm::vec3 vknotpos(knotpos[0u].GetDouble(), knotpos[1u].GetDouble(), knotpos[2u].GetDouble());

				const Value& knotori = knot[JSON_camPathKnotOri];
				glm::quat qknotori(knotori[0u].GetDouble(), knotori[1u].GetDouble(), knotori[2u].GetDouble(), knotori[3u].GetDouble());				

				cp->addKnot(vknotpos, qknotori);
		}
		return cp;
	}
}

Scene *SceneSerialization::SceneDeserialize(const std::string& jsonStr, Scene *s) {
	Document document;	// Default template parameter uses UTF8 and MemoryPoolAllocator.

	document.Parse<0>(jsonStr.c_str());

	if (document.HasParseError()) {
		printf("SceneDeserializer parse error\n");
		return s;
	} else {
		//parse global scene stuff (curr node, curr camea...)
		//TODO
		const Value& bgColor = document[JSON_backgroundColor];
		glm::vec3 vbgcolor(bgColor[0u].GetDouble(), bgColor[1u].GetDouble(), bgColor[2u].GetDouble());
		s->setBackgroundColor(vbgcolor);

		const Value& ambLightLev = document[JSON_ambientLightLevel];
		s->setAmbientLightLevel(ambLightLev.GetDouble());

		const Value& camPaths = document[JSON_cameraPaths];
		assert(camPaths.IsArray());

		for (Value::ConstValueIterator itr = camPaths.Begin(); itr != camPaths.End(); ++itr) {			
			StringBuffer sb;
			Writer<StringBuffer> writer( sb );
			
			itr->Accept(writer);		

			std::string jsonChild = sb.GetString();
			CameraPath *cp = CameraPathDeserialize(jsonChild);			
			s->addCameraPath(cp);
		}

		const Value& nodes   = document[JSON_nodes];
		StringBuffer sb;
		Writer<StringBuffer> writer( sb );			
		nodes.Accept(writer);			
		std::string jsonSceneNodes = sb.GetString();
		SceneNodeDeserialize(jsonSceneNodes, s, nullptr);

		//parse scene graph
		//SceneNodeDeserialize(jsonStr, s, nullptr);
	}
}

SceneNode * SceneSerialization::SceneNodeDeserialize(const std::string& json, Scene *scene, SceneNode *father) {

	Document document;	// Default template parameter uses UTF8 and MemoryPoolAllocator.

	document.Parse<0>(json.c_str());

	if (document.HasParseError()) {
		printf("SceneNodeDeserialize parse error\n");
		//TODO throw
	}

	//document["label"];
	printf("label = %s\n", document[JSON_label].GetString());
	printf("type  = %s\n", document[JSON_type].GetString());
	
	std::string label = document[JSON_label].GetString();
	std::string type  = document[JSON_type].GetString();
	
	SceneNode *sn;
	if (type == JSON_type_sceneNode) {
		sn = new SceneNode(label);
		if (father == nullptr) {
			scene->setRootNode(sn);
		} else {
			scene->addNode(sn, father);
		}
		
	} else if (type == JSON_type_cameraNode || type == JSON_type_fpscameraNode) {
		CameraNode *cn;
		if (type == JSON_type_cameraNode) {
			cn  = new CameraNode(label);
		} else {
			cn = new FpsCameraNode(label);
		}
		
		scene->addCameraNode(cn, father);
		
		float aspectRatio = document[JSON_aspectRatio].GetDouble();
		cn->setAspectRatio(aspectRatio);
		
		float fov = document[JSON_fov].GetDouble();
		cn->setFOV(fov);

		float nearClip = document[JSON_nearClip].GetDouble();
		cn->setNearClipDist(nearClip);

		float farClip  = document[JSON_farClip].GetDouble();
		cn->setFarClipDist(farClip);

		sn = cn;
		
	} else if (type == JSON_type_lightNode) {
		LightNode *ln = new LightNode(label);
		scene->addLightNode(ln, father);

		const Value& color = document[JSON_color];
		glm::vec3 vcolor(color[0u].GetDouble(), color[1u].GetDouble(), color[2u].GetDouble());
		ln->setColor(vcolor);

		LightNode::LightType lType = LightNode::LightType(document[JSON_lightType].GetInt());
		ln->setType(lType);
		
		float constantAttenuation = document[JSON_constantAttenuation].GetDouble();
		ln->setConstantAttenuation(constantAttenuation);

		float linearAttenuation = document[JSON_linearAttenuation].GetDouble();
		ln->setLinearAttenuation(linearAttenuation);

		float quadraticAttenuation = document[JSON_quadraticAttenuation].GetDouble();
		ln->setQuadraticAttenuation(quadraticAttenuation);
			
		/*float spotExponent = document[JSON_spotExponent].GetDouble();
		ln->setSpotExponent(spotExponent);

		float spotInnerCutoff = document[JSON_spotInnerCutoff].GetDouble();
		ln->setSpotInnerCutoff(spotInnerCutoff);

		float spotOuterCutoff = document[JSON_spotOuterCutoff].GetDouble();
		ln->setSpotOuterCutoff(spotOuterCutoff);*/
	
		sn = ln;

	} else if (type == JSON_type_assetNode) {
		std::string filename = document[JSON_assetFilename].GetString();
		Asset *asset = Sandbox::loadAsset(filename);
		AssetNode *an = new AssetNode(filename, asset);
		scene->addAssetNode(an, father);
				
		sn = an;
	} else if (type == JSON_type_animatedAssetNode) {
		std::string filenamePrefix = document[JSON_animatedAssetFilenamePrefix].GetString();
		std::string filenameSuffix = document[JSON_animatedAssetFilenameSuffix].GetString();
		int aaNumFrames    = document[JSON_animatedAssetNumFrames].GetInt();
		float aaFrameRate  = document[JSON_animatedAssetFrameRate].GetDouble();
		AnimatedAsset *asset = Sandbox::loadAnimatedAsset(filenamePrefix, filenameSuffix, aaNumFrames);
		AnimatedAssetNode *an = new AnimatedAssetNode(filenamePrefix + std::to_string(aaNumFrames) + filenameSuffix, asset);
		an->setAnimationFrameRate(aaFrameRate);
		scene->addAssetNode(an, father);
				
		sn = an;
	}
	
	const Value& position    = document[JSON_position];
	const Value& orientation = document[JSON_orientation];
	const Value& scaling     = document[JSON_scaling];

	glm::vec3 snpos = glm::vec3(position[0u].GetDouble(), position[1u].GetDouble(), position[2u].GetDouble());
	sn->setPosition(snpos);	

	
	double qw = orientation[0u].GetDouble();
	double qx = orientation[1u].GetDouble();
	double qy = orientation[2u].GetDouble();
	double qz = orientation[3u].GetDouble();
	printf("loading ori: %3.2f %3.2f %3.2f %3.2f\n", qx, qy, qz, qw);
	//glm::quat snori(qx, qy, qz, qw);
	glm::quat snori(qw, qx, qy, qz);
	printf("loading ori: %3.2f %3.2f %3.2f %3.2f\n", snori.x, snori.y, snori.z, snori.w);
	//glm::quat snori = glm::quat(orientation[0u].GetDouble(), orientation[1u].GetDouble(), orientation[2u].GetDouble(), orientation[3u].GetDouble());
	sn->setOrientation(snori);

	glm::vec3 snscal = glm::vec3(scaling[0u].GetDouble(), scaling[1u].GetDouble(), scaling[2u].GetDouble());
	sn->setScaling(snscal);

	{
		const Value& a = document[JSON_children];	// Using a reference for consecutive access is handy and faster.
		assert(a.IsArray());

		int chilNo = 0;
		for (Value::ConstValueIterator itr = a.Begin(); itr != a.End(); ++itr) {
			printf("child no: %d\n", chilNo++);

			StringBuffer sb;
			Writer<StringBuffer> writer( sb );
			
			itr->Accept(writer);
			//std::cout << sb.GetString() << std::endl;

			std::string jsonChild = sb.GetString();
			//SceneNode *child = SceneNodeDeserialize(jsonChild, scene, sn);			
			SceneNodeDeserialize(jsonChild, scene, sn);			
		}
	}
	
	return sn;
}

void SceneSerialization::SceneNodeSerialize(SceneNode *sn, PrettyWriter<FileStream>& writer) {
	writer.StartObject();
		
		writer.String(JSON_label);
		writer.String(sn->getLabel().c_str(), (SizeType)sn->getLabel().length());

		
		std::string type(JSON_type_sceneNode);
		if (FpsCameraNode *cn = dynamic_cast<FpsCameraNode*>(sn)) {
			type = JSON_type_fpscameraNode;

			writer.String(JSON_aspectRatio);
			writer.Double(cn->getAspectRatio());

			writer.String(JSON_fov);
			writer.Double(cn->getFOV());
		
			writer.String(JSON_nearClip);
			writer.Double(cn->getNearClipDist());

			writer.String(JSON_farClip);
			writer.Double(cn->getFarClipDist());

		} else if (CameraNode *cn = dynamic_cast<CameraNode*>(sn)) {
			type = JSON_type_cameraNode;

			writer.String(JSON_aspectRatio);
			writer.Double(cn->getAspectRatio());

			writer.String(JSON_fov);
			writer.Double(cn->getFOV());
		
			writer.String(JSON_nearClip);
			writer.Double(cn->getNearClipDist());

			writer.String(JSON_farClip);
			writer.Double(cn->getFarClipDist());


		} else if (LightNode *ln = dynamic_cast<LightNode*>(sn)) {
			type = JSON_type_lightNode;

			glm::vec3 color = ln->getColor();
			writer.String(JSON_color);
			writeVec3(color, writer);
			
			writer.String(JSON_lightType);
			writer.Int(ln->getType());

			writer.String(JSON_spotExponent);
			writer.Double(ln->getSpotExponent());

			writer.String(JSON_spotInnerCutoff);
			writer.Double(ln->getSpotInnerCutoff());

			writer.String(JSON_spotOuterCutoff);
			writer.Double(ln->getSpotOuterCutoff());

			writer.String(JSON_constantAttenuation);
			writer.Double(ln->getConstantAttenuation());

			writer.String(JSON_linearAttenuation);
			writer.Double(ln->getLinearAttenuation());

			writer.String(JSON_quadraticAttenuation);
			writer.Double(ln->getQuadraticAttenuation());
				
		} else if (AnimatedAssetNode *aan = dynamic_cast<AnimatedAssetNode*>(sn)) {
			type = JSON_type_animatedAssetNode;

			AnimatedAsset *aa = static_cast<AnimatedAsset*>(aan->getAttached());

			writer.String(JSON_animatedAssetFilenamePrefix);
			writer.String(aa->getFilePrefix().c_str(), static_cast<SizeType>(aa->getFilePrefix().length()));
			
			writer.String(JSON_animatedAssetFilenameSuffix);
			writer.String(aa->getFileSuffix().c_str(),  static_cast<SizeType>(aa->getFileSuffix().length()));
			
			writer.String(JSON_animatedAssetNumFrames);
			writer.Int(aa->getNumFrames());

			writer.String(JSON_animatedAssetFrameRate);
			writer.Double(aan->getAnimationFrameRate());		
				
		} else if (AssetNode *an = dynamic_cast<AssetNode*>(sn)) {
			type = JSON_type_assetNode;

			writer.String(JSON_assetFilename);
			Renderable *asset = an->getAttached();
			writer.String(asset->getLabel().c_str(), (SizeType)asset->getLabel().length());
		}

		writer.String(JSON_type);
		writer.String(type.c_str(), (SizeType) type.length());


		glm::vec3 pos = sn->getPosition();
		writer.String(JSON_position);
		writeVec3(pos, writer);
		
		glm::quat ori = sn->getOrientation();
		printf("saving ori: %3.2f %3.2f %3.2f %3.2f\n", ori.x, ori.y, ori.z, ori.w);

		writer.String(JSON_orientation);
		writeQuat(ori, writer);
		
		glm::vec3 scaling = sn->getScaling();
		writer.String(JSON_scaling);
		writeVec3(scaling, writer);
		
		writer.String(JSON_children);
		writer.StartArray();
		for(std::vector<SceneNode*>::const_iterator it = sn->getChildNodes().begin();
			it != sn->getChildNodes().end();
			++it) {
				SceneNode *child = (*it);
				SceneNodeSerialize(child, writer);
		}
		writer.EndArray();	
	
	writer.EndObject();
}
