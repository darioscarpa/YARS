#include "Asset.h"

#include "Renderer.h"
#include "Texture.h"

#include "Sandbox.h"

#include <assimp/Logger.hpp>
#include <assimp/DefaultLogger.hpp>

// STL
#include <iostream>

Asset::Asset(Assimp::Importer& importer, const std::string& pFile) {	
	std::string::size_type slashidx = pFile.find_last_of("/");
	std::string assetFilename = pFile.substr(slashidx + 1) ;    
	initLogging(assetFilename);

	//bool retModel = loadFile( importer, pFile );
	//if (!retModel) throw std::exception("can't load file");

	loadFile(importer, pFile); // might throw
}

Asset::~Asset() {
	for (unsigned int i = 0; i < thisMeshes.size(); ++i) {			
		glDeleteVertexArrays(1,&(thisMeshes[i].vao));		
	}
	for (unsigned int i = 0; i < materials.size(); ++i) {			
		glDeleteBuffers(1, &materialsUBO[i]);

		glDeleteTextures(1, (const GLuint*) &(materials[i].diffuseTexId));
		glDeleteTextures(1, (const GLuint*) &(materials[i].normalMapId));
		glDeleteTextures(1, (const GLuint*) &(materials[i].specularMapId));
		glDeleteTextures(1, (const GLuint*) &(materials[i].opacityMapId));			
	}	
	recursive_freeGraph(rootNode);	
}

void Asset::initLogging(std::string filename) {
	//Assimp::Logger::LogSeverity severity = Assimp::Logger::NORMAL;
	Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;

	// Create a logger instance for File Output (found in project folder or near .exe)	
	std::string logfn = "_logs/assimp_log."+ filename + ".txt";
	Assimp::DefaultLogger::create(logfn.c_str(), severity, aiDefaultLogStream_FILE); 
}

void Asset::render (Renderer *boundRenderer, bool withMaterials) {
	//recursive_render ( *(boundRenderer->getModelMatrix()), scene->mRootNode) ;
	recursive_render (boundRenderer, boundRenderer->getModelMatrix(), rootNode, withMaterials) ;
	//recursive_render_opaque ( *(boundRenderer->getModelMatrix()), scene->mRootNode) ;
	//recursive_render_alpha ( *(boundRenderer->getModelMatrix()), scene->mRootNode) ;
	//alphaRender();
}

void Asset::recursive_render (Renderer *boundRenderer,
							  glm::mat4 modelMatrix,
							  const AssetSubNode *n,
							  bool withMaterials ) {
	modelMatrix = modelMatrix * n->transformation;
	boundRenderer->setModelMatrix(modelMatrix);
	
	// draw this
	for (auto m : n->meshIds) drawMesh(thisMeshes[m], withMaterials);
	
	// draw children
	for (auto c : n->children) recursive_render(boundRenderer, modelMatrix, c, withMaterials);
}

void Asset::drawMesh(const MeshDescriptor& md, bool withMaterials ) {
	if (withMaterials) {
		int matIdx = md.materialIndex;
		MaterialDescriptor *m = &(materials[matIdx]);

		//bind material
		//glBindBufferRange(GL_UNIFORM_BUFFER, Renderable::ubo_material, materialsUBO[matIdx], 0, sizeof(struct MaterialDescriptor));	
		glBindBufferBase(GL_UNIFORM_BUFFER, Renderable::ubo_material, materialsUBO[matIdx]);
	
		// bind textures		
		glActiveTexture(GL_TEXTURE0 + Renderable::tu_diffuse);
		glBindTexture(GL_TEXTURE_2D, m->diffuseTexId);
		glActiveTexture(GL_TEXTURE0 + Renderable::tu_normalMap);		
		glBindTexture(GL_TEXTURE_2D, m->normalMapId);
		glActiveTexture(GL_TEXTURE0 + Renderable::tu_specularMap);		
		glBindTexture(GL_TEXTURE_2D, m->specularMapId);
		glActiveTexture(GL_TEXTURE0 + Renderable::tu_opacityMap);		
		glBindTexture(GL_TEXTURE_2D, m->opacityMapId);		
	}
	// bind VAO
	glBindVertexArray(md.vao);

	// draw
	glDrawElements(GL_TRIANGLES, md.numFaces*3, GL_UNSIGNED_INT, 0);
}


