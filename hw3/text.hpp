#ifndef CG_TEXT_H_
#define CG_TEXT_H_

#include <exception>
#include <iostream>
#include <map>
#include <string>

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "shader.hpp"

namespace cg
{

/// Holds all state information relevant to a character as loaded using FreeType
struct Character
{
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};

class Text
{
public:
	explicit Text(bool useDefaultShader=true) : shader_(nullptr)
	{
		if (useDefaultShader) {
			if ((shader_ = Shader::CreateFromStrings(defaultVert, defaultFrag)) == nullptr) {
				throw std::runtime_error("ERROR: Text: cannot create default shaders.");
			}
		}

		glGenVertexArrays(1, &VAO_);
		glBindVertexArray(VAO_);

		glGenBuffers(1, &VBO_);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

		// <vec2 pos, vec2 tex>
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	virtual ~Text()
	{
		glDeleteVertexArrays(1, &VAO_);
		glDeleteBuffers(1, &VBO_);
	}

	bool LoadShaders(const char* const vertexShaderFile, const char* const fragmentShaderFile)
	{
		shader_ = Shader::Create(vertexShaderFile, fragmentShaderFile);
		if (shader_ == nullptr) {
			std::cerr << "ERROR: Text: failed to create Shader Program" << std::endl;
			return false;
		}
		return true;
	}

	bool LoadFont(const char* const fontFile)
	{
		// FreeType
		FT_Library ft;
		// All functions return a value different than 0 whenever an error occurred
		if (FT_Init_FreeType(&ft)) {
			std::cout << "ERROR: FREETYPE: Could not init FreeType Library" << std::endl;
			return false;
		}

		// Load font as face
		FT_Face face;
		if (FT_New_Face(ft, fontFile, 0, &face)) {
			std::cout << "ERROR: FREETYPE: Failed to load font" << std::endl;
			FT_Done_FreeType(ft);
			return false;
		}

		// Set size to load glyphs as
		FT_Set_Pixel_Sizes(face, 0, 48);

		// Disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Load first 128 characters of ASCII set
		if (characters_.size() > 0) {
			characters_.clear();
		}

		GLuint textures[128];
		glGenTextures(128, textures);
		for (GLubyte c = 0; c < 128; c++) {
			// Load character glyph
			if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0) {
				std::cout << "WARNING: FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}
			// Generate texture
			glBindTexture(GL_TEXTURE_2D, textures[int(c)]);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);
			// Set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// Now store character for later use
			Character character{
				textures[int(c)],
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				GLuint(face->glyph->advance.x)
			};
			characters_.insert(std::pair<GLchar, Character>(c, character));
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		// Destroy FreeType once we're finished
		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		return true;
	}

	void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, const glm::vec3& color, int screenWidth, int screenHeight) const
	{
		// Activate corresponding render state
		shader_->Use();

		glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(screenWidth), 0.0f, static_cast<GLfloat>(screenHeight));
		glUniformMatrix4fv(glGetUniformLocation(shader_->Program(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3f(glGetUniformLocation(shader_->Program(), "textColor"), color.x, color.y, color.z);

		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(VAO_);

		// Iterate through all characters
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++) {
			Character ch = characters_.at(*c);

			GLfloat xpos = x + ch.Bearing.x * scale;
			GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			GLfloat w = ch.Size.x * scale;
			GLfloat h = ch.Size.y * scale;
			// Update VBO for each character
			GLfloat vertices[6][4] = {
				{ xpos,     ypos + h,   0.0, 1.0 },
				{ xpos,     ypos,       0.0, 0.0 },
				{ xpos + w, ypos,       1.0, 0.0 },

				{ xpos,     ypos + h,   0.0, 1.0 },
				{ xpos + w, ypos,       1.0, 0.0 },
				{ xpos + w, ypos + h,   1.0, 1.0 }
			};

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.TextureID);

			// Update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, VBO_);
			// Be sure to use glBufferSubData and not glBufferData
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// draw char
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisable(GL_BLEND);

			// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

private:
	GLuint VAO_;
	GLuint VBO_;
	std::unique_ptr<Shader> shader_;
	std::map<GLchar, Character> characters_;

public:
	static constexpr auto defaultVert = 
		"#version 330 core\n"
		"// <vec2 pos, vec2 tex>\n"
		"layout(location = 0) in vec4 vertex;"
		"out vec2 TexCoords;"
		"uniform mat4 projection;"
		"void main() {"
		"    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);"
		"    TexCoords = vec2(vertex.z, 1.0f - vertex.w); }"
	;
	static constexpr auto defaultFrag = 
		"#version 330 core\n"
		"in vec2 TexCoords;"
		"out vec4 color;"
		"uniform sampler2D text;"
		"uniform vec3 textColor;"
		"void main() {"
		"    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);"
		"    color = vec4(textColor, 1.0) * sampled; }"
	;
};

} /* namespace cg */

#endif /* CG_TEXT_H_ */
