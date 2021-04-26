#ifndef CG_SHADER_H_
#define CG_SHADER_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include <glad/glad.h>

namespace cg
{

class Shader
{
	const GLuint shaderProgram;

	Shader() = delete;
	Shader(const Shader&) = delete;
	Shader(Shader&&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader& operator=(Shader&&) = delete;

	explicit Shader(const GLuint& prog) : shaderProgram(prog) {}
	explicit Shader(GLuint&& prog) : shaderProgram(prog) {}

public:
	virtual ~Shader() { glDeleteProgram(shaderProgram); }

	static std::unique_ptr<Shader> Create(const std::string& vertexFilename, const std::string& fragmentFilename)
	{
		// Build and compile our shader programs

		// Vertex shaders
		const GLuint vertexShader = CompileShader(vertexFilename, GL_VERTEX_SHADER);
		if (vertexShader == 0) {
			std::cerr << "Cannot create Vertex Shader from file '" << vertexFilename << "'." << std::endl;
			return nullptr;
		}

		// Fragment shaders
		const GLuint fragmentShader = CompileShader(fragmentFilename, GL_FRAGMENT_SHADER);
		if (fragmentShader == 0) {
			std::cerr << "Cannot create Fragment Shader from file '" << fragmentFilename << "'." << std::endl;
			glDeleteShader(vertexShader);
			return nullptr;
		}

		// link shaders: including vertex & fragment shaders
		const GLuint program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		// release input shaders
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		// check for linking errors
		GLint success;
		const GLsizei logLen = 512;
		GLchar infoLog[logLen];
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(program, logLen, NULL, infoLog);
			std::cerr << "Link Shader error: " << infoLog << std::endl;
			glDeleteProgram(program);
			return nullptr;
		}

		return std::unique_ptr<Shader>(new Shader(program));
	}

	const GLuint Program() const { return shaderProgram; }

	void Use() const { glUseProgram(shaderProgram); }

private:

	static const GLuint CompileShader(const std::string& filename, GLenum type)
	{
		std::ifstream fin;

		// ensures ifstream objects can throw exceptions:
		fin.exceptions(std::ifstream::badbit);
		try {
			fin.open(filename, std::ios::in);
		}
		catch (const std::ifstream::failure& e) {
			std::cerr << "Shader: open file '" << filename << "' error: " << e.what() << std::endl;
			return 0;
		}

		// read all content from file
		std::ostringstream stream;
		fin.exceptions(std::ifstream::badbit);
		try {
			stream << fin.rdbuf();
		}
		catch (const std::ifstream::failure& e) {
			std::cerr << "Shader: read file '" << filename << "' error: " << e.what() << std::endl;
			fin.close();
			return 0;
		}

		// finish reading
		fin.close();

		const std::string source = stream.str();
		const GLchar* source_cstr = source.c_str();
		const GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &source_cstr, NULL);
		glCompileShader(shader);

		// Check compile errors
		GLint success;
		const GLsizei logLen = 512;
		GLchar infoLog[logLen];
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, logLen, NULL, infoLog);
			std::cerr << "Shader: Shader file '" << filename << "' compile error: " << infoLog << std::endl;
			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}
};

} /* namespace cg */

#endif /* CG_SHADER_H_ */