void Asset::loadFileError(const std::string& errmsg, Assimp::Importer &importer, const std::string& pFile) {
	std::string err = "error loading " + pFile + ": " + errmsg + " - Assimp: " + importer.GetErrorString();
	throw std::exception(err.c_str());
}
void Asset::loadFile(Assimp::Importer &importer, const std::string& pFile) {
	//check if file exists
	std::ifstream fin(pFile.c_str());
	if( !fin.fail() ) {
		fin.close();
	} else {
		loadFileError("can't open file for reading", importer, pFile);
		return;
	}

	printf("Loading assets from: %s\n", pFile.c_str());
	const aiScene *scene = importer.ReadFile( pFile, aiProcessPreset_TargetRealtime_Quality);
	//const aiScene *scene = importer.ReadFile( pFile, aiProcessPreset_TargetRealtime_Fast);
	//const aiScene *scene = importer.ReadFile( pFile, aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_GenUVCoords);

	// Assimp import failed
	if( !scene ) {
		loadFileError("import failure", importer, pFile);
		return;
	}

	// Now we can access the file's contents.
	//printf("Import of scene %s succeeded.\n", pFile.c_str());
	std::cout << "succesfully loaded " << pFile << std::endl;

	// save file path for texture loading	 
    std::string::size_type slashidx = pFile.find_last_of("/");
	std::string assetFolder;
    if (slashidx == std::string::npos) {
        assetFolder = "./";
    } else if (slashidx == 0) {
        assetFolder = "/";
    } else {
        assetFolder = pFile.substr(0, slashidx + 1) ;
    }

	/////////////////////////////
	calcBoundingBox(*scene, *(scene->mRootNode), &bbox_min, &bbox_max, glm::mat4(1.0));
	//printf("bbox min: %3.2f %3.2f %3.2f\n", bbox_min.x, bbox_min.y, bbox_min.z);
	//printf("bbox max: %3.2f %3.2f %3.2f\n", bbox_max.x, bbox_max.y, bbox_max.z);

	calcSuggestedScaleFactor();
	//printf("scale factor: %3.2f\n", scaleFactor);
	/////////////////////////////

	rootNode = recursive_makeGraph(scene->mRootNode);
	
	loadTextures(*scene, assetFolder);	
	loadMaterials(*scene);
	genVAOs(*scene);	
	
}

AssetSubNode* Asset::recursive_makeGraph(const aiNode *n) {
	AssetSubNode *node = new AssetSubNode();

	aiMatrix4x4 m = n->mTransformation;
	m.Transpose();	
	
	node->transformation = glm::make_mat4((float*)&m);
		

	node->meshIds.reserve(n->mNumMeshes);	
	for (unsigned int i=0; i < n->mNumMeshes; ++i) {
		node->meshIds.push_back( n->mMeshes[i] );
	}

	//printf("recursive_makeGraph - meshes %d - children %d\n", node->meshIds.size(), node->children.capacity());
	
	node->children.reserve(n->mNumChildren);
	for (unsigned int i=0; i < n->mNumChildren; ++i){
		node->children.push_back( recursive_makeGraph(n->mChildren[i]) );
	}
	return node;
}

void Asset::recursive_freeGraph(AssetSubNode *node) {	
	for (auto c : node->children) recursive_freeGraph(c);
	delete node;	
}


bool Asset::loadTextures(const aiScene& scene, const std::string& assetFolder) {
	bool everythingOk = true;
	for (unsigned int m = 0; m < scene.mNumMaterials; ++m)	{
		for (int x = 0; x < aiTextureType_UNKNOWN ; x++) {
			int nt = scene.mMaterials[m]->GetTextureCount((aiTextureType)x);
			for (int j = 0; j < nt; j++) {
				aiString path;	
				aiReturn texFound;
				texFound = scene.mMaterials[m]->GetTexture((aiTextureType)x, j, &path);
				if (texFound == AI_SUCCESS) {
					textureIdMap[path.data] = 0; 
				}				
			}
		}
	}
	for (std::map<std::string, GLuint>::iterator itr = textureIdMap.begin();
		 itr != textureIdMap.end();
		 ++itr) {
		std::string filename = (*itr).first;  
		std::string fullFilename = assetFolder + filename;
		
		std::shared_ptr<Texture> t = Sandbox::textureManager.loadTexture(fullFilename);

		//Texture t(GL_TEXTURE_2D, fullFilename);
		//everythingOk = everythingOk && t.loadAndBuildMipmaps();
		itr->second = t.get()->getId();
	}
	return everythingOk;
}


