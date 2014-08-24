#ifndef SHADER_HANDLING_H
#define SHADER_HANDLING_H

// OPENGL
#include "util/inc_opengl.h"

// MATH
#include "util/inc_math.h"

// STL
#include <sstream>
#include <memory>
#include <vector>
#include <assert.h>
#include <string>
#include <map>


class Shader {
public:
	enum ShaderType { 
		VERTEX   = GL_VERTEX_SHADER, 
		FRAGMENT = GL_FRAGMENT_SHADER,
		GEOMETRY = GL_GEOMETRY_SHADER,
		COMPUTE  = GL_COMPUTE_SHADER
	};

	Shader(ShaderType t);  
	~Shader();			  

	void setCode(const std::string& code);
	bool loadCode(const char * file_path);
	bool compile();

	inline GLuint getId() const { return id; }

	static void dumpLogToFile(const std::vector<char> & log, const std::string& suffix);
	
	template<typename T>
	void setConstantValue(const std::string& constantName, const T& value) {
		std::ostringstream k;
		k << "#" << constantName << "#";
		std::ostringstream v;
		v << value;		
		compileTimeConstants[k] = v;
	}

	typedef std::map<std::string, std::string> compConstMap_t;
	void Shader::setCompileTimeConstantsMap(compConstMap_t m) {
		compileTimeConstants = m;		
	}
private:
	std::string readFromFileIncluding(const char * filename);
	void dumpToFileForDebug();
	
	GLuint id;
	std::string code;
	std::string file_path;

	compConstMap_t compileTimeConstants;
};



class ShaderSubroutine {
public:
	ShaderSubroutine(const std::string& l):	label(l), active(false), currentFunc(0) {}

	void   addFunction(const std::string& name) {
		functionNames.push_back(name);
	};
	
	
/*
#ifdef USE_SUBROUTINES
	void   init(GLint programId, Shader::ShaderType type) {
		uniform = glGetSubroutineUniformLocation(programId, type, label.c_str());	
		if (uniform == -1) {
			std::string err("subroutine uniform not found:" + label);
			throw std::exception(err.c_str());			
		} else {
			//assert(uniform != -1);
			functionIds.resize(functionNames.size());
			for (size_t i = 0; i < functionNames.size(); ++i) {
				functionIds[i] = glGetSubroutineIndex(programId, type, functionNames[i].c_str());
				//assert( functionIds[i] != GL_INVALID_INDEX );
			}		
			active = true;
		}
	};
#else 
	void   init(GLint programId, Shader::ShaderType type) {
		std::string uniformId(label + "Selector");
		uniform = glGetUniformLocation(programId, uniformId.c_str()); 
		if (uniform == -1) {
			std::string err("subroutine uniform not found:" + label);
			throw std::exception(err.c_str());			
		} else {
			functionIds.resize(functionNames.size());
			for (size_t i = 0; i < functionNames.size(); ++i) {
				functionIds[i] = i;
			}
			active = true;
		}
	};
#endif
*/
	void   init(GLint programId, Shader::ShaderType type) {
		printf("subroutine init: %s\n", label.c_str());
		uniform = glGetSubroutineUniformLocation(programId, type, label.c_str());	
		if (uniform != -1) {
			functionIds.resize(functionNames.size());
			for (size_t i = 0; i < functionNames.size(); ++i) {
				functionIds[i] = glGetSubroutineIndex(programId, type, functionNames[i].c_str());
				assert( functionIds[i] != GL_INVALID_INDEX );
				printf(" func: %s (id %d)\n", functionNames[i].c_str(), functionIds[i]);
			}		
			
			active = true;
			realSubroutine = true;
		} else {
			std::string err("subroutine uniform not found:" + label);
			//std::cerr << err << std::endl;
			printf("%s\n", err.c_str());
			//throw std::exception(err.c_str());

			std::string uniformId(label + "Selector");
			uniform = glGetUniformLocation(programId, uniformId.c_str()); 
		
			printf(" fallback uniform: %s (%d)\n", uniformId.c_str(), uniform);
			if (uniform != -1) {
				functionIds.resize(functionNames.size());
				for (size_t i = 0; i < functionNames.size(); ++i) {
					functionIds[i] = i;
					printf(" func: %s (id %d)\n", functionNames[i].c_str(), functionIds[i]);
				}
				active = true;
				realSubroutine = false;				
			} else {
				std::string err("subroutine uniform (fallback) not found:" + label);
				throw std::exception(err.c_str());		
			}
		}		
	}

	
	inline void   setFunction(int id)       { currentFunc = id; }
	inline GLuint getCurrentFuncId()  const { return functionIds[currentFunc]; }

