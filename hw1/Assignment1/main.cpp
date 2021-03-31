/*
 * OpenGL version 3.3 project.
 */
#include <iostream>
#include <fstream>
#include <memory>
#include <cstdlib>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "obj.hpp"

using namespace cg;

// how to display: vertices, wireframe, faces, faces and edges
enum class DisplayType : int
{
    VERTEX = 0,
    FACE = 1,
    EDGE_FACE = 2,
    WIREFRAME = 3,
};

// generate a vertex with given postion and random color
struct VertRandColor
{
    GLfloat x, y, z, r, g, b;
    VertRandColor(const Vertex& v) :
        x(v.x), y(v.y), z(v.z),
        r(GLfloat(rand() % 224 + 32) / 255.0f),
        g(GLfloat(rand() % 224 + 32) / 255.0f),
        b(GLfloat(rand() % 224 + 32) / 255.0f) {}
};

// ================================================================

// window settings
constexpr int SCR_WIDTH = 800;
constexpr int SCR_HEIGHT = 600;

// object file path
const char* const OBJ_FILE = "eight.uniform.obj";

// point color: light green
constexpr GLfloat POINT_COLOR[3] = {0.1f, 0.95f, 0.1f};

constexpr float ROTATE_SPEED = glm::radians(5.0f);
constexpr float TRANSLATE_SPEED = 0.15f;

constexpr glm::vec3 GLM_UP(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 GLM_RIGHT(0.0f, 0.0f, 1.0f);
constexpr glm::vec3 GLM_DOWN = -GLM_UP;
constexpr glm::vec3 GLM_LEFT = -GLM_RIGHT;

// edge color: init to be light green
GLfloat edgeColor[3] = {0.1f, 0.95f, 0.1f};

// pointers to model / view / projection matrices
std::unique_ptr<glm::mat4> init_model = nullptr;
std::unique_ptr<glm::mat4> model = nullptr;
std::unique_ptr<glm::mat4> view = nullptr;
std::unique_ptr<glm::mat4> projection = nullptr;

// current display type, default to be faces only
DisplayType current_display = DisplayType::FACE;

// ================================================================

// callbacks function
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

// buffer binding & drawing functions
GLsizei bindFaces(GLuint VAO, GLuint VBO, const Obj& obj);
void drawFaces(const Shader& shader, GLuint VAO, int num);

GLsizei bindVertices(GLuint VAO, GLuint VBO, const Obj& obj);
void drawVertices(const Shader& shader, GLuint VAO, int num);

GLsizei bindEdges(GLuint VAO, GLuint VBO, const Obj& obj);
void drawEdges(const Shader& shader, GLuint VAO, int num);

// ================================================================

int main()
{
	// Setup a GLFW window

	// init GLFW, set GL version & pipeline info
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Yifei Li - Assignment 1: display a 3D object; press ESC to exit", nullptr, nullptr);
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
	auto faceShader = Shader::create("face.vert", "face.frag");
	if (faceShader == nullptr) {
		std::cerr << "Error creating Shader Program" << std::endl;
		glfwTerminate();
		return -3;
	}

    auto pointShader = Shader::create("point.vert", "point.frag");
    if (pointShader == nullptr) {
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

	// alloc & bind VAOs & VBOs for drawing faces, vertices and edges

    const int FACE_IDX = 0;
    const int VERT_IDX = 1;
    const int EDGE_IDX = 2;

    GLuint VAO[3]{0};
    GLuint VBO[3]{0};
    // record # of vertices to draw for each case
    GLsizei nVert[3]{0};

    glGenVertexArrays(3, VAO);
    glGenBuffers(3, VBO);

    nVert[FACE_IDX] = bindFaces(VAO[FACE_IDX], VBO[FACE_IDX], my_obj);
    nVert[VERT_IDX] = bindVertices(VAO[VERT_IDX], VBO[VERT_IDX], my_obj);
    nVert[EDGE_IDX] = bindEdges(VAO[EDGE_IDX], VBO[EDGE_IDX], my_obj);

	// ---------------------------------------------------------------

	// Define the viewport dimensions
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Create transformations
    init_model = std::make_unique<glm::mat4>(glm::rotate(
        glm::rotate(glm::mat4(1.0f), glm::radians(50.0f), GLM_UP),
        glm::radians(70.0f), GLM_RIGHT));
    //init_model = std::make_unique<glm::mat4>(1.0f);
    model = std::make_unique<glm::mat4>(*init_model);
    view = std::make_unique<glm::mat4>(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f)));
    projection = std::make_unique<glm::mat4>(
        glm::perspective(glm::radians(45.0f), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f));

    // Update loop

	while (glfwWindowShouldClose(window) == 0) {
		// check event queue
		glfwPollEvents();

		// draw background: dark gray
		glClearColor(0.1f, 0.1f, 0.1f, 0.9f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// display
        switch (current_display) {
        case DisplayType::VERTEX:
            drawVertices(*pointShader, VAO[VERT_IDX], nVert[VERT_IDX]);
            break;
        case DisplayType::WIREFRAME:
            drawEdges(*pointShader, VAO[EDGE_IDX], nVert[EDGE_IDX]);
            break;
        case DisplayType::FACE:
            drawFaces(*faceShader, VAO[FACE_IDX], nVert[FACE_IDX]);
            break;
        case DisplayType::EDGE_FACE:
            drawFaces(*faceShader, VAO[FACE_IDX], nVert[FACE_IDX]);
            drawEdges(*pointShader, VAO[EDGE_IDX], nVert[EDGE_IDX]);
            break;
        default:
            std::cerr << "Unsupported display type: " << int(current_display) << std::endl;
            current_display = DisplayType::FACE;
            break;
        }

		// swap buffer
		glfwSwapBuffers(window);
	}

	// properly de-allocate all resources
	glDeleteVertexArrays(3, VAO);
	glDeleteBuffers(3, VBO);

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
    else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        model = std::make_unique<glm::mat4>(glm::rotate(*model, ROTATE_SPEED, GLM_UP));
    }
    else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        model = std::make_unique<glm::mat4>(glm::rotate(*model, ROTATE_SPEED, GLM_DOWN));
    }
    else if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        model = std::make_unique<glm::mat4>(glm::rotate(*model, ROTATE_SPEED, GLM_LEFT));
    }
    else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        model = std::make_unique<glm::mat4>(glm::rotate(*model, ROTATE_SPEED, GLM_RIGHT));
    }
    // translate object
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        model = std::make_unique<glm::mat4>(glm::translate(*model, GLM_RIGHT * TRANSLATE_SPEED));
    }
    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        model = std::make_unique<glm::mat4>(glm::translate(*model, GLM_LEFT * TRANSLATE_SPEED));
    }
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        model = std::make_unique<glm::mat4>(glm::translate(*model, GLM_UP * TRANSLATE_SPEED));
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        model = std::make_unique<glm::mat4>(glm::translate(*model, GLM_DOWN * TRANSLATE_SPEED));
    }
    // reset object position
    else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        model = std::make_unique<glm::mat4>(*init_model);
    }
    // switch display type
    else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        current_display = DisplayType((int(current_display) + 1) % int(sizeof(DisplayType)));
    }
    // change wireframe color
    else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        for (int i = 0; i < 3; i++) {
            edgeColor[i] = GLfloat((rand() % 128 + 128) / 255.0f);
        }
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	// resize window
	glViewport(0, 0, width, height);
}