// load textures first
void Asset::loadMaterials(const aiScene& scene) {
	static const glm::vec4 DEFAULTCOLOR_DIFFUSE(0.8f, 0.8f, 0.8f, 1.0f);
	static const glm::vec4 DEFAULTCOLOR_AMBIENT(0.2f, 0.2f, 0.2f, 1.0f);
	static const glm::vec4 DEFAULTCOLOR_SPECULAR(0.0f, 0.0f, 0.0f, 1.0f);
	static const glm::vec4 DEFAULTCOLOR_EMISSIVE(0.0f, 0.0f, 0.0f, 1.0f);

	for (unsigned int i=0; i < scene.mNumMaterials; i++) {
		aiMaterial *mtl = scene.mMaterials[i];
		
		aiString mtlname;
		mtl->Get(AI_MATKEY_NAME, mtlname);	

		MaterialDescriptor tmp;		
		
		aiColor4D color;	
		
		if(AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {		
			Asset::aiColor4_to_vec4(color, tmp.diffuse);
		} else {
			tmp.diffuse = DEFAULTCOLOR_DIFFUSE; 
		}
		if(AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_AMBIENT, color)) {		
			Asset::aiColor4_to_vec4(color, tmp.ambient);
		} else {
			tmp.ambient = DEFAULTCOLOR_AMBIENT; 
		}
		if(AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_SPECULAR, color)) {		
			Asset::aiColor4_to_vec4(color, tmp.specular);
		} else {
			tmp.specular = DEFAULTCOLOR_SPECULAR;
		}		
		if(AI_SUCCESS == mtl->Get(AI_MATKEY_COLOR_EMISSIVE, color)) {		
			Asset::aiColor4_to_vec4(color, tmp.emissive);
		} else {
			tmp.emissive =  DEFAULTCOLOR_EMISSIVE; 
		}
		
		float shininess = 0.0f;
		unsigned int max = 1;
		if (AI_SUCCESS == mtl->Get(AI_MATKEY_SHININESS, &shininess, &max)) {
			max = 1;
			float strength;
			if (AI_SUCCESS == mtl->Get(AI_MATKEY_SHININESS_STRENGTH, &strength, &max)) {
				shininess *= strength;
			}
		}
		tmp.shininess = shininess;

		/////
		aiString texPath;
		tmp.diffuseTexId = 0;
		if(AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath)){
			tmp.diffuseTexId = textureIdMap.find(texPath.data)->second;
			//tmp.diffuseTexId = textureManager::getTexture(texPath.data).getGLid();
		}	
		tmp.normalMapId = 0;
		if (AI_SUCCESS == mtl->GetTexture(aiTextureType_HEIGHT, 0 , &texPath)) {
			tmp.normalMapId = textureIdMap.find(texPath.data)->second;
		}
		tmp.specularMapId = 0;
		if (AI_SUCCESS == mtl->GetTexture(aiTextureType_SPECULAR, 0 , &texPath)) {
			tmp.specularMapId = textureIdMap.find(texPath.data)->second;
		}		
		tmp.opacityMapId = 0;
		if (AI_SUCCESS == mtl->GetTexture(aiTextureType_OPACITY, 0 , &texPath)) {
			tmp.opacityMapId = textureIdMap.find(texPath.data)->second;
		}		
		//////////////////////////
		GLuint uboindex;
		glGenBuffers(1, &uboindex);
		glBindBuffer(GL_UNIFORM_BUFFER, uboindex);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(tmp), (void *)(&tmp), GL_STATIC_DRAW);		
		//////////////////////////		
		materialMap[mtlname.data] = i;
		materials.push_back(tmp);
		materialsUBO.push_back(uboindex);
	}
	
	std::map<std::string, int>::iterator itr = materialMap.begin();	
	for (; itr != materialMap.end(); ++itr) {
		std::string materialName = (*itr).first;
		MaterialDescriptor tmp = materials[(*itr).second];
		printf("%s\t uboidx:%d\n\td:%.2f|%.2f|%.2f a:%.2f|%.2f|%.2f\n\ts:%.2f|%.2f|%.2f e:%.2f|%.2f|%.2f\n\tshi:%.2f td:%d tn:%d ts:%d to:%d\n", 
			materialName.c_str(), materialsUBO[(*itr).second],
			tmp.diffuse.r, tmp.diffuse.g, tmp.diffuse.b, 
			tmp.ambient.r, tmp.ambient.g, tmp.ambient.b,
			tmp.specular.r, tmp.specular.g, tmp.specular.b,
			tmp.emissive.r, tmp.emissive.g, tmp.emissive.b,
			tmp.shininess, 
			tmp.diffuseTexId, tmp.normalMapId, tmp.specularMapId, tmp.opacityMapId);
	}
}


