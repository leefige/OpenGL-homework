/*
 * OpenGL version 4.6 project.
 * Draw a Bezier surface with tesselation.
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

#include "camera.hpp"
#include "shader.hpp"

using namespace cg;

// window settings
int screenWidth = 800;
int screenHeight = 600;

// normalized coordinates
constexpr GLfloat vertices[] = {
        -1.5, 1., -4.,
        -0.5, 2., -4.,
        0., 0., -4.,
        0.5, 2., -4.,
        1.5, 1., -4.,

        -1.5, 2., -3.,
        -0.5, 1., -3.,
        0., 0.5, -3.,
        0.5, 1., -3.,
        1.5, 2., -3.,

        -1.5, 1., -2.,
        -0.5, -2., -2.,
        0., -1., -2.,
        0.5, 1., -2.,
        1.5, 0., -2.,

        -1.5, 0., -1.,
        -0.5, 1., -1.,
        0., 1.5, -1.,
        0.5, 0., -1.,
        1.5, -1., -1.,

        -1.5, 0., 0.,
        -0.5, 1., 0.,
        0., 1., 0.,
        0.5, -1., 0.,
        1.5, -1., 0.
};

float level = 5.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024]{false};

// callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void moveCamera(GLfloat deltaTime);
void changeScale(GLfloat deltaTime);

int main()
{
	// Setup a GLFW window

	// init GLFW, set GL version & pipeline info
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Yifei Li - Assignment 5", nullptr, nullptr);
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
	auto surfaceShader = Shader::Create("bezier.vert", "bezier.frag", "bezier.tesc", "bezier.tese");
	if (surfaceShader == nullptr) {
		std::cerr << "Error creating Shader Program" << std::endl;
		glfwTerminate();
		return -3;
	}

    auto pointShader = Shader::Create("bezier.vert", "point.frag");
    if (pointShader == nullptr) {
        std::cerr << "Error creating Shader Program" << std::endl;
        glfwTerminate();
        return -3;
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// set vertex attribute pointers
	// position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

	// unbind VBO & VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

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

		/* your update code here */
        moveCamera(deltaTime);
        changeScale(deltaTime);
	
		// draw background
		GLfloat red = 0.2f;
		GLfloat green = 0.3f;
		GLfloat blue = 0.3f;
		glClearColor(red, green, blue, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw a triangle
        glm::mat4 model(1);
        glm::mat4 view = camera.ViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom()), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

        // Draw Bezier surface
        surfaceShader->Use();
        glUniform1f(glGetUniformLocation(surfaceShader->Program(), "uOuter02"), level);
        glUniform1f(glGetUniformLocation(surfaceShader->Program(), "uOuter13"), level);
        glUniform1f(glGetUniformLocation(surfaceShader->Program(), "uInner0"), level);
        glUniform1f(glGetUniformLocation(surfaceShader->Program(), "uInner1"), level);
        glUniformMatrix4fv(glGetUniformLocation(surfaceShader->Program(), "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(surfaceShader->Program(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(surfaceShader->Program(), "model"), 1, GL_FALSE, glm::value_ptr(model));

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindVertexArray(VAO);
        glPatchParameteri(GL_PATCH_VERTICES, 25);
        glDrawArrays(GL_PATCHES, 0, 25);
        glBindVertexArray(0);

        // Draw control points
        pointShader->Use();
        glUniformMatrix4fv(glGetUniformLocation(pointShader->Program(), "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(pointShader->Program(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(pointShader->Program(), "model"), 1, GL_FALSE, glm::value_ptr(model));
        glPointSize(5.0f);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, 25);
        glBindVertexArray(0);

		// swap buffer
		glfwSwapBuffers(window);
	}

	// properly de-allocate all resources
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}

/* ======================== helper functions ======================== */

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // exit when pressing ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    } else if (key >= 0 && key < 1024) {
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
	glViewport(0, 0, width, height);
}

void changeScale(GLfloat deltaTime)
{
    if (keys[GLFW_KEY_Z] && level > 1) {
        if (level <= 20.0f)
            level -= deltaTime * 5.0f;
        else
            level -= deltaTime * 10.0f;
        level = level <= 1.0f ? 1.0f : level;
        std::cout << "\rLevel: " << level << "    ";
    }
    if (keys[GLFW_KEY_X] && level < 40) {
        if (level < 20.0f)
            level += deltaTime * 5.0f;
        else
            level += deltaTime * 10.0f;
        level = level >= 40.0f ? 40.0f : level;
        std::cout << "\rLevel: " << level << "    ";
    }
    if (keys[GLFW_KEY_C]) {
        /*drawMode = 1 - drawMode;
        std::cout << "\rDrawMode: " << drawMode << "    ";
        */
        keys[GLFW_KEY_C] = false;
    }
}