	inline const std::string& getLabel() const { return label; }
	inline GLint  getUniform()        const { return uniform; }
	inline bool   isActive()          const { return active; }
	inline bool   isRealSubroutine()  const { return realSubroutine; }
private:
	std::string     label;     // name of subroutine uniform
	bool			active;
	bool            realSubroutine;
	//Shader::ShaderType   type;      // needed by glGetSubroutine*
	//GLuint               programId; // needed by glGetSubroutine*

	GLint                    uniform;
	std::vector<std::string> functionNames;	
	std::vector<GLuint>      functionIds; // function -> opengl function id
	int                      currentFunc;

	
};

class ShaderProgram {	
public:
	ShaderProgram(const std::string& _label);			//id = glCreateProgram();
	virtual ~ShaderProgram();	//glDeleteProgram(id);

	
	void addShader(const Shader &s); //m_shaders.push_back(shaderID);  glAttachShader(id, shaderID);
	bool link();					 // glLinkProgram(id);
		
	inline void use() { 
		glUseProgram(id); 
		active = true;  
		//if (vertexSubroutinesStatus.size() > 0 )
			setVertexSubroutines();
		//if (geometrySubroutinesStatus.size() > 0 )
			setGeometrySubroutines();
		//if (fragmentSubroutinesStatus.size() > 0 )
			setFragmentSubroutines();
	};

	inline void setVertexSubroutines() {
		if (vertexSubroutinesStatus.size() > 0 )
			glUniformSubroutinesuiv(GL_VERTEX_SHADER,   GLsizei(vertexSubroutinesStatus.size()),   &vertexSubroutinesStatus[0]); 
	}
	inline void setGeometrySubroutines() { 
		if (geometrySubroutinesStatus.size() > 0 )
			glUniformSubroutinesuiv(GL_GEOMETRY_SHADER, GLsizei(geometrySubroutinesStatus.size()), &geometrySubroutinesStatus[0]); 
	}
	inline void setFragmentSubroutines() { 
		if (fragmentSubroutinesStatus.size() > 0 )
			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, GLsizei(fragmentSubroutinesStatus.size()), &fragmentSubroutinesStatus[0]); 
	}


	inline void deactivate() { active = false; }
	inline bool isActive()  const { return active; };
	
	inline GLuint getId()         const { return id; };
	inline bool   isInitialized() const { return initialized; };

	const std::string& getLabel() { return label; }
		