// load textures and setup mtl ubo before calling this
void Asset::genVAOs(const aiScene& scene) {	
	MeshDescriptor thisMesh;	
	
	const GLuint vertexLoc    = Renderable::v_positionLoc;
	const GLuint normalLoc = Renderable::v_normalLoc;
	const GLuint texCoordLoc = Renderable::v_uvLoc;
	const GLuint tangentLoc = Renderable::v_tangentLoc;
	const GLuint bitangentLoc = Renderable::v_bitangentLoc;
	
	// for each mesh in loaded scene
	for (unsigned int n = 0; n < scene.mNumMeshes; ++n) {
		GLuint buffer;
		const struct aiMesh* mesh = scene.mMeshes[n];

		////////////////////////////////
		// create faces array, converting from Assimp format 
		unsigned int *faceArray = new unsigned int[mesh->mNumFaces * 3];
		
		unsigned int faceIndex = 0;
		for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
			const struct aiFace* face = &mesh->mFaces[t];

			memcpy(&faceArray[faceIndex], face->mIndices, 3 * sizeof(float));
			faceIndex += 3;
		}
		
		thisMesh.numFaces = mesh->mNumFaces;
		glGenVertexArrays(1,&(thisMesh.vao));
		glBindVertexArray(thisMesh.vao);		
		
		// buffer for faces
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->mNumFaces * 3, faceArray, GL_STATIC_DRAW);
		
		delete[] faceArray;

		////////////////////////////////

		// buffer for vertex positions
		if (mesh->HasPositions()) {
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(vertexLoc);
				glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, 0);			
		}		

		// buffer for vertex normals
		if (mesh->HasNormals()) {
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mNormals, GL_STATIC_DRAW);
				glEnableVertexAttribArray(normalLoc);
				glVertexAttribPointer(normalLoc, 3, GL_FLOAT, 0, 0, 0);
		}

		// buffer for vertex tangents & bitangents
		if (mesh->HasTangentsAndBitangents()) {
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mTangents, GL_STATIC_DRAW);
				glEnableVertexAttribArray(tangentLoc);
				glVertexAttribPointer(tangentLoc, 3, GL_FLOAT, 0, 0, 0);

			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mBitangents, GL_STATIC_DRAW);
				glEnableVertexAttribArray(bitangentLoc);
				glVertexAttribPointer(bitangentLoc, 3, GL_FLOAT, 0, 0, 0);
		}

		// buffer for vertex texture coordinates
		if (mesh->HasTextureCoords(0)) {
			float *texCoords = new float[2 * mesh->mNumVertices];
			for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {
				texCoords[k*2]   = mesh->mTextureCoords[0][k].x;
				texCoords[k*2+1] = mesh->mTextureCoords[0][k].y; 				
			}
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * mesh->mNumVertices, texCoords, GL_STATIC_DRAW);
				glEnableVertexAttribArray(texCoordLoc);
				glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, 0, 0, 0);
			delete[] texCoords;
		}

		// unbind buffers
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER,0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	
		// set material
		struct aiMaterial *mtl = scene.mMaterials[mesh->mMaterialIndex];
		aiString mtlname;
		mtl->Get(AI_MATKEY_NAME, mtlname);
		thisMesh.materialIndex = materialMap.find(mtlname.data)->second;
		
		// store mesh
		thisMeshes.push_back(thisMesh);
	}
}


void Asset::calcBoundingBox(const aiScene& pcScene, const aiNode& piNode, glm::vec3* min, glm::vec3* max, const glm::mat4& piMatrix) {
	
	aiMatrix4x4 mTemp = piNode.mTransformation;
	mTemp.Transpose();
	//aiMatrix4x4 aiMe = mTemp * piMatrix;

	glm::mat4 temp = glm::make_mat4((float*)&mTemp);
	glm::mat4 me   = temp * piMatrix;

	for (unsigned int i = 0; i < piNode.mNumMeshes; ++i) {
		for( unsigned int a = 0; a < pcScene.mMeshes[piNode.mMeshes[i]]->mNumVertices; ++a) {
			aiVector3D pc = pcScene.mMeshes[piNode.mMeshes[i]]->mVertices[a];

			glm::vec3 pcv(pc.x, pc.y, pc.z);

			glm::vec3 pc1v = glm::mat3(me) * pcv; 
			
			min->x = std::min( min->x, pc1v.x);
			min->y = std::min( min->y, pc1v.y);
			min->z = std::min( min->z, pc1v.z);
			max->x = std::max( max->x, pc1v.x);
			max->y = std::max( max->y, pc1v.y);
			max->z = std::max( max->z, pc1v.z);
		}
	}
	for (unsigned int i = 0; i < piNode.mNumChildren;++i) {
		calcBoundingBox( pcScene, *(piNode.mChildren[i]), min, max, me );
	}
	
}

// must call calcBoundingBox first
void Asset::calcSuggestedScaleFactor() {
	glm::vec3 size = bbox_max-bbox_min;
	float max = std::max(size.x, size.y);
	max = std::max(max, size.z);
	scaleFactor = 100.0f / max;
}

