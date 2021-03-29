/*
 * OpenGL version 3.3 project.
 */
#include <iostream>
#include <fstream>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "obj.hpp"

using namespace cg;

// window settings
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

const char* const OBJ_FILE = "eight.uniform.obj";

std::unique_ptr<glm::mat4> model = nullptr;
std::unique_ptr<glm::mat4> view = nullptr;
std::unique_ptr<glm::mat4> projection = nullptr;

// callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

// drawing function
void bindFaces(GLuint VAO, GLuint VBO, const Obj& obj);
void drawFaces(const Shader& shader, GLuint VAO, int num);


int main()
{
	// Setup a GLFW window

	// init GLFW, set GL version & pipeline info
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 1: display a 3D object", nullptr, nullptr);
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
	auto shaderProgram = Shader::create("VertexShader.vert", "FragmentShader.frag");
	if (shaderProgram == nullptr) {
		std::cerr << "Error creating Shader Program" << std::endl;
		glfwTerminate();
		return -3;
	}

    // ---------------------------------------------------------------
    // load model
    std::ifstream objfile;
    Obj my_obj;

    // ensures ifstream objects can throw exceptions:
    objfile.exceptions(std::ifstream::badbit);
    try {
        objfile.open(OBJ_FILE, std::ios::in);
        objfile >> my_obj;
        objfile.close();
    }
    catch (const std::ifstream::failure& e) {
        std::cerr << "Load obj file '" << OBJ_FILE << "' error: " << e.what() << std::endl;
        glfwTerminate();
        return -4;;
    }

    // ---------------------------------------------------------------

	// Set up vertex data (and buffer(s)) and attribute pointers
    GLuint VAO;
    GLuint VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    bindFaces(VAO, VBO, my_obj);
	

	// ---------------------------------------------------------------

	// Define the viewport dimensions
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	// Update loop

    // Create transformations
    glm::mat4 init_model = glm::rotate(
        glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::radians(90.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    model = std::make_unique<glm::mat4>(init_model);
    view = std::make_unique<glm::mat4>(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f)));
    // Note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
    projection = std::make_unique<glm::mat4>(glm::perspective(glm::radians(45.0f), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f));

    GLfloat ourColor[3] = {0.9f, 0.9f, 0.95f};

	while (glfwWindowShouldClose(window) == 0) {
		// check event queue
		glfwPollEvents();

		/* your update code here */
	
		// draw background
		glClearColor(0.3f, 0.5f, 0.4f, 0.6f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // get uniform locations
        GLint modelLoc = glGetUniformLocation(shaderProgram->getProgram(), "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram->getProgram(), "view");
        GLint projLoc = glGetUniformLocation(shaderProgram->getProgram(), "projection");
        GLint colorLoc = glGetUniformLocation(shaderProgram->getProgram(), "ourColor");

        // pass uniform values to shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(*model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(*view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(*projection));
        glUniform3fv(colorLoc, 1, ourColor);

		// draw a triangle
        drawFaces(*shaderProgram, VAO, my_obj.numTriangles() * 3);

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
    }
    // rotate object
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        model = std::make_unique<glm::mat4>(glm::rotate(*model, glm::radians(5.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    }
    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        model = std::make_unique<glm::mat4>(glm::rotate(*model, glm::radians(5.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    }
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        model = std::make_unique<glm::mat4>(glm::rotate(*model, glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        model = std::make_unique<glm::mat4>(glm::rotate(*model, glm::radians(5.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	// resize window
	glViewport(0, 0, width, height);
}

void bindFaces(GLuint VAO, GLuint VBO, const Obj& obj)
{
    // bind VAO
    glBindVertexArray(VAO);

    // bind VBO, buffer data to it
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    std::vector<Obj::Vertex> data;
    for (const auto& face : obj.faces) {
        for (int i = 0; i < 3; i++) {
            int vid = face[i] - 1;
            data.push_back(obj.vertices[vid]);
        }
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(Obj::Vertex) * data.size(), &data.front(), GL_STATIC_DRAW);

    // set vertex attribute pointers
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // unbind VBO & VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawFaces(const Shader& shader, GLuint VAO, int num)
{
    shader.use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, num);
    glBindVertexArray(0);
}
