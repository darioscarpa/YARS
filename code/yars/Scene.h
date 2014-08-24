#ifndef SCENE_H
#define SCENE_H

#include "AssetManager.h"

#include "SceneNode.h"

#include "AnimatedAssetNode.h"
#include "AssetNode.h"
#include "LightNode.h"
#include "CameraNode.h"
#include "FpsCameraNode.h"

#include "CameraPath.h"
#include "CameraPathWalker.h"

// OPENGL
#include "util/inc_opengl.h"

// MATH
#include "util/inc_math.h"

// STL
#include <vector>
#include <string>
#include <memory>

class Input;
class Renderer;

class Scene {
	public:
/////////////////////////////////////////////
		static std::unique_ptr<Scene> getEmptyScene();
/////////////////////////////////////////////

		Scene();	
		virtual ~Scene();
		
		void setLabel(const std::string& l) { label = l; }
		const std::string& getLabel() { return label; }

		virtual void onActivate();
		virtual void setup();
		void reshape(int winW, int winH);

		virtual void update(Input * input, float deltaTime); //deltaTime in secs			
				
		void addNode(SceneNode *n, SceneNode *father = nullptr);		
		void remNode(SceneNode *n, SceneNode *father = nullptr);
		
		void addLightNode(LightNode *l,   SceneNode *father = nullptr);
		void addAssetNode(AssetNode *a,   SceneNode *father = nullptr);
		void addCameraNode(CameraNode *c, SceneNode *father = nullptr);
		
		void remCameraPath(CameraPath *cp);
		void addCameraPath(CameraPath *cp);
		void addCameraPathByCameras();
		void startPlayingCameraPath(bool loop = false);
		void stopPlayingCameraPath();
		void updateCameraPathWalking(const float dt);
		void updateAnimatedNodes(const float dt);
		
		
		void setActiveNode(SceneNode *n)	  { activeNode   = n;   }	

		void setActiveCamera(CameraNode *c)   { activeCamera = c;   }
		void setActiveLight(LightNode *l)     { activeLight = l;    }
		void setActiveAsset(AssetNode *a)     { activeAsset = a;    }
		void setActiveCamPath(CameraPath *cp) { activeCamPath = cp; }

		inline void setRootNode(SceneNode *rn)  { rootNode = rn;    setActiveNode(rn); }
		inline SceneNode  * const getRootNode()     const { return rootNode;     }
		inline SceneNode  * const getActiveNode()   const { return activeNode;   }		
		SceneNode *const getNodeByLabel(const std::string& name);
		
		
		inline CameraNode * const getActiveCamera()  const { assert(activeCamera!=nullptr); return activeCamera; }		
		inline LightNode  * const getActiveLight()   const { assert(activeLight!=nullptr);  return activeLight;  }
		inline AssetNode  * const getActiveAsset()   const { assert(activeAsset!=nullptr);  return activeAsset;  }
		inline CameraPath * const getActiveCamPath() const { return activeCamPath; }
		
		inline CameraPathWalker& getCameraPathWalker()  { return camPathWalker; }
				
		SceneNode * const  nextNode();
		SceneNode * const  prevNode();
				
		CameraNode * const nextCamera();
		CameraNode * const prevCamera();

		LightNode * const  nextLight();
		LightNode * const  prevLight();

		AssetNode * const  nextAsset();
		AssetNode * const  prevAsset();

		// nodes
		inline const std::vector<AssetNode*>&  getAssetNodes()   const { return assets;  }
		inline const std::vector<CameraNode*>& getCameraNodes()  const { return cameras; }
		inline const std::vector<LightNode*>&  getLightNodes()   const { return lights;  }
		inline const std::vector< std::shared_ptr<CameraPath> >&  getCameraPaths() const { return camPaths;  }
		
		// color used to clear the screen
		inline void setBackgroundColor(glm::vec3 bg)       { backgroundColor = bg;	 }
		inline const glm::vec3& getBackgroundColor() const { return backgroundColor; }

		// ambient light level
		inline void  setAmbientLightLevel(float f)       { ambientLightLevel = f;	 }
		inline float getAmbientLightLevel() const { return ambientLightLevel; }


		const AssetNode* getPointLightAsset() const { return &pointLightAsset; }

		inline void      setSpecialPoint();
		inline glm::vec2 getSpecialPoint() const { return screenSpacePoint; }

		inline void      setEmisphereCenterPoint();
		inline glm::vec2 getEmisphereCenterPoint() const { return emisphereCenterPoint; }
private:
		// disable copy and assignment
		Scene(Scene&);
		const Scene& operator=( const Scene& );
		
		void makeUniqueLabel(SceneNode *n, const char * baseName);
		// -----------------------------------------------------------------------------------------------

		// id name
		std::string label;

		glm::vec3 backgroundColor;		
		float     ambientLightLevel;

		// all nodes
		std::vector<SceneNode*>  nodes;

		// nodes separated by type
		std::vector<CameraNode*> cameras;
		std::vector<LightNode*>  lights;
		std::vector<AssetNode*>  assets;		
		//std::vector<AnimatedAssetNode*>  animatedAssets;	
		
		
		// scene hierarchy root node
		SceneNode *rootNode;	

		AssetNode pointLightAsset;

		// current node (of any type)
		int currNode;
		SceneNode*  activeNode;
		std::string currNodeLabel;
		
		// current node (by type)
		int currCamera;
		CameraNode* activeCamera;

		int currAsset;
		AssetNode* activeAsset;

		int currLight;
		LightNode* activeLight;

		// camera paths		
		std::vector<std::shared_ptr<CameraPath>> camPaths;
		CameraPath* activeCamPath;

		CameraPathWalker camPathWalker;

		glm::vec2 screenSpacePoint;
		glm::vec2 emisphereCenterPoint;
};

#endif