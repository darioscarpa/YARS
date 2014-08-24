#ifndef ASSET_H
#define	ASSET_H

// inherits
#include "Renderable.h"

// OPENGL
#include "util/inc_opengl.h"

// DevIL
#include <IL/il.h>

// Assimp
#include <assimp/Importer.hpp>	
#include <assimp/PostProcess.h>
#include <assimp/Scene.h>
#include <assimp/types.h>

// GLM
#include "util/inc_math.h" 

// STL
#include <fstream>
#include <map>
#include <vector>

// fwd decs
class Renderer;


struct MaterialDescriptor {
	glm::vec4 diffuse;
	glm::vec4 ambient;
	glm::vec4 specular;
	glm::vec4 emissive;
	float     shininess;

	int       diffuseTexId;
	int       normalMapId;
	int       specularMapId;
	int		  opacityMapId;
};

struct MeshDescriptor {
	GLuint vao;	
	int    numFaces;	
	int    materialIndex;	
};


class AssetSubNode {
public:
	glm::mat4                  transformation;	
	std::vector<unsigned int>  meshIds;
	std::vector<AssetSubNode*> children;	
};


class Asset : public Renderable {
	friend class AssetManager;
public:	
    ~Asset();
	   
	void init() {};
	void destroy() {};

	virtual void render(Renderer *boundRenderer, bool withMaterials = true);

	const glm::vec3 getBoundingBoxSize() const { return bbox_max - bbox_min; };
	const float     getScaleFactor()     const { return scaleFactor; };

private:  
	/////////////////////////////////////////////////////////////////////////////////////////////

	Asset(Assimp::Importer& importer, const std::string& pFile);

	// Can't send color down as a pointer to aiColor4D because AI colors are ABGR.
	static void aiColor4_to_vec4(const aiColor4D &c, glm::vec4 &cv) {
		cv.r = c.r; cv.g = c.g; cv.b = c.b; cv.a = c.a;
	}
	
	AssetSubNode* recursive_makeGraph(const aiNode *n);	
	void          recursive_freeGraph(AssetSubNode *node);

    void initLogging(std::string filename);

	void loadFileError(const std::string& errmsg, Assimp::Importer &importer, const std::string& pFile);
	void loadFile( Assimp::Importer &importer, const std::string& pFile );
    
	void calcBoundingBox(const aiScene& pcScene, const aiNode& piNode, glm::vec3* min, glm::vec3* max, const glm::mat4& piMatrix);	
	void calcSuggestedScaleFactor();

	void recursive_render(glm::mat4 modelMatrix, const aiNode *nd);
    	
    bool loadTextures(const aiScene& scene, const std::string& assetFolder);	
	void loadMaterials(const aiScene& scene);
	void genVAOs(const aiScene& scene);

	void drawMesh(const MeshDescriptor& m, bool withMaterials );
    
	void recursive_render(Renderer *boundRenderer, glm::mat4 modelMatrix, const AssetSubNode *n, bool withMaterials = true);
	
	/////////////////////////////////////////////////////////////////////////////////////////////
	
	AssetSubNode *rootNode;   
	std::string   label;

	std::map<std::string, GLuint>   textureIdMap;  // image filename -> textureId   
	std::vector<MeshDescriptor>     thisMeshes;    // meshes used in the "scene"
	std::vector<MaterialDescriptor> materials;     // mtl data
	std::vector<GLuint>             materialsUBO;  // mtl ubo indexes
	std::map<std::string, int>      materialMap;   // mtl name -> mtl data index

	glm::vec3  bbox_min, bbox_max;
	float      scaleFactor;

	/////////////////////////////////////////////////////////////////////////////////////////////	
};

#endif