protected:
	struct UniformBufferObject {
		GLuint blkid;
		GLuint binding;
	};	
	struct TextureSampler {
		GLuint loc;
		GLuint unit;
	};	

	void setUBObinding(GLuint uboid, GLchar* blockname, GLuint ubobinding) {
		uboBindings[uboid].blkid = glGetUniformBlockIndex(getId(), blockname);
		uboBindings[uboid].binding = ubobinding;
		glUniformBlockBinding(getId(), uboBindings[uboid].blkid, ubobinding);
	};
	void setTextureSampler(GLuint texid, GLchar* samplername, GLuint texunit) {
		textureSamplers[texid].loc  = glGetUniformLocation(getId(), samplername);
		textureSamplers[texid].unit = texunit;
		
		//glProgramUniform1i(getId(), textureSamplers[texunit].loc, texunit);		
		glProgramUniform1i(getId(), textureSamplers[texid].loc, texunit);		
	}


	//TODO
	/*
	template< std::vector<std::unique_ptr<ShaderSubroutine>>& SV, std::vector<GLuint>& SSV >
	inline void setSubroutine<SV sv, SSV ssv>(unsigned int id, unsigned int subid) {
		const auto &r = sv[id].get();
		if (r->isActive()) {
			r->setFunction(subid);	
			if (r->isRealSubroutine()) {
				ssv[id] = r->getCurrentFuncId();
				setSubroutines<sv>()
			} else {
				glUniform1i(r->getUniform(), subid);
			}
		}
	};
	*/

	inline void setVertexSubroutine(unsigned int id, unsigned int subid) {
		assert(isActive());
		const auto &r = vertexSubroutines[id].get();
		if (r->isActive()) {
			r->setFunction(subid);	
			if (r->isRealSubroutine()) {
				vertexSubroutinesStatus[vertexSubroutinesIdx[id]] = r->getCurrentFuncId();
				setVertexSubroutines();
			} else {
				glUniform1i(r->getUniform(), subid);
			}
		}		
	};
	inline void setGeometrySubroutine(unsigned int id, unsigned int subid) {
		assert(isActive());
		const auto &r = geometrySubroutines[id].get();
		if (r->isActive()) {
			r->setFunction(subid);	
			if (r->isRealSubroutine()) {
				geometrySubroutinesStatus[geometrySubroutinesIdx[id]] = r->getCurrentFuncId();
				setGeometrySubroutines();
			} else {
				glUniform1i(r->getUniform(), subid);
			}
		}	
	};	
	inline void setFragmentSubroutine(unsigned int id, unsigned int subid) {
		assert(isActive());
		const auto &r = fragmentSubroutines[id].get();
		if (r->isActive()) {
			r->setFunction(subid);	
			if (r->isRealSubroutine()) {
				fragmentSubroutinesStatus[fragmentSubroutinesIdx[id]] = r->getCurrentFuncId();
				printf("subroutine %s (uniform %d) pos %d func %d\n",  r->getLabel().c_str(), r->getUniform(), fragmentSubroutinesIdx[id], r->getCurrentFuncId());
				setFragmentSubroutines();
			} else {
				glUniform1i(r->getUniform(), subid);
			}
		}	
	};	

	inline void initSubroutines() {
		use();

		size_t activeVertSubroutines = 0;
		size_t iv = 0;
		vertexSubroutinesIdx.resize(vertexSubroutines.size());
		vertexSubroutinesStatus.resize(vertexSubroutines.size());
		for ( auto &vs : vertexSubroutines ) {
			try {
				vs.get()->init(getId(), Shader::ShaderType::VERTEX);
				if (vs.get()->isRealSubroutine()) {
					vertexSubroutinesIdx[iv] = activeVertSubroutines;
					vertexSubroutinesStatus[activeVertSubroutines] = vs.get()->getCurrentFuncId();
					++activeVertSubroutines;				
				}
				iv++;
			} catch (std::exception& e) {
				printf("%s\n", e.what());
			}
		}
		vertexSubroutinesStatus.resize(activeVertSubroutines);	
		for ( int i = 0 ; i < vertexSubroutines.size(); ++i ) setVertexSubroutine(i, 0);

		size_t activeGeomSubroutines = 0;
		size_t ig = 0;
		geometrySubroutinesIdx.resize(geometrySubroutines.size());
		geometrySubroutinesStatus.resize(geometrySubroutines.size());
		for ( auto &gs : geometrySubroutines ) {
			try {
				gs.get()->init(getId(), Shader::ShaderType::GEOMETRY);
				if (gs.get()->isRealSubroutine()) {
					geometrySubroutinesIdx[iv] = activeGeomSubroutines;
					geometrySubroutinesStatus[activeGeomSubroutines] = gs.get()->getCurrentFuncId();
					++activeGeomSubroutines;
				}
				ig++;
			} catch (std::exception& e) {
				printf("%s\n", e.what());
			}
		}
		geometrySubroutinesStatus.resize(activeGeomSubroutines);
		for ( int i = 0 ; i < geometrySubroutines.size(); ++i ) setGeometrySubroutine(i, 0);

		size_t activeFragSubroutines = 0;
		size_t ii = 0;
		fragmentSubroutinesIdx.resize(fragmentSubroutines.size());
		fragmentSubroutinesStatus.resize(fragmentSubroutines.size());
		for ( auto &fs : fragmentSubroutines ) {
			try {
				fs.get()->init(getId(), Shader::ShaderType::FRAGMENT);
				if (fs.get()->isRealSubroutine()) {
					fragmentSubroutinesIdx[ii] = activeFragSubroutines;
					fragmentSubroutinesStatus[activeFragSubroutines] = fs.get()->getCurrentFuncId();
					++activeFragSubroutines;
				}
				ii++;
			} catch (std::exception& e) {
				printf("%s\n", e.what());
			}
		}
		fragmentSubroutinesStatus.resize(activeFragSubroutines);		
		for ( int i = 0 ; i < fragmentSubroutines.size(); ++i ) setFragmentSubroutine(i, 0);
	};

	

	inline void setBoolUniform(unsigned int id, bool enabled) {
		assert(isActive() && id < uniformLocs.size());
		glUniform1i(uniformLocs[id], enabled);
	};
	inline void setFloatUniform(unsigned int id, float f) {
		assert(isActive() && id < uniformLocs.size());
		glUniform1f(uniformLocs[id], f);
	};
	inline void setIntUniform(unsigned int id, int i) {
		assert(isActive() && id < uniformLocs.size());	
		glUniform1i(uniformLocs[id], i);
	}

	inline void setVec2Uniform(unsigned int id, const glm::vec2& v) {
		assert(isActive() && id < uniformLocs.size());	
		glUniform2f(uniformLocs[id], v.x, v.y);	
	}

	inline void setVec2Uniform(unsigned int id, const glm::ivec2& v) {
		assert(isActive() && id < uniformLocs.size());	
		glUniform2i(uniformLocs[id], v.x, v.y);	
	}

	std::vector<TextureSampler>      textureSamplers;
	std::vector<UniformBufferObject> uboBindings;
	std::vector<GLuint>              uniformLocs;		


	std::vector<std::unique_ptr<ShaderSubroutine>> vertexSubroutines;
	std::vector<int>                               vertexSubroutinesIdx;
	std::vector<GLuint>                            vertexSubroutinesStatus;

	std::vector<std::unique_ptr<ShaderSubroutine>> geometrySubroutines;
	std::vector<int>                               geometrySubroutinesIdx;
	std::vector<GLuint>                            geometrySubroutinesStatus;

	std::vector<std::unique_ptr<ShaderSubroutine>> fragmentSubroutines;
	std::vector<int>                               fragmentSubroutinesIdx;
	std::vector<GLuint>                            fragmentSubroutinesStatus;