GLsizei bindFaces(GLuint VAO, GLuint VBO, const Obj& obj)
{
    // bind VAO
    glBindVertexArray(VAO);

    // bind VBO, buffer data to it
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    std::vector<VertRandColor> data;
    for (const auto& face : obj.faces) {
        for (int i = 0; i < 3; i++) {
            int vid = face[i] - 1;
            data.emplace_back(obj.vertices[vid]);
        }
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(VertRandColor) * data.size(), &data.front(), GL_STATIC_DRAW);

    // set vertex attribute pointers
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertRandColor), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertRandColor), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // unbind VBO & VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return GLsizei(data.size());
}

void drawFaces(const Shader& shader, GLuint VAO, int num)
{
    shader.use();

    // get uniform locations
    GLint modelLoc = glGetUniformLocation(shader.getProgram(), "model");
    GLint viewLoc = glGetUniformLocation(shader.getProgram(), "view");
    GLint projLoc = glGetUniformLocation(shader.getProgram(), "projection");

    // pass uniform values to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(*model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(*view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(*projection));

    glBindVertexArray(VAO);
    // draw flat color for each face instead of interpolating
    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
    glDrawArrays(GL_TRIANGLES, 0, num);
    glBindVertexArray(0);
}

GLsizei bindVertices(GLuint VAO, GLuint VBO, const Obj& obj)
{
    // bind VAO
    glBindVertexArray(VAO);

    // bind VBO, buffer data to it
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * obj.numVertices(), &obj.vertices.front(), GL_STATIC_DRAW);

    // set vertex attribute pointers
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // unbind VBO & VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return GLsizei(obj.numVertices());
}

void drawVertices(const Shader& shader, GLuint VAO, int num)
{
    shader.use();

    // get uniform locations
    GLint modelLoc = glGetUniformLocation(shader.getProgram(), "model");
    GLint viewLoc = glGetUniformLocation(shader.getProgram(), "view");
    GLint projLoc = glGetUniformLocation(shader.getProgram(), "projection");

    // pass uniform values to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(*model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(*view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(*projection));

    // use the same color for all points
    GLint colorLoc = glGetUniformLocation(shader.getProgram(), "ourColor");
    glUniform3fv(colorLoc, 1, POINT_COLOR);

    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, num);
    glBindVertexArray(0);
}

GLsizei bindEdges(GLuint VAO, GLuint VBO, const Obj& obj)
{
    // bind VAO
    glBindVertexArray(VAO);

    // bind VBO, buffer data to it
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // each face has 3 edges, though we are drawing one edge twice...
    std::vector<Vertex> data;
    for (const auto& face : obj.faces) {
        int vid[3] = {face[0] - 1, face[1] - 1, face[2] - 1};
        // edge (0, 1)
        data.push_back(obj.vertices[vid[0]]);
        data.push_back(obj.vertices[vid[1]]);
        // edge (0, 2)
        data.push_back(obj.vertices[vid[0]]);
        data.push_back(obj.vertices[vid[2]]);
        // edge (1, 2)
        data.push_back(obj.vertices[vid[1]]);
        data.push_back(obj.vertices[vid[2]]);
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * data.size(), &data.front(), GL_STATIC_DRAW);

    // set vertex attribute pointers
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // unbind VBO & VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return GLsizei(data.size());
}

void drawEdges(const Shader& shader, GLuint VAO, int num)
{
    shader.use();

    // get uniform locations
    GLint modelLoc = glGetUniformLocation(shader.getProgram(), "model");
    GLint viewLoc = glGetUniformLocation(shader.getProgram(), "view");
    GLint projLoc = glGetUniformLocation(shader.getProgram(), "projection");

    // pass uniform values to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(*model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(*view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(*projection));

    // use the same color for all points
    GLint colorLoc = glGetUniformLocation(shader.getProgram(), "ourColor");
    glUniform3fv(colorLoc, 1, edgeColor);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, num);
    glBindVertexArray(0);
}
