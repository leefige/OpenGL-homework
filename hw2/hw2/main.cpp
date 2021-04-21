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
#include "spirals.hpp"
#include "text.hpp"

using namespace cg;

// window settings
int screenWidth = 1280;
int screenHeight = 960;

float spriteScale = 80;
float period = 4;

enum class DisplayMode : int
{
    ARCHIMEDES,
    FERMAT,
    LOGARITHMIC
};

ArchimedesSpiral archi(period, 24, 18, spriteScale, 50);
LogarithmicSpiral logar(period, 24, 18, spriteScale, 10, 0.45);
FermatSpiral ferma(period, 12, 10, spriteScale, 190, 50);

constexpr const char* SPIRAL_NAMES[3] = {
    "Archimedes",
    "Fermat",
    "Logarithmic",
};

DisplayMode currentMode = DisplayMode::ARCHIMEDES;
bool showText = true;

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

    Text arial;
    if (!arial.LoadFont("arial.ttf")) {
        std::cerr << "Error loading font '" << "arial.ttf" << "'" << std::endl;
        glfwTerminate();
        return -5;
    }

    if (!arial.LoadShaders("text.vert", "text.frag")) {
        std::cerr << "Error creating text shaders" << std::endl;
        glfwTerminate();
        return -5;
    }

    // ---------------------------------------------------------------

    // Load and create a texture

    // Load, create texture and generate mipmaps
    // Load and create a texture
    GLuint texture;
    // ====================
    // Texture
    // ====================
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // Set texture wrapping to GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    int width, height;
    unsigned char* image = SOIL_load_image(SPRITE_FILE, &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.


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

    glm::mat4 view = glm::lookAt(glm::vec3{0, 0, 10}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});

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
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glm::mat4 projection = glm::ortho(
            -GLfloat(screenWidth) / 2,
            GLfloat(screenWidth) / 2,
            -GLfloat(screenHeight) / 2,
            GLfloat(screenHeight) / 2,
            -1.0f, 1000.0f
        );

        shaderProgram->Use();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram->Program(), "pv"), 1, GL_FALSE, glm::value_ptr(projection * view));

        archi.Update(deltaTime);
        logar.Update(deltaTime);
        ferma.Update(deltaTime);

        switch (currentMode) {
        case DisplayMode::ARCHIMEDES:
            archi.Draw(*shaderProgram, VAO, texture);
            break;
        case DisplayMode::FERMAT:
            ferma.Draw(*shaderProgram, VAO, texture);
            break;
        case DisplayMode::LOGARITHMIC:
            logar.Draw(*shaderProgram, VAO, texture);
            break;
        default:
            break;
        }

        if (showText) {
            auto screenOrigin = glm::vec2{-static_cast<GLfloat>(screenWidth) / 2, -static_cast<GLfloat>(screenHeight) / 2};
            arial.RenderText(std::string("Press Enter to switch spiral type. Current type: ") + SPIRAL_NAMES[static_cast<int>(currentMode)] + ".", screenOrigin.x + 25, screenOrigin.y + 55, 0.5, projection, glm::vec3{0.8f, 0.7f, 0.3f});
            arial.RenderText("Press Ctrl to turn on/off showing this message.", screenOrigin.x + 25, screenOrigin.y + 25, 0.5, projection, glm::vec3{0.8f, 0.7f, 0.3f});
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
    } else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        currentMode = static_cast<DisplayMode>((static_cast<int>(currentMode) + 1) % 3);
    } else if ((key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) && action == GLFW_PRESS) {
        showText = !showText;
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    screenWidth = width;
    screenHeight = height;
	// resize window
	glViewport(0, 0, width, height);
}


