/*
 * OpenGL version 3.3 project.
 */
#include <iostream>
#include <vector>
#include <array>

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

GLfloat speed = 0.5f;


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

constexpr const float PLANET_RADIA[] = {
    20.0f,
    1,
    3,
    3.5,
    2,
    15.209f,
    11.44f,
    8.007f,
    6.883f,
    0.35f
};

constexpr const float PLANET_ORB[] = {
    0.0f,
    25,
    32,
    43,
    57,
    80,
    140,
    200,
    270,
    3.9f
};

constexpr const float PLANET_SPEED[] = {
    5,
    90,
    80,
    70,
    60,
    50,
    40,
    30,
    15,
    800
};

constexpr const float TEXT_Y[] = {
    90,
    40,
    -40,
    60,
    -50,
    70,
    60,
    60,
    60,
    -40
};

GLuint textures[10]{0};

std::array<glm::vec3, 10> positions;
std::array<GLfloat, 10> angles{0.0f};
std::array<glm::vec4, 10> textPos;

Camera camera({0, 0, 100}, {0, 0, -1}, 20);

GLfloat sizeFactor = 4;

bool keys[1024]{false};


// callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void moveCamera(GLfloat deltaTime);

char Upper(const char& c) { return char(c - 32); }
void releaseTextures() { glDeleteTextures(10, textures); }

glm::vec3 rotateAround(glm::vec3 position, glm::vec3 origin, GLfloat angle);

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
    //glfwSetCursorPosCallback(window, mouseCallback);


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
    const std::shared_ptr<UnitSphere> unitSphere = std::make_shared<UnitSphere>(50, 50);

    std::vector<Sphere> planets;
    for (int i = 0; i < 10; i++) {
        planets.emplace_back(PLANET_RADIA[i] * sizeFactor, unitSphere);
    }

	// ---------------------------------------------------------------

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);

	// Update loop
    GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
    GLfloat lastFrame = 0.0f;    // Time of last frame

    // init positions
    for (int i = 0; i < 10; i++) {
        if (i == 0) {
            positions[i] = glm::vec3{PLANET_ORB[i] * sizeFactor, 0, 0};
        } else if (i == 9) {
            positions[9] = glm::vec3{PLANET_ORB[i] * sizeFactor, 0, 0};
        } else {
            positions[i] = glm::vec3{PLANET_ORB[i] * sizeFactor, 0, 0};
        }
    }

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
		GLfloat red = 0.1f;
		GLfloat green = 0.1f;
		GLfloat blue = 0.1f;
		glClearColor(red, green, blue, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto screenOrigin = glm::vec2{-static_cast<GLfloat>(screenWidth) / 2, -static_cast<GLfloat>(screenHeight) / 2};

		shaderProgram->Use();
        glm::mat4 view = glm::lookAt(camera.Position(), {0.0f, 0.0f, 0.0f}, camera.Up());
        glm::mat4 UIprojection = glm::ortho(
            screenOrigin.x,
            screenOrigin.x + screenWidth,
            screenOrigin.y,
            screenOrigin.y + screenHeight,
            -5000.0f, 5000.0f
        );

        for (int i = 0; i < 10; i++) {
            positions[i] = rotateAround(positions[i], glm::vec3(0.0f), deltaTime * PLANET_SPEED[i] * speed);
            angles[i] = fmod(angles[i] + deltaTime * PLANET_RADIA[i] * 50 * speed, 360);

            glm::mat4 model = planets[i].Model();

            if (i < 9) {
                model = glm::translate(glm::mat4(1.0f), positions[i]) * glm::rotate(glm::mat4(1.0f), glm::radians(angles[i]), {0,1,0}) * model;
            } else {
                model = glm::translate(glm::mat4(1.0f), positions[i] + positions[3]) * glm::rotate(glm::mat4(1.0f), glm::radians(angles[i]), {0,1,0}) * model;
            }

            auto pvm = UIprojection * view * model;

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram->Program(), "pvm"), 1, GL_FALSE, glm::value_ptr(pvm));

            glBindTexture(GL_TEXTURE_2D, textures[i]);

            planets[i].Draw();

            glBindTexture(GL_TEXTURE_2D, 0);

        }
        for (int i = 0; i < 10; i++) {
            const auto& pos = i < 9 ? positions[i] : positions[i] + positions[3];
            auto textPos = view * glm::vec4{pos.x, pos.y, pos.z, 1.0f};
            arial.RenderText(PLANET_NAMES[i], textPos.x - 30.0f, TEXT_Y[i], 0.5f, UIprojection, glm::vec3{0.6f, 0.9f, 0.6f});
        }

        arial.RenderText("Hello, hw3", screenOrigin.x + 25, screenOrigin.y + 25, 1.5, UIprojection, glm::vec3{0.8f, 0.7f, 0.3f});

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

glm::vec3 rotateAround(glm::vec3 position, glm::vec3 origin, GLfloat angle)
{
    auto toward = position - origin;
    auto res = glm::rotate(glm::mat4(1.0f), glm::radians(angle), {0, 1, 0}) * glm::vec4{toward.x, toward.y, toward.z, 1.0f};
    return origin + glm::vec3{res.x, res.y, res.z};
}
