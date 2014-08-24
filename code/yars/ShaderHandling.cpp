#include "ShaderHandling.h"

#include "util/vsGLInfoLib.h"
#include "util/string_utils.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

static const std::string SHADERS_FOLDER = "_shaders/";

Shader::Shader(ShaderType t) {
	id = glCreateShader(t);
	// if (id==0) throw ex	TODO
}

Shader::~Shader() {
	glDeleteShader(id);
}

void Shader::setCode(const std::string& s) {
	code = s;
}

std::string Shader::readFromFileIncluding(const char * filename) {	
	std::string shaderCode;	
	std::string filepath = SHADERS_FOLDER + "/" + filename;
	std::ifstream shaderStream(filepath, std::ios::in);	

	if(shaderStream.is_open()){
		std::string line = "";
		while(getline(shaderStream, line)) {
			if (line.substr(0, 9) == "#constant") {
				std::string constline = line.substr(10);
				for (auto& kv : compileTimeConstants) {
					constline = StringUtils::strReplace(constline, kv.first, kv.second);
				}
				shaderCode += "\n" + constline;
			} else 
			if (line.substr(0, 8) == "#include") {
				shaderCode += "\n" + readFromFileIncluding(line.substr(9).c_str());
			} else {
				shaderCode += "\n" + line;
			}
		}
		shaderStream.close();		
	} else {
		// TODO throw
		printf("Impossible to open %s. Check your paths!\n", filepath.c_str());		
		throw new std::exception("file loading failed\n");		
	}
	const std::string beginMarker("\n// ## FILE BEGIN: " + std::string(filename) + " #############\n");
	const std::string endMarker  ("\n// ## FILE END:   " + std::string(filename) + " #############\n");
	return beginMarker + shaderCode + endMarker;
}

void Shader::dumpToFileForDebug() {	
	std::string filepath = SHADERS_FOLDER + "/dbg/" + file_path;
	std::ofstream shaderStream(filepath, std::ios::out);
	if (shaderStream.is_open()) {
		shaderStream << code;
		shaderStream.close();
	}
}

void Shader::dumpLogToFile(const std::vector<char> & log, const std::string& basefilename) {
	if (log.size()<=2) return; // TODO CLEAN
	std::string filepath = SHADERS_FOLDER + "/dbg/" + basefilename + ".log";
	std::ofstream shaderStream(filepath, std::ios::out);
	if (shaderStream.is_open()) {
		shaderStream << log.data();
		shaderStream.close();
	}
}

bool Shader::loadCode(const char * filename) {	
	try {
		file_path = filename;
		std::string shaderCode = readFromFileIncluding(file_path.c_str());
		setCode(shaderCode);
		dumpToFileForDebug();
		return true;
	} catch (...) {
		return false;
	}
}

bool Shader::compile() {
	GLint result = GL_FALSE;
	GLint infoLogLength;

	// compile
	printf("Compiling shader : %s\n", file_path.c_str());
	char const * sourceCodeP = code.c_str();
	glShaderSource(id, 1, &sourceCodeP , NULL);
	glCompileShader(id);

	// check 
	glGetShaderiv(id, GL_COMPILE_STATUS,  &result);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
	if ( infoLogLength > 0 ){
		// TODO throw
		std::vector<char> shaderErrorMessage(infoLogLength+1);
		glGetShaderInfoLog(id, infoLogLength, NULL, &shaderErrorMessage[0]);
		printf("%s\n", &shaderErrorMessage[0]);	

		Shader::dumpLogToFile(shaderErrorMessage, file_path + ".compile."+std::to_string(id));
	}
	return (result == GL_TRUE);	
}

/*
void Shader::setConstantValue<T>(const std::string& constantName, const T& value) {
		std::ostringstream s;
		s << value;
		compileTimeConstants[constantName] = s;
}
*/

///////

ShaderProgram::ShaderProgram(const std::string& _label) : label(_label), active(false), initialized(false) {
	id = glCreateProgram();	
}

ShaderProgram::~ShaderProgram() {
	printf("deleting shader program %s (%d)\n", label.c_str(), id);
	glDeleteProgram(id);
}

GfxShaderProgram::GfxShaderProgram(const std::string& _label,
							 const char * vert_file_path,
							 const char * frag_file_path,
							 const char * geom_file_path,
							 std::map<std::string, std::string> compileTimeConst) : ShaderProgram(_label) {
	Shader vs(Shader::VERTEX);
	vs.setCompileTimeConstantsMap(compileTimeConst);
	vs.loadCode(vert_file_path);
	vs.compile();
	addShader(vs);
		
	Shader fs(Shader::FRAGMENT);
	fs.setCompileTimeConstantsMap(compileTimeConst);
	fs.loadCode(frag_file_path);
	fs.compile();
	addShader(fs);
	
	if (geom_file_path) {
		Shader gs(Shader::GEOMETRY);
		gs.setCompileTimeConstantsMap(compileTimeConst);
		gs.loadCode(geom_file_path);
		gs.compile();
		addShader(gs);
	}	
}

ComputeShaderProgram::ComputeShaderProgram(const std::string& _label, const char * comp_file_path	 ) : ShaderProgram(_label) {
	Shader cs(Shader::COMPUTE);    
	cs.loadCode(comp_file_path);
	cs.compile();
	addShader(cs);
}
	
void ShaderProgram::addShader(const Shader &s) {	 
	glAttachShader(id, s.getId());
}

bool ShaderProgram::link() {
	// link
	printf("Linking shader program %s\n", label.c_str());
	glLinkProgram(id);

	// check
	GLint result = GL_FALSE;
	GLint infoLogLength;	
	glGetProgramiv(id, GL_LINK_STATUS,     &result);
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
	if ( infoLogLength > 0 ) {
		std::vector<char> programErrorMessage(infoLogLength+1);
		glGetProgramInfoLog(id, infoLogLength, NULL, &programErrorMessage[0]);
		printf("%s\n", &programErrorMessage[0]);
		//Shader::dumpLogToFile(programErrorMessage, "link"+std::to_string(id));
		Shader::dumpLogToFile(programErrorMessage, label + ".link."+std::to_string(id));
	}
	initialized = true;

//	VSGLInfoLib::getUniformsInfo(getId());
//	assert(result);
	return (result == GL_TRUE);	
}


