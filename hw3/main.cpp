/*
 * OpenGL version 3.3 project.
 */
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL2/SOIL2.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "camera.hpp"
#include "shader.hpp"
#include "text.hpp"
#include "sphere.hpp"

using namespace cg;

// window settings
int screenWidth = 800;
int screenHeight = 600;


// normalized coordinates
constexpr GLfloat vertices[] = {
	// Positions        // Colors
	0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // Bottom Right
	-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  // Bottom Left
	0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f   // Top 
};

constexpr const char* const PLANET_NAMES[] = {
    "sun",
    "mercury",
    "venus",
    "earth",
    "mars",
    "jupiter",
    "saturn",
    "uranus",
    "neptune",
    "moon"
};

const UnitSphere unitSphere(50, 50);

GLuint textures[10]{0};

Camera camera({0, 0, 20}, {0, 0, -1}, 1);

bool keys[1024]{false};


// callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void moveCamera(GLfloat deltaTime);

char Upper(const char& c) { return char(c - 32); }
void releaseTextures() { glDeleteTextures(10, textures); }

int main()
{
	// Setup a GLFW window

	// init GLFW, set GL version & pipeline info
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Yifei Li - Assignment 3", nullptr, nullptr);
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
    glfwSetScrollCallback(window, scrollCallback);


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
	auto shaderProgram = Shader::Create("sphere.vert", "sphere.frag");
	if (shaderProgram == nullptr) {
		std::cerr << "Error creating Shader Program" << std::endl;
		glfwTerminate();
		return -3;
	}

    Text arial;
    if (!arial.LoadFont("arial.ttf")) {
        std::cerr << "Error loading font '" << "arial.ttf" << "'" << std::endl;
        glfwTerminate();
        return -4;
    }

    if (!arial.LoadShaders("text.vert", "text.frag")) {
        std::cerr << "Error creating text shaders" << std::endl;
        glfwTerminate();
        return -4;
    }

    for (int i = 0; i < 10; i++) {
        if ((textures[i] = SOIL_load_OGL_texture(
                (std::string("textures/") + PLANET_NAMES[i] + ".jpg").c_str(),
                SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
                SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
        )) == 0) {
            glfwTerminate();
            releaseTextures();
            return -5;
        }
    }

	// ---------------------------------------------------------------

	// Set up vertex data (and buffer(s)) and attribute pointers

	

	// ---------------------------------------------------------------

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);

	// Update loop
    GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
    GLfloat lastFrame = 0.0f;    // Time of last frame

    Sphere earth(5.0f, unitSphere);

	while (glfwWindowShouldClose(window) == 0) {
        // Calculate deltatime of current frame
        GLfloat currentFrame = GLfloat(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

		// check event queue
		glfwPollEvents();

        moveCamera(deltaTime);

		/* your update code here */
	
		// draw background
		GLfloat red = 0.2f;
		GLfloat green = 0.3f;
		GLfloat blue = 0.3f;
		glClearColor(red, green, blue, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw a triangle
		shaderProgram->Use();

        glm::mat4 model = earth.BaseModel();
        glm::mat4 view = glm::lookAt(camera.Position(), {0.0f, 0.0f, 0.0f}, camera.Up());
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom()), float(screenWidth) / float(screenHeight), 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram->Program(), "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram->Program(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram->Program(), "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindTexture(GL_TEXTURE_2D, textures[3]);

        earth.DrawSphere();

        glBindTexture(GL_TEXTURE_2D, 0);

        arial.RenderText("Hello, hw3", 25, 25, 1.5, screenWidth, screenHeight, glm::vec3{0.8f, 0.7f, 0.3f});

		// swap buffer
		glfwSwapBuffers(window);
	}

	// properly de-allocate all resources
    releaseTextures();

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
    else if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            keys[key] = false;
        }
    }
}

void moveCamera(GLfloat deltaTime)
{
    // Camera controls
    if (keys[GLFW_KEY_W]) {
        camera.ProcessKeyboard(Camera::Movement::FORWARD, deltaTime);
    }
    if (keys[GLFW_KEY_S]) {
        camera.ProcessKeyboard(Camera::Movement::BACKWARD, deltaTime);
    }
    if (keys[GLFW_KEY_A]) {
        camera.ProcessKeyboard(Camera::Movement::LEFT, deltaTime);
    }
    if (keys[GLFW_KEY_D]) {
        camera.ProcessKeyboard(Camera::Movement::RIGHT, deltaTime);
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    static GLfloat lastX = GLfloat(screenWidth) / 2;
    static GLfloat lastY = GLfloat(screenHeight) / 2;

    GLfloat xoffset = GLfloat(xpos) - lastX;
    GLfloat yoffset = lastY - GLfloat(ypos); // Reversed since y-coordinates go from bottom to left
    lastX = GLfloat(xpos);
    lastY = GLfloat(ypos);

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(GLfloat(yoffset));
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    screenWidth = width;
    screenHeight = height;
    // resize window
    glViewport(0, 0, screenWidth, screenHeight);
}

