#ifndef __PROG_GL_HPP__
#define __PROG_GL_HPP__

#include <string>
#include <cstdarg>
#include <cstdlib>
#include <vector>
#include <stdexcept>

#include <GL/glew.h>
#include <GL/gl.h>

class ProgGlBaseException : public std::runtime_error {
	char* m_msg = nullptr;
	size_t m_msg_lenght = 0;
	
public:
	ProgGlBaseException(const char* what) : std::runtime_error(what) {};
	ProgGlBaseException(const ProgGlBaseException& other);
	ProgGlBaseException(ProgGlBaseException&& other);
	
	virtual ~ProgGlBaseException() {delete [] m_msg;}
	
	ProgGlBaseException& operator=(const ProgGlBaseException& other);
	ProgGlBaseException& operator=(ProgGlBaseException&& other);
	const char* getMessage() const {return m_msg;}
	
protected:
	char*& getMsg() {return m_msg;}
	void allocateMsg(size_t strlen) {m_msg_lenght = strlen ;m_msg = new char[strlen+1];}
	size_t lenght() const {return m_msg_lenght;}
};

class ProgGlCompileException : public ProgGlBaseException {
	ProgGlCompileException(const char* what) : ProgGlBaseException(what) {}
	
public:
	static ProgGlCompileException create(int shaderType);
	void setMessage(GLuint shaderId);
};

class ProgGlLinkException : public ProgGlBaseException {
	ProgGlLinkException(const char* what) : ProgGlBaseException(what) {}
	
public:
	static ProgGlLinkException create() {return ProgGlLinkException("Erreur d'edition de lien: ");}
	void setMessage(GLuint programId);
};

class ProgGlUndefinedVariableException : public ProgGlBaseException {
	ProgGlUndefinedVariableException(const char* what) : ProgGlBaseException(what) {}
	
public:
	enum VariableType {
		ATTRIBUTE,
		UNIFORM
	};
	
	static ProgGlUndefinedVariableException create(VariableType type);
	void setMessage(const char* msg);
};

class ProgramsGL {
  private:
	class Shader {
	  private:
		GLuint m_shader_id = 0;
		
	  public:
		Shader(const Shader& shader) = delete;
		Shader(std::string src_location, int type);
		~Shader()  {glDeleteShader(m_shader_id);}
			
		GLuint getShaderId() {return m_shader_id;};
	};

  public:
	enum ProgramInstance {
		DEFAULT_PROGRAM = 0
	};

	enum Uniform {
		MATRIX = 0,
		COLOR  = 1
	};

	enum Attribute {
		POSITION = 0
	};
	  
	class Program {
	  private:
		GLuint m_program_id = 0;
		std::vector<GLint> m_uniforms;
		std::vector<GLint> m_attributes;
		
		Program() = default;
		Program(int) {m_program_id = glCreateProgram();}
		Program(const Program& other) = delete;
		Program(Program&& other) : m_program_id(other.m_program_id), m_uniforms(std::move(other.m_uniforms)), m_attributes(std::move(other.m_attributes)) {
			other.m_program_id = 0;
		}
		
		~Program();
		
		void attachShaders(Shader** shaders, size_t num_shaders);
		void link();
		void loadUniforms(const char* names[], size_t num);
		void loadAttributes(const char* names[], size_t num);
		void detachShaders();
		
	  public:
		void use() const {glUseProgram(m_program_id);}
		void unuse() const {glUseProgram(0);}
		
		GLint getUniform(Uniform uniform) const {return m_uniforms.at(uniform);}
		GLint getAttribute(Attribute attribute) const {return m_attributes.at(attribute);}
		
		Program& operator=(Program&& other);
		
		friend ProgramsGL;
	};
	
  private:
	Program m_programs[1];
	
  public:
	ProgramsGL() = default;
	ProgramsGL(int);
	ProgramsGL(const ProgramsGL& other) = delete;
	ProgramsGL(ProgramsGL&& other) = default;
	
	const Program& getProgram(ProgramInstance program) const {return m_programs[program];}
	
	ProgramsGL& operator=(ProgramsGL&& other) = default;
};

#endif // __PROG_GL_HPP__
