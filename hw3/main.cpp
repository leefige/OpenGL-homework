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

using namespace cg;

struct Vertex
{
    // position
    GLfloat x, y, z;
    // normal
    GLfloat nx, ny, nz;
    // texture coord
    GLfloat s, t;

    Vertex(GLfloat x_, GLfloat y_, GLfloat z_, GLfloat nx_, GLfloat ny_, GLfloat nz_, GLfloat s_, GLfloat t_) :
        x(x_), y(y_), z(z_), nx(nx_), ny(ny_), nz(nz_), s(s_), t(t_) { }
};

struct TriFace
{
    int idx[3];

    TriFace(int f1, int f2, int f3) : idx{f1, f2, f3} {}
};

// window settings
int screenWidth = 800;
int screenHeight = 600;

constexpr auto PI = 3.14159265358979;

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

GLuint textures[10]{0};

Camera camera({0, 0, 3}, {0, 0, -1}, 1);

bool keys[1024]{false};

// callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void moveCamera(GLfloat deltaTime);

char Upper(const char& c) { return char(c - 32); }
void releaseTextures() { glDeleteTextures(10, textures); }
int bindUnitSphere(GLuint VAO, GLuint VBO, GLuint EBO, int sectorCount, int stackCount);

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
    glfwSetCursorPosCallback(window, mouseCallback);
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

	// bind VAO
	GLuint VAO;
	glGenVertexArrays(1, &VAO);

	// bind VBO, buffer data to it
	GLuint VBO;
	glGenBuffers(1, &VBO);

    GLuint EBO;
    glGenBuffers(1, &EBO);

    int numIdxSphere = bindUnitSphere(VAO, VBO, EBO, 20, 20);

	// ---------------------------------------------------------------

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);

	// Update loop
    GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
    GLfloat lastFrame = 0.0f;    // Time of last frame


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

        glm::mat4 model(1);
        glm::mat4 view = camera.ViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom()), float(screenWidth) / float(screenHeight), 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram->Program(), "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram->Program(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram->Program(), "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

        glBindTexture(GL_TEXTURE_2D, textures[3]);

		glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, numIdxSphere, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);

        arial.RenderText("Hello, hw3", 25, 25, 1.5, {0.8f, 0.7f, 0.3f}, screenWidth, screenHeight);

		// swap buffer
		glfwSwapBuffers(window);
	}

	// properly de-allocate all resources
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
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

int bindUnitSphere(GLuint VAO, GLuint VBO, GLuint EBO, int sectorCount, int stackCount)
{
    // Reference: http://www.songho.ca/opengl/gl_sphere.html
    std::vector<Vertex> vertices;

    float x, y, z, xy;                              // vertex position
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = cosf(stackAngle);             // r * cos(u)
        z = sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; ++j)     {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            vertices.emplace_back(x, y, z, x, y, z, s, t);
        }
    }

    std::vector<TriFace> faces;
    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)     {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0)         {
                faces.emplace_back(k1, k2, k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1))         {
                faces.emplace_back(k1 + 1, k2, k2 + 1);
            }
        }
    }

    glBindVertexArray(VAO);

    // buffer VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);

    // buffer EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(TriFace), &faces.front(), GL_STATIC_DRAW);

    // set vertex attribute pointers
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // texCoord attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // unbind
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return faces.size() * sizeof(TriFace);
}
