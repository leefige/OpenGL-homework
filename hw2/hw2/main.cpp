/*
 * OpenGL version 3.3 project.
 */
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL2/SOIL2.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "shader.hpp"
#include "particalsys.hpp"

using namespace cg;

// window settings
int screenWidth = 1920;
int screenHeight = 1080;

float spriteScale = 80;

std::vector<std::unique_ptr<ArchimedesRad> > archs;

constexpr const char* const SPRITE_FILE = "Star.bmp";

constexpr const GLfloat particle_quad[] = {
    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,

    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 0.0f
};

// callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

void drawMass(const Mass& mass, const Shader& shader, GLuint VAO, GLuint texture);

int main()
{
	// Setup a GLFW window

	// init GLFW, set GL version & pipeline info
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Yifei Li - Assignment 2", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Error creating window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// use newly created window as context
	glfwMakeContextCurrent(window);

	// register callbacks
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	// ---------------------------------------------------------------

	// Connect GLAD to GLFW by registerring glfwGetProcAddress() as GLAD loader function,
	// this must be done after setting current context

	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		std::cerr << "Error registerring gladLoadGLLoader" << std::endl;
		glfwTerminate();
		return -2;
	}

	// ---------------------------------------------------------------

	// Setup OpenGL options
	glEnable(GL_DEPTH_TEST);

	// Install GLSL Shader programs
	auto shaderProgram = Shader::Create("star.vert", "star.frag");
	if (shaderProgram == nullptr) {
		std::cerr << "Error creating Shader Program" << std::endl;
		glfwTerminate();
		return -3;
	}

    // ---------------------------------------------------------------

    // Load and create a texture

    // Load, create texture and generate mipmaps
    GLuint texture = SOIL_load_OGL_texture(
        SPRITE_FILE,
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_TEXTURE_REPEATS
    );
    if (texture == 0) {
        std::cerr << "Error loading sprite '" << SPRITE_FILE << "'" << std::endl;
        glfwTerminate();
        return -4;
    }
    // Unbind texture when done, so we won't accidentily mess up our texture.
    glBindTexture(GL_TEXTURE_2D, 0);


    // ---------------------------------------------------------------

    for (int i = 0; i < 360; i += 30) {
        archs.emplace_back(std::make_unique<ArchimedesRad>(4, glm::vec3(0, 0, 0), 5, i, 30));
    }
        
	// ---------------------------------------------------------------

	// Set up vertex data (and buffer(s)) and attribute pointers

	// bind VAO
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// bind VBO, buffer data to it
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

	// set vertex attribute pointers
	// position attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// unbind VBO & VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// ---------------------------------------------------------------

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);

    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;

    Mass origin;
    origin.color = glm::vec4(1.0f);
    origin.position = glm::vec3(-0.1f);

	// Update loop
	while (glfwWindowShouldClose(window) == 0) {
        // Calculate deltatime of current frame
        GLfloat currentFrame = GLfloat(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

		// check event queue
		glfwPollEvents();

		/* your update code here */
	
		// draw background
		glClearColor(0, 0, 0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Draw
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);

        glm::mat4 projection = glm::ortho(
            -GLfloat(screenWidth) / 2,
            GLfloat(screenWidth) / 2,
            -GLfloat(screenHeight) / 2,
            GLfloat(screenHeight) / 2,
            -1.0f, 100.0f
        );

        shaderProgram->Use();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram->Program(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        drawMass(origin, *shaderProgram, VAO, texture);

        for (auto& spawner : archs) {
            // let particle system update
            spawner->Process(deltaTime * 3, {0, 0, 0}); // apply gravity and update speed, position
            // draw particle
            for (int j = 0; j < spawner->GetMassNum(); ++j) {
                drawMass(spawner->GetMass(j), *shaderProgram, VAO, texture);
            }
        }

		// swap buffer
		glfwSwapBuffers(window);
	}

	// properly de-allocate all resources
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &texture);

	glfwTerminate();
	return 0;
}

/* ======================== helper functions ======================== */

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// exit when pressing ESC
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    screenWidth = width;
    screenHeight = height;
	// resize window
	glViewport(0, 0, width, height);
}

void drawMass(const Mass& mass, const Shader& shader, GLuint VAO, GLuint texture)
{
    glUniform2fv(glGetUniformLocation(shader.Program(), "offset"), 1, glm::value_ptr(mass.position));
    glUniform4fv(glGetUniformLocation(shader.Program(), "color"), 1, glm::value_ptr(mass.color));
    glUniform1f(glGetUniformLocation(shader.Program(), "scale"), spriteScale);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
