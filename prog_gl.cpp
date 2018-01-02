#include <cassert>
#include <cstring>

#include <glibmm/bytes.h>
#include <giomm/resource.h>

#include "prog_gl.hpp"
#include "train_tracks_app_window.hpp"

#define MAX_GLSL_NAME_LENGHT 256

ProgGlBaseException::ProgGlBaseException(const ProgGlBaseException& other) : std::runtime_error(other), m_msg(new char[other.m_msg_lenght+1]),
		m_msg_lenght(other.m_msg_lenght) {
	strcpy(m_msg, other.m_msg);
}


ProgGlBaseException::ProgGlBaseException(ProgGlBaseException&& other) : std::runtime_error(std::move(other)), m_msg(other.m_msg), m_msg_lenght(other.m_msg_lenght) {
	other.m_msg = nullptr;
	other.m_msg_lenght = 0;
}

ProgGlBaseException& ProgGlBaseException::operator=(const ProgGlBaseException& other) {
	std::runtime_error::operator=(other);
	
	delete [] m_msg;
	m_msg_lenght = other.m_msg_lenght;
	m_msg = new char[m_msg_lenght+1];
	strcpy(m_msg, other.m_msg);
	return *this;
}

ProgGlBaseException& ProgGlBaseException::operator=(ProgGlBaseException&& other) {
	std::runtime_error::operator=(std::move(other));
	
	delete [] m_msg;
	m_msg_lenght = other.m_msg_lenght;
	m_msg = other.m_msg;
	other.m_msg = nullptr;
	other.m_msg_lenght = 0;
	return *this;
}

ProgGlCompileException ProgGlCompileException::create(int shaderType) {
	const char* msg;
	switch (shaderType) {
		case GL_VERTEX_SHADER:
			msg = "Erreur de compilation du vertex shader: ";
			break;
		
		case GL_FRAGMENT_SHADER:
			msg = "Erreur de compilation du fragment shader: ";
			break;
		
		case GL_GEOMETRY_SHADER:
			msg = "Erreur de compilation du geometry shader: ";
			break;
		
		default:
			msg = "Erreur de compilation du shader de type inconnu: ";
			break;
	};
	return ProgGlCompileException(msg);
}

void ProgGlCompileException::setMessage(GLuint shaderId) {
	int logLen;
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLen);
	allocateMsg(logLen);
	glGetShaderInfoLog(shaderId, logLen, NULL, getMsg());
}

void ProgGlLinkException::setMessage(GLuint programId) {
	int logLen = 0;
	glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLen);
	allocateMsg(logLen);
	glGetProgramInfoLog(programId, logLen, NULL, getMsg());
}

ProgGlUndefinedVariableException ProgGlUndefinedVariableException::create(VariableType type) {
	const char* msg;
	switch (type) {
		case ATTRIBUTE:
			msg = "Attribut non-definie: ";
			break;
		
		case UNIFORM:
			msg = "Variable uniforme non-definie: ";
			break;
	}
	return ProgGlUndefinedVariableException(msg);
}

void ProgGlUndefinedVariableException::setMessage(const char* msg) {
	allocateMsg(strnlen(msg, MAX_GLSL_NAME_LENGHT));
	strncpy(getMsg(), msg, MAX_GLSL_NAME_LENGHT);
	getMsg()[lenght()] = '\0';
}





ProgramsGL::Shader::Shader(std::string src_location, int type) {
	gsize size;
	Glib::RefPtr<const Glib::Bytes> src(Gio::Resource::lookup_data_global(src_location, Gio::RESOURCE_LOOKUP_FLAGS_NONE));
	const char* str_src = static_cast<const char*>(src->get_data(size));
	
	m_shader_id = glCreateShader(type);
	glShaderSource (m_shader_id, 1, &str_src, NULL);
	glCompileShader (m_shader_id);
	
	int status;
	glGetShaderiv(m_shader_id, GL_COMPILE_STATUS, &status);
	if (status==GL_FALSE) {
		ProgGlCompileException err(ProgGlCompileException::create(GL_VERTEX_SHADER));
		err.setMessage(m_shader_id);
		glDeleteShader (m_shader_id);
		throw err;
	}
}