private:
	ShaderProgram(const ShaderProgram&);
	const ShaderProgram& operator=(const ShaderProgram&);
	
	GLuint id;	
	bool active;
	bool initialized;

	std::string label;
};


class GfxShaderProgram : public ShaderProgram {
public:
	virtual ~GfxShaderProgram() {};
	GfxShaderProgram(const std::string& _label,
				  const char * vert_file_path,
	              const char * geom_file_path,
				  const char * frag_file_path = NULL,
				  std::map<std::string, std::string> = std::map<std::string, std::string>() );	
};
class ComputeShaderProgram : public ShaderProgram {	
public:
	virtual ~ComputeShaderProgram() {};
	ComputeShaderProgram(const std::string& _label, const char * comp_file_path);	
};

#endif

/*void setUBO(GLuint uboid, GLuint ubobinding) {
		uboBindings[uboid].binding = ubobinding;
		glUniformBlockBinding(getId(), uboBindings[uboid].blkid, ubobinding);
	};
	inline GLuint getUBObinding(GLuint uboid) const { return uboBindings[uboid].binding; }
	
	
	void setTextureSamplerUnit(GLuint texid, GLuint texunit) {
		//TODO assert (prev_id == id)
		GLint prev_id;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prev_id);

		glUseProgram(getId());
		textureSamplers[texid].unit = texunit;
		glUniform1i(textureSamplers[texid].loc, texunit);	

		glUseProgram(prev_id);
	};
	GLuint getTextureSamplerUnit(GLuint texid) const { return textureSamplers[texid].unit; };
	*/

	
	/*
	struct boolUniform {
		GLuint loc;
		bool val;
	}
	struct intUniform {
		GLuint loc;
		int val;
	}
	struct floatUniform {
		GLuint loc;
		float val;
	}
	*/
	