#include <glib.h>
#include <gio/gio.h>

#include "prog_gl.hpp"
#include "train_tracks_app_window.h"

ProgramsGL::Shader::Shader(std::string src_location, int type) {
	GBytes* src = g_resources_lookup_data(src_location.c_str(), G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
	const char* str_src = static_cast<const char*>(g_bytes_get_data (src, NULL));
	
	m_shader_id = glCreateShader(type);
	glShaderSource (m_shader_id, 1, &str_src, NULL);
	glCompileShader (m_shader_id);
	g_bytes_unref(src);
	
	int status;
	glGetShaderiv(m_shader_id, GL_COMPILE_STATUS, &status);
	if (status==GL_FALSE) {
		int log_len;
		glGetShaderiv(m_shader_id, GL_INFO_LOG_LENGTH, &log_len);

		char *buffer = new char[log_len + 1];
		glGetShaderInfoLog(m_shader_id, log_len, NULL, buffer);

		std::string msg;
		msg += "Erreur de compilation du ";
		msg += (type == GL_VERTEX_SHADER ? "vertex " : "fragment ");
		msg += "shader: ";
		msg += buffer;

		delete [] buffer;
		glDeleteShader (m_shader_id);
		throw msg;
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
		int log_len = 0;
		glGetProgramiv(m_program_id, GL_INFO_LOG_LENGTH, &log_len);

		char *buffer = new char[log_len + 1];
		glGetProgramInfoLog(m_program_id, log_len, NULL, buffer);
		
		std::string msg;
		msg += "Erreur d'edition de lien: ";
		msg += buffer;

		delete [] buffer;

		glDeleteProgram(m_program_id);
		throw msg;
    }
}

void ProgramsGL::Program::loadUniforms(const char** names, size_t num) {
	for (size_t i=0; i<num; i++) {
		GLint uniform = glGetUniformLocation(m_program_id, names[i]);
		if (uniform != -1)
			m_uniforms.push_back(uniform);
		else
			throw std::string("Variable uniforme \"") + names[i] + "\" non-definie";
	}
}

void ProgramsGL::Program::loadAttributes(const char** names, size_t num) {
	for (size_t i=0; i<num; i++) {
		GLint attribute = glGetAttribLocation(m_program_id, names[i]);
		if (attribute != -1)
			m_attributes.push_back(attribute);
		else
			throw std::string("Attribut \"") + names[i] + "\" non-definie";
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