void ProgramsGL::Program::attachShaders(Shader** shaders, size_t num_shaders) {
	for (size_t i=0; i<num_shaders; i++)
	{
		glAttachShader(m_program_id, shaders[i]->getShaderId());
	}
}

void ProgramsGL::Program::link() {
	glLinkProgram(m_program_id);
	
	int status = 0;
	glGetProgramiv (m_program_id, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		ProgGlLinkException err(ProgGlLinkException::create());
		err.setMessage(m_program_id);
		glDeleteProgram(m_program_id);
		throw err;
    }
}

void ProgramsGL::Program::loadUniforms(const char** names, size_t num) {
	for (size_t i=0; i<num; i++) {
		GLint uniform = glGetUniformLocation(m_program_id, names[i]);
		if (uniform != -1) {
			m_uniforms.push_back(uniform);
		} else {
			ProgGlUndefinedVariableException err(ProgGlUndefinedVariableException::create(ProgGlUndefinedVariableException::UNIFORM));
			err.setMessage(names[i]);
			throw err;
		}
	}
}

void ProgramsGL::Program::loadAttributes(const char** names, size_t num) {
	for (size_t i=0; i<num; i++) {
		GLint attribute = glGetAttribLocation(m_program_id, names[i]);
		if (attribute != -1) {
			m_attributes.push_back(attribute);
		} else {
			ProgGlUndefinedVariableException err(ProgGlUndefinedVariableException::create(ProgGlUndefinedVariableException::ATTRIBUTE));
			err.setMessage(names[i]);
			throw err;
		}
	}
}

void ProgramsGL::Program::detachShaders()
{
	if (!m_program_id) return;
	
	GLsizei count;
	glGetProgramiv(m_program_id, GL_ATTACHED_SHADERS, &count);
	if (count > 0) {
		GLuint* attached_shaders = new GLuint[count];
		glGetAttachedShaders(m_program_id, count, NULL, attached_shaders);
		
		for (int i=0; i<count; i++) 
				glDetachShader(m_program_id, attached_shaders[i]);
		
		delete [] attached_shaders;
	}
}

ProgramsGL::Program& ProgramsGL::Program::operator=(Program&& other) {
	if (glInited) { // Cause une erreur de segmentation si OpenGl n'est pas initialise
		detachShaders();
		glDeleteProgram(m_program_id);
	}
	
	m_program_id = other.m_program_id;
	m_uniforms = std::move(other.m_uniforms);
	m_attributes = std::move(other.m_attributes);
	other.m_program_id = 0;
	return *this;
}

ProgramsGL::Program::~Program() {
	if (glInited) { // Cause une erreur de segmentation si OpenGl n'est pas initialise
		detachShaders();
		glDeleteProgram(m_program_id);
	}
}

ProgramsGL::ProgramsGL(int) : m_programs{std::move(Program(0))} {
	Shader vertex_shader("/ca/usherbrooke/math/lwatson/train_tracks/train_tracks_vertex.glsl", GL_VERTEX_SHADER);
	Shader fragment_shader("/ca/usherbrooke/math/lwatson/train_tracks/train_tracks_fragment.glsl", GL_FRAGMENT_SHADER);
	
	Shader* shaders[2] = {&vertex_shader, &fragment_shader};
	const char*  uniforms[2]   = {"matrix", "vertexColor"};
	const char*  attributes[1] = {"position"};
	m_programs[0].attachShaders(shaders, 2);
	m_programs[0].link();
	m_programs[0].loadUniforms(uniforms, 2);
	m_programs[0].loadAttributes(attributes, 1);
	m_programs[0].detachShaders();
}
