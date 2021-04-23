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

#include "shader.hpp"
#include "snow.hpp"

using namespace cg;

// window settings
int screenWidth = 800;
int screenHeight = 600;

Snowing snowing(3, 0.15f, 0.5f, screenWidth, screenHeight, {0, -4.0f, 0}, 12, 10, 50, {0, -9.8f, 0});

// normalized coordinates
constexpr GLfloat background[] = {
	// Positions        // tex coord
    -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // Top Left
    0.5f,   0.5f,  0.0f, 1.0f, 1.0f,  // Top Right
    0.5f,  -0.5f,  0.0f, 1.0f, 0.0f,  // Bottom Right

    0.5f,  -0.5f,  0.0f, 1.0f, 0.0f,  // Bottom Right
    -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  // Bottom Left
    -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // Top Left
};

// callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

void drawBackground(const Shader& shader, GLuint VAO, GLuint texture, const glm::mat4& view, const glm::mat4& projection);

int main()
{
	// Setup a GLFW window

	// init GLFW, set GL version & pipeline info
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Yifei Li - Assignment 4", nullptr, nullptr);
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

	// Setup OpenGL options
	glEnable(GL_DEPTH_TEST);

    // ---------------------------------------------------------------

	// Install GLSL Shader programs
	auto snowShader = Shader::Create("snow.vert", "snow.frag");
	if (snowShader == nullptr) {
		std::cerr << "Error creating Shader Program" << std::endl;
		glfwTerminate();
		return -3;
	}

    auto backShader = Shader::Create("background.vert", "background.frag");
    if (backShader == nullptr) {
        std::cerr << "Error creating Shader Program" << std::endl;
        glfwTerminate();
        return -3;
    }

    GLuint texBack = 0;
    if ((texBack = SOIL_load_OGL_texture(
        "bg.jpg",
        SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    )) == 0) {
        glfwTerminate();
        return -4;
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    GLuint texSnow = 0;
    if ((texSnow = SOIL_load_OGL_texture(
        "snow.png",
        SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    )) == 0) {
        glfwTerminate();
        glDeleteTextures(1, &texBack);
        return -4;
    }
    glBindTexture(GL_TEXTURE_2D, 0);


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
	glBufferData(GL_ARRAY_BUFFER, sizeof(background), background, GL_STATIC_DRAW);

	// set vertex attribute pointers
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// tex coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// unbind VBO & VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// ---------------------------------------------------------------

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);

	// Update loop

    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;

    glm::mat4 view = glm::lookAt(glm::vec3{0, 0, 100}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});

	while (glfwWindowShouldClose(window) == 0) {
        GLfloat currentFrame = GLfloat(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

		// check event queue
		glfwPollEvents();

		/* your update code here */
        snowing.Update(deltaTime);
	
		// draw background
		GLfloat red = 0.2f;
		GLfloat green = 0.3f;
		GLfloat blue = 0.3f;
		glClearColor(red, green, blue, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::ortho(
            -GLfloat(screenWidth) / 2,
            GLfloat(screenWidth) / 2,
            -GLfloat(screenHeight) / 2,
            GLfloat(screenHeight) / 2,
            -1000.0f, 1000.0f
        );

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// draw
        drawBackground(*backShader, VAO, texBack, view, projection);
        snowing.Draw(*snowShader, VAO, texSnow, view, projection);

		// swap buffer
		glfwSwapBuffers(window);
	}

	// properly de-allocate all resources
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &texBack);
    glDeleteTextures(1, &texSnow);

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
    snowing.SetPositionRange(screenWidth, screenHeight);
	// resize window
	glViewport(0, 0, width, height);
}

void drawBackground(const Shader& shader, GLuint VAO, GLuint texture, const glm::mat4& view, const glm::mat4& projection)
{
    glm::mat4 model = glm::translate(
        glm::scale(glm::mat4(1.0f), {screenWidth, screenHeight, 1.0f}),
        {0, 0, -50.0f}
    );

    shader.Use();
    glUniformMatrix4fv(glGetUniformLocation(shader.Program(), "projection"), 1, false, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program(), "view"), 1, false, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program(), "model"), 1, false, glm::value_ptr(model));

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
